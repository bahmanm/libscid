from __future__ import annotations

import ctypes
import os
from pathlib import Path

from ._native_constants import SCID_ERROR_BUFFER_FULL, SCID_OK, STANDARD_FEN
from ._native_errors import LibScidError
from ._native_loader import enable_windows_dll_search_dirs, find_library
from ._native_text import decode_buffer, encode
from ._native_types import PgnOptionsProtocol, ScidMoveSpec


class NativeLibrary:
    def __init__(self, library_path: str | os.PathLike[str] | None = None):
        self.library_path = (
            Path(library_path).resolve() if library_path else find_library().resolve()
        )
        enable_windows_dll_search_dirs(self.library_path)
        self._lib = ctypes.CDLL(str(self.library_path))
        self._bind_functions()

    def create_blank_game(self) -> ctypes.c_void_p:
        position = ctypes.c_void_p()
        game = ctypes.c_void_p()

        self._check(
            "scid_position_create_from_fen",
            self._lib.scid_position_create_from_fen(
                STANDARD_FEN, ctypes.byref(position)
            ),
        )
        try:
            self._check(
                "scid_game_create_blank",
                self._lib.scid_game_create_blank(position, ctypes.byref(game)),
            )
            return game
        finally:
            self._lib.scid_position_free(position)

    def create_game_from_pgn(self, pgn: str | bytes) -> ctypes.c_void_p:
        pgn_bytes = encode(pgn)
        position = ctypes.c_void_p()
        game = ctypes.c_void_p()
        diagnostic = ctypes.create_string_buffer(4096)
        diagnostic_size = ctypes.c_size_t()

        self._check(
            "scid_position_create_from_fen",
            self._lib.scid_position_create_from_fen(
                STANDARD_FEN, ctypes.byref(position)
            ),
        )
        try:
            error = self._lib.scid_game_create(
                position,
                pgn_bytes,
                len(pgn_bytes),
                ctypes.byref(game),
                diagnostic,
                len(diagnostic),
                ctypes.byref(diagnostic_size),
            )
            if error != SCID_OK:
                raise LibScidError(
                    "scid_game_create",
                    error,
                    decode_buffer(diagnostic, diagnostic_size.value),
                )
            return game
        finally:
            self._lib.scid_position_free(position)

    def free_game(self, game: ctypes.c_void_p) -> None:
        self._lib.scid_game_free(game)

    def free_position(self, position: ctypes.c_void_p) -> None:
        self._lib.scid_position_free(position)

    def free_cursor(self, cursor: ctypes.c_void_p) -> None:
        self._lib.scid_game_cursor_free(cursor)

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

    def game_mainline_move_count(self, game: ctypes.c_void_p) -> int:
        count = ctypes.c_size_t()
        self._check(
            "scid_game_mainline_halfmove_count_get",
            self._lib.scid_game_mainline_halfmove_count_get(
                game, ctypes.byref(count)
            ),
        )
        return count.value

    def game_start_position(self, game: ctypes.c_void_p) -> ctypes.c_void_p:
        position = self._create_standard_position()
        try:
            self._check(
                "scid_game_start_position_get",
                self._lib.scid_game_start_position_get(game, position),
            )
            return position
        except Exception:
            self._lib.scid_position_free(position)
            raise

    def game_final_position(self, game: ctypes.c_void_p) -> ctypes.c_void_p:
        position = self._create_standard_position()
        try:
            self._check(
                "scid_game_final_position_get",
                self._lib.scid_game_final_position_get(game, position),
            )
            return position
        except Exception:
            self._lib.scid_position_free(position)
            raise

    def game_create_cursor(self, game: ctypes.c_void_p) -> ctypes.c_void_p:
        cursor = ctypes.c_void_p()
        self._check(
            "scid_game_cursor_create",
            self._lib.scid_game_cursor_create(game, ctypes.byref(cursor)),
        )
        return cursor

    def game_clone_cursor(
        self, game: ctypes.c_void_p, cursor: ctypes.c_void_p
    ) -> ctypes.c_void_p:
        clone = ctypes.c_void_p()
        self._check(
            "scid_game_cursor_clone",
            self._lib.scid_game_cursor_clone(game, cursor, ctypes.byref(clone)),
        )
        return clone

    def cursor_ply(self, cursor: ctypes.c_void_p) -> int:
        return self._cursor_size_result("scid_game_cursor_ply_get", cursor)

    def cursor_variation_count(self, cursor: ctypes.c_void_p) -> int:
        return self._cursor_size_result("scid_game_cursor_variation_count_get", cursor)

    def cursor_variation_depth(self, cursor: ctypes.c_void_p) -> int:
        return self._cursor_size_result("scid_game_cursor_variation_depth_get", cursor)

    def cursor_variation_index(self, cursor: ctypes.c_void_p) -> int:
        return self._cursor_size_result("scid_game_cursor_variation_index_get", cursor)

    def cursor_is_line_start(self, cursor: ctypes.c_void_p) -> bool:
        return self._cursor_bool_result("scid_game_cursor_is_line_start", cursor)

    def cursor_is_line_end(self, cursor: ctypes.c_void_p) -> bool:
        return self._cursor_bool_result("scid_game_cursor_is_line_end", cursor)

    def cursor_next(self, cursor: ctypes.c_void_p) -> ctypes.c_void_p | None:
        return self._cursor_navigation_result("scid_game_cursor_next", cursor)

    def cursor_previous(self, cursor: ctypes.c_void_p) -> ctypes.c_void_p | None:
        return self._cursor_navigation_result("scid_game_cursor_previous", cursor)

    def cursor_to_game_start(self, cursor: ctypes.c_void_p) -> ctypes.c_void_p:
        return self._cursor_result("scid_game_cursor_to_start", cursor)

    def cursor_to_game_end(self, cursor: ctypes.c_void_p) -> ctypes.c_void_p:
        return self._cursor_result("scid_game_cursor_to_end", cursor)

    def cursor_enter_variation(
        self, cursor: ctypes.c_void_p, index: int
    ) -> ctypes.c_void_p | None:
        return self._cursor_navigation_result(
            "scid_game_cursor_variation_enter", cursor, index
        )

    def cursor_exit_variation(self, cursor: ctypes.c_void_p) -> ctypes.c_void_p | None:
        return self._cursor_navigation_result(
            "scid_game_cursor_variation_exit", cursor
        )

    def cursor_add_variation(
        self,
        game: ctypes.c_void_p,
        cursor: ctypes.c_void_p,
        preceding_comment: str | bytes = "",
    ) -> ctypes.c_void_p | None:
        added = ctypes.c_int()
        variation_cursor = ctypes.c_void_p()
        self._check(
            "scid_game_cursor_variation_add",
            self._lib.scid_game_cursor_variation_add(
                game,
                cursor,
                encode(preceding_comment),
                ctypes.byref(added),
                ctypes.byref(variation_cursor),
            ),
        )
        if not added.value:
            return None
        return variation_cursor

    def cursor_remove_variation(
        self, game: ctypes.c_void_p, cursor: ctypes.c_void_p
    ) -> ctypes.c_void_p | None:
        removed = ctypes.c_int()
        parent_cursor = ctypes.c_void_p()
        self._check(
            "scid_game_cursor_variation_delete",
            self._lib.scid_game_cursor_variation_delete(
                game,
                cursor,
                ctypes.byref(removed),
                ctypes.byref(parent_cursor),
            ),
        )
        if not removed.value:
            return None
        return parent_cursor

    def cursor_promote_variation_to_first(
        self, game: ctypes.c_void_p, cursor: ctypes.c_void_p
    ) -> ctypes.c_void_p | None:
        promoted = ctypes.c_int()
        promoted_cursor = ctypes.c_void_p()
        self._check(
            "scid_game_cursor_variation_promote_to_first",
            self._lib.scid_game_cursor_variation_promote_to_first(
                game,
                cursor,
                ctypes.byref(promoted),
                ctypes.byref(promoted_cursor),
            ),
        )
        if not promoted.value:
            return None
        return promoted_cursor

    def cursor_previous_move_san(self, cursor: ctypes.c_void_p) -> str:
        return self._string_result("scid_game_cursor_previous_move_san_get", cursor)

    def cursor_next_move_san(self, cursor: ctypes.c_void_p) -> str:
        return self._string_result("scid_game_cursor_next_move_san_get", cursor)

    def cursor_previous_move_uci(self, cursor: ctypes.c_void_p) -> str:
        return self._cursor_move_uci(
            "scid_game_cursor_previous_movespec_get", cursor
        )

    def cursor_next_move_uci(self, cursor: ctypes.c_void_p) -> str:
        return self._cursor_move_uci("scid_game_cursor_next_movespec_get", cursor)

    def cursor_previous_move_nags(self, cursor: ctypes.c_void_p) -> tuple[int, ...]:
        return self._cursor_nags_result(
            "scid_game_cursor_previous_move_nag_count_get",
            "scid_game_cursor_previous_move_nag_at_get",
            cursor,
        )

    def cursor_next_move_nags(self, cursor: ctypes.c_void_p) -> tuple[int, ...]:
        return self._cursor_nags_result(
            "scid_game_cursor_next_move_nag_count_get",
            "scid_game_cursor_next_move_nag_at_get",
            cursor,
        )

    def cursor_comment(self, cursor: ctypes.c_void_p) -> str:
        return self._string_result("scid_game_cursor_comment_get", cursor)

    def cursor_set_comment(
        self, game: ctypes.c_void_p, cursor: ctypes.c_void_p, comment: str | bytes
    ) -> None:
        self._check(
            "scid_game_cursor_comment_set",
            self._lib.scid_game_cursor_comment_set(game, cursor, encode(comment)),
        )

    def cursor_add_nag(
        self, game: ctypes.c_void_p, cursor: ctypes.c_void_p, nag: int
    ) -> bool:
        added = ctypes.c_int()
        self._check(
            "scid_game_cursor_nag_add",
            self._lib.scid_game_cursor_nag_add(
                game, cursor, ctypes.c_ubyte(nag), ctypes.byref(added)
            ),
        )
        return bool(added.value)

    def cursor_remove_nag(
        self, game: ctypes.c_void_p, cursor: ctypes.c_void_p, move_nag: bool
    ) -> bool:
        removed = ctypes.c_int()
        self._check(
            "scid_game_cursor_nag_remove",
            self._lib.scid_game_cursor_nag_remove(
                game, cursor, int(move_nag), ctypes.byref(removed)
            ),
        )
        return bool(removed.value)

    def cursor_remove_nags(
        self, game: ctypes.c_void_p, cursor: ctypes.c_void_p
    ) -> None:
        self._check(
            "scid_game_cursor_nag_clear",
            self._lib.scid_game_cursor_nag_clear(game, cursor),
        )

    def cursor_position(self, cursor: ctypes.c_void_p) -> ctypes.c_void_p:
        position = self._create_standard_position()
        try:
            self._check(
                "scid_game_cursor_position_get",
                self._lib.scid_game_cursor_position_get(cursor, position),
            )
            return position
        except Exception:
            self._lib.scid_position_free(position)
            raise

    def game_get_tag(self, game: ctypes.c_void_p, name: str | bytes) -> str:
        return self._string_result("scid_game_tag_get", game, encode(name))

    def game_set_tag(
        self, game: ctypes.c_void_p, name: str | bytes, value: str | bytes
    ) -> None:
        self._check(
            "scid_game_tag_set",
            self._lib.scid_game_tag_set(game, encode(name), encode(value)),
        )

    def game_tag_count(self, game: ctypes.c_void_p) -> int:
        count = ctypes.c_size_t()
        self._check(
            "scid_game_tag_count_get",
            self._lib.scid_game_tag_count_get(game, ctypes.byref(count)),
        )
        return count.value

    def game_tag_at(self, game: ctypes.c_void_p, index: int) -> tuple[str, str]:
        name_capacity = 1024
        value_capacity = 1024
        while True:
            name = ctypes.create_string_buffer(name_capacity)
            name_size = ctypes.c_size_t()
            value = ctypes.create_string_buffer(value_capacity)
            value_size = ctypes.c_size_t()
            error = self._lib.scid_game_tag_at_get(
                game,
                index,
                name,
                name_capacity,
                ctypes.byref(name_size),
                value,
                value_capacity,
                ctypes.byref(value_size),
            )
            if error == SCID_OK:
                return (
                    decode_buffer(name, name_size.value),
                    decode_buffer(value, value_size.value),
                )
            if error != SCID_ERROR_BUFFER_FULL:
                raise LibScidError("scid_game_tag_at_get", error)
            name_capacity = max(name_capacity * 2, name_size.value + 1)
            value_capacity = max(value_capacity * 2, value_size.value + 1)

    def game_get_tags(self, game: ctypes.c_void_p) -> tuple[tuple[str, str], ...]:
        return tuple(
            self.game_tag_at(game, index)
            for index in range(self.game_tag_count(game))
        )

    def game_remove_tag(self, game: ctypes.c_void_p, name: str | bytes) -> bool:
        removed = ctypes.c_int()
        self._check(
            "scid_game_tag_remove",
            self._lib.scid_game_tag_remove(
                game, encode(name), ctypes.byref(removed)
            ),
        )
        return bool(removed.value)

    def game_to_pgn(
        self, game: ctypes.c_void_p, options: PgnOptionsProtocol | None = None
    ) -> str:
        if options is None:
            return self._string_result("scid_game_to_pgn", game, None)

        native_options = self._create_pgn_options(options)
        try:
            return self._string_result("scid_game_to_pgn", game, native_options)
        finally:
            self._lib.scid_game_pgn_options_free(native_options)

    def _create_pgn_options(self, options: PgnOptionsProtocol) -> ctypes.c_void_p:
        native_options = ctypes.c_void_p()
        self._check(
            "scid_game_pgn_options_create",
            self._lib.scid_game_pgn_options_create(ctypes.byref(native_options)),
        )
        try:
            self._check(
                "scid_game_pgn_options_symbolic_nags_set",
                self._lib.scid_game_pgn_options_symbolic_nags_set(
                    native_options, int(options.symbolic_nags)
                ),
            )
            self._check(
                "scid_game_pgn_options_supplemental_tags_set",
                self._lib.scid_game_pgn_options_supplemental_tags_set(
                    native_options, int(options.supplemental_tags)
                ),
            )
            self._check(
                "scid_game_pgn_options_comments_set",
                self._lib.scid_game_pgn_options_comments_set(
                    native_options, int(options.comments)
                ),
            )
            self._check(
                "scid_game_pgn_options_variations_set",
                self._lib.scid_game_pgn_options_variations_set(
                    native_options, int(options.variations)
                ),
            )
            self._check(
                "scid_game_pgn_options_line_width_set",
                self._lib.scid_game_pgn_options_line_width_set(
                    native_options,
                    0 if options.line_width is None else options.line_width,
                ),
            )
        except Exception:
            self._lib.scid_game_pgn_options_free(native_options)
            raise
        return native_options

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

    def _bind_functions(self) -> None:
        c_size_t_p = ctypes.POINTER(ctypes.c_size_t)
        c_void_p_p = ctypes.POINTER(ctypes.c_void_p)

        self._lib.scid_position_create_from_fen.argtypes = [
            ctypes.c_char_p,
            c_void_p_p,
        ]
        self._lib.scid_position_create_from_fen.restype = ctypes.c_ushort

        self._lib.scid_position_free.argtypes = [ctypes.c_void_p]
        self._lib.scid_position_free.restype = None

        self._lib.scid_position_to_fen.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ]
        self._lib.scid_position_to_fen.restype = ctypes.c_ushort

        self._lib.scid_nag_create_from_string.argtypes = [
            ctypes.c_char_p,
            ctypes.POINTER(ctypes.c_ubyte),
        ]
        self._lib.scid_nag_create_from_string.restype = ctypes.c_ushort

        self._lib.scid_nag_to_string.argtypes = [
            ctypes.c_ubyte,
            ctypes.c_int,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ]
        self._lib.scid_nag_to_string.restype = ctypes.c_ushort

        self._lib.scid_game_create.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_void_p_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ]
        self._lib.scid_game_create.restype = ctypes.c_ushort

        self._lib.scid_game_create_blank.argtypes = [
            ctypes.c_void_p,
            c_void_p_p,
        ]
        self._lib.scid_game_create_blank.restype = ctypes.c_ushort

        self._lib.scid_game_free.argtypes = [ctypes.c_void_p]
        self._lib.scid_game_free.restype = None

        self._lib.scid_game_pgn_options_create.argtypes = [c_void_p_p]
        self._lib.scid_game_pgn_options_create.restype = ctypes.c_ushort

        self._lib.scid_game_pgn_options_free.argtypes = [ctypes.c_void_p]
        self._lib.scid_game_pgn_options_free.restype = None

        self._lib.scid_game_pgn_options_symbolic_nags_set.argtypes = [
            ctypes.c_void_p,
            ctypes.c_int,
        ]
        self._lib.scid_game_pgn_options_symbolic_nags_set.restype = ctypes.c_ushort

        self._lib.scid_game_pgn_options_supplemental_tags_set.argtypes = [
            ctypes.c_void_p,
            ctypes.c_int,
        ]
        self._lib.scid_game_pgn_options_supplemental_tags_set.restype = ctypes.c_ushort

        self._lib.scid_game_pgn_options_comments_set.argtypes = [
            ctypes.c_void_p,
            ctypes.c_int,
        ]
        self._lib.scid_game_pgn_options_comments_set.restype = ctypes.c_ushort

        self._lib.scid_game_pgn_options_variations_set.argtypes = [
            ctypes.c_void_p,
            ctypes.c_int,
        ]
        self._lib.scid_game_pgn_options_variations_set.restype = ctypes.c_ushort

        self._lib.scid_game_pgn_options_line_width_set.argtypes = [
            ctypes.c_void_p,
            ctypes.c_uint,
        ]
        self._lib.scid_game_pgn_options_line_width_set.restype = ctypes.c_ushort

        self._lib.scid_game_mainline_halfmove_count_get.argtypes = [
            ctypes.c_void_p,
            c_size_t_p,
        ]
        self._lib.scid_game_mainline_halfmove_count_get.restype = ctypes.c_ushort

        self._lib.scid_game_start_position_get.argtypes = [
            ctypes.c_void_p,
            ctypes.c_void_p,
        ]
        self._lib.scid_game_start_position_get.restype = ctypes.c_ushort

        self._lib.scid_game_final_position_get.argtypes = [
            ctypes.c_void_p,
            ctypes.c_void_p,
        ]
        self._lib.scid_game_final_position_get.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_create.argtypes = [
            ctypes.c_void_p,
            c_void_p_p,
        ]
        self._lib.scid_game_cursor_create.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_clone.argtypes = [
            ctypes.c_void_p,
            ctypes.c_void_p,
            c_void_p_p,
        ]
        self._lib.scid_game_cursor_clone.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_free.argtypes = [ctypes.c_void_p]
        self._lib.scid_game_cursor_free.restype = None

        self._lib.scid_game_cursor_ply_get.argtypes = [
            ctypes.c_void_p,
            c_size_t_p,
        ]
        self._lib.scid_game_cursor_ply_get.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_variation_count_get.argtypes = [
            ctypes.c_void_p,
            c_size_t_p,
        ]
        self._lib.scid_game_cursor_variation_count_get.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_variation_depth_get.argtypes = [
            ctypes.c_void_p,
            c_size_t_p,
        ]
        self._lib.scid_game_cursor_variation_depth_get.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_variation_index_get.argtypes = [
            ctypes.c_void_p,
            c_size_t_p,
        ]
        self._lib.scid_game_cursor_variation_index_get.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_position_get.argtypes = [
            ctypes.c_void_p,
            ctypes.c_void_p,
        ]
        self._lib.scid_game_cursor_position_get.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_is_line_start.argtypes = [
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_int),
        ]
        self._lib.scid_game_cursor_is_line_start.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_is_line_end.argtypes = [
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_int),
        ]
        self._lib.scid_game_cursor_is_line_end.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_next.argtypes = [
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_int),
            c_void_p_p,
        ]
        self._lib.scid_game_cursor_next.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_previous.argtypes = [
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_int),
            c_void_p_p,
        ]
        self._lib.scid_game_cursor_previous.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_to_start.argtypes = [
            ctypes.c_void_p,
            c_void_p_p,
        ]
        self._lib.scid_game_cursor_to_start.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_to_end.argtypes = [
            ctypes.c_void_p,
            c_void_p_p,
        ]
        self._lib.scid_game_cursor_to_end.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_variation_enter.argtypes = [
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.POINTER(ctypes.c_int),
            c_void_p_p,
        ]
        self._lib.scid_game_cursor_variation_enter.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_variation_exit.argtypes = [
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_int),
            c_void_p_p,
        ]
        self._lib.scid_game_cursor_variation_exit.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_variation_add.argtypes = [
            ctypes.c_void_p,
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.POINTER(ctypes.c_int),
            c_void_p_p,
        ]
        self._lib.scid_game_cursor_variation_add.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_variation_delete.argtypes = [
            ctypes.c_void_p,
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_int),
            c_void_p_p,
        ]
        self._lib.scid_game_cursor_variation_delete.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_variation_promote_to_first.argtypes = [
            ctypes.c_void_p,
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_int),
            c_void_p_p,
        ]
        self._lib.scid_game_cursor_variation_promote_to_first.restype = (
            ctypes.c_ushort
        )

        self._lib.scid_game_cursor_previous_move_san_get.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ]
        self._lib.scid_game_cursor_previous_move_san_get.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_next_move_san_get.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ]
        self._lib.scid_game_cursor_next_move_san_get.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_previous_movespec_get.argtypes = [
            ctypes.c_void_p,
            ctypes.POINTER(ScidMoveSpec),
        ]
        self._lib.scid_game_cursor_previous_movespec_get.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_next_movespec_get.argtypes = [
            ctypes.c_void_p,
            ctypes.POINTER(ScidMoveSpec),
        ]
        self._lib.scid_game_cursor_next_movespec_get.restype = ctypes.c_ushort

        self._lib.scid_movespec_to_uci.argtypes = [
            ScidMoveSpec,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ]
        self._lib.scid_movespec_to_uci.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_previous_move_nag_count_get.argtypes = [
            ctypes.c_void_p,
            c_size_t_p,
        ]
        self._lib.scid_game_cursor_previous_move_nag_count_get.restype = (
            ctypes.c_ushort
        )

        self._lib.scid_game_cursor_previous_move_nag_at_get.argtypes = [
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.POINTER(ctypes.c_ubyte),
        ]
        self._lib.scid_game_cursor_previous_move_nag_at_get.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_next_move_nag_count_get.argtypes = [
            ctypes.c_void_p,
            c_size_t_p,
        ]
        self._lib.scid_game_cursor_next_move_nag_count_get.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_next_move_nag_at_get.argtypes = [
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.POINTER(ctypes.c_ubyte),
        ]
        self._lib.scid_game_cursor_next_move_nag_at_get.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_comment_get.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ]
        self._lib.scid_game_cursor_comment_get.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_comment_set.argtypes = [
            ctypes.c_void_p,
            ctypes.c_void_p,
            ctypes.c_char_p,
        ]
        self._lib.scid_game_cursor_comment_set.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_nag_add.argtypes = [
            ctypes.c_void_p,
            ctypes.c_void_p,
            ctypes.c_ubyte,
            ctypes.POINTER(ctypes.c_int),
        ]
        self._lib.scid_game_cursor_nag_add.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_nag_remove.argtypes = [
            ctypes.c_void_p,
            ctypes.c_void_p,
            ctypes.c_int,
            ctypes.POINTER(ctypes.c_int),
        ]
        self._lib.scid_game_cursor_nag_remove.restype = ctypes.c_ushort

        self._lib.scid_game_cursor_nag_clear.argtypes = [
            ctypes.c_void_p,
            ctypes.c_void_p,
        ]
        self._lib.scid_game_cursor_nag_clear.restype = ctypes.c_ushort

        self._lib.scid_game_tag_get.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ]
        self._lib.scid_game_tag_get.restype = ctypes.c_ushort

        self._lib.scid_game_tag_set.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_char_p,
        ]
        self._lib.scid_game_tag_set.restype = ctypes.c_ushort

        self._lib.scid_game_tag_count_get.argtypes = [
            ctypes.c_void_p,
            c_size_t_p,
        ]
        self._lib.scid_game_tag_count_get.restype = ctypes.c_ushort

        self._lib.scid_game_tag_at_get.argtypes = [
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ]
        self._lib.scid_game_tag_at_get.restype = ctypes.c_ushort

        self._lib.scid_game_tag_remove.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.POINTER(ctypes.c_int),
        ]
        self._lib.scid_game_tag_remove.restype = ctypes.c_ushort

        self._lib.scid_game_to_pgn.argtypes = [
            ctypes.c_void_p,
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ]
        self._lib.scid_game_to_pgn.restype = ctypes.c_ushort


def load_library(library_path: str | os.PathLike[str] | None = None) -> NativeLibrary:
    return NativeLibrary(library_path)
