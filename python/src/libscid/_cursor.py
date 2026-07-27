from __future__ import annotations

import ctypes
from typing import Any

from ._arbiter import Arbiter
from ._nag import Nag
from ._native import NativeLibrary
from ._position import Position


class Cursor:
    _native: NativeLibrary
    _game: Any
    _handle: ctypes.c_void_p

    def __init__(self):
        raise TypeError("Cursor objects are returned by libscid APIs")

    @classmethod
    def _from_handle(
        cls,
        native: NativeLibrary,
        game: Any,
        handle: ctypes.c_void_p,
    ) -> Cursor:
        cursor = cls.__new__(cls)
        cursor._native = native
        cursor._game = game
        cursor._handle = handle
        return cursor

    def clone(self) -> Cursor:
        return self._from_handle(
            self._native,
            self._game,
            self._native.game_clone_cursor(self._game._handle, self._handle),
        )

    def next(self) -> Cursor | None:
        return self._from_optional_handle(self._native.cursor_next(self._handle))

    def previous(self) -> Cursor | None:
        return self._from_optional_handle(self._native.cursor_previous(self._handle))

    def to_game_start(self) -> Cursor:
        return self._from_handle(
            self._native,
            self._game,
            self._native.cursor_to_game_start(self._handle),
        )

    def to_game_end(self) -> Cursor:
        return self._from_handle(
            self._native,
            self._game,
            self._native.cursor_to_game_end(self._handle),
        )

    def to_main_line_offset(self, offset: int) -> Cursor | None:
        return self._from_optional_handle(
            self._native.cursor_to_main_line_offset(self._handle, offset)
        )

    def enter_variation(self, index: int) -> Cursor | None:
        return self._from_optional_handle(
            self._native.cursor_enter_variation(self._handle, index)
        )

    def exit_variation(self) -> Cursor | None:
        return self._from_optional_handle(
            self._native.cursor_exit_variation(self._handle)
        )

    def append_move(self, san: str | bytes) -> Cursor:
        self._require_line_end("append_move")
        return self._from_handle(
            self._native,
            self._game,
            self._native.cursor_append_move(self._game._handle, self._handle, san),
        )

    def append_game(self, source_game: Any) -> Cursor:
        self._require_line_end("append_game")
        return self._from_handle(
            self._native,
            self._game,
            self._native.cursor_append_game(
                self._game._handle, self._handle, source_game._handle
            ),
        )

    def add_variation(self, preceding_comment: str | bytes = "") -> Cursor | None:
        return self._from_optional_handle(
            self._native.cursor_add_variation(
                self._game._handle, self._handle, preceding_comment
            )
        )

    def remove_variation(self) -> Cursor | None:
        return self._from_optional_handle(
            self._native.cursor_remove_variation(self._game._handle, self._handle)
        )

    def promote_variation_to_first(self) -> Cursor | None:
        return self._from_optional_handle(
            self._native.cursor_promote_variation_to_first(
                self._game._handle, self._handle
            )
        )

    def promote_variation_to_mainline(self) -> Cursor | None:
        return self._from_optional_handle(
            self._native.cursor_promote_variation_to_mainline(
                self._game._handle, self._handle
            )
        )

    def truncate(self) -> Cursor:
        return self._from_handle(
            self._native,
            self._game,
            self._native.cursor_truncate(self._game._handle, self._handle),
        )

    def truncate_before(self) -> Cursor:
        return self._from_handle(
            self._native,
            self._game,
            self._native.cursor_truncate_before(self._game._handle, self._handle),
        )

    def _from_optional_handle(self, handle: ctypes.c_void_p | None) -> Cursor | None:
        if handle is None:
            return None
        return self._from_handle(self._native, self._game, handle)

    def _require_line_end(self, method: str) -> None:
        if not self.is_line_end:
            raise ValueError(f"{method} requires cursor at line end")

    def _path_move_uci(self) -> tuple[str, ...]:
        cursor = self.clone()
        moves = []
        while True:
            move = cursor.previous_move_uci
            if move is not None:
                moves.append(move)
                previous = cursor.previous()
                if previous is None:
                    raise RuntimeError("Cursor previous move disappeared")
                cursor = previous
                continue

            parent = cursor.exit_variation()
            if parent is not None:
                cursor = parent
                continue

            return tuple(reversed(moves))

    @property
    def arbiter(self) -> Arbiter:
        return Arbiter(self)

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
    def previous_move_uci(self) -> str | None:
        if self.is_line_start:
            return None
        return self._native.cursor_previous_move_uci(self._handle)

    @property
    def next_move_uci(self) -> str | None:
        if self.is_line_end:
            return None
        return self._native.cursor_next_move_uci(self._handle)

    @property
    def previous_move_nags(self) -> tuple[Nag, ...] | None:
        if self.is_line_start:
            return None
        return tuple(
            Nag(code) for code in self._native.cursor_previous_move_nags(self._handle)
        )

    @property
    def next_move_nags(self) -> tuple[Nag, ...] | None:
        if self.is_line_end:
            return None
        return tuple(
            Nag(code) for code in self._native.cursor_next_move_nags(self._handle)
        )

    @property
    def comment(self) -> str | None:
        if self.is_line_start:
            return None
        return self._native.cursor_comment(self._handle)

    @property
    def preceding_comment(self) -> str | None:
        if not self.is_line_start:
            return None
        return self._native.cursor_comment(self._handle)

    def set_comment(self, comment: str | bytes) -> None:
        self._native.cursor_set_comment(self._game._handle, self._handle, comment)

    def remove_comment(self) -> None:
        self.set_comment("")

    def add_nag(self, nag: Nag) -> bool:
        return self._native.cursor_add_nag(self._game._handle, self._handle, nag.code)

    def remove_move_nag(self) -> bool:
        return self._native.cursor_remove_nag(
            self._game._handle, self._handle, move_nag=True
        )

    def remove_position_nag(self) -> bool:
        return self._native.cursor_remove_nag(
            self._game._handle, self._handle, move_nag=False
        )

    def remove_nags(self) -> None:
        self._native.cursor_remove_nags(self._game._handle, self._handle)

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
