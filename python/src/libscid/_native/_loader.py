from __future__ import annotations

import os
import sys
from pathlib import Path


def _candidate_library_names() -> tuple[str, ...]:
    if sys.platform == "darwin":
        return ("libscid.dylib",)
    if sys.platform.startswith("win") or os.name == "nt":
        return ("scid.dll", "libscid.dll")
    return ("libscid.so",)


def _prefix_candidate_directories(prefix: Path) -> tuple[Path, ...]:
    return (
        prefix / "lib",
        prefix / "lib64",
        prefix / "bin",
        prefix,
    )


def _candidate_staging_directories() -> tuple[Path, ...]:
    package_dir = Path(__file__).resolve().parents[1]
    if len(package_dir.parents) < 3:
        return ()

    source_root = package_dir.parents[2]
    staging_dir = source_root / "_staging"
    if not staging_dir.is_dir():
        return ()

    return (
        staging_dir / "install" / "capi" / "release" / "lib",
        staging_dir / "install" / "capi" / "debug" / "lib",
        staging_dir / "build" / "capi" / "release" / "shared",
        staging_dir / "build" / "capi" / "debug" / "shared",
    )


def find_library() -> Path:
    # 1. LIBSCID_LIBRARY_PATH: explicit path to the library file
    library_path = os.environ.get("LIBSCID_LIBRARY_PATH")
    if library_path:
        path = Path(library_path).resolve()
        if not path.is_file():
            raise FileNotFoundError(f"LIBSCID_LIBRARY_PATH does not exist: {path}")
        return path

    library_names = _candidate_library_names()

    # 2. LIBSCID_LIBRARY_PREFIX: installation prefix
    prefix = os.environ.get("LIBSCID_LIBRARY_PREFIX")
    if prefix:
        prefix_path = Path(prefix).resolve()
        for directory in _prefix_candidate_directories(prefix_path):
            for name in library_names:
                candidate = directory / name
                if candidate.is_file():
                    return candidate
        raise FileNotFoundError(
            "Could not find libscid shared library under "
            f"LIBSCID_LIBRARY_PREFIX={prefix_path}"
        )

    # 3. Staging directory
    for directory in _candidate_staging_directories():
        for name in library_names:
            candidate = directory / name
            if candidate.is_file():
                return candidate

    # 4. Bundle (installed wheel package or in-tree development)
    native_dir = Path(__file__).resolve().parent
    for name in library_names:
        candidate = native_dir / name
        if candidate.is_file():
            return candidate

    names = ", ".join(library_names)
    raise FileNotFoundError(
        f"Could not find libscid shared library ({names}). "
        "Set LIBSCID_LIBRARY_PATH to the library file, or "
        "LIBSCID_LIBRARY_PREFIX to the installation directory."
    )


def enable_windows_dll_search_dirs(library_path: Path) -> None:
    if os.name != "nt" or not hasattr(os, "add_dll_directory"):
        return
    os.add_dll_directory(str(library_path.parent))
