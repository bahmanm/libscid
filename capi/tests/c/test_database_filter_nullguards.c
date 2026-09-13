#include "test_libscid.h"

#include "scid/scid.h"

#include <stddef.h>
#include <string.h>

static void
add_game(
    scid_database* database,
    const char*    pgn)
{
    scid_game* game = NULL;

    TEST_ASSERT(test_game_create(pgn, strlen(pgn), &game, NULL, 0, NULL) == SCID_OK);
    TEST_ASSERT(game != NULL);
    TEST_ASSERT(scid_database_game_add(database, game, NULL) == SCID_OK);
    scid_game_free(game);
}

static scid_database*
create_filter_database(void)
{
    scid_database* database = NULL;

    TEST_ASSERT(scid_database_create_memory("filter-tests", &database) == SCID_OK);
    TEST_ASSERT(database != NULL);
    add_game(
        database, "[Event \"Filter 1\"]\n"
                  "[Site \"Toronto\"]\n"
                  "[Date \"2024.06.14\"]\n"
                  "[Round \"1\"]\n"
                  "[White \"Alpha\"]\n"
                  "[Black \"Beta\"]\n"
                  "[Result \"1-0\"]\n"
                  "\n"
                  "1. e4 e5 2. Nf3 1-0\n");
    return database;
}

static void
test_database_filter_lifecycle_nullguards(void)
{
    scid_database* database = create_filter_database();
    scid_filter_id filter_id = 0;

    TEST_ASSERT(scid_database_filter_create(NULL, &filter_id) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filter_create(database, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filter_create(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_filter_delete(NULL, 1) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filter_delete(database, 0) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filter_delete(database, SCID_FILTER_ALL_GAMES) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filter_delete(database, SCID_FILTER_PRIMARY) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filter_delete(database, 9999) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_close(database) == SCID_OK);
    TEST_ASSERT(scid_database_filter_create(database, &filter_id) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filter_delete(database, 1) == SCID_ERROR_BAD_ARG);

    scid_database_free(database);
}

static void
test_database_filter_query_nullguards(void)
{
    scid_database* database = create_filter_database();
    size_t         game_indexes[4] = {0, 0, 0, 0};
    size_t         count = 0;
    size_t         list_count = 0;
    size_t         sorted_position = 0;

    TEST_ASSERT(
        scid_database_filter_game_count_get(NULL, SCID_FILTER_ALL_GAMES, &count) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_count_get(database, 9999, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_count_get(database, SCID_FILTER_ALL_GAMES, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_count_get(NULL, SCID_FILTER_ALL_GAMES, NULL) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            NULL, SCID_FILTER_ALL_GAMES, "d+", 0, 1, game_indexes, 4, &list_count) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, 9999, "d+", 0, 1, game_indexes, 4, &list_count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, SCID_FILTER_ALL_GAMES, NULL, 0, 1, game_indexes, 4, &list_count) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, 1, NULL, 0, &list_count) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, 1, game_indexes, 4, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            NULL, SCID_FILTER_ALL_GAMES, NULL, 0, 1, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_database_filter_game_index_at_row_get(NULL, SCID_FILTER_ALL_GAMES, "d+", 0, &count) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_index_at_row_get(database, 9999, "d+", 0, &count) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_index_at_row_get(
            database, SCID_FILTER_ALL_GAMES, NULL, 0, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_index_at_row_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 9999, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_index_at_row_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_index_at_row_get(NULL, SCID_FILTER_ALL_GAMES, NULL, 0, NULL) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_database_filter_game_row_for_index_get(
            NULL, SCID_FILTER_ALL_GAMES, "d+", 0, &sorted_position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_row_for_index_get(
            database, 9999, "d+", 0, &sorted_position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_row_for_index_get(
            database, SCID_FILTER_ALL_GAMES, NULL, 0, &sorted_position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_row_for_index_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 9999, &sorted_position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_row_for_index_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_row_for_index_get(
            NULL, SCID_FILTER_ALL_GAMES, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_close(database) == SCID_OK);
    TEST_ASSERT(
        scid_database_filter_game_count_get(database, SCID_FILTER_ALL_GAMES, &count) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, 1, game_indexes, 4, &list_count) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_index_at_row_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_row_for_index_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, &sorted_position) == SCID_ERROR_BAD_ARG);

    scid_database_free(database);
}

void
test_database_filter_nullguards(void)
{
    test_database_filter_lifecycle_nullguards();
    test_database_filter_query_nullguards();
}
