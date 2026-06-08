# 080 Search Position

This example finds the Dr Lasker-Capablanca game in the Dr Lasker SCID5
database, takes a position from its main line, and searches the database for
games that reached that position.

It reuses the SCID5 database from `examples/fixtures/dr-lasker/dr-lasker`.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/libscid-cpp/080-search-position -B _build/examples/libscid-cpp/080-search-position -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/libscid-cpp/080-search-position
ctest --test-dir _build/examples/libscid-cpp/080-search-position --output-on-failure
```
