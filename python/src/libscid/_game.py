"""Chess game aggregate entity and PGN serialisation."""

from __future__ import annotations

import ctypes
from collections.abc import Iterator
from typing import TYPE_CHECKING

from ._cursor import Cursor
from ._native import NativeLibrary, load_library
from ._pgn import PgnOptions
from ._position import Position

if TYPE_CHECKING:
    from ._domain_support._movetext_iteration import MovetextEvent


class Game:
    """Chess game entity encapsulating metadata tags and hierarchical movetext.

    A `Game` represents a complete chess contest, including the Seven Tag
    Roster (STR) metadata (`Event`, `Site`, `Date`, `Round`, `White`, `Black`,
    `Result`), supplemental PGN header tags, the initial board starting
    position, and the hierarchical movetext tree (mainline moves, alternative
    variations, NAG annotations, and commentary).

    Game tree traversal and editing are performed by creating a
    [`Cursor`][libscid.Cursor] via [`create_cursor()`][libscid.Game.create_cursor].

    Examples:
        >>> import libscid
        >>> game = libscid.Game.from_pgn(
        ...     '[Event "World Championship"]\\n'
        ...     '[White "Kasparov, Garry"]\\n'
        ...     '[Black "Karpov, Anatoly"]\\n'
        ...     '[Result "1-0"]\\n\\n'
        ...     '1. e4 e5 2. Nf3 Nc6 3. Bb5 1-0'
        ... )
        >>> game.get_tag("White")
        'Kasparov, Garry'
        >>> game.mainline_move_count
        5
        >>> cursor = game.create_cursor()
        >>> cursor.next().previous_move_san
        'e4'
    """

    _native: NativeLibrary
    _handle: ctypes.c_void_p

    def __init__(self, position: Position | None = None):
        """Initialise a blank chess game.

        Initialises standard default PGN tags (`Event`, `Site`, `Date`,
        `Round`, `White`, `Black`, `Result`). If a custom starting `position`
        is supplied, a `FEN` tag is automatically attached.

        Args:
            position: Optional custom starting [`Position`][libscid.Position].
                If omitted or None, the standard chess starting board setup is
                used.

        Examples:
            >>> import libscid
            >>> game = libscid.Game()
            >>> game.get_tag("Result")
            '*'
            >>> pos = libscid.Position.from_fen("4k3/8/8/8/8/8/8/4K2R w K - 0 1")
            >>> custom_game = libscid.Game(pos)
            >>> custom_game.get_tag("FEN")
            '4k3/8/8/8/8/8/8/4K2R w K - 0 1'
        """
        if position is None:
            self._native = load_library()
            self._handle = self._native.create_blank_game()
            return

        self._native = position._native
        self._handle = self._native.create_blank_game(position._handle)

    @classmethod
    def from_pgn(cls, pgn: str | bytes, position: Position | None = None) -> Game:
        """Create a game by parsing PGN text.

        Parses PGN header tags, move notation, nested variations, comments,
        and Numeric Annotation Glyphs (NAGs) from the supplied string buffer.

        Args:
            pgn: PGN text string or bytes to parse.
            position: Optional custom starting [`Position`][libscid.Position]
                context for games starting from a non-standard setup.

        Returns:
            A newly allocated [`Game`][libscid.Game] instance.

        Raises:
            LibScidError: If the PGN syntax is invalid, illegal moves are
                encountered, or parsing fails.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. d4 d5 2. c4 e6 *")
            >>> game.mainline_move_count
            4
        """
        if position is None:
            native = load_library()
            return cls._from_handle(native, native.create_game_from_pgn(pgn))

        native = position._native
        return cls._from_handle(
            native, native.create_game_from_pgn(pgn, position._handle)
        )

    @classmethod
    def _from_handle(cls, native: NativeLibrary, handle: ctypes.c_void_p) -> Game:
        game = cls.__new__(cls)
        game._native = native
        game._handle = handle
        return game

    @property
    def mainline_move_count(self) -> int:
        """Total number of halfmoves (ply) in the mainline of the game."""
        return self._native.game_mainline_move_count(self._handle)

    @property
    def start_position(self) -> Position:
        """Initial board state at the start of the game."""
        return Position._from_handle(
            self._native, self._native.game_start_position(self._handle)
        )

    @property
    def end_position(self) -> Position:
        """Final board state at the conclusion of the mainline."""
        return Position._from_handle(
            self._native, self._native.game_final_position(self._handle)
        )

    def get_tag(self, name: str | bytes) -> str:
        """Retrieve the value of a PGN header tag by name.

        Queries Seven Tag Roster (STR) headers, well-known supplemental tags
        (e.g. "ECO", "FEN"), and custom tags.

        Args:
            name: PGN tag header name (e.g. "White", "ECO", "Event").

        Returns:
            The tag value string, or an empty string if the tag is absent.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn('[White "Tal, Mikhail"] 1. e4 *')
            >>> game.get_tag("White")
            'Tal, Mikhail'
            >>> game.get_tag("Annotator")
            ''
        """
        return self._native.game_get_tag(self._handle, name)

    def set_tag(self, name: str | bytes, value: str | bytes) -> None:
        """Set or update the value of a PGN header tag.

        Args:
            name: PGN tag header name.
            value: Value string to assign to the tag.

        Raises:
            LibScidError: If the tag value is invalid (e.g. malformed "Result").

        Examples:
            >>> import libscid
            >>> game = libscid.Game()
            >>> game.set_tag("White", "Spassky, Boris")
            >>> game.set_tag("WhiteElo", "2660")
            >>> game.get_tag("WhiteElo")
            '2660'
        """
        self._native.game_set_tag(self._handle, name, value)

    def remove_tag(self, name: str | bytes) -> bool:
        """Remove a supplemental PGN header tag by name.

        Standard mandatory Seven Tag Roster (STR) tags and the `FEN` tag
        cannot be removed.

        Args:
            name: PGN tag header name to remove.

        Returns:
            True if the tag was present and successfully removed; False if the
                tag was not found or is non-removable.

        Examples:
            >>> import libscid
            >>> game = libscid.Game()
            >>> game.set_tag("Annotator", "Fischer")
            >>> game.remove_tag("Annotator")
            True
            >>> game.remove_tag("White")
            False
        """
        return self._native.game_remove_tag(self._handle, name)

    def get_tags(self) -> tuple[tuple[str, str], ...]:
        """Retrieve all PGN header tags present in the game.

        Returns:
            A tuple of `(name, value)` string pairs representing all tags.

        Examples:
            >>> import libscid
            >>> game = libscid.Game()
            >>> dict(game.get_tags())["Result"]
            '*'
        """
        return self._native.game_get_tags(self._handle)

    def create_cursor(self) -> Cursor:
        """Create a new cursor initialised at the game's starting position.

        Returns:
            A new [`Cursor`][libscid.Cursor] positioned at the start of the
                game.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 *")
            >>> cursor = game.create_cursor()
            >>> cursor.next().previous_move_san
            'e4'
        """
        return Cursor._from_handle(
            self._native, self, self._native.game_create_cursor(self._handle)
        )

    def iter_movetext(self, *, variations: bool = True) -> Iterator[MovetextEvent]:
        """Iterate over hierarchical movetext events from the game start.

        Args:
            variations: Whether to recursively traverse nested variation branches.
                Defaults to True.

        Returns:
            An iterator yielding [`MovetextEvent`][libscid.MovetextEvent]
                instances.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 (1. d4 d5) 1... e5 *")
            >>> moves = [
            ...     e.san for e in game.iter_movetext()
            ...     if isinstance(e, libscid.MovetextMove)
            ... ]
            >>> moves
            ['e4', 'd4', 'd5', 'e5']
        """
        return self.create_cursor().iter_movetext(variations=variations)

    def to_pgn(self, options: PgnOptions | None = None) -> str:
        """Serialise the game to a standard PGN-formatted string.

        Args:
            options: Optional [`PgnOptions`][libscid.PgnOptions] specifying
                formatting controls such as line wrapping, NAG notation,
                comments, and variation inclusion.

        Returns:
            PGN-formatted string representing the game.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 $1 {King pawn} (1. c4) e5 *")
            >>> options = libscid.PgnOptions(
            ...     symbolic_nags=True,
            ...     variations=False,
            ... )
            >>> pgn = game.to_pgn(options)
            >>> "1.e4 ! {King pawn} 1...e5" in pgn
            True
        """
        return self._native.game_to_pgn(self._handle, options)

    def _dispose(self) -> None:
        handle = getattr(self, "_handle", None)
        if handle:
            self._native.free_game(self._handle)
            self._handle = ctypes.c_void_p()

    def __del__(self) -> None:
        self._dispose()
