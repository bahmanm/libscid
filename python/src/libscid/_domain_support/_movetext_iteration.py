"""Hierarchical chess movetext tree iteration and event streams."""

from __future__ import annotations

from collections.abc import Iterator
from dataclasses import dataclass
from typing import TYPE_CHECKING

from .._cursor import Cursor

if TYPE_CHECKING:
    from .._nag import Nag
    from .._position import Position


@dataclass(frozen=True, slots=True)
class MovetextLineStart:
    """Event emitted at the beginning of a line or sub-variation branch.

    Attributes:
        cursor: Cursor positioned at the start of this line or variation.
        preceding_comment: Optional introductory commentary text preceding the
            first move of the line, or None if absent.
        variation_depth: Variation nesting level (0 for the mainline, 1 for
            first-level sub-variations, etc.).
        variation_index: Sibling index among variations branching at this
            fork point (0 for mainline or first alternative).

    Examples:
        >>> import libscid
        >>> game = libscid.Game.from_pgn("{Intro} 1. e4 e5 *")
        >>> start_event = next(game.iter_movetext())
        >>> isinstance(start_event, libscid.MovetextLineStart)
        True
        >>> start_event.preceding_comment
        'Intro'
        >>> start_event.variation_depth
        0
    """

    cursor: Cursor
    preceding_comment: str | None
    variation_depth: int
    variation_index: int


@dataclass(frozen=True, slots=True)
class MovetextMove:
    """Event emitted for each move transition in the movetext tree.

    Encapsulates the move in both Standard Algebraic Notation (SAN) and
    Universal Chess Interface (UCI) formats, associated NAG annotations,
    trailing commentary, variation nesting hierarchy, and board position
    snapshots immediately before and after the move.

    Attributes:
        before: Cursor positioned immediately prior to this move.
        after: Cursor positioned immediately following this move.
        san: Standard Algebraic Notation string (e.g. "e4", "Nf3", "O-O").
        uci: Universal Chess Interface coordinate string (e.g. "e2e4", "g1f3").
        nags: Tuple of [`Nag`][libscid.Nag] glyphs attached to this move.
        comment: Commentary text attached to this move, or empty string.
        variation_depth: Variation nesting level (0 for mainline).
        variation_index: Sibling variation index at the parent fork.
        position_before: [`Position`][libscid.Position] snapshot before the move.
        position_after: [`Position`][libscid.Position] snapshot after the move.

    Examples:
        >>> import libscid
        >>> game = libscid.Game.from_pgn("1. e4 $1 {King pawn} e5 *")
        >>> move = next(
        ...     e for e in game.iter_movetext()
        ...     if isinstance(e, libscid.MovetextMove)
        ... )
        >>> move.san
        'e4'
        >>> move.uci
        'e2e4'
        >>> move.comment
        'King pawn'
    """

    before: Cursor
    after: Cursor
    san: str
    uci: str
    nags: tuple[Nag, ...]
    comment: str
    variation_depth: int
    variation_index: int
    position_before: Position
    position_after: Position


@dataclass(frozen=True, slots=True)
class MovetextLineEnd:
    """Event emitted at the terminal end of a line or sub-variation branch.

    Attributes:
        cursor: Cursor positioned at the end of this line or variation.
        variation_depth: Variation nesting level (0 for mainline).
        variation_index: Sibling variation index at the parent fork.

    Examples:
        >>> import libscid
        >>> game = libscid.Game.from_pgn("1. e4 e5 *")
        >>> events = list(game.iter_movetext())
        >>> end_event = events[-1]
        >>> isinstance(end_event, libscid.MovetextLineEnd)
        True
        >>> end_event.cursor.is_line_end
        True
    """

    cursor: Cursor
    variation_depth: int
    variation_index: int


MovetextEvent = MovetextLineStart | MovetextMove | MovetextLineEnd
"""Union of all movetext iteration event types."""


def iter_movetext(
    cursor: Cursor,
    *,
    variations: bool = True,
) -> Iterator[MovetextEvent]:
    """Iterate over movetext events starting from a given cursor position.

    Recursively traverses the movetext tree from `cursor`, yielding structured
    [`MovetextLineStart`][libscid.MovetextLineStart],
    [`MovetextMove`][libscid.MovetextMove], and
    [`MovetextLineEnd`][libscid.MovetextLineEnd] events. When `variations` is
    True, sub-variation branches are explored in depth-first order immediately
    after the parent move from which they branch.

    Args:
        cursor: Starting [`Cursor`][libscid.Cursor] location for the traversal.
        variations: Whether to recursively traverse nested variation branches.
            Defaults to True.

    Yields:
        Sequential [`MovetextEvent`][libscid.MovetextEvent] instances.

    Raises:
        TypeError: If `cursor` is not a `Cursor` or `variations` is not a boolean.
        RuntimeError: If cursor navigation encounters an unexpected state.

    Examples:
        >>> import libscid
        >>> game = libscid.Game.from_pgn("1. e4 (1. d4 d5) e5 *")
        >>> moves = [
        ...     event.san for event in game.iter_movetext()
        ...     if isinstance(event, libscid.MovetextMove)
        ... ]
        >>> moves
        ['e4', 'd4', 'd5', 'e5']
    """
    if not isinstance(cursor, Cursor):
        raise TypeError("cursor must be a Cursor")
    if not isinstance(variations, bool):
        raise TypeError("variations must be bool")

    yield from _iter_line(cursor, variations=variations)


def _iter_line(
    cursor: Cursor,
    *,
    variations: bool,
) -> Iterator[MovetextEvent]:
    yield MovetextLineStart(
        cursor=cursor,
        preceding_comment=cursor.preceding_comment,
        variation_depth=cursor.variation_depth,
        variation_index=cursor.variation_index,
    )

    while not cursor.is_line_end:
        before = cursor
        after = before.next()
        if after is None:
            raise RuntimeError("Cursor next move disappeared")

        yield _move_event(before, after)

        if variations:
            for index in range(before.variation_count):
                variation = before.enter_variation(index)
                if variation is None:
                    raise RuntimeError("Cursor variation disappeared")
                yield from _iter_line(variation, variations=variations)

        cursor = after

    yield MovetextLineEnd(
        cursor=cursor,
        variation_depth=cursor.variation_depth,
        variation_index=cursor.variation_index,
    )


def _move_event(before: Cursor, after: Cursor) -> MovetextMove:
    san = before.next_move_san
    uci = before.next_move_uci
    nags = before.next_move_nags
    comment = after.comment

    if san is None or uci is None or nags is None:
        raise RuntimeError("Cursor move metadata disappeared")
    if comment is None:
        raise RuntimeError("Cursor move comment disappeared")

    return MovetextMove(
        before=before,
        after=after,
        san=san,
        uci=uci,
        nags=nags,
        comment=comment,
        variation_depth=before.variation_depth,
        variation_index=before.variation_index,
        position_before=before.position,
        position_after=after.position,
    )
