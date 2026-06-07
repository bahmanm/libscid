#include "scid/scid.h"

#include <assert.h>

int main(void) {
    scid_nag nag = 0;

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

    return 0;
}
