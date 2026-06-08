from __future__ import annotations

import argparse
from pathlib import Path

from libscid import LibScid


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--library",
        help="Path to libscid.dylib, libscid.so, or scid.dll. "
        "Defaults to LIBSCID_LIBRARY or a local build tree.",
    )
    parser.add_argument(
        "--pgn",
        default=Path(__file__).resolve().parents[2] / "fixtures" / "import.pgn",
        type=Path,
        help="PGN file to read.",
    )
    args = parser.parse_args()

    libscid = LibScid(args.library)
    pgn = args.pgn.read_text(encoding="utf-8")

    with libscid.game_from_pgn(pgn) as game:
        print(f"library: {libscid.library_path}")
        print(f"white: {game.tag('White')}")
        print(f"black: {game.tag('Black')}")
        print(f"result: {game.tag('Result')}")

        game.set_tag("Annotator", "Python ctypes example")
        print()
        print(game.to_pgn())

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
