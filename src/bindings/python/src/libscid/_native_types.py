from __future__ import annotations

from typing import Protocol


class PgnOptionsProtocol(Protocol):
    symbolic_nags: bool
    supplemental_tags: bool
    comments: bool
    variations: bool
    line_width: int | None
