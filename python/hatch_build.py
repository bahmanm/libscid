from __future__ import annotations

import os
import sys

from hatchling.builders.hooks.plugin.interface import BuildHookInterface
from hatchling.metadata.plugin.interface import MetadataHookInterface
from packaging.tags import sys_tags

DEFAULT_VERSION = "0.0.0"


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
        metadata["version"] = os.environ.get("LIBSCID_PYTHON_VERSION", DEFAULT_VERSION)


def get_metadata_hook() -> type[LibScidMetadataHook]:
    return LibScidMetadataHook
