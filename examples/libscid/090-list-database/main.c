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
add_pgn(
    scid_database* database,
    const char* pgn,
    const char* flags)
{
    const char* start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    scid_game* game = NULL;
    scid_position* position = NULL;
    char diagnostic[1024];
    size_t diagnostic_size = 0;

    if (!check(
            scid_position_create_from_fen(start_fen, &position), "scid_position_create_from_fen") ||
        !check(
            scid_game_create(
                position, pgn, strlen(pgn), &game, diagnostic, sizeof(diagnostic),
                &diagnostic_size),
            "scid_game_create"))
    {
        fprintf(stderr, "%.*s\n", (int)diagnostic_size, diagnostic);
        scid_position_free(position);
        scid_game_free(game);
        return 0;
    }

    if (!check(scid_database_game_add(database, game, flags), "scid_database_game_add"))
    {
        scid_position_free(position);
        scid_game_free(game);
        return 0;
    }

    scid_position_free(position);
    scid_game_free(game);
    return 1;
}

static int
print_game_row(
    scid_database* database,
    size_t index)
{
    char event[128];
    char white[128];
    char black[128];
    char date[32];
    char result[16];
    char eco_text[SCID_ECO_STRING_CAPACITY];
    size_t number = 0;
    size_t halfmoves = 0;
    size_t event_size = 0;
    size_t white_size = 0;
    size_t black_size = 0;
    size_t date_size = 0;
    size_t result_size = 0;
    size_t eco_size = 0;
    scid_eco_code eco = SCID_ECO_NONE;
    int deleted = 0;

    if (!check(
            scid_database_game_number_get(database, index, &number),
            "scid_database_game_number_get") ||
        !check(
            scid_database_game_deleted_get(database, index, &deleted),
            "scid_database_game_deleted_get") ||
        !check(
            scid_database_game_tag_get(database, index, "Event", event, sizeof(event), &event_size),
            "scid_database_game_tag_get") ||
        !check(
            scid_database_game_tag_get(database, index, "White", white, sizeof(white), &white_size),
            "scid_database_game_tag_get") ||
        !check(
            scid_database_game_tag_get(database, index, "Black", black, sizeof(black), &black_size),
            "scid_database_game_tag_get") ||
        !check(
            scid_database_game_date_get(database, index, date, sizeof(date), &date_size),
            "scid_database_game_date_get") ||
        !check(
            scid_database_game_result_get(database, index, result, sizeof(result), &result_size),
            "scid_database_game_result_get") ||
        !check(scid_database_game_eco_get(database, index, &eco), "scid_database_game_eco_get") ||
        !check(
            scid_eco_code_to_string(
                eco, SCID_ECO_FORMAT_BASIC, eco_text, sizeof(eco_text), &eco_size),
            "scid_eco_code_to_string") ||
        !check(
            scid_database_game_halfmove_count_get(database, index, &halfmoves),
            "scid_database_game_halfmove_count_get"))
    {
        return 0;
    }

    printf(
        "%zu | %.*s | %.*s - %.*s | %.*s | %.*s | %.*s | %zu halfmoves | deleted=%d\n", number,
        (int)event_size, event, (int)white_size, white, (int)black_size, black, (int)date_size,
        date, (int)result_size, result, (int)eco_size, eco_text, halfmoves, deleted);

    return 1;
}

int
main(
    void)
{
    const char* first_pgn = "[Event \"Archive One\"]\n"
                            "[Site \"Toronto\"]\n"
                            "[Date \"2024.06.14\"]\n"
                            "[White \"Alpha\"]\n"
                            "[Black \"Beta\"]\n"
                            "[Result \"1-0\"]\n"
                            "[ECO \"C20\"]\n"
                            "\n"
                            "1. e4 e5 2. Nf3 1-0\n";
    const char* second_pgn = "[Event \"Archive Two\"]\n"
                             "[Site \"Vancouver\"]\n"
                             "[Date \"2025.01.02\"]\n"
                             "[White \"Gamma\"]\n"
                             "[Black \"Delta\"]\n"
                             "[Result \"0-1\"]\n"
                             "[ECO \"D00\"]\n"
                             "\n"
                             "1. d4 d5 0-1\n";
    scid_database* database = NULL;
    char text[128];
    size_t count = 0;
    size_t text_size = 0;
    int deleted = 0;

    if (!check(scid_database_create_memory("listing", &database), "scid_database_create_memory") ||
        !add_pgn(database, first_pgn, "D") || !add_pgn(database, second_pgn, "M") ||
        !check(scid_database_game_count_get(database, &count), "scid_database_game_count_get") ||
        count != 2)
    {
        scid_database_free(database);
        return 1;
    }

    for (size_t index = 0; index < count; ++index)
    {
        if (!print_game_row(database, index))
        {
            scid_database_free(database);
            return 1;
        }
    }

    if (!check(
            scid_database_game_tag_get(database, 0, "Event", text, sizeof(text), &text_size),
            "scid_database_game_tag_get") ||
        !text_equals(text, text_size, "Archive One") ||
        !check(
            scid_database_game_deleted_get(database, 0, &deleted),
            "scid_database_game_deleted_get") ||
        deleted != 1 ||
        !check(
            scid_database_game_tag_get(database, 1, "Event", text, sizeof(text), &text_size),
            "scid_database_game_tag_get") ||
        !text_equals(text, text_size, "Archive Two"))
    {
        scid_database_free(database);
        return 1;
    }

    scid_database_free(database);
    return 0;
}
