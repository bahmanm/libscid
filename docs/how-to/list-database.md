# How to Enumerate Games via Lightweight Metadata

This guide shows how to efficiently iterate through and display game records in a libscid database using lightweight metadata inspection functions rather than incurring the overhead of materialising full game entities.

---

## 1. Overview of Symbols

- `scid_database_game_number_get`: Retrieves the ordinal number of a game in the database.
- `scid_database_game_deleted_get`: Checks whether a game has been marked as deleted (tombstoned).
- `scid_database_game_tag_get`: Queries header tags directly from the database index without loading the game.
- `scid_database_game_date_get`: Extracts the date string associated with the game record.
- `scid_database_game_result_get`: Retrieves the game result descriptor (`"1-0"`, `"0-1"`, `"1/2-1/2"`, `"*"`).
- `scid_database_game_eco_get`: Reads the cached ECO opening code.
- `scid_database_game_halfmove_count_get`: Retrieves the total number of plies (half-moves) played.

---

## 2. Complete Recipe

```c
--8<-- "examples/c/090-list-database/main.c"
```

---

## 3. Key Concepts and Patterns

### Lightweight Index Queries

When building game list tables or paginating through large collections containing millions of games, calling `scid_database_game_get` for every row is inefficient. Instead, query the metadata index directly:

```c
for (size_t index = 1; index <= game_count; ++index)
{
    char white[128], black[128], date[32], result[16];
    size_t white_size = 0, black_size = 0, date_size = 0, result_size = 0;
    size_t halfmoves = 0;
    int deleted = 0;

    scid_database_game_deleted_get(database, index, &deleted);
    scid_database_game_tag_get(database, index, "White", white, sizeof(white), &white_size);
    scid_database_game_tag_get(database, index, "Black", black, sizeof(black), &black_size);
    scid_database_game_date_get(database, index, date, sizeof(date), &date_size);
    scid_database_game_result_get(database, index, result, sizeof(result), &result_size);
    scid_database_game_halfmove_count_get(database, index, &halfmoves);

    printf("%zu: %.*s vs %.*s [%.*s] (Plies: %zu)%s\n",
        index, (int)white_size, white, (int)black_size, black,
        (int)result_size, result, halfmoves, deleted ? " [DELETED]" : "");
}
```
