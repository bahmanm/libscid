#include "test_libscid.h"

#include "scid/scid.h"

#include <stddef.h>

static void
test_game_pgn_options_lifecycle_nullguards(void)
{
    TEST_ASSERT(scid_game_pgn_options_create(NULL) == SCID_ERROR_BAD_ARG);
    scid_game_pgn_options_free(NULL);
}

static void
test_game_pgn_options_setters_nullguards(void)
{
    TEST_ASSERT(scid_game_pgn_options_symbolic_nags_set(NULL, 1) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_pgn_options_symbolic_nags_set(NULL, 0) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_pgn_options_supplemental_tags_set(NULL, 1) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_pgn_options_supplemental_tags_set(NULL, 0) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_pgn_options_comments_set(NULL, 1) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_pgn_options_comments_set(NULL, 0) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_pgn_options_variations_set(NULL, 1) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_pgn_options_variations_set(NULL, 0) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_pgn_options_line_width_set(NULL, 80) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_pgn_options_line_width_set(NULL, 0) == SCID_ERROR_BAD_ARG);
}

static void
test_game_to_pgn_nullguards(void)
{
    scid_game*             game = NULL;
    scid_game_pgn_options* options = NULL;
    char                   pgn[1024];
    size_t                 text_size = 0;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(scid_game_pgn_options_create(&options) == SCID_OK);

    TEST_ASSERT(
        scid_game_to_pgn(NULL, options, pgn, sizeof(pgn), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_to_pgn(NULL, NULL, pgn, sizeof(pgn), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_to_pgn(game, options, pgn, sizeof(pgn), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_to_pgn(game, NULL, pgn, sizeof(pgn), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_to_pgn(NULL, NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_game_to_pgn(game, NULL, pgn, sizeof(pgn), &text_size) == SCID_OK);
    TEST_ASSERT(text_size > 0);

    TEST_ASSERT(
        scid_game_to_pgn(game, NULL, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size > 0);

    scid_game_pgn_options_free(options);
    scid_game_free(game);
}

void
test_game_pgn_nullguards(void)
{
    test_game_pgn_options_lifecycle_nullguards();
    test_game_pgn_options_setters_nullguards();
    test_game_to_pgn_nullguards();
}
