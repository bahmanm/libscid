# How to Search Games by Exact Position

This guide shows how to locate games in a libscid database containing an exact board layout matching a target FEN position using the C ABI.

---

## 1. Overview of Symbols

- `scid_position_create_from_fen`: Initialises the target board layout to search for.
- `scid_database_search_position`: Searches the database for all games that reached the given board position at any point in their move trees.
- `scid_database_filter_game_indexes_get`: Returns the list of matching database game records.

---

## 2. Complete Recipe

```c
--8<-- "examples/c/140-search-position/main.c"
```

---

## 3. Key Concepts and Patterns

### Searching for Board Layouts

```c
scid_position* target_position = NULL;
scid_position_create_from_fen(target_fen, &target_position);

scid_filter_id position_filter = 0;
size_t match_count = 0;

scid_database_search_position(
    database,
    target_position,
    &position_filter,
    &match_count);

printf("Found %zu games reaching this position\n", match_count);

scid_position_free(target_position);
```
