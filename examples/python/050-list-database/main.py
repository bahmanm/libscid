#!/usr/bin/env python3
"""Example 050: Open PGN databases, inspect metadata, and paginate filter games."""

from pathlib import Path

import libscid

FIXTURES_DIR = Path(__file__).resolve().parent.parent.parent / "fixtures"
PGN_PATH = FIXTURES_DIR / "import.pgn"


def main() -> None:
    if not PGN_PATH.exists():
        raise FileNotFoundError(f"Fixture PGN not found at {PGN_PATH}")

    # 1. Open read-only PGN database
    db = libscid.Database.open_pgn_read_only(PGN_PATH)
    print(f"Database Type: {db.type}")
    print(f"Read Only:     {db.read_only}")
    print(f"Total Games:   {db.game_count}")

    # 2. Inspect lightweight game metadata without loading full game trees
    print("\nGame Metadata (First 3 games):")
    for i in range(min(3, db.game_count)):
        white = db.get_tag(i, "White")
        black = db.get_tag(i, "Black")
        result = db.get_tag(i, "Result")
        print(f"  Game {i + 1}: {white} vs {black} [{result}]")

    # 3. Paginate game indices via all_games filter
    all_filter = db.filters.all_games
    indices = all_filter.get_game_indices(sort_criteria="N+", start_row=0, row_count=5)
    print(f"\nFiltered Indices (First 5): {indices}")

    # Close database connection
    db.close()


if __name__ == "__main__":
    main()
