# How to Author Games with Moves, Comments, and Variations

This guide demonstrates how to construct a chess game programmatically through the libscid C ABI by creating a blank game, appending moves via immutable cursors, attaching comments and Numeric Annotation Glyphs (NAGs), creating branching variations, and configuring custom PGN export options.

---

## 1. Overview of Symbols

- `scid_game_create_blank`: Constructs an empty game initialised to the standard start position or an arbitrary custom position.
- `scid_game_cursor_create`: Allocates an initial navigation cursor positioned at the root of the game tree.
- `scid_game_cursor_move_add`: Applies a move from the current cursor position, appending it to the game tree and producing a new cursor pointing to the resulting board state.
- `scid_game_cursor_comment_set`: Attaches an explanatory annotation or text comment to the move referenced by the cursor.
- `scid_game_cursor_nag_add`: Attaches a standard Numeric Annotation Glyph (e.g. `$1` for "!", `$2` for "?") to the move.
- `scid_game_cursor_variation_add`: Branches from the current move, creating an alternative line of play.
- `scid_game_pgn_options_create`: Allocates a configuration handle to control PGN export formatting (such as symbolic NAGs or omitting variations).
- `scid_game_to_pgn`: Serialises the complete game structure to PGN.

---

## 2. Complete Recipe

```c
--8<-- "examples/c/020-author-pgn/main.c"
```

---

## 3. Key Concepts and Patterns

### Cursor Handover Pattern

Because `scid_game_cursor` instances are immutable, each navigational or structural modification produces a newly allocated cursor:

```c
static int
take_cursor(
    scid_game_cursor** cursor,
    scid_game_cursor** next_cursor)
{
    if (next_cursor == NULL || *next_cursor == NULL)
    {
        return 0;
    }

    scid_game_cursor_free(*cursor);
    *cursor = *next_cursor;
    *next_cursor = NULL;
    return 1;
}
```

This helper ensures clean ownership handover and avoids cursor leaks during successive move applications.

### Adding Comments and NAGs

```c
scid_game_cursor_comment_set(cursor, "Initiates the game");
scid_game_cursor_nag_add(cursor, 1); /* $1 = ! (good move) */
```

### Mainline vs Variation Export

By default, `scid_game_to_pgn` exports the mainline along with all recursive sub-variations. You can configure `scid_game_pgn_options` to emit mainline-only text or format NAGs as symbolic punctuation (`!`, `?`) rather than numeric notation (`$1`, `$2`):

```c
scid_game_pgn_options* options = NULL;
scid_game_pgn_options_create(&options);
scid_game_pgn_options_use_symbols_set(options, 1);
scid_game_pgn_options_export_variations_set(options, 0); /* mainline only */

scid_game_to_pgn(game, options, buffer, sizeof(buffer), &size);
scid_game_pgn_options_free(options);
```
