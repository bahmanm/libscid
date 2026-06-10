#include "test_libscid.h"

#include "scid/scid.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

void test_game(void) {
    const char* pgn =
        "[Event \"Friendly\"]\n"
        "[Site \"Toronto\"]\n"
        "[Date \"2024.05.01\"]\n"
        "[Round \"1\"]\n"
        "[White \"Player A\"]\n"
        "[Black \"Player B\"]\n"
        "[Result \"*\"]\n"
        "[Annotator \"Example\"]\n"
        "\n"
        "1. e4 e5 *\n";
    const char* custom_fen = "8/K7/8/8/7k/8/8/8 w - - 45 25";
    const char* start_fen =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    scid_game* game = NULL;
    scid_position* position = NULL;
    char name[64];
    char text[1024];
    size_t name_size = 0;
    size_t text_size = 0;
    size_t tag_count = 0;
    int removed = 0;

    assert(scid_game_create_empty(&game) == SCID_OK);
    assert(game != NULL);

    assert(scid_game_tag_set(game, "Event", "Manual") == SCID_OK);
    assert(scid_game_tag_get(game, "Event", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Manual") == 0);
    assert(text_size == strlen("Manual"));

    assert(scid_game_tag_set(game, "Date", "2024.06.07") == SCID_OK);
    assert(scid_game_tag_get(game, "Date", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "2024.06.07") == 0);

    assert(scid_game_tag_set(game, "Result", "1-0") == SCID_OK);
    assert(scid_game_tag_get(game, "Result", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "1-0") == 0);

    assert(scid_game_tag_set(game, "Result", "bad-result") == SCID_ERROR_BAD_ARG);

    assert(scid_game_tag_set(game, "Annotator", "Example") == SCID_OK);
    assert(scid_game_tag_get(game, "Annotator", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Example") == 0);

    assert(scid_game_to_pgn(game, text, sizeof(text), &text_size) == SCID_OK);
    assert(strstr(text, "[Event \"Manual\"]") != NULL);
    assert(strstr(text, "[Result \"1-0\"]") != NULL);
    assert(strstr(text, "[Annotator \"Example\"]") != NULL);
    scid_game_free(game);

    game = NULL;
    assert(scid_position_create_standard(&position) == SCID_OK);
    assert(scid_game_create_from_position(position, &game) == SCID_OK);
    assert(game != NULL);
    assert(scid_game_tag_get(game, "FEN", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "") == 0);
    assert(scid_game_tag_count_get(game, &tag_count) == SCID_OK);
    assert(tag_count == 7);
    assert(scid_game_to_pgn(game, text, sizeof(text), &text_size) == SCID_OK);
    assert(strstr(text, "[FEN ") == NULL);
    scid_game_free(game);
    scid_position_free(position);

    game = NULL;
    position = NULL;
    assert(scid_position_create_from_fen(custom_fen, &position) == SCID_OK);
    assert(scid_game_create_from_position(position, &game) == SCID_OK);
    assert(game != NULL);
    assert(scid_game_tag_get(game, "FEN", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, custom_fen) == 0);
    assert(text_size == strlen(custom_fen));
    assert(scid_game_tag_count_get(game, &tag_count) == SCID_OK);
    assert(tag_count == 8);
    assert(scid_game_tag_at_get(
               game,
               7,
               name,
               sizeof(name),
               &name_size,
               text,
               sizeof(text),
               &text_size) == SCID_OK);
    assert(strcmp(name, "FEN") == 0);
    assert(strcmp(text, custom_fen) == 0);
    assert(scid_game_start_position_get(game, position) == SCID_OK);
    assert(scid_position_to_fen(position, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, custom_fen) == 0);
    assert(scid_game_to_pgn(game, text, sizeof(text), &text_size) == SCID_OK);
    assert(strstr(text, "[FEN \"8/K7/8/8/7k/8/8/8 w - - 45 25\"]") != NULL);
    assert(scid_game_tag_remove(game, "FEN", &removed) == SCID_OK);
    assert(removed == 0);
    assert(scid_game_tag_get(game, "FEN", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, custom_fen) == 0);
    scid_game_free(game);
    scid_position_free(position);
    position = NULL;

    game = NULL;
    text_size = 99;
    assert(scid_game_create_from_pgn(
               pgn,
               strlen(pgn),
               &game,
               text,
               sizeof(text),
               &text_size) == SCID_OK);
    assert(game != NULL);
    assert(text_size == 0);

    assert(scid_game_tag_get(game, "Event", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Friendly") == 0);

    assert(scid_game_tag_get(game, "White", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Player A") == 0);

    assert(scid_game_tag_get(game, "Date", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "2024.05.01") == 0);

    assert(scid_game_tag_get(game, "Annotator", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Example") == 0);

    assert(scid_game_tag_get(game, "Missing", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "") == 0);
    assert(text_size == 0);

    assert(scid_game_tag_set(game, "ECO", "C20") == SCID_OK);
    assert(scid_game_tag_get(game, "ECO", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "C20") == 0);

    assert(scid_game_tag_set(game, "EventDate", "2024.04.30") == SCID_OK);
    assert(scid_game_tag_get(game, "EventDate", text, sizeof(text), &text_size) ==
           SCID_OK);
    assert(strcmp(text, "2024.04.30") == 0);

    assert(scid_game_to_pgn(game, text, sizeof(text), &text_size) == SCID_OK);
    assert(strstr(text, "[ECO \"C20\"]") != NULL);
    assert(strstr(text, "[EventDate \"2024.04.30\"]") != NULL);

    assert(scid_game_tag_count_get(game, &tag_count) == SCID_OK);
    assert(tag_count == 10);

    assert(scid_game_tag_at_get(
               game,
               0,
               name,
               sizeof(name),
               &name_size,
               text,
               sizeof(text),
               &text_size) == SCID_OK);
    assert(strcmp(name, "Event") == 0);
    assert(name_size == strlen("Event"));
    assert(strcmp(text, "Friendly") == 0);
    assert(text_size == strlen("Friendly"));

    assert(scid_game_tag_at_get(
               game,
               7,
               name,
               sizeof(name),
               &name_size,
               text,
               sizeof(text),
               &text_size) == SCID_OK);
    assert(strcmp(name, "ECO") == 0);
    assert(strcmp(text, "C20") == 0);

    assert(scid_game_tag_at_get(
               game,
               8,
               name,
               sizeof(name),
               &name_size,
               text,
               sizeof(text),
               &text_size) == SCID_OK);
    assert(strcmp(name, "EventDate") == 0);
    assert(strcmp(text, "2024.04.30") == 0);

    assert(scid_game_tag_at_get(
               game,
               9,
               name,
               sizeof(name),
               &name_size,
               text,
               sizeof(text),
               &text_size) == SCID_OK);
    assert(strcmp(name, "Annotator") == 0);
    assert(strcmp(text, "Example") == 0);

    assert(scid_game_tag_at_get(
               game,
               tag_count,
               name,
               sizeof(name),
               &name_size,
               text,
               sizeof(text),
               &text_size) == SCID_ERROR_BAD_ARG);

    assert(scid_game_tag_at_get(
               game,
               0,
               NULL,
               0,
               &name_size,
               text,
               sizeof(text),
               &text_size) == SCID_ERROR_BUFFER_FULL);
    assert(name_size == strlen("Event"));

    assert(scid_game_tag_remove(game, "ECO", &removed) == SCID_OK);
    assert(removed == 1);
    assert(scid_game_tag_get(game, "ECO", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "") == 0);
    assert(scid_game_tag_count_get(game, &tag_count) == SCID_OK);
    assert(tag_count == 9);
    assert(scid_game_tag_remove(game, "ECO", &removed) == SCID_OK);
    assert(removed == 0);

    assert(scid_game_tag_remove(game, "EventDate", &removed) == SCID_OK);
    assert(removed == 1);
    assert(scid_game_tag_get(game, "EventDate", text, sizeof(text), &text_size) ==
           SCID_OK);
    assert(strcmp(text, "") == 0);
    assert(scid_game_tag_count_get(game, &tag_count) == SCID_OK);
    assert(tag_count == 8);
    assert(scid_game_tag_remove(game, "EventDate", &removed) == SCID_OK);
    assert(removed == 0);

    assert(scid_game_tag_remove(game, "Annotator", &removed) == SCID_OK);
    assert(removed == 1);
    assert(scid_game_tag_get(game, "Annotator", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "") == 0);
    assert(scid_game_tag_count_get(game, &tag_count) == SCID_OK);
    assert(tag_count == 7);
    assert(scid_game_tag_remove(game, "Annotator", &removed) == SCID_OK);
    assert(removed == 0);
    assert(scid_game_tag_remove(game, "Event", &removed) == SCID_OK);
    assert(removed == 0);

    assert(scid_game_tag_get(game, "Event", NULL, 0, &text_size) ==
           SCID_ERROR_BUFFER_FULL);
    assert(text_size == strlen("Friendly"));

    assert(scid_game_to_pgn(game, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    assert(text_size > 0);

    assert(scid_game_to_pgn(game, text, sizeof(text), &text_size) == SCID_OK);
    assert(strstr(text, "[Event \"Friendly\"]") != NULL);
    assert(strstr(text, "1.e4 e5") != NULL);

    assert(scid_position_create_empty(&position) == SCID_OK);
    assert(scid_game_start_position_get(game, position) == SCID_OK);
    assert(scid_position_to_fen(position, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, start_fen) == 0);

    assert(scid_game_final_position_get(game, position) == SCID_OK);
    assert(scid_position_to_fen(position, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2") == 0);
    scid_position_free(position);
    position = NULL;

    scid_game_free(game);

    game = NULL;
    assert(scid_game_create_from_pgn(NULL, 0, &game, NULL, 0, NULL) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_game_create_from_pgn(pgn, strlen(pgn), NULL, NULL, 0, NULL) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_game_create_empty(NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_create_from_position(NULL, &game) == SCID_ERROR_BAD_ARG);
    assert(scid_game_create_from_position(position, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_to_pgn(NULL, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    assert(scid_game_tag_get(NULL, "Event", text, sizeof(text), &text_size) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_game_tag_set(NULL, "Event", "x") == SCID_ERROR_BAD_ARG);
    assert(scid_game_tag_count_get(NULL, &tag_count) == SCID_ERROR_BAD_ARG);
    assert(scid_game_tag_at_get(
               NULL,
               0,
               name,
               sizeof(name),
               &name_size,
               text,
               sizeof(text),
               &text_size) == SCID_ERROR_BAD_ARG);
    assert(scid_game_tag_remove(NULL, "Event", &removed) == SCID_ERROR_BAD_ARG);
    assert(scid_game_start_position_get(NULL, position) == SCID_ERROR_BAD_ARG);
    assert(scid_game_final_position_get(NULL, position) == SCID_ERROR_BAD_ARG);

    assert(scid_game_create_empty(&game) == SCID_OK);
    assert(scid_game_start_position_get(game, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_final_position_get(game, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_tag_get(game, NULL, text, sizeof(text), &text_size) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_game_tag_get(game, "Event", text, sizeof(text), NULL) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_game_tag_set(game, NULL, "x") == SCID_ERROR_BAD_ARG);
    assert(scid_game_tag_set(game, "Event", NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_tag_count_get(game, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_game_tag_remove(game, NULL, &removed) == SCID_ERROR_BAD_ARG);
    assert(scid_game_tag_remove(game, "Event", NULL) == SCID_ERROR_BAD_ARG);
    scid_game_free(game);

    scid_game_free(NULL);
}
