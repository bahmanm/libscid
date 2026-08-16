#ifndef SCID_TEST_LIBSCID_H
#define SCID_TEST_LIBSCID_H

#include "scid/scid.h"

#include <assert.h>

static const char* TEST_STANDARD_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

static const char* TEST_SCRATCH_FEN = "8/K7/8/8/7k/8/8/8 w - - 0 1";

static scid_error
test_position_create_standard(scid_position** out_position)
{
    return scid_position_create_from_fen(TEST_STANDARD_FEN, out_position);
}


static scid_error
test_position_create_empty(scid_position** out_position)
{
    return scid_position_create_from_fen(TEST_SCRATCH_FEN, out_position);
}


static scid_error
test_game_create_blank(scid_game** out_game)
{
    scid_position* position = 0;
    scid_error     error = test_position_create_standard(&position);
    if (error != SCID_OK)
    {
        return error;
    }

    error = scid_game_create_blank(position, out_game);
    scid_position_free(position);
    return error;
}


static scid_error
test_game_create(
    const char* pgn,
    size_t      pgn_size,
    scid_game** out_game,
    char*       out_diagnostic,
    size_t      out_diagnostic_capacity,
    size_t*     out_diagnostic_size)
{
    scid_position* position = 0;
    scid_error     error = test_position_create_standard(&position);
    if (error != SCID_OK)
    {
        return error;
    }

    error = scid_game_create(
        position, pgn, pgn_size, out_game, out_diagnostic, out_diagnostic_capacity,
        out_diagnostic_size);
    scid_position_free(position);
    return error;
}


static void
test_cursor_take(
    scid_game_cursor** cursor,
    scid_game_cursor*  next_cursor)
{
    assert(cursor != 0);
    assert(next_cursor != 0);
    scid_game_cursor_free(*cursor);
    *cursor = next_cursor;
}


void
test_cursor(void);
void
test_cursor_mutation(void);
void
test_database(void);
void
test_database_filters(void);
void
test_database_search(void);
void
test_eco(void);
void
test_game(void);
void
test_movespec(void);
void
test_nag(void);
void
test_piece(void);
void
test_pgn_contract(void);
void
test_position(void);
void
test_square(void);

#endif
