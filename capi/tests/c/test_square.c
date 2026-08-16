#include "test_libscid.h"

#include "scid/scid.h"

#include <stddef.h>
#include <string.h>

void
test_square(void)
{
    scid_square square = 0;
    char        text[8];
    size_t      text_size = 0;

    TEST_ASSERT(scid_square_from_string("a1", &square) == SCID_OK);
    TEST_ASSERT(square == 0);

    TEST_ASSERT(scid_square_from_string("E4", &square) == SCID_OK);
    TEST_ASSERT(square == 28);

    TEST_ASSERT(scid_square_from_string("h8", &square) == SCID_OK);
    TEST_ASSERT(square == 63);

    TEST_ASSERT(scid_square_from_string("i4", &square) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_square_from_string("a9", &square) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_square_from_string("a10", &square) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_square_from_string(NULL, &square) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_square_from_string("a1", NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_square_to_string(28, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "e4") == 0);
    TEST_ASSERT(text_size == 2);

    TEST_ASSERT(scid_square_to_string(28, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == 2);

    TEST_ASSERT(scid_square_to_string(28, text, 2, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == 2);

    TEST_ASSERT(scid_square_to_string(64, text, sizeof(text), &text_size) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_square_to_string(28, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
}
