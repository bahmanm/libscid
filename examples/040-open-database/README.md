# 040 Open Database

This example opens a SCID5 database in read-only mode and prints a small summary
of its first game.

It expects the shared SCID5 fixture at `examples/fixtures/dr-lasker/dr-lasker`,
made from the three files `dr-lasker.si5`, `dr-lasker.sg5`, and `dr-lasker.sn5`.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/040-open-database -B _build/examples/040-open-database -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/040-open-database
ctest --test-dir _build/examples/040-open-database --output-on-failure
```
