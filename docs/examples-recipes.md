# Examples and Recipes

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

## Language Bindings (Python)

- `examples/python/010-edit-pgn`: parse, inspect, edit header tags, and export PGN.
- `examples/python/020-author-pgn`: author games with moves, comments, NAGs, and variations.
- `examples/python/030-navigate-pgn`: traverse variation trees with `Cursor` and iterate movetext.
- `examples/python/040-position-and-arbiter`: position analysis, legal moves, and arbitral draw claims.
- `examples/python/050-list-database`: database opening, metadata inspection, and filter pagination.
- `examples/python/060-search-database`: header criteria, position, and board pawn structure search.

## Games, Positions And PGN

- `examples/c/010-edit-pgn`: parse, inspect, edit and write a PGN game.
- `examples/c/020-author-pgn`: author a PGN game with moves, comments,
  NAGs and a variation, then export both default PGN and an options-controlled
  mainline form.
- `examples/c/030-nonstandard-start`: create a game from a FEN start
  position with `scid_game_create_blank()` and export it.
- `examples/c/040-navigate-pgn`: navigate mainline moves and variations
  with immutable game cursors.
- `examples/c/050-position-and-moves`: apply moves to a position, inspect
  the resulting state, and enumerate legal moves.
- `examples/c/060-mutate-pgn`: edit move metadata and merge a source
  game's moves as a variation in an existing PGN game with
  `scid_game_merge_moves()`.

## Databases

- `examples/c/080-memory-database`: create an in-memory database and list
  a filtered game.
- `examples/c/090-list-database`: list database games using lightweight
  metadata calls.
- `examples/c/100-edit-database`: replace games and toggle the deleted
  marker.
- `examples/c/110-persistent-database`: create, close and reopen a SCID5
  database.
- `examples/c/120-import-export-pgn`: import and export PGN through a
  database. Database export uses the default PGN policy; custom export options
  are available by loading the game and calling `scid_game_to_pgn()`.

## ECO

- `examples/c/070-eco-lookup`: load an ECO book and classify a position.

## Fixtures

The examples use small fixtures under `examples/fixtures`:

- `dr-lasker/dr-lasker.{si5,sg5,sn5}`: a compact SCID5 database.
- `import.pgn`: PGN input used by the import example.
- `mini.eco`: a small ECO book.
- `mini.ssp`: a small spelling file.

These fixtures are intentionally tiny and are part of the release examples so
that the example test suite can run without external chess data.
