# 100 Import PGN

This example opens an in-memory database, imports games from a PGN file, and
loads one imported game back through the database API.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/libscid-cpp/100-import-pgn -B _build/examples/libscid-cpp/100-import-pgn -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/libscid-cpp/100-import-pgn
ctest --test-dir _build/examples/libscid-cpp/100-import-pgn --output-on-failure
```
