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
    def __init__(self, cursor: Cursor):
        self._cursor = cursor

    @property
    def can_claim_fifty_move_rule(self) -> bool:
        return self._cursor.position.halfmove_clock >= 100

    @property
    def can_claim_threefold_repetition(self) -> bool:
        position_ids = self._path_position_ids()
        return position_ids.count(position_ids[-1]) >= 3

    def _path_position_ids(self) -> list[_PositionId]:
        position = self._cursor._game.start_position
        position_ids = [_PositionId.from_fen(position.fen)]
        for move in self._cursor._path_move_uci():
            position.apply_uci(move)
            position_ids.append(_PositionId.from_fen(position.fen))
        return position_ids
