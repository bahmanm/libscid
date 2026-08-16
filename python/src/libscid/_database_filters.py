from __future__ import annotations

from typing import TYPE_CHECKING

from ._filter import Filter
from ._native import NativeLibrary
from ._native._constants import SCID_FILTER_ALL_GAMES, SCID_FILTER_PRIMARY

if TYPE_CHECKING:
    from ._database import Database


class DatabaseFilters:
    _native: NativeLibrary
    _database: Database

    def __init__(self):
        raise TypeError("DatabaseFilters objects are returned by libscid APIs")

    @classmethod
    def _from_database(
        cls, native: NativeLibrary, database: Database
    ) -> DatabaseFilters:
        filters = cls.__new__(cls)
        filters._native = native
        filters._database = database
        return filters

    @property
    def all_games(self) -> Filter:
        return Filter._from_id(
            self._native, self._database, SCID_FILTER_ALL_GAMES, owned=False
        )

    @property
    def primary(self) -> Filter:
        return Filter._from_id(
            self._native, self._database, SCID_FILTER_PRIMARY, owned=False
        )

    def create(self) -> Filter:
        return Filter._from_id(
            self._native,
            self._database,
            self._native.database_filter_create(self._database._handle),
            owned=True,
        )
