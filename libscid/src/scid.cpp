#include "scid/scid.h"

#include "scid/core/nags.h"

scid_error scid_nag_create_from_string(
    const char* text,
    scid_nag* out_nag
) {
    if (text == nullptr || out_nag == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        *out_nag = scid::core::nagCode(scid::core::nagFromString(text));
        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}
