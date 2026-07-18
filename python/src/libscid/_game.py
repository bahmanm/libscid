from __future__ import annotations

import ctypes

from ._cursor import Cursor
from ._native import NativeLibrary, load_library
from ._pgn import PgnOptions
from ._position import Position


class Game:
    _native: NativeLibrary
    _handle: ctypes.c_void_p

    def __init__(self, position: Position | None = None):
        if position is None:
            self._native = load_library()
            self._handle = self._native.create_blank_game()
            return

        self._native = position._native
        self._handle = self._native.create_blank_game(position._handle)

    @classmethod
    def from_pgn(cls, pgn: str | bytes, position: Position | None = None) -> Game:
        if position is None:
            native = load_library()
            return cls._from_handle(native, native.create_game_from_pgn(pgn))

        native = position._native
        return cls._from_handle(
            native, native.create_game_from_pgn(pgn, position._handle)
        )

    @classmethod
    def _from_handle(cls, native: NativeLibrary, handle: ctypes.c_void_p) -> Game:
        game = cls.__new__(cls)
        game._native = native
        game._handle = handle
        return game

    @property
    def mainline_move_count(self) -> int:
        return self._native.game_mainline_move_count(self._handle)

    @property
    def start_position(self) -> Position:
        return Position._from_handle(
            self._native, self._native.game_start_position(self._handle)
        )

    @property
    def end_position(self) -> Position:
        return Position._from_handle(
            self._native, self._native.game_final_position(self._handle)
        )

    def get_tag(self, name: str | bytes) -> str:
        return self._native.game_get_tag(self._handle, name)

    def set_tag(self, name: str | bytes, value: str | bytes) -> None:
        self._native.game_set_tag(self._handle, name, value)

    def remove_tag(self, name: str | bytes) -> bool:
        return self._native.game_remove_tag(self._handle, name)

    def get_tags(self) -> tuple[tuple[str, str], ...]:
        return self._native.game_get_tags(self._handle)

    def create_cursor(self) -> Cursor:
        return Cursor._from_handle(
            self._native, self, self._native.game_create_cursor(self._handle)
        )

    def to_pgn(self, options: PgnOptions | None = None) -> str:
        return self._native.game_to_pgn(self._handle, options)

    def _dispose(self) -> None:
        handle = getattr(self, "_handle", None)
        if handle:
            self._native.free_game(self._handle)
            self._handle = ctypes.c_void_p()

    def __del__(self) -> None:
        self._dispose()
