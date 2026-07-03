from __future__ import annotations

import ctypes
import os
import platform
from pathlib import Path


SCID_OK = 0
SCID_ERROR_BUFFER_FULL = 601
STANDARD_FEN = b"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


class LibScidError(RuntimeError):
    def __init__(self, function: str, code: int, diagnostic: str | None = None):
        message = f"{function} failed with scid_error {code}"
        if diagnostic:
            message = f"{message}: {diagnostic}"
        super().__init__(message)
        self.function = function
        self.code = code
        self.diagnostic = diagnostic


class LibScid:
    def __init__(self, library_path: str | os.PathLike[str] | None = None):
        self._library_path = (
            Path(library_path).resolve() if library_path else find_library().resolve()
        )
        _enable_windows_dll_search_dirs(self._library_path)
        self._lib = ctypes.CDLL(str(self._library_path))
        self._bind_functions()

    @property
    def library_path(self) -> Path:
        return self._library_path

    def game_from_pgn(self, pgn: str | bytes) -> "Game":
        pgn_bytes = _encode(pgn)
        position = ctypes.c_void_p()
        game = ctypes.c_void_p()
        diagnostic = ctypes.create_string_buffer(4096)
        diagnostic_size = ctypes.c_size_t()
        self._check(
            "scid_position_create_from_fen",
            self._lib.scid_position_create_from_fen(STANDARD_FEN, ctypes.byref(position)),
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
            return Game(self, game)
        finally:
            self._lib.scid_position_free(position)

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

        self._lib.scid_position_create_from_fen.argtypes = [ctypes.c_char_p, c_void_p_p]
        self._lib.scid_position_create_from_fen.restype = ctypes.c_ushort

        self._lib.scid_position_free.argtypes = [ctypes.c_void_p]
        self._lib.scid_position_free.restype = None

        self._lib.scid_game_free.argtypes = [ctypes.c_void_p]
        self._lib.scid_game_free.restype = None

        self._lib.scid_game_to_pgn.argtypes = [
            ctypes.c_void_p,
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ]
        self._lib.scid_game_to_pgn.restype = ctypes.c_ushort

        self._lib.scid_game_tag_get.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
            c_size_t_p,
        ]
        self._lib.scid_game_tag_get.restype = ctypes.c_ushort

        self._lib.scid_game_tag_set.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_char_p,
        ]
        self._lib.scid_game_tag_set.restype = ctypes.c_ushort


class Game:
    def __init__(self, libscid: LibScid, handle: ctypes.c_void_p):
        self._libscid = libscid
        self._handle = handle

    def __enter__(self) -> "Game":
        return self

    def __exit__(self, exc_type, exc, traceback) -> None:
        self.close()

    def close(self) -> None:
        if self._handle:
            self._libscid._lib.scid_game_free(self._handle)
            self._handle = ctypes.c_void_p()

    def tag(self, name: str) -> str:
        self._require_open()
        return self._libscid._string_result(
            "scid_game_tag_get",
            self._handle,
            _encode(name),
        )

    def set_tag(self, name: str, value: str) -> None:
        self._require_open()
        self._libscid._check(
            "scid_game_tag_set",
            self._libscid._lib.scid_game_tag_set(
                self._handle,
                _encode(name),
                _encode(value),
            ),
        )

    def to_pgn(self) -> str:
        self._require_open()
        return self._libscid._string_result("scid_game_to_pgn", self._handle, None)

    def _require_open(self) -> None:
        if not self._handle:
            raise RuntimeError("game is closed")

    def __del__(self) -> None:
        self.close()


def find_library() -> Path:
    env_path = os.environ.get("LIBSCID_LIBRARY")
    if env_path:
        return Path(env_path)

    root = Path(__file__).resolve().parents[3]
    name = _library_name()
    candidates = [
        root / "_build-shared" / "src" / "libscid" / name,
        root / "_build-shared" / "src" / "libscid" / "Release" / name,
        root / "_build" / "src" / "libscid" / name,
        root / "_build" / "src" / "libscid" / "Release" / name,
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
    root = Path(__file__).resolve().parents[3]
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
