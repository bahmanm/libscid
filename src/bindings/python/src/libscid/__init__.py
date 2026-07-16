"""Python bindings for libscid."""

from ._game import Game
from ._native_errors import LibScidError
from ._pgn import PgnOptions

__version__ = "0.0.0"

__all__ = ["Game", "LibScidError", "PgnOptions", "__version__"]
