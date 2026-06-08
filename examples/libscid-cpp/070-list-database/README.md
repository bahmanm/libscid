# 070 List Database

This example opens the Dr Lasker SCID5 database, creates a sort cache, and lists
the first few games in date order.

It reuses the SCID5 database from `examples/fixtures/dr-lasker/dr-lasker`.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/libscid-cpp/070-list-database -B _build/examples/libscid-cpp/070-list-database -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/libscid-cpp/070-list-database
ctest --test-dir _build/examples/libscid-cpp/070-list-database --output-on-failure
```
