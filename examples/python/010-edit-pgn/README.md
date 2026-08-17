# 010-edit-pgn

This example demonstrates how to parse a PGN string into a `Game` object, inspect and mutate header tags, and export the resulting game with custom `PgnOptions`.

## Code Walkthrough

1. `Game.from_pgn(SAMPLE_PGN)` parses the input string into a new game instance.
2. `game.get_tag("White")` and `game.set_tag("ECO", "C84")` interact with PGN metadata.
3. `PgnOptions` customises the exported text output (e.g. line wrapping, symbolic NAG formatting).
4. `game.to_pgn(options)` serialises the game back to a standard PGN string.

## Running the Example

```sh
uv run python examples/python/010-edit-pgn/main.py
```
