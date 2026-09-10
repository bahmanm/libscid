# In-Memory Indexing and Filter Algebra

This document explains the algorithmic foundations of `libscid` database indexing and query evaluation, showing why in-memory header tables and bitset filter algebra deliver instantaneous searches across large PGN archives.

---

## 1. The Bottleneck of Naive PGN Processing

Standard PGN files are sequential, unstructured text files. In typical Python chess packages, querying games (e.g. finding games where Garry Kasparov won as White) requires sequentially parsing every single game:

- Parsing Overhead: The parser must tokenize tags, parse move tokens, validate legality against board rules, and construct full move trees.
- Memory Pressure: Allocating Python objects for thousands of moves consumes gigabytes of heap memory.
- Throughput Ceiling: Sequential full parsing generally maxes out at a few thousand games per second on modern hardware, making searches across archives of 500,000 games take minutes.

---

## 2. The Two-Phase Indexing Architecture

`libscid` solves this performance bottleneck by decoupling header indexing from movetext decoding:

```
Phase 1: Ingest & Fast Header Indexing (Once upon open)
┌────────────────────────────────────────────────────────┐
│ PGN Text File (.pgn)                                   │
└───────────────────────────┬────────────────────────────┘
                            │ Scan tags & byte offsets
┌───────────────────────────▼────────────────────────────┐
│ In-Memory Database Index                               │
│  - Array of Game Headers (STR: White, Black, Date, etc)│
│  - Array of File Offsets & Byte Lengths                │
│  - Game Count N                                        │
└────────────────────────────────────────────────────────┘

Phase 2: Instant Query Evaluation (Multi-criteria searches)
┌────────────────────────────────────────────────────────┐
│ Filter Bitsets (64-bit word vectors)                   │
│  - Instant bitwise operations: AND, OR, NOT            │
│  - Fast sorting and windowed index extraction          │
└───────────────────────────┬────────────────────────────┘
                            │ On-demand lazy fetch
Phase 3: Targeted Game Deserialisation
┌────────────────────────────────────────────────────────┐
│ Deserialise only matching games: db.get_game(index)    │
└────────────────────────────────────────────────────────┘
```

1. Index Scan on Open: When [`Database.open_pgn_read_only`][libscid.Database.open_pgn_read_only] is called, the C++ engine scans the file to locate game boundaries (`[Event ...]` tags) and extracts header metadata into contiguous memory arrays. Movetext bodies are skipped entirely during this pass.
2. Direct Header Queries: Because player names, ratings, results, dates, and ECO codes reside in contiguous C++ arrays, searches evaluate without touching disk or reading movetext.
3. Lazy On-Demand Deserialisation: Only when client code explicitly requests [`db.get_game(index)`][libscid.Database.get_game] does the engine seek to the exact file offset and parse the movetext for that single contest.

---

## 3. Filter Bitsets and Set Algebra

A [`Filter`][libscid.Filter] represents an arbitrary selection subset of game indices from a database containing $N$ games:

### 3.1. Compact Word Bitset Storage

Internally, a filter is implemented as an array of 64-bit unsigned integers (`uint64_t`). For a database of 1,000,000 games:

$$\text{Memory required} = \frac{1,000,000 \text{ bits}}{8 \times 1024} \approx 122 \text{ KB}$$

An entire million-game selection set is represented in just 122 kilobytes of memory, fitting entirely within CPU L2 cache.

### 3.2. Fast Bitwise Algebra

Filter combinations operate directly on 64-bit machine words:

- Union (Bitwise OR): Computes bitwise OR across words (`word1 |= word2`).
- Intersection (Bitwise AND): Computes bitwise AND across words (`word1 &= word2`).
- Difference (Bitwise AND NOT): Computes bitwise AND NOT (`word1 &= ~word2`).
- Negation (Bitwise NOT): Inverts all bits (`word1 = ~word1`).

A union or intersection across 1,000,000 games requires only 15,625 machine instructions, executing in less than a single millisecond.

---

## 4. Sorted Window Extraction

When querying a database of 500,000 games, an interface or report typically needs only the top 20 or 50 games sorted by a specific field (such as Date descending or Player name ascending).

Calling [`filter.get_game_indices(sort_criteria="D-", start_row=0, row_count=20)`][libscid.Filter.get_game_indices]:

1. Gathers the active indices from the filter bitset.
2. Applies sorting against the contiguous in-memory header index table.
3. Returns only the requested slice of integers, avoiding both full-database sort overhead and unnecessary Python object allocations.

---

## 5. Architectural Summary

- Throughput: Header queries run at native C speed across compact memory arrays.
- Memory Efficiency: Selections of hundreds of thousands of games are represented as compact bitsets.
- Lazy I/O: Disk access for movetext occurs only when individual games are analysed.
