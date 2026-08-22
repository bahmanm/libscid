"""Hierarchical chess game movetext cursor and tree navigation."""

from __future__ import annotations

import ctypes
from collections.abc import Iterator
from typing import TYPE_CHECKING, Any

from ._arbiter import Arbiter
from ._nag import Nag
from ._native import NativeLibrary
from ._position import Position

if TYPE_CHECKING:
    from ._domain_support._movetext_iteration import MovetextEvent


class Cursor:
    """Navigational node within a hierarchical chess game movetext tree.

    A `Cursor` points to a specific ply within a [`Game`][libscid.Game],
    providing access to the board [`Position`][libscid.Position], incoming and
    departing moves, commentary, NAG annotations, and variation branching.

    Navigation methods (such as [`next()`][libscid.Cursor.next],
    [`previous()`][libscid.Cursor.previous], and
    [`enter_variation()`][libscid.Cursor.enter_variation]) return newly
    allocated cursor instances representing target locations, maintaining
    immutable value semantics during traversal. Tree mutation operations
    (such as [`append_move()`][libscid.Cursor.append_move] and
    [`set_comment()`][libscid.Cursor.set_comment]) modify the underlying game.

    Direct instantiation of `Cursor` is disallowed; instances must be obtained
    via [`Game.create_cursor()`][libscid.Game.create_cursor] or navigation
    methods.

    Examples:
        >>> import libscid
        >>> game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 *")
        >>> cursor = game.create_cursor()
        >>> cursor.is_line_start
        True
        >>> move1 = cursor.next()
        >>> move1.previous_move_san
        'e4'
        >>> move1.position.side_to_move
        'black'
    """

    _native: NativeLibrary
    _game: Any
    _handle: ctypes.c_void_p

    def __init__(self):
        """Disallow direct cursor instantiation.

        Raises:
            TypeError: Always raised if instantiated directly.
        """
        raise TypeError("Cursor objects are returned by libscid APIs")

    @classmethod
    def _from_handle(
        cls,
        native: NativeLibrary,
        game: Any,
        handle: ctypes.c_void_p,
    ) -> Cursor:
        cursor = cls.__new__(cls)
        cursor._native = native
        cursor._game = game
        cursor._handle = handle
        return cursor

    def clone(self) -> Cursor:
        """Duplicate this cursor at its current position.

        Returns:
            A new [`Cursor`][libscid.Cursor] pointing to the same node.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 *")
            >>> cursor = game.create_cursor().next()
            >>> cloned = cursor.clone()
            >>> cloned.previous_move_san
            'e4'
        """
        return self._from_handle(
            self._native,
            self._game,
            self._native.game_clone_cursor(self._game._handle, self._handle),
        )

    def next(self) -> Cursor | None:
        """Advance the cursor to the next move in the current line.

        Returns:
            A new [`Cursor`][libscid.Cursor] advanced by one ply, or None if
                the cursor is already at the end of the line.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 e5 *")
            >>> cursor = game.create_cursor()
            >>> cursor.next().previous_move_san
            'e4'
            >>> cursor.to_game_end().next() is None
            True
        """
        return self._from_optional_handle(self._native.cursor_next(self._handle))

    def previous(self) -> Cursor | None:
        """Step the cursor backward to the preceding move in the current line.

        Returns:
            A new [`Cursor`][libscid.Cursor] moved back by one ply, or None if
                the cursor is already at the start of the line.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 e5 *")
            >>> cursor = game.create_cursor().to_game_end()
            >>> cursor.previous().previous_move_san
            'e4'
            >>> game.create_cursor().previous() is None
            True
        """
        return self._from_optional_handle(self._native.cursor_previous(self._handle))

    def to_game_start(self) -> Cursor:
        """Move the cursor directly to the beginning of the mainline.

        Returns:
            A new [`Cursor`][libscid.Cursor] at the initial starting position
                of the game.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 *")
            >>> cursor = game.create_cursor().to_game_end()
            >>> cursor.to_game_start().is_line_start
            True
        """
        return self._from_handle(
            self._native,
            self._game,
            self._native.cursor_to_game_start(self._handle),
        )

    def to_game_end(self) -> Cursor:
        """Move the cursor directly to the terminal position of the mainline.

        Returns:
            A new [`Cursor`][libscid.Cursor] at the end of the mainline.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 *")
            >>> cursor = game.create_cursor()
            >>> cursor.to_game_end().is_line_end
            True
        """
        return self._from_handle(
            self._native,
            self._game,
            self._native.cursor_to_game_end(self._handle),
        )

    def to_main_line_offset(self, offset: int) -> Cursor | None:
        """Move the cursor to a specific 0-based ply offset on the mainline.

        Args:
            offset: Zero-based ply offset from the start of the mainline.

        Returns:
            A new [`Cursor`][libscid.Cursor] at the target ply offset, or None
                if the offset exceeds the mainline ply count.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 *")
            >>> cursor = game.create_cursor()
            >>> ply2 = cursor.to_main_line_offset(2)
            >>> ply2.previous_move_san
            'e5'
            >>> cursor.to_main_line_offset(10) is None
            True
        """
        return self._from_optional_handle(
            self._native.cursor_to_main_line_offset(self._handle, offset)
        )

    def enter_variation(self, index: int) -> Cursor | None:
        """Descend into a child variation branching from the upcoming move.

        Args:
            index: Zero-based index of the variation branch to enter.

        Returns:
            A new [`Cursor`][libscid.Cursor] at the start of the specified
                variation branch, or None if `index` is out of bounds or no
                variations exist.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 (1. d4 d5) 1... e5 *")
            >>> cursor = game.create_cursor()
            >>> var_cursor = cursor.enter_variation(0)
            >>> var_cursor.next().previous_move_san
            'd4'
            >>> var_cursor.variation_depth
            1
        """
        return self._from_optional_handle(
            self._native.cursor_enter_variation(self._handle, index)
        )

    def exit_variation(self) -> Cursor | None:
        """Ascend from the current variation back to its parent line.

        Returns:
            A new [`Cursor`][libscid.Cursor] positioned on the parent line
                where this variation branched, or None if the cursor is already
                on the mainline.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 (1. d4 d5) 1... e5 *")
            >>> var_cursor = game.create_cursor().enter_variation(0)
            >>> parent = var_cursor.exit_variation()
            >>> parent.variation_depth
            0
        """
        return self._from_optional_handle(
            self._native.cursor_exit_variation(self._handle)
        )

    def append_move(self, san: str | bytes) -> Cursor:
        """Append a move in Standard Algebraic Notation to the end of the line.

        The cursor must currently be located at the end of the line
        ([`is_line_end`][libscid.Cursor.is_line_end] must be True).

        Args:
            san: Standard Algebraic Notation move string (e.g. "e4", "Nf3").

        Returns:
            A new [`Cursor`][libscid.Cursor] positioned after the newly appended
                move.

        Raises:
            ValueError: If the cursor is not at the end of the line.
            LibScidError: If the move is illegal or cannot be parsed.

        Examples:
            >>> import libscid
            >>> game = libscid.Game()
            >>> cursor = game.create_cursor()
            >>> cursor = cursor.append_move("e4")
            >>> cursor.previous_move_san
            'e4'
            >>> cursor = cursor.append_move("e5")
            >>> cursor.previous_move_san
            'e5'
        """
        self._require_line_end("append_move")
        return self._from_handle(
            self._native,
            self._game,
            self._native.cursor_append_move(self._game._handle, self._handle, san),
        )

    def append_game(self, source_game: Any) -> Cursor:
        """Append all moves and variations from another game onto this line.

        The cursor must currently be located at the end of the line
        ([`is_line_end`][libscid.Cursor.is_line_end] must be True).

        Args:
            source_game: The [`Game`][libscid.Game] whose moves to append.

        Returns:
            A new [`Cursor`][libscid.Cursor] positioned at the end of the
                appended moves.

        Raises:
            ValueError: If the cursor is not at the end of the line.
            LibScidError: If the positions are incompatible or appending fails.

        Examples:
            >>> import libscid
            >>> game1 = libscid.Game.from_pgn("1. e4 e5 *")
            >>> game2 = libscid.Game.from_pgn(
            ...     "2. Nf3 Nc6 *", position=game1.end_position
            ... )
            >>> cursor = game1.create_cursor().to_game_end()
            >>> cursor = cursor.append_game(game2)
            >>> game1.mainline_move_count
            4
        """
        self._require_line_end("append_game")
        return self._from_handle(
            self._native,
            self._game,
            self._native.cursor_append_game(
                self._game._handle, self._handle, source_game._handle
            ),
        )

    def add_variation(self, preceding_comment: str | bytes = "") -> Cursor | None:
        """Add a new variation branch departing from the upcoming move.

        Args:
            preceding_comment: Optional introductory commentary text to attach
                to the start of the new variation branch.

        Returns:
            A new [`Cursor`][libscid.Cursor] at the start of the newly created
                variation, or None if the cursor is at the line end or the variation
                could not be created.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 e5 *")
            >>> cursor = game.create_cursor()
            >>> var_cursor = cursor.add_variation("Alternative opening")
            >>> var_cursor = var_cursor.append_move("d4")
            >>> var_cursor.previous_move_san
            'd4'
        """
        return self._from_optional_handle(
            self._native.cursor_add_variation(
                self._game._handle, self._handle, preceding_comment
            )
        )

    def remove_variation(self) -> Cursor | None:
        """Delete the current variation branch from the game tree.

        Returns:
            A new [`Cursor`][libscid.Cursor] on the parent line where the
                variation branched, or None if the cursor is on the mainline.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 (1. d4 d5) 1... e5 *")
            >>> var_cursor = game.create_cursor().enter_variation(0)
            >>> parent = var_cursor.remove_variation()
            >>> game.create_cursor().variation_count
            0
        """
        return self._from_optional_handle(
            self._native.cursor_remove_variation(self._game._handle, self._handle)
        )

    def promote_variation_to_first(self) -> Cursor | None:
        """Promote the current variation to become the first alternative sibling.

        Returns:
            A new [`Cursor`][libscid.Cursor] on the promoted variation, or None
                if the cursor is on the mainline.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 (1. d4) (1. c4) 1... e5 *")
            >>> var2 = game.create_cursor().enter_variation(1)
            >>> var2.next().previous_move_san
            'c4'
            >>> promoted = var2.promote_variation_to_first()
            >>> game.create_cursor().enter_variation(0).next().previous_move_san
            'c4'
        """
        return self._from_optional_handle(
            self._native.cursor_promote_variation_to_first(
                self._game._handle, self._handle
            )
        )

    def promote_variation_to_mainline(self) -> Cursor | None:
        """Promote the current variation to become the new mainline of the game.

        Swaps the current variation with the existing mainline from the branching
        point onward.

        Returns:
            A new [`Cursor`][libscid.Cursor] on the new mainline, or None if the
                cursor is already on the mainline.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 (1. d4 d5) 1... e5 *")
            >>> var = game.create_cursor().enter_variation(0)
            >>> new_main = var.promote_variation_to_mainline()
            >>> game.create_cursor().next().previous_move_san
            'd4'
        """
        return self._from_optional_handle(
            self._native.cursor_promote_variation_to_mainline(
                self._game._handle, self._handle
            )
        )

    def truncate(self) -> Cursor:
        """Remove all subsequent moves in the current line from this point.

        Returns:
            A new [`Cursor`][libscid.Cursor] at the newly truncated line end.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 *")
            >>> cursor = game.create_cursor().to_main_line_offset(2)
            >>> cursor.previous_move_san
            'e5'
            >>> truncated = cursor.truncate()
            >>> game.mainline_move_count
            2
        """
        return self._from_handle(
            self._native,
            self._game,
            self._native.cursor_truncate(self._game._handle, self._handle),
        )

    def truncate_before(self) -> Cursor:
        """Remove all preceding moves in the current line up to this point.

        Returns:
            A new [`Cursor`][libscid.Cursor] at the beginning of the truncated
                line.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 *")
            >>> cursor = game.create_cursor().to_main_line_offset(2)
            >>> start = cursor.truncate_before()
            >>> game.mainline_move_count
            2
            >>> start.next().previous_move_san
            'Nf3'
        """
        return self._from_handle(
            self._native,
            self._game,
            self._native.cursor_truncate_before(self._game._handle, self._handle),
        )

    def iter_movetext(self, *, variations: bool = True) -> Iterator[MovetextEvent]:
        """Iterate over movetext events starting from this cursor position.

        Args:
            variations: Whether to recursively traverse variation branches.
                Defaults to True.

        Returns:
            An iterator yielding [`MovetextEvent`][libscid.MovetextEvent]
                instances.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 (1. d4 d5) 1... e5 *")
            >>> moves = [
            ...     e.san for e in game.create_cursor().iter_movetext()
            ...     if isinstance(e, libscid.MovetextMove)
            ... ]
            >>> moves
            ['e4', 'd4', 'd5', 'e5']
        """
        from ._domain_support._movetext_iteration import iter_movetext

        return iter_movetext(self, variations=variations)

    def _from_optional_handle(self, handle: ctypes.c_void_p | None) -> Cursor | None:
        if handle is None:
            return None
        return self._from_handle(self._native, self._game, handle)

    def _require_line_end(self, method: str) -> None:
        if not self.is_line_end:
            raise ValueError(f"{method} requires cursor at line end")

    def _path_move_uci(self) -> tuple[str, ...]:
        cursor = self.clone()
        moves = []
        while True:
            move = cursor.previous_move_uci
            if move is not None:
                moves.append(move)
                previous = cursor.previous()
                if previous is None:
                    raise RuntimeError("Cursor previous move disappeared")
                cursor = previous
                continue

            parent = cursor.exit_variation()
            if parent is not None:
                cursor = parent
                continue

            return tuple(reversed(moves))

    @property
    def arbiter(self) -> Arbiter:
        """Retrieve an arbiter to evaluate tournament rules at this position.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 e5 *")
            >>> cursor = game.create_cursor().to_game_end()
            >>> cursor.arbiter.can_claim_fifty_move_rule
            False
        """
        return Arbiter(self)

    @property
    def previous_move_san(self) -> str | None:
        """Standard Algebraic Notation of the incoming move, or None at line start."""
        if self.is_line_start:
            return None
        return self._native.cursor_previous_move_san(self._handle)

    @property
    def next_move_san(self) -> str | None:
        """Standard Algebraic Notation of the upcoming move, or None at line end."""
        if self.is_line_end:
            return None
        return self._native.cursor_next_move_san(self._handle)

    @property
    def previous_move_uci(self) -> str | None:
        """Universal Chess Interface notation of the incoming move, or None at start."""
        if self.is_line_start:
            return None
        return self._native.cursor_previous_move_uci(self._handle)

    @property
    def next_move_uci(self) -> str | None:
        """Universal Chess Interface notation of the upcoming move, or None at end."""
        if self.is_line_end:
            return None
        return self._native.cursor_next_move_uci(self._handle)

    @property
    def previous_move_nags(self) -> tuple[Nag, ...] | None:
        """NAG annotations attached to the incoming move, or None at line start."""
        if self.is_line_start:
            return None
        return tuple(
            Nag(code) for code in self._native.cursor_previous_move_nags(self._handle)
        )

    @property
    def next_move_nags(self) -> tuple[Nag, ...] | None:
        """NAG annotations attached to the upcoming move, or None at line end."""
        if self.is_line_end:
            return None
        return tuple(
            Nag(code) for code in self._native.cursor_next_move_nags(self._handle)
        )

    @property
    def comment(self) -> str | None:
        """Commentary text attached to the incoming move, or None at line start."""
        if self.is_line_start:
            return None
        return self._native.cursor_comment(self._handle)

    @property
    def preceding_comment(self) -> str | None:
        """Introductory commentary preceding the first move, or None if not at start."""
        if not self.is_line_start:
            return None
        return self._native.cursor_comment(self._handle)

    def set_comment(self, comment: str | bytes) -> None:
        """Set commentary text at the current cursor node.

        Args:
            comment: Commentary text to attach.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 e5 *")
            >>> cursor = game.create_cursor().next()
            >>> cursor.set_comment("King Pawn opening")
            >>> cursor.comment
            'King Pawn opening'
        """
        self._native.cursor_set_comment(self._game._handle, self._handle, comment)

    def remove_comment(self) -> None:
        """Clear commentary text at the current cursor node.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 {King pawn} e5 *")
            >>> cursor = game.create_cursor().next()
            >>> cursor.remove_comment()
            >>> cursor.comment == ""
            True
        """
        self.set_comment("")

    def add_nag(self, nag: Nag) -> bool:
        """Attach a Numeric Annotation Glyph to the incoming move.

        Args:
            nag: The [`Nag`][libscid.Nag] glyph to add.

        Returns:
            True if the NAG was attached successfully; otherwise False.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 e5 *")
            >>> cursor = game.create_cursor().next()
            >>> cursor.add_nag(libscid.Nag("!"))
            True
            >>> [nag.symbol for nag in cursor.previous_move_nags]
            ['!']
        """
        return self._native.cursor_add_nag(self._game._handle, self._handle, nag.code)

    def remove_move_nag(self) -> bool:
        """Remove move evaluation NAGs ($1-$9) from the incoming move.

        Returns:
            True if a move NAG was found and removed; otherwise False.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 $1 $14 e5 *")
            >>> cursor = game.create_cursor().next()
            >>> cursor.remove_move_nag()
            True
            >>> [nag.text for nag in cursor.previous_move_nags]
            ['$14']
        """
        return self._native.cursor_remove_nag(
            self._game._handle, self._handle, move_nag=True
        )

    def remove_position_nag(self) -> bool:
        """Remove positional evaluation NAGs ($10-$255) from the incoming move.

        Returns:
            True if a positional NAG was found and removed; otherwise False.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 $1 $14 e5 *")
            >>> cursor = game.create_cursor().next()
            >>> cursor.remove_position_nag()
            True
            >>> [nag.text for nag in cursor.previous_move_nags]
            ['$1']
        """
        return self._native.cursor_remove_nag(
            self._game._handle, self._handle, move_nag=False
        )

    def remove_nags(self) -> None:
        """Remove all Numeric Annotation Glyphs from the incoming move.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 $1 $14 e5 *")
            >>> cursor = game.create_cursor().next()
            >>> cursor.remove_nags()
            >>> cursor.previous_move_nags
            ()
        """
        self._native.cursor_remove_nags(self._game._handle, self._handle)

    @property
    def variation_count(self) -> int:
        """Number of alternative variations branching from the upcoming move."""
        return self._native.cursor_variation_count(self._handle)

    @property
    def variation_depth(self) -> int:
        """Variation nesting depth (0 for mainline, 1 for sub-variation, etc.)."""
        return self._native.cursor_variation_depth(self._handle)

    @property
    def variation_index(self) -> int:
        """Sibling index among alternative variations at the parent fork."""
        return self._native.cursor_variation_index(self._handle)

    @property
    def is_main_line(self) -> bool:
        """True if the cursor is positioned on the game's mainline."""
        return self.variation_depth == 0

    @property
    def is_variation_line(self) -> bool:
        """True if the cursor is positioned on a sub-variation branch."""
        return self.variation_depth > 0

    @property
    def is_line_start(self) -> bool:
        """True if the cursor is at the beginning of the current line."""
        return self._native.cursor_is_line_start(self._handle)

    @property
    def is_line_end(self) -> bool:
        """True if the cursor is at the terminal end of the current line."""
        return self._native.cursor_is_line_end(self._handle)

    @property
    def position(self) -> Position:
        """Board state snapshot at this cursor position.

        Examples:
            >>> import libscid
            >>> game = libscid.Game.from_pgn("1. e4 e5 *")
            >>> cursor = game.create_cursor().next()
            >>> cursor.position.side_to_move
            'black'
            >>> cursor.position.get_piece_at("e4")
            'P'
        """
        return Position._from_handle(
            self._native, self._native.cursor_position(self._handle)
        )

    def _dispose(self) -> None:
        handle = getattr(self, "_handle", None)
        if handle:
            self._native.free_cursor(self._handle)
            self._handle = ctypes.c_void_p()

    def __del__(self) -> None:
        self._dispose()
