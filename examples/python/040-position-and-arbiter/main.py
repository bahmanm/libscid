#!/usr/bin/env python3
"""Example 040: Position inspection, move application, and rule claims via Arbiter."""

import libscid

# FEN representing a position with check
CHECK_FEN = "rnbqkbnr/ppp2ppp/8/3pp3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq d6 0 3"


def main() -> None:
    # 1. Parse FEN into Position
    pos = libscid.Position.from_fen(CHECK_FEN)

    print(f"Side to move:     {pos.side_to_move}")
    print(f"Fullmove number:  {pos.fullmove_number}")
    print(f"Halfmove clock:   {pos.halfmove_clock}")
    print(f"Piece at e4:      {pos.get_piece_at('e4')}")
    print(f"Legal moves count:{len(pos.legal_moves)}")
    print(f"Is in check:      {pos.is_check}")

    # 2. Convert move to SAN and inspect metadata
    move = "f3e5"  # UCI: Nxe5
    san = pos.to_san(move)
    metadata = pos.get_move_metadata(move)
    print(f"Move '{move}' -> SAN='{san}', Metadata={metadata}")

    # 3. Arbitral Rule Claims Demonstration
    # Create a game and apply moves to trigger rule check conditions
    game = libscid.Game()
    cursor = game.create_cursor()

    # Move sequence to repeat position (threefold repetition)
    moves = ["Nf3", "Nf6", "Ng1", "Ng8", "Nf3", "Nf6", "Ng1", "Ng8"]
    for m in moves:
        cursor = cursor.append_move(m)

    arbiter = cursor.arbiter
    print(f"\nThreefold repetition claimable: {arbiter.can_claim_threefold_repetition}")
    print(f"50-move rule claimable:          {arbiter.can_claim_fifty_move_rule}")


if __name__ == "__main__":
    main()
