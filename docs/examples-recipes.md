# Examples and Recipes {#examples_recipes}

The examples are small standalone consumers of an installed libscid package.
They double as concept guides: each one introduces a narrow public workflow and
keeps the surrounding project structure minimal.

Install libscid first, then configure the example tree with `CMAKE_PREFIX_PATH`
pointing at the installation prefix:

```sh
cmake -S examples -B _build/examples -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples
ctest --test-dir _build/examples --output-on-failure
```

The same examples are included in release archives under
`share/doc/libscid-cpp/examples`.

## First Consumer

- `examples/000-minimal-consumer`: discover the installed CMake package, link
  `LibScidCpp::Core`, and run a minimal program.

## Games, Positions And PGN

- `examples/010-pgn-roundtrip`: parse a PGN game into `scid::core::Game`, read
  metadata and encode it back to PGN.
- `examples/020-position-and-moves`: start from the normal chess position,
  apply coordinate moves, print FEN and generate legal replies.
- `examples/030-build-game`: build a `scid::core::Game` programmatically and
  encode it as PGN.

## Databases

- `examples/040-open-database`: open a SCID5 database in read-only mode and
  print a summary of the first game.
- `examples/070-list-database`: create a sort cache and list games in date
  order.
- `examples/080-search-position`: load a position from a game and search the
  database for games that reached that position.
- `examples/090-opening-tree`: build opening-tree statistics for games matching
  a searched position.
- `examples/100-import-pgn`: import PGN into an in-memory database and load an
  imported game back through the database API.
- `examples/110-database-stats`: read aggregate database statistics such as
  dates, results, ratings and ECO coverage.

## ECO And Spelling

- `examples/050-eco-classification`: load an ECO book and classify an opening
  position.
- `examples/060-spelling`: load a spelling file, correct a player-name alias
  and read player metadata.

## Fixtures

The examples use small fixtures under `examples/fixtures`:

- `dr-lasker/dr-lasker.{si5,sg5,sn5}`: a compact SCID5 database.
- `import.pgn`: PGN input used by the import example.
- `mini.eco`: a small ECO book.
- `mini.ssp`: a small spelling file.

These fixtures are intentionally tiny and are part of the release examples so
that the example test suite can run without external chess data.
