# Tournament Arbiter Adjudication

The [`Arbiter`][libscid.Arbiter] component provides evaluation for standard FIDE tournament draw claims along game lines.

---

## 1. Overview

In tournament chess, players can claim draws under specific technical conditions:
1. **Fifty-Move Rule**: No pawn moves or piece captures have occurred in the last 50 full moves (100 plies).
2. **Threefold Repetition**: The exact same board position (identical piece arrangement, active side to move, castling availability, and en passant target square) has occurred at least three times along the current line of play.

---

## 2. Using the Arbiter

The `Arbiter` is instantiated for a [`Game`][libscid.Game] or accessed via property on [`Game`][libscid.Game].

```python
import libscid

# Game repeating positions
pgn = "1. Nf3 Nf6 2. Ng1 Ng8 3. Nf3 Nf6 4. Ng1 Ng8 *"
game = libscid.Game.from_pgn(pgn)

arbiter = game.arbiter
cursor = game.create_cursor().to_game_end()

# Check draw conditions at the current cursor position:
print(f"Can claim threefold repetition: {arbiter.can_claim_threefold_repetition(cursor)}")
print(f"Can claim fifty-move rule: {arbiter.can_claim_fifty_move_rule(cursor)}")
```

---

## 3. Position Clock Inspection

The underlying [`Position`][libscid.Position] also exposes halfmove clocks and move numbers:

```python
import libscid

game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 *")
cursor = game.create_cursor().to_game_end()

pos = cursor.position
print(f"Fullmove number: {pos.fullmove_number}")  # 2
print(f"Halfmove clock (plies since capture/pawn move): {pos.halfmove_clock}")  # 2
```
