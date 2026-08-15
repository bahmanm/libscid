#include "test_libscid.h"

#include "scid/scid.h"

#include <assert.h>
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
    assert(file != NULL);

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

    assert(fclose(file) == 0);
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
    size_t                      game_indexes[4] = {99, 99, 99, 99};
    size_t                      imported_count = 99;
    size_t                      key_size = 99;
    size_t                      list_count = 99;
    size_t                      max_date_size = 99;
    size_t                      sorted_position = 99;
    size_t                      text_size = 99;
    scid_eco_code               eco_code = 0;
    scid_eco_code               expected_eco_code = 0;
    scid_filter_id              filter_id = 0;
    scid_filter_id              filter_id_two = 0;
    scid_search_header_criteria header_search = {0};
    int                         is_open = 0;
    int                         read_only = 99;
    int                         deleted = 99;
    struct progress_report_data progress = {0, 0, 0, 0};
    struct should_cancel_data   cancel = {0, 1};

    assert(scid_database_create_memory("scratch", &database) == SCID_OK);
    assert(database != NULL);

    assert(scid_database_is_open(database, &is_open) == SCID_OK);
    assert(is_open == 1);
    assert(scid_database_filename_get(database, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "<clipbase>") == 0);
    assert(scid_database_type_get(database, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "memory") == 0);
    assert(scid_database_read_only_get(database, &read_only) == SCID_OK);
    assert(read_only == 0);
    assert(scid_database_save(database) == SCID_OK);
    assert(scid_database_metadata_get(database, "type", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "0") == 0);
    assert(
        scid_database_metadata_get(database, "description", text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "") == 0);
    assert(text_size == 0);
    assert(scid_database_metadata_count_get(database, &count) == SCID_OK);
    assert(count == 1);
    assert(
        scid_database_metadata_at_get(
            database, 0, key, sizeof(key), &key_size, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(key, "type") == 0);
    assert(key_size == strlen("type"));
    assert(strcmp(text, "0") == 0);
    assert(text_size == strlen("0"));

    assert(scid_database_game_count_get(database, &count) == SCID_OK);
    assert(count == 0);
    assert(
        scid_database_stats_date_range_get(
            database, text, sizeof(text), &text_size, max_date, sizeof(max_date), &max_date_size) ==
        SCID_OK);
    assert(strcmp(text, "????.??.??") == 0);
    assert(strcmp(max_date, "????.??.??") == 0);
    assert(scid_database_stats_result_count_get(database, "1-0", &count) == SCID_OK);
    assert(count == 0);

    assert(test_game_create(pgn, strlen(pgn), &game, NULL, 0, NULL) == SCID_OK);
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
    assert(scid_database_game_result_get(database, 1, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "1-0") == 0);
    assert(scid_eco_code_from_string("C20", &expected_eco_code) == SCID_OK);
    assert(scid_database_game_eco_get(database, 1, &eco_code) == SCID_OK);
    assert(eco_code == expected_eco_code);
    assert(scid_database_game_date_get(database, 1, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "2024.06.14") == 0);
    assert(
        scid_database_game_tag_get(database, 1, "Event", text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "Stored") == 0);
    assert(
        scid_database_game_tag_get(database, 1, "Site", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Toronto") == 0);
    assert(
        scid_database_game_tag_get(database, 1, "Date", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "2024.06.14") == 0);
    assert(
        scid_database_game_tag_get(database, 1, "Round", text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "7") == 0);
    assert(
        scid_database_game_tag_get(database, 1, "White", text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "Alpha") == 0);
    assert(
        scid_database_game_tag_get(database, 1, "Black", text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "Beta") == 0);
    assert(
        scid_database_game_tag_get(database, 1, "Result", text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "1-0") == 0);
    assert(
        scid_database_game_tag_get(database, 1, "ECO", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "C20") == 0);
    assert(
        scid_database_game_tag_get(database, 1, "EventDate", text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "2024.06.01") == 0);
    assert(
        scid_database_game_tag_get(database, 1, "Missing", text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "") == 0);
    assert(text_size == 0);
    assert(
        scid_database_game_tag_get(database, 1, "Event", NULL, 0, &text_size) ==
        SCID_ERROR_BUFFER_FULL);
    assert(text_size == strlen("Stored"));

    assert(scid_database_game_get(database, 0, &loaded, NULL, 0, NULL) == SCID_OK);
    assert(loaded != NULL);
    assert(scid_game_mainline_halfmove_count_get(loaded, &count) == SCID_OK);
    assert(count == 3);
    scid_game_free(loaded);
    loaded = NULL;

    assert(
        scid_database_game_get(database, 1, &loaded, flags, sizeof(flags), &flags_size) == SCID_OK);
    assert(loaded != NULL);
    assert(strcmp(flags, "M") == 0);
    assert(flags_size == strlen("M"));
    assert(scid_game_tag_get(loaded, "Event", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Stored") == 0);
    assert(scid_game_mainline_halfmove_count_get(loaded, &count) == SCID_OK);
    assert(count == 3);
    scid_game_free(loaded);
    loaded = NULL;

    assert(
        scid_database_game_get(database, 1, &loaded, NULL, 0, &flags_size) ==
        SCID_ERROR_BUFFER_FULL);
    assert(loaded == NULL);
    assert(flags_size == strlen("M"));
    assert(scid_database_game_get(database, 99, &loaded, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    assert(loaded == NULL);

    assert(
        test_game_create(replacement_pgn, strlen(replacement_pgn), &replacement, NULL, 0, NULL) ==
        SCID_OK);
    assert(replacement != NULL);
    assert(scid_database_game_replace(database, 1, replacement, "Q") == SCID_OK);
    assert(scid_database_game_count_get(database, &count) == SCID_OK);
    assert(count == 2);
    assert(
        scid_database_game_tag_get(database, 1, "Event", text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "Replacement") == 0);
    assert(scid_database_game_result_get(database, 1, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "0-1") == 0);
    assert(scid_database_game_date_get(database, 1, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "2025.01.02") == 0);
    assert(scid_database_game_halfmove_count_get(database, 1, &count) == SCID_OK);
    assert(count == 2);
    assert(
        scid_database_game_get(database, 1, &loaded, flags, sizeof(flags), &flags_size) == SCID_OK);
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

    assert(
        scid_database_import_pgn(
            database, imported_pgn, strlen(imported_pgn), diagnostic, sizeof(diagnostic),
            &diagnostic_size, &imported_count) == SCID_OK);
    assert(diagnostic_size == 0);
    assert(imported_count == 2);
    assert(scid_database_game_count_get(database, &count) == SCID_OK);
    assert(count == 4);
    assert(
        scid_database_game_tag_get(database, 2, "Event", text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "Imported") == 0);
    assert(
        scid_database_game_tag_get(database, 3, "Event", text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "Imported Two") == 0);
    assert(scid_database_game_export_pgn(database, 2, text, sizeof(text), &text_size) == SCID_OK);
    assert(strstr(text, "[Event \"Imported\"]") != NULL);
    assert(strstr(text, "1/2-1/2") != NULL);
    assert(
        scid_database_game_export_pgn(database, 2, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    assert(text_size > 0);
    assert(
        scid_database_stats_date_range_get(
            database, text, sizeof(text), &text_size, max_date, sizeof(max_date), &max_date_size) ==
        SCID_OK);
    assert(strcmp(text, "2024.06.14") == 0);
    assert(strcmp(max_date, "2026.02.04") == 0);
    assert(scid_database_stats_result_count_get(database, "1-0", &count) == SCID_OK);
    assert(count == 1);
    assert(scid_database_stats_result_count_get(database, "0-1", &count) == SCID_OK);
    assert(count == 2);
    assert(scid_database_stats_result_count_get(database, "1/2-1/2", &count) == SCID_OK);
    assert(count == 1);
    assert(scid_database_stats_result_count_get(database, "*", &count) == SCID_OK);
    assert(count == 0);

    assert(scid_database_filter_create(database, &filter_id) == SCID_OK);
    assert(filter_id > 0);
    assert(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    assert(count == 4);
    assert(scid_database_filter_game_count_get(database, SCID_FILTER_ALL_GAMES, &count) == SCID_OK);
    assert(count == 4);
    assert(scid_database_filter_game_count_get(database, SCID_FILTER_PRIMARY, &count) == SCID_OK);
    assert(count == 4);

    assert(scid_database_filter_create(database, &filter_id_two) == SCID_OK);
    assert(filter_id_two > 0);
    assert(filter_id != filter_id_two);
    assert(scid_database_filter_game_count_get(database, filter_id_two, &count) == SCID_OK);
    assert(count == 4);

    header_search.white = "Gamma";
    assert(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, filter_id, &header_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    assert(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    assert(count == 1);
    assert(
        scid_database_filter_game_indices_get(
            database, filter_id, "d+", 0, 1, game_indexes, 4, &list_count) == SCID_OK);
    assert(list_count == 1);
    assert(game_indexes[0] == 1);

    memset(&header_search, 0, sizeof(header_search));
    header_search.result = "0-1";
    assert(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, filter_id_two, &header_search, progress_report,
            &progress, NULL, NULL) == SCID_OK);
    assert(progress.calls > 0);
    assert(scid_database_filter_game_count_get(database, filter_id_two, &count) == SCID_OK);
    assert(count == 2);

    memset(&header_search, 0, sizeof(header_search));
    header_search.result = "1-0, 1/2-1/2";
    assert(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, filter_id, &header_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    assert(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    assert(count == 2);

    memset(&header_search, 0, sizeof(header_search));
    header_search.event = "Imported";
    assert(
        scid_database_search_headers(
            database, filter_id_two, filter_id, &header_search, NULL, NULL, NULL, NULL) == SCID_OK);
    assert(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    assert(count == 1);
    assert(
        scid_database_filter_game_indices_get(
            database, filter_id, "d+", 0, 1, game_indexes, 4, &list_count) == SCID_OK);
    assert(list_count == 1);
    assert(game_indexes[0] == 3);

    memset(&header_search, 0, sizeof(header_search));
    header_search.date_min = "2026.02.01";
    header_search.date_max = "2026.02.04";
    assert(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, filter_id, &header_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    assert(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    assert(count == 2);

    memset(&header_search, 0, sizeof(header_search));
    header_search.result = "bad-result";
    assert(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, filter_id, &header_search, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);

    memset(&header_search, 0, sizeof(header_search));
    header_search.event = "Imported";
    cancel.calls = 0;
    cancel.cancel_after_calls = 1;
    assert(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, filter_id, &header_search, NULL, NULL, should_cancel,
            &cancel) == SCID_ERROR_USER_CANCEL);
    assert(cancel.calls > 0);

    assert(
        scid_database_filter_game_indices_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, 4, game_indexes, 4, &list_count) == SCID_OK);
    assert(list_count == 4);
    assert(game_indexes[0] == 0);
    assert(game_indexes[1] == 1);
    assert(game_indexes[2] == 2);
    assert(game_indexes[3] == 3);
    assert(
        scid_database_filter_game_indices_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 1, 2, game_indexes, 4, &list_count) == SCID_OK);
    assert(list_count == 2);
    assert(game_indexes[0] == 1);
    assert(game_indexes[1] == 2);
    assert(
        scid_database_filter_game_index_at_row_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 2, &count) == SCID_OK);
    assert(count == 2);
    assert(
        scid_database_filter_game_row_for_index_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 2, &sorted_position) == SCID_OK);
    assert(sorted_position == 2);
    assert(
        scid_database_filter_game_indices_get(
            database, filter_id_two, "d+", 0, 2, game_indexes, 4, &list_count) == SCID_OK);
    assert(list_count == 2);
    assert(game_indexes[0] == 1);
    assert(game_indexes[1] == 3);
    assert(
        scid_database_filter_game_index_at_row_get(database, filter_id_two, "d+", 1, &count) ==
        SCID_OK);
    assert(count == 3);
    assert(
        scid_database_filter_game_row_for_index_get(
            database, filter_id_two, "d+", 3, &sorted_position) == SCID_OK);
    assert(sorted_position == 1);
    assert(
        scid_database_filter_game_index_at_row_get(database, filter_id_two, "d+", 99, &count) ==
        SCID_ERROR_BAD_ARG);
    assert(scid_database_filter_delete(database, SCID_FILTER_ALL_GAMES) == SCID_ERROR_BAD_ARG);
    assert(scid_database_filter_delete(database, SCID_FILTER_PRIMARY) == SCID_ERROR_BAD_ARG);
    assert(scid_database_filter_delete(database, filter_id) == SCID_OK);
    assert(scid_database_filter_delete(database, filter_id_two) == SCID_OK);

    remove_scid5_database(persisted_path);
    remove_scid5_database(missing_path);
    assert(scid_database_create_scid5(persisted_path, &persisted) == SCID_OK);
    assert(persisted != NULL);
    assert(scid_database_is_open(persisted, &is_open) == SCID_OK);
    assert(is_open == 1);
    assert(scid_database_filename_get(persisted, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "_libscid_c_test_scid5.si5") == 0);
    assert(scid_database_type_get(persisted, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "scid5") == 0);
    assert(scid_database_read_only_get(persisted, &read_only) == SCID_OK);
    assert(read_only == 0);
    assert(
        scid_database_metadata_get(persisted, "description", text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "") == 0);
    assert(text_size == 0);
    assert(scid_database_metadata_count_get(persisted, &count) == SCID_OK);
    assert(count == 9);
    assert(
        scid_database_metadata_at_get(
            persisted, 1, key, sizeof(key), &key_size, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(key, "description") == 0);
    assert(key_size == strlen("description"));
    assert(strcmp(text, "") == 0);
    assert(text_size == 0);
    assert(
        scid_database_metadata_set(persisted, "description", "C ABI persistent database") ==
        SCID_OK);
    assert(
        scid_database_metadata_get(persisted, "description", text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "C ABI persistent database") == 0);
    assert(
        scid_database_metadata_at_get(
            persisted, 1, key, sizeof(key), &key_size, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(key, "description") == 0);
    assert(strcmp(text, "C ABI persistent database") == 0);
    assert(
        scid_database_metadata_get(persisted, "missing", text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "") == 0);
    assert(text_size == 0);
    assert(
        scid_database_metadata_get(persisted, "description", NULL, 0, &text_size) ==
        SCID_ERROR_BUFFER_FULL);
    assert(text_size == strlen("C ABI persistent database"));
    assert(
        scid_database_metadata_at_get(
            persisted, 1, NULL, 0, &key_size, text, sizeof(text), &text_size) ==
        SCID_ERROR_BUFFER_FULL);
    assert(key_size == strlen("description"));
    assert(
        scid_database_metadata_at_get(
            persisted, 1, key, sizeof(key), &key_size, NULL, 0, &text_size) ==
        SCID_ERROR_BUFFER_FULL);
    assert(text_size == strlen("C ABI persistent database"));
    assert(
        scid_database_metadata_at_get(
            persisted, 99, key, sizeof(key), &key_size, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(scid_database_game_add(persisted, replacement, "P") == SCID_OK);
    assert(scid_database_game_count_get(persisted, &count) == SCID_OK);
    assert(count == 1);
    assert(scid_database_save(persisted) == SCID_OK);
    assert(scid_database_close(persisted) == SCID_OK);
    assert(scid_database_is_open(persisted, &is_open) == SCID_OK);
    assert(is_open == 0);
    assert(scid_database_save(persisted) == SCID_ERROR_BAD_ARG);
    assert(scid_database_filter_create(persisted, &filter_id) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_filter_game_count_get(persisted, SCID_FILTER_ALL_GAMES, &count) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_stats_date_range_get(
            persisted, text, sizeof(text), &text_size, max_date, sizeof(max_date),
            &max_date_size) == SCID_ERROR_BAD_ARG);
    assert(scid_database_stats_result_count_get(persisted, "0-1", &count) == SCID_ERROR_BAD_ARG);
    assert(scid_database_metadata_count_get(persisted, &count) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_metadata_at_get(
            persisted, 0, key, sizeof(key), &key_size, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_metadata_get(persisted, "description", text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(scid_database_metadata_set(persisted, "description", "closed") == SCID_ERROR_BAD_ARG);
    assert(scid_database_filename_get(persisted, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "<empty>") == 0);
    assert(scid_database_type_get(persisted, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "") == 0);
    assert(scid_database_close(persisted) == SCID_OK);
    scid_database_free(persisted);
    persisted = NULL;

    assert(scid_database_open_scid5(persisted_path, &reopened) == SCID_OK);
    assert(reopened != NULL);
    assert(scid_database_filename_get(reopened, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "_libscid_c_test_scid5.si5") == 0);
    assert(scid_database_type_get(reopened, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "scid5") == 0);
    assert(scid_database_read_only_get(reopened, &read_only) == SCID_OK);
    assert(read_only == 0);
    assert(scid_database_save(reopened) == SCID_OK);
    assert(
        scid_database_metadata_get(reopened, "description", text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "C ABI persistent database") == 0);
    assert(
        scid_database_metadata_at_get(
            reopened, 1, key, sizeof(key), &key_size, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(key, "description") == 0);
    assert(strcmp(text, "C ABI persistent database") == 0);
    assert(
        scid_database_stats_date_range_get(
            reopened, text, sizeof(text), &text_size, max_date, sizeof(max_date), &max_date_size) ==
        SCID_OK);
    assert(strcmp(text, "2025.01.02") == 0);
    assert(strcmp(max_date, "2025.01.02") == 0);
    assert(scid_database_stats_result_count_get(reopened, "0-1", &count) == SCID_OK);
    assert(count == 1);
    assert(scid_database_stats_result_count_get(reopened, "1-0", &count) == SCID_OK);
    assert(count == 0);
    assert(scid_database_game_count_get(reopened, &count) == SCID_OK);
    assert(count == 1);
    assert(
        scid_database_game_tag_get(reopened, 0, "Event", text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "Replacement") == 0);
    assert(scid_database_game_result_get(reopened, 0, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "0-1") == 0);
    assert(
        scid_database_game_get(reopened, 0, &loaded, flags, sizeof(flags), &flags_size) == SCID_OK);
    assert(strcmp(flags, "P") == 0);
    scid_game_free(loaded);
    loaded = NULL;
    assert(scid_database_close(reopened) == SCID_OK);
    scid_database_free(reopened);
    reopened = NULL;

    assert(scid_database_open_scid5_read_only(persisted_path, &read_only_database) == SCID_OK);
    assert(read_only_database != NULL);
    assert(scid_database_read_only_get(read_only_database, &read_only) == SCID_OK);
    assert(read_only == 1);
    assert(scid_database_save(read_only_database) == SCID_ERROR_FILE_READ_ONLY);
    assert(
        scid_database_metadata_get(
            read_only_database, "description", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "C ABI persistent database") == 0);
    assert(scid_database_metadata_count_get(read_only_database, &count) == SCID_OK);
    assert(count == 9);
    assert(
        scid_database_metadata_at_get(
            read_only_database, 1, key, sizeof(key), &key_size, text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(key, "description") == 0);
    assert(strcmp(text, "C ABI persistent database") == 0);
    assert(
        scid_database_stats_date_range_get(
            read_only_database, text, sizeof(text), &text_size, max_date, sizeof(max_date),
            &max_date_size) == SCID_OK);
    assert(strcmp(text, "2025.01.02") == 0);
    assert(strcmp(max_date, "2025.01.02") == 0);
    assert(scid_database_stats_result_count_get(read_only_database, "0-1", &count) == SCID_OK);
    assert(count == 1);
    assert(
        scid_database_metadata_set(read_only_database, "description", "readonly") ==
        SCID_ERROR_FILE_READ_ONLY);
    assert(scid_database_game_count_get(read_only_database, &count) == SCID_OK);
    assert(count == 1);
    assert(
        scid_database_game_tag_get(
            read_only_database, 0, "Event", text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Replacement") == 0);
    assert(
        scid_database_game_export_pgn(read_only_database, 0, text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strstr(text, "[Event \"Replacement\"]") != NULL);
    assert(
        scid_database_game_add(read_only_database, replacement, NULL) == SCID_ERROR_FILE_READ_ONLY);
    assert(
        scid_database_game_replace(read_only_database, 0, replacement, NULL) ==
        SCID_ERROR_FILE_READ_ONLY);
    assert(scid_database_game_delete(read_only_database, 0) == SCID_ERROR_FILE_READ_ONLY);
    assert(scid_database_game_undelete(read_only_database, 0) == SCID_ERROR_FILE_READ_ONLY);
    assert(
        scid_database_import_pgn(
            read_only_database, imported_pgn, strlen(imported_pgn), diagnostic, sizeof(diagnostic),
            &diagnostic_size, &imported_count) == SCID_ERROR_FILE_READ_ONLY);
    assert(scid_database_close(read_only_database) == SCID_OK);
    scid_database_free(read_only_database);
    read_only_database = NULL;
    remove_scid5_database(persisted_path);

    assert(scid_database_create_memory(NULL, &database) == SCID_ERROR_BAD_ARG);
    assert(scid_database_create_memory("bad", NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_create_scid5(NULL, &persisted) == SCID_ERROR_BAD_ARG);
    assert(scid_database_create_scid5("bad", NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_open_scid5(NULL, &reopened) == SCID_ERROR_BAD_ARG);
    assert(scid_database_open_scid5("bad", NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_open_scid5(missing_path, &reopened) == SCID_ERROR_FILE_OPEN);
    assert(reopened == NULL);
    assert(scid_database_open_scid5_read_only(NULL, &read_only_database) == SCID_ERROR_BAD_ARG);
    assert(scid_database_open_scid5_read_only("bad", NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_open_scid5_read_only(missing_path, &read_only_database) ==
        SCID_ERROR_FILE_OPEN);
    assert(read_only_database == NULL);

    remove_file(pgn_path);
    write_pgn_database(pgn_path, 2);
    assert(
        scid_database_open_pgn_read_only(
            pgn_path, progress_report, &progress, NULL, NULL, &pgn_database) == SCID_OK);
    assert(pgn_database != NULL);
    assert(progress.calls > 0);
    assert(progress.last_done == 1);
    assert(progress.last_total == 1);
    assert(progress.saw_message == 1);
    assert(scid_database_type_get(pgn_database, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "PGN") == 0);
    assert(scid_database_read_only_get(pgn_database, &read_only) == SCID_OK);
    assert(read_only == 1);
    assert(scid_database_game_count_get(pgn_database, &count) == SCID_OK);
    assert(count == 2);
    assert(
        scid_database_game_tag_get(pgn_database, 1, "Event", text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "PGN 2") == 0);
    assert(scid_database_close(pgn_database) == SCID_OK);
    scid_database_free(pgn_database);
    pgn_database = NULL;

    remove_file(large_pgn_path);
    write_pgn_database(large_pgn_path, 1030);
    assert(
        scid_database_open_pgn_read_only(
            large_pgn_path, NULL, NULL, should_cancel, &cancel, &pgn_database) ==
        SCID_ERROR_USER_CANCEL);
    assert(pgn_database == NULL);
    assert(cancel.calls > 0);

    assert(
        scid_database_open_pgn_read_only(NULL, NULL, NULL, NULL, NULL, &pgn_database) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_open_pgn_read_only(pgn_path, NULL, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);

    assert(scid_database_filter_create(NULL, &filter_id) == SCID_ERROR_BAD_ARG);
    assert(scid_database_filter_create(database, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_filter_delete(NULL, 999) == SCID_ERROR_BAD_ARG);
    assert(scid_database_filter_delete(database, 999) == SCID_ERROR_BAD_ARG);
    assert(scid_database_filter_delete(database, SCID_FILTER_ALL_GAMES) == SCID_ERROR_BAD_ARG);
    assert(scid_database_filter_delete(database, SCID_FILTER_PRIMARY) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_filter_game_count_get(NULL, SCID_FILTER_ALL_GAMES, &count) ==
        SCID_ERROR_BAD_ARG);
    assert(scid_database_filter_game_count_get(database, 999, &count) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_filter_game_count_get(database, SCID_FILTER_ALL_GAMES, NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_filter_game_indices_get(
            NULL, SCID_FILTER_ALL_GAMES, "d+", 0, 1, game_indexes, 4, &list_count) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_filter_game_indices_get(
            database, 999, "d+", 0, 1, game_indexes, 4, &list_count) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_filter_game_indices_get(
            database, SCID_FILTER_ALL_GAMES, NULL, 0, 1, game_indexes, 4, &list_count) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_filter_game_indices_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, 1, NULL, 0, &list_count) ==
        SCID_ERROR_BUFFER_FULL);
    assert(list_count == 1);
    assert(
        scid_database_filter_game_indices_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, 1, game_indexes, 4, NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_filter_game_index_at_row_get(NULL, SCID_FILTER_ALL_GAMES, "d+", 0, &count) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_filter_game_index_at_row_get(database, 999, "d+", 0, &count) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_filter_game_index_at_row_get(
            database, SCID_FILTER_ALL_GAMES, NULL, 0, &count) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_filter_game_index_at_row_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 99, &count) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_filter_game_index_at_row_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_filter_game_row_for_index_get(
            NULL, SCID_FILTER_ALL_GAMES, "d+", 0, &sorted_position) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_filter_game_row_for_index_get(database, 999, "d+", 0, &sorted_position) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_filter_game_row_for_index_get(
            database, SCID_FILTER_ALL_GAMES, NULL, 0, &sorted_position) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_filter_game_row_for_index_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 99, &sorted_position) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_filter_game_row_for_index_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, NULL) == SCID_ERROR_BAD_ARG);
    memset(&header_search, 0, sizeof(header_search));
    assert(
        scid_database_search_headers(
            NULL, SCID_FILTER_ALL_GAMES, filter_id, &header_search, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_search_headers(
            database, 999, filter_id, &header_search, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, 999, &header_search, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, SCID_FILTER_ALL_GAMES, &header_search, NULL, NULL,
            NULL, NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, filter_id, NULL, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(scid_database_close(NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_save(NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_metadata_get(NULL, "description", text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_metadata_get(database, NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_metadata_get(database, "description", text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(scid_database_metadata_count_get(NULL, &count) == SCID_ERROR_BAD_ARG);
    assert(scid_database_metadata_count_get(database, NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_metadata_at_get(
            NULL, 0, key, sizeof(key), &key_size, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_metadata_at_get(
            database, 0, key, sizeof(key), NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_metadata_at_get(
            database, 0, key, sizeof(key), &key_size, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_stats_date_range_get(
            NULL, text, sizeof(text), &text_size, max_date, sizeof(max_date), &max_date_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_stats_date_range_get(
            database, text, sizeof(text), NULL, max_date, sizeof(max_date), &max_date_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_stats_date_range_get(
            database, text, sizeof(text), &text_size, max_date, sizeof(max_date), NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_stats_date_range_get(
            database, NULL, 0, &text_size, max_date, sizeof(max_date), &max_date_size) ==
        SCID_ERROR_BUFFER_FULL);
    assert(text_size == strlen("2024.06.14"));
    assert(scid_database_stats_result_count_get(NULL, "1-0", &count) == SCID_ERROR_BAD_ARG);
    assert(scid_database_stats_result_count_get(database, NULL, &count) == SCID_ERROR_BAD_ARG);
    assert(scid_database_stats_result_count_get(database, "bad", &count) == SCID_ERROR_BAD_ARG);
    assert(scid_database_stats_result_count_get(database, "1-0", NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_metadata_set(NULL, "description", "bad") == SCID_ERROR_BAD_ARG);
    assert(scid_database_metadata_set(database, NULL, "bad") == SCID_ERROR_BAD_ARG);
    assert(scid_database_metadata_set(database, "description", NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_is_open(NULL, &is_open) == SCID_ERROR_BAD_ARG);
    assert(scid_database_is_open(database, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_filename_get(NULL, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    assert(scid_database_filename_get(database, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_filename_get(database, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    assert(text_size == strlen("<clipbase>"));
    assert(scid_database_type_get(NULL, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    assert(scid_database_type_get(database, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_type_get(database, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    assert(text_size == strlen("memory"));
    assert(scid_database_read_only_get(NULL, &read_only) == SCID_ERROR_BAD_ARG);
    assert(scid_database_read_only_get(database, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_count_get(NULL, &count) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_count_get(database, NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_import_pgn(
            NULL, imported_pgn, strlen(imported_pgn), diagnostic, sizeof(diagnostic),
            &diagnostic_size, &imported_count) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_import_pgn(
            database, NULL, 0, diagnostic, sizeof(diagnostic), &diagnostic_size, &imported_count) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_import_pgn(
            database, imported_pgn, strlen(imported_pgn), diagnostic, sizeof(diagnostic),
            &diagnostic_size, NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_import_pgn(
            database, imported_pgn, strlen(imported_pgn), NULL, 0, NULL, &imported_count) ==
        SCID_OK);
    assert(imported_count == 2);
    assert(
        scid_database_game_export_pgn(NULL, 0, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_game_export_pgn(database, 99, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_game_export_pgn(database, 0, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_add(NULL, game, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_add(database, NULL, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_replace(NULL, 0, replacement, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_replace(database, 99, replacement, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_replace(database, 0, NULL, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_delete(NULL, 0) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_delete(database, 99) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_undelete(NULL, 0) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_undelete(database, 99) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_game_tag_get(NULL, 0, "Event", text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_game_tag_get(database, 0, NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_game_tag_get(database, 99, "Event", text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(scid_database_game_halfmove_count_get(NULL, 0, &count) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_halfmove_count_get(database, 99, &count) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_halfmove_count_get(database, 0, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_number_get(NULL, 0, &count) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_number_get(database, 99, &count) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_number_get(database, 0, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_deleted_get(NULL, 0, &deleted) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_deleted_get(database, 99, &deleted) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_deleted_get(database, 0, NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_game_result_get(NULL, 0, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_game_result_get(database, 99, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_game_result_get(database, 0, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_eco_get(NULL, 0, &eco_code) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_eco_get(database, 99, &eco_code) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_eco_get(database, 0, NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_game_date_get(NULL, 0, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_game_date_get(database, 99, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_game_date_get(database, 0, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_get(NULL, 0, &loaded, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_database_game_get(database, 0, NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

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
