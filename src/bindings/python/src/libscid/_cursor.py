from __future__ import annotations

import ctypes
from typing import Any

from ._native import NativeLibrary
from ._position import Position


class Cursor:
    def __init__(self):
        raise TypeError("Cursor objects are returned by libscid APIs")

    @classmethod
    def _from_handle(
        cls,
        native: NativeLibrary,
        game: Any,
        handle: ctypes.c_void_p,
    ) -> "Cursor":
        cursor = cls.__new__(cls)
        cursor._native = native
        cursor._game = game
        cursor._handle = handle
        return cursor

    def clone(self) -> "Cursor":
        return self._from_handle(
            self._native,
            self._game,
            self._native.game_clone_cursor(self._game._handle, self._handle),
        )

    @property
    def ply(self) -> int:
        return self._native.cursor_ply(self._handle)

    @property
    def position(self) -> Position:
        return Position._from_handle(
            self._native, self._native.cursor_position(self._handle)
        )

    def _dispose(self) -> None:
        handle = getattr(self, "_handle", None)
        if handle:
            self._native.free_cursor(self._handle)
            self._handle = ctypes.c_void_p()

    def __del__(self) -> None:
        self._dispose()
