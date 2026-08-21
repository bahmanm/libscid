"""Numeric Annotation Glyph (NAG) representation and codecs."""

from __future__ import annotations

from dataclasses import dataclass

from ._native import load_library


@dataclass(frozen=True, init=False)
class Nag:
    """Immutable Numeric Annotation Glyph (NAG) for chess move evaluation.

    Encapsulates PGN standard annotation codes (0 to 255) representing move
    qualities (e.g. `!` for good move, `?` for mistake, `!!` for brilliant)
    and positional assessments (e.g. `+=` for slight advantage).

    Attributes:
        code: Integer NAG code ranging from 0 to 255. A code of 0 denotes
            an absent or empty annotation.

    Examples:
        >>> import libscid
        >>> nag = libscid.Nag("!")
        >>> nag.code
        1
        >>> nag.text
        '$1'
        >>> nag.symbol
        '!'
        >>> libscid.Nag("$14").symbol
        '+='
        >>> str(libscid.Nag(3))
        '$3'
    """

    code: int

    def __init__(self, value: int | str | bytes):
        """Initialises a Nag instance from an integer code, text, or glyph symbol.

        Args:
            value: An integer code (0..255), dollar string/bytes (e.g. `"$1"`, `b"$1"`),
                or typographical glyph (e.g. `"!"`, `"!?"`, `"+="`, `"N"`). Unrecognised
                string values default to code 0.

        Raises:
            ValueError: If `value` is an integer outside the range 0..255.
            LibScidError: If an underlying native conversion error occurs.
        """
        code = (
            load_library().nag_from_string(value)
            if isinstance(value, str | bytes)
            else value
        )
        if code < 0 or code > 255:
            raise ValueError("Nag code must be between 0 and 255")
        object.__setattr__(self, "code", code)

    @property
    def text(self) -> str:
        """Standard PGN dollar notation string for the NAG.

        Returns:
            The dollar-formatted string (e.g. `"$1"`, `"$14"`), or an empty
            string `""` when `code` is 0.
        """
        return load_library().nag_to_string(self.code)

    @property
    def symbol(self) -> str:
        """Typographical glyph or symbolic representation of the NAG.

        Returns:
            The typographical symbol (e.g. `"!"`, `"?"`, `"+="`, `"N"`), or an
            empty string `""` when no symbolic representation exists or `code` is 0.
        """
        return load_library().nag_to_string(self.code, symbolic=True)

    def __str__(self) -> str:
        """Returns the standard dollar notation string."""
        return self.text
