libscid is a C ABI library extracted from Scid/ScidUp for applications that need
chess game, PGN, database and ECO functionality without depending on a desktop
application.

*The generated API reference documents the installed public C header. The C++
implementation is intentionally not part of the installed consumer contract.*

## Quick Start

[Quick Start](quick-start.md) shows how to compile a minimal C ABI consumer
with `cc`, Make or CMake.

## Installation

[Installation](installation.md) covers release archives, source installation,
package contents and local documentation builds.

## Examples and Recipes

[Examples and Recipes](examples-recipes.md) maps common tasks to the standalone
C ABI example projects.

## Public Library

- `LibScid::LibScid` provides the stable C ABI declared by `scid/scid.h`.

## Where To Start

- Use `scid_position` for board state, legal move handling and FEN/UCI/SAN
  helpers.
- Use `scid_game`, `scid_game_cursor` and `scid_game_pgn_options` for PGN
  import, editing, traversal, merge and export workflows.
- Use `scid_database` for in-memory and persistent Scid database workflows.
- Use `scid_eco_book` for ECO opening lookup.
