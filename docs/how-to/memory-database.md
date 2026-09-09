# How to Work with In-Memory Databases

This guide shows how to instantiate and operate transient in-memory databases with the libscid C ABI: storing games, retrieving records by 1-based index, generating filtered subsets, and applying multi-criteria sorting.

---

## 1. Overview of Symbols

- `scid_database_create_memory`: Allocates a new transient in-memory database.
- `scid_database_is_open`: Checks whether the database handle is active and open.
- `scid_database_game_add`: Appends a game entity into the database.
- `scid_database_game_count_get`: Returns the total number of games stored in the database.
- `scid_database_game_get`: Materialises and loads a complete `scid_game` from a 1-based game index.
- `scid_database_filter_create`: Creates a filtered view across the database.
- `scid_database_sort`: Sorts database records according to specified criteria.
- `scid_database_free`: Closes and releases the database and its allocated resources.

---

## 2. Complete Recipe

```c
--8<-- "examples/c/080-memory-database/main.c"
```

---

## 3. Key Concepts and Patterns

### Database Indices are 1-Based

Following Scid's heritage, database game indices in libscid are 1-based (i.e. games range from index 1 to `game_count` inclusive):

```c
scid_database* database = NULL;
scid_database_create_memory("example", &database);

/* Add game */
scid_database_game_add(database, game);

/* Retrieve game at index 1 */
scid_game* loaded_game = NULL;
scid_database_game_get(database, 1, &loaded_game);

/* Cleanup */
scid_game_free(loaded_game);
scid_database_free(database);
```
