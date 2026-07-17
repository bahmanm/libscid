from __future__ import annotations

import os
import sys
from pathlib import Path


def _candidate_library_names() -> tuple[str, ...]:
    if sys.platform == "darwin":
        return ("libscid.dylib",)
    if os.name == "nt":
        return ("scid.dll", "libscid.dll")
    return ("libscid.so",)


def _candidate_directories() -> tuple[Path, ...]:
    native_dir = Path(__file__).resolve().parent
    package_dir = Path(__file__).resolve().parents[1]
    source_root = package_dir.parents[4]
    return (
        native_dir,
        package_dir,
        source_root / "_build" / "release" / "src" / "libscid",
        source_root / "_build" / "debug" / "src" / "libscid",
        source_root / "build" / "libscid",
        source_root / "build" / "src" / "libscid",
        source_root / "src" / "libscid",
        Path.cwd(),
    )


def find_library() -> Path:
    override = os.environ.get("LIBSCID_LIBRARY")
    if override:
        path = Path(override)
        if not path.exists():
            raise FileNotFoundError(f"LIBSCID_LIBRARY does not exist: {path}")
        return path

    for directory in _candidate_directories():
        for name in _candidate_library_names():
            candidate = directory / name
            if candidate.exists():
                return candidate

    searched = ", ".join(str(path) for path in _candidate_directories())
    names = ", ".join(_candidate_library_names())
    raise FileNotFoundError(
        f"Could not find libscid shared library ({names}); searched {searched}. "
        "Set LIBSCID_LIBRARY to the library path."
    )


def enable_windows_dll_search_dirs(library_path: Path) -> None:
    if os.name != "nt" or not hasattr(os, "add_dll_directory"):
        return
    os.add_dll_directory(str(library_path.parent))
