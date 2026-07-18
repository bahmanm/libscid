from __future__ import annotations

import os

from ._cursors import NativeCursorMixin
from ._errors import LibScidError
from ._games import NativeGameMixin
from ._primitives import NativePrimitiveMixin


class NativeLibrary(
    NativeCursorMixin,
    NativeGameMixin,
    NativePrimitiveMixin,
):
    pass


def load_library(library_path: str | os.PathLike[str] | None = None) -> NativeLibrary:
    return NativeLibrary(library_path)


__all__ = ["LibScidError", "NativeLibrary", "load_library"]
