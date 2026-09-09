# How to Navigate Variation Trees with Cursors

This guide demonstrates how to traverse branching game trees using `scid_game_cursor` handles in the libscid C ABI, stepping forward and backward through plies, entering sub-variations, and inspecting moves, comments, and NAGs at each position.

---

## 1. Overview of Symbols

- `scid_game_cursor_create`: Spawns a cursor at the root position of a game.
- `scid_game_cursor_next`: Steps forward along the active variation, producing a new cursor pointing to the next move.
- `scid_game_cursor_previous`: Steps backward toward the root position.
- `scid_game_cursor_variation_count_get`: Returns the number of sub-variations diverging from the current position.
- `scid_game_cursor_variation_enter`: Enters a specified sub-variation at a given 0-based index.
- `scid_game_cursor_variation_leave`: Ascends out of the current sub-variation back to the parent variation.
- `scid_game_cursor_next_move_san_get`: Inspects the Standard Algebraic Notation (SAN) string of the upcoming move without stepping forward.
- `scid_game_cursor_next_movespec_get`: Retrieves the low-level `scid_movespec` descriptor of the upcoming move.

---

## 2. Complete Recipe

```c
--8<-- "examples/c/040-navigate-pgn/main.c"
```

---

## 3. Key Concepts and Patterns

### Cursor Immutability and Variation Traversal

Navigation cursors in libscid never mutate internal state in place. Every navigational call (`scid_game_cursor_next`, `scid_game_cursor_variation_enter`, etc.) accepts an output pointer `scid_game_cursor** out_cursor` to return a newly allocated cursor:

```c
scid_game_cursor* cursor = NULL;
scid_game_cursor_create(game, &cursor);

/* Inspect variations available at current position */
size_t variation_count = 0;
scid_game_cursor_variation_count_get(cursor, &variation_count);

if (variation_count > 0)
{
    scid_game_cursor* var_cursor = NULL;
    /* Enter variation 0 */
    scid_game_cursor_variation_enter(cursor, 0, &var_cursor);

    /* ... navigate within variation ... */

    scid_game_cursor_free(var_cursor);
}

scid_game_cursor_free(cursor);
```
