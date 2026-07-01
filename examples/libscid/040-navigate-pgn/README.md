# 040 Navigate PGN

This example parses a PGN game through the C ABI, walks mainline moves with a
game cursor, inspects SAN/comments/NAGs, and enters a variation.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/libscid/040-navigate-pgn -B _build/examples/libscid/040-navigate-pgn -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/libscid/040-navigate-pgn
ctest --test-dir _build/examples/libscid/040-navigate-pgn --output-on-failure
```
