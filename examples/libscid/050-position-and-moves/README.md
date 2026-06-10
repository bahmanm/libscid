# 050 Position and Moves

This example starts from the standard position through the C ABI, applies SAN
and UCI moves, converts moves between SAN/UCI, prints FEN, and inspects side to
move, move counters, and board contents.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/libscid/050-position-and-moves -B _build/examples/libscid/050-position-and-moves -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/libscid/050-position-and-moves
ctest --test-dir _build/examples/libscid/050-position-and-moves --output-on-failure
```
