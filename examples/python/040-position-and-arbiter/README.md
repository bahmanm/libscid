# 040-position-and-arbiter

This example demonstrates board position analysis using `Position` and arbitral rule evaluation using `Cursor.arbiter`.

## Code Walkthrough

1. `Position.from_fen(CHECK_FEN)` parses FEN strings to inspect position metrics (side to move, check status, legal move list).
2. `pos.get_move_metadata("f3e5")` checks move attributes (e.g. check, checkmate, castling, promotion).
3. `cursor.arbiter` evaluates game state conditions to determine whether a player can claim a draw under the **50-move rule** or **threefold repetition**.

## Running the Example

```sh
uv run python examples/python/040-position-and-arbiter/main.py
```
