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
ShouldCancelFn = Callable[[], bool]
HeaderResult = str | Iterable[str] | None
BoardSearchMatch = Literal["exact", "pawns", "files"]

BOARD_MATCH_EXACT: BoardSearchMatch = "exact"
BOARD_MATCH_PAWNS: BoardSearchMatch = "pawns"
BOARD_MATCH_FILES: BoardSearchMatch = "files"

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
    HeaderCriteria = HeaderCriteria
    BOARD_MATCH_EXACT = BOARD_MATCH_EXACT
    BOARD_MATCH_PAWNS = BOARD_MATCH_PAWNS
    BOARD_MATCH_FILES = BOARD_MATCH_FILES

    _native: NativeLibrary
    _database: Database

    def __init__(self):
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
