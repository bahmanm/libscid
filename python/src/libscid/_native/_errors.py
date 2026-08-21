"""Exception hierarchy for libscid C ABI errors."""

from __future__ import annotations


class LibScidError(RuntimeError):
    """Exception raised when an underlying libscid C ABI operation fails.

    Encapsulates the name of the failing C function, the numeric `scid_error`
    status code, and any diagnostic message emitted by the native subsystem.

    Attributes:
        function: The name of the C API function that reported the failure
            (e.g. `"scid_position_create_from_fen"`).
        code: The numeric `scid_error` status code returned by the C ABI
            (e.g. `301` for `SCID_ERROR_INVALID_FEN`).
        diagnostic: Optional supplementary diagnostic information or parser
            error details provided by the native library.

    Examples:
        >>> import libscid
        >>> try:
        ...     libscid.Position.from_fen("invalid fen string")
        ... except libscid.LibScidError as exc:
        ...     exc.function
        ...     exc.code
        'scid_position_create_from_fen'
        301
    """

    def __init__(self, function: str, code: int, diagnostic: str | None = None):
        """Initialises a new LibScidError instance.

        Args:
            function: Name of the failing C API function.
            code: Numeric error status code returned by the C ABI.
            diagnostic: Optional diagnostic detail string.
        """
        message = f"{function} failed with scid_error {code}"
        if diagnostic:
            message = f"{message}: {diagnostic}"
        super().__init__(message)
        self.function = function
        self.code = code
        self.diagnostic = diagnostic
