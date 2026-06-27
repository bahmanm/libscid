#include <scid/scid.h>

#include <stdio.h>
#include <string.h>

static int
check(
    scid_error error,
    const char* call)
{
    if (error == SCID_OK)
    {
        return 1;
    }

    fprintf(stderr, "%s failed with scid_error %hu\n", call, error);
    return 0;
}

static int
contains(
    const char* text,
    const char* expected)
{
    return strstr(text, expected) != NULL;
}

int
main(
    void)
{
    const char* pgn = "[Event \"Database PGN\"]\n"
                      "[Site \"Toronto\"]\n"
                      "[Date \"2026.06.14\"]\n"
                      "[White \"Alpha\"]\n"
                      "[Black \"Beta\"]\n"
                      "[Result \"1-0\"]\n"
                      "\n"
                      "1. e4 e5 2. Nf3 1-0\n"
                      "\n"
                      "[Event \"Database PGN Two\"]\n"
                      "[Site \"Vancouver\"]\n"
                      "[Date \"2026.06.15\"]\n"
                      "[White \"Gamma\"]\n"
                      "[Black \"Delta\"]\n"
                      "[Result \"0-1\"]\n"
                      "\n"
                      "1. d4 Nf6 0-1\n";
    scid_database* database = NULL;
    char diagnostic[1024];
    char exported[2048];
    char event[128];
    size_t count = 0;
    size_t diagnostic_size = 0;
    size_t exported_size = 0;
    size_t event_size = 0;
    size_t imported_count = 0;

    if (!check(scid_database_create_memory("pgn-io", &database), "scid_database_create_memory") ||
        !check(
            scid_database_import_pgn(
                database, pgn, strlen(pgn), diagnostic, sizeof(diagnostic), &diagnostic_size,
                &imported_count),
            "scid_database_import_pgn") ||
        diagnostic_size != 0 || imported_count != 2 ||
        !check(scid_database_game_count_get(database, &count), "scid_database_game_count_get") ||
        count != 2 ||
        !check(
            scid_database_game_tag_get(database, 0, "Event", event, sizeof(event), &event_size),
            "scid_database_game_tag_get") ||
        !check(
            scid_database_game_export_pgn(database, 0, exported, sizeof(exported), &exported_size),
            "scid_database_game_export_pgn") ||
        !contains(exported, "[Event \"Database PGN\"]") || !contains(exported, "1-0"))
    {
        fprintf(stderr, "%.*s\n", (int)diagnostic_size, diagnostic);
        scid_database_free(database);
        return 1;
    }

    printf("imported games: %zu\n", imported_count);
    printf("first event: %.*s\n", (int)event_size, event);
    printf("exported bytes: %zu\n", exported_size);

    scid_database_free(database);
    return 0;
}
