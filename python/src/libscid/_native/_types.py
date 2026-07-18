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
