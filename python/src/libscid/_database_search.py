"""Chess database search engine, multi-criteria header filters, and board matching."""

from __future__ import annotations

import ctypes
from collections.abc import Callable, Iterable
from dataclasses import dataclass
from typing import TYPE_CHECKING, Literal

from ._filter import Filter
from ._native import NativeLibrary
from ._native._constants import (
    SCID_BOARD_SEARCH_MATCH_EXACT,
    SCID_BOARD_SEARCH_MATCH_FILES,
    SCID_BOARD_SEARCH_MATCH_PAWNS,
    SCID_FILTER_ALL_GAMES,
)
from ._native._text import encode
from ._position import Position

if TYPE_CHECKING:
    from ._database import Database

ProgressReportCallback = Callable[[int, int, str | None], None]
"""Progress callback receiving `(done: int, total: int, message: str | None)`."""

ShouldCancelFn = Callable[[], bool]
"""Predicate callback returning `True` to request cooperative cancellation."""

HeaderResult = str | Iterable[str] | None
"""Permitted game result specifications (e.g. `'1-0'`, `('1-0', '0-1')`)."""

BoardSearchMatch = Literal["exact", "pawns", "files"]
"""Board pattern match strictness algorithm modes."""

BOARD_MATCH_EXACT: BoardSearchMatch = "exact"
"""Exact board piece placement: all pieces and pawns on identical squares."""

BOARD_MATCH_PAWNS: BoardSearchMatch = "pawns"
"""Pawn structure and material match: exact pawn placement and piece counts."""

BOARD_MATCH_FILES: BoardSearchMatch = "files"
"""Piece distribution match: identical piece count per vertical file."""

_BOARD_MATCH_TO_NATIVE = {
    BOARD_MATCH_EXACT: SCID_BOARD_SEARCH_MATCH_EXACT,
    BOARD_MATCH_PAWNS: SCID_BOARD_SEARCH_MATCH_PAWNS,
    BOARD_MATCH_FILES: SCID_BOARD_SEARCH_MATCH_FILES,
}

_TEXT_SETTERS = (
    ("player", "scid_search_header_criteria_player_set"),
    ("white", "scid_search_header_criteria_white_set"),
    ("black", "scid_search_header_criteria_black_set"),
    ("event", "scid_search_header_criteria_event_set"),
    ("site", "scid_search_header_criteria_site_set"),
    ("site_country", "scid_search_header_criteria_site_country_set"),
    ("round", "scid_search_header_criteria_round_set"),
)

_TEXT_RANGE_SETTERS = (
    ("date_min", "date_max", "scid_search_header_criteria_date_range_set"),
    (
        "event_date_min",
        "event_date_max",
        "scid_search_header_criteria_event_date_range_set",
    ),
    ("eco_min", "eco_max", "scid_search_header_criteria_eco_range_set"),
)

_UINT_RANGE_SETTERS = (
    (
        "game_number_min",
        "game_number_max",
        "scid_search_header_criteria_game_number_range_set",
    ),
    (
        "halfmove_count_min",
        "halfmove_count_max",
        "scid_search_header_criteria_halfmove_count_range_set",
    ),
    (
        "white_elo_min",
        "white_elo_max",
        "scid_search_header_criteria_white_elo_range_set",
    ),
    (
        "black_elo_min",
        "black_elo_max",
        "scid_search_header_criteria_black_elo_range_set",
    ),
)

_INT_RANGE_SETTERS = (
    (
        "elo_difference_min",
        "elo_difference_max",
        "scid_search_header_criteria_elo_difference_range_set",
    ),
)

_BOOL_SETTERS = (
    ("has_variations", "scid_search_header_criteria_has_variations_set"),
    ("has_comments", "scid_search_header_criteria_has_comments_set"),
    ("has_nags", "scid_search_header_criteria_has_nags_set"),
)


@dataclass
class HeaderCriteria:
    """Multi-criteria search parameters for chess game headers.

    Encapsulates text pattern filters, date/ECO ranges, rating limits, game
    lengths, tournament results, and structural movetext flags.

    Attributes:
        player: Substring match on either White or Black player name.
        white: Substring match on White player name.
        black: Substring match on Black player name.
        event: Substring match on event/tournament name.
        site: Substring match on venue/site name.
        site_country: Substring match on site country name or code.
        round: Substring match on round identifier.
        date_min: Minimum game date string (e.g. "1921.01.01").
        date_max: Maximum game date string (e.g. "1927.12.31").
        event_date_min: Minimum event date string.
        event_date_max: Maximum event date string.
        eco_min: Minimum ECO code classification (e.g. "B20").
        eco_max: Maximum ECO code classification (e.g. "B99").
        result: Desired game outcome (e.g. "1-0", "0-1", "1/2-1/2", "*") or an
            iterable of acceptable outcomes.
        game_number_min: Minimum 1-based game number in the database.
        game_number_max: Maximum 1-based game number in the database.
        halfmove_count_min: Minimum game length in halfmoves (ply).
        halfmove_count_max: Maximum game length in halfmoves (ply).
        white_elo_min: Minimum Elo rating for White.
        white_elo_max: Maximum Elo rating for White.
        black_elo_min: Minimum Elo rating for Black.
        black_elo_max: Maximum Elo rating for Black.
        elo_difference_min: Minimum Elo difference (`white_elo - black_elo`).
        elo_difference_max: Maximum Elo difference (`white_elo - black_elo`).
        has_variations: If True, matches only games with alternative variations.
        has_comments: If True, matches only games with text commentary.
        has_nags: If True, matches only games with Numeric Annotation Glyphs.

    Examples:
        >>> import libscid
        >>> criteria = libscid.HeaderCriteria(
        ...     white="Kasparov",
        ...     result="1-0",
        ...     eco_min="B80",
        ...     eco_max="B89",
        ...     has_comments=True,
        ... )
        >>> criteria.white
        'Kasparov'
        >>> criteria.result
        '1-0'
    """

    player: str | None = None
    white: str | None = None
    black: str | None = None
    event: str | None = None
    site: str | None = None
    site_country: str | None = None
    round: str | None = None

    date_min: str | None = None
    date_max: str | None = None
    event_date_min: str | None = None
    event_date_max: str | None = None

    eco_min: str | None = None
    eco_max: str | None = None

    result: HeaderResult = None

    game_number_min: int | None = None
    game_number_max: int | None = None
    halfmove_count_min: int | None = None
    halfmove_count_max: int | None = None

    white_elo_min: int | None = None
    white_elo_max: int | None = None
    black_elo_min: int | None = None
    black_elo_max: int | None = None
    elo_difference_min: int | None = None
    elo_difference_max: int | None = None

    has_variations: bool = False
    has_comments: bool = False
    has_nags: bool = False

    def _create_native(self, native: NativeLibrary) -> ctypes.c_void_p:
        handle = native.search_header_criteria_create()
        try:
            for field_name, setter_name in _TEXT_SETTERS:
                value = getattr(self, field_name)
                if value is not None:
                    native._check(
                        setter_name,
                        getattr(native._lib, setter_name)(handle, encode(value)),
                    )

            if self.result is not None:
                normalised = _normalise_result(self.result)
                if normalised is not None:
                    native._check(
                        "scid_search_header_criteria_result_set",
                        native._lib.scid_search_header_criteria_result_set(
                            handle, encode(normalised)
                        ),
                    )

            for min_field, max_field, setter_name in _TEXT_RANGE_SETTERS:
                min_val = getattr(self, min_field)
                max_val = getattr(self, max_field)
                if min_val is not None or max_val is not None:
                    native._check(
                        setter_name,
                        getattr(native._lib, setter_name)(
                            handle,
                            encode(min_val) if min_val is not None else None,
                            encode(max_val) if max_val is not None else None,
                        ),
                    )

            for min_field, max_field, setter_name in _UINT_RANGE_SETTERS:
                min_val = getattr(self, min_field)
                max_val = getattr(self, max_field)
                if min_val is not None or max_val is not None:
                    native._check(
                        setter_name,
                        getattr(native._lib, setter_name)(
                            handle,
                            _optional_unsigned_integer(min_field, min_val),
                            _optional_unsigned_integer(max_field, max_val),
                        ),
                    )

            for min_field, max_field, setter_name in _INT_RANGE_SETTERS:
                min_val = getattr(self, min_field)
                max_val = getattr(self, max_field)
                if min_val is not None or max_val is not None:
                    native._check(
                        setter_name,
                        getattr(native._lib, setter_name)(
                            handle,
                            _optional_signed_integer(min_field, min_val),
                            _optional_signed_integer(max_field, max_val),
                        ),
                    )

            for field_name, setter_name in _BOOL_SETTERS:
                val = getattr(self, field_name)
                if val:
                    native._check(
                        setter_name,
                        getattr(native._lib, setter_name)(
                            handle, _boolean(field_name, val)
                        ),
                    )
            return handle
        except Exception:
            native.search_header_criteria_free(handle)
            raise


class DatabaseSearch:
    """Chess database query and search engine.

    Executes fast header queries, exact position lookups via transposition
    hashing, and flexible board pattern searches against database filter
    subsets.

    Direct instantiation of `DatabaseSearch` is disallowed; instances are
    accessed via the [`Database.search`][libscid.Database.search] property.

    Examples:
        >>> import tempfile, pathlib, libscid
        >>> pgn = (
        ...     '[Event "E"]\\n[White "Fischer"]\\n[Black "Spassky"]\\n'
        ...     '[Result "1-0"]\\n\\n1. e4 e5 1-0\\n'
        ... )
        >>> with tempfile.NamedTemporaryFile(
        ...     "w+", suffix=".pgn", delete=False
        ... ) as f:
        ...     _ = f.write(pgn)
        ...     f.flush()
        ...     path = f.name
        >>> database = libscid.Database.open_pgn_read_only(path)
        >>> criteria = libscid.HeaderCriteria(white="Fischer", result="1-0")
        >>> matched_filter = database.search.headers(criteria)
        >>> matched_filter.game_count
        1
        >>> database.close()
        >>> pathlib.Path(path).unlink()
    """

    HeaderCriteria = HeaderCriteria
    BOARD_MATCH_EXACT = BOARD_MATCH_EXACT
    BOARD_MATCH_PAWNS = BOARD_MATCH_PAWNS
    BOARD_MATCH_FILES = BOARD_MATCH_FILES

    _native: NativeLibrary
    _database: Database

    def __init__(self):
        """Disallow direct search engine instantiation.

        Raises:
            TypeError: Always raised if instantiated directly.
        """
        raise TypeError("DatabaseSearch objects are returned by libscid APIs")

    @classmethod
    def _from_database(
        cls, native: NativeLibrary, database: Database
    ) -> DatabaseSearch:
        search = cls.__new__(cls)
        search._native = native
        search._database = database
        return search

    def headers(
        self,
        criteria: HeaderCriteria,
        *,
        source: Filter | None = None,
        destination: Filter | None = None,
        progress_report_callback: ProgressReportCallback | None = None,
        should_cancel_fn: ShouldCancelFn | None = None,
    ) -> Filter:
        """Execute a multi-criteria header query across a database filter subset.

        Scans game headers matching the criteria from the `source` filter,
        writing matching game indices into the `destination` filter.

        Args:
            criteria: [`HeaderCriteria`][libscid.HeaderCriteria] specifying
                header filters, rating ranges, results, and structural flags.
            source: Source [`Filter`][libscid.Filter] defining the search
                universe. If None, searches all games in the database.
            destination: Destination [`Filter`][libscid.Filter] to receive
                matching games. If None, a new filter is created.
            progress_report_callback: Optional progress callback receiving
                `(done, total, message)`.
            should_cancel_fn: Optional predicate returning True to request
                cooperative cancellation.

        Returns:
            The `destination` [`Filter`][libscid.Filter] containing matching
                game indices.

        Raises:
            TypeError: If `criteria` is not a `HeaderCriteria` or filter
                parameters are invalid.
            ValueError: If `destination` is the `all_games` filter or belongs to
                a different database.
            LibScidError: If search execution fails or is cancelled.

        Examples:
            >>> import tempfile, pathlib, libscid
            >>> pgn = (
            ...     '[Event "E1"]\\n[White "Fischer"]\\n[Result "1-0"]\\n\\n'
            ...     '1. e4 1-0\\n\\n'
            ...     '[Event "E2"]\\n[White "Spassky"]\\n[Result "0-1"]\\n\\n'
            ...     '1. d4 0-1\\n'
            ... )
            >>> with tempfile.NamedTemporaryFile(
            ...     "w+", suffix=".pgn", delete=False
            ... ) as f:
            ...     _ = f.write(pgn)
            ...     f.flush()
            ...     path = f.name
            >>> db = libscid.Database.open_pgn_read_only(path)
            >>> criteria = libscid.HeaderCriteria(white="Fischer")
            >>> result_filter = db.search.headers(criteria)
            >>> result_filter.game_count
            1
            >>> result_filter.get_game_indices()
            (0,)
            >>> db.close()
            >>> pathlib.Path(path).unlink()
        """
        if not isinstance(criteria, HeaderCriteria):
            raise TypeError("criteria must be a HeaderCriteria")

        source_filter = (
            self._database.filters.all_games
            if source is None
            else self._validate_filter("source", source)
        )
        destination_filter = (
            self._database.filters.create()
            if destination is None
            else self._validate_filter("destination", destination)
        )

        if destination_filter._id == SCID_FILTER_ALL_GAMES:
            raise ValueError("destination cannot be the all_games filter")

        native_criteria = criteria._create_native(self._native)
        try:
            self._native.database_search_headers(
                self._database._handle,
                source_filter._available_id(),
                destination_filter._available_id(),
                native_criteria,
                progress_report_callback=progress_report_callback,
                should_cancel_fn=should_cancel_fn,
            )
        finally:
            self._native.search_header_criteria_free(native_criteria)
        return destination_filter

    def position(
        self,
        position: Position,
        *,
        source: Filter | None = None,
        destination: Filter | None = None,
        progress_report_callback: ProgressReportCallback | None = None,
        should_cancel_fn: ShouldCancelFn | None = None,
    ) -> Filter:
        """Search for games reaching an exact board position snapshot.

        Performs fast transposition hash lookup across games in the `source`
        filter, writing matches into `destination`.

        Args:
            position: Target [`Position`][libscid.Position] to find.
            source: Source [`Filter`][libscid.Filter] defining the search subset.
                If None, searches all games in the database.
            destination: Destination [`Filter`][libscid.Filter] receiving
                matching games. If None, a new filter is created.
            progress_report_callback: Optional progress callback.
            should_cancel_fn: Optional cooperative cancellation predicate.

        Returns:
            The `destination` [`Filter`][libscid.Filter] containing matching
                game indices.

        Raises:
            TypeError: If `position` is not a `Position`.
            ValueError: If `destination` is the `all_games` filter or belongs to
                a different database.
            LibScidError: If search execution fails or is cancelled.

        Examples:
            >>> import tempfile, pathlib, libscid
            >>> pgn = (
            ...     '[Event "E1"]\\n\\n1. e4 e5 1-0\\n\\n'
            ...     '[Event "E2"]\\n\\n1. d4 d5 1-0\\n'
            ... )
            >>> with tempfile.NamedTemporaryFile(
            ...     "w+", suffix=".pgn", delete=False
            ... ) as f:
            ...     _ = f.write(pgn)
            ...     f.flush()
            ...     path = f.name
            >>> db = libscid.Database.open_pgn_read_only(path)
            >>> fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1"
            >>> pos = libscid.Position.from_fen(fen)
            >>> matched_filter = db.search.position(pos)
            >>> matched_filter.game_count
            1
            >>> matched_filter.get_game_indices()
            (0,)
            >>> db.close()
            >>> pathlib.Path(path).unlink()
        """
        if not isinstance(position, Position):
            raise TypeError("position must be a Position")

        source_filter = (
            self._database.filters.all_games
            if source is None
            else self._validate_filter("source", source)
        )
        destination_filter = (
            self._database.filters.create()
            if destination is None
            else self._validate_filter("destination", destination)
        )

        if destination_filter._id == SCID_FILTER_ALL_GAMES:
            raise ValueError("destination cannot be the all_games filter")

        self._native.database_search_position(
            self._database._handle,
            source_filter._available_id(),
            destination_filter._available_id(),
            position._handle,
            progress_report_callback=progress_report_callback,
            should_cancel_fn=should_cancel_fn,
        )
        return destination_filter

    def board(
        self,
        position: Position,
        *,
        match: BoardSearchMatch = BOARD_MATCH_EXACT,
        source: Filter | None = None,
        destination: Filter | None = None,
        include_variations: bool = False,
        include_flipped: bool = False,
        progress_report_callback: ProgressReportCallback | None = None,
        should_cancel_fn: ShouldCancelFn | None = None,
    ) -> Filter:
        """Search for games matching a board configuration or material pattern.

        Args:
            position: Target [`Position`][libscid.Position] layout to match.
            match: Matching mode algorithm: "exact" (identical piece squares),
                "pawns" (identical pawn structure and piece count balance), or
                "files" (identical piece counts per file). Defaults to "exact".
            source: Source [`Filter`][libscid.Filter] defining the search subset.
                If None, searches all games in the database.
            destination: Destination [`Filter`][libscid.Filter] receiving
                matches. If None, a new filter is created.
            include_variations: Whether to search alternative variation branches
                in addition to the mainline. Defaults to False.
            include_flipped: Whether to also match colour-flipped board
                positions (White and Black swapped). Defaults to False.
            progress_report_callback: Optional progress callback.
            should_cancel_fn: Optional cooperative cancellation predicate.

        Returns:
            The `destination` [`Filter`][libscid.Filter] containing matching
                game indices.

        Raises:
            TypeError: If `position` is not a `Position`.
            ValueError: If `match` is invalid or `destination` is the `all_games`
                filter.
            LibScidError: If search execution fails or is cancelled.

        Examples:
            >>> import tempfile, pathlib, libscid
            >>> pgn = (
            ...     '[Event "E1"]\\n\\n1. e4 e5 1-0\\n\\n'
            ...     '[Event "E2"]\\n\\n1. d4 d5 1-0\\n'
            ... )
            >>> with tempfile.NamedTemporaryFile(
            ...     "w+", suffix=".pgn", delete=False
            ... ) as f:
            ...     _ = f.write(pgn)
            ...     f.flush()
            ...     path = f.name
            >>> db = libscid.Database.open_pgn_read_only(path)
            >>> fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1"
            >>> pos = libscid.Position.from_fen(fen)
            >>> matched_filter = db.search.board(pos, match="exact")
            >>> matched_filter.game_count
            1
            >>> matched_filter.get_game_indices()
            (0,)
            >>> db.close()
            >>> pathlib.Path(path).unlink()
        """
        if not isinstance(position, Position):
            raise TypeError("position must be a Position")
        native_match = _native_board_match(match)
        include_variations = _boolean("include_variations", include_variations) != 0
        include_flipped = _boolean("include_flipped", include_flipped) != 0

        source_filter = (
            self._database.filters.all_games
            if source is None
            else self._validate_filter("source", source)
        )
        destination_filter = (
            self._database.filters.create()
            if destination is None
            else self._validate_filter("destination", destination)
        )

        if destination_filter._id == SCID_FILTER_ALL_GAMES:
            raise ValueError("destination cannot be the all_games filter")

        criteria = self._native.search_board_criteria_create()
        try:
            self._native._check(
                "scid_search_board_criteria_position_set",
                self._native._lib.scid_search_board_criteria_position_set(
                    criteria, position._handle
                ),
            )
            self._native._check(
                "scid_search_board_criteria_match_set",
                self._native._lib.scid_search_board_criteria_match_set(
                    criteria, native_match
                ),
            )
            self._native._check(
                "scid_search_board_criteria_include_variations_set",
                self._native._lib.scid_search_board_criteria_include_variations_set(
                    criteria, int(include_variations)
                ),
            )
            self._native._check(
                "scid_search_board_criteria_include_flipped_set",
                self._native._lib.scid_search_board_criteria_include_flipped_set(
                    criteria, int(include_flipped)
                ),
            )
            self._native.database_search_board(
                self._database._handle,
                source_filter._available_id(),
                destination_filter._available_id(),
                criteria,
                progress_report_callback=progress_report_callback,
                should_cancel_fn=should_cancel_fn,
            )
        finally:
            self._native.search_board_criteria_free(criteria)
        return destination_filter

    def _validate_filter(self, name: str, filter_: Filter) -> Filter:
        if not isinstance(filter_, Filter):
            raise TypeError(f"{name} must be a Filter")
        if filter_._database is not self._database:
            raise ValueError(f"{name} filter belongs to a different database")
        filter_._available_id()
        return filter_


def _encode_optional_text(
    name: str, value: str | None, keepalive: list[bytes]
) -> bytes | None:
    if value is None or value == "":
        return None
    if not isinstance(value, str):
        raise TypeError(f"{name} must be str or None")
    encoded = encode(value)
    keepalive.append(encoded)
    return encoded


def _normalise_result(value: HeaderResult) -> str | None:
    if value is None:
        return None
    if isinstance(value, str):
        return value
    if not isinstance(value, Iterable):
        raise TypeError("result must be str, an iterable of str, or None")

    parts = []
    for item in value:
        if not isinstance(item, str):
            raise TypeError("result items must be str")
        parts.append(item)
    return ", ".join(parts)


def _native_board_match(value: BoardSearchMatch) -> int:
    if not isinstance(value, str):
        raise TypeError("match must be str")
    try:
        return _BOARD_MATCH_TO_NATIVE[value]
    except KeyError:
        raise ValueError("match must be 'exact', 'pawns', or 'files'") from None


def _optional_unsigned_integer(name: str, value: int | None) -> int:
    if value is None:
        return 0
    if isinstance(value, bool) or not isinstance(value, int):
        raise TypeError(f"{name} must be int or None")
    if value < 0:
        raise ValueError(f"{name} must be non-negative")
    return value


def _optional_signed_integer(name: str, value: int | None) -> int:
    if value is None:
        return 0
    if isinstance(value, bool) or not isinstance(value, int):
        raise TypeError(f"{name} must be int or None")
    return value


def _boolean(name: str, value: bool) -> int:
    if not isinstance(value, bool):
        raise TypeError(f"{name} must be bool")
    return int(value)
