#!/usr/bin/env python3
"""Example 010: Parse, inspect, edit tags, and export PGN using libscid."""

import libscid

SAMPLE_PGN = """[Event "World Championship 34th"]
[Site "London ENG"]
[Date "1986.08.11"]
[Round "7"]
[White "Kasparov, Garry"]
[Black "Karpov, Anatoly"]
[Result "1-0"]

1. e4 e5 2. Nf3 Nc6 3. Bb5 a6 4. Ba4 Nf6 1-0
"""


def main() -> None:
    game = libscid.Game.from_pgn(SAMPLE_PGN)

    print("Initial PGN Tags:")
    print(f"  White:  {game.get_tag('White')}")
    print(f"  Black:  {game.get_tag('Black')}")
    print(f"  Event:  {game.get_tag('Event')}")
    print(f"  Result: {game.get_tag('Result')}")

    # Mutate header tags
    game.set_tag("Annotator", "Grandmaster Analysis")
    game.set_tag("ECO", "C84")

    print("\nUpdated Tags:")
    for key, value in game.get_tags():
        print(f"  {key}: {value}")

    # Export with custom PgnOptions
    options = libscid.PgnOptions(symbolic_nags=True, line_width=60)
    exported_pgn = game.to_pgn(options)

    print("\nExported PGN:")
    print(exported_pgn)


if __name__ == "__main__":
    main()
