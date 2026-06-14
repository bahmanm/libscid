#include "test_libscid.h"

#include "scid/scid.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

void test_database(void) {
    const char* pgn =
        "[Event \"Stored\"]\n"
        "[Site \"Toronto\"]\n"
        "[Date \"2024.06.14\"]\n"
        "[Round \"7\"]\n"
        "[White \"Alpha\"]\n"
        "[Black \"Beta\"]\n"
        "[Result \"1-0\"]\n"
        "[ECO \"C20\"]\n"
        "[EventDate \"2024.06.01\"]\n"
        "\n"
        "1. e4 e5 2. Nf3 1-0\n";
    scid_database* database = NULL;
    scid_game* game = NULL;
    scid_game* loaded = NULL;
    char flags[22];
    char text[128];
    size_t count = 99;
    size_t flags_size = 99;
    size_t text_size = 99;
    int is_open = 0;

    assert(scid_database_create_memory("scratch", &database) == SCID_OK);
    assert(database != NULL);

    assert(scid_database_is_open(database, &is_open) == SCID_OK);
    assert(is_open == 1);

    assert(scid_database_game_count_get(database, &count) == SCID_OK);
    assert(count == 0);

    assert(scid_game_create_from_pgn(pgn, strlen(pgn), &game, NULL, 0, NULL) ==
           SCID_OK);
    assert(game != NULL);

    assert(scid_database_game_add(database, game, NULL) == SCID_OK);
    assert(scid_database_game_count_get(database, &count) == SCID_OK);
    assert(count == 1);

    assert(scid_database_game_add(database, game, "M") == SCID_OK);
    assert(scid_database_game_count_get(database, &count) == SCID_OK);
    assert(count == 2);

    assert(scid_database_game_halfmove_count_get(database, 1, &count) == SCID_OK);
    assert(count == 3);
    assert(scid_database_game_tag_get(
               database, 1, "Event", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Stored") == 0);
    assert(scid_database_game_tag_get(
               database, 1, "Site", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Toronto") == 0);
    assert(scid_database_game_tag_get(
               database, 1, "Date", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "2024.06.14") == 0);
    assert(scid_database_game_tag_get(
               database, 1, "Round", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "7") == 0);
    assert(scid_database_game_tag_get(
               database, 1, "White", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Alpha") == 0);
    assert(scid_database_game_tag_get(
               database, 1, "Black", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Beta") == 0);
    assert(scid_database_game_tag_get(
               database, 1, "Result", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "1-0") == 0);
    assert(scid_database_game_tag_get(
               database, 1, "ECO", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "C20") == 0);
    assert(scid_database_game_tag_get(
               database, 1, "EventDate", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "2024.06.01") == 0);
    assert(scid_database_game_tag_get(
               database, 1, "Missing", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "") == 0);
    assert(text_size == 0);
    assert(scid_database_game_tag_get(database, 1, "Event", NULL, 0, &text_size) ==
           SCID_ERROR_BUFFER_FULL);
    assert(text_size == strlen("Stored"));

    assert(scid_database_game_get(database, 0, &loaded, NULL, 0, NULL) == SCID_OK);
    assert(loaded != NULL);
    assert(scid_game_mainline_halfmove_count_get(loaded, &count) == SCID_OK);
    assert(count == 3);
    scid_game_free(loaded);
    loaded = NULL;

    assert(scid_database_game_get(
               database, 1, &loaded, flags, sizeof(flags), &flags_size) == SCID_OK);
    assert(loaded != NULL);
    assert(strcmp(flags, "M") == 0);
    assert(flags_size == strlen("M"));
    assert(scid_game_tag_get(loaded, "Event", text, sizeof(text), &text_size) ==
           SCID_OK);
    assert(strcmp(text, "Stored") == 0);
    assert(scid_game_mainline_halfmove_count_get(loaded, &count) == SCID_OK);
    assert(count == 3);
    scid_game_free(loaded);
    loaded = NULL;

    assert(scid_database_game_get(
               database, 1, &loaded, NULL, 0, &flags_size) == SCID_ERROR_BUFFER_FULL);
    assert(loaded == NULL);
    assert(flags_size == strlen("M"));
    assert(scid_database_game_get(database, 99, &loaded, NULL, 0, NULL) ==
           SCID_ERROR_BAD_ARG);
    assert(loaded == NULL);

    assert(scid_database_create_memory(NULL, &database) == SCID_ERROR_BAD_ARG);
    assert(scid_database_create_memory("bad", NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_is_open(NULL, &is_open) == SCID_ERROR_BAD_ARG);
    assert(scid_database_is_open(database, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_count_get(NULL, &count) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_count_get(database, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_add(NULL, game, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_add(database, NULL, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_tag_get(NULL, 0, "Event", text, sizeof(text), &text_size) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_tag_get(database, 0, NULL, text, sizeof(text), &text_size) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_tag_get(
               database, 99, "Event", text, sizeof(text), &text_size) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_halfmove_count_get(NULL, 0, &count) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_halfmove_count_get(database, 99, &count) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_halfmove_count_get(database, 0, NULL) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_get(NULL, 0, &loaded, NULL, 0, NULL) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_get(database, 0, NULL, NULL, 0, NULL) ==
           SCID_ERROR_BAD_ARG);

    scid_game_free(game);
    scid_database_free(database);
    scid_database_free(NULL);
}
