# libscid Examples

This directory contains examples that use the C ABI from `libscid`.

- `000-python-bindings/`: a small Python `ctypes` wrapper and PGN roundtrip script.
- `010-edit-pgn/`: parse, inspect, edit, and write a PGN game through the C ABI.
- `020-author-pgn/`: author a PGN game with moves, comments, NAGs, and a variation.
- `030-nonstandard-start/`: create a game from a FEN start position and export it.
- `040-navigate-pgn/`: navigate mainline moves and variations with a movetext cursor.
- `050-position-and-moves/`: apply moves to a position and inspect the resulting state.
- `060-mutate-pgn/`: edit move metadata and variations in an existing PGN game.
- `070-eco-lookup/`: load an ECO book and classify a position.
- `080-memory-database/`: create an in-memory database and add games to it.
- `090-list-database/`: list database games using lightweight metadata calls.
- `100-edit-database/`: replace games and toggle the deleted marker.
- `110-persistent-database/`: create, close, and reopen a SCID5 database.
