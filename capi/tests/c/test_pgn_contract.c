#include "test_libscid.h"

#include "scid/scid.h"

#include <stddef.h>
#include <string.h>

void
test_pgn_contract(void)
{
    const char*            input = "[Event \"Original\"]\n"
                                   "[Site \"Internet\"]\n"
                                   "[Date \"2026.06.07\"]\n"
                                   "[Round \"1\"]\n"
                                   "[White \"White\"]\n"
                                   "[Black \"Black\"]\n"
                                   "[Result \"*\"]\n"
                                   "\n"
                                   "1. e4 e5 2. Nf3 Nc6 *\n";
    scid_game*             game = NULL;
    scid_game*             reparsed = NULL;
    scid_game_cursor*      cursor = NULL;
    scid_game_cursor*      next_cursor = NULL;
    scid_game_pgn_options* pgn_options = NULL;
    scid_movespec          move = {0, 0, 0, 0};
    scid_nag               nag = 0;
    char                   pgn[4096];
    char                   text[256];
    int                    truth = 0;
    size_t                 count = 0;
    size_t                 text_size = 0;

    TEST_ASSERT(test_game_create(input, strlen(input), &game, NULL, 0, NULL) == SCID_OK);
    TEST_ASSERT(scid_game_tag_set(game, "Event", "Edited") == SCID_OK);
    TEST_ASSERT(scid_game_tag_set(game, "Annotator", "C ABI") == SCID_OK);

    TEST_ASSERT(scid_game_cursor_create(game, &cursor) == SCID_OK);
    TEST_ASSERT(scid_game_cursor_comment_set(game, cursor, "Contract start") == SCID_OK);

    TEST_ASSERT(scid_game_cursor_next(cursor, &truth, &next_cursor) == SCID_OK);
    TEST_ASSERT(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    TEST_ASSERT(scid_game_cursor_comment_set(game, cursor, "King pawn") == SCID_OK);
    TEST_ASSERT(scid_game_cursor_nag_add(game, cursor, 1, &truth) == SCID_OK);
    TEST_ASSERT(truth == 1);

    TEST_ASSERT(
        scid_game_cursor_variation_add(game, cursor, "Sicilian branch", &truth, &next_cursor) ==
        SCID_OK);
    TEST_ASSERT(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    TEST_ASSERT(scid_movespec_create_from_uci("c7c5", &move) == SCID_OK);
    TEST_ASSERT(scid_game_cursor_move_add(game, cursor, move, &next_cursor) == SCID_OK);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    TEST_ASSERT(scid_game_cursor_variation_exit(cursor, &truth, &next_cursor) == SCID_OK);
    TEST_ASSERT(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;

    TEST_ASSERT(scid_game_cursor_to_end(cursor, &next_cursor) == SCID_OK);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    TEST_ASSERT(scid_movespec_create_from_uci("b1c3", &move) == SCID_OK);
    TEST_ASSERT(scid_game_cursor_move_add(game, cursor, move, &next_cursor) == SCID_OK);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    TEST_ASSERT(scid_game_cursor_comment_set(game, cursor, "Developed") == SCID_OK);

    TEST_ASSERT(scid_game_to_pgn(game, NULL, pgn, sizeof(pgn), &text_size) == SCID_OK);
    TEST_ASSERT(strstr(pgn, "[Event \"Edited\"]") != NULL);
    TEST_ASSERT(strstr(pgn, "[Annotator \"C ABI\"]") != NULL);
    TEST_ASSERT(strstr(pgn, "{Contract start}") != NULL);
    TEST_ASSERT(strstr(pgn, "e4 $1 {King pawn}") != NULL);
    TEST_ASSERT(strstr(pgn, "{Sicilian branch}") != NULL);
    TEST_ASSERT(strstr(pgn, "c5") != NULL);
    TEST_ASSERT(strstr(pgn, "Nc3 {Developed}") != NULL);

    TEST_ASSERT(scid_game_pgn_options_create(&pgn_options) == SCID_OK);
    TEST_ASSERT(pgn_options != NULL);
    TEST_ASSERT(scid_game_pgn_options_symbolic_nags_set(pgn_options, 1) == SCID_OK);
    TEST_ASSERT(scid_game_to_pgn(game, pgn_options, pgn, sizeof(pgn), &text_size) == SCID_OK);
    TEST_ASSERT(strstr(pgn, "e4 ! {King pawn}") != NULL);
    TEST_ASSERT(strstr(pgn, "$1") == NULL);

    TEST_ASSERT(scid_game_pgn_options_variations_set(pgn_options, 0) == SCID_OK);
    TEST_ASSERT(scid_game_to_pgn(game, pgn_options, pgn, sizeof(pgn), &text_size) == SCID_OK);
    TEST_ASSERT(strstr(pgn, "{Sicilian branch}") == NULL);
    TEST_ASSERT(strstr(pgn, "c5") == NULL);
    TEST_ASSERT(strstr(pgn, "Nc3 {Developed}") != NULL);

    TEST_ASSERT(scid_game_pgn_options_comments_set(pgn_options, 0) == SCID_OK);
    TEST_ASSERT(scid_game_to_pgn(game, pgn_options, pgn, sizeof(pgn), &text_size) == SCID_OK);
    TEST_ASSERT(strstr(pgn, "{King pawn}") == NULL);
    TEST_ASSERT(strstr(pgn, "{Developed}") == NULL);
    TEST_ASSERT(strstr(pgn, "$1") == NULL);
    TEST_ASSERT(strstr(pgn, " ! ") == NULL);
    TEST_ASSERT(strstr(pgn, "Nc3") != NULL);
    scid_game_pgn_options_free(pgn_options);
    pgn_options = NULL;

    TEST_ASSERT(scid_game_to_pgn(game, NULL, pgn, sizeof(pgn), &text_size) == SCID_OK);
    scid_game_cursor_free(cursor);
    cursor = NULL;
    scid_game_free(game);
    game = NULL;

    TEST_ASSERT(test_game_create(pgn, strlen(pgn), &reparsed, NULL, 0, NULL) == SCID_OK);
    TEST_ASSERT(scid_game_tag_get(reparsed, "Event", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Edited") == 0);
    TEST_ASSERT(
        scid_game_tag_get(reparsed, "Annotator", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "C ABI") == 0);

    TEST_ASSERT(scid_game_cursor_create(reparsed, &cursor) == SCID_OK);
    TEST_ASSERT(scid_game_cursor_comment_get(cursor, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Contract start") == 0);

    TEST_ASSERT(scid_game_cursor_next(cursor, &truth, &next_cursor) == SCID_OK);
    TEST_ASSERT(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    TEST_ASSERT(
        scid_game_cursor_previous_move_comment_get(cursor, text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "King pawn") == 0);
    TEST_ASSERT(scid_game_cursor_previous_move_nag_count_get(cursor, &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(scid_game_cursor_previous_move_nag_at_get(cursor, 0, &nag) == SCID_OK);
    TEST_ASSERT(nag == 1);

    TEST_ASSERT(scid_game_cursor_variation_count_get(cursor, &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(scid_game_cursor_variation_enter(cursor, 0, &truth, &next_cursor) == SCID_OK);
    TEST_ASSERT(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    TEST_ASSERT(scid_game_cursor_comment_get(cursor, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Sicilian branch") == 0);
    TEST_ASSERT(
        scid_game_cursor_next_move_san_get(cursor, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "c5") == 0);
    TEST_ASSERT(scid_game_cursor_variation_exit(cursor, &truth, &next_cursor) == SCID_OK);
    TEST_ASSERT(truth == 1);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;

    TEST_ASSERT(scid_game_cursor_to_end(cursor, &next_cursor) == SCID_OK);
    test_cursor_take(&cursor, next_cursor);
    next_cursor = NULL;
    TEST_ASSERT(
        scid_game_cursor_previous_move_san_get(cursor, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Nc3") == 0);
    TEST_ASSERT(
        scid_game_cursor_previous_move_comment_get(cursor, text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "Developed") == 0);

    scid_game_cursor_free(cursor);
    scid_game_cursor_free(next_cursor);
    scid_game_free(reparsed);
}
