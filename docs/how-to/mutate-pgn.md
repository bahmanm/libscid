# How to Mutate and Merge Move Trees

This guide explains how to structurally alter chess game trees using the libscid C ABI: truncating variations at a specific ply, deleting unwanted sub-variations, and merging moves from an external source game into an existing target game.

---

## 1. Overview of Symbols

- `scid_game_cursor_truncate`: Removes all moves following the cursor along the current variation, cutting the tree short at the active ply.
- `scid_game_cursor_variation_delete`: Deletes a specific sub-variation branching from the current position.
- `scid_game_merge_moves`: Merges moves, variations, comments, and NAGs from a source game into a target game starting at the active cursor position.

---

## 2. Complete Recipe

```c
--8<-- "examples/c/060-mutate-pgn/main.c"
```

---

## 3. Key Concepts and Patterns

### Truncating Plies

To slice off moves after a given board position, position a cursor at the desired terminal move and invoke `scid_game_cursor_truncate`:

```c
scid_game_cursor* cursor = NULL;
scid_game_cursor_create(game, &cursor);

/* Navigate to ply 2 */
scid_game_cursor* next_cursor = NULL;
scid_game_cursor_next(cursor, &next_cursor);
take_cursor(&cursor, &next_cursor);

/* Truncate everything after this move */
scid_game_cursor_truncate(cursor);
```

### Merging Independent Games

The `scid_game_merge_moves` function combines variation trees, grafting alternative lines of play as variations if the mainline diverges:

```c
scid_game* target_game = NULL;
scid_game* source_game = NULL;

/* Merge source_game into target_game at target_cursor position */
scid_game_merge_moves(target_game, target_cursor, source_game, source_cursor);
```
