# How-To Guides

How-to guides provide goal-oriented, step-by-step instructions to solve specific problems and accomplish real-world tasks using the libscid C ABI.

---

## 1. Setup and Environment

- [Build and Install libscid](installation.md): Compiling from source, installing release archives, and linking with CMake, Make, or cc.

---

## 2. Games and PGN

- [Parse and Edit PGN Tags](edit-pgn.md): Reading PGN games, inspecting header metadata, modifying tags, and serialising back to PGN strings.
- [Author Games with Variations](author-pgn.md): Constructing game moves, appending comments, assigning NAG annotations, and building variations.
- [Nonstandard Start Positions](nonstandard-start.md): Constructing games starting from arbitrary FEN board setups.
- [Navigate Variation Trees](navigate-pgn.md): Moving through branching game trees using immutable game cursors.
- [Mutate and Merge Move Trees](mutate-pgn.md): Truncating variations and merging moves from one game into another.

---

## 3. Board Position and Classification

- [Apply Moves and Enumerate Legal Moves](position-and-moves.md): Mutating board positions, verifying check/checkmate, and enumerating legal moves.
- [Classify Positions with ECO](eco-lookup.md): Loading opening books, looking up ECO codes, and retrieving opening nomenclature.

---

## 4. Database Operations

- [Work with In-Memory Databases](memory-database.md): Managing transient game databases and filtered subsets.
- [Enumerate Games via Lightweight Metadata](list-database.md): Listing games with lightweight metadata without loading entire game trees.
- [Replace Games and Manage Deleted Flags](edit-database.md): Replacing stored games and toggling deleted flags.
- [Create and Open Persistent SCID5 Databases](persistent-database.md): Creating, flushing, saving, and reopening native SCID5 database collections.
- [Batch Import and Export PGN](import-export-pgn.md): Importing multi-game PGN streams directly into databases and batch exporting.

---

## 5. Database Search

- [Filter Games by Header Criteria](search-headers.md): Filtering games by player name, Elo range, event, date, or result.
- [Search Games by Exact Position](search-position.md): Locating all games containing an exact board layout.
- [Search Games by Board and Pawn Structure](search-board.md): Finding games matching specific material distributions and pawn structures.
