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
