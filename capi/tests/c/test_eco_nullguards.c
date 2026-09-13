#include "test_libscid.h"

#include "scid/scid.h"

#include <stdio.h>
#include <string.h>

static void
write_temp_eco_file(const char* path)
{
    FILE* file = fopen(path, "w");
    TEST_ASSERT(file != NULL);
    fputs(
        "# Test ECO file\n"
        "A00a \"Start position\" *\n",
        file);
    TEST_ASSERT(fclose(file) == 0);
}

static void
test_eco_code_nullguards(void)
{
    char          text[32];
    scid_eco_code code = 0;
    size_t        text_size = 0;

    TEST_ASSERT(scid_eco_code_from_string(NULL, &code) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_eco_code_from_string("B20", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_eco_code_from_string(NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_eco_code_from_string("B20", &code) == SCID_OK);
    TEST_ASSERT(
        scid_eco_code_to_string(code, 99, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_eco_code_to_string(code, -1, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_eco_code_to_string(code, SCID_ECO_FORMAT_BASIC, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_eco_code_to_string(code, SCID_ECO_FORMAT_BASIC, NULL, 0, &text_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == 3);
    TEST_ASSERT(
        scid_eco_code_to_string(code, SCID_ECO_FORMAT_BASIC, text, 1, &text_size) ==
        SCID_ERROR_BUFFER_FULL);
}

static void
test_eco_book_lifecycle_nullguards(void)
{
    scid_eco_book* book = NULL;

    TEST_ASSERT(scid_eco_book_load(NULL, &book) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_eco_book_load("missing.eco", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_eco_book_load(NULL, NULL) == SCID_ERROR_BAD_ARG);

    scid_eco_book_free(NULL);
}

static void
test_eco_book_lookup_nullguards(void)
{
    char           path[128];
    char           text[32];
    scid_eco_book* book = NULL;
    scid_eco_code  code = 0;
    scid_position* position = NULL;
    size_t         text_size = 0;

    snprintf(path, sizeof(path), "libscid_temp_eco_nullguards.eco");
    remove(path);
    write_temp_eco_file(path);

    TEST_ASSERT(scid_eco_book_load(path, &book) == SCID_OK);
    TEST_ASSERT(book != NULL);
    TEST_ASSERT(test_position_create_standard(&position) == SCID_OK);

    TEST_ASSERT(scid_eco_book_code_find(NULL, position, &code) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_eco_book_code_find(book, NULL, &code) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_eco_book_code_find(book, position, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_eco_book_code_find(NULL, NULL, NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_eco_book_name_find(NULL, position, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_eco_book_name_find(book, NULL, text, sizeof(text), &text_size) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_eco_book_name_find(book, position, text, sizeof(text), NULL) ==
        SCID_ERROR_BAD_ARG);
    TEST_ASSERT(
        scid_eco_book_name_find(NULL, NULL, NULL, 0, NULL) ==
        SCID_ERROR_BAD_ARG);

    TEST_ASSERT(
        scid_eco_book_name_find(book, position, NULL, 0, &text_size) ==
        SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size > 0);
    TEST_ASSERT(
        scid_eco_book_name_find(book, position, text, 1, &text_size) ==
        SCID_ERROR_BUFFER_FULL);

    scid_position_free(position);
    scid_eco_book_free(book);
    remove(path);
}

void
test_eco_nullguards(void)
{
    test_eco_code_nullguards();
    test_eco_book_lifecycle_nullguards();
    test_eco_book_lookup_nullguards();
}
