#include "test_libscid.h"

#include "scid/scid.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

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
add_game(
    scid_database* database,
    const char*    pgn)
{
    scid_game* game = NULL;

    assert(test_game_create(pgn, strlen(pgn), &game, NULL, 0, NULL) == SCID_OK);
    assert(game != NULL);
    assert(scid_database_game_add(database, game, NULL) == SCID_OK);
    scid_game_free(game);
}


static scid_database*
create_search_database(void)
{
    scid_database* database = NULL;

    assert(scid_database_create_memory("search-tests", &database) == SCID_OK);
    assert(database != NULL);
    add_game(
        database, "[Event \"Stored\"]\n"
                  "[Site \"Toronto\"]\n"
                  "[Date \"2024.06.14\"]\n"
                  "[Round \"7\"]\n"
                  "[White \"Alpha\"]\n"
                  "[Black \"Beta\"]\n"
                  "[Result \"1-0\"]\n"
                  "[ECO \"C20\"]\n"
                  "[EventDate \"2024.06.01\"]\n"
                  "\n"
                  "1. e4 e5 2. Nf3 1-0\n");
    add_game(
        database, "[Event \"Replacement\"]\n"
                  "[Site \"Vancouver\"]\n"
                  "[Date \"2025.01.02\"]\n"
                  "[Round \"1\"]\n"
                  "[White \"Gamma\"]\n"
                  "[Black \"Delta\"]\n"
                  "[Result \"0-1\"]\n"
                  "\n"
                  "1. d4 d5 0-1\n");
    add_game(
        database, "[Event \"Imported\"]\n"
                  "[Site \"Montreal\"]\n"
                  "[Date \"2026.02.03\"]\n"
                  "[White \"Epsilon\"]\n"
                  "[Black \"Zeta\"]\n"
                  "[Result \"1/2-1/2\"]\n"
                  "\n"
                  "1. c4 c5 1/2-1/2\n");
    add_game(
        database, "[Event \"Imported Two\"]\n"
                  "[Site \"Calgary\"]\n"
                  "[Date \"2026.02.04\"]\n"
                  "[White \"Eta\"]\n"
                  "[Black \"Theta\"]\n"
                  "[Result \"0-1\"]\n"
                  "\n"
                  "1. d4 Nf6 0-1\n");
    add_game(
        database, "[Event \"Variation\"]\n"
                  "[Site \"Victoria\"]\n"
                  "[Date \"2027.01.01\"]\n"
                  "[White \"Iota\"]\n"
                  "[Black \"Kappa\"]\n"
                  "[Result \"*\"]\n"
                  "\n"
                  "1. a3 (1. h4 h5) a6 *\n");
    return database;
}


void
test_database_search(void)
{
    scid_database*              database = create_search_database();
    scid_filter_id              filter_id = 0;
    scid_filter_id              filter_id_two = 0;
    scid_position*              search_position = NULL;
    scid_position*              board_position = NULL;
    scid_position*              board_files_position = NULL;
    scid_position*              board_flipped_position = NULL;
    scid_position*              board_variation_position = NULL;
    scid_search_header_criteria header_search = {0};
    scid_search_board_criteria  board_search = {0};
    size_t                      count = 99;
    size_t                      game_indexes[5] = {99, 99, 99, 99, 99};
    size_t                      list_count = 99;
    struct progress_report_data progress = {0, 0, 0, 0};
    struct should_cancel_data   cancel = {0, 1};

    assert(scid_database_filter_create(database, &filter_id) == SCID_OK);
    assert(scid_database_filter_create(database, &filter_id_two) == SCID_OK);
    assert(filter_id != filter_id_two);

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
        scid_position_create_from_fen(
            "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2", &search_position) ==
        SCID_OK);
    assert(search_position != NULL);
    assert(
        scid_database_search_position(
            database, SCID_FILTER_ALL_GAMES, filter_id, search_position, progress_report, &progress,
            NULL, NULL) == SCID_OK);
    assert(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    assert(count == 1);
    assert(
        scid_database_filter_game_indices_get(
            database, filter_id, "N+", 0, 1, game_indexes, 4, &list_count) == SCID_OK);
    assert(list_count == 1);
    assert(game_indexes[0] == 1);

    assert(
        scid_database_search_position(
            database, filter_id_two, filter_id, search_position, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    assert(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    assert(count == 1);
    assert(
        scid_database_filter_game_indices_get(
            database, filter_id, "N+", 0, 1, game_indexes, 4, &list_count) == SCID_OK);
    assert(list_count == 1);
    assert(game_indexes[0] == 1);

    assert(
        scid_database_search_position(
            database, SCID_FILTER_PRIMARY, filter_id, search_position, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    assert(
        scid_database_search_position(
            database, filter_id, filter_id, search_position, NULL, NULL, NULL, NULL) == SCID_OK);
    assert(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    assert(count == 1);

    cancel.calls = 0;
    cancel.cancel_after_calls = 1;
    assert(
        scid_database_search_position(
            database, SCID_FILTER_ALL_GAMES, filter_id, search_position, NULL, NULL, should_cancel,
            &cancel) == SCID_ERROR_USER_CANCEL);
    assert(cancel.calls > 0);

    assert(
        scid_position_create_from_fen(
            "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2", &board_position) ==
        SCID_OK);
    assert(board_position != NULL);

    board_search.position = board_position;
    board_search.match = SCID_BOARD_SEARCH_MATCH_EXACT;
    assert(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, &board_search, progress_report, &progress,
            NULL, NULL) == SCID_OK);
    assert(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    assert(count == 1);
    assert(
        scid_database_filter_game_indices_get(
            database, filter_id, "N+", 0, 1, game_indexes, 4, &list_count) == SCID_OK);
    assert(list_count == 1);
    assert(game_indexes[0] == 1);

    assert(
        scid_database_search_board(
            database, filter_id_two, filter_id, &board_search, NULL, NULL, NULL, NULL) == SCID_OK);
    assert(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    assert(count == 1);
    assert(
        scid_database_filter_game_indices_get(
            database, filter_id, "N+", 0, 1, game_indexes, 4, &list_count) == SCID_OK);
    assert(list_count == 1);
    assert(game_indexes[0] == 1);

    board_search.match = SCID_BOARD_SEARCH_MATCH_PAWNS;
    assert(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, &board_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    assert(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    assert(count == 1);

    assert(
        scid_position_create_from_fen(
            "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1", &board_files_position) ==
        SCID_OK);
    assert(board_files_position != NULL);
    board_search.position = board_files_position;
    board_search.match = SCID_BOARD_SEARCH_MATCH_FILES;
    assert(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, &board_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    assert(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    assert(count == 5);
    assert(
        scid_database_filter_game_indices_get(
            database, filter_id, "N+", 0, 5, game_indexes, 5, &list_count) == SCID_OK);
    assert(list_count == 5);
    assert(game_indexes[0] == 0);
    assert(game_indexes[1] == 1);
    assert(game_indexes[2] == 2);
    assert(game_indexes[3] == 3);
    assert(game_indexes[4] == 4);

    assert(
        scid_position_create_from_fen(
            "rnbqkbnr/pppp1ppp/8/4p3/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            &board_flipped_position) == SCID_OK);
    assert(board_flipped_position != NULL);
    board_search.position = board_flipped_position;
    board_search.match = SCID_BOARD_SEARCH_MATCH_EXACT;
    board_search.include_flipped = 0;
    assert(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, &board_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    assert(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    assert(count == 0);
    board_search.include_flipped = 1;
    assert(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, &board_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    assert(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    assert(count == 1);
    board_search.include_flipped = 0;

    assert(
        scid_position_create_from_fen(
            "rnbqkbnr/ppppppp1/8/7p/7P/8/PPPPPPP1/RNBQKBNR w KQkq - 0 2",
            &board_variation_position) == SCID_OK);
    assert(board_variation_position != NULL);
    board_search.position = board_variation_position;
    board_search.match = SCID_BOARD_SEARCH_MATCH_EXACT;
    board_search.include_variations = 0;
    assert(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, &board_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    assert(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    assert(count == 0);
    board_search.include_variations = 1;
    assert(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, &board_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    assert(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    assert(count == 1);
    board_search.include_variations = 0;

    board_search.match = 999;
    assert(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, &board_search, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);

    board_search.position = board_position;
    board_search.match = SCID_BOARD_SEARCH_MATCH_EXACT;
    cancel.calls = 0;
    cancel.cancel_after_calls = 1;
    assert(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, &board_search, NULL, NULL, should_cancel,
            &cancel) == SCID_ERROR_USER_CANCEL);
    assert(cancel.calls > 0);

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
    assert(
        scid_database_search_position(
            NULL, SCID_FILTER_ALL_GAMES, filter_id, search_position, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_search_position(
            database, 999, filter_id, search_position, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_search_position(
            database, SCID_FILTER_ALL_GAMES, 999, search_position, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_search_position(
            database, SCID_FILTER_ALL_GAMES, SCID_FILTER_ALL_GAMES, search_position, NULL, NULL,
            NULL, NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_search_position(
            database, SCID_FILTER_ALL_GAMES, filter_id, NULL, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_search_board(
            NULL, SCID_FILTER_ALL_GAMES, filter_id, &board_search, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_search_board(
            database, 999, filter_id, &board_search, NULL, NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, 999, &board_search, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, SCID_FILTER_ALL_GAMES, &board_search, NULL, NULL, NULL,
            NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, NULL, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    board_search.position = NULL;
    assert(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, &board_search, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);

    scid_position_free(search_position);
    scid_position_free(board_position);
    scid_position_free(board_files_position);
    scid_position_free(board_flipped_position);
    scid_position_free(board_variation_position);
    search_position = NULL;
    board_position = NULL;
    board_files_position = NULL;
    board_flipped_position = NULL;
    board_variation_position = NULL;
    assert(scid_database_filter_delete(database, filter_id) == SCID_OK);
    assert(scid_database_filter_delete(database, filter_id_two) == SCID_OK);
    assert(scid_database_close(database) == SCID_OK);
    scid_database_free(database);
}
