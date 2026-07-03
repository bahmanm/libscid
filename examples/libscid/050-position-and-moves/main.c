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
text_equals(
    const char* text,
    size_t      text_size,
    const char* expected)
{
    return text_size == strlen(expected) && strncmp(text, expected, text_size) == 0;
}


int
main(void)
{
    const char*    standard_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    const char*    expected_fen = "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/"
                                  "RNBQKBNR w KQkq - 0 2";
    scid_position* position = NULL;
    scid_position* next_position = NULL;
    scid_movespec  move;
    scid_colour    side_to_move = SCID_BLACK;
    scid_piece     piece = SCID_PIECE_NONE;
    char           text[256];
    size_t         text_size = 0;
    unsigned       number = 0;

    if (!check(
            scid_position_create_from_fen(standard_fen, &position),
            "scid_position_create_from_fen") ||
        !check(
            scid_movespec_create_from_san(position, "e4", &move),
            "scid_movespec_create_from_san") ||
        !check(
            scid_movespec_to_uci(move, text, sizeof(text), &text_size), "scid_movespec_to_uci") ||
        !text_equals(text, text_size, "e2e4") ||
        !check(
            scid_position_create_with_san(position, "e4", &next_position),
            "scid_position_create_with_san"))
    {
        scid_position_free(next_position);
        scid_position_free(position);
        return 1;
    }
    scid_position_free(position);
    position = next_position;
    next_position = NULL;

    if (!check(
            scid_position_create_with_uci(position, "c7c5", &next_position),
            "scid_position_create_with_uci"))
    {
        scid_position_free(next_position);
        scid_position_free(position);
        return 1;
    }
    scid_position_free(position);
    position = next_position;
    next_position = NULL;

    if (!check(
            scid_position_to_fen(position, text, sizeof(text), &text_size), "scid_position_to_fen"))
    {
        scid_position_free(next_position);
        scid_position_free(position);
        return 1;
    }

    printf("fen: %.*s\n", (int)text_size, text);
    if (!text_equals(text, text_size, expected_fen))
    {
        scid_position_free(position);
        return 1;
    }


    if (!check(
            scid_position_side_to_move_get(position, &side_to_move),
            "scid_position_side_to_move_get") ||
        side_to_move != SCID_WHITE ||
        !check(
            scid_position_fullmove_number_get(position, &number),
            "scid_position_fullmove_number_get") ||
        number != 2 ||
        !check(
            scid_position_halfmove_clock_get(position, &number),
            "scid_position_halfmove_clock_get") ||
        number != 0 || !check(scid_square_from_string("e4", &move.to), "scid_square_from_string") ||
        !check(
            scid_position_piece_at_get(position, move.to, &piece), "scid_position_piece_at_get") ||
        piece != SCID_PIECE_WHITE_PAWN ||
        !check(
            scid_square_to_string(move.to, text, sizeof(text), &text_size),
            "scid_square_to_string") ||
        !text_equals(text, text_size, "e4"))
    {
        scid_position_free(next_position);
        scid_position_free(position);
        return 1;
    }

    printf("side to move: white\n");
    printf("piece on %.*s: white pawn\n", (int)text_size, text);

    scid_position_free(next_position);
    scid_position_free(position);
    return 0;
}
