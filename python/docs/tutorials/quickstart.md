# First Steps with libscid

This tutorial guides you through installing `libscid`, initialising a chess game, playing a short sequence of moves, and inspecting the board state.

---

## 1. Prerequisites and Installation

`libscid` publishes pre-compiled wheels for macOS, Linux, and Windows bundling the native C engine. Install the package into your virtual environment:

```bash
pip install libscid
```

If you are working from the repository checkout directly:

```bash
cd libscid/python
uv sync
```

Verify that the native module loads cleanly in Python:

```bash
python -c "import libscid; print(libscid.__version__)"
```

---

## 2. Initialising a Chess Game

Start an interactive Python session and import the library:

```python
import libscid

# Create a game starting from the standard initial chess array:
game = libscid.Game()
```

The [`Game`][libscid.Game] object acts as the root container. It encapsulates the Seven Tag Roster (STR) metadata headers, custom PGN tags, and the hierarchical move tree.

Inspect the default tags assigned to new games:

```python
print(game.get_tag("Event"))  # '?'
print(game.get_tag("White"))  # '?'
print(game.get_tag("Black"))  # '?'
print(game.get_tag("Result"))  # '*'
```

Assign descriptive contest metadata:

```python
game.set_tag("Event", "World Chess Championship")
game.set_tag("Site", "London, UK")
game.set_tag("Date", "2024.11.20")
game.set_tag("White", "Gukesh D")
game.set_tag("Black", "Ding Liren")
game.set_tag("Result", "*")
```

---

## 3. Navigating with Cursors

In `libscid`, you do not mutate board positions directly on the game object. Instead, you create a [`Cursor`][libscid.Cursor] that points to a specific ply in the game tree:

```python
cursor = game.create_cursor()
print(f"At root node: {cursor.is_line_start}")
```

Inspect the board position at this starting cursor:

```python
pos = cursor.position
print(f"Side to move: {pos.side_to_move}")
print(f"Fullmove number: {pos.fullmove_number}")
print(f"FEN: {pos.fen}")
```

Notice that [`pos.fen`][libscid.Position.fen] outputs the standard starting Forsyth-Edwards Notation string:

```
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
```

---

## 4. Playing Moves along the Mainline

To advance the game, append moves using Standard Algebraic Notation (SAN):

```python
# Play 1. e4
cursor = cursor.append_move("e4")
print(f"Last move: {cursor.previous_move_san}")
print(f"Active side: {cursor.position.side_to_move}")

# Play 1... e5
cursor = cursor.append_move("e5")
print(f"Last move: {cursor.previous_move_san}")

# Play 2. Nf3
cursor = cursor.append_move("Nf3")

# Play 2... Nc6
cursor = cursor.append_move("Nc6")
```

Check the total number of mainline moves recorded on the game aggregate:

```python
print(f"Mainline plies: {game.mainline_move_count}")  # 4
```

---

## 5. Stepping Through History

`libscid` cursors are immutable value pointers. Navigating backwards or forwards yields new cursor instances rather than mutating your existing variable:

```python
# Jump back to the start of the game:
start_cursor = cursor.to_game_start()
print(f"At start: {start_cursor.is_line_start}")

# Step forward move by move:
c1 = start_cursor.next()
print(f"Move 1: {c1.previous_move_san}")

c2 = c1.next()
print(f"Move 2: {c2.previous_move_san}")
```

---

## 6. Exporting to PGN

Once your game is populated, export it as standard Portable Game Notation:

```python
pgn_output = game.to_pgn()
print(pgn_output)
```

The resulting string includes formatted tag headers followed by properly numbered movetext:

```pgn
[Event "World Chess Championship"]
[Site "London, UK"]
[Date "2024.11.20"]
[Round "?"]
[White "Gukesh D"]
[Black "Ding Liren"]
[Result "*"]

1. e4 e5 2. Nf3 Nc6 *
```

---

## 7. Next Steps

Now that you have grasped game initialisation and cursor navigation:

- Build an interactive terminal viewer in [Building a CLI Game Inspector](game-inspector.md).
- Learn how to branch into alternative lines in [How-To: Navigate and Edit Move Trees](../how-to/navigate-and-edit-trees.md).
- Understand why cursors are immutable in [Explanation: Immutable Cursor Mental Model](../explanation/cursor-immutability.md).
