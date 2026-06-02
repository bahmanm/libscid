# 090 Opening Tree

This example finds the Dr Lasker-Capablanca game in the Dr Lasker SCID5
database, searches for a position from that game, and builds opening-tree
statistics for the matching games.

It reuses the SCID5 database from `examples/fixtures/dr-lasker/dr-lasker`.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/090-opening-tree -B _build/examples/090-opening-tree -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/090-opening-tree
ctest --test-dir _build/examples/090-opening-tree --output-on-failure
```
