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

    def next(self) -> "Cursor | None":
        return self._from_optional_handle(self._native.cursor_next(self._handle))

    def previous(self) -> "Cursor | None":
        return self._from_optional_handle(self._native.cursor_previous(self._handle))

    def enter_variation(self, index: int) -> "Cursor | None":
        return self._from_optional_handle(
            self._native.cursor_enter_variation(self._handle, index)
        )

    def exit_variation(self) -> "Cursor | None":
        return self._from_optional_handle(
            self._native.cursor_exit_variation(self._handle)
        )

    def _from_optional_handle(self, handle: ctypes.c_void_p | None) -> "Cursor | None":
        if handle is None:
            return None
        return self._from_handle(self._native, self._game, handle)

    @property
    def previous_move_san(self) -> str | None:
        if self.is_line_start:
            return None
        return self._native.cursor_previous_move_san(self._handle)

    @property
    def next_move_san(self) -> str | None:
        if self.is_line_end:
            return None
        return self._native.cursor_next_move_san(self._handle)

    @property
    def ply_number(self) -> int:
        return self._native.cursor_ply(self._handle)

    @property
    def variation_count(self) -> int:
        return self._native.cursor_variation_count(self._handle)

    @property
    def variation_depth(self) -> int:
        return self._native.cursor_variation_depth(self._handle)

    @property
    def variation_index(self) -> int:
        return self._native.cursor_variation_index(self._handle)

    @property
    def is_main_line(self) -> bool:
        return self.variation_depth == 0

    @property
    def is_variation_line(self) -> bool:
        return self.variation_depth > 0

    @property
    def is_line_start(self) -> bool:
        return self._native.cursor_is_line_start(self._handle)

    @property
    def is_line_end(self) -> bool:
        return self._native.cursor_is_line_end(self._handle)

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
