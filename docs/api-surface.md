# API Surface {#api_surface}

The installed API surface is the C ABI declared by `scid/scid.h`.

libscid uses opaque handles for owned objects:

- `scid_position`: chess board state and legal move handling.
- `scid_game`: game model containing header tags, start position and movetext.
- `scid_game_pgn_options`: formatting options for PGN export.
- `scid_game_cursor`: immutable navigation pointer into a game's movetext tree.
- `scid_eco_book`: loaded ECO opening classification database.
- `scid_database`: in-memory, SCID5 or read-only PGN database.
- `scid_search_header_criteria`: header search criteria builder and filters.
- `scid_search_board_criteria`: board and pawn structure search criteria builder.

Functions return `scid_error`; `SCID_OK` (0) indicates success. Text inputs are UTF-8 `const char*`. Text and array output parameters use caller-owned buffers with explicit capacity and output size parameters.

Objects created by libscid must be released with the matching `*_free()` call. Passing `NULL` to any `*_free()` function is a valid no-op.

---

## ABI Conventions

The C ABI maintains explicit ownership and mutability contracts:

- Factory Functions: Functions returning owned handles accept an `out_...` pointer argument, e.g., `scid_game_create(..., scid_game** out_game, ...)`.
- Read-Only Parameters: Arguments qualified with `const` are read-only for the duration of the call.
- Caller-Owned Buffers: Text outputs require a target buffer pointer, a capacity (`size_t`), and an output size pointer (`size_t*`). If capacity is insufficient, functions return `SCID_ERROR_BUFFER_FULL` and report the required byte count in `out_..._size`.
- Error and Warning Codes: `scid_error` enumerates standard failure modes (`SCID_ERROR_BAD_ARG`, `SCID_ERROR_INVALID_FEN`, `SCID_ERROR_INVALID_MOVE`, `SCID_ERROR_FILE_OPEN`, `SCID_ERROR_FILE_READ_ONLY`, `SCID_ERROR_CORRUPT`) as well as recoverable warnings (`SCID_WARNING_NAME_DATA_LOSS`).
- Status Classification: `scid_is_warning(status)` and `scid_is_error(status)` classify status codes without preprocessor macros.

---

## Primitives and Value Types

Basic chess types are value types or primitive integer aliases:

- `scid_colour`: `SCID_WHITE` (0) or `SCID_BLACK` (1).
- `scid_square`: 0-indexed square offset (`0` for `a1` to `63` for `h8`). Helpers: `scid_square_from_string()` and `scid_square_to_string()`.
- `scid_piece`: piece types (`SCID_PIECE_NONE`, `SCID_PIECE_KING`..`PAWN`) and color-qualified pieces (`SCID_PIECE_WHITE_KING`, `SCID_PIECE_BLACK_KING`, etc.). Helper: `scid_piece_type_from_string()`.
- `scid_nag`: Numeric Annotation Glyph byte value. Helpers: `scid_nag_create_from_string()` and `scid_nag_to_string()`.
- `scid_movespec`: value struct specifying move details:
  ```c
  typedef struct scid_movespec
  {
      scid_square from;
      scid_square to;
      scid_piece  promotion;
      int         is_castling;
  } scid_movespec;
  ```
  Helpers: `scid_movespec_create()`, `scid_movespec_create_from_san()`, `scid_movespec_create_from_uci()`, `scid_movespec_to_san()`, `scid_movespec_to_uci()`.

---

## Board Positions

`scid_position` represents a complete chess board state (piece placement, active side, castling rights, en passant target, halfmove clock, and fullmove number).

- Lifecycle:
  - `scid_position_create_from_fen(fen, out_position)` parses a FEN string.
  - `scid_position_free(position)` releases position memory.
- Immutable Derivation:
  - `scid_position_create_with_san(position, san, out_position)`
  - `scid_position_create_with_uci(position, uci, out_position)`
- In-Place Mutation:
  - `scid_position_apply_san(position, san)`
  - `scid_position_apply_uci(position, uci)`
- Legal Move Enumeration:
  - `scid_position_legal_moves(position, out_moves, capacity, out_size)` writes `scid_movespec` structs into a caller-allocated array. Callers can allocate `SCID_MAX_LEGAL_MOVES` (256) entries.
- State Queries:
  - `scid_position_to_fen()` formats the current state as FEN.
  - `scid_position_is_start()`, `scid_position_is_check()`, `scid_position_is_checkmate()`, `scid_position_is_legal()`.
  - `scid_position_side_to_move_get()`, `scid_position_fullmove_number_get()`, `scid_position_halfmove_clock_get()`, `scid_position_piece_at_get()`.

---

## Games and PGN

`scid_game` represents a chess game with header tags, start position, and variation tree.

- Creation & Export:
  - `scid_game_create_blank(position, out_game)` creates an empty game with a chosen start board.
  - `scid_game_create(position, pgn, pgn_size, out_game, out_diagnostic, ...)` parses a PGN string.
  - `scid_game_to_pgn(game, options, out_text, capacity, out_size)` exports PGN. Pass `NULL` for default options, or pass a `scid_game_pgn_options` handle configured via:
    - `scid_game_pgn_options_symbolic_nags_set()`
    - `scid_game_pgn_options_supplemental_tags_set()`
    - `scid_game_pgn_options_comments_set()`
    - `scid_game_pgn_options_variations_set()`
    - `scid_game_pgn_options_line_width_set()`
- Tag & Metadata Inspection:
  - `scid_game_tag_get()`, `scid_game_tag_set()`, `scid_game_tag_remove()`, `scid_game_tag_count_get()`, `scid_game_tag_at_get()`.
  - `scid_game_start_position_get()`, `scid_game_final_position_get()`, `scid_game_mainline_halfmove_count_get()`, `scid_game_initial_comment_get()`.

---

## Movetext Navigation and Editing

Navigation and editing use `scid_game_cursor`. Cursors are immutable location pointers bound to a game.

- Cursor Navigation:
  - `scid_game_cursor_create(game, out_cursor)` creates a cursor at the start of a game.
  - `scid_game_cursor_next()`, `scid_game_cursor_previous()` move step-by-step.
  - `scid_game_cursor_to_start()`, `scid_game_cursor_to_end()`, `scid_game_cursor_to_ply()` jump to locations.
  - `scid_game_cursor_variation_enter()`, `scid_game_cursor_variation_exit()` traverse variation branches.
  - `scid_game_cursor_position_get()` extracts the position at the cursor.
- Tree Mutation:
  - `scid_game_cursor_move_add(game, cursor, move, out_next_cursor)` appends a move.
  - `scid_game_cursor_variation_add(game, cursor, initial_comment, ...)` creates a variation branch.
  - `scid_game_cursor_comment_set()`, `scid_game_cursor_nag_add()`, `scid_game_cursor_nag_remove()`, `scid_game_cursor_nag_clear()`.
  - `scid_game_cursor_variation_promote_to_first()`, `scid_game_cursor_variation_promote_to_mainline()`, `scid_game_cursor_variation_delete()`, `scid_game_cursor_truncate()`.
- Game Move Merging:
  - `scid_game_merge_moves(target_game, target_cursor, source_game, mode, out_cursor)` merges moves from a source game. Modes:
    - `SCID_GAME_MERGE_MOVES_APPEND`
    - `SCID_GAME_MERGE_MOVES_INSERT_VARIATION`
    - `SCID_GAME_MERGE_MOVES_REPLACE`

---

## Database Subsystem

`scid_database` handles in-memory collections, native SCID5 persistent storage, and read-only PGN files.

- Database Lifecycle:
  - `scid_database_create_memory(name, out_db)`
  - `scid_database_create_scid5(path, out_db)`
  - `scid_database_open_scid5(path, progress_cb, user_data, cancel_fn, cancel_user_data, out_db)`
  - `scid_database_open_scid5_read_only(path, progress_cb, user_data, cancel_fn, cancel_user_data, out_db)`
  - `scid_database_open_pgn_read_only(path, progress_cb, user_data, cancel_fn, cancel_user_data, out_db)`
  - `scid_database_save()`, `scid_database_close()`, `scid_database_free()`.
- Database Status & Properties:
  - `scid_database_status_open_get()`, `scid_database_status_bad_name_count_get()`.
  - `scid_database_status_is_read_only()`, `scid_database_status_is_dirty()`.
  - `scid_database_is_open()`, `scid_database_read_only_get()`, `scid_database_filename_get()`, `scid_database_type_get()`.
- Game Management & Storage:
  - `scid_database_game_count_get()`
  - `scid_database_game_get(db, index, out_game, out_flags, ...)`
  - `scid_database_game_add()`, `scid_database_game_replace()`, `scid_database_game_delete()`, `scid_database_game_undelete()`.
  - `scid_database_import_pgn()`, `scid_database_game_export_pgn()`.
- Filters and Index Pagination:
  - Filters are referenced by `scid_filter_id`. Special constants: `SCID_FILTER_ALL_GAMES` (-1) and `SCID_FILTER_PRIMARY` (-2).
  - `scid_database_filter_create()`, `scid_database_filter_delete()`, `scid_database_filter_game_count_get()`.
  - `scid_database_filter_game_indices_get()`, `scid_database_filter_game_index_at_row_get()`, `scid_database_filter_game_row_for_index_get()`.
- Search Engines:
  - `scid_database_search_headers(db, src_filter, dst_filter, header_criteria, progress_cb, ...)` filters games by tag fields, Elo ranges, move counts, and flags using opaque `scid_search_header_criteria`.
  - `scid_database_search_position(db, src_filter, dst_filter, position, ...)` filters games matching an exact board position.
  - `scid_database_search_board(db, src_filter, dst_filter, board_criteria, ...)` filters games by exact position, pawn structure, or file configuration using opaque `scid_search_board_criteria`.
- Search Criteria Builders:
  - Header criteria lifecycle: `scid_search_header_criteria_create()`, `scid_search_header_criteria_free()`.
  - Header text filters: `*_player_set()`, `*_player_get()`, `*_white_set()`, `*_white_get()`, `*_black_set()`, `*_black_get()`, `*_event_set()`, `*_event_get()`, `*_site_set()`, `*_site_get()`, `*_site_country_set()`, `*_site_country_get()`, `*_round_set()`, `*_round_get()`, `*_result_set()`, `*_result_get()`.
  - Header ranges: `*_date_range_set()`, `*_date_range_get()`, `*_event_date_range_set()`, `*_event_date_range_get()`, `*_eco_range_set()`, `*_eco_range_get()`, `*_game_number_range_set()`, `*_game_number_range_get()`, `*_halfmove_count_range_set()`, `*_halfmove_count_range_get()`, `*_white_elo_range_set()`, `*_white_elo_range_get()`, `*_black_elo_range_set()`, `*_black_elo_range_get()`, `*_elo_difference_range_set()`, `*_elo_difference_range_get()`.
  - Header flags: `*_has_variations_set()`, `*_has_variations_get()`, `*_has_comments_set()`, `*_has_comments_get()`, `*_has_nags_set()`, `*_has_nags_get()`.
  - Board criteria lifecycle: `scid_search_board_criteria_create()`, `scid_search_board_criteria_free()`.
  - Board configuration: `*_position_set()`, `*_position_get()`, `*_match_set()`, `*_match_get()`, `*_include_variations_set()`, `*_include_variations_get()`, `*_include_flipped_set()`, `*_include_flipped_get()`.
- Database Metadata & Statistics:
  - `scid_database_metadata_get()`, `scid_database_metadata_set()`, `scid_database_metadata_count_get()`, `scid_database_metadata_at_get()`.
  - `scid_database_stats_date_range_get()`, `scid_database_stats_result_count_get()`.

---

## ECO Opening Classification

`scid_eco_book` classifies positions using standard ECO codes and opening names.

- Book Lifecycle:
  - `scid_eco_book_load(path, out_book)` loads an `.eco` book file.
  - `scid_eco_book_free(book)` releases book memory.
- Classification Queries:
  - `scid_eco_book_code_find(book, position, out_code)` looks up the `scid_eco_code` for a position.
  - `scid_eco_book_name_find(book, position, out_text, capacity, out_size)` looks up the full opening name string.
- Code Conversion & Formatting:
  - `scid_eco_code_from_string(text, out_code)`
  - `scid_eco_code_to_string(code, format, out_text, capacity, out_size)`. Formats: `SCID_ECO_FORMAT_BASIC` (e.g. `C84`) and `SCID_ECO_FORMAT_EXTENDED` (e.g. `C84h`).
