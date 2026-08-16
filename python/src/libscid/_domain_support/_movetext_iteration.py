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
    cursor: Cursor
    preceding_comment: str | None
    variation_depth: int
    variation_index: int


@dataclass(frozen=True, slots=True)
class MovetextMove:
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
    cursor: Cursor
    variation_depth: int
    variation_index: int


MovetextEvent = MovetextLineStart | MovetextMove | MovetextLineEnd


def iter_movetext(
    cursor: Cursor,
    *,
    variations: bool = True,
) -> Iterator[MovetextEvent]:
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
