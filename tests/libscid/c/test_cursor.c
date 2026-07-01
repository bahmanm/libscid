#include "test_libscid.h"

#include "scid/scid.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

void
test_cursor(
    void)
{
    const char* pgn = "[Event \"Cursor\"]\n"
                      "[Result \"*\"]\n"
                      "\n"
                      "{Before game} 1. e4 $1 {Best by test} ({Queen pawn alternative} "
                      "1. d4 {Queen pawn} d5) e5 *\n";
    scid_game* game = NULL;
    scid_game* other_game = NULL;
    scid_game_cursor* cursor = NULL;
    scid_game_cursor* clone = NULL;
    scid_game_cursor* next_cursor = NULL;
    scid_position* position = NULL;
    char fen[128];
    char text[128];
    int truth = 0;
    scid_movespec move = {0, 0, 0, 0};
    scid_nag nag = 0;
    size_t count = 0;
    size_t text_size = 0;
    size_t value = 0;

    assert(scid_game_create_from_pgn(pgn, strlen(pgn), &game, NULL, 0, NULL) == SCID_OK);
    assert(scid_game_cursor_create(game, &cursor) == SCID_OK);
    assert(cursor != NULL);
    assert(scid_position_create_empty(&position) == SCID_OK);

    assert(scid_game_cursor_ply_get(cursor, &value) == SCID_OK);
    assert(value == 0);
    assert(scid_game_cursor_variation_depth_get(cursor, &value) == SCID_OK);
    assert(value == 0);
    assert(scid_game_cursor_variation_index_get(cursor, &value) == SCID_OK);
    assert(value == 0);
    assert(scid_game_cursor_variation_count_get(cursor, &count) == SCID_OK);
    assert(count == 1);

    assert(scid_game_cursor_is_game_start(cursor, &truth) == SCID_OK);
    assert(truth == 1);
    assert(scid_game_cursor_is_game_end(cursor, &truth) == SCID_OK);
    assert(truth == 0);
    assert(scid_game_cursor_is_line_start(cursor, &truth) == SCID_OK);
    assert(truth == 1);
    assert(scid_game_cursor_is_line_end(cursor, &truth) == SCID_OK);
    assert(truth == 0);
    assert(scid_game_cursor_is_variation_start(cursor, &truth) == SCID_OK);
    assert(truth == 1);
    assert(scid_game_cursor_is_variation_end(cursor, &truth) == SCID_OK);
    assert(truth == 0);
    assert(scid_game_cursor_is_variation_empty(cursor, &truth) == SCID_OK);
    assert(truth == 0);

    assert(scid_game_cursor_position_get(cursor, position) == SCID_OK);
    assert(scid_position_to_fen(position, fen, sizeof(fen), &value) == SCID_OK);
    assert(strcmp(fen, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") == 0);

    assert(scid_game_cursor_comment_get(cursor, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Before game") == 0);
    assert(text_size == 11);
    assert(scid_game_cursor_previous_movespec_get(cursor, &move) == SCID_ERROR_INVALID_MOVE);
    assert(
        scid_game_cursor_previous_move_san_get(cursor, text, sizeof(text), &text_size) ==
        SCID_ERROR_INVALID_MOVE);
    assert(
        scid_game_cursor_previous_move_comment_get(cursor, text, sizeof(text), &text_size) ==
        SCID_ERROR_INVALID_MOVE);
    assert(
        scid_game_cursor_previous_move_nag_count_get(cursor, &count) ==
        SCID_ERROR_INVALID_MOVE);
    assert(
        scid_game_cursor_previous_move_nag_at_get(cursor, 0, &nag) == SCID_ERROR_INVALID_MOVE);

    assert(scid_game_cursor_next_movespec_get(cursor, &move) == SCID_OK);
    assert(move.from == 12);
    assert(move.to == 28);
    assert(move.promotion == SCID_PIECE_NONE);
    assert(move.is_castling == 0);
    assert(
        scid_game_cursor_next_move_san_get(cursor, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "e4") == 0);
    assert(text_size == 2);
    assert(
        scid_game_cursor_next_move_comment_get(cursor, text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "Best by test") == 0);
    assert(text_size == 12);
    assert(scid_game_cursor_next_move_nag_count_get(cursor, &count) == SCID_OK);
    assert(count == 1);
    assert(scid_game_cursor_next_move_nag_at_get(cursor, 0, &nag) == SCID_OK);
    assert(nag == 1);
    assert(scid_game_cursor_next_move_nag_at_get(cursor, 1, &nag) == SCID_ERROR_BAD_ARG);

    assert(scid_game_cursor_next(cursor, &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_ply_get(cursor, &value) == SCID_OK);
    assert(value == 1);
    assert(scid_game_cursor_clone(game, cursor, &clone) == SCID_OK);
    assert(clone != NULL);
    assert(scid_game_cursor_ply_get(clone, &value) == SCID_OK);
    assert(value == 1);
    assert(scid_game_cursor_position_get(cursor, position) == SCID_OK);
    assert(scid_position_to_fen(position, fen, sizeof(fen), &value) == SCID_OK);
    assert(strcmp(fen, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1") == 0);

    assert(scid_game_cursor_previous_movespec_get(cursor, &move) == SCID_OK);
    assert(move.from == 12);
    assert(move.to == 28);
    assert(move.promotion == SCID_PIECE_NONE);
    assert(move.is_castling == 0);
    assert(
        scid_game_cursor_previous_move_san_get(cursor, text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "e4") == 0);
    assert(text_size == 2);
    assert(
        scid_game_cursor_previous_move_comment_get(cursor, text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "Best by test") == 0);
    assert(text_size == 12);
    assert(scid_game_cursor_previous_move_nag_count_get(cursor, &count) == SCID_OK);
    assert(count == 1);
    assert(scid_game_cursor_previous_move_nag_at_get(cursor, 0, &nag) == SCID_OK);
    assert(nag == 1);
    assert(scid_game_cursor_previous_move_nag_at_get(cursor, 1, &nag) == SCID_ERROR_BAD_ARG);

    assert(scid_game_cursor_comment_set(game, cursor, "Changed e4") == SCID_OK);
    assert(scid_game_cursor_comment_get(cursor, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Changed e4") == 0);
    assert(text_size == 10);
    assert(
        scid_game_cursor_previous_move_comment_get(cursor, text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "Changed e4") == 0);
    assert(text_size == 10);

    assert(scid_game_cursor_next(cursor, &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_next(cursor, &truth, &next_cursor) == SCID_OK);
    assert(truth == 0);
    assert(next_cursor == NULL);
    assert(scid_game_cursor_is_game_end(cursor, &truth) == SCID_OK);
    assert(truth == 1);
    assert(scid_game_cursor_ply_get(clone, &value) == SCID_OK);
    assert(value == 1);
    assert(
        scid_game_cursor_previous_move_san_get(clone, text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "e4") == 0);
    assert(scid_game_cursor_next(clone, &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&clone, next_cursor);
    next_cursor = NULL;
    assert(
        scid_game_cursor_previous_move_san_get(clone, text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "e5") == 0);

    assert(scid_game_cursor_previous(cursor, &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_to_ply(cursor, 0, &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;

    assert(scid_game_cursor_variation_enter(cursor, 0, &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_variation_depth_get(cursor, &value) == SCID_OK);
    assert(value == 1);
    assert(scid_game_cursor_is_variation_empty(cursor, &truth) == SCID_OK);
    assert(truth == 0);
    assert(scid_game_cursor_comment_get(cursor, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Queen pawn alternative") == 0);
    assert(text_size == 22);
    assert(scid_game_cursor_position_get(cursor, position) == SCID_OK);
    assert(scid_position_to_fen(position, fen, sizeof(fen), &value) == SCID_OK);
    assert(strcmp(fen, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") == 0);

    assert(scid_game_cursor_next(cursor, &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_position_get(cursor, position) == SCID_OK);
    assert(scid_position_to_fen(position, fen, sizeof(fen), &value) == SCID_OK);
    assert(strcmp(fen, "rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1") == 0);

    assert(scid_game_cursor_variation_exit(cursor, &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_variation_depth_get(cursor, &value) == SCID_OK);
    assert(value == 0);

    assert(scid_game_cursor_to_ply(cursor, 99, &truth, &next_cursor) == SCID_OK);
    assert(truth == 0);
    assert(next_cursor == NULL);
    assert(scid_game_cursor_variation_enter(cursor, 99, &truth, &next_cursor) == SCID_OK);
    assert(truth == 0);
    assert(next_cursor == NULL);
    assert(scid_game_cursor_variation_exit(cursor, &truth, &next_cursor) == SCID_OK);
    assert(truth == 0);
    assert(next_cursor == NULL);

    assert(scid_game_cursor_to_start(cursor, &next_cursor) == SCID_OK);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_is_game_start(cursor, &truth) == SCID_OK);
    assert(truth == 1);
    assert(scid_game_cursor_to_end(cursor, &next_cursor) == SCID_OK);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_is_game_end(cursor, &truth) == SCID_OK);
    assert(truth == 1);
    assert(scid_game_cursor_next_movespec_get(cursor, &move) == SCID_ERROR_INVALID_MOVE);
    assert(
        scid_game_cursor_next_move_san_get(cursor, text, sizeof(text), &text_size) ==
        SCID_ERROR_INVALID_MOVE);
    assert(
        scid_game_cursor_next_move_comment_get(cursor, text, sizeof(text), &text_size) ==
        SCID_ERROR_INVALID_MOVE);
    assert(scid_game_cursor_next_move_nag_count_get(cursor, &count) == SCID_ERROR_INVALID_MOVE);
    assert(scid_game_cursor_next_move_nag_at_get(cursor, 0, &nag) == SCID_ERROR_INVALID_MOVE);

    assert(scid_game_cursor_create(NULL, &cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_create(game, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_clone(NULL, cursor, &clone) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_clone(game, NULL, &clone) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_clone(game, cursor, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_create_empty(&other_game) == SCID_OK);
    assert(scid_game_cursor_clone(other_game, cursor, &clone) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_position_get(NULL, position) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_position_get(cursor, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_ply_get(NULL, &value) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_ply_get(cursor, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_next(NULL, &truth, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_next(cursor, NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_next(cursor, &truth, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_previous(NULL, &truth, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_previous(cursor, NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_previous(cursor, &truth, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_to_ply(NULL, 0, &truth, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_to_ply(cursor, 0, NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_to_ply(cursor, 0, &truth, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_variation_enter(NULL, 0, &truth, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_variation_enter(cursor, 0, NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_variation_enter(cursor, 0, &truth, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_variation_exit(NULL, &truth, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_variation_exit(cursor, NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_variation_exit(cursor, &truth, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_to_start(NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_to_start(cursor, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_to_end(NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_to_end(cursor, NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_comment_get(NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_comment_get(cursor, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_comment_set(NULL, cursor, "comment") == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_comment_set(game, NULL, "comment") == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_comment_set(game, cursor, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_previous_movespec_get(NULL, &move) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_previous_movespec_get(cursor, NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_previous_move_san_get(NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_previous_move_san_get(cursor, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_previous_move_comment_get(NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_previous_move_comment_get(cursor, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_previous_move_nag_count_get(NULL, &count) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_previous_move_nag_at_get(NULL, 0, &nag) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_previous_move_nag_at_get(cursor, 0, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_next_movespec_get(NULL, &move) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_next_movespec_get(cursor, NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_next_move_san_get(NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_next_move_san_get(cursor, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_next_move_comment_get(NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_next_move_comment_get(cursor, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_next_move_nag_count_get(NULL, &count) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_next_move_nag_at_get(NULL, 0, &nag) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_next_move_nag_at_get(cursor, 0, NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(position);
    scid_game_cursor_free(next_cursor);
    scid_game_cursor_free(clone);
    scid_game_cursor_free(cursor);
    scid_game_cursor_free(NULL);
    scid_game_free(other_game);
    scid_game_free(game);
}
