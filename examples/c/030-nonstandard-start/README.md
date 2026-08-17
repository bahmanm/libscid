# 030 Nonstandard Start

This example creates a position from FEN, creates a blank game from that start
position with `scid_game_create_blank()`, inspects the generated `FEN` tag, and
exports PGN.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/c/030-nonstandard-start -B _build/examples/c/030-nonstandard-start -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/c/030-nonstandard-start
ctest --test-dir _build/examples/c/030-nonstandard-start --output-on-failure
```
