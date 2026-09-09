# How to Filter Games by Header Criteria

This guide demonstrates how to construct search criteria to filter games in a libscid database based on metadata tags such as player names, tournament events, dates, and game results using the C ABI.

---

## 1. Overview of Symbols

- `scid_search_header_criteria_create`: Allocates a new header search criteria builder handle.
- `scid_search_header_criteria_player_set`: Filters by player name across White, Black, or both sides.
- `scid_search_header_criteria_result_set`: Filters by game outcome (e.g. `SCID_RESULT_WHITE_WIN`, `SCID_RESULT_DRAW`).
- `scid_database_search_header`: Executes the header query across the database, yielding a persistent `scid_filter_id` and total match count.
- `scid_database_filter_game_indexes_get`: Retrieves the matched 1-based database game indices associated with the filter.
- `scid_search_header_criteria_free`: Releases the search criteria handle.

---

## 2. Complete Recipe

```c
--8<-- "examples/c/130-search-headers/main.c"
```

---

## 3. Key Concepts and Patterns

### Constructing Header Queries

```c
scid_search_header_criteria* criteria = NULL;
scid_search_header_criteria_create(&criteria);

/* Match games played by Kasparov where White won */
scid_search_header_criteria_player_set(criteria, "Kasparov, Garry");
scid_search_header_criteria_result_set(criteria, SCID_RESULT_WHITE_WIN);

scid_filter_id filter_id = 0;
size_t matched_count = 0;
scid_database_search_header(database, criteria, &filter_id, &matched_count);

printf("Found %zu matching games\n", matched_count);

scid_search_header_criteria_free(criteria);
```
