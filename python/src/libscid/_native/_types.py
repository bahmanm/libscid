from __future__ import annotations

import ctypes
from typing import Protocol

NativeProgressReportCallback = ctypes.CFUNCTYPE(
    None,
    ctypes.c_size_t,
    ctypes.c_size_t,
    ctypes.c_char_p,
    ctypes.c_void_p,
)

NativeShouldCancelFn = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_void_p)


class ScidMoveSpec(ctypes.Structure):
    _fields_ = [
        ("from_square", ctypes.c_uint),
        ("to_square", ctypes.c_uint),
        ("promotion", ctypes.c_uint),
        ("is_castling", ctypes.c_int),
    ]


class ScidSearchHeaderCriteria(ctypes.Structure):
    _fields_ = [
        ("player", ctypes.c_char_p),
        ("white", ctypes.c_char_p),
        ("black", ctypes.c_char_p),
        ("event", ctypes.c_char_p),
        ("site", ctypes.c_char_p),
        ("site_country", ctypes.c_char_p),
        ("round", ctypes.c_char_p),
        ("date_min", ctypes.c_char_p),
        ("date_max", ctypes.c_char_p),
        ("event_date_min", ctypes.c_char_p),
        ("event_date_max", ctypes.c_char_p),
        ("eco_min", ctypes.c_char_p),
        ("eco_max", ctypes.c_char_p),
        ("result", ctypes.c_char_p),
        ("game_number_min", ctypes.c_size_t),
        ("game_number_max", ctypes.c_size_t),
        ("halfmove_count_min", ctypes.c_size_t),
        ("halfmove_count_max", ctypes.c_size_t),
        ("white_elo_min", ctypes.c_size_t),
        ("white_elo_max", ctypes.c_size_t),
        ("black_elo_min", ctypes.c_size_t),
        ("black_elo_max", ctypes.c_size_t),
        ("elo_difference_min", ctypes.c_int),
        ("elo_difference_max", ctypes.c_int),
        ("has_variations", ctypes.c_int),
        ("has_comments", ctypes.c_int),
        ("has_nags", ctypes.c_int),
    ]


class ScidSearchBoardCriteria(ctypes.Structure):
    _fields_ = [
        ("position", ctypes.c_void_p),
        ("match", ctypes.c_int),
        ("include_variations", ctypes.c_int),
        ("include_flipped", ctypes.c_int),
    ]


class PgnOptionsProtocol(Protocol):
    @property
    def symbolic_nags(self) -> bool: ...

    @property
    def supplemental_tags(self) -> bool: ...

    @property
    def comments(self) -> bool: ...

    @property
    def variations(self) -> bool: ...

    @property
    def line_width(self) -> int | None: ...
