#include "test_libscid.h"

#include "scid/scid.h"

#include <stddef.h>
#include <string.h>

static void
test_game_cursor_lifecycle_nullguards(void)
{
    scid_game*        game = NULL;
    scid_game*        other_game = NULL;
    scid_game_cursor* cursor = NULL;
    scid_game_cursor* clone = NULL;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(game != NULL);
    TEST_ASSERT(test_game_create_blank(&other_game) == SCID_OK);
    TEST_ASSERT(other_game != NULL);

    TEST_ASSERT(scid_game_cursor_create(NULL, &cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_create(game, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_create(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_create(game, &cursor) == SCID_OK);
    TEST_ASSERT(cursor != NULL);

    TEST_ASSERT(scid_game_cursor_clone(NULL, cursor, &clone) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_clone(game, NULL, &clone) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_clone(game, cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_clone(other_game, cursor, &clone) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_clone(NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_game_cursor_free(NULL);
    scid_game_cursor_free(clone);
    scid_game_cursor_free(cursor);
    scid_game_free(other_game);
    scid_game_free(game);
}

static void
test_game_cursor_location_nullguards(void)
{
    scid_game*        game = NULL;
    scid_game_cursor* cursor = NULL;
    scid_position*    position = NULL;
    size_t            value = 0;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(scid_game_cursor_create(game, &cursor) == SCID_OK);
    TEST_ASSERT(test_position_create_empty(&position) == SCID_OK);

    TEST_ASSERT(scid_game_cursor_position_get(NULL, position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_position_get(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_position_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_ply_get(NULL, &value) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_ply_get(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_ply_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_variation_count_get(NULL, &value) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_variation_count_get(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_variation_count_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_variation_depth_get(NULL, &value) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_variation_depth_get(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_variation_depth_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_variation_index_get(NULL, &value) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_variation_index_get(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_variation_index_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(position);
    scid_game_cursor_free(cursor);
    scid_game_free(game);
}

static void
test_game_cursor_predicates_nullguards(void)
{
    scid_game*        game = NULL;
    scid_game_cursor* cursor = NULL;
    int               truth = 0;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(scid_game_cursor_create(game, &cursor) == SCID_OK);

    TEST_ASSERT(scid_game_cursor_is_line_start(NULL, &truth) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_is_line_start(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_is_line_start(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_is_line_end(NULL, &truth) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_is_line_end(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_is_line_end(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_is_game_start(NULL, &truth) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_is_game_start(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_is_game_start(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_is_game_end(NULL, &truth) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_is_game_end(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_is_game_end(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_is_variation_start(NULL, &truth) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_is_variation_start(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_is_variation_start(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_is_variation_end(NULL, &truth) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_is_variation_end(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_is_variation_end(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_is_variation_empty(NULL, &truth) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_is_variation_empty(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_is_variation_empty(NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_game_cursor_free(cursor);
    scid_game_free(game);
}

static void
test_game_cursor_comment_nullguards(void)
{
    scid_game*        game = NULL;
    scid_game*        other_game = NULL;
    scid_game_cursor* cursor = NULL;
    char              text[64];
    size_t            text_size = 0;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(test_game_create_blank(&other_game) == SCID_OK);
    TEST_ASSERT(scid_game_cursor_create(game, &cursor) == SCID_OK);

    TEST_ASSERT(
        scid_game_cursor_comment_get(NULL, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_comment_get(cursor, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_comment_get(NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_comment_set(NULL, cursor, "comment") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_comment_set(game, NULL, "comment") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_comment_set(game, cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_comment_set(other_game, cursor, "comment") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_comment_set(NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_game_cursor_free(cursor);
    scid_game_free(other_game);
    scid_game_free(game);
}

static void
test_game_cursor_previous_move_nullguards(void)
{
    scid_game*        game = NULL;
    scid_game_cursor* cursor = NULL;
    scid_movespec     move = {0, 0, 0, 0};
    scid_nag          nag = 0;
    char              text[64];
    size_t            count = 0;
    size_t            text_size = 0;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(scid_game_cursor_create(game, &cursor) == SCID_OK);

    TEST_ASSERT(scid_game_cursor_previous_movespec_get(NULL, &move) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_previous_movespec_get(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_previous_movespec_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_game_cursor_previous_move_san_get(NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_previous_move_san_get(cursor, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_previous_move_san_get(NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_game_cursor_previous_move_comment_get(NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_previous_move_comment_get(cursor, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_previous_move_comment_get(NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_previous_move_nag_count_get(NULL, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_previous_move_nag_count_get(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_previous_move_nag_count_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_previous_move_nag_at_get(NULL, 0, &nag) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_previous_move_nag_at_get(cursor, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_previous_move_nag_at_get(NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    scid_game_cursor_free(cursor);
    scid_game_free(game);
}

static void
test_game_cursor_next_move_nullguards(void)
{
    scid_game*        game = NULL;
    scid_game_cursor* cursor = NULL;
    scid_movespec     move = {0, 0, 0, 0};
    scid_nag          nag = 0;
    char              text[64];
    size_t            count = 0;
    size_t            text_size = 0;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(scid_game_cursor_create(game, &cursor) == SCID_OK);

    TEST_ASSERT(scid_game_cursor_next_movespec_get(NULL, &move) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_next_movespec_get(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_next_movespec_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_game_cursor_next_move_san_get(NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_next_move_san_get(cursor, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_next_move_san_get(NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_game_cursor_next_move_comment_get(NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_next_move_comment_get(cursor, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_next_move_comment_get(NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_next_move_nag_count_get(NULL, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_next_move_nag_count_get(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_next_move_nag_count_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_next_move_nag_at_get(NULL, 0, &nag) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_next_move_nag_at_get(cursor, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_next_move_nag_at_get(NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    scid_game_cursor_free(cursor);
    scid_game_free(game);
}

static void
test_game_cursor_navigation_nullguards(void)
{
    scid_game*        game = NULL;
    scid_game_cursor* cursor = NULL;
    scid_game_cursor* next_cursor = NULL;
    int               truth = 0;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(scid_game_cursor_create(game, &cursor) == SCID_OK);

    TEST_ASSERT(scid_game_cursor_to_start(NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_to_start(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_to_start(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_to_end(NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_to_end(cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_to_end(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_to_ply(NULL, 0, &truth, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_to_ply(cursor, 0, NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_to_ply(cursor, 0, &truth, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_to_ply(NULL, 0, NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_next(NULL, &truth, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_next(cursor, NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_next(cursor, &truth, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_next(NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_previous(NULL, &truth, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_previous(cursor, NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_previous(cursor, &truth, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_previous(NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_game_cursor_variation_enter(NULL, 0, &truth, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_enter(cursor, 0, NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_enter(cursor, 0, &truth, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_enter(NULL, 0, NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_game_cursor_variation_exit(NULL, &truth, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_exit(cursor, NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_exit(cursor, &truth, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_exit(NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_game_cursor_free(cursor);
    scid_game_free(game);
}

static void
test_game_cursor_mutation_nullguards(void)
{
    scid_game*        game = NULL;
    scid_game_cursor* cursor = NULL;
    scid_game_cursor* next_cursor = NULL;
    scid_movespec     move = {12, 28, SCID_PIECE_NONE, 0};
    int               truth = 0;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(scid_game_cursor_create(game, &cursor) == SCID_OK);

    TEST_ASSERT(scid_game_cursor_move_add(NULL, cursor, move, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_move_add(game, NULL, move, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_move_add(game, cursor, move, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_move_add(NULL, NULL, move, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_game_cursor_variation_add(NULL, cursor, "", &truth, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_add(game, NULL, "", &truth, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_add(game, cursor, "", NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_add(game, cursor, "", &truth, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_add(NULL, NULL, "", NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_nag_add(NULL, cursor, 1, &truth) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_nag_add(game, NULL, 1, &truth) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_nag_add(game, cursor, 1, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_nag_add(NULL, NULL, 1, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_nag_remove(NULL, cursor, 1, &truth) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_nag_remove(game, NULL, 1, &truth) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_nag_remove(game, cursor, 1, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_nag_remove(NULL, NULL, 1, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_nag_clear(NULL, cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_nag_clear(game, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_nag_clear(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_game_cursor_variation_promote_to_first(NULL, cursor, &truth, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_promote_to_first(game, NULL, &truth, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_promote_to_first(game, cursor, NULL, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_promote_to_first(game, cursor, &truth, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_promote_to_first(NULL, NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_game_cursor_variation_promote_to_mainline(NULL, cursor, &truth, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_promote_to_mainline(game, NULL, &truth, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_promote_to_mainline(game, cursor, NULL, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_promote_to_mainline(game, cursor, &truth, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_promote_to_mainline(NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_game_cursor_variation_delete(NULL, cursor, &truth, &next_cursor) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_delete(game, NULL, &truth, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_delete(game, cursor, NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_delete(game, cursor, &truth, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_variation_delete(NULL, NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_cursor_truncate(NULL, cursor, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_truncate(game, NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_truncate(game, cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_cursor_truncate(NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_game_cursor_truncate_before_cursor(NULL, cursor, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_truncate_before_cursor(game, NULL, &next_cursor) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_truncate_before_cursor(game, cursor, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_cursor_truncate_before_cursor(NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_game_cursor_free(cursor);
    scid_game_free(game);
}

void
test_game_cursor_nullguards(void)
{
    test_game_cursor_lifecycle_nullguards();
    test_game_cursor_location_nullguards();
    test_game_cursor_predicates_nullguards();
    test_game_cursor_comment_nullguards();
    test_game_cursor_previous_move_nullguards();
    test_game_cursor_next_move_nullguards();
    test_game_cursor_navigation_nullguards();
    test_game_cursor_mutation_nullguards();
}
