from __future__ import annotations

import ctypes

from ._native import NativeLibrary, load_library


class Game:
    def __init__(self, native: NativeLibrary, handle: ctypes.c_void_p):
        self._native = native
        self._handle = handle

    @classmethod
    def from_pgn(cls, pgn: str | bytes) -> "Game":
        native = load_library()
        return cls(native, native.create_game_from_pgn(pgn))

    @property
    def mainline_halfmove_count(self) -> int:
        return self._native.game_mainline_halfmove_count(self._handle)

    def to_pgn(self) -> str:
        return self._native.game_to_pgn(self._handle)

    def _dispose(self) -> None:
        if self._handle:
            self._native.free_game(self._handle)
            self._handle = ctypes.c_void_p()

    def __del__(self) -> None:
        self._dispose()
