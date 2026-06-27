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

int
main(
    void)
{
    const char* pgn = "[Event \"Memory\"]\n"
                      "[White \"Alpha\"]\n"
                      "[Black \"Beta\"]\n"
                      "[Result \"1-0\"]\n"
                      "\n"
                      "1. e4 e5 2. Nf3 1-0\n";
    scid_database* database = NULL;
    scid_game* game = NULL;
    scid_game* loaded = NULL;
    char diagnostic[1024];
    char filter_id[16] = {0};
    char flags[22];
    char text[128];
    size_t diagnostic_size = 0;
    size_t count = 0;
    size_t filter_id_size = 0;
    size_t game_indexes[1] = {0};
    size_t flags_size = 0;
    size_t listed_count = 0;
    size_t sorted_position = 0;
    size_t text_size = 0;
    int is_open = 0;

    if (!check(scid_database_create_memory("example", &database), "scid_database_create_memory") ||
        !check(scid_database_is_open(database, &is_open), "scid_database_is_open") || !is_open ||
        !check(scid_database_game_count_get(database, &count), "scid_database_game_count_get") ||
        count != 0 ||
        !check(
            scid_game_create_from_pgn(
                pgn, strlen(pgn), &game, diagnostic, sizeof(diagnostic), &diagnostic_size),
            "scid_game_create_from_pgn"))
    {
        fprintf(stderr, "%.*s\n", (int)diagnostic_size, diagnostic);
        scid_game_free(game);
        scid_database_free(database);
        return 1;
    }

    printf("open: %d\n", is_open);
    printf("games before add: %zu\n", count);

    if (!check(scid_database_game_add(database, game, NULL), "scid_database_game_add") ||
        !check(scid_database_game_count_get(database, &count), "scid_database_game_count_get") ||
        count != 1)
    {
        scid_game_free(game);
        scid_database_free(database);
        return 1;
    }

    printf("games after first add: %zu\n", count);

    if (!check(scid_database_game_add(database, game, "M"), "scid_database_game_add") ||
        !check(scid_database_game_count_get(database, &count), "scid_database_game_count_get") ||
        count != 2)
    {
        scid_game_free(game);
        scid_database_free(database);
        return 1;
    }

    printf("games after second add: %zu\n", count);

    if (!check(
            scid_database_filter_create(database, filter_id, sizeof(filter_id), &filter_id_size),
            "scid_database_filter_create") ||
        !check(scid_database_filter_fill(database, filter_id, 0), "scid_database_filter_fill") ||
        !check(
            scid_database_filter_value_set(database, filter_id, 1, 1),
            "scid_database_filter_value_set") ||
        !check(
            scid_database_filter_count_get(database, filter_id, &count),
            "scid_database_filter_count_get") ||
        count != 1 ||
        !check(
            scid_database_game_list_get(
                database, filter_id, "d+", 0, 1, game_indexes, 1, &listed_count),
            "scid_database_game_list_get") ||
        listed_count != 1 || game_indexes[0] != 1 ||
        !check(
            scid_database_game_sorted_position_get(
                database, filter_id, "d+", game_indexes[0], &sorted_position),
            "scid_database_game_sorted_position_get") ||
        sorted_position != 0 ||
        !check(
            scid_database_game_get(
                database, game_indexes[0], &loaded, flags, sizeof(flags), &flags_size),
            "scid_database_game_get") ||
        !text_equals(flags, flags_size, "M") ||
        !check(
            scid_game_tag_get(loaded, "Event", text, sizeof(text), &text_size),
            "scid_game_tag_get") ||
        !text_equals(text, text_size, "Memory") ||
        !check(
            scid_game_mainline_halfmove_count_get(loaded, &count),
            "scid_game_mainline_halfmove_count_get") ||
        count != 3)
    {
        scid_database_filter_delete(database, filter_id);
        scid_game_free(loaded);
        scid_game_free(game);
        scid_database_free(database);
        return 1;
    }

    printf("loaded flags: %.*s\n", (int)flags_size, flags);
    printf("loaded event: %.*s\n", (int)text_size, text);
    printf("loaded halfmoves: %zu\n", count);

    scid_database_filter_delete(database, filter_id);
    scid_game_free(loaded);
    scid_game_free(game);
    scid_database_free(database);
    return 0;
}
