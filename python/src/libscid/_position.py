from __future__ import annotations

import ctypes
from typing import Literal

from ._move_metadata import MoveMetadata
from ._native import NativeLibrary, load_library


class Position:
    _native: NativeLibrary
    _handle: ctypes.c_void_p

    def __init__(self):
        raise TypeError("Position objects are returned by libscid APIs")

    @classmethod
    def from_fen(cls, fen: str | bytes) -> Position:
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
        return self._native.position_to_fen(self._handle)

    @property
    def side_to_move(self) -> Literal["white", "black"]:
        return self._native.position_side_to_move(self._handle)

    @property
    def fullmove_number(self) -> int:
        return self._native.position_fullmove_number(self._handle)

    @property
    def halfmove_clock(self) -> int:
        return self._native.position_halfmove_clock(self._handle)

    @property
    def is_check(self) -> bool:
        return self._native.position_is_check(self._handle)

    @property
    def is_checkmate(self) -> bool:
        return self._native.position_is_checkmate(self._handle)

    @property
    def is_stalemate(self) -> bool:
        return not self.is_check and not self.legal_moves

    def get_piece_at(self, square: str | bytes) -> str | None:
        return self._native.position_piece_at(self._handle, square)

    @property
    def legal_moves(self) -> tuple[str, ...]:
        return self._native.position_legal_moves_uci(self._handle)

    def get_move_metadata(self, move: str | bytes) -> MoveMetadata:
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
        return self._native.position_to_san(self._handle, move)

    def apply_san(self, san: str | bytes) -> None:
        self._native.position_apply_san(self._handle, san)

    def apply_uci(self, uci: str | bytes) -> None:
        self._native.position_apply_uci(self._handle, uci)

    def _dispose(self) -> None:
        handle = getattr(self, "_handle", None)
        if handle:
            self._native.free_position(self._handle)
            self._handle = ctypes.c_void_p()

    def __del__(self) -> None:
        self._dispose()
