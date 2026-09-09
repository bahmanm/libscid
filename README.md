<table>
  <tr>
    <td>
      <img src="docs/assets/img/libscid-1x1.png" width="220" height="220" alt="libscid Logo - Winged Pawn"/>
    </td>
    <td align="center">
      <h1>libscid</h1>
      <h3>A C ABI library for chess games, PGN, Scid databases and ECO codes.</h3>
      <img src="https://github.com/bahmanm/libscid/actions/workflows/ci.yml/badge.svg" alt="CI"/>
      <img src="https://github.com/bahmanm/libscid/actions/workflows/dependabot/dependabot-updates/badge.svg" alt="dependabot"/>
      <a href="https://libscid.bahmanm.com/"><img src="https://img.shields.io/badge/documentation-GitHub%20Pages-blue" alt="Documentation"/></a>
      <a href="examples/"><img src="https://img.shields.io/badge/examples-recipes-green" alt="Examples"/></a>
    </td>
  </tr>
</table>

libscid is a standalone extraction of the library-grade parts of
[Scid](https://sourceforge.net/projects/scid/) and
[ScidUp](https://github.com/bahmanm/scidup). It is intended for chess tools,
analysis pipelines, language bindings and GUIs that need mature PGN, game,
database and ECO functionality without depending on a desktop application.

## What It Provides

- Opaque handles for positions, games, game cursors, PGN export options, ECO
  books and databases.
- PGN import, authoring, immutable cursor traversal, merge and export workflows.
- Board state, legal move handling, FEN, UCI and SAN helpers.
- Scid database creation, opening, listing, editing, import and export.
- ECO code parsing and opening-position lookup.

## Start Here

- [Quick Start](https://libscid.bahmanm.com/tutorials/quick-start/): compile a small
  C ABI consumer with `cc`, Make or CMake.
- [Installation](https://libscid.bahmanm.com/how-to/installation/): install a
  release archive or build and install from source.
- [How-To Guides](https://libscid.bahmanm.com/how-to/): practical task-oriented
  recipes for PGN editing, variation traversal and database queries.
- [Explanation & Architecture](https://libscid.bahmanm.com/explanation/architecture/):
  understand the opaque handles, memory model and subsystem designs.
- [C ABI Reference](https://libscid.bahmanm.com/reference/index_modules/): browse
  the generated public header and symbol specification.

## Minimal CMake Consumer

```cmake
cmake_minimum_required( VERSION 3.23 )

project( libscid-consumer LANGUAGES C )

find_package( libscid CONFIG REQUIRED )

add_executable( libscid-consumer main.c )
set_target_properties(
    libscid-consumer
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF )
target_link_libraries( libscid-consumer PRIVATE LibScid::LibScid )
```

Configure it with `CMAKE_PREFIX_PATH` pointing at a libscid installation:

```sh
cmake -S . -B _build -DCMAKE_PREFIX_PATH=/path/to/libscid
cmake --build _build
```

## Licence

libscid is distributed under the GNU GPL v2; see [COPYING](COPYING).
Unless stated otherwise, modifications and additions in this repository are
licensed under the same terms.
