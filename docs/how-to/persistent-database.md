# How to Create and Open Persistent SCID5 Databases

This guide explains how to create, persist, flush, and reopen native SCID5 database collections on disk using the libscid C ABI.

---

## 1. Overview of Symbols

- `scid_database_create_scid5`: Creates a new persistent SCID5 database file triad on disk.
- `scid_database_open_scid5`: Opens an existing SCID5 database collection.
- `scid_database_save`: Flushes uncommitted in-memory index updates, name records, and game bodies to disk storage.
- `scid_database_game_add`: Appends games into the persistent database.
- `scid_database_free`: Synchronises remaining updates and closes file handles.

---

## 2. Complete Recipe

```c
--8<-- "examples/c/110-persistent-database/main.c"
```

---

## 3. Key Concepts and Patterns

### The SCID5 File Triad

SCID5 databases consist of three complementary binary files sharing a base path prefix:

- `.si5`: Game index storing compact metadata, offsets, and header flags.
- `.sg5`: Compressed binary move representations and game body data.
- `.sn5`: Centralised namebase containing player names, event titles, and sites.

```c
/* Create a new SCID5 database at ./my_games */
scid_database* db = NULL;
scid_database_create_scid5("./my_games", &db);

/* Add games */
scid_database_game_add(db, game, "W");

/* Flush to disk */
scid_database_save(db);
scid_database_free(db);

/* Reopen existing database */
scid_database_open_scid5("./my_games", &db);
/* ... read records ... */
scid_database_free(db);
```
