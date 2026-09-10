# Analysing Master Archives

This tutorial walks you through opening and querying chess game databases using `libscid`. You will learn how to open PGN archives, build multi-criteria header queries, inspect filter subsets, and extract game records efficiently.

---

## 1. The PGN Indexing Model

Parsing thousands of games with standard PGN parsers is notoriously slow because every ply of every game must be lexed, validated, and converted into board structures.

`libscid` bypasses this overhead by constructing an in-memory index table of game offsets and Seven Tag Roster (STR) metadata. When you open a PGN archive in read-only mode, the file is scanned once to build header indices, after which search queries and metadata extractions execute almost instantaneously.

---

## 2. Preparing a Sample Database

For this lesson, create a Python script `query_archive.py` that writes a small multi-game archive and opens it:

```python
import pathlib
import libscid

# Sample PGN archive containing games from various eras
ARCHIVE_PGN = """[Event "World Championship 31th"]
[Site "Moscow"]
[Date "1984.10.05"]
[Round "9"]
[White "Karpov, Anatoly"]
[Black "Kasparov, Garry"]
[Result "1-0"]
[ECO "E15"]

1. d4 Nf6 2. c4 e6 3. Nf3 b6 4. g3 Ba6 5. b3 Bb4+ 6. Bd2 Be7 1-0

[Event "World Championship 32th"]
[Site "Moscow"]
[Date "1985.11.09"]
[Round "24"]
[White "Karpov, Anatoly"]
[Black "Kasparov, Garry"]
[Result "0-1"]
[ECO "B44"]

1. e4 c5 2. Nf3 e6 3. d4 cxd4 4. Nxd4 Nc6 5. Nb5 d6 6. c4 Nf6 0-1

[Event "Linares 12th"]
[Site "Linares"]
[Date "1994.02.27"]
[Round "13"]
[White "Kasparov, Garry"]
[Black "Topalov, Veselin"]
[Result "1-0"]
[ECO "B80"]

1. e4 c5 2. Nf3 d6 3. d4 cxd4 4. Nxd4 Nf6 5. Nc3 a6 6. Be3 e6 1-0
"""

# Write to a temporary file
archive_path = pathlib.Path("masters.pgn")
archive_path.write_text(ARCHIVE_PGN, encoding="utf-8")
```

---

## 3. Opening the Database and Reading Headers

Open the archive using [`Database.open_pgn_read_only`][libscid.Database.open_pgn_read_only]:

```python
db = libscid.Database.open_pgn_read_only("masters.pgn")

print(f"Total games in archive: {db.game_count}")
print(f"Read-only mode:         {db.read_only}")
```

Inspect game headers directly by row index without parsing full movetext:

```python
for idx in range(db.game_count):
    white = db.get_tag(idx, "White")
    black = db.get_tag(idx, "Black")
    date = db.get_tag(idx, "Date")
    result = db.get_tag(idx, "Result")
    eco = db.get_tag(idx, "ECO")
    print(f"Game #{idx}: [{date}] {white} vs {black} ({result}) [ECO: {eco}]")
```

---

## 4. Multi-Criteria Header Searching

The [`DatabaseSearch`][libscid.DatabaseSearch] engine allows searching against indexed header attributes using [`HeaderCriteria`][libscid.HeaderCriteria]:

```python
# Construct search parameters: find games won by Kasparov with White
criteria = libscid.HeaderCriteria(
    white="Kasparov",
    result="1-0",
)

# Execute the search into a Filter bitset:
result_filter = db.search.headers(criteria)

print(f"Matching games found: {result_filter.game_count}")
```

---

## 5. Working with Filter Bitsets

A [`Filter`][libscid.Filter] represents a selection subset of game indices matching a query. Filters support boolean algebra and sorted index extraction:

```python
# Retrieve matching game indices sorted by Date descending ("D-"):
indices = result_filter.get_game_indices(sort_criteria="D-", start_row=0, row_count=10)

print("Matching games sorted by date descending:")
for idx in indices:
    print(f"- {db.get_tag(idx, 'Date')}: {db.get_tag(idx, 'White')} vs {db.get_tag(idx, 'Black')}")
```

---

## 6. Deserialising Full Games on Demand

When you need to analyse moves, board positions, or variations for a specific query hit, deserialise the game using [`db.get_game(index)`][libscid.Database.get_game]:

```python
for idx in indices:
    # Full game is parsed on-demand from disk:
    game = db.get_game(idx)
    cursor = game.create_cursor()

    # Step to move 1
    c1 = cursor.next()
    print(f"Game #{idx} opening move: {c1.previous_move_san}")
```

---

## 7. Clean-up

When finished, close the database handle and delete the sample file:

```python
db.close()
archive_path.unlink()
```

---

## 8. Summary and Next Steps

In this tutorial, you have learnt how to:

- Open and inspect PGN databases using [`Database`][libscid.Database].
- Query header metadata without decoding movetext using [`HeaderCriteria`][libscid.HeaderCriteria].
- Manipulate selection results with [`Filter`][libscid.Filter].
- Load individual games lazily on demand with [`db.get_game(index)`][libscid.Database.get_game].

To learn how to combine filters with union and intersection, visit [How-To: Filter and Search Databases](../how-to/filter-and-search-databases.md). To understand the index storage mechanics, read [Explanation: In-Memory Indexing and Filter Algebra](../explanation/indexing-and-filters.md).
