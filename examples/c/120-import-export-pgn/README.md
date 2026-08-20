# 120 Import Export PGN

This example imports a PGN string containing one or more games directly into
an in-memory database and exports one stored game back to PGN through the C ABI.
Database export uses the default PGN policy; use `scid_database_game_get()`
followed by `scid_game_to_pgn()` when custom `scid_game_pgn_options` are
needed.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/c/120-import-export-pgn -B _build/examples/c/120-import-export-pgn -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/c/120-import-export-pgn
ctest --test-dir _build/examples/c/120-import-export-pgn --output-on-failure
```
