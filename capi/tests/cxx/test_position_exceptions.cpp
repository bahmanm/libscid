#include "_support/fault_injector.h"
#include "test_libscid.h"

#include "scid/scid.h"

#include <cassert>
#include <cstddef>

namespace
{

    void
    test_position_create_from_fen_allocation_failure()
    {
        scid_position* pos = nullptr;

        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_position_create_from_fen(
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);
        if (res == SCID_OK)
        {
            scid_position_free(pos);
        }
    }

    void
    test_position_create_with_san_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        scid_position* next_pos = nullptr;
        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_position_create_with_san(pos, "e4", &next_pos);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);
        if (res == SCID_OK)
        {
            scid_position_free(next_pos);
        }

        scid_position_free(pos);
    }

    void
    test_position_create_with_uci_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        scid_position* next_pos = nullptr;
        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_position_create_with_uci(pos, "e2e4", &next_pos);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);
        if (res == SCID_OK)
        {
            scid_position_free(next_pos);
        }

        scid_position_free(pos);
    }

    void
    test_position_apply_san_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_position_apply_san(pos, "e4");
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_position_free(pos);
    }

    void
    test_position_apply_uci_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_position_apply_uci(pos, "e2e4");
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_position_free(pos);
    }

    void
    test_position_legal_moves_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        scid_movespec moves[SCID_MAX_LEGAL_MOVES];
        std::size_t   move_count = 0;

        scid::test::enable_allocation_failure(1);
        const scid_error res =
            scid_position_legal_moves(pos, moves, sizeof(moves) / sizeof(moves[0]), &move_count);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_position_free(pos);
    }

} // namespace

void
test_position_exceptions()
{
    test_position_create_from_fen_allocation_failure();
    test_position_create_with_san_allocation_failure();
    test_position_create_with_uci_allocation_failure();
    test_position_apply_san_allocation_failure();
    test_position_apply_uci_allocation_failure();
    test_position_legal_moves_allocation_failure();
}
