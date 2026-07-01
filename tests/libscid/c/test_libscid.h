#ifndef SCID_TEST_LIBSCID_H
#define SCID_TEST_LIBSCID_H

#include "scid/scid.h"

#include <assert.h>

static void
test_cursor_take(
    scid_game_cursor** cursor,
    scid_game_cursor* next_cursor)
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
