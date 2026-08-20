# 020-author-pgn

This example demonstrates how to author a chess game from scratch using `Game()`, `append_move()`, `add_variation()`, comments, and NAG symbols.

## Code Walkthrough

1. `Game()` initializes a new blank game starting from the standard chess position.
2. `cursor.append_move("e4")` appends moves sequentially along the mainline.
3. `cursor.set_comment(...)` and `cursor.add_nag(Nag("!"))` annotate moves.
4. `cursor.to_main_line_offset(3)` navigates back to move 3 to attach a variation branch with `add_variation()`.

## Running the Example

```sh
uv run python examples/python/020-author-pgn/main.py
```
