from __future__ import annotations

import ctypes
import os
from collections.abc import Callable
from typing import Any

from ._base import NativeLibraryBase
from ._constants import SCID_OK
from ._errors import LibScidError
from ._text import encode
from ._types import (
    NativeProgressReportCallback,
    NativeShouldCancelFn,
    ScidSearchHeaderCriteria,
)

ProgressReportCallback = Callable[[int, int, str | None], None]
ShouldCancelFn = Callable[[], bool]
RaiseCallbackException = Callable[[], None]


class NativeDatabaseMixin(NativeLibraryBase):
    def open_pgn_database_read_only(
        self,
        path: str | os.PathLike[str],
        progress_report_callback: ProgressReportCallback | None = None,
        should_cancel: ShouldCancelFn | None = None,
    ) -> ctypes.c_void_p:
        database = ctypes.c_void_p()
        callback_exception: BaseException | None = None

        def report_progress(
            done: int,
            total: int,
            message: bytes | None,
            _user_data: ctypes.c_void_p,
        ) -> None:
            nonlocal callback_exception
            if callback_exception is not None or progress_report_callback is None:
                return
            try:
                progress_report_callback(
                    done,
                    total,
                    None if message is None else message.decode("utf-8"),
                )
            except BaseException as exception:
                callback_exception = exception

        def cancellation_requested(_user_data: ctypes.c_void_p) -> int:
            nonlocal callback_exception
            if callback_exception is not None:
                return 1
            if should_cancel is None:
                return 0
            try:
                return int(bool(should_cancel()))
            except BaseException as exception:
                callback_exception = exception
                return 1

        progress_callback = (
            NativeProgressReportCallback(report_progress)
            if progress_report_callback is not None
            else NativeProgressReportCallback()
        )
        should_cancel_callback = (
            NativeShouldCancelFn(cancellation_requested)
            if progress_report_callback is not None or should_cancel is not None
            else NativeShouldCancelFn()
        )

        error = self._lib.scid_database_open_pgn_read_only(
            os.fsencode(path),
            progress_callback,
            None,
            should_cancel_callback,
            None,
            ctypes.byref(database),
        )
        if callback_exception is not None:
            if database:
                self.free_database(database)
            raise callback_exception
        if error != SCID_OK:
            raise LibScidError("scid_database_open_pgn_read_only", error)
        return database

    def close_database(self, database: ctypes.c_void_p) -> None:
        self._check("scid_database_close", self._lib.scid_database_close(database))

    def database_type(self, database: ctypes.c_void_p) -> str:
        return self._string_result("scid_database_type_get", database)

    def database_read_only(self, database: ctypes.c_void_p) -> bool:
        read_only = ctypes.c_int()
        self._check(
            "scid_database_read_only_get",
            self._lib.scid_database_read_only_get(database, ctypes.byref(read_only)),
        )
        return bool(read_only.value)

    def database_game_count(self, database: ctypes.c_void_p) -> int:
        count = ctypes.c_size_t()
        self._check(
            "scid_database_game_count_get",
            self._lib.scid_database_game_count_get(database, ctypes.byref(count)),
        )
        return count.value

    def database_filter_create(self, database: ctypes.c_void_p) -> int:
        filter_id = ctypes.c_int()
        self._check(
            "scid_database_filter_create",
            self._lib.scid_database_filter_create(database, ctypes.byref(filter_id)),
        )
        return filter_id.value

    def database_filter_delete(self, database: ctypes.c_void_p, filter_id: int) -> None:
        self._check(
            "scid_database_filter_delete",
            self._lib.scid_database_filter_delete(database, filter_id),
        )

    def database_filter_game_count(
        self, database: ctypes.c_void_p, filter_id: int
    ) -> int:
        count = ctypes.c_size_t()
        self._check(
            "scid_database_filter_game_count_get",
            self._lib.scid_database_filter_game_count_get(
                database, filter_id, ctypes.byref(count)
            ),
        )
        return count.value

    def database_filter_game_indices(
        self,
        database: ctypes.c_void_p,
        filter_id: int,
        sort_criteria: str | bytes,
        start_row: int,
        row_count: int,
    ) -> tuple[int, ...]:
        game_indices = (ctypes.c_size_t * row_count)()
        game_indices_count = ctypes.c_size_t()
        self._check(
            "scid_database_filter_game_indices_get",
            self._lib.scid_database_filter_game_indices_get(
                database,
                filter_id,
                encode(sort_criteria),
                start_row,
                row_count,
                game_indices,
                row_count,
                ctypes.byref(game_indices_count),
            ),
        )
        return tuple(game_indices[index] for index in range(game_indices_count.value))

    def database_filter_game_index_at_row(
        self,
        database: ctypes.c_void_p,
        filter_id: int,
        sort_criteria: str | bytes,
        row: int,
    ) -> int:
        game_index = ctypes.c_size_t()
        self._check(
            "scid_database_filter_game_index_at_row_get",
            self._lib.scid_database_filter_game_index_at_row_get(
                database,
                filter_id,
                encode(sort_criteria),
                row,
                ctypes.byref(game_index),
            ),
        )
        return game_index.value

    def database_filter_game_row_for_index(
        self,
        database: ctypes.c_void_p,
        filter_id: int,
        sort_criteria: str | bytes,
        game_index: int,
    ) -> int:
        row = ctypes.c_size_t()
        self._check(
            "scid_database_filter_game_row_for_index_get",
            self._lib.scid_database_filter_game_row_for_index_get(
                database,
                filter_id,
                encode(sort_criteria),
                game_index,
                ctypes.byref(row),
            ),
        )
        return row.value

    def database_search_headers(
        self,
        database: ctypes.c_void_p,
        source_filter_id: int,
        destination_filter_id: int,
        criteria: ScidSearchHeaderCriteria,
        progress_report_callback: ProgressReportCallback | None = None,
        should_cancel_fn: ShouldCancelFn | None = None,
    ) -> None:
        progress_callback, should_cancel_callback, raise_callback_exception = (
            self._database_search_callbacks(progress_report_callback, should_cancel_fn)
        )

        error = self._lib.scid_database_search_headers(
            database,
            source_filter_id,
            destination_filter_id,
            ctypes.byref(criteria),
            progress_callback,
            None,
            should_cancel_callback,
            None,
        )
        raise_callback_exception()
        self._check("scid_database_search_headers", error)

    def database_search_position(
        self,
        database: ctypes.c_void_p,
        source_filter_id: int,
        destination_filter_id: int,
        position: ctypes.c_void_p,
        progress_report_callback: ProgressReportCallback | None = None,
        should_cancel_fn: ShouldCancelFn | None = None,
    ) -> None:
        progress_callback, should_cancel_callback, raise_callback_exception = (
            self._database_search_callbacks(progress_report_callback, should_cancel_fn)
        )

        error = self._lib.scid_database_search_position(
            database,
            source_filter_id,
            destination_filter_id,
            position,
            progress_callback,
            None,
            should_cancel_callback,
            None,
        )
        raise_callback_exception()
        self._check("scid_database_search_position", error)

    def _database_search_callbacks(
        self,
        progress_report_callback: ProgressReportCallback | None,
        should_cancel_fn: ShouldCancelFn | None,
    ) -> tuple[Any, Any, RaiseCallbackException]:
        callback_exception: BaseException | None = None

        def report_progress(
            done: int,
            total: int,
            message: bytes | None,
            _user_data: ctypes.c_void_p,
        ) -> None:
            nonlocal callback_exception
            if callback_exception is not None or progress_report_callback is None:
                return
            try:
                progress_report_callback(
                    done,
                    total,
                    None if message is None else message.decode("utf-8"),
                )
            except BaseException as exception:
                callback_exception = exception

        def cancellation_requested(_user_data: ctypes.c_void_p) -> int:
            nonlocal callback_exception
            if callback_exception is not None:
                return 1
            if should_cancel_fn is None:
                return 0
            try:
                return int(bool(should_cancel_fn()))
            except BaseException as exception:
                callback_exception = exception
                return 1

        progress_callback = (
            NativeProgressReportCallback(report_progress)
            if progress_report_callback is not None
            else NativeProgressReportCallback()
        )
        should_cancel_callback = (
            NativeShouldCancelFn(cancellation_requested)
            if progress_report_callback is not None or should_cancel_fn is not None
            else NativeShouldCancelFn()
        )

        def raise_callback_exception() -> None:
            if callback_exception is not None:
                raise callback_exception

        return progress_callback, should_cancel_callback, raise_callback_exception

    def database_game_tag(
        self, database: ctypes.c_void_p, index: int, name: str | bytes
    ) -> str:
        return self._string_result(
            "scid_database_game_tag_get", database, index, encode(name)
        )

    def database_game(self, database: ctypes.c_void_p, index: int) -> ctypes.c_void_p:
        game = ctypes.c_void_p()
        self._check(
            "scid_database_game_get",
            self._lib.scid_database_game_get(
                database, index, ctypes.byref(game), None, 0, None
            ),
        )
        return game
