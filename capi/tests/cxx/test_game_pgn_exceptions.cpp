#include "_support/fault_injector.h"
#include "test_libscid.h"

#include "scid/scid.h"

#include <cassert>
#include <cstddef>
#include <cstring>

namespace
{

    void
    test_game_pgn_options_create_allocation_failure()
    {
        scid_game_pgn_options* options = nullptr;
        scid::test::assert_allocation_resilience([&]() {
            options = scid::test::dirty_pointer<scid_game_pgn_options>();
            const scid_error res = scid_game_pgn_options_create(&options);
            if (res == SCID_ERROR_NO_MEMORY)
            {
                assert(options == nullptr);
            }
            return res;
        });

        assert(options != nullptr);
        scid_game_pgn_options_free(options);
    }

    void
    test_game_pgn_options_free_null()
    {
        scid_game_pgn_options_free(nullptr);
    }

    void
    test_game_to_pgn_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        const char* pgn = "[Event \"Test\"]\n"
                          "[Site \"London\"]\n"
                          "[Date \"2024.01.01\"]\n"
                          "[Round \"1\"]\n"
                          "[White \"White\"]\n"
                          "[Black \"Black\"]\n"
                          "[Result \"1-0\"]\n\n"
                          "1. e4 {Opening move} (1. d4 d5) e5 2. Nf3 Nc6 1-0\n";

        scid_game* game = nullptr;
        assert(scid_game_create(pos, pgn, std::strlen(pgn), &game, nullptr, 0, nullptr) == SCID_OK);
        assert(game != nullptr);

        scid_game_pgn_options* options = nullptr;
        assert(scid_game_pgn_options_create(&options) == SCID_OK);
        assert(options != nullptr);

        char        buffer[2048];
        std::size_t written_size = 0;

        scid::test::enable_allocation_failure(1);
        const scid_error res =
            scid_game_to_pgn(game, options, buffer, sizeof(buffer), &written_size);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_game_pgn_options_free(options);
        scid_game_free(game);
        scid_position_free(pos);
    }

} // namespace

void
test_game_pgn_exceptions()
{
    test_game_pgn_options_free_null();
    test_game_pgn_options_create_allocation_failure();
    test_game_to_pgn_allocation_failure();
}
