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
    const char* pgn = "[Event \"Ruy Lopez Main\"]\n"
                      "[White \"Kasparov, Garry\"]\n"
                      "[Black \"Anand, Viswanathan\"]\n"
                      "[Result \"1-0\"]\n\n"
                      "1. e4 e5 2. Nf3 Nc6 3. Bb5 a6 4. Ba4 Nf6 1-0\n\n"
                      "[Event \"Italian Game\"]\n"
                      "[White \"Kasparov, Garry\"]\n"
                      "[Black \"Karpov, Anatoly\"]\n"
                      "[Result \"1/2-1/2\"]\n\n"
                      "1. e4 e5 2. Nf3 Nc6 3. Bc4 Bc5 1/2-1/2\n\n"
                      "[Event \"French Defense\"]\n"
                      "[White \"Short, Nigel\"]\n"
                      "[Black \"Anand, Viswanathan\"]\n"
                      "[Result \"0-1\"]\n\n"
                      "1. e4 e6 2. d4 d5 0-1\n";

    /* Target position after 1. e4 e5 2. Nf3 Nc6 3. Bb5 */
    const char* ruy_lopez_fen = "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3";

    scid_database*               database = NULL;
    scid_position*               target_position = NULL;
    scid_filter_id               kasparov_filter = 0;
    scid_filter_id               ruy_lopez_filter = 0;
    scid_search_header_criteria* header_criteria = NULL;
    char                         diagnostic[1024];
    size_t                       diagnostic_size = 0;
    size_t                       imported_count = 0;
    size_t                       kasparov_count = 0;
    size_t                       ruy_lopez_count = 0;
    size_t                       matched_indices[1] = {0};
    size_t                       listed_count = 0;

    if (!check(
            scid_database_create_memory("search-position-demo", &database),
            "scid_database_create_memory") ||
        !check(
            scid_database_import_pgn(
                database, pgn, strlen(pgn), diagnostic, sizeof(diagnostic), &diagnostic_size,
                &imported_count),
            "scid_database_import_pgn") ||
        imported_count != 3 ||
        !check(
            scid_position_create_from_fen(ruy_lopez_fen, &target_position),
            "scid_position_create_from_fen"))
    {
        fprintf(stderr, "Initialization failed: %.*s\n", (int)diagnostic_size, diagnostic);
        scid_position_free(target_position);
        scid_database_free(database);
        return 1;
    }

    /* Stage 1: Create filter for Kasparov games via Header Search */
    if (!check(
            scid_database_filter_create(database, &kasparov_filter),
            "scid_database_filter_create") ||
        !check(
            scid_database_filter_create(database, &ruy_lopez_filter),
            "scid_database_filter_create"))
    {
        scid_position_free(target_position);
        scid_database_free(database);
        return 1;
    }

    if (!check(
            scid_search_header_criteria_create(&header_criteria),
            "scid_search_header_criteria_create") ||
        !check(
            scid_search_header_criteria_white_set(header_criteria, "Kasparov"),
            "scid_search_header_criteria_white_set") ||
        !check(
            scid_database_search_headers(
                database, SCID_FILTER_ALL_GAMES, kasparov_filter, header_criteria, NULL, NULL, NULL,
                NULL),
            "scid_database_search_headers") ||
        !check(
            scid_database_filter_game_count_get(database, kasparov_filter, &kasparov_count),
            "scid_database_filter_game_count_get") ||
        kasparov_count != 2)
    {
        scid_search_header_criteria_free(header_criteria);
        scid_database_filter_delete(database, ruy_lopez_filter);
        scid_database_filter_delete(database, kasparov_filter);
        scid_position_free(target_position);
        scid_database_free(database);
        return 1;
    }

    /* Stage 2: Filter chaining — search target position ONLY within kasparov_filter */
    if (!check(
            scid_database_search_position(
                database, kasparov_filter, ruy_lopez_filter, target_position, NULL, NULL, NULL,
                NULL),
            "scid_database_search_position") ||
        !check(
            scid_database_filter_game_count_get(database, ruy_lopez_filter, &ruy_lopez_count),
            "scid_database_filter_game_count_get") ||
        ruy_lopez_count != 1 ||
        !check(
            scid_database_filter_game_indices_get(
                database, ruy_lopez_filter, "d+", 0, 1, matched_indices, 1, &listed_count),
            "scid_database_filter_game_indices_get") ||
        listed_count != 1 || matched_indices[0] != 0)
    {
        scid_search_header_criteria_free(header_criteria);
        scid_database_filter_delete(database, ruy_lopez_filter);
        scid_database_filter_delete(database, kasparov_filter);
        scid_position_free(target_position);
        scid_database_free(database);
        return 1;
    }

    printf("imported games: %zu\n", imported_count);
    printf("Kasparov games (stage 1 filter): %zu\n", kasparov_count);
    printf("Kasparov Ruy Lopez games (stage 2 chained filter): %zu\n", ruy_lopez_count);
    printf("matched game index: %zu\n", matched_indices[0]);

    scid_search_header_criteria_free(header_criteria);
    scid_database_filter_delete(database, ruy_lopez_filter);
    scid_database_filter_delete(database, kasparov_filter);
    scid_position_free(target_position);
    scid_database_free(database);
    return 0;
}
