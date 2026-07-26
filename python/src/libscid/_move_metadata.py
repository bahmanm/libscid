from __future__ import annotations

from enum import Flag, auto


class MoveMetadata(Flag):
    NONE = 0
    CHECK = auto()
    CHECKMATE = auto()
    CASTLING = auto()
    PROMOTION = auto()
