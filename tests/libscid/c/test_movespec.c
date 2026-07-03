#include "test_libscid.h"

#include "scid/scid.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

void
test_movespec(
    void)
{
    scid_movespec move = {0, 0, 0, 0};
    scid_position* position = NULL;
    char text[8];
    size_t text_size = 0;

    assert(scid_movespec_create(12, 28, SCID_PIECE_NONE, 0, &move) == SCID_OK);
    assert(move.from == 12);
    assert(move.to == 28);
    assert(move.promotion == SCID_PIECE_NONE);
    assert(move.is_castling == 0);

    assert(scid_movespec_create(4, 6, SCID_PIECE_NONE, 7, &move) == SCID_OK);
    assert(move.from == 4);
    assert(move.to == 6);
    assert(move.promotion == SCID_PIECE_NONE);
    assert(move.is_castling == 1);

    assert(scid_movespec_create(12, 28, SCID_PIECE_NONE, 0, NULL) == SCID_ERROR_BAD_ARG);

    assert(scid_movespec_create_from_uci("e2e4", &move) == SCID_OK);
    assert(move.from == 12);
    assert(move.to == 28);
    assert(move.promotion == SCID_PIECE_NONE);
    assert(move.is_castling == 0);

    assert(scid_movespec_create_from_uci("a7a8q", &move) == SCID_OK);
    assert(move.from == 48);
    assert(move.to == 56);
    assert(move.promotion == SCID_PIECE_QUEEN);
    assert(move.is_castling == 0);

    assert(scid_movespec_create_from_uci("h2h1N", &move) == SCID_OK);
    assert(move.from == 15);
    assert(move.to == 7);
    assert(move.promotion == SCID_PIECE_KNIGHT);
    assert(move.is_castling == 0);

    assert(scid_movespec_create_from_uci("0000", &move) == SCID_OK);
    assert(move.from == 0);
    assert(move.to == 0);
    assert(move.promotion == SCID_PIECE_NONE);
    assert(move.is_castling == 0);

    assert(scid_movespec_create_from_uci(NULL, &move) == SCID_ERROR_BAD_ARG);
    assert(scid_movespec_create_from_uci("e2e4", NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_movespec_create_from_uci("e2e", &move) == SCID_ERROR_BAD_ARG);
    assert(scid_movespec_create_from_uci("e2e4qq", &move) == SCID_ERROR_BAD_ARG);
    assert(scid_movespec_create_from_uci("e9e4", &move) == SCID_ERROR_BAD_ARG);
    assert(scid_movespec_create_from_uci("e2e4k", &move) == SCID_ERROR_BAD_ARG);

    move.from = 12;
    move.to = 28;
    move.promotion = SCID_PIECE_NONE;
    move.is_castling = 0;
    assert(scid_movespec_to_uci(move, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "e2e4") == 0);
    assert(text_size == 4);

    move.from = 48;
    move.to = 56;
    move.promotion = SCID_PIECE_QUEEN;
    move.is_castling = 0;
    assert(scid_movespec_to_uci(move, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "a7a8q") == 0);
    assert(text_size == 5);

    move.from = 0;
    move.to = 0;
    move.promotion = SCID_PIECE_NONE;
    move.is_castling = 0;
    assert(scid_movespec_to_uci(move, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "0000") == 0);
    assert(text_size == 4);

    move.from = 12;
    move.to = 28;
    move.promotion = SCID_PIECE_NONE;
    move.is_castling = 0;
    assert(scid_movespec_to_uci(move, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    assert(text_size == 4);

    assert(scid_movespec_to_uci(move, text, 4, &text_size) == SCID_ERROR_BUFFER_FULL);
    assert(text_size == 4);

    assert(scid_movespec_to_uci(move, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);

    move.from = 64;
    move.to = 28;
    move.promotion = SCID_PIECE_NONE;
    move.is_castling = 0;
    assert(scid_movespec_to_uci(move, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);

    move.from = 12;
    move.to = 28;
    move.promotion = SCID_PIECE_KING;
    move.is_castling = 0;
    assert(scid_movespec_to_uci(move, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);

    assert(test_position_create_standard(&position) == SCID_OK);
    assert(position != NULL);

    assert(scid_movespec_create_from_san(position, "e4", &move) == SCID_OK);
    assert(move.from == 12);
    assert(move.to == 28);
    assert(move.promotion == SCID_PIECE_NONE);
    assert(move.is_castling == 0);

    assert(scid_movespec_to_san(position, move, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "e4") == 0);
    assert(text_size == 2);

    assert(scid_movespec_to_san(position, move, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    assert(text_size == 2);

    assert(scid_movespec_to_san(position, move, text, 2, &text_size) == SCID_ERROR_BUFFER_FULL);
    assert(text_size == 2);

    assert(scid_movespec_create_from_san(position, "not-a-move", &move) == SCID_ERROR_INVALID_MOVE);
    assert(scid_movespec_create_from_san(NULL, "e4", &move) == SCID_ERROR_BAD_ARG);
    assert(scid_movespec_create_from_san(position, NULL, &move) == SCID_ERROR_BAD_ARG);
    assert(scid_movespec_create_from_san(position, "e4", NULL) == SCID_ERROR_BAD_ARG);

    move.from = 64;
    move.to = 28;
    move.promotion = SCID_PIECE_NONE;
    move.is_castling = 0;
    assert(
        scid_movespec_to_san(position, move, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);

    move.from = 12;
    move.to = 28;
    move.promotion = SCID_PIECE_NONE;
    move.is_castling = 0;
    assert(scid_movespec_to_san(NULL, move, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    assert(scid_movespec_to_san(position, move, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(position);
}
