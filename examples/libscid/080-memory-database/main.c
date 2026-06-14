#include <scid/scid.h>

#include <stdio.h>
#include <string.h>

static int check(scid_error error, const char* call) {
    if (error == SCID_OK) {
        return 1;
    }

    fprintf(stderr, "%s failed with scid_error %hu\n", call, error);
    return 0;
}

int main(void) {
    const char* pgn =
        "[Event \"Memory\"]\n"
        "[White \"Alpha\"]\n"
        "[Black \"Beta\"]\n"
        "[Result \"1-0\"]\n"
        "\n"
        "1. e4 e5 2. Nf3 1-0\n";
    scid_database* database = NULL;
    scid_game* game = NULL;
    char diagnostic[1024];
    size_t diagnostic_size = 0;
    size_t count = 0;
    int is_open = 0;

    if (!check(scid_database_create_memory("example", &database),
               "scid_database_create_memory") ||
        !check(scid_database_is_open(database, &is_open),
               "scid_database_is_open") ||
        !is_open ||
        !check(scid_database_game_count_get(database, &count),
               "scid_database_game_count_get") ||
        count != 0 ||
        !check(scid_game_create_from_pgn(
                   pgn,
                   strlen(pgn),
                   &game,
                   diagnostic,
                   sizeof(diagnostic),
                   &diagnostic_size),
               "scid_game_create_from_pgn")) {
        fprintf(stderr, "%.*s\n", (int)diagnostic_size, diagnostic);
        scid_game_free(game);
        scid_database_free(database);
        return 1;
    }

    printf("open: %d\n", is_open);
    printf("games before add: %zu\n", count);

    if (!check(scid_database_game_add(database, game, NULL),
               "scid_database_game_add") ||
        !check(scid_database_game_count_get(database, &count),
               "scid_database_game_count_get") ||
        count != 1) {
        scid_game_free(game);
        scid_database_free(database);
        return 1;
    }

    printf("games after first add: %zu\n", count);

    if (!check(scid_database_game_add(database, game, "M"),
               "scid_database_game_add") ||
        !check(scid_database_game_count_get(database, &count),
               "scid_database_game_count_get") ||
        count != 2) {
        scid_game_free(game);
        scid_database_free(database);
        return 1;
    }

    printf("games after second add: %zu\n", count);

    scid_game_free(game);
    scid_database_free(database);
    return 0;
}
