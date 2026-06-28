\htmlonly
<span class="libscid-landing-logo">
  <img src="libscid-16x10.png" alt="libscid logo" />
</span>
\endhtmlonly

libscid is a C ABI library extracted from Scid/ScidUp for applications that need
chess game, PGN, database and ECO functionality without depending on a desktop
application.

*The generated API reference documents the installed public C header. The C++
implementation is intentionally not part of the installed consumer contract.*

## Quick Start

@ref quick_start "Quick Start" shows how to compile a minimal C ABI consumer
with `cc`, Make or CMake.

## Installation

@ref installation "Installation" covers release archives, source installation,
package contents and local documentation builds.

## Examples and Recipes

@ref examples_recipes "Examples and Recipes" maps common tasks to the standalone
C ABI example projects.

## Public Library

- `LibScid::LibScid` provides the stable C ABI declared by `scid/scid.h`.

\htmlonly
<div style="display: none">
\endhtmlonly

- @subpage api_surface "API Surface"
- @subpage quick_start "Quick Start"
- @subpage installation "Installation"
- @subpage examples_recipes "Examples and Recipes"

\htmlonly
</div>
\endhtmlonly

## Where To Start

- Use `scid_position` for board state, legal move handling and FEN/UCI/SAN
  helpers.
- Use `scid_game` for PGN import, editing, traversal and export.
- Use `scid_database` for in-memory and persistent Scid database workflows.
- Use `scid_eco_book` for ECO opening lookup.
