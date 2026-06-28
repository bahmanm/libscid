#!/usr/bin/env python3
"""Patch Doxygen's generated navigation into libscid's preferred shape."""

from __future__ import annotations

import re
import sys
from pathlib import Path


def _extract_node_by_url(text: str, url: str) -> str:
    marker = f'", "{url}",'
    marker_pos = text.index(marker)
    start = text.rfind("[ ", 0, marker_pos)
    if start == -1:
        raise ValueError(f"could not find NAVTREE node start for {url!r}")
    depth = 0
    end = start
    in_string = False
    escaped = False
    for i in range(start, len(text)):
        ch = text[i]
        if in_string:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == '"':
                in_string = False
            continue
        if ch == '"':
            in_string = True
        elif ch == "[":
            depth += 1
        elif ch == "]":
            depth -= 1
            if depth == 0:
                end = i + 1
                break
    if end == start:
        raise ValueError(f"could not extract NAVTREE node {url!r}")
    return text[start:end]


def _try_extract_node_by_url(text: str, url: str) -> str | None:
    try:
        return _extract_node_by_url(text, url)
    except ValueError:
        return None


def _extract_menu_object(text: str, title: str) -> str:
    marker = f'{{text:"{title}",'
    start = text.index(marker)
    depth = 0
    end = start
    in_string = False
    escaped = False
    for i in range(start, len(text)):
        ch = text[i]
        if in_string:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == '"':
                in_string = False
            continue
        if ch == '"':
            in_string = True
        elif ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                end = i + 1
                break
    if end == start:
        raise ValueError(f"could not extract menu object {title!r}")
    return text[start:end]


def _try_extract_menu_object(text: str, title: str) -> str | None:
    try:
        return _extract_menu_object(text, title)
    except ValueError:
        return None


def _patch_navtree(html_dir: Path) -> None:
    nav_path = html_dir / "navtreedata.js"
    text = nav_path.read_text()

    root_nodes = []
    for url in (
        "quick_start.html",
        "installation.html",
        "examples_recipes.html",
        "api_surface.html",
    ):
        node = _try_extract_node_by_url(text, url)
        if node is not None:
            root_nodes.append(node)

    api_nodes = []
    for url in (
        "annotated.html",
        "files.html",
        "globals.html",
    ):
        node = _try_extract_node_by_url(text, url)
        if node is not None:
            api_nodes.append(node)

    if api_nodes:
        root_nodes.append(
            '[ "API Reference", "files.html", [\n      '
            + ",\n      ".join(api_nodes)
            + "\n    ] ]"
        )

    root = f'''var NAVTREE =
[
  [ "libscid", "index.html", [
    {",\n    ".join(root_nodes)}
  ] ]
];
'''

    text = re.sub(r"var NAVTREE =\n\[.*?\n\];\n", root, text, count=1, flags=re.S)
    nav_path.write_text(text)


def _read_navtree_index_paths(html_dir: Path) -> dict[str, tuple[int, ...]]:
    text = "\n".join(path.read_text() for path in sorted(html_dir.glob("navtreeindex*.js")))
    paths = {}
    for url in (
        "quick_start.html",
        "installation.html",
        "examples_recipes.html",
        "api_surface.html",
        "annotated.html",
        "files.html",
        "globals.html",
    ):
        match = re.search(rf'"{re.escape(url)}":\[(.*?)\]', text)
        if match:
            paths[url] = tuple(int(part) for part in match.group(1).split(",") if part)
    return paths


def _patch_navtree_indexes(
    html_dir: Path,
    old_paths: dict[str, tuple[int, ...]],
) -> None:
    path_map = {}
    root_index = 0
    for url in (
        "quick_start.html",
        "installation.html",
        "examples_recipes.html",
        "api_surface.html",
    ):
        if url in old_paths:
            path_map[old_paths[url]] = (root_index,)
            root_index += 1

    api_index = 0
    for url in (
        "annotated.html",
        "files.html",
        "globals.html",
    ):
        if url in old_paths:
            path_map[old_paths[url]] = (root_index, api_index)
            api_index += 1
    ordered = sorted(path_map.items(), key=lambda item: len(item[0]), reverse=True)

    def replace(match: re.Match[str]) -> str:
        prefix = match.group(1)
        current = tuple(int(part) for part in match.group(2).split(",") if part)
        suffix = match.group(3)
        for old, new in ordered:
            if current[: len(old)] == old:
                updated = new + current[len(old) :]
                return f"{prefix}[{','.join(str(part) for part in updated)}]{suffix}"
        return match.group(0)

    pattern = re.compile(r"(:\s*)\[([0-9,]+)\]([,\n])")
    for path in sorted(html_dir.glob("navtreeindex*.js")):
        text = path.read_text()
        path.write_text(pattern.sub(replace, text))


def _patch_menudata(html_dir: Path) -> None:
    menu_path = html_dir / "menudata.js"
    text = menu_path.read_text()

    api_children = []
    for title in (
        "Data Structures",
        "Classes",
        "Files",
        "Globals",
    ):
        child = _try_extract_menu_object(text, title)
        if child is not None:
            api_children.append(child)

    menu = f'''var menudata={{children:[
{{text:"Quick Start",url:"quick_start.html"}},
{{text:"Installation",url:"installation.html"}},
{{text:"Examples and Recipes",url:"examples_recipes.html"}},
{{text:"API Surface",url:"api_surface.html"}},
{{text:"API Reference",url:"files.html",children:[
{",\n".join(api_children)}]}}]}}
'''

    text = re.sub(r"var menudata=\{children:\[.*?\]\}\s*$", menu, text, count=1, flags=re.S)
    menu_path.write_text(text)


def _patch_header_badges(html_dir: Path) -> None:
    badges = '''<span class="libscid-header-badges">
     <a href="https://github.com/bahmanm/libscid">
      <img src="https://img.shields.io/badge/source_code-blue?style=flat&amp;logo=github&amp;labelColor=gray" alt="Source on GitHub"/>
     </a>
     <img src="https://img.shields.io/badge/GPLv2-blue?style=flat&amp;logo=gnu&amp;labelColor=gray" alt="License GPL v2"/>
    </span>'''
    needle = '<div id="projectbrief">Chess applications made easy.</div>'
    replacement = f"{needle}\n    {badges}"
    for path in sorted(html_dir.glob("*.html")):
        text = path.read_text()
        if "libscid-header-badges" in text:
            continue
        if needle not in text:
            continue
        path.write_text(text.replace(needle, replacement, 1))


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: patch_navigation.py <doxygen-html-dir>", file=sys.stderr)
        return 2

    html_dir = Path(sys.argv[1])
    old_paths = _read_navtree_index_paths(html_dir)
    _patch_navtree(html_dir)
    _patch_navtree_indexes(html_dir, old_paths)
    _patch_menudata(html_dir)
    _patch_header_badges(html_dir)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
