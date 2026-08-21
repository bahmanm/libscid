"""Chess database container, PGN archives, and header indexing."""

from __future__ import annotations

import ctypes
import os
from collections.abc import Callable

from ._database_filters import DatabaseFilters
from ._database_search import DatabaseSearch
from ._game import Game
from ._native import NativeLibrary, load_library

ProgressReportCallback = Callable[[int, int, str | None], None]
"""Progress callback receiving `(done: int, total: int, message: str | None)`."""

ShouldCancelFn = Callable[[], bool]
"""Predicate callback returning `True` to request cooperative cancellation."""


class Database:
    """Chess game database supporting fast header indexing and subset queries.

    A `Database` manages an indexed collection of chess games, supporting
    high-speed header queries, tag extraction, game deserialisation, filter
    subset management via [`DatabaseFilters`][libscid.DatabaseFilters], and
    search execution via [`DatabaseSearch`][libscid.DatabaseSearch].

    Direct instantiation of `Database` is disallowed; instances are opened via
    factory class methods such as
    [`open_pgn_read_only()`][libscid.Database.open_pgn_read_only].

    Examples:
        >>> import libscid
        >>> database = libscid.Database.open_pgn_read_only("tournaments.pgn")
        >>> database.game_count
        42
        >>> database.get_tag(0, "White")
        'Capablanca, Jose Raul'
        >>> game = database.get_game(0)
        >>> game.mainline_move_count
        64
    """

    _native: NativeLibrary
    _handle: ctypes.c_void_p
    _filters: DatabaseFilters
    _search: DatabaseSearch

    def __init__(self):
        """Disallow direct database instantiation.

        Raises:
            TypeError: Always raised if instantiated directly.
        """
        raise TypeError("Database objects are returned by libscid APIs")

    @classmethod
    def open_pgn_read_only(
        cls,
        path: str | os.PathLike[str],
        progress_report_callback: ProgressReportCallback | None = None,
        should_cancel: ShouldCancelFn | None = None,
    ) -> Database:
        """Open a Portable Game Notation (.pgn) archive in read-only mode.

        Scans the PGN text archive, indexing game offsets and header metadata
        in memory for fast random access and searching.

        Args:
            path: Filesystem path to the PGN file.
            progress_report_callback: Optional callback receiving `(done, total,
                message)` progress updates during indexing.
            should_cancel: Optional predicate function returning True to
                request cooperative early cancellation.

        Returns:
            An opened read-only [`Database`][libscid.Database] instance.

        Raises:
            LibScidError: If the file cannot be opened, is malformed, or
                indexing is cancelled.
        """
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
        database._search = DatabaseSearch._from_database(native, database)
        return database

    @property
    def type(self) -> str:
        """Database backend format identifier (e.g. 'PGN', 'Scid5', 'Memory')."""
        return self._native.database_type(self._handle)

    @property
    def read_only(self) -> bool:
        """True if the database was opened in read-only mode."""
        return self._native.database_read_only(self._handle)

    @property
    def game_count(self) -> int:
        """Total number of games indexed in the database."""
        return self._native.database_game_count(self._handle)

    @property
    def filters(self) -> DatabaseFilters:
        """Filter manager for accessing and creating game subset views."""
        return self._filters

    @property
    def search(self) -> DatabaseSearch:
        """Query engine for executing header, board, and position searches."""
        return self._search

    def get_tag(self, index: int, name: str | bytes) -> str:
        """Retrieve a header tag value for a game directly from the index.

        Retrieves metadata (e.g. player names, event, date, ECO, result)
        without parsing the complete game movetext.

        Args:
            index: 0-based database game index.
            name: PGN tag header name (e.g. "White", "Date", "ECO").

        Returns:
            The tag value string, or an empty string if the tag is absent.

        Raises:
            LibScidError: If `index` is out of bounds or the database is closed.
        """
        return self._native.database_game_tag(self._handle, index, name)

    def get_game(self, index: int) -> Game:
        """Load and deserialise the full chess game at the specified index.

        Args:
            index: 0-based database game index.

        Returns:
            A newly allocated [`Game`][libscid.Game] instance containing all
            header tags, mainline moves, variation branches, and comments.

        Raises:
            LibScidError: If `index` is out of bounds or game deserialisation fails.
        """
        return Game._from_handle(
            self._native, self._native.database_game(self._handle, index)
        )

    def close(self) -> None:
        """Close database storage files and release resources."""
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
