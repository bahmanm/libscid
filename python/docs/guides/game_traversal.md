# Game Traversal & Move Tree Navigation

`libscid` models chess games as directed tree structures rooted at a starting board configuration (the standard chess starting position or a custom FEN setup).

---

## 1. Core Abstractions

1. **[`Game`][libscid.Game]**: The primary container representing a full chess game, including PGN header tags, the move tree, and tree metadata.
2. **[`Cursor`][libscid.Cursor]**: An immutable navigational pointer to a specific node (position) in the game tree.
3. **[`Position`][libscid.Position]**: The board state at the cursor's location, exposing FEN generation, piece placement, check status, and legal moves.

---

## 2. Immutable Cursor Semantics

Navigating the move tree produces new `Cursor` instances without mutating existing pointers. This design guarantees thread safety and predictability.

```python
import libscid

game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 3. Bb5 *")

# Create a cursor pointing to the game start
c0 = game.create_cursor()
assert c0.is_line_start
assert c0.position.fen == "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

# Step forward along the mainline
c1 = c0.next()
assert c1.previous_move_san == "e4"

c2 = c1.next()
assert c2.previous_move_san == "e5"

# Jump directly to start or end
c_start = c2.to_game_start()
c_end = c2.to_game_end()
assert c_end.is_line_end
```

---

## 3. Managing Variations (Sub-lines)

Chess games often branch into alternative lines of play. `libscid` fully supports arbitrary nested variations.

### 3.1. Detecting & Entering Variations

```python
import libscid

# Game with a mainline (1. e4 e5) and a variation (1... c5)
game = libscid.Game.from_pgn("1. e4 e5 (1... c5 2. Nf3) 2. Nf3 *")

cursor = game.create_cursor().next()  # After 1. e4

# Inspect variations branching at this point
print(f"Number of variations: {cursor.variation_count}")  # 1

# Enter the variation (1... c5)
var_cursor = cursor.enter_variation(0)
print(f"Variation first move: {var_cursor.previous_move_san}")  # c5
print(f"Variation depth: {var_cursor.variation_depth}")        # 1
print(f"Is variation line: {var_cursor.is_variation_line}")    # True
```

### 3.2. Adding Variations Programmatically

```python
import libscid

game = libscid.Game()
cursor = game.create_cursor()

# Append mainline: 1. e4 e5
cursor = cursor.append_move("e4")
cursor = cursor.append_move("e5")

# Rewind to after 1. e4 and add an alternative line: 1... c5 2. Nf3
c_after_e4 = game.create_cursor().next()
var_c = c_after_e4.add_variation("c5")
var_c = var_c.append_move("Nf3")

print(game.to_pgn())
```

---

## 4. Move Annotations & Comments

### 4.1. Numeric Annotation Glyphs (NAGs)

NAGs convey standard chess assessments (e.g. `$1` for `!`, `$2` for `?`, `$3` for `!!`, `$4` for `??`):

```python
import libscid

game = libscid.Game()
cursor = game.create_cursor()

cursor = cursor.append_move("e4")
cursor.add_nag(libscid.Nag.GOOD_MOVE)  # $1 (!)

print(f"Move NAGs: {cursor.previous_move_nags}")
print(f"NAG Symbol: {libscid.Nag(1).symbol}")  # '!'
```

### 4.2. Preceding and Trailing Comments

```python
import libscid

game = libscid.Game()
cursor = game.create_cursor()

cursor = cursor.append_move("e4")
cursor.comment = "The king's pawn opening."
cursor.preceding_comment = "Starting with open game principles."

print(game.to_pgn())
```
