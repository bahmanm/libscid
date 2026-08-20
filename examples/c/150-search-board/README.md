# 150 Search Board

This example demonstrates board structure search filtering (`scid_database_search_board()`).
It configures `scid_search_board_criteria` using matching modes like `SCID_BOARD_SEARCH_MATCH_EXACT`,
`SCID_BOARD_SEARCH_MATCH_PAWNS`, or `SCID_BOARD_SEARCH_MATCH_FILES`, and searches a database
for games that match the specified board layout options.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/c/150-search-board -B _build/examples/c/150-search-board -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/c/150-search-board
ctest --test-dir _build/examples/c/150-search-board --output-on-failure
```
