# How to Parse and Edit PGN Tags

This guide shows how to parse a PGN string into a game handle, inspect existing header tags, modify or remove tags, and export the updated game back to PGN using the libscid C ABI.

---

## 1. Overview of Symbols

- `scid_position_create_from_fen`: Initialises the starting board position against which moves are validated.
- `scid_game_create`: Parses a PGN text buffer and constructs a game handle.
- `scid_game_tag_count_get`: Retrieves the number of header tags present on the game.
- `scid_game_tag_at_get`: Reads the key name and string value of a tag at a specified 0-based index using caller-allocated buffers.
- `scid_game_tag_set`: Sets or overwrites a tag value.
- `scid_game_tag_remove`: Deletes a tag from the game header.
- `scid_game_to_pgn`: Serialises the game tree and headers back to a PGN-formatted string.

---

## 2. Complete Recipe

```c
--8<-- "examples/c/010-edit-pgn/main.c"
```

---

## 3. Step-by-Step Breakdown

### Parsing the Game

Initialise a starting position from standard FEN, then pass the PGN text buffer to `scid_game_create`. If syntax errors occur during parsing, diagnostic text is written into the caller's diagnostic buffer:

```c
scid_position* position = NULL;
scid_position_create_from_fen(start_fen, &position);

scid_game* game = NULL;
char diagnostic[1024];
size_t diagnostic_size = 0;

scid_error err = scid_game_create(
    position, pgn, strlen(pgn), &game, diagnostic, sizeof(diagnostic), &diagnostic_size);
```

### Inspecting Tags

Tags can be enumerated using 0-based indices. Call `scid_game_tag_count_get` followed by `scid_game_tag_at_get`:

```c
size_t tag_count = 0;
scid_game_tag_count_get(game, &tag_count);

for (size_t i = 0; i < tag_count; ++i)
{
    char name[64];
    char value[256];
    size_t name_size = 0;
    size_t value_size = 0;

    scid_game_tag_at_get(
        game, i, name, sizeof(name), &name_size, value, sizeof(value), &value_size);
    printf("%.*s: %.*s\n", (int)name_size, name, (int)value_size, value);
}
```

### Modifying and Exporting

Use `scid_game_tag_set` to add or update tags, `scid_game_tag_remove` to delete tags, and `scid_game_to_pgn` to export the modified game:

```c
scid_game_tag_set(game, "Annotator", "C ABI example");

int removed = 0;
scid_game_tag_remove(game, "EventDate", &removed);

char encoded[4096];
size_t encoded_size = 0;
scid_game_to_pgn(game, NULL, encoded, sizeof(encoded), &encoded_size);
```

Remember to release handles with `scid_position_free(position)` and `scid_game_free(game)`.
