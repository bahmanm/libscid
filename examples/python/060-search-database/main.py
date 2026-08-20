#!/usr/bin/env python3
"""Example 060: Database searching (header, position, and pawn structure search)."""

from pathlib import Path

import libscid

FIXTURES_DIR = Path(__file__).resolve().parent.parent.parent / "fixtures"
PGN_PATH = FIXTURES_DIR / "import.pgn"


def main() -> None:
    if not PGN_PATH.exists():
        raise FileNotFoundError(f"Fixture PGN not found at {PGN_PATH}")

    db = libscid.Database.open_pgn_read_only(PGN_PATH)

    # 1. Header Search using HeaderCriteria
    print("1. Running Header Search...")
    criteria = libscid.HeaderCriteria(
        white="Lasker, Emanuel",
        has_comments=True,
    )
    header_filter = db.search.headers(criteria)
    print(f"   Matches for Emanuel Lasker with comments: {header_filter.game_count}")

    # 2. Position Search
    print("\n2. Running Position Search...")
    ruy_lopez_fen = "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3"
    ruy_pos = libscid.Position.from_fen(ruy_lopez_fen)
    pos_filter = db.search.position(ruy_pos)
    print(f"   Matches for Ruy Lopez position: {pos_filter.game_count}")

    # 3. Board / Pawn Structure Search
    print("\n3. Running Pawn Structure Board Search...")
    board_filter = db.search.board(ruy_pos, match="pawns")
    print(f"   Matches with matching pawn structure: {board_filter.game_count}")

    # Clean up created filters
    header_filter.delete()
    pos_filter.delete()
    board_filter.delete()

    db.close()


if __name__ == "__main__":
    main()
