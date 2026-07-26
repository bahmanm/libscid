# API Surface {#api_surface}

The installed API surface is the C ABI declared by `scid/scid.h`.

libscid uses opaque handles for owned objects:

- `scid_position`
- `scid_game`
- `scid_game_pgn_options`
- `scid_game_cursor`
- `scid_eco_book`
- `scid_database`

Functions return `scid_error`; `SCID_OK` means success. Strings are passed as
UTF-8 `const char*` inputs or caller-owned output buffers with explicit
capacity and size parameters.

Objects created by libscid must be released with the matching `*_free`
function. The C++ implementation behind the ABI is not installed as a consumer
API.

## ABI Conventions

The C ABI keeps ownership and mutability explicit:

- Functions returning objects use an `out_...` parameter, such as
  `scid_game_create(..., scid_game** out_game, ...)`.
- `const` pointer arguments are read-only for that call.
- Caller-owned text buffers are always paired with a capacity and an output
  size. If the buffer is too small, the function returns
  `SCID_ERROR_BUFFER_FULL` and reports the required size.
- `*_free(NULL)` is accepted for owned opaque objects.

Positions have both mutable and immutable styles. `scid_position_apply_san()`
and `scid_position_apply_uci()` mutate a position in place;
`scid_position_create_with_san()` and `scid_position_create_with_uci()` create a
new position from an existing position and one legal move. Legal moves are
enumerated with `scid_position_legal_moves()`, which writes complete
`scid_movespec` values into a caller-owned array. Callers can allocate
`SCID_MAX_LEGAL_MOVES` entries for the full legal-move upper bound. If the array
is too small, the function returns `SCID_ERROR_BUFFER_FULL` and writes zero
moves.

Game cursors are value objects bound to a particular game. Navigation functions
do not mutate the input cursor; they construct a new cursor in an `out_...`
parameter. The game is still the mutable aggregate: functions such as
`scid_game_cursor_comment_set()`, `scid_game_cursor_move_add()` and
`scid_game_merge_moves()` edit the game at the location identified by a cursor.

## PGN And Games

Use a position to choose the game's starting board:

- `scid_game_create_blank(position, out_game)` creates a game with tags and a
  start position, but no moves.
- `scid_game_create(position, pgn, pgn_size, out_game, diagnostic, ...)`
  creates a game by parsing PGN from that start position.

Use `scid_game_cursor_create(game, out_cursor)` to point at the beginning of a
game. From there, navigation is cursor-to-cursor:

- `scid_game_cursor_next()` and `scid_game_cursor_previous()` move one slot in
  the current line.
- `scid_game_cursor_to_start()`, `scid_game_cursor_to_end()` and
  `scid_game_cursor_to_ply()` construct cursors at common positions.
- `scid_game_cursor_variation_enter()` and
  `scid_game_cursor_variation_exit()` move into and out of variations.

To add or change movetext, pass both the game and the cursor:

- `scid_game_cursor_move_add()` inserts one legal move at the cursor.
- `scid_game_cursor_variation_add()` creates a new variation at the cursor.
- `scid_game_cursor_comment_set()`, `scid_game_cursor_nag_add()`,
  `scid_game_cursor_nag_remove()` and `scid_game_cursor_nag_clear()` edit move
  annotations.
- `scid_game_cursor_variation_promote_to_first()`,
  `scid_game_cursor_variation_promote_to_mainline()`,
  `scid_game_cursor_variation_delete()`, `scid_game_cursor_truncate()` and
  `scid_game_cursor_truncate_before_cursor()` restructure existing movetext.

To apply more than one move at a cursor, create a source game and call
`scid_game_merge_moves()`:

- `SCID_GAME_MERGE_MOVES_APPEND` appends the source moves at an empty target
  location.
- `SCID_GAME_MERGE_MOVES_INSERT_VARIATION` inserts the source moves as a
  variation attached at the target location.
- `SCID_GAME_MERGE_MOVES_REPLACE` deletes the target continuation and replaces
  it with the source moves.

Export uses `scid_game_to_pgn(game, options, out_text, capacity, out_size)`.
Pass `NULL` for `options` to use the default complete PGN export. For a custom
export policy, create `scid_game_pgn_options`, set the desired flags, pass it to
`scid_game_to_pgn()`, and free it afterwards. The current options cover
symbolic NAGs, supplemental tags, comments, variations and line width.

## Database PGN

Database functions are higher-level conveniences over the same game model.
`scid_database_import_pgn()` parses one or more PGN games directly into a
database. `scid_database_game_export_pgn()` exports a stored game with the
default PGN export policy. If a caller needs custom export options, load the
game with `scid_database_game_get()` and then call `scid_game_to_pgn()` with a
`scid_game_pgn_options` object.
