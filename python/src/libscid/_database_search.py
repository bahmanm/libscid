from __future__ import annotations

from collections.abc import Callable, Iterable
from dataclasses import dataclass
from typing import TYPE_CHECKING

from ._filter import Filter
from ._native import NativeLibrary
from ._native._constants import SCID_FILTER_ALL_GAMES
from ._native._text import encode
from ._native._types import ScidSearchHeaderCriteria
from ._position import Position

if TYPE_CHECKING:
    from ._database import Database

ProgressReportCallback = Callable[[int, int, str | None], None]
ShouldCancelFn = Callable[[], bool]
HeaderResult = str | Iterable[str] | None

_STRING_FIELDS = (
    "player",
    "white",
    "black",
    "event",
    "site",
    "site_country",
    "round",
    "date_min",
    "date_max",
    "event_date_min",
    "event_date_max",
    "eco_min",
    "eco_max",
)

_UNSIGNED_INTEGER_FIELDS = (
    "game_number_min",
    "game_number_max",
    "halfmove_count_min",
    "halfmove_count_max",
    "white_elo_min",
    "white_elo_max",
    "black_elo_min",
    "black_elo_max",
)

_SIGNED_INTEGER_FIELDS = (
    "elo_difference_min",
    "elo_difference_max",
)

_BOOLEAN_FIELDS = (
    "has_variations",
    "has_comments",
    "has_nags",
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

    def _to_native(self) -> tuple[ScidSearchHeaderCriteria, list[bytes]]:
        keepalive: list[bytes] = []
        native = ScidSearchHeaderCriteria()

        for field_name in _STRING_FIELDS:
            setattr(
                native,
                field_name,
                _encode_optional_text(field_name, getattr(self, field_name), keepalive),
            )

        native.result = _encode_optional_text(
            "result", _normalise_result(self.result), keepalive
        )

        for field_name in _UNSIGNED_INTEGER_FIELDS:
            setattr(
                native,
                field_name,
                _optional_unsigned_integer(field_name, getattr(self, field_name)),
            )

        for field_name in _SIGNED_INTEGER_FIELDS:
            setattr(
                native,
                field_name,
                _optional_signed_integer(field_name, getattr(self, field_name)),
            )

        for field_name in _BOOLEAN_FIELDS:
            setattr(native, field_name, _boolean(field_name, getattr(self, field_name)))

        return native, keepalive


class DatabaseSearch:
    HeaderCriteria = HeaderCriteria

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

        native_criteria, _keepalive = criteria._to_native()
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

        self._native.database_search_headers(
            self._database._handle,
            source_filter._available_id(),
            destination_filter._available_id(),
            native_criteria,
            progress_report_callback=progress_report_callback,
            should_cancel_fn=should_cancel_fn,
        )
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
