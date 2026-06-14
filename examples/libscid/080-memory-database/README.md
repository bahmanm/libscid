# 080 Memory Database

This example creates an in-memory database, parses a PGN game, adds it to the
database, loads it back, and reads the database game count through the C ABI.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/libscid/080-memory-database -B _build/examples/libscid/080-memory-database -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/libscid/080-memory-database
ctest --test-dir _build/examples/libscid/080-memory-database --output-on-failure
```
