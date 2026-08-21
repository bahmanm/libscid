"""Database game filter, subset views, and sorted row mapping."""

from __future__ import annotations

from typing import TYPE_CHECKING

from ._native import NativeLibrary

if TYPE_CHECKING:
    from ._database import Database


class Filter:
    """Subset view of games within a chess database.

    A `Filter` represents a filtered or selected subset of games within a
    [`Database`][libscid.Database]. Filters provide high-performance pagination,
    multi-criteria sorting, and bidirectional mapping between 0-based database
    game indices and sorted display row positions.

    Filters are obtained via
    [`DatabaseFilters.all_games`][libscid.DatabaseFilters.all_games],
    [`DatabaseFilters.primary`][libscid.DatabaseFilters.primary],
    [`DatabaseFilters.create()`][libscid.DatabaseFilters.create], or as search result
    destinations from [`DatabaseSearch`][libscid.DatabaseSearch].

    Examples:
        >>> import tempfile, pathlib, libscid
        >>> pgn = (
        ...     '[Event "E1"]\\n[White "W1"]\\n\\n1. e4 1-0\\n\\n'
        ...     '[Event "E2"]\\n[White "W2"]\\n\\n1. d4 1-0\\n'
        ... )
        >>> with tempfile.NamedTemporaryFile(
        ...     "w+", suffix=".pgn", delete=False
        ... ) as f:
        ...     _ = f.write(pgn)
        ...     f.flush()
        ...     path = f.name
        >>> database = libscid.Database.open_pgn_read_only(path)
        >>> all_games = database.filters.all_games
        >>> all_games.game_count
        2
        >>> all_games.get_game_indices(start_row=0, row_count=2)
        (0, 1)
        >>> database.close()
        >>> pathlib.Path(path).unlink()
    """

    _native: NativeLibrary
    _database: Database
    _id: int
    _owned: bool
    _deleted: bool

    def __init__(self):
        """Disallow direct filter instantiation.

        Raises:
            TypeError: Always raised if instantiated directly.
        """
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
        """Total number of games currently matched by this filter.

        Examples:
            >>> import tempfile, pathlib, libscid
            >>> pgn = (
            ...     '[Event "E1"]\\n\\n1. e4 1-0\\n\\n'
            ...     '[Event "E2"]\\n\\n1. d4 1-0\\n'
            ... )
            >>> with tempfile.NamedTemporaryFile(
            ...     "w+", suffix=".pgn", delete=False
            ... ) as f:
            ...     _ = f.write(pgn)
            ...     f.flush()
            ...     path = f.name
            >>> db = libscid.Database.open_pgn_read_only(path)
            >>> db.filters.all_games.game_count
            2
            >>> db.close()
            >>> pathlib.Path(path).unlink()
        """
        return self._native.database_filter_game_count(
            self._database._handle, self._available_id()
        )

    def get_game_indices(
        self,
        sort_criteria: str | bytes = "N+",
        start_row: int = 0,
        row_count: int | None = None,
    ) -> tuple[int, ...]:
        """Retrieve database game indices in sorted display row order.

        Args:
            sort_criteria: Sorting specification string (e.g. "N+" for game
                number, "D-" for descending date, "W+" for White player, "B+"
                for Black player, "E+" for ECO code). Defaults to "N+".
            start_row: 0-based starting row offset in the sorted view. Defaults
                to 0.
            row_count: Maximum number of game indices to retrieve. If None,
                retrieves all remaining games from `start_row`.

        Returns:
            A tuple of 0-based database game indices in sorted order.

        Raises:
            ValueError: If `start_row` or `row_count` is negative, or if the
                filter has been deleted.

        Examples:
            >>> import tempfile, pathlib, libscid
            >>> pgn = (
            ...     '[Event "E1"]\\n\\n1. e4 1-0\\n\\n'
            ...     '[Event "E2"]\\n\\n1. d4 1-0\\n\\n'
            ...     '[Event "E3"]\\n\\n1. c4 1-0\\n'
            ... )
            >>> with tempfile.NamedTemporaryFile(
            ...     "w+", suffix=".pgn", delete=False
            ... ) as f:
            ...     _ = f.write(pgn)
            ...     f.flush()
            ...     path = f.name
            >>> db = libscid.Database.open_pgn_read_only(path)
            >>> filter_view = db.filters.all_games
            >>> filter_view.get_game_indices("N+", start_row=1, row_count=2)
            (1, 2)
            >>> filter_view.get_game_indices()
            (0, 1, 2)
            >>> db.close()
            >>> pathlib.Path(path).unlink()
        """
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
        """Retrieve the database game index for a specific sorted display row.

        Args:
            row: 0-based display row index.
            sort_criteria: Sorting specification string. Defaults to "N+".

        Returns:
            The 0-based database game index at the specified row.

        Raises:
            ValueError: If `row` is negative or out of bounds, or if the filter
                has been deleted.

        Examples:
            >>> import tempfile, pathlib, libscid
            >>> pgn = (
            ...     '[Event "E1"]\\n\\n1. e4 1-0\\n\\n'
            ...     '[Event "E2"]\\n\\n1. d4 1-0\\n'
            ... )
            >>> with tempfile.NamedTemporaryFile(
            ...     "w+", suffix=".pgn", delete=False
            ... ) as f:
            ...     _ = f.write(pgn)
            ...     f.flush()
            ...     path = f.name
            >>> db = libscid.Database.open_pgn_read_only(path)
            >>> db.filters.all_games.get_game_index_at_row(1, "N+")
            1
            >>> db.close()
            >>> pathlib.Path(path).unlink()
        """
        self._check_non_negative("row", row)
        return self._native.database_filter_game_index_at_row(
            self._database._handle, self._available_id(), sort_criteria, row
        )

    def get_game_row_for_index(
        self, game_index: int, sort_criteria: str | bytes = "N+"
    ) -> int:
        """Find the sorted display row index for a specific database game index.

        Args:
            game_index: 0-based database game index.
            sort_criteria: Sorting specification string. Defaults to "N+".

        Returns:
            The 0-based display row position of the game in the sorted filter view.

        Raises:
            ValueError: If `game_index` is negative or not present in the filter,
                or if the filter has been deleted.

        Examples:
            >>> import tempfile, pathlib, libscid
            >>> pgn = (
            ...     '[Event "E1"]\\n\\n1. e4 1-0\\n\\n'
            ...     '[Event "E2"]\\n\\n1. d4 1-0\\n'
            ... )
            >>> with tempfile.NamedTemporaryFile(
            ...     "w+", suffix=".pgn", delete=False
            ... ) as f:
            ...     _ = f.write(pgn)
            ...     f.flush()
            ...     path = f.name
            >>> db = libscid.Database.open_pgn_read_only(path)
            >>> db.filters.all_games.get_game_row_for_index(1, "N+")
            1
            >>> db.close()
            >>> pathlib.Path(path).unlink()
        """
        self._check_non_negative("game_index", game_index)
        return self._native.database_filter_game_row_for_index(
            self._database._handle, self._available_id(), sort_criteria, game_index
        )

    def delete(self) -> None:
        """Delete this user-created filter and release its database resources.

        Raises:
            ValueError: If attempting to delete a built-in filter (such as
                `all_games` or `primary`) or if the filter has already been
                deleted.

        Examples:
            >>> import tempfile, pathlib, libscid
            >>> pgn = '[Event "E1"]\\n\\n1. e4 1-0\\n'
            >>> with tempfile.NamedTemporaryFile(
            ...     "w+", suffix=".pgn", delete=False
            ... ) as f:
            ...     _ = f.write(pgn)
            ...     f.flush()
            ...     path = f.name
            >>> db = libscid.Database.open_pgn_read_only(path)
            >>> custom = db.filters.create()
            >>> custom.game_count
            1
            >>> custom.delete()
            >>> db.close()
            >>> pathlib.Path(path).unlink()
        """
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
