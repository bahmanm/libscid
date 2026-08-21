"""Database filter factory and predefined subset view manager."""

from __future__ import annotations

from typing import TYPE_CHECKING

from ._filter import Filter
from ._native import NativeLibrary
from ._native._constants import SCID_FILTER_ALL_GAMES, SCID_FILTER_PRIMARY

if TYPE_CHECKING:
    from ._database import Database


class DatabaseFilters:
    """Manager for chess database selection filters and subset views.

    Provides access to universal and primary predefined filters as well as
    allocation of custom user-defined filters for query execution, sorting, and
    game subset inspection.

    Direct instantiation of `DatabaseFilters` is disallowed; instances are
    accessed via the [`Database.filters`][libscid.Database.filters] property.

    Example:
        >>> import libscid
        >>> database = libscid.Database.open_pgn_read_only("games.pgn")
        >>> all_games = database.filters.all_games
        >>> custom_filter = database.filters.create()
    """

    _native: NativeLibrary
    _database: Database

    def __init__(self):
        """Disallow direct filter manager instantiation.

        Raises:
            TypeError: Always raised if instantiated directly.
        """
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
        """Universal built-in filter matching all games in the database."""
        return Filter._from_id(
            self._native, self._database, SCID_FILTER_ALL_GAMES, owned=False
        )

    @property
    def primary(self) -> Filter:
        """Primary working filter used for interactive search results."""
        return Filter._from_id(
            self._native, self._database, SCID_FILTER_PRIMARY, owned=False
        )

    def create(self) -> Filter:
        """Allocate and register a new, empty user filter in the database.

        Returns:
            A newly allocated user [`Filter`][libscid.Filter].
        """
        return Filter._from_id(
            self._native,
            self._database,
            self._native.database_filter_create(self._database._handle),
            owned=True,
        )
