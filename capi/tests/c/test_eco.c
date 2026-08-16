#include "test_libscid.h"

#include "scid/scid.h"

#include <stdio.h>
#include <string.h>

static void
write_eco_file(const char* path)
{
    FILE* file = fopen(path, "w");
    TEST_ASSERT(file != NULL);
    fputs(
        "# Test ECO file\n"
        "A00a \"Start position\" *\n"
        "B20 \"Sicilian Defence\" 1.e4 c5 *\n"
        "C50a \"Italian Game\" 1.e4 e5 2.Nf3 Nc6 3.Bc4 *\n",
        file);
    TEST_ASSERT(fclose(file) == 0);
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

    TEST_ASSERT(scid_eco_code_from_string("B91a4", &code) == SCID_OK);
    TEST_ASSERT(code != SCID_ECO_NONE);
    TEST_ASSERT(
        scid_eco_code_to_string(code, SCID_ECO_FORMAT_BASIC, text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "B91") == 0);
    TEST_ASSERT(text_size == 3);
    TEST_ASSERT(
        scid_eco_code_to_string(code, SCID_ECO_FORMAT_EXTENDED, text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "B91a4") == 0);
    TEST_ASSERT(text_size == 5);

    TEST_ASSERT(scid_eco_code_from_string("not-eco", &code) == SCID_OK);
    TEST_ASSERT(code == SCID_ECO_NONE);
    TEST_ASSERT(
        scid_eco_code_to_string(
            SCID_ECO_NONE, SCID_ECO_FORMAT_EXTENDED, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "") == 0);
    TEST_ASSERT(text_size == 0);

    TEST_ASSERT(scid_eco_code_from_string(NULL, &code) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_eco_code_from_string("B20", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_eco_code_to_string(code, 99, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_eco_code_to_string(code, SCID_ECO_FORMAT_EXTENDED, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);

    snprintf(path, sizeof(path), "libscid_test_eco_file.eco");
    remove(path);
    write_eco_file(path);

    TEST_ASSERT(scid_eco_book_load("libscid_missing_eco_file.eco", &book) == SCID_ERROR_FILE_OPEN);
    TEST_ASSERT(book == NULL);
    TEST_ASSERT(scid_eco_book_load(path, &book) == SCID_OK);
    TEST_ASSERT(book != NULL);

    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);
    TEST_ASSERT(scid_eco_book_code_find(book, position, &code) == SCID_OK);
    TEST_ASSERT(
        scid_eco_code_to_string(code, SCID_ECO_FORMAT_EXTENDED, text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "A00a") == 0);
    TEST_ASSERT(scid_eco_book_name_find(book, position, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Start position") == 0);
    TEST_ASSERT(text_size == 14);

    TEST_ASSERT(scid_position_apply_uci(position, "e2e4") == SCID_OK);
    TEST_ASSERT(scid_position_apply_uci(position, "c7c5") == SCID_OK);
    TEST_ASSERT(scid_eco_book_code_find(book, position, &code) == SCID_OK);
    TEST_ASSERT(
        scid_eco_code_to_string(code, SCID_ECO_FORMAT_BASIC, text, sizeof(text), &text_size) ==
        SCID_OK);
    TEST_ASSERT(strcmp(text, "B20") == 0);
    TEST_ASSERT(scid_eco_book_name_find(book, position, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "Sicilian Defence") == 0);
    TEST_ASSERT(text_size == 16);

    TEST_ASSERT(scid_position_apply_uci(position, "g1f3") == SCID_OK);
    TEST_ASSERT(scid_eco_book_code_find(book, position, &code) == SCID_OK);
    TEST_ASSERT(code == SCID_ECO_NONE);
    TEST_ASSERT(scid_eco_book_name_find(book, position, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "") == 0);
    TEST_ASSERT(text_size == 0);

    TEST_ASSERT(scid_eco_book_load(NULL, &book) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_eco_book_load(path, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_eco_book_code_find(NULL, position, &code) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_eco_book_code_find(book, NULL, &code) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_eco_book_code_find(book, position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_eco_book_name_find(NULL, position, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_eco_book_name_find(book, NULL, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_eco_book_name_find(book, position, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);

    scid_position_free(position);
    scid_eco_book_free(book);
    scid_eco_book_free(NULL);
    remove(path);
}
