# libscid Examples

This directory contains examples that use the C ABI from `libscid`.

- `000-python-bindings/`: a small Python `ctypes` wrapper and default PGN roundtrip script.
- `010-edit-pgn/`: parse with `scid_game_create()`, inspect/edit tags, and write PGN.
- `020-author-pgn/`: author a game and export with default and custom PGN options.
- `030-nonstandard-start/`: create a blank game from a FEN start position and export it.
- `040-navigate-pgn/`: navigate mainline moves and variations with immutable game cursors.
- `050-position-and-moves/`: apply moves to a position, inspect the resulting state, and enumerate legal moves.
- `060-mutate-pgn/`: edit move metadata and merge source-game moves into an existing game.
- `070-eco-lookup/`: load an ECO book and classify a position.
- `080-memory-database/`: create an in-memory database and list a filtered game.
- `090-list-database/`: list database games using lightweight metadata calls.
- `100-edit-database/`: replace games and toggle the deleted marker.
- `110-persistent-database/`: create, close, and reopen a SCID5 database.
- `120-import-export-pgn/`: import and export PGN through a database.
- `130-search-headers/`: query database games using header criteria and filters.
- `140-search-position/`: search database games for exact board positions and chain search filters.
- `150-search-board/`: filter database games by board structure and pawn layout criteria.
