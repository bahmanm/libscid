from __future__ import annotations

import os
import re
import subprocess
import sys

from hatchling.builders.hooks.plugin.interface import BuildHookInterface
from hatchling.metadata.plugin.interface import MetadataHookInterface
from packaging.tags import sys_tags
from packaging.version import InvalidVersion, Version


def to_pep440(raw_version: str) -> str:
    version = raw_version.strip()
    if version.startswith("v") or version.startswith("V"):
        version = version[1:]

    if version == "snapshot":
        return "0.0.0.dev0"

    # -testing.N, -test.N, -dev.N, -devN -> .devN
    version = re.sub(
        r"-(?:testing|test|dev)\.?(\d+)",
        r".dev\1",
        version,
        flags=re.IGNORECASE,
    )
    # -alpha.N, -a.N, -alphaN, -aN -> aN
    version = re.sub(
        r"-(?:alpha|a)\.?(\d+)",
        r"a\1",
        version,
        flags=re.IGNORECASE,
    )
    # -beta.N, -b.N, -betaN, -bN -> bN
    version = re.sub(
        r"-(?:beta|b)\.?(\d+)",
        r"b\1",
        version,
        flags=re.IGNORECASE,
    )
    # -rc.N, -rcN, -c.N -> rcN
    version = re.sub(
        r"-(?:rc|c)\.?(\d+)",
        r"rc\1",
        version,
        flags=re.IGNORECASE,
    )
    # -post.N, -postN -> .postN
    version = re.sub(
        r"-post\.?(\d+)",
        r".post\1",
        version,
        flags=re.IGNORECASE,
    )

    try:
        parsed = Version(version)
        return str(parsed)
    except InvalidVersion:
        return version


def _platform_tag() -> str:
    if sys.platform == "darwin":
        from hatchling.builders.macos import process_macos_plat_tag

        tag = next(
            item
            for item in sys_tags()
            if "manylinux" not in item.platform and "musllinux" not in item.platform
        )
        return process_macos_plat_tag(tag.platform, compat=False)

    if sys.platform.startswith("linux"):
        tag = next(
            (item for item in sys_tags() if "manylinux" in item.platform),
            None,
        )
        if tag is not None:
            return tag.platform

        tag = next(item for item in sys_tags() if "musllinux" not in item.platform)
        return tag.platform

    if sys.platform.startswith("win") or sys.platform == "win32":
        tag = next(item for item in sys_tags() if item.platform.startswith("win"))
        return tag.platform

    raise RuntimeError(f"Unsupported platform for libscid wheel build: {sys.platform}")


def _candidate_library_names() -> tuple[str, ...]:
    if sys.platform == "darwin":
        return ("libscid.dylib",)
    if sys.platform.startswith("win") or os.name == "nt":
        return ("scid.dll", "libscid.dll")
    return ("libscid.so",)


def _prefix_candidate_directories(prefix: str) -> tuple[str, ...]:
    prefix_path = os.path.abspath(prefix)
    return (
        os.path.join(prefix_path, "lib"),
        os.path.join(prefix_path, "lib64"),
        os.path.join(prefix_path, "bin"),
        prefix_path,
    )


def _find_native_library(root: str) -> str | None:
    # 1. LIBSCID_LIBRARY_PATH: explicit path to the library file
    library_path = os.environ.get("LIBSCID_LIBRARY_PATH")
    if library_path:
        path = os.path.abspath(library_path)
        if os.path.isfile(path):
            return path
        return None

    library_names = _candidate_library_names()

    # 2. LIBSCID_LIBRARY_PREFIX: installation prefix
    prefix = os.environ.get("LIBSCID_LIBRARY_PREFIX")
    if prefix:
        for directory in _prefix_candidate_directories(prefix):
            if not os.path.isdir(directory):
                continue
            for name in library_names:
                candidate = os.path.join(directory, name)
                if os.path.isfile(candidate):
                    return candidate
        return None

    # 3. Staging directory
    repo_root = os.path.abspath(os.path.join(root, ".."))
    staging_dirs = (
        os.path.join(repo_root, "_staging", "install", "capi", "release", "lib"),
        os.path.join(repo_root, "_staging", "install", "capi", "debug", "lib"),
        os.path.join(repo_root, "_staging", "build", "capi", "release", "shared"),
        os.path.join(repo_root, "_staging", "build", "capi", "debug", "shared"),
    )
    for directory in staging_dirs:
        if not os.path.isdir(directory):
            continue
        for name in library_names:
            candidate = os.path.join(directory, name)
            if os.path.isfile(candidate):
                return candidate

    # 4. Bundle (in-tree source package)
    bundle_dir = os.path.join(root, "src", "libscid", "_native")
    if os.path.isdir(bundle_dir):
        for name in library_names:
            candidate = os.path.join(bundle_dir, name)
            if os.path.isfile(candidate):
                return candidate

    return None


def _abort_unsupported_sdist_install() -> None:
    system_name = sys.platform
    try:
        import platform

        system_name = f"{platform.system()} ({platform.machine()})"
    except Exception:
        pass

    message = (
        "\n"
        + "=" * 80
        + "\n"
        + "There are no prebuilt libscid packages for your platform, "
        + f"i.e. {system_name}.\n"
        + "\n"
        + "Please visit the project repository to explore other installation methods,\n"
        + "including building libscid from source:\n"
        + "\n"
        + "  https://github.com/bahmanm/libscid\n"
        + "=" * 80
        + "\n\n"
    )
    sys.stderr.write(message)
    sys.exit(1)


class LibScidBuildHook(BuildHookInterface):
    def initialize(self, version: str, build_data: dict[str, object]) -> None:
        if self.target_name != "wheel" or version != "standard":
            return

        native_library = _find_native_library(self.root)
        if not native_library:
            _abort_unsupported_sdist_install()

        assert native_library is not None
        force_include = build_data.setdefault("force_include", {})
        if isinstance(force_include, dict):
            filename = os.path.basename(native_library)
            force_include[native_library] = f"libscid/_native/{filename}"

        build_data["pure_python"] = False
        build_data["tag"] = f"py3-none-{_platform_tag()}"


def get_build_hook() -> type[LibScidBuildHook]:
    return LibScidBuildHook


class LibScidMetadataHook(MetadataHookInterface):
    def update(self, metadata: dict[str, object]) -> None:
        raw_version = os.environ.get("LIBSCID_PYTHON_VERSION") or os.environ.get(
            "LIBSCID_RELEASE_VERSION"
        )
        if not raw_version:
            version_cmake = os.path.abspath(
                os.path.join(
                    os.path.dirname(__file__), "..", "etc", "cmake", "version.cmake"
                )
            )
            if not os.path.exists(version_cmake):
                _abort_unsupported_sdist_install()

            result = subprocess.run(
                ["cmake", "-P", version_cmake],
                capture_output=True,
                text=True,
                check=True,
            )
            tokens = result.stdout.strip().split()
            raw_version = tokens[1] if len(tokens) > 1 else tokens[0]

        metadata["version"] = to_pep440(raw_version)


def get_metadata_hook() -> type[LibScidMetadataHook]:
    return LibScidMetadataHook
