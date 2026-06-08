# 010 PGN Roundtrip

This example parses the famous Lasker-Capablanca, St Petersburg 1914 game into
a `scid::core::Game`, reads a few fields, and encodes it back to PGN.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/libscid-cpp/010-pgn-roundtrip -B _build/examples/libscid-cpp/010-pgn-roundtrip -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/libscid-cpp/010-pgn-roundtrip
ctest --test-dir _build/examples/libscid-cpp/010-pgn-roundtrip --output-on-failure
```
