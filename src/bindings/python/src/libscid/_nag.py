from __future__ import annotations

from dataclasses import dataclass

from ._native import load_library


@dataclass(frozen=True, init=False)
class Nag:
    code: int

    def __init__(self, value: int | str | bytes):
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
        return load_library().nag_to_string(self.code)

    @property
    def symbol(self) -> str:
        return load_library().nag_to_string(self.code, symbolic=True)

    def __str__(self) -> str:
        return self.text
