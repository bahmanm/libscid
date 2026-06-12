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

static int add_san_move(
    scid_movetext_cursor* cursor,
    scid_position* position,
    const char* san
) {
    scid_movespec move;
    if (!check(scid_movespec_create_from_san(position, san, &move),
               "scid_movespec_create_from_san")) {
        return 0;
    }
    if (!check(scid_movetext_cursor_move_add(cursor, move),
               "scid_movetext_cursor_move_add")) {
        return 0;
    }
    return check(scid_position_apply_san(position, san),
                 "scid_position_apply_san");
}

static int contains(const char* text, const char* needle) {
    return strstr(text, needle) != NULL;
}

int main(void) {
    scid_game* game = NULL;
    scid_position* position = NULL;
    scid_movetext_cursor* cursor = NULL;
    char pgn[4096];
    size_t pgn_size = 0;
    int moved = 0;
    int changed = 0;

    if (!check(scid_game_create_empty(&game), "scid_game_create_empty") ||
        !check(scid_position_create_standard(&position),
               "scid_position_create_standard") ||
        !check(scid_movetext_cursor_create(game, &cursor),
               "scid_movetext_cursor_create")) {
        scid_movetext_cursor_free(cursor);
        scid_position_free(position);
        scid_game_free(game);
        return 1;
    }

    if (!check(scid_game_tag_set(game, "Event", "C ABI Example"),
               "scid_game_tag_set") ||
        !check(scid_game_tag_set(game, "Site", "Example"),
               "scid_game_tag_set") ||
        !check(scid_game_tag_set(game, "Date", "2026.06.10"),
               "scid_game_tag_set") ||
        !check(scid_game_tag_set(game, "White", "White"),
               "scid_game_tag_set") ||
        !check(scid_game_tag_set(game, "Black", "Black"),
               "scid_game_tag_set") ||
        !check(scid_game_tag_set(game, "Result", "*"),
               "scid_game_tag_set")) {
        scid_movetext_cursor_free(cursor);
        scid_position_free(position);
        scid_game_free(game);
        return 1;
    }

    if (!add_san_move(cursor, position, "e4") ||
        !check(scid_movetext_cursor_comment_set(cursor, "King pawn"),
               "scid_movetext_cursor_comment_set") ||
        !check(scid_movetext_cursor_nag_add(cursor, 1, &changed),
               "scid_movetext_cursor_nag_add") ||
        !changed ||
        !add_san_move(cursor, position, "e5") ||
        !add_san_move(cursor, position, "Nf3") ||
        !add_san_move(cursor, position, "Nc6")) {
        scid_movetext_cursor_free(cursor);
        scid_position_free(position);
        scid_game_free(game);
        return 1;
    }

    if (!check(scid_movetext_cursor_to_ply(cursor, 1, &moved),
               "scid_movetext_cursor_to_ply") ||
        !moved ||
        !check(scid_movetext_cursor_position_get(cursor, position),
               "scid_movetext_cursor_position_get") ||
        !check(scid_movetext_cursor_variation_add(
                   cursor,
                   "Sicilian branch",
                   &changed),
               "scid_movetext_cursor_variation_add") ||
        !changed ||
        !add_san_move(cursor, position, "c5") ||
        !check(scid_movetext_cursor_variation_exit(cursor, &moved),
               "scid_movetext_cursor_variation_exit") ||
        !moved ||
        !check(scid_movetext_cursor_to_end(cursor),
               "scid_movetext_cursor_to_end") ||
        !check(scid_game_to_pgn(game, pgn, sizeof(pgn), &pgn_size),
               "scid_game_to_pgn")) {
        scid_movetext_cursor_free(cursor);
        scid_position_free(position);
        scid_game_free(game);
        return 1;
    }

    printf("%.*s", (int)pgn_size, pgn);

    if (!contains(pgn, "[Event \"C ABI Example\"]") ||
        !contains(pgn, "e4 $1 {King pawn}") ||
        !contains(pgn, "{Sicilian branch}") ||
        !contains(pgn, "c5")) {
        scid_movetext_cursor_free(cursor);
        scid_position_free(position);
        scid_game_free(game);
        return 1;
    }

    scid_movetext_cursor_free(cursor);
    scid_position_free(position);
    scid_game_free(game);
    return 0;
}
