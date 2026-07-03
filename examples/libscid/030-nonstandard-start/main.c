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


static int
contains(
    const char* text,
    const char* needle)
{
    return strstr(text, needle) != NULL;
}


int
main(void)
{
    const char*    fen = "8/K7/8/8/7k/8/8/8 w - - 45 25";
    scid_position* position = NULL;
    scid_game*     game = NULL;
    char           text[4096];
    size_t         text_size = 0;
    size_t         tag_count = 0;

    if (!check(scid_position_create_from_fen(fen, &position), "scid_position_create_from_fen") ||
        !check(scid_game_create_blank(position, &game), "scid_game_create_blank") ||
        !check(scid_game_tag_set(game, "Event", "Study position"), "scid_game_tag_set") ||
        !check(scid_game_tag_get(game, "FEN", text, sizeof(text), &text_size), "scid_game_tag_get"))
    {
        scid_game_free(game);
        scid_position_free(position);
        return 1;
    }

    printf("start fen: %.*s\n", (int)text_size, text);
    if (text_size != strlen(fen) || strncmp(text, fen, text_size) != 0)
    {
        scid_game_free(game);
        scid_position_free(position);
        return 1;
    }

    if (!check(scid_game_tag_count_get(game, &tag_count), "scid_game_tag_count_get") ||
        tag_count != 8 ||
        !check(scid_game_to_pgn(game, NULL, text, sizeof(text), &text_size), "scid_game_to_pgn"))
    {
        scid_game_free(game);
        scid_position_free(position);
        return 1;
    }

    printf("%.*s", (int)text_size, text);
    if (!contains(text, "[Event \"Study position\"]") ||
        !contains(text, "[FEN \"8/K7/8/8/7k/8/8/8 w - - 45 25\"]"))
    {
        scid_game_free(game);
        scid_position_free(position);
        return 1;
    }

    scid_game_free(game);
    scid_position_free(position);
    return 0;
}
