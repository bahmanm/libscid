# How to Create Games from Custom FEN Positions

This guide shows how to initialise a chess game starting from an arbitrary Forsyth-Edwards Notation (FEN) position, such as endgame studies or tactical puzzle setups, ensuring that the PGN output automatically includes the appropriate `SetUp` and `FEN` header tags.

---

## 1. Overview of Symbols

- `scid_position_create_from_fen`: Parses a FEN string and constructs an initial board position.
- `scid_game_create_blank`: Initialises a game whose root state matches the provided board position rather than the standard starting setup.
- `scid_game_tag_get`: Queries header tags, including automatically generated `FEN` and `SetUp` tags.
- `scid_game_to_pgn`: Formats and exports the nonstandard game to valid PGN text.

---

## 2. Complete Recipe

```c
--8<-- "examples/c/030-nonstandard-start/main.c"
```

---

## 3. Key Concepts and Patterns

### Handling Nonstandard Start Positions

When a game is created from a non-standard board state via `scid_game_create_blank(position, &game)`, libscid automatically sets the `SetUp` tag to `"1"` and populates the `FEN` tag with the exact board descriptor:

```c
const char* fen = "8/K7/8/8/7k/8/8/8 w - - 45 25";
scid_position* position = NULL;
scid_game* game = NULL;

scid_position_create_from_fen(fen, &position);
scid_game_create_blank(position, &game);
```

When exported to PGN, the resulting text contains:

```
[Event "Study position"]
[Site "?"]
[Date "????.??.??"]
[Round "?"]
[White "?"]
[Black "?"]
[Result "*"]
[SetUp "1"]
[FEN "8/K7/8/8/7k/8/8/8 w - - 45 25"]
```
