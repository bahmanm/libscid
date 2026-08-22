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
    - [`Arbiter`][libscid.Arbiter]: Tournament rules and draw claim arbiter.
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
    - [`iter_movetext`][libscid.iter_movetext]: Hierarchical movetext event
      stream iterator.
    - [`LibScidError`][libscid.LibScidError]: Base exception for libscid C ABI
      errors.

Examples:
    >>> import libscid
    >>> game = libscid.Game()
    >>> game.set_tag("White", "Kasparov, Garry")
    >>> game.set_tag("Black", "Karpov, Anatoly")
    >>> game.set_tag("Result", "1-0")
    >>> cursor = game.create_cursor()
    >>> cursor = cursor.append_move("e4")
    >>> cursor = cursor.append_move("e5")
    >>> cursor = cursor.append_move("Nf3")
    >>> cursor.position.side_to_move
    'black'
    >>> cursor.position.is_check
    False
    >>> "1.e4 e5 2.Nf3" in game.to_pgn()
    True
"""

from importlib.metadata import PackageNotFoundError, version

from ._arbiter import Arbiter
from ._cursor import Cursor
from ._database import Database
from ._database_filters import DatabaseFilters
from ._database_search import DatabaseSearch, HeaderCriteria
from ._domain_support._movetext_iteration import (
    MovetextEvent,
    MovetextLineEnd,
    MovetextLineStart,
    MovetextMove,
    iter_movetext,
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
    "Arbiter",
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
    "iter_movetext",
]
