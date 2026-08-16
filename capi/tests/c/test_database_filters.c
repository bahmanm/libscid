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
    add_game(
        database, "[Event \"Filter 2\"]\n"
                  "[Site \"Vancouver\"]\n"
                  "[Date \"2025.01.02\"]\n"
                  "[Round \"1\"]\n"
                  "[White \"Gamma\"]\n"
                  "[Black \"Delta\"]\n"
                  "[Result \"0-1\"]\n"
                  "\n"
                  "1. d4 d5 0-1\n");
    add_game(
        database, "[Event \"Filter 3\"]\n"
                  "[Site \"Montreal\"]\n"
                  "[Date \"2026.02.03\"]\n"
                  "[Round \"1\"]\n"
                  "[White \"Epsilon\"]\n"
                  "[Black \"Zeta\"]\n"
                  "[Result \"1/2-1/2\"]\n"
                  "\n"
                  "1. c4 c5 1/2-1/2\n");
    add_game(
        database, "[Event \"Filter 4\"]\n"
                  "[Site \"Calgary\"]\n"
                  "[Date \"2026.02.04\"]\n"
                  "[Round \"1\"]\n"
                  "[White \"Eta\"]\n"
                  "[Black \"Theta\"]\n"
                  "[Result \"0-1\"]\n"
                  "\n"
                  "1. d4 Nf6 0-1\n");
    return database;
}


void
test_database_filters(void)
{
    scid_database* database = create_filter_database();
    scid_filter_id filter_id = 0;
    scid_filter_id filter_id_two = 0;
    size_t         count = 99;
    size_t         game_indexes[4] = {99, 99, 99, 99};
    size_t         list_count = 99;
    size_t         sorted_position = 99;

    TEST_ASSERT(scid_database_filter_create(database, &filter_id) == SCID_OK);
    TEST_ASSERT(filter_id > 0);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    TEST_ASSERT(count == 4);
    TEST_ASSERT(
        scid_database_filter_game_count_get(database, SCID_FILTER_ALL_GAMES, &count) == SCID_OK);
    TEST_ASSERT(count == 4);
    TEST_ASSERT(
        scid_database_filter_game_count_get(database, SCID_FILTER_PRIMARY, &count) == SCID_OK);
    TEST_ASSERT(count == 4);

    TEST_ASSERT(scid_database_filter_create(database, &filter_id_two) == SCID_OK);
    TEST_ASSERT(filter_id_two > 0);
    TEST_ASSERT(filter_id != filter_id_two);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id_two, &count) == SCID_OK);
    TEST_ASSERT(count == 4);

    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, 4, game_indexes, 4, &list_count) == SCID_OK);
    TEST_ASSERT(list_count == 4);
    TEST_ASSERT(game_indexes[0] == 0);
    TEST_ASSERT(game_indexes[1] == 1);
    TEST_ASSERT(game_indexes[2] == 2);
    TEST_ASSERT(game_indexes[3] == 3);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 1, 2, game_indexes, 4, &list_count) == SCID_OK);
    TEST_ASSERT(list_count == 2);
    TEST_ASSERT(game_indexes[0] == 1);
    TEST_ASSERT(game_indexes[1] == 2);
    TEST_ASSERT(
        scid_database_filter_game_index_at_row_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 2, &count) == SCID_OK);
    TEST_ASSERT(count == 2);
    TEST_ASSERT(
        scid_database_filter_game_row_for_index_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 2, &sorted_position) == SCID_OK);
    TEST_ASSERT(sorted_position == 2);

    TEST_ASSERT(scid_database_filter_delete(database, SCID_FILTER_ALL_GAMES) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filter_delete(database, SCID_FILTER_PRIMARY) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filter_delete(database, filter_id) == SCID_OK);
    TEST_ASSERT(scid_database_filter_delete(database, filter_id_two) == SCID_OK);

    TEST_ASSERT(scid_database_filter_create(NULL, &filter_id) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filter_create(database, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filter_delete(NULL, 999) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filter_delete(database, 999) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filter_delete(database, SCID_FILTER_ALL_GAMES) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filter_delete(database, SCID_FILTER_PRIMARY) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_count_get(NULL, SCID_FILTER_ALL_GAMES, &count) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_database_filter_game_count_get(database, 999, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_count_get(database, SCID_FILTER_ALL_GAMES, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            NULL, SCID_FILTER_ALL_GAMES, "d+", 0, 1, game_indexes, 4, &list_count) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, 999, "d+", 0, 1, game_indexes, 4, &list_count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, SCID_FILTER_ALL_GAMES, NULL, 0, 1, game_indexes, 4, &list_count) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, 1, NULL, 0, &list_count) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(list_count == 1);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, 1, game_indexes, 4, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_index_at_row_get(NULL, SCID_FILTER_ALL_GAMES, "d+", 0, &count) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_index_at_row_get(database, 999, "d+", 0, &count) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_index_at_row_get(
            database, SCID_FILTER_ALL_GAMES, NULL, 0, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_index_at_row_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 99, &count) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_index_at_row_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_row_for_index_get(
            NULL, SCID_FILTER_ALL_GAMES, "d+", 0, &sorted_position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_row_for_index_get(database, 999, "d+", 0, &sorted_position) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_row_for_index_get(
            database, SCID_FILTER_ALL_GAMES, NULL, 0, &sorted_position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_row_for_index_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 99, &sorted_position) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_filter_game_row_for_index_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_database_close(database) == SCID_OK);
    scid_database_free(database);
}
