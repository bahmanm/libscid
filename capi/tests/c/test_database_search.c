#include "test_libscid.h"

#include "scid/scid.h"

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

    TEST_ASSERT(test_game_create(pgn, strlen(pgn), &game, NULL, 0, NULL) == SCID_OK);
    TEST_ASSERT(game != NULL);
    TEST_ASSERT(scid_database_game_add(database, game, NULL) == SCID_OK);
    scid_game_free(game);
}


static scid_database*
create_search_database(void)
{
    scid_database* database = NULL;

    TEST_ASSERT(scid_database_create_memory("search-tests", &database) == SCID_OK);
    TEST_ASSERT(database != NULL);
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


static void
test_header_criteria_properties(void)
{
    scid_search_header_criteria* criteria = NULL;
    char                         text[128];
    char                         text_max[128];
    size_t                       text_size = 0;
    size_t                       text_max_size = 0;
    size_t                       min_u = 0;
    size_t                       max_u = 0;
    int                          min_i = 0;
    int                          max_i = 0;
    int                          flag = 0;

    TEST_ASSERT(scid_search_header_criteria_create(NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_header_criteria_create(&criteria) == SCID_OK);
    TEST_ASSERT(criteria != NULL);

    /* Text properties */
    TEST_ASSERT(scid_search_header_criteria_player_set(criteria, "Kasparov") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_player_get(criteria, text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "Kasparov") == 0);
    TEST_ASSERT(text_size == strlen("Kasparov"));

    TEST_ASSERT(scid_search_header_criteria_white_set(criteria, "Carlsen") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_white_get(criteria, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Carlsen") == 0);

    TEST_ASSERT(scid_search_header_criteria_black_set(criteria, "Nakamura") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_black_get(criteria, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Nakamura") == 0);

    TEST_ASSERT(scid_search_header_criteria_event_set(criteria, "Candidates") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_event_get(criteria, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Candidates") == 0);

    TEST_ASSERT(scid_search_header_criteria_site_set(criteria, "London") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_site_get(criteria, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "London") == 0);

    TEST_ASSERT(scid_search_header_criteria_site_country_set(criteria, "ENG") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_site_country_get(criteria, text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "ENG") == 0);

    TEST_ASSERT(scid_search_header_criteria_round_set(criteria, "5") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_round_get(criteria, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "5") == 0);

    TEST_ASSERT(scid_search_header_criteria_result_set(criteria, "1-0") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_result_get(criteria, text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "1-0") == 0);

    /* Text ranges */
    TEST_ASSERT(
        scid_search_header_criteria_date_range_set(criteria, "2020.01.01", "2020.12.31") ==
        SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_date_range_get(
            criteria, text, sizeof(text), &text_size, text_max, sizeof(text_max), &text_max_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "2020.01.01") == 0);
    TEST_ASSERT(strcmp(text_max, "2020.12.31") == 0);

    TEST_ASSERT(
        scid_search_header_criteria_event_date_range_set(criteria, "2020.01.01", "2020.01.10") ==
        SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_event_date_range_get(
            criteria, text, sizeof(text), &text_size, text_max, sizeof(text_max), &text_max_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "2020.01.01") == 0);
    TEST_ASSERT(strcmp(text_max, "2020.01.10") == 0);

    TEST_ASSERT(scid_search_header_criteria_eco_range_set(criteria, "B00", "B99") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_eco_range_get(
            criteria, text, sizeof(text), &text_size, text_max, sizeof(text_max), &text_max_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "B00") == 0);
    TEST_ASSERT(strcmp(text_max, "B99") == 0);

    /* Numeric ranges */
    TEST_ASSERT(scid_search_header_criteria_game_number_range_set(criteria, 10, 50) == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_game_number_range_get(criteria, &min_u, &max_u) == SCID_OK);
    TEST_ASSERT(min_u == 10 && max_u == 50);

    TEST_ASSERT(scid_search_header_criteria_halfmove_count_range_set(criteria, 20, 80) == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_halfmove_count_range_get(criteria, &min_u, &max_u) == SCID_OK);
    TEST_ASSERT(min_u == 20 && max_u == 80);

    TEST_ASSERT(scid_search_header_criteria_white_elo_range_set(criteria, 2600, 2800) == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_white_elo_range_get(criteria, &min_u, &max_u) == SCID_OK);
    TEST_ASSERT(min_u == 2600 && max_u == 2800);

    TEST_ASSERT(scid_search_header_criteria_black_elo_range_set(criteria, 2500, 2750) == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_black_elo_range_get(criteria, &min_u, &max_u) == SCID_OK);
    TEST_ASSERT(min_u == 2500 && max_u == 2750);

    TEST_ASSERT(
        scid_search_header_criteria_elo_difference_range_set(criteria, -100, 100) == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_elo_difference_range_get(criteria, &min_i, &max_i) == SCID_OK);
    TEST_ASSERT(min_i == -100 && max_i == 100);

    /* Flags */
    TEST_ASSERT(scid_search_header_criteria_has_variations_set(criteria, 1) == SCID_OK);
    TEST_ASSERT(scid_search_header_criteria_has_variations_get(criteria, &flag) == SCID_OK);
    TEST_ASSERT(flag == 1);

    TEST_ASSERT(scid_search_header_criteria_has_comments_set(criteria, 1) == SCID_OK);
    TEST_ASSERT(scid_search_header_criteria_has_comments_get(criteria, &flag) == SCID_OK);
    TEST_ASSERT(flag == 1);

    TEST_ASSERT(scid_search_header_criteria_has_nags_set(criteria, 1) == SCID_OK);
    TEST_ASSERT(scid_search_header_criteria_has_nags_get(criteria, &flag) == SCID_OK);
    TEST_ASSERT(flag == 1);

    /* Buffer full checks */
    TEST_ASSERT(
        scid_search_header_criteria_player_get(criteria, NULL, 0, &text_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == strlen("Kasparov"));

    /* Error handling */
    TEST_ASSERT(scid_search_header_criteria_player_set(NULL, "a") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_player_get(NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_player_get(criteria, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);

    scid_search_header_criteria_free(criteria);
    scid_search_header_criteria_free(NULL);
}


static void
test_board_criteria_properties(void)
{
    scid_search_board_criteria* criteria = NULL;
    scid_position*              pos = NULL;
    scid_position*              pos_out = NULL;
    scid_board_search_match     match = SCID_BOARD_SEARCH_MATCH_EXACT;
    int                         flag = 0;

    TEST_ASSERT(scid_search_board_criteria_create(NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_board_criteria_create(&criteria) == SCID_OK);
    TEST_ASSERT(criteria != NULL);
    TEST_ASSERT(test_position_create_standard(&pos) == SCID_OK);
    TEST_ASSERT(test_position_create_empty(&pos_out) == SCID_OK);

    /* Position */
    TEST_ASSERT(scid_search_board_criteria_position_get(criteria, pos_out) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_board_criteria_position_set(criteria, pos) == SCID_OK);
    TEST_ASSERT(scid_search_board_criteria_position_get(criteria, pos_out) == SCID_OK);

    /* Match */
    TEST_ASSERT(
        scid_search_board_criteria_match_set(criteria, SCID_BOARD_SEARCH_MATCH_PAWNS) == SCID_OK);
    TEST_ASSERT(scid_search_board_criteria_match_get(criteria, &match) == SCID_OK);
    TEST_ASSERT(match == SCID_BOARD_SEARCH_MATCH_PAWNS);
    TEST_ASSERT(scid_search_board_criteria_match_set(criteria, 999) == SCID_ERROR_BAD_ARG);

    /* Variations and flipped */
    TEST_ASSERT(scid_search_board_criteria_include_variations_set(criteria, 1) == SCID_OK);
    TEST_ASSERT(scid_search_board_criteria_include_variations_get(criteria, &flag) == SCID_OK);
    TEST_ASSERT(flag == 1);

    TEST_ASSERT(scid_search_board_criteria_include_flipped_set(criteria, 1) == SCID_OK);
    TEST_ASSERT(scid_search_board_criteria_include_flipped_get(criteria, &flag) == SCID_OK);
    TEST_ASSERT(flag == 1);

    /* Error handling */
    TEST_ASSERT(scid_search_board_criteria_position_set(NULL, pos) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_board_criteria_position_get(NULL, pos_out) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_board_criteria_position_get(criteria, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_board_criteria_match_set(NULL, SCID_BOARD_SEARCH_MATCH_EXACT) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_board_criteria_match_get(NULL, &match) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_board_criteria_match_get(criteria, NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(pos);
    scid_position_free(pos_out);
    scid_search_board_criteria_free(criteria);
    scid_search_board_criteria_free(NULL);
}


void
test_database_search(void)
{
    scid_database*               database = create_search_database();
    scid_filter_id               filter_id = 0;
    scid_filter_id               filter_id_two = 0;
    scid_position*               search_position = NULL;
    scid_position*               board_position = NULL;
    scid_position*               board_files_position = NULL;
    scid_position*               board_flipped_position = NULL;
    scid_position*               board_variation_position = NULL;
    scid_search_header_criteria* header_search = NULL;
    scid_search_board_criteria*  board_search = NULL;
    size_t                       count = 99;
    size_t                       game_indexes[5] = {99, 99, 99, 99, 99};
    size_t                       list_count = 99;
    struct progress_report_data  progress = {0, 0, 0, 0};
    struct should_cancel_data    cancel = {0, 1};

    test_header_criteria_properties();
    test_board_criteria_properties();

    TEST_ASSERT(scid_search_header_criteria_create(&header_search) == SCID_OK);
    TEST_ASSERT(scid_search_board_criteria_create(&board_search) == SCID_OK);

    TEST_ASSERT(scid_database_filter_create(database, &filter_id) == SCID_OK);
    TEST_ASSERT(scid_database_filter_create(database, &filter_id_two) == SCID_OK);
    TEST_ASSERT(filter_id != filter_id_two);

    TEST_ASSERT(scid_search_header_criteria_white_set(header_search, "Gamma") == SCID_OK);
    TEST_ASSERT(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, filter_id, header_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, filter_id, "d+", 0, 1, game_indexes, 4, &list_count) == SCID_OK);
    TEST_ASSERT(list_count == 1);
    TEST_ASSERT(game_indexes[0] == 1);

    scid_search_header_criteria_free(header_search);
    header_search = NULL;
    TEST_ASSERT(scid_search_header_criteria_create(&header_search) == SCID_OK);
    TEST_ASSERT(scid_search_header_criteria_result_set(header_search, "0-1") == SCID_OK);
    TEST_ASSERT(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, filter_id_two, header_search, progress_report,
            &progress, NULL, NULL) == SCID_OK);
    TEST_ASSERT(progress.calls > 0);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id_two, &count) == SCID_OK);
    TEST_ASSERT(count == 2);

    scid_search_header_criteria_free(header_search);
    header_search = NULL;
    TEST_ASSERT(scid_search_header_criteria_create(&header_search) == SCID_OK);
    TEST_ASSERT(scid_search_header_criteria_result_set(header_search, "1-0, 1/2-1/2") == SCID_OK);
    TEST_ASSERT(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, filter_id, header_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    TEST_ASSERT(count == 2);

    scid_search_header_criteria_free(header_search);
    header_search = NULL;
    TEST_ASSERT(scid_search_header_criteria_create(&header_search) == SCID_OK);
    TEST_ASSERT(scid_search_header_criteria_event_set(header_search, "Imported") == SCID_OK);
    TEST_ASSERT(
        scid_database_search_headers(
            database, filter_id_two, filter_id, header_search, NULL, NULL, NULL, NULL) == SCID_OK);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, filter_id, "d+", 0, 1, game_indexes, 4, &list_count) == SCID_OK);
    TEST_ASSERT(list_count == 1);
    TEST_ASSERT(game_indexes[0] == 3);

    scid_search_header_criteria_free(header_search);
    header_search = NULL;
    TEST_ASSERT(scid_search_header_criteria_create(&header_search) == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_date_range_set(header_search, "2026.02.01", "2026.02.04") ==
        SCID_OK);
    TEST_ASSERT(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, filter_id, header_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    TEST_ASSERT(count == 2);

    scid_search_header_criteria_free(header_search);
    header_search = NULL;
    TEST_ASSERT(scid_search_header_criteria_create(&header_search) == SCID_OK);
    TEST_ASSERT(scid_search_header_criteria_result_set(header_search, "bad-result") == SCID_OK);
    TEST_ASSERT(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, filter_id, header_search, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);

    scid_search_header_criteria_free(header_search);
    header_search = NULL;
    TEST_ASSERT(scid_search_header_criteria_create(&header_search) == SCID_OK);
    TEST_ASSERT(scid_search_header_criteria_event_set(header_search, "Imported") == SCID_OK);
    cancel.calls = 0;
    cancel.cancel_after_calls = 1;
    TEST_ASSERT(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, filter_id, header_search, NULL, NULL, should_cancel,
            &cancel) == SCID_ERROR_USER_CANCEL);
    TEST_ASSERT(cancel.calls > 0);

    TEST_ASSERT(
        scid_position_create_from_fen(
            "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2", &search_position) ==
        SCID_OK);
    TEST_ASSERT(search_position != NULL);
    TEST_ASSERT(
        scid_database_search_position(
            database, SCID_FILTER_ALL_GAMES, filter_id, search_position, progress_report, &progress,
            NULL, NULL) == SCID_OK);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, filter_id, "N+", 0, 1, game_indexes, 4, &list_count) == SCID_OK);
    TEST_ASSERT(list_count == 1);
    TEST_ASSERT(game_indexes[0] == 1);

    TEST_ASSERT(
        scid_database_search_position(
            database, filter_id_two, filter_id, search_position, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, filter_id, "N+", 0, 1, game_indexes, 4, &list_count) == SCID_OK);
    TEST_ASSERT(list_count == 1);
    TEST_ASSERT(game_indexes[0] == 1);

    TEST_ASSERT(
        scid_database_search_position(
            database, SCID_FILTER_PRIMARY, filter_id, search_position, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    TEST_ASSERT(
        scid_database_search_position(
            database, filter_id, filter_id, search_position, NULL, NULL, NULL, NULL) == SCID_OK);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    TEST_ASSERT(count == 1);

    cancel.calls = 0;
    cancel.cancel_after_calls = 1;
    TEST_ASSERT(
        scid_database_search_position(
            database, SCID_FILTER_ALL_GAMES, filter_id, search_position, NULL, NULL, should_cancel,
            &cancel) == SCID_ERROR_USER_CANCEL);
    TEST_ASSERT(cancel.calls > 0);

    TEST_ASSERT(
        scid_position_create_from_fen(
            "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2", &board_position) ==
        SCID_OK);
    TEST_ASSERT(board_position != NULL);

    TEST_ASSERT(scid_search_board_criteria_position_set(board_search, board_position) == SCID_OK);
    TEST_ASSERT(
        scid_search_board_criteria_match_set(board_search, SCID_BOARD_SEARCH_MATCH_EXACT) ==
        SCID_OK);
    TEST_ASSERT(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, board_search, progress_report, &progress,
            NULL, NULL) == SCID_OK);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, filter_id, "N+", 0, 1, game_indexes, 4, &list_count) == SCID_OK);
    TEST_ASSERT(list_count == 1);
    TEST_ASSERT(game_indexes[0] == 1);

    TEST_ASSERT(
        scid_database_search_board(
            database, filter_id_two, filter_id, board_search, NULL, NULL, NULL, NULL) == SCID_OK);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, filter_id, "N+", 0, 1, game_indexes, 4, &list_count) == SCID_OK);
    TEST_ASSERT(list_count == 1);
    TEST_ASSERT(game_indexes[0] == 1);

    TEST_ASSERT(
        scid_search_board_criteria_match_set(board_search, SCID_BOARD_SEARCH_MATCH_PAWNS) ==
        SCID_OK);
    TEST_ASSERT(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, board_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    TEST_ASSERT(count == 1);

    TEST_ASSERT(
        scid_position_create_from_fen(
            "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1", &board_files_position) ==
        SCID_OK);
    TEST_ASSERT(board_files_position != NULL);
    TEST_ASSERT(
        scid_search_board_criteria_position_set(board_search, board_files_position) == SCID_OK);
    TEST_ASSERT(
        scid_search_board_criteria_match_set(board_search, SCID_BOARD_SEARCH_MATCH_FILES) ==
        SCID_OK);
    TEST_ASSERT(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, board_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    TEST_ASSERT(count == 5);
    TEST_ASSERT(
        scid_database_filter_game_indices_get(
            database, filter_id, "N+", 0, 5, game_indexes, 5, &list_count) == SCID_OK);
    TEST_ASSERT(list_count == 5);
    TEST_ASSERT(game_indexes[0] == 0);
    TEST_ASSERT(game_indexes[1] == 1);
    TEST_ASSERT(game_indexes[2] == 2);
    TEST_ASSERT(game_indexes[3] == 3);
    TEST_ASSERT(game_indexes[4] == 4);

    TEST_ASSERT(
        scid_position_create_from_fen(
            "rnbqkbnr/pppp1ppp/8/4p3/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            &board_flipped_position) == SCID_OK);
    TEST_ASSERT(board_flipped_position != NULL);
    TEST_ASSERT(
        scid_search_board_criteria_position_set(board_search, board_flipped_position) == SCID_OK);
    TEST_ASSERT(
        scid_search_board_criteria_match_set(board_search, SCID_BOARD_SEARCH_MATCH_EXACT) ==
        SCID_OK);
    TEST_ASSERT(scid_search_board_criteria_include_flipped_set(board_search, 0) == SCID_OK);
    TEST_ASSERT(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, board_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    TEST_ASSERT(count == 0);
    TEST_ASSERT(scid_search_board_criteria_include_flipped_set(board_search, 1) == SCID_OK);
    TEST_ASSERT(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, board_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(scid_search_board_criteria_include_flipped_set(board_search, 0) == SCID_OK);

    TEST_ASSERT(
        scid_position_create_from_fen(
            "rnbqkbnr/ppppppp1/8/7p/7P/8/PPPPPPP1/RNBQKBNR w KQkq - 0 2",
            &board_variation_position) == SCID_OK);
    TEST_ASSERT(board_variation_position != NULL);
    TEST_ASSERT(
        scid_search_board_criteria_position_set(board_search, board_variation_position) == SCID_OK);
    TEST_ASSERT(
        scid_search_board_criteria_match_set(board_search, SCID_BOARD_SEARCH_MATCH_EXACT) ==
        SCID_OK);
    TEST_ASSERT(scid_search_board_criteria_include_variations_set(board_search, 0) == SCID_OK);
    TEST_ASSERT(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, board_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    TEST_ASSERT(count == 0);
    TEST_ASSERT(scid_search_board_criteria_include_variations_set(board_search, 1) == SCID_OK);
    TEST_ASSERT(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, board_search, NULL, NULL, NULL, NULL) ==
        SCID_OK);
    TEST_ASSERT(scid_database_filter_game_count_get(database, filter_id, &count) == SCID_OK);
    TEST_ASSERT(count == 1);
    TEST_ASSERT(scid_search_board_criteria_include_variations_set(board_search, 0) == SCID_OK);

    TEST_ASSERT(scid_search_board_criteria_position_set(board_search, board_position) == SCID_OK);
    TEST_ASSERT(
        scid_search_board_criteria_match_set(board_search, SCID_BOARD_SEARCH_MATCH_EXACT) ==
        SCID_OK);
    cancel.calls = 0;
    cancel.cancel_after_calls = 1;
    TEST_ASSERT(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, board_search, NULL, NULL, should_cancel,
            &cancel) == SCID_ERROR_USER_CANCEL);
    TEST_ASSERT(cancel.calls > 0);

    scid_search_header_criteria_free(header_search);
    header_search = NULL;
    TEST_ASSERT(scid_search_header_criteria_create(&header_search) == SCID_OK);
    TEST_ASSERT(
        scid_database_search_headers(
            NULL, SCID_FILTER_ALL_GAMES, filter_id, header_search, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_search_headers(
            database, 999, filter_id, header_search, NULL, NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, 999, header_search, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, SCID_FILTER_ALL_GAMES, header_search, NULL, NULL, NULL,
            NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, filter_id, NULL, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_search_position(
            NULL, SCID_FILTER_ALL_GAMES, filter_id, search_position, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_search_position(
            database, 999, filter_id, search_position, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_search_position(
            database, SCID_FILTER_ALL_GAMES, 999, search_position, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_search_position(
            database, SCID_FILTER_ALL_GAMES, SCID_FILTER_ALL_GAMES, search_position, NULL, NULL,
            NULL, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_search_position(
            database, SCID_FILTER_ALL_GAMES, filter_id, NULL, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_search_board(
            NULL, SCID_FILTER_ALL_GAMES, filter_id, board_search, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_search_board(
            database, 999, filter_id, board_search, NULL, NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, 999, board_search, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, SCID_FILTER_ALL_GAMES, board_search, NULL, NULL, NULL,
            NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, NULL, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    scid_search_board_criteria_free(board_search);
    board_search = NULL;
    TEST_ASSERT(scid_search_board_criteria_create(&board_search) == SCID_OK);
    TEST_ASSERT(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, board_search, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);

    scid_search_header_criteria_free(header_search);
    scid_search_board_criteria_free(board_search);
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
    TEST_ASSERT(scid_database_filter_delete(database, filter_id) == SCID_OK);
    TEST_ASSERT(scid_database_filter_delete(database, filter_id_two) == SCID_OK);
    TEST_ASSERT(scid_database_close(database) == SCID_OK);
    scid_database_free(database);
}
