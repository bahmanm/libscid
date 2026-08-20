"""Python bindings for libscid."""

from importlib.metadata import PackageNotFoundError, version

from ._cursor import Cursor
from ._database import Database
from ._database_filters import DatabaseFilters
from ._database_search import DatabaseSearch, HeaderCriteria
from ._domain_support._movetext_iteration import (
    MovetextEvent,
    MovetextLineEnd,
    MovetextLineStart,
    MovetextMove,
)
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
    "HeaderCriteria",
    "LibScidError",
    "MoveMetadata",
    "MovetextEvent",
    "MovetextLineEnd",
    "MovetextLineStart",
    "MovetextMove",
    "Nag",
    "PgnOptions",
    "Position",
    "__version__",
]
