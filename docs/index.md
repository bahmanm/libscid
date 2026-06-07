\htmlonly
<span class="libscid-landing-logo">
  <img src="libscid-16x10.png" alt="libscid logo" />
</span>
\endhtmlonly

libscid is a C++20 library extracted from Scid/ScidUp for applications that need
chess game, PGN, database, ECO and spelling functionality without depending on a
desktop application.

*The generated API reference documents installed public headers only. Internal
codec, storage and test-only headers are intentionally excluded from this site.*

## Quick Start

@ref quick_start "Quick Start" shows how to compile a minimal consumer with
`g++`, Make or CMake.

## Installation

@ref installation "Installation" covers release archives, source installation,
package contents and local documentation builds.

## Examples and Recipes

@ref examples_recipes "Examples and Recipes" maps common tasks to the standalone
example projects.

## Public Libraries

- `LibScidCpp::Core` provides board primitives, positions, moves, games, movetext,
  PGN support, notation and shared chess value types.
- `LibScidCpp::Database` provides SCID-family database storage, indexes, namebases,
  filters, searches, opening-tree statistics and game load/save workflows.
- `LibScidCpp::Eco` provides ECO code parsing and opening-book lookup.
- `LibScidCpp::Spelling` provides name normalisation and spelling-file support for
  database-oriented workflows.

\htmlonly
<div style="display: none">
\endhtmlonly

- @subpage architecture "Architecture and Diagrams"
- @subpage api_surface "API Surface"
- @subpage quick_start "Quick Start"
- @subpage installation "Installation"
- @subpage examples_recipes "Examples and Recipes"

\htmlonly
</div>
\endhtmlonly

## Architecture and Diagrams

@ref architecture "Architecture and Diagrams" collects domain-centred views of
the public model, including the editable game aggregate and its PGN boundaries.

## Where To Start

- Use `scid::core::Game` for an editable chess game model.
- Use `scid::core::Position` for board state, legal move handling and FEN/UCI
  position support.
- Use `scid::database::scidBaseT` for database sessions.
- Use `scid::eco::Book` for ECO opening lookup.
- Use `scid::spelling::NameNormalizer` for spelling-file-backed name handling.
