#include "_support/fault_injector.h"
#include "test_libscid.h"

#include "scid/scid.h"

#include <cstddef>

namespace
{

    void
    test_nag_conversions_allocation_failure()
    {
        char        buffer[16];
        std::size_t text_size = 0;

        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_nag_to_string(1, 0, buffer, sizeof(buffer), &text_size);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);
    }

} // namespace

void
test_primitives_exceptions()
{
    test_nag_conversions_allocation_failure();
}
