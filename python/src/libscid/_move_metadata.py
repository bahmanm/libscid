"""Move metadata flag enumeration for chess moves."""

from __future__ import annotations

from enum import Flag, auto


class MoveMetadata(Flag):
    """Bitwise flag enumeration representing characteristics of a chess move.

    Classifies structural, tactical, and rule-based properties of a move such
    as check, checkmate, castling, and pawn promotion. Flags may be combined
    using bitwise operators (`|`, `&`, `^`, `~`).

    Attributes:
        NONE: No special move characteristics present.
        CHECK: Move delivers a check to the opponent's king (appends `+`).
        CHECKMATE: Move delivers checkmate, concluding the game (appends `#`).
        CASTLING: Move is a kingside (`O-O`) or queenside (`O-O-O`) castling move.
        PROMOTION: Move is a pawn promotion to Queen, Rook, Bishop, or Knight.

    Examples:
        >>> import libscid
        >>> pos = libscid.Position.from_fen("4k3/1P6/8/8/8/8/8/4K3 w - - 0 1")
        >>> meta = pos.get_move_metadata("b7b8q")
        >>> libscid.MoveMetadata.PROMOTION in meta
        True
        >>> bool(meta & libscid.MoveMetadata.CHECK)
        True
    """

    NONE = 0
    CHECK = auto()
    CHECKMATE = auto()
    CASTLING = auto()
    PROMOTION = auto()
