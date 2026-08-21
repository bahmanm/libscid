"""Chess arbiter evaluation for draw claims and rule verifications."""

from __future__ import annotations

from dataclasses import dataclass
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ._cursor import Cursor


@dataclass(frozen=True)
class _PositionId:
    board: str
    side_to_move: str
    castling: str
    en_passant: str

    @classmethod
    def from_fen(cls, fen: str) -> _PositionId:
        board, side_to_move, castling, en_passant, *_ = fen.split()
        return cls(board, side_to_move, castling, en_passant)


class Arbiter:
    """Evaluates tournament rules and draw claim conditions for a cursor position.

    An `Arbiter` instance is typically accessed via the
    [`Cursor.arbiter`][libscid.Cursor.arbiter] property. It evaluates claimable
    draw conditions defined by the FIDE Laws of Chess at the cursor's current
    position, including the fifty-move rule and threefold repetition.

    Examples:
        >>> import libscid
        >>> pgn = "1. Nf3 Nf6 2. Ng1 Ng8 3. Nf3 Nf6 4. Ng1 Ng8 *"
        >>> cursor = libscid.Game.from_pgn(pgn).create_cursor().to_game_end()
        >>> cursor.arbiter.can_claim_threefold_repetition
        True
        >>> cursor.arbiter.can_claim_fifty_move_rule
        False
    """

    def __init__(self, cursor: Cursor):
        """Initialise an arbiter bound to a specific cursor position.

        Args:
            cursor: The cursor navigating the game tree to inspect.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 e5 *")
            >>> cursor = game.create_cursor().to_game_end()
            >>> arbiter = cursor.arbiter
            >>> arbiter.can_claim_fifty_move_rule
            False
        """
        self._cursor = cursor

    @property
    def can_claim_fifty_move_rule(self) -> bool:
        """Check whether a draw can be claimed under the fifty-move rule.

        According to the FIDE Laws of Chess, a player may claim a draw if the
        last 50 consecutive full moves (100 halfmoves / ply) have been completed
        by each player without any piece capture and without any pawn advance.

        Returns:
            True if the halfmove clock is at least 100; otherwise False.

        Examples:
            >>> import libscid
            >>> pos = libscid.Position.from_fen(
            ...     "8/8/8/8/8/4k3/8/4K3 w - - 100 51"
            ... )
            >>> game = libscid.Game(position=pos)
            >>> game.create_cursor().arbiter.can_claim_fifty_move_rule
            True
            >>> normal_game = libscid.Game.from_pgn("1. e4 e5 *")
            >>> normal_game.create_cursor().arbiter.can_claim_fifty_move_rule
            False
        """
        return self._cursor.position.halfmove_clock >= 100

    @property
    def can_claim_threefold_repetition(self) -> bool:
        """Check whether a draw can be claimed under the threefold repetition rule.

        According to the FIDE Laws of Chess, a player may claim a draw if the
        exact same board position has occurred at least three times along the
        path from the game start to this cursor node. Two positions are identical
        if the side to move, piece placements, castling rights, and en passant
        target squares are identical.

        Returns:
            True if the current board state has occurred 3 or more times along
            the line of play; otherwise False.

        Examples:
            >>> import libscid
            >>> pgn = "1. Nf3 Nf6 2. Ng1 Ng8 3. Nf3 Nf6 4. Ng1 Ng8 *"
            >>> cursor = libscid.Game.from_pgn(pgn).create_cursor().to_game_end()
            >>> cursor.arbiter.can_claim_threefold_repetition
            True
            >>> non_rep = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 *")
            >>> non_rep.create_cursor().arbiter.can_claim_threefold_repetition
            False
        """
        position_ids = self._path_position_ids()
        return position_ids.count(position_ids[-1]) >= 3

    def _path_position_ids(self) -> list[_PositionId]:
        position = self._cursor._game.start_position
        position_ids = [_PositionId.from_fen(position.fen)]
        for move in self._cursor._path_move_uci():
            position.apply_uci(move)
            position_ids.append(_PositionId.from_fen(position.fen))
        return position_ids
