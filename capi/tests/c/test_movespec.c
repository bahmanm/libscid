#include "test_libscid.h"

#include "scid/scid.h"

#include <stddef.h>
#include <string.h>

void
test_movespec(void)
{
    scid_movespec  move = {0, 0, 0, 0};
    scid_position* position = NULL;
    char           text[8];
    size_t         text_size = 0;

    TEST_ASSERT(scid_movespec_create(12, 28, SCID_PIECE_NONE, 0, &move) == SCID_OK);
    TEST_ASSERT(move.from == 12);
    TEST_ASSERT(move.to == 28);
    TEST_ASSERT(move.promotion == SCID_PIECE_NONE);
    TEST_ASSERT(move.is_castling == 0);

    TEST_ASSERT(scid_movespec_create(4, 6, SCID_PIECE_NONE, 7, &move) == SCID_OK);
    TEST_ASSERT(move.from == 4);
    TEST_ASSERT(move.to == 6);
    TEST_ASSERT(move.promotion == SCID_PIECE_NONE);
    TEST_ASSERT(move.is_castling == 1);

    TEST_ASSERT(scid_movespec_create(12, 28, SCID_PIECE_NONE, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_movespec_create_from_uci("e2e4", &move) == SCID_OK);
    TEST_ASSERT(move.from == 12);
    TEST_ASSERT(move.to == 28);
    TEST_ASSERT(move.promotion == SCID_PIECE_NONE);
    TEST_ASSERT(move.is_castling == 0);

    TEST_ASSERT(scid_movespec_create_from_uci("a7a8q", &move) == SCID_OK);
    TEST_ASSERT(move.from == 48);
    TEST_ASSERT(move.to == 56);
    TEST_ASSERT(move.promotion == SCID_PIECE_QUEEN);
    TEST_ASSERT(move.is_castling == 0);

    TEST_ASSERT(scid_movespec_create_from_uci("h2h1N", &move) == SCID_OK);
    TEST_ASSERT(move.from == 15);
    TEST_ASSERT(move.to == 7);
    TEST_ASSERT(move.promotion == SCID_PIECE_KNIGHT);
    TEST_ASSERT(move.is_castling == 0);

    TEST_ASSERT(scid_movespec_create_from_uci("0000", &move) == SCID_OK);
    TEST_ASSERT(move.from == 0);
    TEST_ASSERT(move.to == 0);
    TEST_ASSERT(move.promotion == SCID_PIECE_NONE);
    TEST_ASSERT(move.is_castling == 0);

    TEST_ASSERT(scid_movespec_create_from_uci(NULL, &move) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_create_from_uci("e2e4", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_create_from_uci("e2e", &move) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_create_from_uci("e2e4qq", &move) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_create_from_uci("e9e4", &move) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_create_from_uci("e2e4k", &move) == SCID_ERROR_BAD_ARG);

    move.from = 12;
    move.to = 28;
    move.promotion = SCID_PIECE_NONE;
    move.is_castling = 0;
    TEST_ASSERT(scid_movespec_to_uci(move, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "e2e4") == 0);
    TEST_ASSERT(text_size == 4);

    move.from = 48;
    move.to = 56;
    move.promotion = SCID_PIECE_QUEEN;
    move.is_castling = 0;
    TEST_ASSERT(scid_movespec_to_uci(move, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "a7a8q") == 0);
    TEST_ASSERT(text_size == 5);

    move.from = 0;
    move.to = 0;
    move.promotion = SCID_PIECE_NONE;
    move.is_castling = 0;
    TEST_ASSERT(scid_movespec_to_uci(move, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "0000") == 0);
    TEST_ASSERT(text_size == 4);

    move.from = 12;
    move.to = 28;
    move.promotion = SCID_PIECE_NONE;
    move.is_castling = 0;
    TEST_ASSERT(scid_movespec_to_uci(move, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == 4);

    TEST_ASSERT(scid_movespec_to_uci(move, text, 4, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == 4);

    TEST_ASSERT(scid_movespec_to_uci(move, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);

    move.from = 64;
    move.to = 28;
    move.promotion = SCID_PIECE_NONE;
    move.is_castling = 0;
    TEST_ASSERT(scid_movespec_to_uci(move, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);

    move.from = 12;
    move.to = 28;
    move.promotion = SCID_PIECE_KING;
    move.is_castling = 0;
    TEST_ASSERT(scid_movespec_to_uci(move, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(position != NULL);

    TEST_ASSERT(scid_movespec_create_from_san(position, "e4", &move) == SCID_OK);
    TEST_ASSERT(move.from == 12);
    TEST_ASSERT(move.to == 28);
    TEST_ASSERT(move.promotion == SCID_PIECE_NONE);
    TEST_ASSERT(move.is_castling == 0);

    TEST_ASSERT(scid_movespec_to_san(position, move, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "e4") == 0);
    TEST_ASSERT(text_size == 2);

    TEST_ASSERT(
        scid_movespec_to_san(position, move, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == 2);

    TEST_ASSERT(
        scid_movespec_to_san(position, move, text, 2, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == 2);

    TEST_ASSERT(
        scid_movespec_create_from_san(position, "not-a-move", &move) == SCID_ERROR_INVALID_MOVE);
    TEST_ASSERT(scid_movespec_create_from_san(NULL, "e4", &move) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_create_from_san(position, NULL, &move) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_movespec_create_from_san(position, "e4", NULL) == SCID_ERROR_BAD_ARG);

    move.from = 64;
    move.to = 28;
    move.promotion = SCID_PIECE_NONE;
    move.is_castling = 0;
    TEST_ASSERT(
        scid_movespec_to_san(position, move, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);

    move.from = 12;
    move.to = 28;
    move.promotion = SCID_PIECE_NONE;
    move.is_castling = 0;
    TEST_ASSERT(
        scid_movespec_to_san(NULL, move, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_movespec_to_san(position, move, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(position);
}
