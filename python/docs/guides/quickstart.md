# Quick Start

`libscid` provides modern Python bindings for the high-performance `libscid` C++ chess engine backend.

---

## 1. Installation

`libscid` is distributed as a pre-compiled Python wheel bundling the native `libscid` shared library.

```bash
pip install libscid
```

For development or building from source:

```bash
git clone https://github.com/bahmanm/libscid.git
cd libscid/python
uv sync
```

---

## 2. Basic Workflow

### 2.1. Creating and Inspecting a Game

```python
import libscid

# Create a fresh standard starting game
game = libscid.Game()
cursor = game.create_cursor()

print(f"Current position FEN: {cursor.position.fen}")
print(f"Side to move: {cursor.position.side_to_move}")
print(f"Is check: {cursor.position.is_check}")
```

### 2.2. Parsing PGN Movetext

```python
import libscid

pgn_text = """[Event "World Championship 35th-Playoff"]
[Site "Moscow"]
[Date "1985.10.15"]
[Round "16"]
[White "Karpov, Anatoly"]
[Black "Kasparov, Garry"]
[Result "0-1"]
[ECO "B44"]

1. e4 c5 2. Nf3 e6 3. d4 cxd4 4. Nxd4 Nc6 5. Nb5 d6 6. c4 Nf6 7. N1c3 a6 8. Na3 d5 0-1
"""

game = libscid.Game.from_pgn(pgn_text)
print(f"White: {game.get_tag('White')}")
print(f"Black: {game.get_tag('Black')}")
print(f"Total mainline moves: {game.mainline_move_count}")
```

### 2.3. Sequential Move Navigation

```python
import libscid

game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 3. Bb5 a6 4. Ba4 Nf6 *")
cursor = game.create_cursor()

while not cursor.is_line_end:
    cursor = cursor.next()
    move_san = cursor.previous_move_san
    print(f"Played move: {move_san:<6} -> FEN: {cursor.position.fen}")
```

---

## 3. Next Steps

- Explore [Game Traversal & Move Tree](game_traversal.md) for immutable cursor navigation and variation management.
- Learn about [Database Indexing & Searching](database_search.md) for high-speed PGN archive indexing.
- Read [Movetext Event Iteration](movetext_iteration.md) for structured AST stream processing.
