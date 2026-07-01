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
take_cursor(
    scid_movetext_cursor** cursor,
    scid_movetext_cursor** next_cursor)
{
    if (next_cursor == NULL || *next_cursor == NULL)
    {
        return 0;
    }

    scid_movetext_cursor_free(*cursor);
    *cursor = *next_cursor;
    *next_cursor = NULL;
    return 1;
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
    const char* pgn = "[Event \"Mutation\"]\n"
                      "[Result \"*\"]\n"
                      "\n"
                      "1. e4 e5 (1... c5) *\n";
    scid_game* game = NULL;
    scid_movetext_cursor* cursor = NULL;
    scid_movetext_cursor* next_cursor = NULL;
    scid_position* position = NULL;
    scid_movespec move;
    char output[4096];
    int changed = 0;
    int moved = 0;
    size_t output_size = 0;
    size_t variation_count = 0;

    if (!check(
            scid_game_create_from_pgn(pgn, strlen(pgn), &game, NULL, 0, NULL),
            "scid_game_create_from_pgn") ||
        !check(scid_movetext_cursor_create(game, &cursor), "scid_movetext_cursor_create") ||
        !check(scid_position_create_empty(&position), "scid_position_create_empty"))
    {
        scid_position_free(position);
        scid_movetext_cursor_free(cursor);
        scid_game_free(game);
        return 1;
    }

    if (!check(scid_movetext_cursor_next(cursor, &moved, &next_cursor), "scid_movetext_cursor_next") ||
        !moved || !take_cursor(&cursor, &next_cursor))
    {
        scid_movetext_cursor_free(next_cursor);
        scid_position_free(position);
        scid_movetext_cursor_free(cursor);
        scid_game_free(game);
        return 1;
    }
    next_cursor = NULL;

    if (!check(
            scid_movetext_cursor_comment_set(game, cursor, "King pawn"),
            "scid_movetext_cursor_comment_set") ||
        !check(
            scid_movetext_cursor_nag_add(game, cursor, 1, &changed),
            "scid_movetext_cursor_nag_add") ||
        !changed ||
        !check(
            scid_movetext_cursor_variation_count_get(cursor, &variation_count),
            "scid_movetext_cursor_variation_count_get") ||
        variation_count != 1)
    {
        scid_position_free(position);
        scid_movetext_cursor_free(cursor);
        scid_game_free(game);
        return 1;
    }

    if (!check(
            scid_movetext_cursor_variation_add(
                game, cursor, "French branch", &changed, &next_cursor),
            "scid_movetext_cursor_variation_add") ||
        !changed || !take_cursor(&cursor, &next_cursor))
    {
        scid_movetext_cursor_free(next_cursor);
        scid_position_free(position);
        scid_movetext_cursor_free(cursor);
        scid_game_free(game);
        return 1;
    }
    next_cursor = NULL;

    if (!check(
            scid_movetext_cursor_position_get(cursor, position),
            "scid_movetext_cursor_position_get") ||
        !check(
            scid_movespec_create_from_san(position, "e6", &move),
            "scid_movespec_create_from_san") ||
        !check(
            scid_movetext_cursor_move_add(game, cursor, move, &next_cursor),
            "scid_movetext_cursor_move_add") ||
        !take_cursor(&cursor, &next_cursor))
    {
        scid_movetext_cursor_free(next_cursor);
        scid_position_free(position);
        scid_movetext_cursor_free(cursor);
        scid_game_free(game);
        return 1;
    }
    next_cursor = NULL;

    if (!check(
            scid_movetext_cursor_variation_promote_to_first(
                game, cursor, &changed, &next_cursor),
            "scid_movetext_cursor_variation_promote_to_first") ||
        !changed || !take_cursor(&cursor, &next_cursor))
    {
        scid_movetext_cursor_free(next_cursor);
        scid_position_free(position);
        scid_movetext_cursor_free(cursor);
        scid_game_free(game);
        return 1;
    }
    next_cursor = NULL;

    if (!check(
            scid_movetext_cursor_variation_exit(cursor, &moved, &next_cursor),
            "scid_movetext_cursor_variation_exit") ||
        !moved || !take_cursor(&cursor, &next_cursor) ||
        !check(scid_game_to_pgn(game, output, sizeof(output), &output_size), "scid_game_to_pgn"))
    {
        scid_movetext_cursor_free(next_cursor);
        scid_position_free(position);
        scid_movetext_cursor_free(cursor);
        scid_game_free(game);
        return 1;
    }
    next_cursor = NULL;

    printf("%.*s", (int)output_size, output);

    if (!contains(output, "e4 $1 {King pawn}") || !contains(output, "{French branch}") ||
        !contains(output, "e6") || !contains(output, "c5"))
    {
        scid_position_free(position);
        scid_movetext_cursor_free(cursor);
        scid_game_free(game);
        return 1;
    }

    scid_position_free(position);
    scid_movetext_cursor_free(cursor);
    scid_game_free(game);
    return 0;
}
