# libscid Python Bindings

`libscid` provides Python bindings for the `libscid` C ABI, delivering high-performance chess operations, PGN parsing and authoring, immutable game navigation cursors, position analysis, and Scid database access.

## Quick Start

```python
import libscid

# Parse a game from PGN
game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 3. Bb5 a6 *")
print(f"Mainline moves: {game.mainline_move_count}")

# Traverse move-by-move with an immutable cursor
cursor = game.create_cursor()
while not cursor.is_line_end:
    cursor = cursor.next()
    print(f"Move: {cursor.previous_move_san}, FEN: {cursor.position.fen}")
```

## Features

- **Game & Move Management**: Full PGN standard compliance with recursive variations, comments, and NAGs (Numeric Annotation Glyphs).
- **Immutable Cursors**: Safe, predictable navigation and editing of game trees.
- **Position & Arbiter**: FEN import/export, legal move generation, check/checkmate detection, threefold repetition, and fifty-move rule evaluation.
- **Scid Databases**: Read and search Scid databases with customizable criteria and filters.

## Navigation

- [API Reference](reference.md): Detailed reference for all public classes, methods, and types.
