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
    const char* pgn = "[Event \"Match\"]\n"
                      "[White \"Kasparov, Garry\"]\n"
                      "[Black \"Deep Blue\"]\n"
                      "[Result \"1-0\"]\n\n"
                      "1. e4 c5 1-0\n\n"
                      "[Event \"World Ch\"]\n"
                      "[White \"Karpov, Anatoly\"]\n"
                      "[Black \"Kasparov, Garry\"]\n"
                      "[Result \"1/2-1/2\"]\n\n"
                      "1. d4 Nf6 1/2-1/2\n\n"
                      "[Event \"Tournament\"]\n"
                      "[White \"Kasparov, Garry\"]\n"
                      "[Black \"Anand, Viswanathan\"]\n"
                      "[Result \"1-0\"]\n\n"
                      "1. e4 e5 1-0\n";

    scid_database*              database = NULL;
    scid_filter_id              filter_id = 0;
    scid_search_header_criteria criteria = {0};
    char                        diagnostic[1024];
    size_t                      diagnostic_size = 0;
    size_t                      imported_count = 0;
    size_t                      matched_count = 0;
    size_t                      game_indices[2] = {0};
    size_t                      listed_count = 0;
    size_t                      row_index = 0;
    size_t                      translated_game_index = 0;

    if (!check(
            scid_database_create_memory("search-headers-demo", &database),
            "scid_database_create_memory") ||
        !check(
            scid_database_import_pgn(
                database, pgn, strlen(pgn), diagnostic, sizeof(diagnostic), &diagnostic_size,
                &imported_count),
            "scid_database_import_pgn") ||
        imported_count != 3)
    {
        fprintf(stderr, "Failed to import PGN dataset: %.*s\n", (int)diagnostic_size, diagnostic);
        scid_database_free(database);
        return 1;
    }

    if (!check(scid_database_filter_create(database, &filter_id), "scid_database_filter_create"))
    {
        scid_database_free(database);
        return 1;
    }

    criteria.white = "Kasparov";

    if (!check(
            scid_database_search_headers(
                database, SCID_FILTER_ALL_GAMES, filter_id, &criteria, NULL, NULL, NULL, NULL),
            "scid_database_search_headers") ||
        !check(
            scid_database_filter_game_count_get(database, filter_id, &matched_count),
            "scid_database_filter_game_count_get") ||
        matched_count != 2 ||
        !check(
            scid_database_filter_game_indices_get(
                database, filter_id, "d+", 0, 2, game_indices, 2, &listed_count),
            "scid_database_filter_game_indices_get") ||
        listed_count != 2 || game_indices[0] != 0 || game_indices[1] != 2 ||
        !check(
            scid_database_filter_game_row_for_index_get(
                database, filter_id, "d+", game_indices[1], &row_index),
            "scid_database_filter_game_row_for_index_get") ||
        row_index != 1 ||
        !check(
            scid_database_filter_game_index_at_row_get(
                database, filter_id, "d+", 1, &translated_game_index),
            "scid_database_filter_game_index_at_row_get") ||
        translated_game_index != 2)
    {
        scid_database_filter_delete(database, filter_id);
        scid_database_free(database);
        return 1;
    }

    printf("imported games: %zu\n", imported_count);
    printf("matched games for White 'Kasparov': %zu\n", matched_count);
    printf("matched game indices: %zu, %zu\n", game_indices[0], game_indices[1]);
    printf("row for game index 2: %zu\n", row_index);

    scid_database_filter_delete(database, filter_id);
    scid_database_free(database);
    return 0;
}
