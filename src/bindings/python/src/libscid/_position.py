from __future__ import annotations

import ctypes

from ._native import NativeLibrary, load_library


class Position:
    _native: NativeLibrary
    _handle: ctypes.c_void_p

    def __init__(self):
        raise TypeError("Position objects are returned by libscid APIs")

    @classmethod
    def from_fen(cls, fen: str | bytes) -> Position:
        native = load_library()
        return cls._from_handle(native, native.create_position_from_fen(fen))

    @classmethod
    def _from_handle(cls, native: NativeLibrary, handle: ctypes.c_void_p) -> Position:
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
