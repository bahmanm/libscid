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
play(
    scid_position* position,
    const char* san)
{
    return check(scid_position_apply_san(position, san), "scid_position_apply_san");
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
    scid_eco_book* book = NULL;
    scid_position* position = NULL;
    scid_eco_code code = SCID_ECO_NONE;
    char code_text[SCID_ECO_STRING_CAPACITY];
    char name[128];
    char fen[256];
    size_t code_size = 0;
    size_t name_size = 0;
    size_t fen_size = 0;

    if (!check(scid_eco_book_load(LIBSCID_EXAMPLE_ECO_FILE, &book), "scid_eco_book_load") ||
        !check(scid_position_create_standard(&position), "scid_position_create_standard") ||
        !play(position, "d4") || !play(position, "d5") || !play(position, "c4") ||
        !play(position, "e6") ||
        !check(
            scid_position_to_fen(position, fen, sizeof(fen), &fen_size), "scid_position_to_fen") ||
        !check(scid_eco_book_code_find(book, position, &code), "scid_eco_book_code_find") ||
        !check(
            scid_eco_code_to_string(
                code, SCID_ECO_FORMAT_BASIC, code_text, sizeof(code_text), &code_size),
            "scid_eco_code_to_string") ||
        !check(
            scid_eco_book_name_find(book, position, name, sizeof(name), &name_size),
            "scid_eco_book_name_find"))
    {
        scid_position_free(position);
        scid_eco_book_free(book);
        return 1;
    }

    printf("position: %.*s\n", (int)fen_size, fen);
    printf("eco code: %.*s\n", (int)code_size, code_text);
    printf("eco name: %.*s\n", (int)name_size, name);

    if (!text_equals(code_text, code_size, "D30") ||
        !text_equals(name, name_size, "Queen's Gambit Declined"))
    {
        scid_position_free(position);
        scid_eco_book_free(book);
        return 1;
    }

    scid_position_free(position);
    scid_eco_book_free(book);
    return 0;
}
