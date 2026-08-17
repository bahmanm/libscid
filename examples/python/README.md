# libscid Python Examples

This directory contains examples demonstrating idiomatic usage of the high-level `libscid` Python package.

- `010-edit-pgn/`: parse PGN text with `Game.from_pgn()`, inspect and mutate header tags, and export PGN using `PgnOptions`.
- `020-author-pgn/`: author a game from scratch using `Game()`, `append_move()`, `add_variation()`, comments, and `Nag`.
- `030-navigate-pgn/`: traverse game variation trees with `Cursor`, iterate movetext events, and query positions.
- `040-position-and-arbiter/`: parse FEN with `Position`, query legal moves/checks/mates, apply moves, and query rule claims with `cursor.arbiter`.
- `050-list-database/`: open PGN databases with `Database.open_pgn_read_only()`, inspect metadata, and paginate filtered games.
- `060-search-database/`: execute header searches (`HeaderCriteria`), position searches, and pawn structure searches (`search.board()`).

## Running Examples

To run an example using `uv`:

```sh
uv run python examples/python/010-edit-pgn/main.py
```
