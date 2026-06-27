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
read_text(
    scid_error error,
    const char* call,
    const char* label,
    const char* text,
    size_t text_size)
{
    if (!check(error, call))
    {
        return 0;
    }

    printf("%s%.*s\n", label, (int)text_size, text);
    return 1;
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
print_next_move(
    scid_movetext_cursor* cursor)
{
    char san[64];
    char uci[16];
    char comment[256];
    scid_movespec move;
    scid_nag nag = 0;
    size_t san_size = 0;
    size_t uci_size = 0;
    size_t comment_size = 0;
    size_t nag_count = 0;
    size_t variation_count = 0;
    size_t ply = 0;

    if (!check(scid_movetext_cursor_ply_get(cursor, &ply), "scid_movetext_cursor_ply_get") ||
        !check(
            scid_movetext_cursor_next_movespec_get(cursor, &move),
            "scid_movetext_cursor_next_movespec_get") ||
        !read_text(
            scid_movespec_to_uci(move, uci, sizeof(uci), &uci_size), "scid_movespec_to_uci",
            "next uci: ", uci, uci_size) ||
        !read_text(
            scid_movetext_cursor_next_move_san_get(cursor, san, sizeof(san), &san_size),
            "scid_movetext_cursor_next_move_san_get", "next san: ", san, san_size) ||
        !read_text(
            scid_movetext_cursor_next_move_comment_get(
                cursor, comment, sizeof(comment), &comment_size),
            "scid_movetext_cursor_next_move_comment_get", "next comment: ", comment,
            comment_size) ||
        !check(
            scid_movetext_cursor_next_move_nag_count_get(cursor, &nag_count),
            "scid_movetext_cursor_next_move_nag_count_get") ||
        !check(
            scid_movetext_cursor_variation_count_get(cursor, &variation_count),
            "scid_movetext_cursor_variation_count_get"))
    {
        return 0;
    }

    if (!text_equals(san, san_size, "e4") || !text_equals(uci, uci_size, "e2e4") ||
        !text_equals(comment, comment_size, "King pawn") || ply != 0 || nag_count != 1 ||
        variation_count != 1)
    {
        return 0;
    }

    printf("ply before move: %zu\n", ply);
    printf("next nag count: %zu\n", nag_count);
    printf("next variation count: %zu\n", variation_count);

    if (nag_count > 0)
    {
        if (!check(
                scid_movetext_cursor_next_move_nag_at_get(cursor, 0, &nag),
                "scid_movetext_cursor_next_move_nag_at_get"))
        {
            return 0;
        }
        printf("first next nag: %u\n", (unsigned)nag);
    }

    return 1;
}

int
main(
    void)
{
    const char* pgn = "[Event \"Navigation\"]\n"
                      "[Result \"*\"]\n"
                      "\n"
                      "{Before game} 1. e4 $1 {King pawn} "
                      "({Queen pawn line} 1. d4 {Queen pawn} d5) e5 2. Nf3 Nc6 *\n";
    scid_game* game = NULL;
    scid_movetext_cursor* cursor = NULL;
    char diagnostic[1024];
    char text[256];
    scid_movespec move;
    int moved = 0;
    size_t diagnostic_size = 0;
    size_t text_size = 0;
    size_t depth = 0;
    size_t variation_count = 0;

    if (!check(
            scid_game_create_from_pgn(
                pgn, strlen(pgn), &game, diagnostic, sizeof(diagnostic), &diagnostic_size),
            "scid_game_create_from_pgn") ||
        !check(scid_movetext_cursor_create(game, &cursor), "scid_movetext_cursor_create"))
    {
        fprintf(stderr, "%.*s\n", (int)diagnostic_size, diagnostic);
        scid_movetext_cursor_free(cursor);
        scid_game_free(game);
        return 1;
    }

    if (!read_text(
            scid_movetext_cursor_comment_get(cursor, text, sizeof(text), &text_size),
            "scid_movetext_cursor_comment_get", "initial comment: ", text, text_size) ||
        !text_equals(text, text_size, "Before game") || !print_next_move(cursor) ||
        !check(
            scid_movetext_cursor_variation_count_get(cursor, &variation_count),
            "scid_movetext_cursor_variation_count_get") ||
        variation_count != 1 ||
        !check(
            scid_movetext_cursor_variation_enter(cursor, 0, &moved),
            "scid_movetext_cursor_variation_enter") ||
        !moved ||
        !check(
            scid_movetext_cursor_variation_depth_get(cursor, &depth),
            "scid_movetext_cursor_variation_depth_get") ||
        depth != 1 ||
        !read_text(
            scid_movetext_cursor_comment_get(cursor, text, sizeof(text), &text_size),
            "scid_movetext_cursor_comment_get", "variation comment: ", text, text_size) ||
        !text_equals(text, text_size, "Queen pawn line") ||
        !read_text(
            scid_movetext_cursor_next_move_san_get(cursor, text, sizeof(text), &text_size),
            "scid_movetext_cursor_next_move_san_get", "variation first move: ", text, text_size) ||
        !text_equals(text, text_size, "d4") ||
        !check(
            scid_movetext_cursor_variation_exit(cursor, &moved),
            "scid_movetext_cursor_variation_exit") ||
        !moved || !check(scid_movetext_cursor_next(cursor, &moved), "scid_movetext_cursor_next") ||
        !moved ||
        !read_text(
            scid_movetext_cursor_previous_move_san_get(cursor, text, sizeof(text), &text_size),
            "scid_movetext_cursor_previous_move_san_get", "previous san: ", text, text_size) ||
        !text_equals(text, text_size, "e4") ||
        !check(
            scid_movetext_cursor_previous_movespec_get(cursor, &move),
            "scid_movetext_cursor_previous_movespec_get") ||
        !read_text(
            scid_movespec_to_uci(move, text, sizeof(text), &text_size), "scid_movespec_to_uci",
            "previous uci: ", text, text_size) ||
        !text_equals(text, text_size, "e2e4") ||
        !read_text(
            scid_movetext_cursor_next_move_san_get(cursor, text, sizeof(text), &text_size),
            "scid_movetext_cursor_next_move_san_get", "next san after e4: ", text, text_size) ||
        !text_equals(text, text_size, "e5") ||
        !check(
            scid_movetext_cursor_next_movespec_get(cursor, &move),
            "scid_movetext_cursor_next_movespec_get") ||
        !read_text(
            scid_movespec_to_uci(move, text, sizeof(text), &text_size), "scid_movespec_to_uci",
            "next uci after e4: ", text, text_size) ||
        !text_equals(text, text_size, "e7e5"))
    {
        scid_movetext_cursor_free(cursor);
        scid_game_free(game);
        return 1;
    }

    scid_movetext_cursor_free(cursor);
    scid_game_free(game);
    return 0;
}
