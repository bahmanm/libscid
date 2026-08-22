# Database Indexing & Searching

`libscid` provides database indexing for PGN archives and Scid binary databases, allowing instant multi-criteria querying and filtering across hundreds of thousands of games.

---

## 1. Opening Databases

`libscid` opens PGN files in read-only mode by building an in-memory index table of header metadata without needing to parse the full movetext up front.

```python
import libscid

# Open a PGN archive in read-only mode:
db = libscid.Database.open_pgn_read_only("championships.pgn")
print(f"Database type: {db.type}")
print(f"Total games indexed: {db.game_count}")
print(f"Read-only mode: {db.read_only}")
```

---

## 2. Multi-Criteria Header Search

The [`HeaderCriteria`][libscid.HeaderCriteria] dataclass defines query filters across player names, ratings, results, ECO codes, and dates.

```python
import libscid

db = libscid.Database.open_pgn_read_only("games.pgn")

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

## 3. Working with Filters

A [`Filter`][libscid.Filter] represents a selection (bitset) of game indices matching a query.

### 3.1. Iterating & Paginating Results

```python
import libscid

db = libscid.Database.open_pgn_read_only("games.pgn")
result_filter = db.filters.primary

# Fetch top 20 games sorted by Date descending ("D-"):
indices = result_filter.get_game_indices(sort_criteria="D-", start_row=0, row_count=20)

for idx in indices:
    white = db.get_tag(idx, "White")
    black = db.get_tag(idx, "Black")
    date = db.get_tag(idx, "Date")
    result = db.get_tag(idx, "Result")
    print(f"[{date}] {white} vs {black}: {result}")
```

### 3.2. Filter Boolean Operations

Filters can be combined using logical operations:

```python
import libscid

db = libscid.Database.open_pgn_read_only("games.pgn")

f1 = db.filters.create()
f2 = db.filters.create()

db.search.headers(libscid.HeaderCriteria(white="Carlsen"), dest_filter=f1)
db.search.headers(libscid.HeaderCriteria(white="Nakamura"), dest_filter=f2)

# Combine results using union
f1.union_with(f2)
print(f"Total games played as White by Carlsen or Nakamura: {f1.game_count}")
```
