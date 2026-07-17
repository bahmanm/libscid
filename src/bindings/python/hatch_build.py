from __future__ import annotations

import os
import sys

from hatchling.builders.hooks.plugin.interface import BuildHookInterface
from hatchling.metadata.plugin.interface import MetadataHookInterface
from packaging.tags import sys_tags

DEFAULT_VERSION = "0.0.0"


def _platform_tag() -> str:
    tag = next(
        item
        for item in sys_tags()
        if "manylinux" not in item.platform and "musllinux" not in item.platform
    )
    platform = tag.platform

    if sys.platform == "darwin":
        from hatchling.builders.macos import process_macos_plat_tag

        platform = process_macos_plat_tag(platform, compat=False)

    return platform


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
