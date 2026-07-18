#include "test_libscid.h"

#include "scid/scid.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void
write_eco_file(const char* path)
{
    FILE* file = fopen(path, "w");
    assert(file != NULL);
    fputs(
        "# Test ECO file\n"
        "A00a \"Start position\" *\n"
        "B20 \"Sicilian Defence\" 1.e4 c5 *\n"
        "C50a \"Italian Game\" 1.e4 e5 2.Nf3 Nc6 3.Bc4 *\n",
        file);
    assert(fclose(file) == 0);
}


void
test_eco(void)
{
    char           path[128];
    char           text[32];
    scid_eco_book* book = NULL;
    scid_eco_code  code = 0;
    scid_position* position = NULL;
    size_t         text_size = 0;

    assert(scid_eco_code_from_string("B91a4", &code) == SCID_OK);
    assert(code != SCID_ECO_NONE);
    assert(
        scid_eco_code_to_string(code, SCID_ECO_FORMAT_BASIC, text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "B91") == 0);
    assert(text_size == 3);
    assert(
        scid_eco_code_to_string(code, SCID_ECO_FORMAT_EXTENDED, text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "B91a4") == 0);
    assert(text_size == 5);

    assert(scid_eco_code_from_string("not-eco", &code) == SCID_OK);
    assert(code == SCID_ECO_NONE);
    assert(
        scid_eco_code_to_string(
            SCID_ECO_NONE, SCID_ECO_FORMAT_EXTENDED, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "") == 0);
    assert(text_size == 0);

    assert(scid_eco_code_from_string(NULL, &code) == SCID_ERROR_BAD_ARG);
    assert(scid_eco_code_from_string("B20", NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_eco_code_to_string(code, 99, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    assert(
        scid_eco_code_to_string(code, SCID_ECO_FORMAT_EXTENDED, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);

    snprintf(path, sizeof(path), "libscid_test_eco_file.eco");
    remove(path);
    write_eco_file(path);

    assert(scid_eco_book_load("libscid_missing_eco_file.eco", &book) == SCID_ERROR_FILE_OPEN);
    assert(book == NULL);
    assert(scid_eco_book_load(path, &book) == SCID_OK);
    assert(book != NULL);

    assert(test_position_create_standard(&position) == SCID_OK);
    assert(scid_eco_book_code_find(book, position, &code) == SCID_OK);
    assert(
        scid_eco_code_to_string(code, SCID_ECO_FORMAT_EXTENDED, text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "A00a") == 0);
    assert(scid_eco_book_name_find(book, position, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Start position") == 0);
    assert(text_size == 14);

    assert(scid_position_apply_uci(position, "e2e4") == SCID_OK);
    assert(scid_position_apply_uci(position, "c7c5") == SCID_OK);
    assert(scid_eco_book_code_find(book, position, &code) == SCID_OK);
    assert(
        scid_eco_code_to_string(code, SCID_ECO_FORMAT_BASIC, text, sizeof(text), &text_size) ==
        SCID_OK);
    assert(strcmp(text, "B20") == 0);
    assert(scid_eco_book_name_find(book, position, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "Sicilian Defence") == 0);
    assert(text_size == 16);

    assert(scid_position_apply_uci(position, "g1f3") == SCID_OK);
    assert(scid_eco_book_code_find(book, position, &code) == SCID_OK);
    assert(code == SCID_ECO_NONE);
    assert(scid_eco_book_name_find(book, position, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "") == 0);
    assert(text_size == 0);

    assert(scid_eco_book_load(NULL, &book) == SCID_ERROR_BAD_ARG);
    assert(scid_eco_book_load(path, NULL) == SCID_ERROR_BAD_ARG);
    assert(scid_eco_book_code_find(NULL, position, &code) == SCID_ERROR_BAD_ARG);
    assert(scid_eco_book_code_find(book, NULL, &code) == SCID_ERROR_BAD_ARG);
    assert(scid_eco_book_code_find(book, position, NULL) == SCID_ERROR_BAD_ARG);
    assert(
        scid_eco_book_name_find(NULL, position, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    assert(
        scid_eco_book_name_find(book, NULL, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    assert(scid_eco_book_name_find(book, position, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(position);
    scid_eco_book_free(book);
    scid_eco_book_free(NULL);
    remove(path);
}
