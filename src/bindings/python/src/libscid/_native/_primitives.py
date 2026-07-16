from __future__ import annotations

import ctypes

from ._text import encode


class NativePrimitiveMixin:
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
