#include "test_libscid.h"

#include "scid/scid.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static void remove_scid5_database(const char* path) {
    char filename[256];

    snprintf(filename, sizeof(filename), "%s.si5", path);
    remove(filename);

    snprintf(filename, sizeof(filename), "%s.sg5", path);
    remove(filename);

    snprintf(filename, sizeof(filename), "%s.sn5", path);
    remove(filename);
}

void test_database(void) {
    const char* persisted_path = "_libscid_c_test_scid5";
    const char* missing_path = "_libscid_c_test_missing_scid5";
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
    const char* replacement_pgn =
        "[Event \"Replacement\"]\n"
        "[Site \"Vancouver\"]\n"
        "[Date \"2025.01.02\"]\n"
        "[Round \"1\"]\n"
        "[White \"Gamma\"]\n"
        "[Black \"Delta\"]\n"
        "[Result \"0-1\"]\n"
        "\n"
        "1. d4 d5 0-1\n";
    scid_database* database = NULL;
    scid_game* game = NULL;
    scid_game* replacement = NULL;
    scid_game* loaded = NULL;
    scid_database* persisted = NULL;
    scid_database* reopened = NULL;
    char flags[22];
    char text[128];
    size_t count = 99;
    size_t flags_size = 99;
    size_t text_size = 99;
    scid_eco_code eco_code = 0;
    scid_eco_code expected_eco_code = 0;
    int is_open = 0;
    int deleted = 99;

    assert(scid_database_create_memory("scratch", &database) == SCID_OK);
    assert(database != NULL);

    assert(scid_database_is_open(database, &is_open) == SCID_OK);
    assert(is_open == 1);

    assert(scid_database_game_count_get(database, &count) == SCID_OK);
    assert(count == 0);

    assert(scid_game_create_from_pgn(pgn, strlen(pgn), &game, NULL, 0, NULL) ==
           SCID_OK);
    assert(game != NULL);

    assert(scid_database_game_add(database, game, "D") == SCID_OK);
    assert(scid_database_game_count_get(database, &count) == SCID_OK);
    assert(count == 1);

    assert(scid_database_game_add(database, game, "M") == SCID_OK);
    assert(scid_database_game_count_get(database, &count) == SCID_OK);
    assert(count == 2);

    assert(scid_database_game_halfmove_count_get(database, 1, &count) == SCID_OK);
    assert(count == 3);
    assert(scid_database_game_number_get(database, 1, &count) == SCID_OK);
    assert(count == 2);
    assert(scid_database_game_deleted_get(database, 0, &deleted) == SCID_OK);
    assert(deleted == 1);
    assert(scid_database_game_deleted_get(database, 1, &deleted) == SCID_OK);
    assert(deleted == 0);
    assert(scid_database_game_result_get(
               database, 1, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "1-0") == 0);
    assert(scid_eco_code_from_string("C20", &expected_eco_code) == SCID_OK);
    assert(scid_database_game_eco_get(database, 1, &eco_code) == SCID_OK);
    assert(eco_code == expected_eco_code);
    assert(scid_database_game_date_get(
               database, 1, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "2024.06.14") == 0);
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

    assert(scid_game_create_from_pgn(
               replacement_pgn, strlen(replacement_pgn), &replacement, NULL, 0, NULL) ==
           SCID_OK);
    assert(replacement != NULL);
    assert(scid_database_game_replace(database, 1, replacement, "Q") == SCID_OK);
    assert(scid_database_game_count_get(database, &count) == SCID_OK);
    assert(count == 2);
    assert(scid_database_game_tag_get(
               database, 1, "Event", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Replacement") == 0);
    assert(scid_database_game_result_get(
               database, 1, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "0-1") == 0);
    assert(scid_database_game_date_get(
               database, 1, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "2025.01.02") == 0);
    assert(scid_database_game_halfmove_count_get(database, 1, &count) == SCID_OK);
    assert(count == 2);
    assert(scid_database_game_get(
               database, 1, &loaded, flags, sizeof(flags), &flags_size) == SCID_OK);
    assert(strcmp(flags, "Q") == 0);
    scid_game_free(loaded);
    loaded = NULL;

    assert(scid_database_game_deleted_get(database, 1, &deleted) == SCID_OK);
    assert(deleted == 0);
    assert(scid_database_game_delete(database, 1) == SCID_OK);
    assert(scid_database_game_deleted_get(database, 1, &deleted) == SCID_OK);
    assert(deleted == 1);
    assert(scid_database_game_undelete(database, 1) == SCID_OK);
    assert(scid_database_game_deleted_get(database, 1, &deleted) == SCID_OK);
    assert(deleted == 0);
    assert(scid_database_game_count_get(database, &count) == SCID_OK);
    assert(count == 2);

    remove_scid5_database(persisted_path);
    remove_scid5_database(missing_path);
    assert(scid_database_create_scid5(persisted_path, &persisted) == SCID_OK);
    assert(persisted != NULL);
    assert(scid_database_is_open(persisted, &is_open) == SCID_OK);
    assert(is_open == 1);
    assert(scid_database_game_add(persisted, replacement, "P") == SCID_OK);
    assert(scid_database_game_count_get(persisted, &count) == SCID_OK);
    assert(count == 1);
    assert(scid_database_close(persisted) == SCID_OK);
    assert(scid_database_is_open(persisted, &is_open) == SCID_OK);
    assert(is_open == 0);
    assert(scid_database_close(persisted) == SCID_OK);
    scid_database_free(persisted);
    persisted = NULL;

    assert(scid_database_open_scid5(persisted_path, &reopened) == SCID_OK);
    assert(reopened != NULL);
    assert(scid_database_game_count_get(reopened, &count) == SCID_OK);
    assert(count == 1);
    assert(scid_database_game_tag_get(
               reopened, 0, "Event", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Replacement") == 0);
    assert(scid_database_game_result_get(
               reopened, 0, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "0-1") == 0);
    assert(scid_database_game_get(
               reopened, 0, &loaded, flags, sizeof(flags), &flags_size) == SCID_OK);
    assert(strcmp(flags, "P") == 0);
    scid_game_free(loaded);
    loaded = NULL;
    assert(scid_database_close(reopened) == SCID_OK);
    scid_database_free(reopened);
    reopened = NULL;
    remove_scid5_database(persisted_path);

    assert(scid_database_create_memory(NULL, &database) == SCID_ERROR_BAD_ARG);
    assert(scid_database_create_memory("bad", NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_create_scid5(NULL, &persisted) == SCID_ERROR_BAD_ARG);
    assert(scid_database_create_scid5("bad", NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_open_scid5(NULL, &reopened) == SCID_ERROR_BAD_ARG);
    assert(scid_database_open_scid5("bad", NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_open_scid5(missing_path, &reopened) ==
           SCID_ERROR_FILE_OPEN);
    assert(reopened == NULL);
    assert(scid_database_close(NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_is_open(NULL, &is_open) == SCID_ERROR_BAD_ARG);
    assert(scid_database_is_open(database, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_count_get(NULL, &count) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_count_get(database, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_add(NULL, game, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_add(database, NULL, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_replace(NULL, 0, replacement, NULL) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_replace(database, 99, replacement, NULL) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_replace(database, 0, NULL, NULL) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_delete(NULL, 0) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_delete(database, 99) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_undelete(NULL, 0) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_undelete(database, 99) == SCID_ERROR_BAD_ARG);
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
    assert(scid_database_game_number_get(NULL, 0, &count) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_number_get(database, 99, &count) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_number_get(database, 0, NULL) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_deleted_get(NULL, 0, &deleted) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_deleted_get(database, 99, &deleted) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_deleted_get(database, 0, NULL) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_result_get(NULL, 0, text, sizeof(text), &text_size) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_result_get(
               database, 99, text, sizeof(text), &text_size) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_result_get(database, 0, text, sizeof(text), NULL) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_eco_get(NULL, 0, &eco_code) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_eco_get(database, 99, &eco_code) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_eco_get(database, 0, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_date_get(NULL, 0, text, sizeof(text), &text_size) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_date_get(
               database, 99, text, sizeof(text), &text_size) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_date_get(database, 0, text, sizeof(text), NULL) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_get(NULL, 0, &loaded, NULL, 0, NULL) ==
           SCID_ERROR_BAD_ARG);
    assert(scid_database_game_get(database, 0, NULL, NULL, 0, NULL) ==
           SCID_ERROR_BAD_ARG);

    scid_game_free(game);
    scid_game_free(replacement);
    scid_database_free(database);
    scid_database_free(NULL);
    remove_scid5_database(persisted_path);
    remove_scid5_database(missing_path);
}
