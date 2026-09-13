#include "test_libscid.h"

#include "scid/scid.h"

#include <stddef.h>
#include <string.h>

static void
test_game_create_blank_nullguards(void)
{
    scid_position* position = NULL;
    scid_game*     game = NULL;

    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(position != NULL);

    TEST_ASSERT(scid_game_create_blank(NULL, &game) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_create_blank(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_create_blank(NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(position);
}

static void
test_game_create_nullguards(void)
{
    scid_position* position = NULL;
    scid_game*     game = NULL;
    const char*    pgn = "[Event \"Test\"]\n\n1. e4 e5 *\n";
    const size_t   pgn_size = strlen(pgn);

    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(position != NULL);

    TEST_ASSERT(scid_game_create(NULL, pgn, pgn_size, &game, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_create(position, NULL, pgn_size, &game, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_create(position, pgn, pgn_size, NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_create(NULL, NULL, pgn_size, NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(position);
}

static void
test_game_free_nullguards(void)
{
    scid_game_free(NULL);
}

static void
test_game_tag_get_nullguards(void)
{
    scid_game* game = NULL;
    char       text[64];
    size_t     text_size = 0;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(game != NULL);

    TEST_ASSERT(scid_game_tag_get(NULL, "Event", text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_get(game, NULL, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_get(game, "Event", text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_get(NULL, NULL, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_get(game, "Event", NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_get(NULL, NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    scid_game_free(game);
}

static void
test_game_tag_set_nullguards(void)
{
    scid_game* game = NULL;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(game != NULL);

    TEST_ASSERT(scid_game_tag_set(NULL, "Event", "Classical") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_set(game, NULL, "Classical") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_set(game, "Event", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_set(NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_game_free(game);
}

static void
test_game_tag_count_get_nullguards(void)
{
    scid_game* game = NULL;
    size_t     count = 0;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(game != NULL);

    TEST_ASSERT(scid_game_tag_count_get(NULL, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_count_get(game, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_count_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_game_free(game);
}

static void
test_game_tag_at_get_nullguards(void)
{
    scid_game* game = NULL;
    char       name[64];
    char       value[64];
    size_t     name_size = 0;
    size_t     value_size = 0;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(game != NULL);

    TEST_ASSERT(
        scid_game_tag_at_get(NULL, 0, name, sizeof(name), &name_size, value, sizeof(value), &value_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_tag_at_get(game, 0, name, sizeof(name), NULL, value, sizeof(value), &value_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_tag_at_get(game, 0, name, sizeof(name), &name_size, value, sizeof(value), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_tag_at_get(NULL, 0, NULL, 0, NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    scid_game_free(game);
}

static void
test_game_tag_remove_nullguards(void)
{
    scid_game* game = NULL;
    int        removed = 0;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(game != NULL);

    TEST_ASSERT(scid_game_tag_remove(NULL, "ECO", &removed) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_remove(game, NULL, &removed) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_remove(game, "ECO", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_remove(NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_game_free(game);
}

static void
test_game_mainline_halfmove_count_get_nullguards(void)
{
    scid_game* game = NULL;
    size_t     count = 0;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(game != NULL);

    TEST_ASSERT(scid_game_mainline_halfmove_count_get(NULL, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_mainline_halfmove_count_get(game, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_mainline_halfmove_count_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_game_free(game);
}

static void
test_game_initial_comment_get_nullguards(void)
{
    scid_game* game = NULL;
    char       text[64];
    size_t     text_size = 0;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(game != NULL);

    TEST_ASSERT(scid_game_initial_comment_get(NULL, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_initial_comment_get(game, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_initial_comment_get(game, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_initial_comment_get(NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    scid_game_free(game);
}

static void
test_game_boundary_positions_nullguards(void)
{
    scid_game*     game = NULL;
    scid_position* position = NULL;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(game != NULL);
    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(position != NULL);

    TEST_ASSERT(scid_game_start_position_get(NULL, position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_start_position_get(game, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_start_position_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_final_position_get(NULL, position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_final_position_get(game, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_final_position_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(position);
    scid_game_free(game);
}

static void
test_game_merge_moves_nullguards(void)
{
    scid_game*        target_game = NULL;
    scid_game*        source_game = NULL;
    scid_game_cursor* target_cursor = NULL;
    scid_game_cursor* out_cursor = NULL;

    TEST_ASSERT(test_game_create_blank(&target_game) == SCID_OK);
    TEST_ASSERT(target_game != NULL);
    TEST_ASSERT(test_game_create_blank(&source_game) == SCID_OK);
    TEST_ASSERT(source_game != NULL);
    TEST_ASSERT(scid_game_cursor_create(target_game, &target_cursor) == SCID_OK);
    TEST_ASSERT(target_cursor != NULL);

    TEST_ASSERT(
        scid_game_merge_moves(NULL, target_cursor, source_game, SCID_GAME_MERGE_MOVES_APPEND, &out_cursor) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_merge_moves(target_game, NULL, source_game, SCID_GAME_MERGE_MOVES_APPEND, &out_cursor) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_merge_moves(target_game, target_cursor, NULL, SCID_GAME_MERGE_MOVES_APPEND, &out_cursor) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_merge_moves(target_game, target_cursor, source_game, SCID_GAME_MERGE_MOVES_APPEND, NULL) ==
        SCID_ERROR_BAD_ARG);

    scid_game_cursor_free(target_cursor);
    scid_game_free(source_game);
    scid_game_free(target_game);
}

void
test_game_nullguards(void)
{
    test_game_create_blank_nullguards();
    test_game_create_nullguards();
    test_game_free_nullguards();
    test_game_tag_get_nullguards();
    test_game_tag_set_nullguards();
    test_game_tag_count_get_nullguards();
    test_game_tag_at_get_nullguards();
    test_game_tag_remove_nullguards();
    test_game_mainline_halfmove_count_get_nullguards();
    test_game_initial_comment_get_nullguards();
    test_game_boundary_positions_nullguards();
    test_game_merge_moves_nullguards();
}
