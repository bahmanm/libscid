# 020 Position and Moves

This example starts from the standard chess position, applies coordinate moves,
prints the resulting FEN, and generates legal replies from that position.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/020-position-and-moves -B _build/examples/020-position-and-moves -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/020-position-and-moves
ctest --test-dir _build/examples/020-position-and-moves --output-on-failure
```
