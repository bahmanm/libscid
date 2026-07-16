from __future__ import annotations

import ctypes

from ._constants import SCID_ERROR_BUFFER_FULL, SCID_OK, STANDARD_FEN
from ._errors import LibScidError
from ._text import decode_buffer, encode
from ._types import PgnOptionsProtocol


class NativeGameMixin:
    def create_blank_game(
        self, position: ctypes.c_void_p | None = None
    ) -> ctypes.c_void_p:
        game = ctypes.c_void_p()

        if position is not None:
            self._check(
                "scid_game_create_blank",
                self._lib.scid_game_create_blank(position, ctypes.byref(game)),
            )
            return game

        standard_position = ctypes.c_void_p()
        self._check(
            "scid_position_create_from_fen",
            self._lib.scid_position_create_from_fen(
                STANDARD_FEN, ctypes.byref(standard_position)
            ),
        )
        try:
            self._check(
                "scid_game_create_blank",
                self._lib.scid_game_create_blank(
                    standard_position, ctypes.byref(game)
                ),
            )
            return game
        finally:
            self._lib.scid_position_free(standard_position)

    def create_game_from_pgn(
        self, pgn: str | bytes, position: ctypes.c_void_p | None = None
    ) -> ctypes.c_void_p:
        pgn_bytes = encode(pgn)
        owns_position = position is None
        game = ctypes.c_void_p()
        diagnostic = ctypes.create_string_buffer(4096)
        diagnostic_size = ctypes.c_size_t()

        if position is None:
            position = ctypes.c_void_p()
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
            if owns_position:
                self._lib.scid_position_free(position)

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
