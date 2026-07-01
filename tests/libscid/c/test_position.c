#include "test_libscid.h"

#include "scid/scid.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

void
test_position(
    void)
{
    const char* start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    const char* custom_fen = "8/K7/8/8/7k/8/8/8 w - - 45 25";
    const char* check_fen = "4k3/8/8/8/8/8/4q3/4K3 w - - 0 1";
    const char* mate_fen = "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3";
    scid_position* position = NULL;
    scid_position* next_position = NULL;
    scid_colour side_to_move = SCID_WHITE;
    char fen[128];
    int truth = 0;
    scid_piece piece = SCID_PIECE_NONE;
    unsigned number = 0;
    size_t fen_size = 0;

    assert(test_position_create_standard(&position) == SCID_OK);
    assert(position != NULL);
    assert(scid_position_to_fen(position, fen, sizeof(fen), &fen_size) == SCID_OK);
    assert(strcmp(fen, start_fen) == 0);
    assert(fen_size == strlen(start_fen));

    assert(scid_position_is_start(position, &truth) == SCID_OK);
    assert(truth == 1);

    assert(scid_position_is_check(position, &truth) == SCID_OK);
    assert(truth == 0);

    assert(scid_position_is_checkmate(position, &truth) == SCID_OK);
    assert(truth == 0);

    assert(scid_position_is_legal(position, &truth) == SCID_OK);
    assert(truth == 1);

    assert(scid_position_side_to_move_get(position, &side_to_move) == SCID_OK);
    assert(side_to_move == SCID_WHITE);

    assert(scid_position_fullmove_number_get(position, &number) == SCID_OK);
    assert(number == 1);

    assert(scid_position_halfmove_clock_get(position, &number) == SCID_OK);
    assert(number == 0);

    assert(scid_position_piece_at_get(position, 4, &piece) == SCID_OK);
    assert(piece == SCID_PIECE_WHITE_KING);

    assert(scid_position_piece_at_get(position, 56, &piece) == SCID_OK);
    assert(piece == SCID_PIECE_BLACK_ROOK);

    assert(scid_position_piece_at_get(position, 28, &piece) == SCID_OK);
    assert(piece == SCID_PIECE_NONE);

    scid_position_free(position);

    position = NULL;
    assert(test_position_create_standard(&position) == SCID_OK);
    assert(scid_position_create_with_uci(position, "e2e4", &next_position) == SCID_OK);
    assert(scid_position_to_fen(position, fen, sizeof(fen), &fen_size) == SCID_OK);
    assert(strcmp(fen, start_fen) == 0);
    scid_position_free(position);
    position = next_position;
    next_position = NULL;
    assert(scid_position_to_fen(position, fen, sizeof(fen), &fen_size) == SCID_OK);
    assert(strcmp(fen, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1") == 0);
    assert(scid_position_create_with_san(position, "c5", &next_position) == SCID_OK);
    assert(scid_position_to_fen(position, fen, sizeof(fen), &fen_size) == SCID_OK);
    assert(strcmp(fen, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1") == 0);
    scid_position_free(position);
    position = next_position;
    next_position = NULL;
    assert(scid_position_to_fen(position, fen, sizeof(fen), &fen_size) == SCID_OK);
    assert(strcmp(fen, "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2") == 0);
    assert(scid_position_apply_uci(position, "g1f3") == SCID_OK);
    assert(scid_position_apply_uci(position, "not-a-move") == SCID_ERROR_INVALID_MOVE);
    assert(scid_position_apply_san(position, "not-a-move") == SCID_ERROR_INVALID_MOVE);
    assert(scid_position_create_with_uci(NULL, "e2e4", &next_position) == SCID_ERROR_BAD_ARG);
    assert(scid_position_create_with_uci(position, NULL, &next_position) == SCID_ERROR_BAD_ARG);
    assert(scid_position_create_with_uci(position, "not-a-move", &next_position) == SCID_ERROR_INVALID_MOVE);
    assert(next_position == NULL);
    assert(scid_position_create_with_uci(position, "g1f3", NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_position_create_with_san(NULL, "e4", &next_position) == SCID_ERROR_BAD_ARG);
    assert(scid_position_create_with_san(position, NULL, &next_position) == SCID_ERROR_BAD_ARG);
    assert(scid_position_create_with_san(position, "not-a-move", &next_position) == SCID_ERROR_INVALID_MOVE);
    assert(next_position == NULL);
    assert(scid_position_create_with_san(position, "Nf3", NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_position_apply_uci(NULL, "e2e4") == SCID_ERROR_BAD_ARG);
    assert(scid_position_apply_uci(position, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_position_apply_san(NULL, "e4") == SCID_ERROR_BAD_ARG);
    assert(scid_position_apply_san(position, NULL) == SCID_ERROR_BAD_ARG);
    scid_position_free(position);

    position = NULL;
    assert(scid_position_create_from_fen(custom_fen, &position) == SCID_OK);
    assert(position != NULL);
    assert(scid_position_to_fen(position, fen, sizeof(fen), &fen_size) == SCID_OK);
    assert(strcmp(fen, custom_fen) == 0);
    assert(fen_size == strlen(custom_fen));

    assert(scid_position_is_start(position, &truth) == SCID_OK);
    assert(truth == 0);

    assert(scid_position_side_to_move_get(position, &side_to_move) == SCID_OK);
    assert(side_to_move == SCID_WHITE);

    assert(scid_position_fullmove_number_get(position, &number) == SCID_OK);
    assert(number == 25);

    assert(scid_position_halfmove_clock_get(position, &number) == SCID_OK);
    assert(number == 45);

    assert(scid_position_to_fen(position, NULL, 0, &fen_size) == SCID_ERROR_BUFFER_FULL);
    assert(fen_size == strlen(custom_fen));

    assert(
        scid_position_to_fen(position, fen, strlen(custom_fen), &fen_size) ==
        SCID_ERROR_BUFFER_FULL);
    assert(fen_size == strlen(custom_fen));

    assert(scid_position_to_fen(position, fen, sizeof(fen), NULL) == SCID_ERROR_BAD_ARG);
    scid_position_free(position);

    position = NULL;
    assert(scid_position_create_from_fen(check_fen, &position) == SCID_OK);
    assert(scid_position_is_check(position, &truth) == SCID_OK);
    assert(truth == 1);
    assert(scid_position_is_checkmate(position, &truth) == SCID_OK);
    assert(truth == 0);
    scid_position_free(position);

    position = NULL;
    assert(scid_position_create_from_fen(mate_fen, &position) == SCID_OK);
    assert(scid_position_side_to_move_get(position, &side_to_move) == SCID_OK);
    assert(side_to_move == SCID_WHITE);
    assert(scid_position_is_check(position, &truth) == SCID_OK);
    assert(truth == 1);
    assert(scid_position_is_checkmate(position, &truth) == SCID_OK);
    assert(truth == 1);
    scid_position_free(position);

    position = (scid_position*)1;
    assert(
        scid_position_create_from_fen("rnb1k2/Q1p5p/p7/4p3/4q3/8/PPP2R1P/2K5 b", &position) ==
        SCID_ERROR_INVALID_FEN);
    assert(position == NULL);

    assert(test_position_create_standard(NULL) == SCID_ERROR_BAD_ARG);
    assert(test_position_create_empty(NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_position_create_from_fen(NULL, &position) == SCID_ERROR_BAD_ARG);
    assert(scid_position_create_from_fen(custom_fen, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_position_to_fen(NULL, fen, sizeof(fen), &fen_size) == SCID_ERROR_BAD_ARG);
    assert(scid_position_is_start(NULL, &truth) == SCID_ERROR_BAD_ARG);
    assert(scid_position_is_start(position, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_position_is_check(NULL, &truth) == SCID_ERROR_BAD_ARG);
    assert(scid_position_is_check(position, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_position_is_checkmate(NULL, &truth) == SCID_ERROR_BAD_ARG);
    assert(scid_position_is_checkmate(position, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_position_is_legal(NULL, &truth) == SCID_ERROR_BAD_ARG);
    assert(scid_position_is_legal(position, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_position_side_to_move_get(NULL, &side_to_move) == SCID_ERROR_BAD_ARG);
    assert(scid_position_side_to_move_get(position, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_position_fullmove_number_get(NULL, &number) == SCID_ERROR_BAD_ARG);
    assert(scid_position_fullmove_number_get(position, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_position_halfmove_clock_get(NULL, &number) == SCID_ERROR_BAD_ARG);
    assert(scid_position_halfmove_clock_get(position, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_position_piece_at_get(NULL, 4, &piece) == SCID_ERROR_BAD_ARG);
    assert(scid_position_piece_at_get(position, 4, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_position_piece_at_get(position, 64, &piece) == SCID_ERROR_BAD_ARG);

    scid_position_free(next_position);
    scid_position_free(NULL);
}
