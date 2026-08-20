from __future__ import annotations

import os
import re
import sys

from hatchling.builders.hooks.plugin.interface import BuildHookInterface
from hatchling.metadata.plugin.interface import MetadataHookInterface
from packaging.tags import sys_tags
from packaging.version import InvalidVersion, Version

DEFAULT_VERSION = "0.0.0"


def to_pep440(raw_version: str) -> str:
    version = raw_version.strip()
    if version.startswith("v") or version.startswith("V"):
        version = version[1:]

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


class LibScidBuildHook(BuildHookInterface):
    def initialize(self, version: str, build_data: dict[str, object]) -> None:
        if self.target_name != "wheel" or version != "standard":
            return

        build_data["pure_python"] = False
        build_data["tag"] = f"py3-none-{_platform_tag()}"


def get_build_hook() -> type[LibScidBuildHook]:
    return LibScidBuildHook


class LibScidMetadataHook(MetadataHookInterface):
    def update(self, metadata: dict[str, object]) -> None:
        raw_version = os.environ.get(
            "LIBSCID_PYTHON_VERSION",
            os.environ.get("LIBSCID_RELEASE_VERSION", DEFAULT_VERSION),
        )
        metadata["version"] = to_pep440(raw_version)


def get_metadata_hook() -> type[LibScidMetadataHook]:
    return LibScidMetadataHook
