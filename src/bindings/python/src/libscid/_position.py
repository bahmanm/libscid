from __future__ import annotations

import ctypes

from ._native import NativeLibrary


class Position:
    def __init__(self):
        raise TypeError("Position objects are returned by libscid APIs")

    @classmethod
    def _from_handle(cls, native: NativeLibrary, handle: ctypes.c_void_p) -> "Position":
        position = cls.__new__(cls)
        position._native = native
        position._handle = handle
        return position

    @property
    def fen(self) -> str:
        return self._native.position_to_fen(self._handle)

    def _dispose(self) -> None:
        handle = getattr(self, "_handle", None)
        if handle:
            self._native.free_position(self._handle)
            self._handle = ctypes.c_void_p()

    def __del__(self) -> None:
        self._dispose()
