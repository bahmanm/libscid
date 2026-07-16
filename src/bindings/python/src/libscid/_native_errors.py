from __future__ import annotations


class LibScidError(RuntimeError):
    def __init__(self, function: str, code: int, diagnostic: str | None = None):
        message = f"{function} failed with scid_error {code}"
        if diagnostic:
            message = f"{message}: {diagnostic}"
        super().__init__(message)
        self.function = function
        self.code = code
        self.diagnostic = diagnostic
