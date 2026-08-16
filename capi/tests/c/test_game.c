#include "test_libscid.h"

#include "scid/scid.h"

#include <stddef.h>
#include <string.h>

void
test_game(void)
{
    const char*            pgn = "[Event \"Friendly\"]\n"
                                 "[Site \"Toronto\"]\n"
                                 "[Date \"2024.05.01\"]\n"
                                 "[Round \"1\"]\n"
                                 "[White \"Player A\"]\n"
                                 "[Black \"Player B\"]\n"
                                 "[Result \"*\"]\n"
                                 "[Annotator \"Example\"]\n"
                                 "\n"
                                 "1. e4 e5 *\n";
    const char*            custom_fen = "8/K7/8/8/7k/8/8/8 w - - 45 25";
    const char*            start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    scid_game*             game = NULL;
    scid_game_cursor*      cursor = NULL;
    scid_game_pgn_options* pgn_options = NULL;
    scid_position*         position = NULL;
    char                   name[64];
    char                   text[1024];
    size_t                 name_size = 0;
    size_t                 text_size = 0;
    size_t                 tag_count = 0;
    int                    removed = 0;

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(game != NULL);

    TEST_ASSERT(scid_game_tag_set(game, "Event", "Manual") == SCID_OK);
    TEST_ASSERT(scid_game_tag_get(game, "Event", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Manual") == 0);
    TEST_ASSERT(text_size == strlen("Manual"));

    TEST_ASSERT(scid_game_tag_set(game, "Date", "2024.06.07") == SCID_OK);
    TEST_ASSERT(scid_game_tag_get(game, "Date", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "2024.06.07") == 0);

    TEST_ASSERT(scid_game_tag_set(game, "Result", "1-0") == SCID_OK);
    TEST_ASSERT(scid_game_tag_get(game, "Result", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "1-0") == 0);

    TEST_ASSERT(scid_game_tag_set(game, "Result", "bad-result") == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_game_tag_set(game, "Annotator", "Example") == SCID_OK);
    TEST_ASSERT(scid_game_tag_get(game, "Annotator", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Example") == 0);

    TEST_ASSERT(scid_game_to_pgn(game, NULL, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strstr(text, "[Event \"Manual\"]") != NULL);
    TEST_ASSERT(strstr(text, "[Result \"1-0\"]") != NULL);
    TEST_ASSERT(strstr(text, "[Annotator \"Example\"]") != NULL);
    TEST_ASSERT(scid_game_mainline_halfmove_count_get(game, &tag_count) == SCID_OK);
    TEST_ASSERT(tag_count == 0);
    TEST_ASSERT(scid_game_initial_comment_get(game, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "") == 0);
    TEST_ASSERT(text_size == 0);
    scid_game_free(game);

    game = NULL;
    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(scid_game_create_blank(position, &game) == SCID_OK);
    TEST_ASSERT(game != NULL);
    TEST_ASSERT(scid_game_tag_get(game, "FEN", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "") == 0);
    TEST_ASSERT(scid_game_tag_count_get(game, &tag_count) == SCID_OK);
    TEST_ASSERT(tag_count == 7);
    TEST_ASSERT(scid_game_to_pgn(game, NULL, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strstr(text, "[FEN ") == NULL);
    scid_game_free(game);
    scid_position_free(position);

    game = NULL;
    position = NULL;
    TEST_ASSERT(scid_position_create_from_fen(custom_fen, &position) == SCID_OK);
    TEST_ASSERT(scid_game_create_blank(position, &game) == SCID_OK);
    TEST_ASSERT(game != NULL);
    TEST_ASSERT(scid_game_tag_get(game, "FEN", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, custom_fen) == 0);
    TEST_ASSERT(text_size == strlen(custom_fen));
    TEST_ASSERT(scid_game_tag_count_get(game, &tag_count) == SCID_OK);
    TEST_ASSERT(tag_count == 8);
    TEST_ASSERT(
        scid_game_tag_at_get(
            game, 7, name, sizeof(name), &name_size, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(name, "FEN") == 0);
    TEST_ASSERT(strcmp(text, custom_fen) == 0);
    TEST_ASSERT(scid_game_start_position_get(game, position) == SCID_OK);
    TEST_ASSERT(scid_position_to_fen(position, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, custom_fen) == 0);
    TEST_ASSERT(scid_game_to_pgn(game, NULL, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strstr(text, "[FEN \"8/K7/8/8/7k/8/8/8 w - - 45 25\"]") != NULL);
    TEST_ASSERT(scid_game_tag_remove(game, "FEN", &removed) == SCID_OK);
    TEST_ASSERT(removed == 0);
    TEST_ASSERT(scid_game_tag_get(game, "FEN", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, custom_fen) == 0);
    scid_game_free(game);
    scid_position_free(position);
    position = NULL;

    game = NULL;
    text_size = 99;
    TEST_ASSERT(
        test_game_create(pgn, strlen(pgn), &game, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(game != NULL);
    TEST_ASSERT(text_size == 0);

    TEST_ASSERT(scid_game_tag_get(game, "Event", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Friendly") == 0);

    TEST_ASSERT(scid_game_tag_get(game, "White", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Player A") == 0);

    TEST_ASSERT(scid_game_tag_get(game, "Date", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "2024.05.01") == 0);

    TEST_ASSERT(scid_game_tag_get(game, "Annotator", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Example") == 0);

    TEST_ASSERT(scid_game_tag_get(game, "Missing", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "") == 0);
    TEST_ASSERT(text_size == 0);

    TEST_ASSERT(scid_game_cursor_create(game, &cursor) == SCID_OK);
    TEST_ASSERT(scid_game_cursor_comment_set(game, cursor, "Before start") == SCID_OK);
    scid_game_cursor_free(cursor);
    cursor = NULL;
    TEST_ASSERT(scid_game_initial_comment_get(game, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Before start") == 0);
    TEST_ASSERT(text_size == strlen("Before start"));
    TEST_ASSERT(scid_game_initial_comment_get(game, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == strlen("Before start"));

    TEST_ASSERT(scid_game_tag_set(game, "ECO", "C20") == SCID_OK);
    TEST_ASSERT(scid_game_tag_get(game, "ECO", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "C20") == 0);

    TEST_ASSERT(scid_game_tag_set(game, "EventDate", "2024.04.30") == SCID_OK);
    TEST_ASSERT(scid_game_tag_get(game, "EventDate", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "2024.04.30") == 0);

    TEST_ASSERT(scid_game_to_pgn(game, NULL, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strstr(text, "[ECO \"C20\"]") != NULL);
    TEST_ASSERT(strstr(text, "[EventDate \"2024.04.30\"]") != NULL);

    TEST_ASSERT(scid_game_pgn_options_create(&pgn_options) == SCID_OK);
    TEST_ASSERT(pgn_options != NULL);
    TEST_ASSERT(scid_game_pgn_options_supplemental_tags_set(pgn_options, 0) == SCID_OK);
    TEST_ASSERT(scid_game_pgn_options_comments_set(pgn_options, 0) == SCID_OK);
    TEST_ASSERT(scid_game_to_pgn(game, pgn_options, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strstr(text, "[Event \"Friendly\"]") != NULL);
    TEST_ASSERT(strstr(text, "[ECO \"C20\"]") == NULL);
    TEST_ASSERT(strstr(text, "[EventDate \"2024.04.30\"]") == NULL);
    TEST_ASSERT(strstr(text, "[Annotator \"Example\"]") == NULL);
    TEST_ASSERT(strstr(text, "{Before start}") == NULL);
    TEST_ASSERT(strstr(text, "1.e4 e5") != NULL);
    TEST_ASSERT(scid_game_pgn_options_line_width_set(pgn_options, 5) == SCID_OK);
    TEST_ASSERT(scid_game_to_pgn(game, pgn_options, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strstr(text, "1.e4\ne5") != NULL);
    TEST_ASSERT(scid_game_pgn_options_line_width_set(pgn_options, 0) == SCID_OK);
    scid_game_pgn_options_free(pgn_options);
    pgn_options = NULL;

    TEST_ASSERT(scid_game_tag_count_get(game, &tag_count) == SCID_OK);
    TEST_ASSERT(tag_count == 10);

    TEST_ASSERT(
        scid_game_tag_at_get(
            game, 0, name, sizeof(name), &name_size, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(name, "Event") == 0);
    TEST_ASSERT(name_size == strlen("Event"));
    TEST_ASSERT(strcmp(text, "Friendly") == 0);
    TEST_ASSERT(text_size == strlen("Friendly"));

    TEST_ASSERT(
        scid_game_tag_at_get(
            game, 7, name, sizeof(name), &name_size, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(name, "ECO") == 0);
    TEST_ASSERT(strcmp(text, "C20") == 0);

    TEST_ASSERT(
        scid_game_tag_at_get(
            game, 8, name, sizeof(name), &name_size, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(name, "EventDate") == 0);
    TEST_ASSERT(strcmp(text, "2024.04.30") == 0);

    TEST_ASSERT(
        scid_game_tag_at_get(
            game, 9, name, sizeof(name), &name_size, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(name, "Annotator") == 0);
    TEST_ASSERT(strcmp(text, "Example") == 0);

    TEST_ASSERT(
        scid_game_tag_at_get(
            game, tag_count, name, sizeof(name), &name_size, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_game_tag_at_get(game, 0, NULL, 0, &name_size, text, sizeof(text), &text_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(name_size == strlen("Event"));

    TEST_ASSERT(scid_game_tag_remove(game, "ECO", &removed) == SCID_OK);
    TEST_ASSERT(removed == 1);
    TEST_ASSERT(scid_game_tag_get(game, "ECO", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "") == 0);
    TEST_ASSERT(scid_game_tag_count_get(game, &tag_count) == SCID_OK);
    TEST_ASSERT(tag_count == 9);
    TEST_ASSERT(scid_game_tag_remove(game, "ECO", &removed) == SCID_OK);
    TEST_ASSERT(removed == 0);

    TEST_ASSERT(scid_game_tag_remove(game, "EventDate", &removed) == SCID_OK);
    TEST_ASSERT(removed == 1);
    TEST_ASSERT(scid_game_tag_get(game, "EventDate", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "") == 0);
    TEST_ASSERT(scid_game_tag_count_get(game, &tag_count) == SCID_OK);
    TEST_ASSERT(tag_count == 8);
    TEST_ASSERT(scid_game_tag_remove(game, "EventDate", &removed) == SCID_OK);
    TEST_ASSERT(removed == 0);

    TEST_ASSERT(scid_game_tag_remove(game, "Annotator", &removed) == SCID_OK);
    TEST_ASSERT(removed == 1);
    TEST_ASSERT(scid_game_tag_get(game, "Annotator", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "") == 0);
    TEST_ASSERT(scid_game_tag_count_get(game, &tag_count) == SCID_OK);
    TEST_ASSERT(tag_count == 7);
    TEST_ASSERT(scid_game_tag_remove(game, "Annotator", &removed) == SCID_OK);
    TEST_ASSERT(removed == 0);
    TEST_ASSERT(scid_game_tag_remove(game, "Event", &removed) == SCID_OK);
    TEST_ASSERT(removed == 0);

    TEST_ASSERT(scid_game_tag_get(game, "Event", NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == strlen("Friendly"));

    TEST_ASSERT(scid_game_to_pgn(game, NULL, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size > 0);

    TEST_ASSERT(scid_game_to_pgn(game, NULL, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strstr(text, "[Event \"Friendly\"]") != NULL);
    TEST_ASSERT(strstr(text, "1.e4 e5") != NULL);
    TEST_ASSERT(scid_game_mainline_halfmove_count_get(game, &tag_count) == SCID_OK);
    TEST_ASSERT(tag_count == 2);

    TEST_ASSERT(test_position_create_empty(&position) == SCID_OK);
    TEST_ASSERT(scid_game_start_position_get(game, position) == SCID_OK);
    TEST_ASSERT(scid_position_to_fen(position, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, start_fen) == 0);

    TEST_ASSERT(scid_game_final_position_get(game, position) == SCID_OK);
    TEST_ASSERT(scid_position_to_fen(position, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2") == 0);
    scid_position_free(position);
    position = NULL;

    scid_game_free(game);

    game = NULL;
    TEST_ASSERT(scid_position_create_from_fen(custom_fen, &position) == SCID_OK);
    TEST_ASSERT(
        scid_game_create(position, "25. Kb7 *\n", strlen("25. Kb7 *\n"), &game, NULL, 0, NULL) ==
        SCID_OK);
    TEST_ASSERT(scid_game_start_position_get(game, position) == SCID_OK);
    TEST_ASSERT(scid_position_to_fen(position, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, custom_fen) == 0);
    TEST_ASSERT(scid_game_final_position_get(game, position) == SCID_OK);
    TEST_ASSERT(scid_position_to_fen(position, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "8/1K6/8/8/7k/8/8/8 b - - 46 25") == 0);
    scid_position_free(position);
    position = NULL;
    scid_game_free(game);

    game = NULL;
    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(
        scid_game_create(NULL, pgn, strlen(pgn), &game, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_create(position, NULL, 0, &game, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_create(position, pgn, strlen(pgn), NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    scid_position_free(position);
    position = NULL;
    TEST_ASSERT(test_game_create_blank(NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_create_blank(NULL, &game) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_create_blank(position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_to_pgn(NULL, NULL, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_pgn_options_create(NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_pgn_options_symbolic_nags_set(NULL, 1) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_pgn_options_supplemental_tags_set(NULL, 1) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_pgn_options_comments_set(NULL, 1) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_pgn_options_variations_set(NULL, 1) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_pgn_options_line_width_set(NULL, 80) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_mainline_halfmove_count_get(NULL, &tag_count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_initial_comment_get(NULL, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_tag_get(NULL, "Event", text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_set(NULL, "Event", "x") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_count_get(NULL, &tag_count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_tag_at_get(
            NULL, 0, name, sizeof(name), &name_size, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_remove(NULL, "Event", &removed) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_start_position_get(NULL, position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_final_position_get(NULL, position) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(scid_game_start_position_get(game, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_final_position_get(game, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_mainline_halfmove_count_get(game, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_initial_comment_get(game, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_game_tag_get(game, NULL, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_get(game, "Event", text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_set(game, NULL, "x") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_set(game, "Event", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_count_get(game, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_remove(game, NULL, &removed) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_game_tag_remove(game, "Event", NULL) == SCID_ERROR_BAD_ARG);
    scid_game_free(game);

    scid_game_free(NULL);
}
