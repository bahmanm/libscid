# 100 Edit Database

This example creates an in-memory database, parses PGN games, replaces an
existing stored game, marks the game deleted, and then clears the deleted
marker through the C ABI.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/libscid/100-edit-database -B _build/examples/libscid/100-edit-database -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/libscid/100-edit-database
ctest --test-dir _build/examples/libscid/100-edit-database --output-on-failure
```
