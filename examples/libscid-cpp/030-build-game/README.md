# 030 Build Game

This example builds a `scid::core::Game` programmatically and encodes it as PGN.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/libscid-cpp/030-build-game -B _build/examples/libscid-cpp/030-build-game -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/libscid-cpp/030-build-game
ctest --test-dir _build/examples/libscid-cpp/030-build-game --output-on-failure
```
