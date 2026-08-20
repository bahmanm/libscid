# 020 Author PGN

This example authors a PGN game through the C ABI. It creates a blank game
with `scid_game_create_blank()`, uses immutable game cursors to add moves,
comments, NAGs and a variation, then writes PGN twice: once with default export
options and once with `scid_game_pgn_options` for symbolic NAGs and
mainline-only output.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/c/020-author-pgn -B _build/examples/c/020-author-pgn -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/c/020-author-pgn
ctest --test-dir _build/examples/c/020-author-pgn --output-on-failure
```
