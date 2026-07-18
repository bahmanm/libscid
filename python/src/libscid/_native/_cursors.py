from __future__ import annotations

import ctypes

from ._base import NativeLibraryBase
from ._constants import SCID_GAME_MERGE_MOVES_APPEND
from ._text import encode
from ._types import ScidMoveSpec


class NativeCursorMixin(NativeLibraryBase):
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

    def cursor_to_main_line_offset(
        self, cursor: ctypes.c_void_p, offset: int
    ) -> ctypes.c_void_p | None:
        return self._cursor_navigation_result("scid_game_cursor_to_ply", cursor, offset)

    def cursor_enter_variation(
        self, cursor: ctypes.c_void_p, index: int
    ) -> ctypes.c_void_p | None:
        return self._cursor_navigation_result(
            "scid_game_cursor_variation_enter", cursor, index
        )

    def cursor_exit_variation(self, cursor: ctypes.c_void_p) -> ctypes.c_void_p | None:
        return self._cursor_navigation_result("scid_game_cursor_variation_exit", cursor)

    def cursor_append_move(
        self, game: ctypes.c_void_p, cursor: ctypes.c_void_p, san: str | bytes
    ) -> ctypes.c_void_p:
        position = self.cursor_position(cursor)
        move = ScidMoveSpec()
        try:
            self._check(
                "scid_movespec_create_from_san",
                self._lib.scid_movespec_create_from_san(
                    position, encode(san), ctypes.byref(move)
                ),
            )
        finally:
            self._lib.scid_position_free(position)

        next_cursor = ctypes.c_void_p()
        self._check(
            "scid_game_cursor_move_add",
            self._lib.scid_game_cursor_move_add(
                game, cursor, move, ctypes.byref(next_cursor)
            ),
        )
        return next_cursor

    def cursor_append_game(
        self,
        game: ctypes.c_void_p,
        cursor: ctypes.c_void_p,
        source_game: ctypes.c_void_p,
    ) -> ctypes.c_void_p:
        next_cursor = ctypes.c_void_p()
        self._check(
            "scid_game_merge_moves",
            self._lib.scid_game_merge_moves(
                game,
                cursor,
                source_game,
                SCID_GAME_MERGE_MOVES_APPEND,
                ctypes.byref(next_cursor),
            ),
        )
        return next_cursor

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

    def cursor_promote_variation_to_mainline(
        self, game: ctypes.c_void_p, cursor: ctypes.c_void_p
    ) -> ctypes.c_void_p | None:
        promoted = ctypes.c_int()
        mainline_cursor = ctypes.c_void_p()
        self._check(
            "scid_game_cursor_variation_promote_to_mainline",
            self._lib.scid_game_cursor_variation_promote_to_mainline(
                game,
                cursor,
                ctypes.byref(promoted),
                ctypes.byref(mainline_cursor),
            ),
        )
        if not promoted.value:
            return None
        return mainline_cursor

    def cursor_truncate(
        self, game: ctypes.c_void_p, cursor: ctypes.c_void_p
    ) -> ctypes.c_void_p:
        truncated_cursor = ctypes.c_void_p()
        self._check(
            "scid_game_cursor_truncate",
            self._lib.scid_game_cursor_truncate(
                game,
                cursor,
                ctypes.byref(truncated_cursor),
            ),
        )
        return truncated_cursor

    def cursor_truncate_before(
        self, game: ctypes.c_void_p, cursor: ctypes.c_void_p
    ) -> ctypes.c_void_p:
        truncated_cursor = ctypes.c_void_p()
        self._check(
            "scid_game_cursor_truncate_before_cursor",
            self._lib.scid_game_cursor_truncate_before_cursor(
                game,
                cursor,
                ctypes.byref(truncated_cursor),
            ),
        )
        return truncated_cursor

    def cursor_previous_move_san(self, cursor: ctypes.c_void_p) -> str:
        return self._string_result("scid_game_cursor_previous_move_san_get", cursor)

    def cursor_next_move_san(self, cursor: ctypes.c_void_p) -> str:
        return self._string_result("scid_game_cursor_next_move_san_get", cursor)

    def cursor_previous_move_uci(self, cursor: ctypes.c_void_p) -> str:
        return self._cursor_move_uci("scid_game_cursor_previous_movespec_get", cursor)

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
