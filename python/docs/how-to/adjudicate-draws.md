# How to Adjudicate Tournament Draws

This guide demonstrates how to evaluate FIDE tournament draw claims (the fifty-move rule and threefold repetition) along any game line using [`Arbiter`][libscid.Arbiter].

---

## 1. Overview of FIDE Tournament Rules

Under World Chess Federation (FIDE) Laws of Chess, a game is drawn upon a correct claim by the player having the move under two specific conditions:

- Fifty-Move Rule (FIDE Article 9.3): The last 50 consecutive moves (100 plies) have been completed by each player without the movement of any pawn and without any capture.
- Threefold Repetition (FIDE Article 9.2): The exact same board position has arisen (or is about to arise) at least three times. Identical positions require the same active side to move, identical piece placement, identical castling rights, and identical en passant capture possibilities.

---

## 2. Checking Claims with the Arbiter

The [`Arbiter`][libscid.Arbiter] component provides evaluation for draw claims. Access the arbiter through the [`Cursor.arbiter`][libscid.Cursor.arbiter] property or by passing a cursor to [`Arbiter(cursor)`][libscid.Arbiter]:

```python
import libscid

# Game repeating positions:
pgn = """
1. Nf3 Nf6 2. Ng1 Ng8 3. Nf3 Nf6 4. Ng1 Ng8 *
"""
game = libscid.Game.from_pgn(pgn)

# Create cursor at final position:
cursor = game.create_cursor().to_game_end()

# Check draw conditions via properties:
can_repeat = cursor.arbiter.can_claim_threefold_repetition
can_fifty = cursor.arbiter.can_claim_fifty_move_rule

print(f"Can claim threefold repetition: {can_repeat}")  # True
print(f"Can claim fifty-move rule:      {can_fifty}")   # False
```

---

## 3. Inspecting the Halfmove Clock

For fifty-move rule tracking, inspect [`halfmove_clock`][libscid.Position.halfmove_clock] directly on [`Position`][libscid.Position]:

```python
import libscid

game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 3. Bb5 a6 *")
cursor = game.create_cursor().to_game_end()

pos = cursor.position
print(f"Halfmove clock (plies): {pos.halfmove_clock}")  # 0 (after pawn push a6)
print(f"Fullmove number:        {pos.fullmove_number}")  # 3
```

When `pos.halfmove_clock >= 100`, 50 full moves have elapsed without a pawn advance or capture, making a draw claim valid.

---

## 4. Evaluating Draw Claims in Playback Loops

In chess engines, tournament arbiters, or game validator scripts, check draw claims at every step:

```python
import libscid


def monitor_game_status(game: libscid.Game) -> None:
    cursor = game.create_cursor()

    while not cursor.is_line_end:
        cursor = cursor.next()

        if cursor.arbiter.can_claim_threefold_repetition:
            print(
                f"Draw claim available at ply {cursor.position.fullmove_number} "
                f"({cursor.position.side_to_move}): Threefold Repetition"
            )

        if cursor.arbiter.can_claim_fifty_move_rule:
            print(
                f"Draw claim available at ply {cursor.position.fullmove_number} "
                f"({cursor.position.side_to_move}): Fifty-Move Rule"
            )
```

---

## 5. Related Topics

- For a complete guided tutorial, see [Building a CLI Game Inspector](../tutorials/game-inspector.md).
- To inspect board evaluation flags such as check and stalemate, see [How to Navigate and Edit Move Trees](navigate-and-edit-trees.md).
- For complete method signatures, inspect the [Core API Reference](../reference/core.md).
