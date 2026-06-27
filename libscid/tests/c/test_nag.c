#include "test_libscid.h"

#include "scid/scid.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

void
test_nag(
    void)
{
    scid_nag nag = 0;
    char text[8];
    size_t text_size = 0;

    assert(scid_nag_create_from_string("!", &nag) == SCID_OK);
    assert(nag == 1);

    assert(scid_nag_create_from_string("??", &nag) == SCID_OK);
    assert(nag == 4);

    assert(scid_nag_create_from_string("$146", &nag) == SCID_OK);
    assert(nag == 146);

    assert(scid_nag_create_from_string("not-a-nag", &nag) == SCID_OK);
    assert(nag == 0);

    assert(scid_nag_create_from_string(NULL, &nag) == SCID_ERROR_BAD_ARG);
    assert(scid_nag_create_from_string("!", NULL) == SCID_ERROR_BAD_ARG);

    assert(scid_nag_to_string(1, 1, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "!") == 0);
    assert(text_size == 1);

    assert(scid_nag_to_string(1, 0, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "$1") == 0);
    assert(text_size == 2);

    assert(scid_nag_to_string(146, 1, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "N") == 0);
    assert(text_size == 1);

    assert(scid_nag_to_string(146, 0, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "$146") == 0);
    assert(text_size == 4);

    text_size = 99;
    assert(scid_nag_to_string(146, 0, NULL, 0, &text_size) == SCID_ERROR_BUFFER_FULL);
    assert(text_size == 4);

    assert(scid_nag_to_string(146, 0, text, 4, &text_size) == SCID_ERROR_BUFFER_FULL);
    assert(text_size == 4);

    assert(scid_nag_to_string(0, 1, text, sizeof(text), &text_size) == SCID_OK);
    assert(strcmp(text, "") == 0);
    assert(text_size == 0);

    assert(scid_nag_to_string(1, 1, text, sizeof(text), NULL) == SCID_ERROR_BAD_ARG);
}
