#include "test_libscid.h"

#include "scid/scid.h"

#include <stddef.h>
#include <string.h>

static void
test_square_from_string_nullguards(void)
{
    scid_square square = 0;

    TEST_ASSERT(scid_square_from_string(NULL, &square) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_square_from_string("e4", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_square_from_string(NULL, NULL) == SCID_ERROR_BAD_ARG);
}

static void
test_square_to_string_nullguards(void)
{
    char   buffer[16];
    size_t text_size = 0;

    TEST_ASSERT(scid_square_to_string(28, buffer, sizeof(buffer), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_square_to_string(28, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_square_to_string(28, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == 2);
}

static void
test_piece_type_from_string_nullguards(void)
{
    scid_piece piece = SCID_PIECE_NONE;

    TEST_ASSERT(scid_piece_type_from_string(NULL, &piece) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_piece_type_from_string("Q", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_piece_type_from_string(NULL, NULL) == SCID_ERROR_BAD_ARG);
}

static void
test_nag_create_from_string_nullguards(void)
{
    scid_nag nag = 0;

    TEST_ASSERT(scid_nag_create_from_string(NULL, &nag) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_nag_create_from_string("!", NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_nag_create_from_string(NULL, NULL) == SCID_ERROR_BAD_ARG);
}

static void
test_nag_to_string_nullguards(void)
{
    char   buffer[16];
    size_t text_size = 0;

    TEST_ASSERT(scid_nag_to_string(1, 0, buffer, sizeof(buffer), NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_nag_to_string(1, 0, NULL, 0, NULL) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_nag_to_string(1, 0, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == 2);
}

void
test_primitives_nullguards(void)
{
    test_square_from_string_nullguards();
    test_square_to_string_nullguards();
    test_piece_type_from_string_nullguards();
    test_nag_create_from_string_nullguards();
    test_nag_to_string_nullguards();
}
