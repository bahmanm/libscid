# libscid API Reference {#mainpage}

libscid is a C++20 library extracted from Scid/ScidUp for applications that need
chess game, PGN, database, ECO and spelling functionality without depending on a
desktop application.

## Public Libraries

- `LibScid::Core` provides board primitives, positions, moves, games, movetext,
  PGN support, notation and shared chess value types.
- `LibScid::Database` provides SCID-family database storage, indexes, namebases,
  filters, searches, opening-tree statistics and game load/save workflows.
- `LibScid::Eco` provides ECO code parsing and opening-book lookup.
- `LibScid::Spelling` provides name normalisation and spelling-file support for
  database-oriented workflows.

## Starting Points

- Use `scid::core::Game` for an editable chess game model.
- Use `scid::core::Position` for board state, legal move handling and FEN/UCI
  position support.
- Use `scid::database::scidBaseT` for database sessions.
- Use `scid::eco::Book` for ECO opening lookup.
- Use `scid::spelling::NameNormalizer` for spelling-file-backed name handling.

## Boundaries

The generated API reference documents installed public headers only. Internal
codec, storage and test-only headers are intentionally excluded from this site.
