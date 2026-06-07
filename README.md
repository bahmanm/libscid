<table>
  <tr>
    <td>
      <img src="docs/assets/img/libscid-1x1.png" width="220" height="220" alt="libscid Logo - Winged Pawn"/>
    </td>
    <td align="center">
      <h1>libscid</h1>
      <h3>A C++20 library for chess games, PGN, Scid databases, ECO codes and spelling files.</h3>
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
analysis pipelines and GUIs that need mature PGN, game, database, ECO and name
normalisation functionality without depending on a desktop application.

## What It Provides

- `LibScidCpp::Core`: board state, moves, games, movetext, notation and PGN.
- `LibScidCpp::Database`: Scid database sessions, indexes, names, filters,
  searches, opening-tree statistics and game load/save workflows.
- `LibScidCpp::Eco`: ECO code values and opening-position classification.
- `LibScidCpp::Spelling`: spelling-file-backed name correction and player metadata.

## Start Here

- [Quick Start](https://libscid.bahmanm.com/quick_start.html): compile a small
  consumer with `g++`, Make or CMake.
- [Installation](https://libscid.bahmanm.com/installation.html): install a
  release archive or build and install from source.
- [Examples and Recipes](https://libscid.bahmanm.com/examples_recipes.html):
  choose an example by task.
- [Architecture and Diagrams](https://libscid.bahmanm.com/architecture.html):
  understand the public model before drilling into API details.
- [API Documentation](https://libscid.bahmanm.com/): browse the generated public
  header reference.

## Minimal CMake Consumer

```cmake
cmake_minimum_required( VERSION 3.23 )

project( libscid-consumer LANGUAGES CXX )

find_package( libscid-cpp CONFIG REQUIRED )

add_executable( libscid-consumer main.cpp )
target_compile_features( libscid-consumer PRIVATE cxx_std_20 )
target_link_libraries( libscid-consumer PRIVATE LibScidCpp::Core )
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
