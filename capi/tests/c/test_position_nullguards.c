#include "test_libscid.h"

#include "scid/scid.h"

#include <stddef.h>

static void
test_position_create_from_fen_nullguards(void)
{
    scid_position* position = NULL;
    const char*    fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    TEST_ASSERT(scid_position_create_from_fen(NULL, &position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_create_from_fen(fen, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_create_from_fen(NULL, NULL) == SCID_ERROR_BAD_ARG);
}

static void
test_position_create_with_san_nullguards(void)
{
    scid_position* position = NULL;
    scid_position* next_position = NULL;

    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(position != NULL);

    TEST_ASSERT(scid_position_create_with_san(NULL, "e4", &next_position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_create_with_san(position, NULL, &next_position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_create_with_san(position, "e4", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_create_with_san(NULL, NULL, &next_position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_create_with_san(NULL, "e4", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_create_with_san(position, NULL, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_create_with_san(NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(position);
}

static void
test_position_create_with_uci_nullguards(void)
{
    scid_position* position = NULL;
    scid_position* next_position = NULL;

    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(position != NULL);

    TEST_ASSERT(scid_position_create_with_uci(NULL, "e2e4", &next_position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_create_with_uci(position, NULL, &next_position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_create_with_uci(position, "e2e4", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_create_with_uci(NULL, NULL, &next_position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_create_with_uci(NULL, "e2e4", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_create_with_uci(position, NULL, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_create_with_uci(NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(position);
}

static void
test_position_free_nullguards(void)
{
    scid_position_free(NULL);
}

static void
test_position_to_fen_nullguards(void)
{
    scid_position* position = NULL;
    char           buffer[128];
    size_t         text_size = 0;

    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(position != NULL);

    TEST_ASSERT(scid_position_to_fen(NULL, buffer, sizeof(buffer), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_to_fen(position, buffer, sizeof(buffer), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_to_fen(NULL, buffer, sizeof(buffer), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_to_fen(NULL, NULL, 0, &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_to_fen(position, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_position_to_fen(position, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size > 0);

    scid_position_free(position);
}

static void
test_position_apply_san_nullguards(void)
{
    scid_position* position = NULL;

    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(position != NULL);

    TEST_ASSERT(scid_position_apply_san(NULL, "e4") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_apply_san(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_apply_san(NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(position);
}

static void
test_position_apply_uci_nullguards(void)
{
    scid_position* position = NULL;

    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(position != NULL);

    TEST_ASSERT(scid_position_apply_uci(NULL, "e2e4") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_apply_uci(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_apply_uci(NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(position);
}

static void
test_position_legal_moves_nullguards(void)
{
    scid_position* position = NULL;
    scid_movespec  moves[SCID_MAX_LEGAL_MOVES];
    size_t         move_count = 0;

    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(position != NULL);

    TEST_ASSERT(scid_position_legal_moves(NULL, moves, SCID_MAX_LEGAL_MOVES, &move_count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_legal_moves(position, NULL, SCID_MAX_LEGAL_MOVES, &move_count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_legal_moves(position, moves, SCID_MAX_LEGAL_MOVES, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_legal_moves(NULL, NULL, SCID_MAX_LEGAL_MOVES, &move_count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_legal_moves(NULL, moves, SCID_MAX_LEGAL_MOVES, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_legal_moves(position, NULL, SCID_MAX_LEGAL_MOVES, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_legal_moves(NULL, NULL, SCID_MAX_LEGAL_MOVES, NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(position);
}

static void
test_position_predicate_nullguards(void)
{
    scid_position* position = NULL;
    int            flag = 0;

    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(position != NULL);

    TEST_ASSERT(scid_position_is_start(NULL, &flag) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_is_start(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_is_start(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_position_is_check(NULL, &flag) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_is_check(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_is_check(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_position_is_checkmate(NULL, &flag) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_is_checkmate(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_is_checkmate(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_position_is_legal(NULL, &flag) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_is_legal(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_is_legal(NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(position);
}

static void
test_position_state_get_nullguards(void)
{
    scid_position* position = NULL;
    scid_colour    colour = SCID_WHITE;
    unsigned       number = 0;
    scid_piece     piece = SCID_PIECE_NONE;

    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(position != NULL);

    TEST_ASSERT(scid_position_side_to_move_get(NULL, &colour) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_side_to_move_get(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_side_to_move_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_position_fullmove_number_get(NULL, &number) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_fullmove_number_get(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_fullmove_number_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_position_halfmove_clock_get(NULL, &number) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_halfmove_clock_get(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_halfmove_clock_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_position_piece_at_get(NULL, 0, &piece) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_piece_at_get(position, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_position_piece_at_get(NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(position);
}

void
test_position_nullguards(void)
{
    test_position_create_from_fen_nullguards();
    test_position_create_with_san_nullguards();
    test_position_create_with_uci_nullguards();
    test_position_free_nullguards();
    test_position_to_fen_nullguards();
    test_position_apply_san_nullguards();
    test_position_apply_uci_nullguards();
    test_position_legal_moves_nullguards();
    test_position_predicate_nullguards();
    test_position_state_get_nullguards();
}
