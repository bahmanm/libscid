# 130 Search Headers

This example demonstrates database header search filtering (`scid_database_search_headers()`).
It builds a database with multiple games, applies header search criteria
(`scid_search_header_criteria`) to populate a destination filter, queries matching game
counts, and paginates sorted game indices through the C ABI.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/c/130-search-headers -B _build/examples/c/130-search-headers -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/c/130-search-headers
ctest --test-dir _build/examples/c/130-search-headers --output-on-failure
```
