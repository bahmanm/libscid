# How-To Guides

How-to guides provide goal-oriented, step-by-step instructions to solve specific problems and accomplish real-world tasks using the libscid C ABI.

---

## 1. Setup and Environment

- [Build and Install libscid](installation.md): Compiling from source, installing release archives, and linking with CMake, Make, or cc.

---

## 2. Games and PGN

- Parse and Edit PGN Tags: Reading PGN games, inspecting header metadata, modifying tags, and serialising back to PGN strings.
- Author Games with Variations: Constructing game moves, appending comments, assigning NAG annotations, and building variations.
- Nonstandard Start Positions: Constructing games starting from arbitrary FEN board setups.
- Navigate Variation Trees: Moving through branching game trees using immutable game cursors.
- Mutate and Merge Move Trees: Truncating variations and merging moves from one game into another.

---

## 3. Board Position and Classification

- Apply Moves and Inspect State: Mutating board positions, verifying check/checkmate, and enumerating legal moves.
- Classify Positions with ECO: Loading opening books, looking up ECO codes, and retrieving opening nomenclature.

---

## 4. Database Operations

- In-Memory Databases: Managing transient game databases and filtered subsets.
- Enumerate Database Games: Listing games with lightweight metadata without loading entire game trees.
- Edit Database Games: Replacing stored games and toggling deleted flags.
- Persistent SCID5 Databases: Creating, flushing, saving, and reopening native SCID5 database collections.
- Batch Import and Export PGN: Importing multi-game PGN streams directly into databases and batch exporting.

---

## 5. Database Search

- Search by Header Criteria: Filtering games by player name, Elo range, event, date, or result.
- Search by Exact Position: Locating all games containing an exact board layout.
- Search by Board and Pawn Structure: Finding games matching specific material distributions and pawn structures.
