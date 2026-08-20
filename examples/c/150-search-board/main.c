#include <scid/scid.h>

#include <stdio.h>
#include <string.h>

static int
check(
    scid_error  error,
    const char* call)
{
    if (error == SCID_OK)
    {
        return 1;
    }

    fprintf(stderr, "%s failed with scid_error %hu\n", call, error);
    return 0;
}


int
main(void)
{
    const char* pgn = "[Event \"French Defense\"]\n"
                      "[White \"Short, Nigel\"]\n"
                      "[Black \"Anand, Viswanathan\"]\n"
                      "[Result \"0-1\"]\n\n"
                      "1. e4 e6 2. d4 d5 0-1\n\n"
                      "[Event \"Italian Game\"]\n"
                      "[White \"Kasparov, Garry\"]\n"
                      "[Black \"Karpov, Anatoly\"]\n"
                      "[Result \"1/2-1/2\"]\n\n"
                      "1. e4 e5 2. Nf3 Nc6 3. Bc4 Bc5 1/2-1/2\n\n"
                      "[Event \"French Defense Two\"]\n"
                      "[White \"Leko, Peter\"]\n"
                      "[Black \"Bareev, Evgeny\"]\n"
                      "[Result \"1-0\"]\n\n"
                      "1. e4 e6 2. d4 d5 3. Nc3 Nf6 1-0\n";

    /* Target position after 1. e4 e6 2. d4 d5 (French pawn structure) */
    const char* french_fen = "rnbqkbnr/ppp2ppp/4p3/3p4/3PP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 3";

    scid_database*              database = NULL;
    scid_position*              french_position = NULL;
    scid_filter_id              filter_id = 0;
    scid_search_board_criteria* criteria = NULL;
    char                        diagnostic[1024];
    size_t                      diagnostic_size = 0;
    size_t                      imported_count = 0;
    size_t                      matched_count = 0;
    size_t                      matched_indices[2] = {0};
    size_t                      listed_count = 0;

    if (!check(
            scid_database_create_memory("search-board-demo", &database),
            "scid_database_create_memory") ||
        !check(
            scid_database_import_pgn(
                database, pgn, strlen(pgn), diagnostic, sizeof(diagnostic), &diagnostic_size,
                &imported_count),
            "scid_database_import_pgn") ||
        imported_count != 3 ||
        !check(
            scid_position_create_from_fen(french_fen, &french_position),
            "scid_position_create_from_fen"))
    {
        fprintf(stderr, "Initialization failed: %.*s\n", (int)diagnostic_size, diagnostic);
        scid_position_free(french_position);
        scid_database_free(database);
        return 1;
    }

    if (!check(scid_database_filter_create(database, &filter_id), "scid_database_filter_create") ||
        !check(scid_search_board_criteria_create(&criteria), "scid_search_board_criteria_create") ||
        !check(
            scid_search_board_criteria_position_set(criteria, french_position),
            "scid_search_board_criteria_position_set") ||
        !check(
            scid_search_board_criteria_match_set(criteria, SCID_BOARD_SEARCH_MATCH_PAWNS),
            "scid_search_board_criteria_match_set") ||
        !check(
            scid_search_board_criteria_include_variations_set(criteria, 0),
            "scid_search_board_criteria_include_variations_set") ||
        !check(
            scid_search_board_criteria_include_flipped_set(criteria, 0),
            "scid_search_board_criteria_include_flipped_set"))
    {
        scid_search_board_criteria_free(criteria);
        scid_position_free(french_position);
        scid_database_free(database);
        return 1;
    }

    if (!check(
            scid_database_search_board(
                database, SCID_FILTER_ALL_GAMES, filter_id, criteria, NULL, NULL, NULL, NULL),
            "scid_database_search_board") ||
        !check(
            scid_database_filter_game_count_get(database, filter_id, &matched_count),
            "scid_database_filter_game_count_get") ||
        matched_count != 2 ||
        !check(
            scid_database_filter_game_indices_get(
                database, filter_id, "d+", 0, 2, matched_indices, 2, &listed_count),
            "scid_database_filter_game_indices_get") ||
        listed_count != 2 || matched_indices[0] != 0 || matched_indices[1] != 2)
    {
        scid_search_board_criteria_free(criteria);
        scid_database_filter_delete(database, filter_id);
        scid_position_free(french_position);
        scid_database_free(database);
        return 1;
    }

    printf("imported games: %zu\n", imported_count);
    printf("matched games for French Pawn Structure: %zu\n", matched_count);
    printf("matched game indices: %zu, %zu\n", matched_indices[0], matched_indices[1]);

    scid_search_board_criteria_free(criteria);
    scid_database_filter_delete(database, filter_id);
    scid_position_free(french_position);
    scid_database_free(database);
    return 0;
}
