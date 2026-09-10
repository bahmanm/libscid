# How to Filter and Search Databases

This guide demonstrates how to index PGN archives, build multi-criteria header queries, combine filter bitsets with boolean algebra, and paginate sorted game indices.

---

## 1. Opening PGN Archives in Read-Only Mode

`libscid` parses and indexes PGN archives into memory on opening, allowing fast random access and searching:

```python
import libscid

# Open PGN archive:
db = libscid.Database.open_pgn_read_only("championships.pgn")
print(f"Total games indexed: {db.game_count}")
print(f"Database type:       {db.type}")
```

To monitor indexing progress on large multi-megabyte archives, supply a progress callback:

```python
def on_progress(done: int, total: int, message: str | None) -> None:
    percent = (done / total) * 100 if total > 0 else 0
    print(f"Indexing progress: {percent:.1f}% ({done}/{total})")

db = libscid.Database.open_pgn_read_only(
    "large_archive.pgn",
    progress_report_callback=on_progress,
)
```

---

## 2. Multi-Criteria Header Queries

Use [`HeaderCriteria`][libscid.HeaderCriteria] to filter games by player names, Elo ratings, ECO classification codes, tournament dates, and results:

```python
import libscid

db = libscid.Database.open_pgn_read_only("master_games.pgn")

# Construct query criteria:
criteria = libscid.HeaderCriteria(
    white="Kasparov",
    result="1-0",
    white_elo_min=2700,
    eco_min="B80",
    eco_max="B89",
    year_min=1985,
    year_max=2000,
)

# Execute search against database index:
result_filter = db.search.headers(criteria)
print(f"Found {result_filter.game_count} matching games.")
```

---

## 3. Chaining Searches with Source and Destination Filters

A [`Filter`][libscid.Filter] maintains a bitset of selected game indices. You can chain searches by passing a previously computed filter as the `source` parameter:

```python
import libscid

db = libscid.Database.open_pgn_read_only("master_games.pgn")

# Step 1: Query all games played by Kasparov
f_kasparov = db.search.headers(
    libscid.HeaderCriteria(player="Kasparov")
)
print(f"Total Kasparov games: {f_kasparov.game_count}")

# Step 2: From the Kasparov subset, narrow down to decisive Sicilian Defences
f_sicilian = db.search.headers(
    libscid.HeaderCriteria(
        eco_min="B20",
        eco_max="B99",
        result=["1-0", "0-1"],
    ),
    source=f_kasparov,
)
print(f"Decisive Kasparov Sicilians: {f_sicilian.game_count}")
```

---

## 4. Paginating and Sorting Query Results

To display search results in user interfaces or CLI tables without loading every game into memory, fetch windowed slices of sorted game indices:

```python
import libscid

db = libscid.Database.open_pgn_read_only("master_games.pgn")
result_filter = db.filters.primary

# Fetch the top 20 games sorted by Date descending ("D-"):
# Sort criteria codes: "D-" (Date descending), "D+" (Date ascending),
#                      "W-" (White descending), "W+" (White ascending),
#                      "B-" (Black descending), "B+" (Black ascending).
indices = result_filter.get_game_indices(
    sort_criteria="D-",
    start_row=0,
    row_count=20,
)

for row_num, idx in enumerate(indices, start=1):
    date = db.get_tag(idx, "Date")
    white = db.get_tag(idx, "White")
    black = db.get_tag(idx, "Black")
    result = db.get_tag(idx, "Result")
    print(f"{row_num:02d}. [{date}] {white} vs {black}: {result}")
```

---

## 5. Lazy Game Deserialisation

Header indices allow filtering thousands of games instantaneously. When a specific game must be analysed, call [`get_game`][libscid.Database.get_game] to parse its full movetext lazily:

```python
# Deserialise only the selected game:
game = db.get_game(indices[0])
cursor = game.create_cursor()

print(f"Mainline plies: {game.mainline_move_count}")
print(f"Opening move:   {cursor.next().previous_move_san}")
```

---

## 6. Related Topics

- For a guided beginner walkthrough, see [Analysing Master Archives](../tutorials/database-queries.md).
- To understand bitset performance and index layout, read [Explanation: In-Memory Indexing and Filter Algebra](../explanation/indexing-and-filters.md).
- For complete method signatures, inspect the [Database and Search Reference](../reference/database.md).
