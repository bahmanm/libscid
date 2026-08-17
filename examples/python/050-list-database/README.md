# 050-list-database

This example demonstrates how to open a PGN database using `Database.open_pgn_read_only()`, inspect game metadata without loading entire game objects, and paginate games via `Filter`.

## Code Walkthrough

1. `Database.open_pgn_read_only(PGN_PATH)` opens a PGN file as a read-only database.
2. `db.get_tag(i, "White")` reads specific game header fields directly from the database index.
3. `db.filters.all_games.get_game_indices(sort_criteria="N+", start_row=0, row_count=5)` fetches paginated game indices.

## Running the Example

```sh
uv run python examples/python/050-list-database/main.py
```
