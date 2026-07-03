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
    scid_game_cursor** cursor,
    scid_game_cursor** next_cursor)
{
    if (next_cursor == NULL || *next_cursor == NULL)
    {
        return 0;
    }

    scid_game_cursor_free(*cursor);
    *cursor = *next_cursor;
    *next_cursor = NULL;
    return 1;
}

static int
take_position(
    scid_position** position,
    scid_position* next_position)
{
    if (next_position == NULL)
    {
        return 0;
    }

    scid_position_free(*position);
    *position = next_position;
    return 1;
}

static int
add_san_move(
    scid_game* game,
    scid_game_cursor** cursor,
    scid_position** position,
    const char* san)
{
    scid_movespec move;
    scid_game_cursor* next_cursor = NULL;
    scid_position* next_position = NULL;
    if (!check(
            scid_movespec_create_from_san(*position, san, &move), "scid_movespec_create_from_san"))
    {
        return 0;
    }
    if (!check(
            scid_game_cursor_move_add(game, *cursor, move, &next_cursor),
            "scid_game_cursor_move_add") ||
        !check(
            scid_position_create_with_san(*position, san, &next_position),
            "scid_position_create_with_san"))
    {
        scid_game_cursor_free(next_cursor);
        scid_position_free(next_position);
        return 0;
    }

    return take_cursor(cursor, &next_cursor) && take_position(position, next_position);
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
    const char* standard_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    scid_game* game = NULL;
    scid_position* position = NULL;
    scid_game_cursor* cursor = NULL;
    scid_game_pgn_options* pgn_options = NULL;
    char pgn[4096];
    size_t pgn_size = 0;
    int moved = 0;
    int changed = 0;

    if (!check(
            scid_position_create_from_fen(standard_fen, &position),
            "scid_position_create_from_fen") ||
        !check(scid_game_create_blank(position, &game), "scid_game_create_blank") ||
        !check(scid_game_cursor_create(game, &cursor), "scid_game_cursor_create"))
    {
        scid_game_cursor_free(cursor);
        scid_position_free(position);
        scid_game_free(game);
        return 1;
    }

    if (!check(scid_game_tag_set(game, "Event", "C ABI Example"), "scid_game_tag_set") ||
        !check(scid_game_tag_set(game, "Site", "Example"), "scid_game_tag_set") ||
        !check(scid_game_tag_set(game, "Date", "2026.06.10"), "scid_game_tag_set") ||
        !check(scid_game_tag_set(game, "White", "White"), "scid_game_tag_set") ||
        !check(scid_game_tag_set(game, "Black", "Black"), "scid_game_tag_set") ||
        !check(scid_game_tag_set(game, "Result", "*"), "scid_game_tag_set"))
    {
        scid_game_cursor_free(cursor);
        scid_position_free(position);
        scid_game_free(game);
        return 1;
    }

    if (!add_san_move(game, &cursor, &position, "e4") ||
        !check(
            scid_game_cursor_comment_set(game, cursor, "King pawn"),
            "scid_game_cursor_comment_set") ||
        !check(scid_game_cursor_nag_add(game, cursor, 1, &changed), "scid_game_cursor_nag_add") ||
        !changed || !add_san_move(game, &cursor, &position, "e5") ||
        !add_san_move(game, &cursor, &position, "Nf3") ||
        !add_san_move(game, &cursor, &position, "Nc6"))
    {
        scid_game_cursor_free(cursor);
        scid_position_free(position);
        scid_game_free(game);
        return 1;
    }

    {
        scid_game_cursor* next_cursor = NULL;
        if (!check(
                scid_game_cursor_to_ply(cursor, 1, &moved, &next_cursor),
                "scid_game_cursor_to_ply") ||
            !moved || !take_cursor(&cursor, &next_cursor))
        {
            scid_game_cursor_free(next_cursor);
            scid_game_cursor_free(cursor);
            scid_position_free(position);
            scid_game_free(game);
            return 1;
        }
    }

    {
        scid_game_cursor* next_cursor = NULL;
        if (!check(
                scid_game_cursor_position_get(cursor, position), "scid_game_cursor_position_get") ||
            !check(
                scid_game_cursor_variation_add(
                    game, cursor, "Sicilian branch", &changed, &next_cursor),
                "scid_game_cursor_variation_add") ||
            !changed || !take_cursor(&cursor, &next_cursor))
        {
            scid_game_cursor_free(next_cursor);
            scid_game_cursor_free(cursor);
            scid_position_free(position);
            scid_game_free(game);
            return 1;
        }
        next_cursor = NULL;
        if (!add_san_move(game, &cursor, &position, "c5"))
        {
            scid_game_cursor_free(cursor);
            scid_position_free(position);
            scid_game_free(game);
            return 1;
        }
    }

    {
        scid_game_cursor* next_cursor = NULL;
        if (!check(
                scid_game_cursor_variation_exit(cursor, &moved, &next_cursor),
                "scid_game_cursor_variation_exit") ||
            !moved || !take_cursor(&cursor, &next_cursor))
        {
            scid_game_cursor_free(next_cursor);
            scid_game_cursor_free(cursor);
            scid_position_free(position);
            scid_game_free(game);
            return 1;
        }
        next_cursor = NULL;
        if (!check(scid_game_cursor_to_end(cursor, &next_cursor), "scid_game_cursor_to_end") ||
            !take_cursor(&cursor, &next_cursor))
        {
            scid_game_cursor_free(next_cursor);
            scid_game_cursor_free(cursor);
            scid_position_free(position);
            scid_game_free(game);
            return 1;
        }
    }

    if (!check(scid_game_to_pgn(game, NULL, pgn, sizeof(pgn), &pgn_size), "scid_game_to_pgn"))
    {
        scid_game_cursor_free(cursor);
        scid_position_free(position);
        scid_game_free(game);
        return 1;
    }

    printf("%.*s", (int)pgn_size, pgn);

    if (!contains(pgn, "[Event \"C ABI Example\"]") || !contains(pgn, "e4 $1 {King pawn}") ||
        !contains(pgn, "{Sicilian branch}") || !contains(pgn, "c5"))
    {
        scid_game_cursor_free(cursor);
        scid_position_free(position);
        scid_game_free(game);
        return 1;
    }

    if (!check(scid_game_pgn_options_create(&pgn_options), "scid_game_pgn_options_create") ||
        !check(
            scid_game_pgn_options_symbolic_nags_set(pgn_options, 1),
            "scid_game_pgn_options_symbolic_nags_set") ||
        !check(
            scid_game_pgn_options_variations_set(pgn_options, 0),
            "scid_game_pgn_options_variations_set") ||
        !check(
            scid_game_to_pgn(game, pgn_options, pgn, sizeof(pgn), &pgn_size), "scid_game_to_pgn"))
    {
        scid_game_pgn_options_free(pgn_options);
        scid_game_cursor_free(cursor);
        scid_position_free(position);
        scid_game_free(game);
        return 1;
    }

    printf("\nMainline with symbolic NAGs:\n%.*s", (int)pgn_size, pgn);

    if (!contains(pgn, "e4 ! {King pawn}") || contains(pgn, "{Sicilian branch}") ||
        contains(pgn, "c5"))
    {
        scid_game_pgn_options_free(pgn_options);
        scid_game_cursor_free(cursor);
        scid_position_free(position);
        scid_game_free(game);
        return 1;
    }

    scid_game_pgn_options_free(pgn_options);
    scid_game_cursor_free(cursor);
    scid_position_free(position);
    scid_game_free(game);
    return 0;
}
