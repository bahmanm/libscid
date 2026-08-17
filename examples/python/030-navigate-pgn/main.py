#!/usr/bin/env python3
"""Example 030: Navigate game variation trees using Cursor
and MovetextEvent iteration.
"""

import libscid

ANNOTATED_PGN = """[Event "Demonstration"]
[White "Player A"]
[Black "Player B"]

1. e4 e5 2. Nf3 Nc6 (2... Nf6 3. Nxe5 d6) 3. Bb5 a6 1-0
"""


def main() -> None:
    game = libscid.Game.from_pgn(ANNOTATED_PGN)

    # 1. Iterate over all movetext events (moves, variations, comments)
    print("Movetext Event Stream:")
    for event in game.iter_movetext(variations=True):
        print(f"  {event}")

    # 2. Step-by-step navigation using Cursor
    print("\nManual Cursor Navigation:")
    cursor = game.create_cursor()

    c1 = cursor.next()  # move 1. e4
    c2 = c1.next() if c1 else None  # move 1... e5
    c3 = c2.next() if c2 else None  # move 2. Nf3

    if c3:
        print(f"Position at move 2. Nf3: SAN='{c3.previous_move_san}'")
        print(f"Variation count at 2. Nf3: {c3.variation_count}")

        # Enter the first variation (2... Nf6)
        var_cursor = c3.enter_variation(0)
        if var_cursor:
            print(f"Inside variation move 1: SAN='{var_cursor.next_move_san}'")
            var_c1 = var_cursor.next()
            if var_c1:
                print(f"Inside variation move 2: SAN='{var_c1.next_move_san}'")


if __name__ == "__main__":
    main()
