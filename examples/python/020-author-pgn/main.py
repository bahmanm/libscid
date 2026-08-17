#!/usr/bin/env python3
"""Example 020: Author a chess game with moves, comments, NAGs, and variations."""

import libscid


def main() -> None:
    # 1. Create a blank game
    game = libscid.Game()
    game.set_tag("Event", "Casual Blitz")
    game.set_tag("White", "Player 1")
    game.set_tag("Black", "Player 2")

    # 2. Append mainline moves
    cursor = game.create_cursor()
    c1 = cursor.append_move("e4")
    c2 = c1.append_move("e5")
    c3 = c2.append_move("Nf3")
    c4 = c3.append_move("Nc6")
    c5 = c4.append_move("Bb5")  # Ruy Lopez

    # 3. Add commentary and NAG annotation on Bb5
    c5.set_comment("The Ruy Lopez opening.")
    c5.add_nag(libscid.Nag("!"))

    # 4. Add a variation on Black's move 3 (3... Nf6 - Berlin Defense)
    # We navigate back to move 3 (after 3. Nf3)
    c3_pos = cursor.to_main_line_offset(3)
    if c3_pos is not None:
        var_cursor = c3_pos.add_variation(preceding_comment="Berlin Defense variation")
        if var_cursor is not None:
            v1 = var_cursor.append_move("Nf6")
            v1.set_comment("Solid defensive choice.")

    # 5. Export and print the authored PGN
    print("Authored Game PGN:")
    print(game.to_pgn())


if __name__ == "__main__":
    main()
