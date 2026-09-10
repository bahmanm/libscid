# How to Navigate and Edit Move Trees

This guide demonstrates how to traverse chess move trees, create and prune variations, and attach commentary or Numeric Annotation Glyphs (NAGs) using immutable cursors.

---

## 1. Navigating Mainline Moves

A [`Cursor`][libscid.Cursor] points to a specific node within a [`Game`][libscid.Game] tree. Navigation methods return new cursor instances:

```python
import libscid

game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 3. Bb5 a6 *")

# Initialise cursor at root:
cursor = game.create_cursor()
assert cursor.is_line_start

# Step forward along the mainline:
c1 = cursor.next()
print(f"Move 1: {c1.previous_move_san}")  # 'e4'

c2 = c1.next()
print(f"Move 2: {c2.previous_move_san}")  # 'e5'

# Step backward:
c_back = c2.previous()
assert c_back.previous_move_san == "e4"

# Fast-forward to the end of the line:
c_end = cursor.to_game_end()
assert c_end.is_line_end
```

---

## 2. Appending Moves Programmatically

To build a game programmatically, append moves in Standard Algebraic Notation (SAN):

```python
import libscid

game = libscid.Game()
cursor = game.create_cursor()

# Chain moves sequentially:
cursor = cursor.append_move("d4")
cursor = cursor.append_move("Nf6")
cursor = cursor.append_move("c4")
cursor = cursor.append_move("e6")

print(game.to_pgn())
```

---

## 3. Creating and Entering Variations

When alternative moves are explored, branch into sub-lines using [`add_variation`][libscid.Cursor.add_variation]:

```python
import libscid

game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 *")

# Rewind to after 1. e4:
c_after_e4 = game.create_cursor().next()

# Add Sicilian variation 1... c5 branching off after 1. e4:
var_cursor = c_after_e4.add_variation("c5")
var_cursor = var_cursor.append_move("Nf3")
var_cursor = var_cursor.append_move("d6")

# Inspect variation metadata:
print(f"Variation depth: {var_cursor.variation_depth}")      # 1
print(f"Is variation line: {var_cursor.is_variation_line}")  # True

# Return to parent mainline:
parent_cursor = var_cursor.exit_variation()
print(f"Parent move: {parent_cursor.previous_move_san}")     # 'e5'
```

---

## 4. Traversing Existing Variations

When inspecting a game containing branching lines, use [`variation_count`][libscid.Cursor.variation_count] and [`enter_variation`][libscid.Cursor.enter_variation]:

```python
import libscid

game = libscid.Game.from_pgn("1. e4 e5 (1... c5 2. Nf3) (1... e6 2. d4) 2. Nf3 *")
cursor = game.create_cursor().next()  # Position after 1. e4

print(f"Available variations: {cursor.variation_count}")  # 2

# Enter first variation (1... c5):
var0 = cursor.enter_variation(0)
print(f"Branch 0 first move: {var0.previous_move_san}")   # 'c5'

# Enter second variation (1... e6):
var1 = cursor.enter_variation(1)
print(f"Branch 1 first move: {var1.previous_move_san}")   # 'e6'
```

---

## 5. Attaching Commentary and NAG Annotations

Decorate moves with textual commentary or standard evaluation symbols:

```python
import libscid

game = libscid.Game()
cursor = game.create_cursor()

# Play move and attach comments:
cursor = cursor.append_move("e4")
cursor.comment = "The open game."
cursor.preceding_comment = "Round 1 opening preparation."

# Attach Numeric Annotation Glyphs (NAGs):
cursor.add_nag(libscid.Nag.GOOD_MOVE)  # $1 (!)

# Read back annotations:
print(f"Comment:           {cursor.comment}")
print(f"Preceding comment: {cursor.preceding_comment}")
print(f"Assigned NAGs:     {cursor.previous_move_nags}")

# Output PGN with annotations:
print(game.to_pgn())
```

---

## 6. Pruning and Promoting Variations

Adjust tree structures using variation manipulation methods:

```python
# Promote a variation to become the primary mainline line:
var_cursor.promote_variation()

# Delete a variation line completely:
c_after_e4.delete_variation(0)
```

---

## 7. Related Topics

- For event-driven traversal of variation trees without manual cursors, see [How to Stream Movetext Events](stream-movetext-events.md).
- To understand why navigation methods return new instances, read [Explanation: Immutable Cursor Mental Model](../explanation/cursor-immutability.md).
- For complete method signatures, inspect the [Core API Reference](../reference/core.md).
