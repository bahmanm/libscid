"""Python bindings for libscid: high-performance chess library.

libscid provides standalone C++ and Python capabilities for chess game
representation, PGN parsing and generation, move tree traversal and editing,
FIDE tournament rule adjudication, position analysis, and fast database
querying.

Core Classes:
    - [`Game`][libscid.Game]: Root chess game aggregate managing metadata
      tags and the mainline/variation move tree.
    - [`Cursor`][libscid.Cursor]: Functional navigation and tree-editing
      cursor over game moves and variations.
    - [`Position`][libscid.Position]: Board state representation, FEN parsing,
      and legal move generation.
    - [`Database`][libscid.Database]: Persistent and in-memory chess database
      storage supporting fast header indexing.
    - [`Filter`][libscid.Filter]: Dynamic subset view and multi-criteria
      sorting of database games.
    - [`DatabaseFilters`][libscid.DatabaseFilters]: Factory and manager for
      database filter views.
    - [`DatabaseSearch`][libscid.DatabaseSearch]: Database query engine for
      header, board pattern, and position searches.
    - [`HeaderCriteria`][libscid.HeaderCriteria]: Structured search parameters
      for database header queries.
    - [`Nag`][libscid.Nag]: Numeric Annotation Glyphs and typographical
      evaluation symbols.
    - [`MoveMetadata`][libscid.MoveMetadata]: Move classification flags (check,
      checkmate, castling, promotion).
    - [`PgnOptions`][libscid.PgnOptions]: Customisable PGN export formatting
      configuration.
    - [`LibScidError`][libscid.LibScidError]: Base exception for libscid C ABI
      errors.
"""

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
