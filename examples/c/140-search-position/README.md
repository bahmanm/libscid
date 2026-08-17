# 140 Search Position

This example demonstrates position search filtering (`scid_database_search_position()`) and filter chaining.
It creates a target position from a FEN string, searches a database for games that reach that exact
board position at any ply, and uses filter chaining to refine search results across multiple stages.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/c/140-search-position -B _build/examples/c/140-search-position -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/c/140-search-position
ctest --test-dir _build/examples/c/140-search-position --output-on-failure
```
