# 110 Persistent Database

This example creates a SCID5 database on disk, adds a PGN game, closes the
database, reopens it, reads the persisted metadata, and removes the generated
SCID5 files.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/libscid/110-persistent-database -B _build/examples/libscid/110-persistent-database -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/libscid/110-persistent-database
ctest --test-dir _build/examples/libscid/110-persistent-database --output-on-failure
```
