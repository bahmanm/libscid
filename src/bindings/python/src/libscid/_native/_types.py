from __future__ import annotations

import ctypes
from typing import Protocol


class ScidMoveSpec(ctypes.Structure):
    _fields_ = [
        ("from_square", ctypes.c_uint),
        ("to_square", ctypes.c_uint),
        ("promotion", ctypes.c_uint),
        ("is_castling", ctypes.c_int),
    ]


class PgnOptionsProtocol(Protocol):
    symbolic_nags: bool
    supplemental_tags: bool
    comments: bool
    variations: bool
    line_width: int | None
