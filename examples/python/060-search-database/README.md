# 060-search-database

This example demonstrates how to execute header searches, position searches, and pawn structure board searches using `Database.search`.

## Code Walkthrough

1. `HeaderCriteria(white="Lasker, Emanuel", has_comments=True)` sets filtering rules for header searches via `db.search.headers(criteria)`.
2. `db.search.position(ruy_pos)` searches for games containing a specific `Position`.
3. `db.search.board(ruy_pos, match="pawns")` searches for games with matching pawn structure criteria.
4. `filter.delete()` releases dynamic database filter handles after query processing.

## Running the Example

```sh
uv run python examples/python/060-search-database/main.py
```
