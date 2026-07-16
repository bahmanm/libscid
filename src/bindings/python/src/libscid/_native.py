from __future__ import annotations

import ctypes
import os
import platform
from pathlib import Path
from typing import Final, Protocol


SCID_OK: Final = 0
SCID_ERROR_BUFFER_FULL: Final = 601

_STANDARD_FEN: Final = (
    b"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
)


class _PgnOptions(Protocol):
    symbolic_nags: bool
    supplemental_tags: bool
    comments: bool
    variations: bool
    line_width: int | None


class LibScidError(RuntimeError):
    def __init__(self, function: str, code: int, diagnostic: str | None = None):
        message = f"{function} failed with scid_error {code}"
        if diagnostic:
            message = f"{message}: {diagnostic}"
        super().__init__(message)
        self.function = function
        self.code = code
        self.diagnostic = diagnostic


class NativeLibrary:
    def __init__(self, library_path: str | os.PathLike[str] | None = None):
        self.library_path = (
            Path(library_path).resolve() if library_path else _find_library().resolve()
        )
        _enable_windows_dll_search_dirs(self.library_path)
        self._lib = ctypes.CDLL(str(self.library_path))
        self._bind_functions()

    def create_game_from_pgn(self, pgn: str | bytes) -> ctypes.c_void_p:
        pgn_bytes = _encode(pgn)
        position = ctypes.c_void_p()
        game = ctypes.c_void_p()
        diagnostic = ctypes.create_string_buffer(4096)
        diagnostic_size = ctypes.c_size_t()

        self._check(
            "scid_position_create_from_fen",
            self._lib.scid_position_create_from_fen(
                _STANDARD_FEN, ctypes.byref(position)
            ),
        )
        try:
            error = self._lib.scid_game_create(
                position,
                pgn_bytes,
                len(pgn_bytes),
                ctypes.byref(game),
                diagnostic,
                len(diagnostic),
                ctypes.byref(diagnostic_size),
            )
            if error != SCID_OK:
                raise LibScidError(
                    "scid_game_create",
                    error,
                    _decode_buffer(diagnostic, diagnostic_size.value),
                )
            return game
        finally:
            self._lib.scid_position_free(position)

    def free_game(self, game: ctypes.c_void_p) -> None:
        self._lib.scid_game_free(game)

    def game_mainline_halfmove_count(self, game: ctypes.c_void_p) -> int:
        count = ctypes.c_size_t()
        self._check(
            "scid_game_mainline_halfmove_count_get",
            self._lib.scid_game_mainline_halfmove_count_get(
                game, ctypes.byref(count)
            ),
        )
        return count.value

    def game_to_pgn(
        self, game: ctypes.c_void_p, options: _PgnOptions | None = None
    ) -> str:
        if options is None:
            return self._string_result("scid_game_to_pgn", game, None)

        native_options = self._create_pgn_options(options)
        try:
            return self._string_result("scid_game_to_pgn", game, native_options)
        finally:
            self._lib.scid_game_pgn_options_free(native_options)

    def _create_pgn_options(self, options: _PgnOptions) -> ctypes.c_void_p:
        native_options = ctypes.c_void_p()
        self._check(
            "scid_game_pgn_options_create",
            self._lib.scid_game_pgn_options_create(ctypes.byref(native_options)),
        )
        try:
            self._check(
                "scid_game_pgn_options_symbolic_nags_set",
                self._lib.scid_game_pgn_options_symbolic_nags_set(
                    native_options, int(options.symbolic_nags)
                ),
            )
            self._check(
                "scid_game_pgn_options_supplemental_tags_set",
                self._lib.scid_game_pgn_options_supplemental_tags_set(
                    native_options, int(options.supplemental_tags)
                ),
            )
            self._check(
                "scid_game_pgn_options_comments_set",
                self._lib.scid_game_pgn_options_comments_set(
                    native_options, int(options.comments)
                ),
            )
            self._check(
                "scid_game_pgn_options_variations_set",
                self._lib.scid_game_pgn_options_variations_set(
                    native_options, int(options.variations)
                ),
            )
            self._check(
                "scid_game_pgn_options_line_width_set",
                self._lib.scid_game_pgn_options_line_width_set(
                    native_options,
                    0 if options.line_width is None else options.line_width,
                ),
            )
        except Exception:
            self._lib.scid_game_pgn_options_free(native_options)
            raise
        return native_options

    def _check(self, function: str, error: int) -> None:
        if error != SCID_OK:
            raise LibScidError(function, error)

    def _string_result(self, function_name: str, *args: object) -> str:
        function = getattr(self._lib, function_name)
        capacity = 1024
        while True:
            output = ctypes.create_string_buffer(capacity)
            output_size = ctypes.c_size_t()
            error = function(*args, output, capacity, ctypes.byref(output_size))
            if error == SCID_OK:
                return _decode_buffer(output, output_size.value)
            if error != SCID_ERROR_BUFFER_FULL:
                raise LibScidError(function_name, error)
            capacity *= 2

    def _bind_functions(self) -> None:
        c_size_t_p = ctypes.POINTER(ctypes.c_size_t)
        c_void_p_p = ctypes.POINTER(ctypes.c_void_p)

        self._lib.scid_position_create_from_fen.argtypes = [
            ctypes.c_char_p,
            c_void_p_p,
        ]
        self._lib.scid_position_create_from_fen.restype = ctypes.c_ushort

        self._lib.scid_position_free.argtypes = [ctypes.c_void_p]
        self._lib.scid_position_free.restype = None

        self._lib.scid_game_create.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_void_p_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ]
        self._lib.scid_game_create.restype = ctypes.c_ushort

        self._lib.scid_game_free.argtypes = [ctypes.c_void_p]
        self._lib.scid_game_free.restype = None

        self._lib.scid_game_pgn_options_create.argtypes = [c_void_p_p]
        self._lib.scid_game_pgn_options_create.restype = ctypes.c_ushort

        self._lib.scid_game_pgn_options_free.argtypes = [ctypes.c_void_p]
        self._lib.scid_game_pgn_options_free.restype = None

        self._lib.scid_game_pgn_options_symbolic_nags_set.argtypes = [
            ctypes.c_void_p,
            ctypes.c_int,
        ]
        self._lib.scid_game_pgn_options_symbolic_nags_set.restype = ctypes.c_ushort

        self._lib.scid_game_pgn_options_supplemental_tags_set.argtypes = [
            ctypes.c_void_p,
            ctypes.c_int,
        ]
        self._lib.scid_game_pgn_options_supplemental_tags_set.restype = ctypes.c_ushort

        self._lib.scid_game_pgn_options_comments_set.argtypes = [
            ctypes.c_void_p,
            ctypes.c_int,
        ]
        self._lib.scid_game_pgn_options_comments_set.restype = ctypes.c_ushort

        self._lib.scid_game_pgn_options_variations_set.argtypes = [
            ctypes.c_void_p,
            ctypes.c_int,
        ]
        self._lib.scid_game_pgn_options_variations_set.restype = ctypes.c_ushort

        self._lib.scid_game_pgn_options_line_width_set.argtypes = [
            ctypes.c_void_p,
            ctypes.c_uint,
        ]
        self._lib.scid_game_pgn_options_line_width_set.restype = ctypes.c_ushort

        self._lib.scid_game_mainline_halfmove_count_get.argtypes = [
            ctypes.c_void_p,
            c_size_t_p,
        ]
        self._lib.scid_game_mainline_halfmove_count_get.restype = ctypes.c_ushort

        self._lib.scid_game_to_pgn.argtypes = [
            ctypes.c_void_p,
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ]
        self._lib.scid_game_to_pgn.restype = ctypes.c_ushort


def load_library(library_path: str | os.PathLike[str] | None = None) -> NativeLibrary:
    return NativeLibrary(library_path)


def _find_library() -> Path:
    env_path = os.environ.get("LIBSCID_LIBRARY")
    if env_path:
        return Path(env_path)

    root = _repository_root()
    name = _library_name()
    candidates = [
        root / "_build" / "src" / "libscid" / name,
        root / "_build" / "release" / "src" / "libscid" / name,
        root / "_build" / "current-install" / "lib" / name,
        root / "_build-shared" / "src" / "libscid" / name,
        root / "_build-shared" / "src" / "libscid" / "Release" / name,
        root / "install" / "libscid" / "lib" / name,
        root / "install" / "libscid" / "bin" / name,
    ]
    for candidate in candidates:
        if candidate.exists():
            return candidate

    raise FileNotFoundError(
        "Could not find libscid shared library. Set LIBSCID_LIBRARY to the "
        "absolute path of libscid.dylib, libscid.so, or scid.dll."
    )


def _repository_root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "CMakeLists.txt").exists() and (parent / "src/libscid").exists():
            return parent
    raise FileNotFoundError(
        "Could not infer the libscid repository root. Set LIBSCID_LIBRARY to "
        "the absolute path of libscid.dylib, libscid.so, or scid.dll."
    )


def _library_name() -> str:
    system = platform.system()
    if system == "Darwin":
        return "libscid.dylib"
    if system == "Windows":
        return "scid.dll"
    return "libscid.so"


def _enable_windows_dll_search_dirs(library_path: Path) -> None:
    if platform.system() != "Windows" or not hasattr(os, "add_dll_directory"):
        return

    directories = [library_path.parent]
    root = _repository_root()
    for build_dir in ["_build-shared", "_build"]:
        for relative in [
            ("lib", "Release"),
            ("src", "libscid", "Release"),
        ]:
            directories.append(root / build_dir / Path(*relative))

    seen: set[Path] = set()
    for directory in directories:
        if directory.exists() and directory not in seen:
            os.add_dll_directory(str(directory))
            seen.add(directory)


def _encode(value: str | bytes) -> bytes:
    if isinstance(value, bytes):
        return value
    return value.encode("utf-8")


def _decode_buffer(buffer: ctypes.Array[ctypes.c_char], size: int) -> str:
    return bytes(buffer.raw[:size]).decode("utf-8")
