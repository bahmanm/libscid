# 090 List Database

This example creates an in-memory database, parses and adds PGN games, and
lists each game using lightweight database metadata functions instead of
loading every full game.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/c/090-list-database -B _build/examples/c/090-list-database -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/c/090-list-database
ctest --test-dir _build/examples/c/090-list-database --output-on-failure
```
