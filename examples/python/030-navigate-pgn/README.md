# 030-navigate-pgn

This example demonstrates how to traverse game variation trees using `Cursor` and iterate movetext events with `iter_movetext()`.

## Code Walkthrough

1. `game.iter_movetext(variations=True)` streams structured movetext events (move tokens, variation start/end markers, comments).
2. `game.create_cursor()` creates an immutable cursor focused on the start of the game.
3. `cursor.next()` advances to the next move.
4. `cursor.enter_variation(0)` enters the specified variation branch at a move node.

## Running the Example

```sh
uv run python examples/python/030-navigate-pgn/main.py
```
