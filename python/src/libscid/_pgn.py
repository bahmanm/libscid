"""PGN formatting and serialisation options."""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True, slots=True)
class PgnOptions:
    """Configuration options controlling PGN formatting and export serialisation.

    Pass an instance of `PgnOptions` to `Game.to_pgn()` to customise the output
    representation of PGN tags, move annotations, commentary, variations, and
    line wrapping.

    Attributes:
        symbolic_nags: If `True`, renders standard NAGs as typographical symbols
            (e.g. `!`, `?`, `!!`, `??`, `!?`, `?!`). If `False` (the default),
            renders standard dollar notation (`$1`, `$2`, etc.).
        supplemental_tags: If `True` (the default), includes non-standard
            supplemental header tags (e.g. `WhiteElo`, `ECO`, `Annotator`). If
            `False`, emits only the mandatory Seven Tag Roster (STR) and `FEN`.
        comments: If `True` (the default), includes `{ comment }` blocks in the
            exported movetext. If `False`, strips all commentary.
        variations: If `True` (the default), recursively includes `( variation )`
            branches. If `False`, outputs only the mainline moves.
        line_width: Maximum column line width for movetext wrapping (default
            `None` uses the native default of 80 columns; `0` disables line
            wrapping for single-line movetext).

    Examples:
        >>> import libscid
        >>> game = libscid.Game.from_pgn("1. e4 $1 {King pawn} (1. c4) e5 *")
        >>> options = libscid.PgnOptions(
        ...     symbolic_nags=True,
        ...     comments=False,
        ...     variations=False,
        ... )
        >>> "1. e4 ! e5 *" in game.to_pgn(options)
        True
    """

    symbolic_nags: bool = False
    supplemental_tags: bool = True
    comments: bool = True
    variations: bool = True
    line_width: int | None = None

    def __post_init__(self) -> None:
        """Validates that line_width is non-negative when specified.

        Raises:
            ValueError: If `line_width` is negative.
        """
        if self.line_width is not None and self.line_width < 0:
            raise ValueError("line_width must be non-negative or None")
