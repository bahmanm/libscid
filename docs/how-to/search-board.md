# How to Search Games by Board and Pawn Structure

This guide shows how to search a libscid database for games matching pawn structures and material distributions (such as French Defence pawn chains) using the C ABI.

---

## 1. Overview of Symbols

- `scid_search_board_criteria_create`: Allocates a board search criteria builder handle.
- `scid_search_board_criteria_target_position_set`: Specifies the reference board position against which material and structure comparisons are made.
- `scid_search_board_criteria_pawns_set`: Configures whether pawn structures must match identically.
- `scid_search_board_criteria_files_set`: Configures file-matching flexibility.
- `scid_database_search_board`: Executes the board search across the database collection, populating a filter.
- `scid_search_board_criteria_free`: Releases the search criteria builder handle.

---

## 2. Complete Recipe

```c
--8<-- "examples/c/150-search-board/main.c"
```

---

## 3. Key Concepts and Patterns

### Structural Pawn Searches

```c
scid_position* position = NULL;
scid_position_create_from_fen(french_fen, &position);

scid_search_board_criteria* criteria = NULL;
scid_search_board_criteria_create(&criteria);
scid_search_board_criteria_target_position_set(criteria, position);
scid_search_board_criteria_pawns_set(criteria, 1); /* Match pawn structure */

scid_filter_id filter_id = 0;
size_t matched_count = 0;
scid_database_search_board(database, criteria, &filter_id, &matched_count);

printf("Matched %zu games with similar pawn structure\n", matched_count);

scid_search_board_criteria_free(criteria);
scid_position_free(position);
```
