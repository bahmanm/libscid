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
text_equals(
    const char* text,
    size_t text_size,
    const char* expected)
{
    return text_size == strlen(expected) && strncmp(text, expected, text_size) == 0;
}

static int
parse_pgn(
    const char* pgn,
    scid_game** out_game)
{
    char diagnostic[1024];
    size_t diagnostic_size = 0;

    if (!check(
            scid_game_create_from_pgn(
                pgn, strlen(pgn), out_game, diagnostic, sizeof(diagnostic), &diagnostic_size),
            "scid_game_create_from_pgn"))
    {
        fprintf(stderr, "%.*s\n", (int)diagnostic_size, diagnostic);
        return 0;
    }

    return 1;
}

int
main(
    void)
{
    const char* original_pgn = "[Event \"Original\"]\n"
                               "[Date \"2024.06.14\"]\n"
                               "[White \"Alpha\"]\n"
                               "[Black \"Beta\"]\n"
                               "[Result \"1-0\"]\n"
                               "\n"
                               "1. e4 e5 2. Nf3 1-0\n";
    const char* replacement_pgn = "[Event \"Replacement\"]\n"
                                  "[Date \"2025.01.02\"]\n"
                                  "[White \"Gamma\"]\n"
                                  "[Black \"Delta\"]\n"
                                  "[Result \"0-1\"]\n"
                                  "\n"
                                  "1. d4 d5 0-1\n";
    scid_database* database = NULL;
    scid_game* original = NULL;
    scid_game* replacement = NULL;
    scid_game* loaded = NULL;
    char flags[22];
    char event[128];
    char result[16];
    size_t count = 0;
    size_t event_size = 0;
    size_t flags_size = 0;
    size_t result_size = 0;
    int deleted = 0;

    if (!check(scid_database_create_memory("editing", &database), "scid_database_create_memory") ||
        !parse_pgn(original_pgn, &original) || !parse_pgn(replacement_pgn, &replacement) ||
        !check(scid_database_game_add(database, original, NULL), "scid_database_game_add") ||
        !check(scid_database_game_count_get(database, &count), "scid_database_game_count_get") ||
        count != 1)
    {
        scid_game_free(loaded);
        scid_game_free(replacement);
        scid_game_free(original);
        scid_database_free(database);
        return 1;
    }

    if (!check(
            scid_database_game_replace(database, 0, replacement, "Q"),
            "scid_database_game_replace") ||
        !check(scid_database_game_count_get(database, &count), "scid_database_game_count_get") ||
        count != 1 ||
        !check(
            scid_database_game_tag_get(database, 0, "Event", event, sizeof(event), &event_size),
            "scid_database_game_tag_get") ||
        !text_equals(event, event_size, "Replacement") ||
        !check(
            scid_database_game_result_get(database, 0, result, sizeof(result), &result_size),
            "scid_database_game_result_get") ||
        !text_equals(result, result_size, "0-1") ||
        !check(
            scid_database_game_get(database, 0, &loaded, flags, sizeof(flags), &flags_size),
            "scid_database_game_get") ||
        !text_equals(flags, flags_size, "Q"))
    {
        scid_game_free(loaded);
        scid_game_free(replacement);
        scid_game_free(original);
        scid_database_free(database);
        return 1;
    }

    printf(
        "after replace: %.*s %.*s flags=%.*s\n", (int)event_size, event, (int)result_size, result,
        (int)flags_size, flags);

    scid_game_free(loaded);
    loaded = NULL;

    if (!check(scid_database_game_delete(database, 0), "scid_database_game_delete") ||
        !check(
            scid_database_game_deleted_get(database, 0, &deleted),
            "scid_database_game_deleted_get") ||
        deleted != 1 ||
        !check(scid_database_game_undelete(database, 0), "scid_database_game_undelete") ||
        !check(
            scid_database_game_deleted_get(database, 0, &deleted),
            "scid_database_game_deleted_get") ||
        deleted != 0)
    {
        scid_game_free(replacement);
        scid_game_free(original);
        scid_database_free(database);
        return 1;
    }

    printf("after undelete: deleted=%d\n", deleted);

    scid_game_free(replacement);
    scid_game_free(original);
    scid_database_free(database);
    return 0;
}
