from __future__ import annotations

import ctypes
import os
from pathlib import Path

from ._bindings import bind_functions
from ._constants import SCID_ERROR_BUFFER_FULL, SCID_OK, STANDARD_FEN
from ._errors import LibScidError
from ._loader import enable_windows_dll_search_dirs, find_library
from ._text import decode_buffer
from ._types import ScidMoveSpec


class NativeLibraryBase:
    def __init__(self, library_path: str | os.PathLike[str] | None = None):
        self.library_path = (
            Path(library_path).resolve() if library_path else find_library().resolve()
        )
        enable_windows_dll_search_dirs(self.library_path)
        self._lib = ctypes.CDLL(str(self.library_path))
        bind_functions(self._lib)

    def free_game(self, game: ctypes.c_void_p) -> None:
        self._lib.scid_game_free(game)

    def free_position(self, position: ctypes.c_void_p) -> None:
        self._lib.scid_position_free(position)

    def free_cursor(self, cursor: ctypes.c_void_p) -> None:
        self._lib.scid_game_cursor_free(cursor)

    def _check(self, function: str, error: int) -> None:
        if error != SCID_OK:
            raise LibScidError(function, error)

    def _create_standard_position(self) -> ctypes.c_void_p:
        position = ctypes.c_void_p()
        self._check(
            "scid_position_create_from_fen",
            self._lib.scid_position_create_from_fen(
                STANDARD_FEN, ctypes.byref(position)
            ),
        )
        return position

    def _cursor_size_result(self, function_name: str, cursor: ctypes.c_void_p) -> int:
        value = ctypes.c_size_t()
        function = getattr(self._lib, function_name)
        self._check(function_name, function(cursor, ctypes.byref(value)))
        return value.value

    def _cursor_bool_result(self, function_name: str, cursor: ctypes.c_void_p) -> bool:
        value = ctypes.c_int()
        function = getattr(self._lib, function_name)
        self._check(function_name, function(cursor, ctypes.byref(value)))
        return bool(value.value)

    def _cursor_navigation_result(
        self,
        function_name: str,
        cursor: ctypes.c_void_p,
        *args: object,
    ) -> ctypes.c_void_p | None:
        moved = ctypes.c_int()
        next_cursor = ctypes.c_void_p()
        function = getattr(self._lib, function_name)
        self._check(
            function_name,
            function(cursor, *args, ctypes.byref(moved), ctypes.byref(next_cursor)),
        )
        if not moved.value:
            return None
        return next_cursor

    def _cursor_result(
        self, function_name: str, cursor: ctypes.c_void_p
    ) -> ctypes.c_void_p:
        next_cursor = ctypes.c_void_p()
        function = getattr(self._lib, function_name)
        self._check(function_name, function(cursor, ctypes.byref(next_cursor)))
        return next_cursor

    def _cursor_nags_result(
        self,
        count_function_name: str,
        at_function_name: str,
        cursor: ctypes.c_void_p,
    ) -> tuple[int, ...]:
        count = self._cursor_size_result(count_function_name, cursor)
        at_function = getattr(self._lib, at_function_name)
        nags = []
        for index in range(count):
            nag = ctypes.c_ubyte()
            self._check(
                at_function_name,
                at_function(cursor, index, ctypes.byref(nag)),
            )
            nags.append(nag.value)
        return tuple(nags)

    def _cursor_move_uci(self, function_name: str, cursor: ctypes.c_void_p) -> str:
        move = ScidMoveSpec()
        function = getattr(self._lib, function_name)
        self._check(function_name, function(cursor, ctypes.byref(move)))
        return self._string_result("scid_movespec_to_uci", move)

    def _string_result(self, function_name: str, *args: object) -> str:
        function = getattr(self._lib, function_name)
        capacity = 1024
        while True:
            output = ctypes.create_string_buffer(capacity)
            output_size = ctypes.c_size_t()
            error = function(*args, output, capacity, ctypes.byref(output_size))
            if error == SCID_OK:
                return decode_buffer(output, output_size.value)
            if error != SCID_ERROR_BUFFER_FULL:
                raise LibScidError(function_name, error)
            capacity *= 2
