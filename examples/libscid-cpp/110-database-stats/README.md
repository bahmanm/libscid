# 110 Database Stats

This example opens the Dr Lasker SCID5 database and reads aggregate database
statistics: date range, result buckets, rating coverage, and ECO coverage.

It reuses the SCID5 database from `examples/fixtures/dr-lasker/dr-lasker`.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/libscid-cpp/110-database-stats -B _build/examples/libscid-cpp/110-database-stats -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/libscid-cpp/110-database-stats
ctest --test-dir _build/examples/libscid-cpp/110-database-stats --output-on-failure
```
