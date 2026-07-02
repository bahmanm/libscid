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
    const char* needle)
{
    return strstr(text, needle) != NULL;
}

int
main(
    void)
{
    const char* start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    const char* pgn = "[Event \"St Petersburg final\"]\n"
                      "[Site \"St Petersburg\"]\n"
                      "[Date \"1914.05.18\"]\n"
                      "[Round \"7\"]\n"
                      "[White \"Lasker, Emanuel\"]\n"
                      "[Black \"Capablanca, Jose Raul\"]\n"
                      "[Result \"1-0\"]\n"
                      "[ECO \"C68\"]\n"
                      "[EventDate \"1914.04.21\"]\n"
                      "[Annotator \"Example\"]\n"
                      "\n"
                      "1. e4 e5 2. Nf3 Nc6 3. Bb5 a6 1-0\n";
    scid_game* game = NULL;
    scid_position* position = NULL;
    char diagnostic[1024];
    char name[64];
    char value[256];
    char encoded[4096];
    size_t diagnostic_size = 0;
    size_t name_size = 0;
    size_t value_size = 0;
    size_t encoded_size = 0;
    size_t tag_count = 0;
    int removed = 0;

    if (!check(
            scid_position_create_from_fen(start_fen, &position),
            "scid_position_create_from_fen") ||
        !check(
            scid_game_create(
                position, pgn, strlen(pgn), &game, diagnostic, sizeof(diagnostic),
                &diagnostic_size),
            "scid_game_create"))
    {
        fprintf(stderr, "%.*s\n", (int)diagnostic_size, diagnostic);
        scid_position_free(position);
        return 1;
    }

    if (!check(scid_game_tag_count_get(game, &tag_count), "scid_game_tag_count_get"))
    {
        scid_position_free(position);
        scid_game_free(game);
        return 1;
    }

    for (size_t i = 0; i < tag_count; ++i)
    {
        if (!check(
                scid_game_tag_at_get(
                    game, i, name, sizeof(name), &name_size, value, sizeof(value), &value_size),
                "scid_game_tag_at_get"))
        {
            scid_position_free(position);
            scid_game_free(game);
            return 1;
        }

        printf("%.*s: %.*s\n", (int)name_size, name, (int)value_size, value);
    }

    if (!check(scid_game_tag_set(game, "Annotator", "C ABI example"), "scid_game_tag_set") ||
        !check(scid_game_tag_remove(game, "EventDate", &removed), "scid_game_tag_remove") ||
        !check(scid_game_to_pgn(game, encoded, sizeof(encoded), &encoded_size), "scid_game_to_pgn"))
    {
        scid_position_free(position);
        scid_game_free(game);
        return 1;
    }

    printf("\n%.*s", (int)encoded_size, encoded);

    if (!removed || !contains(encoded, "[Annotator \"C ABI example\"]") ||
        contains(encoded, "[EventDate "))
    {
        scid_position_free(position);
        scid_game_free(game);
        return 1;
    }

    scid_position_free(position);
    scid_game_free(game);
    return 0;
}
