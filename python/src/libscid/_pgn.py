from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True, slots=True)
class PgnOptions:
    symbolic_nags: bool = False
    supplemental_tags: bool = True
    comments: bool = True
    variations: bool = True
    line_width: int | None = None

    def __post_init__(self) -> None:
        if self.line_width is not None and self.line_width < 0:
            raise ValueError("line_width must be non-negative or None")
