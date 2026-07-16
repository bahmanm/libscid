"""Python bindings for libscid."""

from ._cursor import Cursor
from ._game import Game
from ._nag import Nag
from ._native import LibScidError
from ._pgn import PgnOptions
from ._position import Position

__version__ = "0.0.0"

__all__ = [
    "Cursor",
    "Game",
    "LibScidError",
    "Nag",
    "PgnOptions",
    "Position",
    "__version__",
]
