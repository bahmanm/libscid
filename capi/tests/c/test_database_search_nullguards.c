#include "test_libscid.h"

#include "scid/scid.h"

#include <stddef.h>
#include <string.h>

static void
test_search_header_criteria_lifecycle_nullguards(void)
{
    TEST_ASSERT(scid_search_header_criteria_create(NULL) == SCID_ERROR_BAD_ARG);
    scid_search_header_criteria_free(NULL);
}

static void
test_search_header_criteria_text_nullguards(void)
{
    scid_search_header_criteria* criteria = NULL;
    char                         text[128];
    size_t                       size = 0;

    TEST_ASSERT(scid_search_header_criteria_create(&criteria) == SCID_OK);
    TEST_ASSERT(criteria != NULL);

    TEST_ASSERT(scid_search_header_criteria_player_set(NULL, "a") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_player_get(NULL, text, sizeof(text), &size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_player_get(criteria, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_player_set(criteria, "Kasparov") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_player_get(criteria, NULL, 0, &size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(size == strlen("Kasparov"));

    TEST_ASSERT(scid_search_header_criteria_white_set(NULL, "a") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_white_get(NULL, text, sizeof(text), &size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_white_get(criteria, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_header_criteria_white_set(criteria, "Carlsen") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_white_get(criteria, NULL, 0, &size) ==
        SCID_ERROR_BUFFER_FULL);

    TEST_ASSERT(scid_search_header_criteria_black_set(NULL, "a") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_black_get(NULL, text, sizeof(text), &size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_black_get(criteria, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_header_criteria_black_set(criteria, "Nakamura") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_black_get(criteria, NULL, 0, &size) ==
        SCID_ERROR_BUFFER_FULL);

    TEST_ASSERT(scid_search_header_criteria_event_set(NULL, "a") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_event_get(NULL, text, sizeof(text), &size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_event_get(criteria, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_header_criteria_event_set(criteria, "Match") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_event_get(criteria, NULL, 0, &size) ==
        SCID_ERROR_BUFFER_FULL);

    TEST_ASSERT(scid_search_header_criteria_site_set(NULL, "a") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_site_get(NULL, text, sizeof(text), &size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_site_get(criteria, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_header_criteria_site_set(criteria, "London") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_site_get(criteria, NULL, 0, &size) ==
        SCID_ERROR_BUFFER_FULL);

    TEST_ASSERT(scid_search_header_criteria_site_country_set(NULL, "a") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_site_country_get(NULL, text, sizeof(text), &size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_site_country_get(criteria, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_header_criteria_site_country_set(criteria, "ENG") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_site_country_get(criteria, NULL, 0, &size) ==
        SCID_ERROR_BUFFER_FULL);

    TEST_ASSERT(scid_search_header_criteria_round_set(NULL, "a") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_round_get(NULL, text, sizeof(text), &size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_round_get(criteria, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_header_criteria_round_set(criteria, "1") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_round_get(criteria, NULL, 0, &size) ==
        SCID_ERROR_BUFFER_FULL);

    TEST_ASSERT(scid_search_header_criteria_result_set(NULL, "a") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_result_get(NULL, text, sizeof(text), &size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_result_get(criteria, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_header_criteria_result_set(criteria, "1-0") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_result_get(criteria, NULL, 0, &size) ==
        SCID_ERROR_BUFFER_FULL);

    scid_search_header_criteria_free(criteria);
}

static void
test_search_header_criteria_ranges_and_flags_nullguards(void)
{
    scid_search_header_criteria* criteria = NULL;
    char                         min_text[128];
    char                         max_text[128];
    size_t                       min_size = 0;
    size_t                       max_size = 0;
    size_t                       min_u = 0;
    size_t                       max_u = 0;
    int                          min_i = 0;
    int                          max_i = 0;
    int                          flag = 0;

    TEST_ASSERT(scid_search_header_criteria_create(&criteria) == SCID_OK);
    TEST_ASSERT(criteria != NULL);

    TEST_ASSERT(
        scid_search_header_criteria_date_range_set(NULL, "2020.01.01", "2020.12.31") ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_date_range_set(criteria, "2020.01.01", "2020.12.31") ==
        SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_date_range_get(
            NULL, min_text, sizeof(min_text), &min_size, max_text, sizeof(max_text), &max_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_date_range_get(
            criteria, NULL, 0, &min_size, max_text, sizeof(max_text), &max_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(
        scid_search_header_criteria_date_range_get(
            criteria, min_text, sizeof(min_text), NULL, max_text, sizeof(max_text), &max_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_date_range_get(
            criteria, min_text, sizeof(min_text), &min_size, NULL, 0, &max_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(
        scid_search_header_criteria_date_range_get(
            criteria, min_text, sizeof(min_text), &min_size, max_text, sizeof(max_text), NULL) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_search_header_criteria_event_date_range_set(NULL, "2020.01.01", "2020.12.31") ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_event_date_range_set(criteria, "2020.01.01", "2020.12.31") ==
        SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_event_date_range_get(
            NULL, min_text, sizeof(min_text), &min_size, max_text, sizeof(max_text), &max_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_event_date_range_get(
            criteria, NULL, 0, &min_size, max_text, sizeof(max_text), &max_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(
        scid_search_header_criteria_event_date_range_get(
            criteria, min_text, sizeof(min_text), NULL, max_text, sizeof(max_text), &max_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_event_date_range_get(
            criteria, min_text, sizeof(min_text), &min_size, NULL, 0, &max_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(
        scid_search_header_criteria_event_date_range_get(
            criteria, min_text, sizeof(min_text), &min_size, max_text, sizeof(max_text), NULL) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_search_header_criteria_eco_range_set(NULL, "A00", "E99") == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_header_criteria_eco_range_set(criteria, "A00", "E99") == SCID_OK);
    TEST_ASSERT(
        scid_search_header_criteria_eco_range_get(
            NULL, min_text, sizeof(min_text), &min_size, max_text, sizeof(max_text), &max_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_eco_range_get(
            criteria, NULL, 0, &min_size, max_text, sizeof(max_text), &max_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(
        scid_search_header_criteria_eco_range_get(
            criteria, min_text, sizeof(min_text), NULL, max_text, sizeof(max_text), &max_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_eco_range_get(
            criteria, min_text, sizeof(min_text), &min_size, NULL, 0, &max_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(
        scid_search_header_criteria_eco_range_get(
            criteria, min_text, sizeof(min_text), &min_size, max_text, sizeof(max_text), NULL) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_search_header_criteria_game_number_range_set(NULL, 1, 100) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_game_number_range_get(NULL, &min_u, &max_u) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_game_number_range_get(criteria, NULL, &max_u) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_game_number_range_get(criteria, &min_u, NULL) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_search_header_criteria_halfmove_count_range_set(NULL, 1, 100) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_halfmove_count_range_get(NULL, &min_u, &max_u) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_halfmove_count_range_get(criteria, NULL, &max_u) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_halfmove_count_range_get(criteria, &min_u, NULL) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_search_header_criteria_white_elo_range_set(NULL, 2000, 2800) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_white_elo_range_get(NULL, &min_u, &max_u) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_white_elo_range_get(criteria, NULL, &max_u) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_white_elo_range_get(criteria, &min_u, NULL) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_search_header_criteria_black_elo_range_set(NULL, 2000, 2800) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_black_elo_range_get(NULL, &min_u, &max_u) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_black_elo_range_get(criteria, NULL, &max_u) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_black_elo_range_get(criteria, &min_u, NULL) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_search_header_criteria_elo_difference_range_set(NULL, -200, 200) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_elo_difference_range_get(NULL, &min_i, &max_i) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_elo_difference_range_get(criteria, NULL, &max_i) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_header_criteria_elo_difference_range_get(criteria, &min_i, NULL) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_search_header_criteria_has_variations_set(NULL, 1) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_header_criteria_has_variations_get(NULL, &flag) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_header_criteria_has_variations_get(criteria, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_search_header_criteria_has_comments_set(NULL, 1) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_header_criteria_has_comments_get(NULL, &flag) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_header_criteria_has_comments_get(criteria, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_search_header_criteria_has_nags_set(NULL, 1) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_header_criteria_has_nags_get(NULL, &flag) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_header_criteria_has_nags_get(criteria, NULL) == SCID_ERROR_BAD_ARG);

    scid_search_header_criteria_free(criteria);
}

static void
test_search_board_criteria_nullguards(void)
{
    scid_search_board_criteria* criteria = NULL;
    scid_position*              pos = NULL;
    scid_position*              pos_out = NULL;
    scid_board_search_match     match = SCID_BOARD_SEARCH_MATCH_EXACT;
    int                         flag = 0;

    TEST_ASSERT(scid_search_board_criteria_create(NULL) == SCID_ERROR_BAD_ARG);
    scid_search_board_criteria_free(NULL);

    TEST_ASSERT(scid_search_board_criteria_create(&criteria) == SCID_OK);
    TEST_ASSERT(criteria != NULL);
    TEST_ASSERT(test_position_create_standard(&pos) == SCID_OK);
    TEST_ASSERT(test_position_create_empty(&pos_out) == SCID_OK);

    TEST_ASSERT(scid_search_board_criteria_position_set(NULL, pos) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_board_criteria_position_get(NULL, pos_out) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_board_criteria_position_get(criteria, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_board_criteria_position_get(criteria, pos_out) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_search_board_criteria_match_set(NULL, SCID_BOARD_SEARCH_MATCH_EXACT) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_board_criteria_match_set(criteria, 999) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_board_criteria_match_get(NULL, &match) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_board_criteria_match_get(criteria, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_search_board_criteria_include_variations_set(NULL, 1) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_board_criteria_include_variations_get(NULL, &flag) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_board_criteria_include_variations_get(criteria, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_search_board_criteria_include_flipped_set(NULL, 1) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_search_board_criteria_include_flipped_get(NULL, &flag) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_search_board_criteria_include_flipped_get(criteria, NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(pos);
    scid_position_free(pos_out);
    scid_search_board_criteria_free(criteria);
}

static void
test_database_search_execution_nullguards(void)
{
    scid_database*               database = NULL;
    scid_filter_id               filter_id = 0;
    scid_position*               search_position = NULL;
    scid_search_header_criteria* header_search = NULL;
    scid_search_board_criteria*  board_search = NULL;

    TEST_ASSERT(scid_database_create_memory("search-nullguards", &database) == SCID_OK);
    TEST_ASSERT(database != NULL);
    TEST_ASSERT(scid_database_filter_create(database, &filter_id) == SCID_OK);
    TEST_ASSERT(scid_search_header_criteria_create(&header_search) == SCID_OK);
    TEST_ASSERT(scid_search_board_criteria_create(&board_search) == SCID_OK);
    TEST_ASSERT(test_position_create_standard(&search_position) == SCID_OK);

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

    TEST_ASSERT(scid_search_header_criteria_result_set(header_search, "bad-result") == SCID_OK);
    TEST_ASSERT(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, filter_id, header_search, NULL, NULL, NULL, NULL) ==
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
    TEST_ASSERT(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, board_search, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_search_board_criteria_position_set(board_search, search_position) == SCID_OK);
    TEST_ASSERT(scid_database_close(database) == SCID_OK);

    TEST_ASSERT(
        scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, filter_id, header_search, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_search_position(
            database, SCID_FILTER_ALL_GAMES, filter_id, search_position, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, filter_id, board_search, NULL, NULL, NULL, NULL) ==
        SCID_ERROR_BAD_ARG);

    scid_search_header_criteria_free(header_search);
    scid_search_board_criteria_free(board_search);
    scid_position_free(search_position);
    scid_database_free(database);
}

void
test_database_search_nullguards(void)
{
    test_search_header_criteria_lifecycle_nullguards();
    test_search_header_criteria_text_nullguards();
    test_search_header_criteria_ranges_and_flags_nullguards();
    test_search_board_criteria_nullguards();
    test_database_search_execution_nullguards();
}
