# 010 Edit PGN

This example parses a PGN game with `scid_game_create()`, inspects its header
tags, edits metadata, removes a supplemental tag, and writes the game back to
PGN with `scid_game_to_pgn(..., NULL, ...)`.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/c/010-edit-pgn -B _build/examples/c/010-edit-pgn -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/c/010-edit-pgn
ctest --test-dir _build/examples/c/010-edit-pgn --output-on-failure
```
