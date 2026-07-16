"""Python bindings for libscid."""

from ._game import Game
from ._native import LibScidError

__version__ = "0.0.0"

__all__ = ["Game", "LibScidError", "__version__"]
