#include "test_libscid.h"

#include "scid/scid.h"

#include <stddef.h>
#include <string.h>

static int
test_moves_include(
    const scid_movespec* moves,
    size_t               move_count,
    scid_square          from,
    scid_square          to,
    scid_piece           promotion,
    int                  is_castling)
{
    size_t i = 0;
    for (i = 0; i < move_count; ++i)
    {
        if (moves[i].from == from && moves[i].to == to && moves[i].promotion == promotion &&
            moves[i].is_castling == is_castling)
        {
            return 1;
        }
    }
    return 0;
}

void
test_position(void)
{
    const char*    start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    const char*    custom_fen = "8/K7/8/8/7k/8/8/8 w - - 45 25";
    const char*    check_fen = "4k3/8/8/8/8/8/4q3/4K3 w - - 0 1";
    const char*    mate_fen = "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3";
    const char*    promotion_fen = "4k3/P7/8/8/8/8/8/4K3 w - - 0 1";
    scid_position* position = NULL;
    scid_position* next_position = NULL;
    scid_colour    side_to_move = SCID_WHITE;
    char           fen[128];
    int            truth = 0;
    scid_piece     piece = SCID_PIECE_NONE;
    unsigned       number = 0;
    size_t         fen_size = 0;
    scid_movespec  moves[SCID_MAX_LEGAL_MOVES];
    size_t         move_count = 999;

    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(position != NULL);
    TEST_ASSERT(scid_position_to_fen(position, fen, sizeof(fen), &fen_size) == SCID_OK);
    TEST_ASSERT(strcmp(fen, start_fen) == 0);
    TEST_ASSERT(fen_size == strlen(start_fen));

    TEST_ASSERT(scid_position_is_start(position, &truth) == SCID_OK);
    TEST_ASSERT(truth == 1);

    TEST_ASSERT(scid_position_is_check(position, &truth) == SCID_OK);
    TEST_ASSERT(truth == 0);

    TEST_ASSERT(scid_position_is_checkmate(position, &truth) == SCID_OK);
    TEST_ASSERT(truth == 0);

    TEST_ASSERT(scid_position_is_legal(position, &truth) == SCID_OK);
    TEST_ASSERT(truth == 1);

    TEST_ASSERT(scid_position_side_to_move_get(position, &side_to_move) == SCID_OK);
    TEST_ASSERT(side_to_move == SCID_WHITE);

    TEST_ASSERT(scid_position_fullmove_number_get(position, &number) == SCID_OK);
    TEST_ASSERT(number == 1);

    TEST_ASSERT(scid_position_halfmove_clock_get(position, &number) == SCID_OK);
    TEST_ASSERT(number == 0);

    TEST_ASSERT(scid_position_piece_at_get(position, 4, &piece) == SCID_OK);
    TEST_ASSERT(piece == SCID_PIECE_WHITE_KING);

    TEST_ASSERT(scid_position_piece_at_get(position, 56, &piece) == SCID_OK);
    TEST_ASSERT(piece == SCID_PIECE_BLACK_ROOK);

    TEST_ASSERT(scid_position_piece_at_get(position, 28, &piece) == SCID_OK);
    TEST_ASSERT(piece == SCID_PIECE_NONE);

    TEST_ASSERT(
        scid_position_legal_moves(position, moves, SCID_MAX_LEGAL_MOVES, &move_count) == SCID_OK);
    TEST_ASSERT(move_count == 20);
    TEST_ASSERT(test_moves_include(moves, move_count, 12, 28, SCID_PIECE_NONE, 0));
    TEST_ASSERT(test_moves_include(moves, move_count, 6, 21, SCID_PIECE_NONE, 0));
    TEST_ASSERT(!test_moves_include(moves, move_count, 12, 36, SCID_PIECE_NONE, 0));

    move_count = 999;
    TEST_ASSERT(
        scid_position_legal_moves(position, moves, 19, &move_count) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(move_count == 0);

    move_count = 999;
    TEST_ASSERT(
        scid_position_legal_moves(position, NULL, SCID_MAX_LEGAL_MOVES, &move_count) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(move_count == 0);
    TEST_ASSERT(
        scid_position_legal_moves(position, moves, SCID_MAX_LEGAL_MOVES, NULL) ==
        SCID_ERROR_BAD_ARG);

    scid_position_free(position);

    position = NULL;
    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(scid_position_create_with_uci(position, "e2e4", &next_position) == SCID_OK);
    TEST_ASSERT(scid_position_to_fen(position, fen, sizeof(fen), &fen_size) == SCID_OK);
    TEST_ASSERT(strcmp(fen, start_fen) == 0);
    scid_position_free(position);
    position = next_position;
    next_position = NULL;
    TEST_ASSERT(scid_position_to_fen(position, fen, sizeof(fen), &fen_size) == SCID_OK);
    TEST_ASSERT(strcmp(fen, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1") == 0);
    TEST_ASSERT(scid_position_create_with_san(position, "c5", &next_position) == SCID_OK);
    TEST_ASSERT(scid_position_to_fen(position, fen, sizeof(fen), &fen_size) == SCID_OK);
    TEST_ASSERT(strcmp(fen, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1") == 0);
    scid_position_free(position);
    position = next_position;
    next_position = NULL;
    TEST_ASSERT(scid_position_to_fen(position, fen, sizeof(fen), &fen_size) == SCID_OK);
    TEST_ASSERT(strcmp(fen, "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2") == 0);
    TEST_ASSERT(scid_position_apply_uci(position, "g1f3") == SCID_OK);
    TEST_ASSERT(scid_position_apply_uci(position, "not-a-move") == SCID_ERROR_INVALID_MOVE);
    TEST_ASSERT(scid_position_apply_san(position, "not-a-move") == SCID_ERROR_INVALID_MOVE);
    TEST_ASSERT(scid_position_create_with_uci(NULL, "e2e4", &next_position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_position_create_with_uci(position, NULL, &next_position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_position_create_with_uci(position, "not-a-move", &next_position) ==
        SCID_ERROR_INVALID_MOVE);
    TEST_ASSERT(next_position == NULL);
    TEST_ASSERT(scid_position_create_with_uci(position, "g1f3", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_create_with_san(NULL, "e4", &next_position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_position_create_with_san(position, NULL, &next_position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_position_create_with_san(position, "not-a-move", &next_position) ==
        SCID_ERROR_INVALID_MOVE);
    TEST_ASSERT(next_position == NULL);
    TEST_ASSERT(scid_position_create_with_san(position, "Nf3", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_apply_uci(NULL, "e2e4") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_apply_uci(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_apply_san(NULL, "e4") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_apply_san(position, NULL) == SCID_ERROR_BAD_ARG);
    scid_position_free(position);

    position = NULL;
    TEST_ASSERT(scid_position_create_from_fen(custom_fen, &position) == SCID_OK);
    TEST_ASSERT(position != NULL);
    TEST_ASSERT(scid_position_to_fen(position, fen, sizeof(fen), &fen_size) == SCID_OK);
    TEST_ASSERT(strcmp(fen, custom_fen) == 0);
    TEST_ASSERT(fen_size == strlen(custom_fen));

    TEST_ASSERT(scid_position_is_start(position, &truth) == SCID_OK);
    TEST_ASSERT(truth == 0);

    TEST_ASSERT(scid_position_side_to_move_get(position, &side_to_move) == SCID_OK);
    TEST_ASSERT(side_to_move == SCID_WHITE);

    TEST_ASSERT(scid_position_fullmove_number_get(position, &number) == SCID_OK);
    TEST_ASSERT(number == 25);

    TEST_ASSERT(scid_position_halfmove_clock_get(position, &number) == SCID_OK);
    TEST_ASSERT(number == 45);

    TEST_ASSERT(scid_position_to_fen(position, NULL, 0, &fen_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(fen_size == strlen(custom_fen));

    TEST_ASSERT(
        scid_position_to_fen(position, fen, strlen(custom_fen), &fen_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(fen_size == strlen(custom_fen));

    TEST_ASSERT(scid_position_to_fen(position, fen, sizeof(fen), NULL) == SCID_ERROR_BAD_ARG);
    scid_position_free(position);

    position = NULL;
    TEST_ASSERT(scid_position_create_from_fen(check_fen, &position) == SCID_OK);
    TEST_ASSERT(scid_position_is_check(position, &truth) == SCID_OK);
    TEST_ASSERT(truth == 1);
    TEST_ASSERT(scid_position_is_checkmate(position, &truth) == SCID_OK);
    TEST_ASSERT(truth == 0);
    scid_position_free(position);

    position = NULL;
    TEST_ASSERT(scid_position_create_from_fen(mate_fen, &position) == SCID_OK);
    TEST_ASSERT(scid_position_side_to_move_get(position, &side_to_move) == SCID_OK);
    TEST_ASSERT(side_to_move == SCID_WHITE);
    TEST_ASSERT(scid_position_is_check(position, &truth) == SCID_OK);
    TEST_ASSERT(truth == 1);
    TEST_ASSERT(scid_position_is_checkmate(position, &truth) == SCID_OK);
    TEST_ASSERT(truth == 1);
    TEST_ASSERT(
        scid_position_legal_moves(position, moves, SCID_MAX_LEGAL_MOVES, &move_count) == SCID_OK);
    TEST_ASSERT(move_count == 0);
    scid_position_free(position);

    position = NULL;
    TEST_ASSERT(scid_position_create_from_fen(promotion_fen, &position) == SCID_OK);
    TEST_ASSERT(
        scid_position_legal_moves(position, moves, SCID_MAX_LEGAL_MOVES, &move_count) == SCID_OK);
    TEST_ASSERT(test_moves_include(moves, move_count, 48, 56, SCID_PIECE_QUEEN, 0));
    TEST_ASSERT(test_moves_include(moves, move_count, 48, 56, SCID_PIECE_ROOK, 0));
    TEST_ASSERT(test_moves_include(moves, move_count, 48, 56, SCID_PIECE_BISHOP, 0));
    TEST_ASSERT(test_moves_include(moves, move_count, 48, 56, SCID_PIECE_KNIGHT, 0));
    scid_position_free(position);

    position = (scid_position*)1;
    TEST_ASSERT(
        scid_position_create_from_fen("rnb1k2/Q1p5p/p7/4p3/4q3/8/PPP2R1P/2K5 b", &position) ==
        SCID_ERROR_INVALID_FEN);
    TEST_ASSERT(position == NULL);

    TEST_ASSERT(test_position_create_standard(NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(test_position_create_empty(NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_create_from_fen(NULL, &position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_create_from_fen(custom_fen, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_to_fen(NULL, fen, sizeof(fen), &fen_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_is_start(NULL, &truth) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_is_start(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_is_check(NULL, &truth) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_is_check(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_is_checkmate(NULL, &truth) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_is_checkmate(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_is_legal(NULL, &truth) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_is_legal(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_side_to_move_get(NULL, &side_to_move) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_side_to_move_get(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_fullmove_number_get(NULL, &number) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_fullmove_number_get(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_halfmove_clock_get(NULL, &number) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_halfmove_clock_get(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_piece_at_get(NULL, 4, &piece) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_piece_at_get(position, 4, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_piece_at_get(position, 64, &piece) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_position_legal_moves(NULL, moves, SCID_MAX_LEGAL_MOVES, &move_count) ==
        SCID_ERROR_BAD_ARG);

    scid_position_free(next_position);
    scid_position_free(NULL);
}
