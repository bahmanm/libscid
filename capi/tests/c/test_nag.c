#include "test_libscid.h"

#include "scid/scid.h"

#include <stddef.h>
#include <string.h>

void
test_nag(void)
{
    scid_nag nag = 0;
    char     text[8];
    size_t   text_size = 0;

    TEST_ASSERT(scid_nag_create_from_string("!", &nag) == SCID_OK);
    TEST_ASSERT(nag == 1);

    TEST_ASSERT(scid_nag_create_from_string("??", &nag) == SCID_OK);
    TEST_ASSERT(nag == 4);

    TEST_ASSERT(scid_nag_create_from_string("$146", &nag) == SCID_OK);
    TEST_ASSERT(nag == 146);

    TEST_ASSERT(scid_nag_create_from_string("not-a-nag", &nag) == SCID_OK);
    TEST_ASSERT(nag == 0);

    TEST_ASSERT(scid_nag_create_from_string(NULL, &nag) == SCID_ERROR_BAD_ARG);
    TEST_ASSERT(scid_nag_create_from_string("!", NULL) == SCID_ERROR_BAD_ARG);

    TEST_ASSERT(scid_nag_to_string(1, 1, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "!") == 0);
    TEST_ASSERT(text_size == 1);

    TEST_ASSERT(scid_nag_to_string(1, 0, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "$1") == 0);
    TEST_ASSERT(text_size == 2);

    TEST_ASSERT(scid_nag_to_string(146, 1, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "N") == 0);
    TEST_ASSERT(text_size == 1);

    TEST_ASSERT(scid_nag_to_string(146, 0, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "$146") == 0);
    TEST_ASSERT(text_size == 4);

    text_size = 99;
    TEST_ASSERT(scid_nag_to_string(146, 0, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == 4);

    TEST_ASSERT(scid_nag_to_string(146, 0, text, 4, &text_size) == SCID_ERROR_BUFFER_FULL);
    TEST_ASSERT(text_size == 4);

    TEST_ASSERT(scid_nag_to_string(0, 1, text, sizeof(text), &text_size) == SCID_OK);
    TEST_ASSERT(strcmp(text, "") == 0);
    TEST_ASSERT(text_size == 0);

    TEST_ASSERT(scid_nag_to_string(1, 1, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
}
