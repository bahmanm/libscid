from __future__ import annotations

from typing import TYPE_CHECKING

from ._native import NativeLibrary

if TYPE_CHECKING:
    from ._database import Database


class Filter:
    _native: NativeLibrary
    _database: Database
    _id: int
    _owned: bool
    _deleted: bool

    def __init__(self):
        raise TypeError("Filter objects are returned by libscid APIs")

    @classmethod
    def _from_id(
        cls, native: NativeLibrary, database: Database, filter_id: int, *, owned: bool
    ) -> Filter:
        filter_ = cls.__new__(cls)
        filter_._native = native
        filter_._database = database
        filter_._id = filter_id
        filter_._owned = owned
        filter_._deleted = False
        return filter_

    @property
    def game_count(self) -> int:
        return self._native.database_filter_game_count(
            self._database._handle, self._available_id()
        )

    def get_game_indices(
        self,
        sort_criteria: str | bytes = "N+",
        start_row: int = 0,
        row_count: int | None = None,
    ) -> tuple[int, ...]:
        self._check_non_negative("start_row", start_row)
        if row_count is None:
            row_count = max(self.game_count - start_row, 0)
        self._check_non_negative("row_count", row_count)
        return self._native.database_filter_game_indices(
            self._database._handle,
            self._available_id(),
            sort_criteria,
            start_row,
            row_count,
        )

    def get_game_index_at_row(self, row: int, sort_criteria: str | bytes = "N+") -> int:
        self._check_non_negative("row", row)
        return self._native.database_filter_game_index_at_row(
            self._database._handle, self._available_id(), sort_criteria, row
        )

    def get_game_row_for_index(
        self, game_index: int, sort_criteria: str | bytes = "N+"
    ) -> int:
        self._check_non_negative("game_index", game_index)
        return self._native.database_filter_game_row_for_index(
            self._database._handle, self._available_id(), sort_criteria, game_index
        )

    def delete(self) -> None:
        if not self._owned:
            raise ValueError("built-in filters cannot be deleted")
        self._native.database_filter_delete(
            self._database._handle, self._available_id()
        )
        self._deleted = True

    def _available_id(self) -> int:
        if self._deleted:
            raise ValueError("filter has been deleted")
        return self._id

    @staticmethod
    def _check_non_negative(name: str, value: int) -> None:
        if value < 0:
            raise ValueError(f"{name} must be non-negative")
