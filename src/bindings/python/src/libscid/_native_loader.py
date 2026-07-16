from __future__ import annotations

import os
import platform
from pathlib import Path


def find_library() -> Path:
    env_path = os.environ.get("LIBSCID_LIBRARY")
    if env_path:
        return Path(env_path)

    root = repository_root()
    name = library_name()
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


def repository_root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "CMakeLists.txt").exists() and (parent / "src/libscid").exists():
            return parent
    raise FileNotFoundError(
        "Could not infer the libscid repository root. Set LIBSCID_LIBRARY to "
        "the absolute path of libscid.dylib, libscid.so, or scid.dll."
    )


def library_name() -> str:
    system = platform.system()
    if system == "Darwin":
        return "libscid.dylib"
    if system == "Windows":
        return "scid.dll"
    return "libscid.so"


def enable_windows_dll_search_dirs(library_path: Path) -> None:
    if platform.system() != "Windows" or not hasattr(os, "add_dll_directory"):
        return

    directories = [library_path.parent]
    root = repository_root()
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
