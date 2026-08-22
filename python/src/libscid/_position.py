"""Chess board position state representation and rule evaluation."""

from __future__ import annotations

import ctypes
from typing import Literal

from ._move_metadata import MoveMetadata
from ._native import NativeLibrary, load_library


class Position:
    """Represents a chess board state and rule validation context.

    Encapsulates piece placement across all 64 squares, active side to move,
    castling availability rights, en passant target square, halfmove clock
    (fifty-move rule), and fullmove counter. Provides methods to query legal
    moves, evaluate check, checkmate, and stalemate states, calculate move
    metadata flags, and execute moves in-place.

    Examples:
        >>> import libscid
        >>> fen = (
        ...     "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
        ... )
        >>> pos = libscid.Position.from_fen(fen)
        >>> pos.side_to_move
        'black'
        >>> pos.is_check
        False
        >>> pos.get_piece_at("e4")
        'P'
        >>> pos.apply_san("e5")
        >>> pos.side_to_move
        'white'
    """

    _native: NativeLibrary
    _handle: ctypes.c_void_p

    def __init__(self):
        """Disallows direct construction.

        Raises:
            TypeError: Position objects cannot be instantiated directly and
                must be obtained via factory classmethods or domain handles.
        """
        raise TypeError("Position objects are returned by libscid APIs")

    @classmethod
    def from_fen(cls, fen: str | bytes) -> Position:
        """Creates a new board position initialised from a FEN string.

        Args:
            fen: Forsyth–Edwards Notation (FEN) string or UTF-8 encoded bytes.

        Returns:
            A new `Position` instance initialised to the board state specified by `fen`.

        Raises:
            LibScidError: If `fen` is malformed or describes an illegal board setup.

        Examples:
            >>> import libscid
            >>> pos = libscid.Position.from_fen(
            ...     "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
            ... )
            >>> pos.side_to_move
            'black'
            >>> pos.get_piece_at("e4")
            'P'
        """
        native = load_library()
        return cls._from_handle(native, native.create_position_from_fen(fen))

    @classmethod
    def _from_handle(cls, native: NativeLibrary, handle: ctypes.c_void_p) -> Position:
        position = cls.__new__(cls)
        position._native = native
        position._handle = handle
        return position

    @property
    def fen(self) -> str:
        """Full 6-field Forsyth–Edwards Notation (FEN) string of the board state.

        Returns:
            The complete FEN string including piece placement, side to move,
                castling availability, en passant target square, halfmove clock,
                and fullmove number.

        Examples:
            >>> import libscid
            >>> pos = libscid.Position.from_fen(
            ...     "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
            ... )
            >>> pos.fen
            'rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1'
        """
        return self._native.position_to_fen(self._handle)

    @property
    def side_to_move(self) -> Literal["white", "black"]:
        """The active player colour whose turn it is to move.

        Returns:
            `"white"` if White is to move, or `"black"` if Black is to move.

        Examples:
            >>> import libscid
            >>> pos = libscid.Position.from_fen(
            ...     "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
            ... )
            >>> pos.side_to_move
            'white'
        """
        return self._native.position_side_to_move(self._handle)

    @property
    def fullmove_number(self) -> int:
        """The 1-based fullmove counter.

        Starts at 1 and increments after each move made by Black.

        Returns:
            The positive integer fullmove number.

        Examples:
            >>> import libscid
            >>> pos = libscid.Position.from_fen(
            ...     "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
            ... )
            >>> pos.fullmove_number
            1
        """
        return self._native.position_fullmove_number(self._handle)

    @property
    def halfmove_clock(self) -> int:
        """The halfmove clock (ply count) for the fifty-move draw rule.

        Counts the number of halfmoves since the last pawn advance or piece
        capture.

        Returns:
            The non-negative integer halfmove clock.

        Examples:
            >>> import libscid
            >>> pos = libscid.Position.from_fen(
            ...     "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1"
            ... )
            >>> pos.halfmove_clock
            1
        """
        return self._native.position_halfmove_clock(self._handle)

    @property
    def is_check(self) -> bool:
        """Whether the king of the player to move is currently in check.

        Returns:
            `True` if the active player is in check; `False` otherwise.

        Examples:
            >>> import libscid
            >>> pos = libscid.Position.from_fen("4k3/8/8/8/8/8/4R3/4K3 b - - 0 1")
            >>> pos.is_check
            True
        """
        return self._native.position_is_check(self._handle)

    @property
    def is_checkmate(self) -> bool:
        """Whether the position is in checkmate.

        A position is checkmate when the active player's king is in check
        and has no legal moves.

        Returns:
            `True` if the active player is checkmated; `False` otherwise.

        Examples:
            >>> import libscid
            >>> pos = libscid.Position.from_fen("R3k3/8/4K3/8/8/8/8/8 b - - 0 1")
            >>> pos.is_checkmate
            True
        """
        return self._native.position_is_checkmate(self._handle)

    @property
    def is_stalemate(self) -> bool:
        """Whether the position is in stalemate.

        A position is stalemate when the active player is not in check
        and has no legal moves available.

        Returns:
            `True` if the position is stalemated; `False` otherwise.

        Examples:
            >>> import libscid
            >>> pos = libscid.Position.from_fen("k7/2Q5/1K6/8/8/8/8/8 b - - 0 1")
            >>> pos.is_stalemate
            True
        """
        return not self.is_check and not self.legal_moves

    def get_piece_at(self, square: str | bytes) -> str | None:
        """Retrieves the piece residing on a specified board square.

        Args:
            square: Algebraic coordinate string or UTF-8 bytes (e.g. `"e4"`,
                `"a1"`, `b"h8"`).

        Returns:
            Single-character piece letter (`"K"`, `"Q"`, `"R"`, `"B"`, `"N"`, `"P"`
                for White; `"k"`, `"q"`, `"r"`, `"b"`, `"n"`, `"p"` for Black), or
                `None` if the square is empty.

        Raises:
            LibScidError: If `square` is not a valid coordinate string.

        Examples:
            >>> import libscid
            >>> pos = libscid.Position.from_fen(
            ...     "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
            ... )
            >>> pos.get_piece_at("e1")
            'K'
            >>> pos.get_piece_at("e8")
            'k'
            >>> pos.get_piece_at("e4") is None
            True
        """
        return self._native.position_piece_at(self._handle, square)

    @property
    def legal_moves(self) -> tuple[str, ...]:
        """All strictly legal moves available in the current position.

        Returns:
            Tuple of legal moves formatted as coordinate UCI strings
                (e.g. `("e2e4", "g1f3")`).

        Examples:
            >>> import libscid
            >>> pos = libscid.Position.from_fen(
            ...     "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
            ... )
            >>> len(pos.legal_moves)
            20
            >>> "e2e4" in pos.legal_moves
            True
        """
        return self._native.position_legal_moves_uci(self._handle)

    def get_move_metadata(self, move: str | bytes) -> MoveMetadata:
        """Calculates structural and rule characteristics for a move.

        Evaluates check, checkmate, castling, and pawn promotion properties
        for the given move text against the current position.

        Args:
            move: Standard Algebraic Notation (SAN) or coordinate UCI move string
                or bytes (e.g. `"Nf3"`, `"e2e4"`, `"b7b8q"`).

        Returns:
            A `MoveMetadata` bitmask flag containing applicable attributes.

        Raises:
            LibScidError: If `move` is illegal or ambiguous in the current position.

        Examples:
            >>> import libscid
            >>> pos = libscid.Position.from_fen(
            ...     "r1bqkb1r/pppp1ppp/2n5/4p3/2B1n3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 5"
            ... )
            >>> flags = pos.get_move_metadata("O-O")
            >>> bool(flags & libscid.MoveMetadata.CASTLING)
            True
            >>> mate_pos = libscid.Position.from_fen(
            ...     "r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR"
            ...     " w KQkq - 4 4"
            ... )
            >>> mate_flags = mate_pos.get_move_metadata("Qxf7#")
            >>> bool(mate_flags & libscid.MoveMetadata.CHECKMATE)
            True
        """
        movespec, san = self._native.position_move_metadata(self._handle, move)
        metadata = MoveMetadata.NONE
        if san.endswith("+") or san.endswith("#"):
            metadata |= MoveMetadata.CHECK
        if san.endswith("#"):
            metadata |= MoveMetadata.CHECKMATE
        if movespec.is_castling:
            metadata |= MoveMetadata.CASTLING
        if movespec.promotion != 0:
            metadata |= MoveMetadata.PROMOTION
        return metadata

    def to_san(self, move: str | bytes) -> str:
        """Converts and normalises a move string to canonical SAN against this position.

        Does not mutate the board state. Accepts standard SAN, permissive move
        notations, or coordinate UCI strings.

        Args:
            move: Move notation string or bytes to normalise (e.g. `"e2e4"`,
                `"OO"`, `"b8Q"`).

        Returns:
            Canonical Standard Algebraic Notation string (e.g. `"e4"`, `"O-O"`,
                `"b8=Q+"`).

        Raises:
            LibScidError: If `move` is illegal or ambiguous in the current position.

        Examples:
            >>> import libscid
            >>> pos = libscid.Position.from_fen(
            ...     "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
            ... )
            >>> pos.to_san("e2e4")
            'e4'
            >>> pos.to_san("g1f3")
            'Nf3'
        """
        return self._native.position_to_san(self._handle, move)

    def apply_san(self, san: str | bytes) -> None:
        """Applies a Standard Algebraic Notation (SAN) move in-place to this position.

        Updates piece positions, active colour, castling rights, en passant
        state, halfmove clock, and fullmove number.

        Args:
            san: Standard Algebraic Notation move string or bytes (e.g. `"e4"`,
                `"Nf3"`, `"O-O"`).

        Raises:
            LibScidError: If `san` is illegal or ambiguous in the current position.

        Examples:
            >>> import libscid
            >>> pos = libscid.Position.from_fen(
            ...     "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
            ... )
            >>> pos.apply_san("e4")
            >>> pos.get_piece_at("e4")
            'P'
            >>> pos.side_to_move
            'black'
        """
        self._native.position_apply_san(self._handle, san)

    def apply_uci(self, uci: str | bytes) -> None:
        """Applies a coordinate UCI move in-place to this position.

        Updates piece positions, active colour, castling rights, en passant
        state, halfmove clock, and fullmove number.

        Args:
            uci: Coordinate UCI move string or bytes (e.g. `"e2e4"`, `"a7a8q"`).

        Raises:
            LibScidError: If `uci` is illegal in the current position.

        Examples:
            >>> import libscid
            >>> pos = libscid.Position.from_fen(
            ...     "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
            ... )
            >>> pos.apply_uci("e2e4")
            >>> pos.get_piece_at("e4")
            'P'
            >>> pos.side_to_move
            'black'
        """
        self._native.position_apply_uci(self._handle, uci)

    def _dispose(self) -> None:
        handle = getattr(self, "_handle", None)
        if handle:
            self._native.free_position(self._handle)
            self._handle = ctypes.c_void_p()

    def __del__(self) -> None:
        self._dispose()
