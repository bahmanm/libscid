# How to Batch Import and Export PGN

This guide shows how to ingest multi-game PGN streams directly into a libscid database and export stored games back into formatted PGN text using the C ABI.

---

## 1. Overview of Symbols

- `scid_database_import_pgn`: Ingests a raw PGN buffer containing one or more games, appending them into the database collection.
- `scid_database_game_export_pgn`: Serialises an individual game record identified by database index into PGN format.
- `scid_game_pgn_options`: Optional formatting configuration handle for exports.

---

## 2. Complete Recipe

```c
--8<-- "examples/c/120-import-export-pgn/main.c"
```

---

## 3. Key Concepts and Patterns

### Streaming PGN Ingestion

Rather than manually parsing individual games in application code, `scid_database_import_pgn` consumes multi-game PGN buffers directly:

```c
const char* multi_game_pgn = "...";
size_t imported_count = 0;
char diagnostic[1024];
size_t diagnostic_size = 0;

scid_database_import_pgn(
    database,
    multi_game_pgn,
    strlen(multi_game_pgn),
    &imported_count,
    diagnostic,
    sizeof(diagnostic),
    &diagnostic_size);

printf("Imported %zu games\n", imported_count);
```
