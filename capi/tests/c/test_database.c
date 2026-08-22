#include "test_libscid.h"

#include "scid/scid.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

static void
remove_scid5_database(const char* path)
{
    char filename[256];

    snprintf(filename, sizeof(filename), "%s.si5", path);
    remove(filename);

    snprintf(filename, sizeof(filename), "%s.sg5", path);
    remove(filename);

    snprintf(filename, sizeof(filename), "%s.sn5", path);
    remove(filename);
}


static void
remove_file(const char* path)
{
    remove(path);
}


struct progress_report_data
{
        size_t calls;
        size_t last_done;
        size_t last_total;
        int    saw_message;
};


static void
progress_report(
    size_t      done,
    size_t      total,
    const char* message,
    void*       user_data)
{
    struct progress_report_data* data = (struct progress_report_data*)user_data;

    ++data->calls;
    data->last_done = done;
    data->last_total = total;
    data->saw_message = message != NULL;
}


struct should_cancel_data
{
        size_t calls;
        size_t cancel_after_calls;
};


static int
should_cancel(void* user_data)
{
    struct should_cancel_data* data = (struct should_cancel_data*)user_data;

    ++data->calls;
    return data->calls >= data->cancel_after_calls;
}


static void
write_pgn_database(
    const char* path,
    size_t      game_count)
{
    FILE* file = fopen(path, "w");
    TEST_ASSERT(file != NULL);

    for (size_t i = 0; i < game_count; ++i)
    {
        fprintf(
            file,
            "[Event \"PGN %zu\"]\n"
            "[Site \"Vancouver\"]\n"
            "[Date \"2026.08.13\"]\n"
            "[Round \"%zu\"]\n"
            "[White \"White %zu\"]\n"
            "[Black \"Black %zu\"]\n"
            "[Result \"1-0\"]\n"
            "\n"
            "1. e4 e5 2. Nf3 1-0\n\n",
            i + 1, i + 1, i + 1, i + 1);
    }

    TEST_ASSERT(fclose(file) == 0);
}


void
test_database(void)
{
    const char*                 persisted_path = "_libscid_c_test_scid5";
    const char*                 missing_path = "_libscid_c_test_missing_scid5";
    const char*                 pgn_path = "_libscid_c_test.pgn";
    const char*                 large_pgn_path = "_libscid_c_test_large.pgn";
    const char*                 pgn = "[Event \"Stored\"]\n"
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
    const char*                 replacement_pgn = "[Event \"Replacement\"]\n"
                                                  "[Site \"Vancouver\"]\n"
                                                  "[Date \"2025.01.02\"]\n"
                                                  "[Round \"1\"]\n"
                                                  "[White \"Gamma\"]\n"
                                                  "[Black \"Delta\"]\n"
                                                  "[Result \"0-1\"]\n"
                                                  "\n"
                                                  "1. d4 d5 0-1\n";
    const char*                 imported_pgn = "[Event \"Imported\"]\n"
                                               "[Site \"Montreal\"]\n"
                                               "[Date \"2026.02.03\"]\n"
                                               "[White \"Epsilon\"]\n"
                                               "[Black \"Zeta\"]\n"
                                               "[Result \"1/2-1/2\"]\n"
                                               "\n"
                                               "1. c4 c5 1/2-1/2\n"
                                               "\n"
                                               "[Event \"Imported Two\"]\n"
                                               "[Site \"Calgary\"]\n"
                                               "[Date \"2026.02.04\"]\n"
                                               "[White \"Eta\"]\n"
                                               "[Black \"Theta\"]\n"
                                               "[Result \"0-1\"]\n"
                                               "\n"
                                               "1. d4 Nf6 0-1\n";
    scid_database*              database = NULL;
    scid_game*                  game = NULL;
    scid_game*                  replacement = NULL;
    scid_game*                  loaded = NULL;
    scid_database*              persisted = NULL;
    scid_database*              pgn_database = NULL;
    scid_database*              reopened = NULL;
    scid_database*              read_only_database = NULL;
    char                        flags[22];
    char                        diagnostic[1024];
    char                        key[64];
    char                        max_date[32];
    char                        text[1024];
    size_t                      count = 99;
    size_t                      diagnostic_size = 99;
    size_t                      flags_size = 99;
    size_t                      imported_count = 99;
    size_t                      key_size = 99;
    size_t                      max_date_size = 99;
    size_t                      text_size = 99;
    scid_eco_code               eco_code = 0;
    scid_eco_code               expected_eco_code = 0;
    scid_filter_id              filter_id = 0;
    int                         is_open = 0;
    int                         read_only = 99;
    int                         deleted = 99;
    struct progress_report_data progress = {0, 0, 0, 0};
    struct should_cancel_data   cancel = {0, 1};

    TEST_ASSERT(scid_database_create_memory("scratch", &database) == SCID_OK);
    TEST_ASSERT(database != NULL);

    TEST_ASSERT(scid_database_is_open(database, &is_open) == SCID_OK);
    TEST_ASSERT(is_open == 1);
    TEST_ASSERT(scid_database_filename_get(database, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "<clipbase>") == 0);
    TEST_ASSERT(scid_database_type_get(database, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "memory") == 0);
    TEST_ASSERT(scid_database_read_only_get(database, &read_only) == SCID_OK);
    TEST_ASSERT(read_only == 0);
    TEST_ASSERT(scid_database_save(database) == SCID_OK);
    TEST_ASSERT(
        scid_database_metadata_get(database, "type", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "0") == 0);
    TEST_ASSERT(
        scid_database_metadata_get(database, "description", text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "") == 0);
    TEST_ASSERT(text_size == 0);
    TEST_ASSERT(scid_database_metadata_count_get(database, &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(
        scid_database_metadata_at_get(
            database, 0, key, sizeof(key), &key_size, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(key, "type") == 0);
    TEST_ASSERT(key_size == strlen("type"));
    TEST_ASSERT(strcmp(text, "0") == 0);
    TEST_ASSERT(text_size == strlen("0"));

    TEST_ASSERT(scid_database_game_count_get(database, &count) == SCID_OK);
    TEST_ASSERT(count == 0);
    TEST_ASSERT(
        scid_database_stats_date_range_get(
            database, text, sizeof(text), &text_size, max_date, sizeof(max_date), &max_date_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "????.??.??") == 0);
    TEST_ASSERT(strcmp(max_date, "????.??.??") == 0);
    TEST_ASSERT(scid_database_stats_result_count_get(database, "1-0", &count) == SCID_OK);
    TEST_ASSERT(count == 0);

    TEST_ASSERT(test_game_create(pgn, strlen(pgn), &game, NULL, 0, NULL) == SCID_OK);
    TEST_ASSERT(game != NULL);

    TEST_ASSERT(scid_database_game_add(database, game, "D") == SCID_OK);
    TEST_ASSERT(scid_database_game_count_get(database, &count) == SCID_OK);
    TEST_ASSERT(count == 1);

    TEST_ASSERT(scid_database_game_add(database, game, "M") == SCID_OK);
    TEST_ASSERT(scid_database_game_count_get(database, &count) == SCID_OK);
    TEST_ASSERT(count == 2);

    TEST_ASSERT(scid_database_game_halfmove_count_get(database, 1, &count) == SCID_OK);
    TEST_ASSERT(count == 3);
    TEST_ASSERT(scid_database_game_number_get(database, 1, &count) == SCID_OK);
    TEST_ASSERT(count == 2);
    TEST_ASSERT(scid_database_game_deleted_get(database, 0, &deleted) == SCID_OK);
    TEST_ASSERT(deleted == 1);
    TEST_ASSERT(scid_database_game_deleted_get(database, 1, &deleted) == SCID_OK);
    TEST_ASSERT(deleted == 0);
    TEST_ASSERT(
        scid_database_game_result_get(database, 1, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "1-0") == 0);
    TEST_ASSERT(scid_eco_code_from_string("C20", &expected_eco_code) == SCID_OK);
    TEST_ASSERT(scid_database_game_eco_get(database, 1, &eco_code) == SCID_OK);
    TEST_ASSERT(eco_code == expected_eco_code);
    TEST_ASSERT(
        scid_database_game_date_get(database, 1, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "2024.06.14") == 0);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 1, "Event", text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "Stored") == 0);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 1, "Site", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Toronto") == 0);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 1, "Date", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "2024.06.14") == 0);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 1, "Round", text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "7") == 0);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 1, "White", text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "Alpha") == 0);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 1, "Black", text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "Beta") == 0);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 1, "Result", text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "1-0") == 0);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 1, "ECO", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "C20") == 0);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 1, "EventDate", text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "2024.06.01") == 0);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 1, "Missing", text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "") == 0);
    TEST_ASSERT(text_size == 0);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 1, "Event", NULL, 0, &text_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == strlen("Stored"));

    TEST_ASSERT(scid_database_game_get(database, 0, &loaded, NULL, 0, NULL) == SCID_OK);
    TEST_ASSERT(loaded != NULL);
    TEST_ASSERT(scid_game_mainline_halfmove_count_get(loaded, &count) == SCID_OK);
    TEST_ASSERT(count == 3);
    scid_game_free(loaded);
    loaded = NULL;

    TEST_ASSERT(
        scid_database_game_get(database, 1, &loaded, flags, sizeof(flags), &flags_size) == SCID_OK);
    TEST_ASSERT(loaded != NULL);
    TEST_ASSERT(strcmp(flags, "M") == 0);
    TEST_ASSERT(flags_size == strlen("M"));
    TEST_ASSERT(scid_game_tag_get(loaded, "Event", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Stored") == 0);
    TEST_ASSERT(scid_game_mainline_halfmove_count_get(loaded, &count) == SCID_OK);
    TEST_ASSERT(count == 3);
    scid_game_free(loaded);
    loaded = NULL;

    TEST_ASSERT(
        scid_database_game_get(database, 1, &loaded, NULL, 0, &flags_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(loaded == NULL);
    TEST_ASSERT(flags_size == strlen("M"));
    TEST_ASSERT(scid_database_game_get(database, 99, &loaded, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(loaded == NULL);

    TEST_ASSERT(
        test_game_create(replacement_pgn, strlen(replacement_pgn), &replacement, NULL, 0, NULL) ==
        SCID_OK);
    TEST_ASSERT(replacement != NULL);
    TEST_ASSERT(scid_database_game_replace(database, 1, replacement, "Q") == SCID_OK);
    TEST_ASSERT(scid_database_game_count_get(database, &count) == SCID_OK);
    TEST_ASSERT(count == 2);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 1, "Event", text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "Replacement") == 0);
    TEST_ASSERT(
        scid_database_game_result_get(database, 1, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "0-1") == 0);
    TEST_ASSERT(
        scid_database_game_date_get(database, 1, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "2025.01.02") == 0);
    TEST_ASSERT(scid_database_game_halfmove_count_get(database, 1, &count) == SCID_OK);
    TEST_ASSERT(count == 2);
    TEST_ASSERT(
        scid_database_game_get(database, 1, &loaded, flags, sizeof(flags), &flags_size) == SCID_OK);
    TEST_ASSERT(strcmp(flags, "Q") == 0);
    scid_game_free(loaded);
    loaded = NULL;

    TEST_ASSERT(scid_database_game_deleted_get(database, 1, &deleted) == SCID_OK);
    TEST_ASSERT(deleted == 0);
    TEST_ASSERT(scid_database_game_delete(database, 1) == SCID_OK);
    TEST_ASSERT(scid_database_game_deleted_get(database, 1, &deleted) == SCID_OK);
    TEST_ASSERT(deleted == 1);
    TEST_ASSERT(scid_database_game_undelete(database, 1) == SCID_OK);
    TEST_ASSERT(scid_database_game_deleted_get(database, 1, &deleted) == SCID_OK);
    TEST_ASSERT(deleted == 0);
    TEST_ASSERT(scid_database_game_count_get(database, &count) == SCID_OK);
    TEST_ASSERT(count == 2);

    TEST_ASSERT(
        scid_database_import_pgn(
            database, imported_pgn, strlen(imported_pgn), diagnostic, sizeof(diagnostic),
            &diagnostic_size, &imported_count) == SCID_OK);
    TEST_ASSERT(diagnostic_size == 0);
    TEST_ASSERT(imported_count == 2);
    TEST_ASSERT(scid_database_game_count_get(database, &count) == SCID_OK);
    TEST_ASSERT(count == 4);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 2, "Event", text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "Imported") == 0);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 3, "Event", text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "Imported Two") == 0);
    TEST_ASSERT(
        scid_database_game_export_pgn(database, 2, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strstr(text, "[Event \"Imported\"]") != NULL);
    TEST_ASSERT(strstr(text, "1/2-1/2") != NULL);
    TEST_ASSERT(
        scid_database_game_export_pgn(database, 2, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size > 0);
    TEST_ASSERT(
        scid_database_stats_date_range_get(
            database, text, sizeof(text), &text_size, max_date, sizeof(max_date), &max_date_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "2024.06.14") == 0);
    TEST_ASSERT(strcmp(max_date, "2026.02.04") == 0);
    TEST_ASSERT(scid_database_stats_result_count_get(database, "1-0", &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(scid_database_stats_result_count_get(database, "0-1", &count) == SCID_OK);
    TEST_ASSERT(count == 2);
    TEST_ASSERT(scid_database_stats_result_count_get(database, "1/2-1/2", &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(scid_database_stats_result_count_get(database, "*", &count) == SCID_OK);
    TEST_ASSERT(count == 0);

    remove_scid5_database(persisted_path);
    remove_scid5_database(missing_path);
    TEST_ASSERT(scid_database_create_scid5(persisted_path, &persisted) == SCID_OK);
    TEST_ASSERT(persisted != NULL);
    TEST_ASSERT(scid_database_is_open(persisted, &is_open) == SCID_OK);
    TEST_ASSERT(is_open == 1);
    TEST_ASSERT(scid_database_filename_get(persisted, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "_libscid_c_test_scid5.si5") == 0);
    TEST_ASSERT(scid_database_type_get(persisted, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "scid5") == 0);
    TEST_ASSERT(scid_database_read_only_get(persisted, &read_only) == SCID_OK);
    TEST_ASSERT(read_only == 0);
    TEST_ASSERT(
        scid_database_metadata_get(persisted, "description", text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "") == 0);
    TEST_ASSERT(text_size == 0);
    TEST_ASSERT(scid_database_metadata_count_get(persisted, &count) == SCID_OK);
    TEST_ASSERT(count == 9);
    TEST_ASSERT(
        scid_database_metadata_at_get(
            persisted, 1, key, sizeof(key), &key_size, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(key, "description") == 0);
    TEST_ASSERT(key_size == strlen("description"));
    TEST_ASSERT(strcmp(text, "") == 0);
    TEST_ASSERT(text_size == 0);
    TEST_ASSERT(
        scid_database_metadata_set(persisted, "description", "C ABI persistent database") ==
        SCID_OK);
    TEST_ASSERT(
        scid_database_metadata_get(persisted, "description", text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "C ABI persistent database") == 0);
    TEST_ASSERT(
        scid_database_metadata_at_get(
            persisted, 1, key, sizeof(key), &key_size, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(key, "description") == 0);
    TEST_ASSERT(strcmp(text, "C ABI persistent database") == 0);
    TEST_ASSERT(
        scid_database_metadata_get(persisted, "missing", text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "") == 0);
    TEST_ASSERT(text_size == 0);
    TEST_ASSERT(
        scid_database_metadata_get(persisted, "description", NULL, 0, &text_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == strlen("C ABI persistent database"));
    TEST_ASSERT(
        scid_database_metadata_at_get(
            persisted, 1, NULL, 0, &key_size, text, sizeof(text), &text_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(key_size == strlen("description"));
    TEST_ASSERT(
        scid_database_metadata_at_get(
            persisted, 1, key, sizeof(key), &key_size, NULL, 0, &text_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == strlen("C ABI persistent database"));
    TEST_ASSERT(
        scid_database_metadata_at_get(
            persisted, 99, key, sizeof(key), &key_size, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_add(persisted, replacement, "P") == SCID_OK);
    TEST_ASSERT(scid_database_game_count_get(persisted, &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(scid_database_save(persisted) == SCID_OK);
    TEST_ASSERT(scid_database_close(persisted) == SCID_OK);
    TEST_ASSERT(scid_database_is_open(persisted, &is_open) == SCID_OK);
    TEST_ASSERT(is_open == 0);
    TEST_ASSERT(scid_database_save(persisted) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filter_create(persisted, &filter_id) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_count_get(persisted, SCID_FILTER_ALL_GAMES, &count) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_stats_date_range_get(
            persisted, text, sizeof(text), &text_size, max_date, sizeof(max_date),
            &max_date_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_stats_result_count_get(persisted, "0-1", &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_metadata_count_get(persisted, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_metadata_at_get(
            persisted, 0, key, sizeof(key), &key_size, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_metadata_get(persisted, "description", text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_metadata_set(persisted, "description", "closed") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filename_get(persisted, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "<empty>") == 0);
    TEST_ASSERT(scid_database_type_get(persisted, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "") == 0);
    TEST_ASSERT(scid_database_close(persisted) == SCID_OK);
    scid_database_free(persisted);
    persisted = NULL;

    TEST_ASSERT(
        scid_database_open_scid5(persisted_path, NULL, NULL, NULL, NULL, &reopened) == SCID_OK);
    TEST_ASSERT(reopened != NULL);
    TEST_ASSERT(scid_database_filename_get(reopened, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "_libscid_c_test_scid5.si5") == 0);
    TEST_ASSERT(scid_database_type_get(reopened, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "scid5") == 0);
    TEST_ASSERT(scid_database_read_only_get(reopened, &read_only) == SCID_OK);
    TEST_ASSERT(read_only == 0);
    TEST_ASSERT(scid_database_save(reopened) == SCID_OK);
    TEST_ASSERT(
        scid_database_metadata_get(reopened, "description", text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "C ABI persistent database") == 0);
    TEST_ASSERT(
        scid_database_metadata_at_get(
            reopened, 1, key, sizeof(key), &key_size, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(key, "description") == 0);
    TEST_ASSERT(strcmp(text, "C ABI persistent database") == 0);
    TEST_ASSERT(
        scid_database_stats_date_range_get(
            reopened, text, sizeof(text), &text_size, max_date, sizeof(max_date), &max_date_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "2025.01.02") == 0);
    TEST_ASSERT(strcmp(max_date, "2025.01.02") == 0);
    TEST_ASSERT(scid_database_stats_result_count_get(reopened, "0-1", &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(scid_database_stats_result_count_get(reopened, "1-0", &count) == SCID_OK);
    TEST_ASSERT(count == 0);
    TEST_ASSERT(scid_database_game_count_get(reopened, &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(
        scid_database_game_tag_get(reopened, 0, "Event", text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "Replacement") == 0);
    TEST_ASSERT(
        scid_database_game_result_get(reopened, 0, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "0-1") == 0);
    TEST_ASSERT(
        scid_database_game_get(reopened, 0, &loaded, flags, sizeof(flags), &flags_size) == SCID_OK);
    TEST_ASSERT(strcmp(flags, "P") == 0);
    scid_game_free(loaded);
    loaded = NULL;
    TEST_ASSERT(scid_database_close(reopened) == SCID_OK);
    scid_database_free(reopened);
    reopened = NULL;

    /* Verify Scid 5 open with progress monitoring */
    progress = (struct progress_report_data){0, 0, 0, 0};
    TEST_ASSERT(
        scid_database_open_scid5(
            persisted_path, progress_report, &progress, NULL, NULL, &reopened) == SCID_OK);
    TEST_ASSERT(reopened != NULL);
    TEST_ASSERT(progress.calls > 0);
    TEST_ASSERT(progress.last_done == 1);
    TEST_ASSERT(progress.last_total == 1);
    TEST_ASSERT(scid_database_close(reopened) == SCID_OK);
    scid_database_free(reopened);
    reopened = NULL;

    /* Verify Scid 5 open cancellation */
    cancel = (struct should_cancel_data){0, 1};
    TEST_ASSERT(
        scid_database_open_scid5(
            persisted_path, NULL, NULL, should_cancel, &cancel, &reopened) ==
        SCID_ERROR_USER_CANCEL);
    TEST_ASSERT(reopened == NULL);
    TEST_ASSERT(cancel.calls > 0);

    /* Verify Scid 5 read-only open cancellation */
    cancel = (struct should_cancel_data){0, 1};
    TEST_ASSERT(
        scid_database_open_scid5_read_only(
            persisted_path, NULL, NULL, should_cancel, &cancel, &read_only_database) ==
        SCID_ERROR_USER_CANCEL);
    TEST_ASSERT(read_only_database == NULL);
    TEST_ASSERT(cancel.calls > 0);

    /* Verify Scid 5 read-only open with progress monitoring */
    progress = (struct progress_report_data){0, 0, 0, 0};
    TEST_ASSERT(
        scid_database_open_scid5_read_only(
            persisted_path, progress_report, &progress, NULL, NULL, &read_only_database) == SCID_OK);
    TEST_ASSERT(read_only_database != NULL);
    TEST_ASSERT(progress.calls > 0);
    TEST_ASSERT(progress.last_done == 1);
    TEST_ASSERT(progress.last_total == 1);
    TEST_ASSERT(scid_database_read_only_get(read_only_database, &read_only) == SCID_OK);
    TEST_ASSERT(read_only == 1);
    TEST_ASSERT(scid_database_save(read_only_database) == SCID_ERROR_FILE_READ_ONLY);
    TEST_ASSERT(
        scid_database_metadata_get(
            read_only_database, "description", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "C ABI persistent database") == 0);
    TEST_ASSERT(scid_database_metadata_count_get(read_only_database, &count) == SCID_OK);
    TEST_ASSERT(count == 9);
    TEST_ASSERT(
        scid_database_metadata_at_get(
            read_only_database, 1, key, sizeof(key), &key_size, text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(key, "description") == 0);
    TEST_ASSERT(strcmp(text, "C ABI persistent database") == 0);
    TEST_ASSERT(
        scid_database_stats_date_range_get(
            read_only_database, text, sizeof(text), &text_size, max_date, sizeof(max_date),
            &max_date_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "2025.01.02") == 0);
    TEST_ASSERT(strcmp(max_date, "2025.01.02") == 0);
    TEST_ASSERT(scid_database_stats_result_count_get(read_only_database, "0-1", &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(
        scid_database_metadata_set(read_only_database, "description", "readonly") ==
        SCID_ERROR_FILE_READ_ONLY);
    TEST_ASSERT(scid_database_game_count_get(read_only_database, &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(
        scid_database_game_tag_get(
            read_only_database, 0, "Event", text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Replacement") == 0);
    TEST_ASSERT(
        scid_database_game_export_pgn(read_only_database, 0, text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strstr(text, "[Event \"Replacement\"]") != NULL);
    TEST_ASSERT(
        scid_database_game_add(read_only_database, replacement, NULL) == SCID_ERROR_FILE_READ_ONLY);
    TEST_ASSERT(
        scid_database_game_replace(read_only_database, 0, replacement, NULL) ==
        SCID_ERROR_FILE_READ_ONLY);
    TEST_ASSERT(scid_database_game_delete(read_only_database, 0) == SCID_ERROR_FILE_READ_ONLY);
    TEST_ASSERT(scid_database_game_undelete(read_only_database, 0) == SCID_ERROR_FILE_READ_ONLY);
    TEST_ASSERT(
        scid_database_import_pgn(
            read_only_database, imported_pgn, strlen(imported_pgn), diagnostic, sizeof(diagnostic),
            &diagnostic_size, &imported_count) == SCID_ERROR_FILE_READ_ONLY);
    TEST_ASSERT(scid_database_close(read_only_database) == SCID_OK);
    scid_database_free(read_only_database);
    read_only_database = NULL;
    remove_scid5_database(persisted_path);

    TEST_ASSERT(scid_database_create_memory(NULL, &database) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_create_memory("bad", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_create_scid5(NULL, &persisted) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_create_scid5("bad", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_open_scid5(NULL, NULL, NULL, NULL, NULL, &reopened) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_open_scid5("bad", NULL, NULL, NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_open_scid5(missing_path, NULL, NULL, NULL, NULL, &reopened) ==
        SCID_ERROR_FILE_OPEN);
    TEST_ASSERT(reopened == NULL);
    TEST_ASSERT(
        scid_database_open_scid5_read_only(NULL, NULL, NULL, NULL, NULL, &read_only_database) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_open_scid5_read_only("bad", NULL, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_open_scid5_read_only(missing_path, NULL, NULL, NULL, NULL, &read_only_database) ==
        SCID_ERROR_FILE_OPEN);
    TEST_ASSERT(read_only_database == NULL);

    remove_file(pgn_path);
    write_pgn_database(pgn_path, 2);
    TEST_ASSERT(
        scid_database_open_pgn_read_only(
            pgn_path, progress_report, &progress, NULL, NULL, &pgn_database) == SCID_OK);
    TEST_ASSERT(pgn_database != NULL);
    TEST_ASSERT(progress.calls > 0);
    TEST_ASSERT(progress.last_done == 1);
    TEST_ASSERT(progress.last_total == 1);
    TEST_ASSERT(progress.saw_message == 1);
    TEST_ASSERT(scid_database_type_get(pgn_database, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "PGN") == 0);
    TEST_ASSERT(scid_database_read_only_get(pgn_database, &read_only) == SCID_OK);
    TEST_ASSERT(read_only == 1);
    TEST_ASSERT(scid_database_game_count_get(pgn_database, &count) == SCID_OK);
    TEST_ASSERT(count == 2);
    TEST_ASSERT(
        scid_database_game_tag_get(pgn_database, 1, "Event", text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "PGN 2") == 0);
    TEST_ASSERT(scid_database_close(pgn_database) == SCID_OK);
    scid_database_free(pgn_database);
    pgn_database = NULL;

    remove_file(large_pgn_path);
    write_pgn_database(large_pgn_path, 1030);
    TEST_ASSERT(
        scid_database_open_pgn_read_only(
            large_pgn_path, NULL, NULL, should_cancel, &cancel, &pgn_database) ==
        SCID_ERROR_USER_CANCEL);
    TEST_ASSERT(pgn_database == NULL);
    TEST_ASSERT(cancel.calls > 0);

    TEST_ASSERT(
        scid_database_open_pgn_read_only(NULL, NULL, NULL, NULL, NULL, &pgn_database) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_open_pgn_read_only(pgn_path, NULL, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_close(NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_save(NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_metadata_get(NULL, "description", text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_metadata_get(database, NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_metadata_get(database, "description", text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_metadata_count_get(NULL, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_metadata_count_get(database, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_metadata_at_get(
            NULL, 0, key, sizeof(key), &key_size, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_metadata_at_get(
            database, 0, key, sizeof(key), NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_metadata_at_get(
            database, 0, key, sizeof(key), &key_size, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_stats_date_range_get(
            NULL, text, sizeof(text), &text_size, max_date, sizeof(max_date), &max_date_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_stats_date_range_get(
            database, text, sizeof(text), NULL, max_date, sizeof(max_date), &max_date_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_stats_date_range_get(
            database, text, sizeof(text), &text_size, max_date, sizeof(max_date), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_stats_date_range_get(
            database, NULL, 0, &text_size, max_date, sizeof(max_date), &max_date_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == strlen("2024.06.14"));
    TEST_ASSERT(scid_database_stats_result_count_get(NULL, "1-0", &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_stats_result_count_get(database, NULL, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_stats_result_count_get(database, "bad", &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_stats_result_count_get(database, "1-0", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_metadata_set(NULL, "description", "bad") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_metadata_set(database, NULL, "bad") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_metadata_set(database, "description", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_is_open(NULL, &is_open) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_is_open(database, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filename_get(NULL, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filename_get(database, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filename_get(database, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == strlen("<clipbase>"));
    TEST_ASSERT(scid_database_type_get(NULL, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_type_get(database, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_type_get(database, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == strlen("memory"));
    TEST_ASSERT(scid_database_read_only_get(NULL, &read_only) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_read_only_get(database, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_count_get(NULL, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_count_get(database, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_import_pgn(
            NULL, imported_pgn, strlen(imported_pgn), diagnostic, sizeof(diagnostic),
            &diagnostic_size, &imported_count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_import_pgn(
            database, NULL, 0, diagnostic, sizeof(diagnostic), &diagnostic_size, &imported_count) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_import_pgn(
            database, imported_pgn, strlen(imported_pgn), diagnostic, sizeof(diagnostic),
            &diagnostic_size, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_import_pgn(
            database, imported_pgn, strlen(imported_pgn), NULL, 0, NULL, &imported_count) ==
        SCID_OK);
    TEST_ASSERT(imported_count == 2);
    TEST_ASSERT(
        scid_database_game_export_pgn(NULL, 0, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_export_pgn(database, 99, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_export_pgn(database, 0, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_add(NULL, game, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_add(database, NULL, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_replace(NULL, 0, replacement, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_replace(database, 99, replacement, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_replace(database, 0, NULL, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_delete(NULL, 0) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_delete(database, 99) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_undelete(NULL, 0) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_undelete(database, 99) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_tag_get(NULL, 0, "Event", text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 0, NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 99, "Event", text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_halfmove_count_get(NULL, 0, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_halfmove_count_get(database, 99, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_halfmove_count_get(database, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_number_get(NULL, 0, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_number_get(database, 99, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_number_get(database, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_deleted_get(NULL, 0, &deleted) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_deleted_get(database, 99, &deleted) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_deleted_get(database, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_result_get(NULL, 0, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_result_get(database, 99, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_result_get(database, 0, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_eco_get(NULL, 0, &eco_code) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_eco_get(database, 99, &eco_code) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_eco_get(database, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_date_get(NULL, 0, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_date_get(database, 99, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_date_get(database, 0, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_get(NULL, 0, &loaded, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_get(database, 0, NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    scid_game_free(game);
    scid_game_free(replacement);
    scid_database_free(read_only_database);
    scid_database_free(database);
    scid_database_free(NULL);
    remove_scid5_database(persisted_path);
    remove_scid5_database(missing_path);
    remove_file(pgn_path);
    remove_file(large_pgn_path);
}
