# How to Replace Games and Manage Deleted Flags

This guide demonstrates how to update game records in-place within a libscid database, replacing game content and toggling deleted (tombstone) status flags without requiring full database rebuilds.

---

## 1. Overview of Symbols

- `scid_database_game_replace`: Replaces an existing game at a specified index with updated moves, headers, and custom flags.
- `scid_database_game_deleted_set`: Sets or unsets the tombstone deleted status flag for a specific game record.
- `scid_database_game_deleted_get`: Queries whether a game is currently flagged as deleted.
- `scid_database_game_get`: Materialises a stored game record along with its user flags string.

---

## 2. Complete Recipe

```c
--8<-- "examples/c/100-edit-database/main.c"
```

---

## 3. Key Concepts and Patterns

### In-Place Replacement and Flag Management

Games can be overwritten with new content or flagged as deleted:

```c
/* Replace existing record */
scid_database_game_replace(database, game_index, updated_game, "Q");

/* Mark game as deleted */
scid_database_game_deleted_set(database, game_index, 1);

/* Check deleted status */
int is_deleted = 0;
scid_database_game_deleted_get(database, game_index, &is_deleted);

/* Restore game by clearing deleted flag */
scid_database_game_deleted_set(database, game_index, 0);
```
