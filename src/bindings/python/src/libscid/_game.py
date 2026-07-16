from __future__ import annotations

import ctypes

from ._native import NativeLibrary, load_library
from ._pgn import PgnOptions


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

    def get_tag(self, name: str | bytes) -> str:
        return self._native.game_get_tag(self._handle, name)

    def set_tag(self, name: str | bytes, value: str | bytes) -> None:
        self._native.game_set_tag(self._handle, name, value)

    def remove_tag(self, name: str | bytes) -> bool:
        return self._native.game_remove_tag(self._handle, name)

    def get_tags(self) -> tuple[tuple[str, str], ...]:
        return self._native.game_get_tags(self._handle)

    def to_pgn(self, options: PgnOptions | None = None) -> str:
        return self._native.game_to_pgn(self._handle, options)

    def _dispose(self) -> None:
        if self._handle:
            self._native.free_game(self._handle)
            self._handle = ctypes.c_void_p()

    def __del__(self) -> None:
        self._dispose()
