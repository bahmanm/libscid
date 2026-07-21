from __future__ import annotations

import ctypes
from typing import Literal

from ._base import NativeLibraryBase
from ._constants import SCID_BLACK, SCID_PIECE_SYMBOLS, SCID_WHITE
from ._text import encode


class NativePrimitiveMixin(NativeLibraryBase):
    def square_from_string(self, square: str | bytes) -> int:
        value = ctypes.c_uint()
        self._check(
            "scid_square_from_string",
            self._lib.scid_square_from_string(encode(square), ctypes.byref(value)),
        )
        return value.value

    def create_position_from_fen(self, fen: str | bytes) -> ctypes.c_void_p:
        position = ctypes.c_void_p()
        self._check(
            "scid_position_create_from_fen",
            self._lib.scid_position_create_from_fen(
                encode(fen), ctypes.byref(position)
            ),
        )
        return position

    def position_to_fen(self, position: ctypes.c_void_p) -> str:
        return self._string_result("scid_position_to_fen", position)

    def position_side_to_move(
        self, position: ctypes.c_void_p
    ) -> Literal["white", "black"]:
        side_to_move = ctypes.c_int()
        self._check(
            "scid_position_side_to_move_get",
            self._lib.scid_position_side_to_move_get(
                position, ctypes.byref(side_to_move)
            ),
        )
        if side_to_move.value == SCID_WHITE:
            return "white"
        if side_to_move.value == SCID_BLACK:
            return "black"
        raise ValueError(f"unexpected side-to-move value: {side_to_move.value}")

    def position_fullmove_number(self, position: ctypes.c_void_p) -> int:
        fullmove_number = ctypes.c_uint()
        self._check(
            "scid_position_fullmove_number_get",
            self._lib.scid_position_fullmove_number_get(
                position, ctypes.byref(fullmove_number)
            ),
        )
        return fullmove_number.value

    def position_halfmove_clock(self, position: ctypes.c_void_p) -> int:
        halfmove_clock = ctypes.c_uint()
        self._check(
            "scid_position_halfmove_clock_get",
            self._lib.scid_position_halfmove_clock_get(
                position, ctypes.byref(halfmove_clock)
            ),
        )
        return halfmove_clock.value

    def position_piece_at(
        self, position: ctypes.c_void_p, square: str | bytes
    ) -> str | None:
        square_value = self.square_from_string(square)
        piece = ctypes.c_uint()
        self._check(
            "scid_position_piece_at_get",
            self._lib.scid_position_piece_at_get(
                position, ctypes.c_uint(square_value), ctypes.byref(piece)
            ),
        )
        try:
            return SCID_PIECE_SYMBOLS[piece.value]
        except KeyError:
            raise ValueError(f"unexpected piece value: {piece.value}") from None

    def position_apply_san(self, position: ctypes.c_void_p, san: str | bytes) -> None:
        self._check(
            "scid_position_apply_san",
            self._lib.scid_position_apply_san(position, encode(san)),
        )

    def position_apply_uci(self, position: ctypes.c_void_p, uci: str | bytes) -> None:
        self._check(
            "scid_position_apply_uci",
            self._lib.scid_position_apply_uci(position, encode(uci)),
        )

    def nag_from_string(self, text: str | bytes) -> int:
        nag = ctypes.c_ubyte()
        self._check(
            "scid_nag_create_from_string",
            self._lib.scid_nag_create_from_string(encode(text), ctypes.byref(nag)),
        )
        return nag.value

    def nag_to_string(self, nag: int, symbolic: bool = False) -> str:
        return self._string_result(
            "scid_nag_to_string", ctypes.c_ubyte(nag), int(symbolic)
        )
