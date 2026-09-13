#include "test_libscid.h"

#include "scid/scid.h"

#include <stddef.h>
#include <string.h>

static void
test_database_lifecycle_nullguards(void)
{
    scid_database* database = NULL;
    int            is_open = 0;

    TEST_ASSERT(scid_database_create_memory(NULL, &database) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_create_memory("scratch", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_create_memory(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_create_scid5(NULL, &database) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_create_scid5("scratch", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_create_scid5(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_database_open_scid5(NULL, NULL, NULL, NULL, NULL, &database) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_open_scid5("scratch", NULL, NULL, NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_open_scid5(NULL, NULL, NULL, NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_database_open_scid5_read_only(NULL, NULL, NULL, NULL, NULL, &database) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_open_scid5_read_only("scratch", NULL, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_open_scid5_read_only(NULL, NULL, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_database_open_pgn_read_only(NULL, NULL, NULL, NULL, NULL, &database) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_open_pgn_read_only("scratch.pgn", NULL, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_open_pgn_read_only(NULL, NULL, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_close(NULL) == SCID_ERROR_BAD_ARG);
    scid_database_free(NULL);

    TEST_ASSERT(scid_database_is_open(NULL, &is_open) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_save(NULL) == SCID_ERROR_BAD_ARG);
}

static void
test_database_status_nullguards(void)
{
    scid_database* database = NULL;
    scid_error     status = SCID_ERROR;
    size_t         count = 0;
    int            is_open = 0;
    int            read_only = 0;
    int            is_dirty = 0;

    TEST_ASSERT(scid_database_create_memory("scratch", &database) == SCID_OK);
    TEST_ASSERT(database != NULL);

    TEST_ASSERT(scid_database_is_open(NULL, &is_open) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_is_open(database, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_is_open(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_status_open_get(NULL, &status) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_status_open_get(database, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_status_open_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_status_bad_name_count_get(NULL, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_status_bad_name_count_get(database, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_status_bad_name_count_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_status_is_read_only(NULL, &read_only) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_status_is_read_only(database, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_status_is_read_only(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_status_is_dirty(NULL, &is_dirty) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_status_is_dirty(database, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_status_is_dirty(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_close(database) == SCID_OK);
    TEST_ASSERT(scid_database_status_open_get(database, &status) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_status_bad_name_count_get(database, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_status_is_read_only(database, &read_only) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_status_is_dirty(database, &is_dirty) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_save(database) == SCID_ERROR_BAD_ARG);

    scid_database_free(database);
}

static void
test_database_properties_and_metadata_nullguards(void)
{
    scid_database* database = NULL;
    char           key[64];
    char           text[128];
    size_t         count = 0;
    size_t         key_size = 0;
    size_t         text_size = 0;
    int            read_only = 0;

    TEST_ASSERT(scid_database_create_memory("scratch", &database) == SCID_OK);
    TEST_ASSERT(database != NULL);

    TEST_ASSERT(
        scid_database_filename_get(NULL, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filename_get(database, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filename_get(NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filename_get(database, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);

    TEST_ASSERT(
        scid_database_type_get(NULL, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_type_get(database, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_type_get(NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_type_get(database, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);

    TEST_ASSERT(scid_database_read_only_get(NULL, &read_only) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_read_only_get(database, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_read_only_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_database_metadata_get(NULL, "type", text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_metadata_get(database, NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_metadata_get(database, "type", text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_metadata_get(NULL, NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_metadata_get(database, "type", NULL, 0, &text_size) ==
        SCID_ERROR_BUFFER_FULL);

    TEST_ASSERT(scid_database_metadata_set(NULL, "key", "value") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_metadata_set(database, NULL, "value") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_metadata_set(database, "key", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_metadata_set(NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_metadata_count_get(NULL, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_metadata_count_get(database, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_metadata_count_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

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
        scid_database_metadata_at_get(
            database, 9999, key, sizeof(key), &key_size, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_metadata_at_get(
            database, 0, NULL, 0, &key_size, text, sizeof(text), &text_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(
        scid_database_metadata_at_get(
            database, 0, key, sizeof(key), &key_size, NULL, 0, &text_size) ==
        SCID_ERROR_BUFFER_FULL);

    TEST_ASSERT(scid_database_close(database) == SCID_OK);
    TEST_ASSERT(
        scid_database_metadata_get(database, "type", text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_metadata_set(database, "key", "val") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_metadata_count_get(database, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_metadata_at_get(
            database, 0, key, sizeof(key), &key_size, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);

    scid_database_free(database);
}

static void
test_database_stats_nullguards(void)
{
    scid_database* database = NULL;
    char           max_date[32];
    char           min_date[32];
    size_t         count = 0;
    size_t         max_date_size = 0;
    size_t         min_date_size = 0;

    TEST_ASSERT(scid_database_create_memory("scratch", &database) == SCID_OK);
    TEST_ASSERT(database != NULL);

    TEST_ASSERT(
        scid_database_stats_date_range_get(
            NULL, min_date, sizeof(min_date), &min_date_size, max_date, sizeof(max_date),
            &max_date_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_stats_date_range_get(
            database, min_date, sizeof(min_date), NULL, max_date, sizeof(max_date),
            &max_date_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_stats_date_range_get(
            database, min_date, sizeof(min_date), &min_date_size, max_date, sizeof(max_date),
            NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_stats_date_range_get(NULL, NULL, 0, NULL, NULL, 0, NULL) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_database_stats_result_count_get(NULL, "1-0", &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_stats_result_count_get(database, NULL, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_stats_result_count_get(database, "1-0", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_stats_result_count_get(database, "bad_result", &count) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_stats_result_count_get(NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_game_count_get(NULL, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_count_get(database, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_count_get(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_close(database) == SCID_OK);
    TEST_ASSERT(
        scid_database_stats_date_range_get(
            database, min_date, sizeof(min_date), &min_date_size, max_date, sizeof(max_date),
            &max_date_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_stats_result_count_get(database, "1-0", &count) == SCID_ERROR_BAD_ARG);

    scid_database_free(database);
}

static void
test_database_game_crud_nullguards(void)
{
    scid_database* database = NULL;
    scid_game*     game = NULL;
    char           diagnostic[128];
    size_t         diagnostic_size = 0;
    size_t         imported_count = 0;

    TEST_ASSERT(scid_database_create_memory("scratch", &database) == SCID_OK);
    TEST_ASSERT(database != NULL);
    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(game != NULL);

    TEST_ASSERT(
        scid_database_import_pgn(
            NULL, "1. e4", 5, diagnostic, sizeof(diagnostic), &diagnostic_size,
            &imported_count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_import_pgn(
            database, NULL, 0, diagnostic, sizeof(diagnostic), &diagnostic_size,
            &imported_count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_import_pgn(
            database, "1. e4", 5, diagnostic, sizeof(diagnostic), &diagnostic_size, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_import_pgn(NULL, NULL, 0, NULL, 0, NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_game_add(NULL, game, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_add(database, NULL, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_add(NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_game_add(database, game, NULL) == SCID_OK);

    TEST_ASSERT(scid_database_game_replace(NULL, 0, game, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_replace(database, 0, NULL, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_replace(database, 9999, game, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_replace(NULL, 0, NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_game_delete(NULL, 0) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_delete(database, 9999) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_game_undelete(NULL, 0) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_undelete(database, 9999) == SCID_ERROR_BAD_ARG);

    scid_game_free(game);
    scid_database_free(database);
}

static void
test_database_game_queries_nullguards(void)
{
    scid_database* database = NULL;
    scid_game*     game = NULL;
    scid_game*     loaded = NULL;
    char           flags[32];
    char           text[128];
    size_t         count = 0;
    size_t         flags_size = 0;
    size_t         text_size = 0;
    scid_eco_code  eco_code = 0;
    int            deleted = 0;

    TEST_ASSERT(scid_database_create_memory("scratch", &database) == SCID_OK);
    TEST_ASSERT(database != NULL);
    TEST_ASSERT(test_game_create_blank(&game) == SCID_OK);
    TEST_ASSERT(game != NULL);
    TEST_ASSERT(scid_database_game_add(database, game, "W") == SCID_OK);

    TEST_ASSERT(
        scid_database_game_get(NULL, 0, &loaded, flags, sizeof(flags), &flags_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_get(database, 0, NULL, flags, sizeof(flags), &flags_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_get(database, 9999, &loaded, flags, sizeof(flags), &flags_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_get(NULL, 0, NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_database_game_export_pgn(NULL, 0, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_export_pgn(database, 9999, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_export_pgn(database, 0, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_export_pgn(NULL, 0, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_export_pgn(database, 0, NULL, 0, &text_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(
        scid_database_game_export_pgn(database, 0, text, 1, &text_size) ==
        SCID_ERROR_BUFFER_FULL);

    TEST_ASSERT(
        scid_database_game_tag_get(NULL, 0, "Event", text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 0, NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 9999, "Event", text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_tag_get(database, 0, "Event", text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_tag_get(NULL, 0, NULL, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_database_game_halfmove_count_get(NULL, 0, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_halfmove_count_get(database, 9999, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_halfmove_count_get(database, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_halfmove_count_get(NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_game_number_get(NULL, 0, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_number_get(database, 9999, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_number_get(database, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_number_get(NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_game_deleted_get(NULL, 0, &deleted) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_deleted_get(database, 9999, &deleted) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_deleted_get(database, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_deleted_get(NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_database_game_result_get(NULL, 0, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_result_get(database, 9999, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_result_get(database, 0, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_result_get(NULL, 0, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_result_get(database, 0, NULL, 0, &text_size) ==
        SCID_ERROR_BUFFER_FULL);

    TEST_ASSERT(scid_database_game_eco_get(NULL, 0, &eco_code) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_eco_get(database, 9999, &eco_code) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_eco_get(database, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_game_eco_get(NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_database_game_date_get(NULL, 0, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_date_get(database, 9999, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_date_get(database, 0, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_date_get(NULL, 0, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_game_date_get(database, 0, NULL, 0, &text_size) ==
        SCID_ERROR_BUFFER_FULL);

    scid_game_free(game);
    scid_database_free(database);
}

void
test_database_nullguards(void)
{
    test_database_lifecycle_nullguards();
    test_database_status_nullguards();
    test_database_properties_and_metadata_nullguards();
    test_database_stats_nullguards();
    test_database_game_crud_nullguards();
    test_database_game_queries_nullguards();
}
