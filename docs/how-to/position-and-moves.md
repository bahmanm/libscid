# How to Apply Moves and Enumerate Legal Moves

This guide shows how to interact with chess board positions directly using the libscid C ABI: generating legal moves, converting moves between UCI and SAN representations, applying moves to update board states, and querying check and checkmate conditions.

---

## 1. Overview of Symbols

- `scid_position_create_from_fen`: Initialises a board position from a FEN string.
- `scid_position_legal_moves`: Populates a caller-allocated array of `scid_movespec` with all legal moves available in the position.
- `SCID_MAX_LEGAL_MOVES`: Compile-time constant specifying the maximum number of legal moves possible in any chess position (256).
- `scid_movespec_to_san`: Formats a low-level move descriptor into Standard Algebraic Notation (SAN) based on the board context.
- `scid_movespec_to_uci`: Formats a move into Universal Chess Interface (UCI) notation (e.g. `e2e4`, `e7e8q`).
- `scid_position_apply_san`: Applies a SAN move directly to the position, mutating its state.
- `scid_position_is_check`: Tests whether the side to move is currently in check.
- `scid_position_is_checkmate`: Tests whether the current position is checkmate.
- `scid_position_to_fen`: Formats the current position back into a FEN string.

---

## 2. Complete Recipe

```c
--8<-- "examples/c/050-position-and-moves/main.c"
```

---

## 3. Key Concepts and Patterns

### Enumerating Legal Moves

Legal move generation uses caller-allocated arrays with `SCID_MAX_LEGAL_MOVES` capacity:

```c
scid_movespec moves[SCID_MAX_LEGAL_MOVES];
size_t move_count = 0;

if (scid_position_legal_moves(position, moves, SCID_MAX_LEGAL_MOVES, &move_count) == SCID_OK)
{
    for (size_t i = 0; i < move_count; ++i)
    {
        char san[32];
        char uci[8];
        size_t san_size = 0;
        size_t uci_size = 0;

        scid_movespec_to_san(position, moves[i], san, sizeof(san), &san_size);
        scid_movespec_to_uci(moves[i], uci, sizeof(uci), &uci_size);

        printf("Move %zu: %.*s (UCI: %.*s)\n", i, (int)san_size, san, (int)uci_size, uci);
    }
}
```

### Mutating Board State and Detecting Check

```c
scid_position_apply_san(position, "Qxf7#");

int is_check = 0;
int is_checkmate = 0;
scid_position_is_check(position, &is_check);
scid_position_is_checkmate(position, &is_checkmate);
```
