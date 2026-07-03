# Examples and Recipes {#examples_recipes}

The examples are small standalone consumers of an installed libscid package.
They double as concept guides: each one introduces a narrow C ABI workflow and
keeps the surrounding project structure minimal.

Install libscid first, then configure the example tree with `CMAKE_PREFIX_PATH`
pointing at the installation prefix:

```sh
cmake -S examples -B _build/examples -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples
ctest --test-dir _build/examples --output-on-failure
```

The same examples are included in release archives under
`share/doc/libscid/examples`.

## Language Bindings

- `examples/libscid/000-python-bindings`: load the shared C ABI from Python
  with `ctypes`, create a game with `scid_game_create()` and roundtrip PGN
  through the default export path.

## Games, Positions And PGN

- `examples/libscid/010-edit-pgn`: parse, inspect, edit and write a PGN game.
- `examples/libscid/020-author-pgn`: author a PGN game with moves, comments,
  NAGs and a variation, then export both default PGN and an options-controlled
  mainline form.
- `examples/libscid/030-nonstandard-start`: create a game from a FEN start
  position with `scid_game_create_blank()` and export it.
- `examples/libscid/040-navigate-pgn`: navigate mainline moves and variations
  with immutable game cursors.
- `examples/libscid/050-position-and-moves`: apply moves to a position and
  inspect the resulting state.
- `examples/libscid/060-mutate-pgn`: edit move metadata and merge a source
  game's moves as a variation in an existing PGN game with
  `scid_game_merge_moves()`.

## Databases

- `examples/libscid/080-memory-database`: create an in-memory database and list
  a filtered game.
- `examples/libscid/090-list-database`: list database games using lightweight
  metadata calls.
- `examples/libscid/100-edit-database`: replace games and toggle the deleted
  marker.
- `examples/libscid/110-persistent-database`: create, close and reopen a SCID5
  database.
- `examples/libscid/120-import-export-pgn`: import and export PGN through a
  database. Database export uses the default PGN policy; custom export options
  are available by loading the game and calling `scid_game_to_pgn()`.

## ECO

- `examples/libscid/070-eco-lookup`: load an ECO book and classify a position.

## Fixtures

The examples use small fixtures under `examples/fixtures`:

- `dr-lasker/dr-lasker.{si5,sg5,sn5}`: a compact SCID5 database.
- `import.pgn`: PGN input used by the import example.
- `mini.eco`: a small ECO book.
- `mini.ssp`: a small spelling file.

These fixtures are intentionally tiny and are part of the release examples so
that the example test suite can run without external chess data.
