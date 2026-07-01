#include "test_libscid.h"

#include "scid/scid.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

static scid_game*
create_source_game_from_cursor_san(
    const scid_game_cursor* cursor,
    const char* san)
{
    scid_position* position = NULL;
    scid_movespec move = {0, 0, 0, 0};
    scid_game* game = NULL;

    assert(test_position_create_empty(&position) == SCID_OK);
    assert(scid_game_cursor_position_get(cursor, position) == SCID_OK);
    assert(scid_movespec_create_from_san(position, san, &move) == SCID_OK);
    assert(scid_game_create_from_moves(position, &move, 1, &game) == SCID_OK);

    scid_position_free(position);
    return game;
}

static void
test_cursor_move_and_nag_mutation(
    void)
{
    scid_game* game = NULL;
    scid_game_cursor* cursor = NULL;
    scid_game_cursor* next_cursor = NULL;
    scid_movespec move = {0, 0, 0, 0};
    scid_nag nag = 0;
    int truth = 0;
    size_t count = 0;

    assert(test_game_create_empty(&game) == SCID_OK);
    assert(scid_game_cursor_create(game, &cursor) == SCID_OK);

    assert(scid_movespec_create_from_uci("e2e4", &move) == SCID_OK);
    assert(scid_game_cursor_move_add(game, cursor, move, &next_cursor) == SCID_OK);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_ply_get(cursor, &count) == SCID_OK);
    assert(count == 1);
    assert(scid_game_cursor_move_add(game, cursor, move, &next_cursor) == SCID_ERROR_INVALID_MOVE);
    assert(next_cursor == NULL);

    assert(scid_game_cursor_nag_add(game, cursor, 3, &truth) == SCID_OK);
    assert(truth == 1);
    assert(scid_game_cursor_nag_add(game, cursor, 10, &truth) == SCID_OK);
    assert(truth == 1);
    assert(scid_game_cursor_previous_move_nag_count_get(cursor, &count) == SCID_OK);
    assert(count == 2);
    assert(scid_game_cursor_previous_move_nag_at_get(cursor, 0, &nag) == SCID_OK);
    assert(nag == 3);
    assert(scid_game_cursor_previous_move_nag_at_get(cursor, 1, &nag) == SCID_OK);
    assert(nag == 10);

    assert(scid_game_cursor_nag_remove(game, cursor, 1, &truth) == SCID_OK);
    assert(truth == 1);
    assert(scid_game_cursor_previous_move_nag_count_get(cursor, &count) == SCID_OK);
    assert(count == 1);
    assert(scid_game_cursor_previous_move_nag_at_get(cursor, 0, &nag) == SCID_OK);
    assert(nag == 10);
    assert(scid_game_cursor_nag_remove(game, cursor, 1, &truth) == SCID_OK);
    assert(truth == 0);
    assert(scid_game_cursor_nag_clear(game, cursor) == SCID_OK);
    assert(scid_game_cursor_previous_move_nag_count_get(cursor, &count) == SCID_OK);
    assert(count == 0);

    assert(scid_movespec_create_from_uci("e7e5", &move) == SCID_OK);
    assert(scid_game_cursor_move_add(game, cursor, move, &next_cursor) == SCID_OK);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_to_ply(cursor, 1, &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_variation_add(game, cursor, "Sicilian", &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_variation_depth_get(cursor, &count) == SCID_OK);
    assert(count == 1);

    assert(scid_movespec_create_from_uci("c7c5", &move) == SCID_OK);
    assert(scid_game_cursor_move_add(game, cursor, move, &next_cursor) == SCID_OK);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_variation_exit(cursor, &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_variation_count_get(cursor, &count) == SCID_OK);
    assert(count == 1);
    assert(scid_game_cursor_variation_enter(cursor, 0, &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_variation_delete(game, cursor, &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_variation_count_get(cursor, &count) == SCID_OK);
    assert(count == 0);

    assert(scid_game_cursor_truncate(game, cursor, &next_cursor) == SCID_OK);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_next(cursor, &truth, &next_cursor) == SCID_OK);
    assert(truth == 0);
    assert(next_cursor == NULL);
    assert(scid_game_cursor_variation_add(game, cursor, NULL, &truth, &next_cursor) == SCID_OK);
    assert(truth == 0);
    assert(next_cursor == NULL);
    assert(scid_game_cursor_truncate_before_cursor(game, cursor, &next_cursor) == SCID_OK);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;

    assert(scid_game_cursor_move_add(NULL, cursor, move, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_move_add(game, NULL, move, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_move_add(game, cursor, move, NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_variation_add(NULL, cursor, "", &truth, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_variation_add(game, NULL, "", &truth, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_variation_add(game, cursor, "", NULL, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_variation_add(game, cursor, "", &truth, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_nag_add(NULL, cursor, 1, &truth) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_nag_add(game, NULL, 1, &truth) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_nag_add(game, cursor, 1, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_nag_remove(NULL, cursor, 1, &truth) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_nag_remove(game, NULL, 1, &truth) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_nag_remove(game, cursor, 1, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_nag_clear(NULL, cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_nag_clear(game, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_variation_delete(NULL, cursor, &truth, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_variation_delete(game, NULL, &truth, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_variation_delete(game, cursor, NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_variation_delete(game, cursor, &truth, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_truncate(NULL, cursor, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_truncate(game, NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_truncate(game, cursor, NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_truncate_before_cursor(NULL, cursor, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_truncate_before_cursor(game, NULL, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    assert(scid_game_cursor_truncate_before_cursor(game, cursor, NULL) == SCID_ERROR_BAD_ARG);

    scid_game_cursor_free(next_cursor);
    scid_game_cursor_free(cursor);
    scid_game_free(game);
}

static void
test_game_merge_moves(
    void)
{
    const char* pgn = "[Event \"Target\"]\n"
                      "[Result \"*\"]\n"
                      "\n"
                      "1. e4 e5 *\n";
    scid_game* game = NULL;
    scid_game* source = NULL;
    scid_game_cursor* cursor = NULL;
    scid_game_cursor* next_cursor = NULL;
    scid_position* position = NULL;
    scid_position* next_position = NULL;
    scid_movespec moves[2];
    char text[2048];
    int truth = 0;
    size_t text_size = 0;
    size_t count = 0;

    assert(scid_game_create_from_pgn(pgn, strlen(pgn), &game, NULL, 0, NULL) == SCID_OK);
    assert(scid_game_cursor_create(game, &cursor) == SCID_OK);
    assert(scid_game_cursor_to_end(cursor, &next_cursor) == SCID_OK);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;

    source = create_source_game_from_cursor_san(cursor, "Nf3");
    assert(
        scid_game_merge_moves(
            game, cursor, source, SCID_GAME_MERGE_MOVES_APPEND, &next_cursor) == SCID_OK);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_ply_get(cursor, &count) == SCID_OK);
    assert(count == 3);
    assert(scid_game_cursor_previous_move_san_get(cursor, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Nf3") == 0);
    scid_game_free(source);
    source = NULL;

    scid_game_cursor_free(cursor);
    scid_game_free(game);
    game = NULL;
    cursor = NULL;

    assert(scid_game_create_from_pgn(pgn, strlen(pgn), &game, NULL, 0, NULL) == SCID_OK);
    assert(scid_game_cursor_create(game, &cursor) == SCID_OK);
    assert(scid_game_cursor_to_ply(cursor, 1, &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    source = create_source_game_from_cursor_san(cursor, "c5");

    assert(
        scid_game_merge_moves(
            game, cursor, source, SCID_GAME_MERGE_MOVES_APPEND, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    assert(next_cursor == NULL);
    assert(scid_game_cursor_next_move_san_get(cursor, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "e5") == 0);
    assert(scid_game_cursor_variation_count_get(cursor, &count) == SCID_OK);
    assert(count == 0);

    assert(
        scid_game_merge_moves(
            game, cursor, source, SCID_GAME_MERGE_MOVES_INSERT_VARIATION, &next_cursor) ==
        SCID_OK);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_variation_depth_get(cursor, &count) == SCID_OK);
    assert(count == 1);
    assert(scid_game_cursor_previous_move_san_get(cursor, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "c5") == 0);
    scid_game_free(source);
    source = NULL;

    assert(scid_game_cursor_variation_exit(cursor, &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_to_end(cursor, &next_cursor) == SCID_OK);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    source = create_source_game_from_cursor_san(cursor, "Nf3");
    assert(
        scid_game_merge_moves(
            game, cursor, source, SCID_GAME_MERGE_MOVES_INSERT_VARIATION, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    assert(next_cursor == NULL);
    scid_game_free(source);
    source = NULL;

    scid_game_cursor_free(cursor);
    scid_game_free(game);
    game = NULL;
    cursor = NULL;

    assert(scid_game_create_from_pgn(pgn, strlen(pgn), &game, NULL, 0, NULL) == SCID_OK);
    assert(scid_game_tag_set(game, "Event", "Target Event") == SCID_OK);
    assert(scid_game_cursor_create(game, &cursor) == SCID_OK);
    assert(scid_game_cursor_to_ply(cursor, 1, &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    source = create_source_game_from_cursor_san(cursor, "c5");
    assert(scid_game_tag_set(source, "Event", "Source Event") == SCID_OK);
    assert(
        scid_game_merge_moves(
            game, cursor, source, SCID_GAME_MERGE_MOVES_REPLACE, &next_cursor) == SCID_OK);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_previous_move_san_get(cursor, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "c5") == 0);
    assert(scid_game_cursor_next(cursor, &truth, &next_cursor) == SCID_OK);
    assert(truth == 0);
    assert(next_cursor == NULL);
    assert(scid_game_tag_get(game, "Event", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Target Event") == 0);
    scid_game_free(source);
    source = NULL;

    assert(test_position_create_standard(&position) == SCID_OK);
    assert(scid_movespec_create_from_san(position, "e4", &moves[0]) == SCID_OK);
    assert(scid_game_create_from_moves(position, moves, 1, &source) == SCID_OK);
    assert(
        scid_game_merge_moves(
            game, cursor, source, SCID_GAME_MERGE_MOVES_APPEND, &next_cursor) ==
        SCID_ERROR_INVALID_MOVE);
    assert(next_cursor == NULL);
    scid_position_free(position);
    position = NULL;
    scid_game_free(source);
    source = NULL;

    assert(test_position_create_standard(&position) == SCID_OK);
    assert(scid_game_create_from_moves(position, NULL, 0, &source) == SCID_OK);
    scid_game_free(source);
    source = NULL;
    assert(scid_game_create_from_moves(position, NULL, 1, &source) == SCID_ERROR_BAD_ARG);
    assert(source == NULL);
    assert(scid_movespec_create_from_san(position, "e4", &moves[0]) == SCID_OK);
    assert(scid_position_create_with_san(position, "e4", &next_position) == SCID_OK);
    assert(scid_movespec_create_from_san(next_position, "e5", &moves[1]) == SCID_OK);
    scid_position_free(next_position);
    next_position = NULL;
    assert(scid_game_create_from_moves(position, moves, 2, &source) == SCID_OK);
    assert(scid_game_mainline_halfmove_count_get(source, &count) == SCID_OK);
    assert(count == 2);
    scid_game_free(source);
    source = NULL;
    scid_position_free(position);
    position = NULL;

    scid_game_cursor_free(cursor);
    scid_game_free(game);
}

static void
test_cursor_variation_promotion(
    void)
{
    const char* pgn = "[Event \"Promote\"]\n"
                      "[Result \"*\"]\n"
                      "\n"
                      "1. e4 e5 (1... c5) (1... e6) *\n";
    scid_game* game = NULL;
    scid_game_cursor* cursor = NULL;
    scid_game_cursor* next_cursor = NULL;
    char text[32];
    int truth = 0;
    size_t count = 0;

    assert(scid_game_create_from_pgn(pgn, strlen(pgn), &game, NULL, 0, NULL) == SCID_OK);
    assert(scid_game_cursor_create(game, &cursor) == SCID_OK);
    assert(scid_game_cursor_next(cursor, &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_variation_count_get(cursor, &count) == SCID_OK);
    assert(count == 2);

    assert(scid_game_cursor_variation_enter(cursor, 1, &truth, &next_cursor) == SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_variation_index_get(cursor, &count) == SCID_OK);
    assert(count == 1);
    assert(
        scid_game_cursor_variation_promote_to_first(game, cursor, &truth, &next_cursor) ==
        SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_variation_index_get(cursor, &count) == SCID_OK);
    assert(count == 0);

    assert(
        scid_game_cursor_variation_promote_to_mainline(game, cursor, &truth, &next_cursor) ==
        SCID_OK);
    assert(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    assert(scid_game_cursor_variation_depth_get(cursor, &count) == SCID_OK);
    assert(count == 0);
    assert(scid_game_cursor_next_move_san_get(cursor, text, sizeof(text), &count) == SCID_OK);
    assert(strcmp(text, "e6") == 0);

    assert(
        scid_game_cursor_variation_promote_to_first(game, cursor, &truth, &next_cursor) ==
        SCID_OK);
    assert(truth == 0);
    assert(next_cursor == NULL);
    assert(
        scid_game_cursor_variation_promote_to_mainline(game, cursor, &truth, &next_cursor) ==
        SCID_OK);
    assert(truth == 0);
    assert(next_cursor == NULL);
    assert(
        scid_game_cursor_variation_promote_to_first(NULL, cursor, &truth, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_variation_promote_to_first(game, NULL, &truth, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_variation_promote_to_first(game, cursor, NULL, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_variation_promote_to_first(game, cursor, &truth, NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_variation_promote_to_mainline(NULL, cursor, &truth, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_variation_promote_to_mainline(game, NULL, &truth, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_variation_promote_to_mainline(game, cursor, NULL, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_game_cursor_variation_promote_to_mainline(game, cursor, &truth, NULL) ==
        SCID_ERROR_BAD_ARG);

    scid_game_cursor_free(next_cursor);
    scid_game_cursor_free(cursor);
    scid_game_free(game);
}

void
test_cursor_mutation(
    void)
{
    test_cursor_move_and_nag_mutation();
    test_game_merge_moves();
    test_cursor_variation_promotion();
}
