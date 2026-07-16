"""Python bindings for libscid."""

from ._cursor import Cursor
from ._game import Game
from ._native_errors import LibScidError
from ._pgn import PgnOptions
from ._position import Position

__version__ = "0.0.0"

__all__ = ["Cursor", "Game", "LibScidError", "PgnOptions", "Position", "__version__"]
