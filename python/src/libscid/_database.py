from __future__ import annotations

import ctypes
import os
from collections.abc import Callable

from ._database_filters import DatabaseFilters
from ._game import Game
from ._native import NativeLibrary, load_library

ProgressReportCallback = Callable[[int, int, str | None], None]
ShouldCancelFn = Callable[[], bool]


class Database:
    _native: NativeLibrary
    _handle: ctypes.c_void_p
    _filters: DatabaseFilters

    def __init__(self):
        raise TypeError("Database objects are returned by libscid APIs")

    @classmethod
    def open_pgn_read_only(
        cls,
        path: str | os.PathLike[str],
        progress_report_callback: ProgressReportCallback | None = None,
        should_cancel: ShouldCancelFn | None = None,
    ) -> Database:
        native = load_library()
        return cls._from_handle(
            native,
            native.open_pgn_database_read_only(
                path,
                progress_report_callback=progress_report_callback,
                should_cancel=should_cancel,
            ),
        )

    @classmethod
    def _from_handle(cls, native: NativeLibrary, handle: ctypes.c_void_p) -> Database:
        database = cls.__new__(cls)
        database._native = native
        database._handle = handle
        database._filters = DatabaseFilters._from_database(native, database)
        return database

    @property
    def type(self) -> str:
        return self._native.database_type(self._handle)

    @property
    def read_only(self) -> bool:
        return self._native.database_read_only(self._handle)

    @property
    def game_count(self) -> int:
        return self._native.database_game_count(self._handle)

    @property
    def filters(self) -> DatabaseFilters:
        return self._filters

    def get_tag(self, index: int, name: str | bytes) -> str:
        return self._native.database_game_tag(self._handle, index, name)

    def get_game(self, index: int) -> Game:
        return Game._from_handle(
            self._native, self._native.database_game(self._handle, index)
        )

    def close(self) -> None:
        handle = getattr(self, "_handle", None)
        if handle:
            self._native.close_database(handle)

    def _dispose(self) -> None:
        handle = getattr(self, "_handle", None)
        if handle:
            self._native.free_database(handle)
            self._handle = ctypes.c_void_p()

    def __del__(self) -> None:
        self._dispose()
