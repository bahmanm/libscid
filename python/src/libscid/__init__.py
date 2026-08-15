"""Python bindings for libscid."""

from importlib.metadata import PackageNotFoundError, version

from ._cursor import Cursor
from ._database import Database
from ._database_filters import DatabaseFilters
from ._database_search import DatabaseSearch
from ._filter import Filter
from ._game import Game
from ._move_metadata import MoveMetadata
from ._nag import Nag
from ._native import LibScidError
from ._pgn import PgnOptions
from ._position import Position

try:
    __version__ = version("libscid")
except PackageNotFoundError:
    __version__ = "0.0.0"

__all__ = [
    "Cursor",
    "Database",
    "DatabaseFilters",
    "DatabaseSearch",
    "Filter",
    "Game",
    "LibScidError",
    "MoveMetadata",
    "Nag",
    "PgnOptions",
    "Position",
    "__version__",
]
