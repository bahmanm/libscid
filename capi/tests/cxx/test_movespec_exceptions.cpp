#include "_support/fault_injector.h"
#include "test_libscid.h"

#include "scid/scid.h"

#include <cstddef>

namespace
{

    void
    test_movespec_create_from_san_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        scid_movespec move{};
        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_movespec_create_from_san(pos, "e4", &move);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_position_free(pos);
    }

    void
    test_movespec_to_san_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        scid_movespec move{};
        assert(scid_movespec_create(12, 28, SCID_PIECE_NONE, 0, &move) == SCID_OK);

        char        buffer[16];
        std::size_t text_size = 0;

        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_movespec_to_san(pos, move, buffer, sizeof(buffer), &text_size);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_position_free(pos);
    }

} // namespace

void
test_movespec_exceptions()
{
    test_movespec_create_from_san_allocation_failure();
    test_movespec_to_san_allocation_failure();
}
