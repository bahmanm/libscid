#include "_support/fault_injector.h"
#include "test_libscid.h"

#include "scid/scid.h"

#include <cassert>
#include <cstddef>
#include <cstring>

namespace
{

    void
    test_game_create_blank_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        scid_game* game = nullptr;
        scid::test::assert_allocation_resilience([&]() {
            game = scid::test::dirty_pointer<scid_game>();
            const scid_error res = scid_game_create_blank(pos, &game);
            if (res == SCID_ERROR_NO_MEMORY)
            {
                assert(game == nullptr);
            }
            return res;
        });

        assert(game != nullptr);
        scid_game_free(game);
        scid_position_free(pos);
    }

    void
    test_game_create_from_pgn_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        const char* pgn = "[Event \"Friendly Match\"]\n"
                          "[Site \"London\"]\n"
                          "[Date \"2024.01.01\"]\n"
                          "[Round \"1\"]\n"
                          "[White \"Player One\"]\n"
                          "[Black \"Player Two\"]\n"
                          "[Result \"1-0\"]\n\n"
                          "1. e4 e5 2. Nf3 Nc6 3. Bb5 a6 1-0\n";

        scid_game* game = nullptr;
        scid::test::assert_allocation_resilience([&]() {
            game = scid::test::dirty_pointer<scid_game>();
            const scid_error res =
                scid_game_create(pos, pgn, std::strlen(pgn), &game, nullptr, 0, nullptr);
            if (res == SCID_ERROR_NO_MEMORY)
            {
                assert(game == nullptr);
            }
            return res;
        });

        assert(game != nullptr);
        scid_game_free(game);
        scid_position_free(pos);
    }

    void
    test_game_tag_set_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        scid_game* game = nullptr;
        assert(scid_game_create_blank(pos, &game) == SCID_OK);
        assert(game != nullptr);

        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_game_tag_set(
            game, "Annotator",
            "Grandmaster Annotator With A Substantially Long Name Exceeding Small Buffer");
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_game_free(game);
        scid_position_free(pos);
    }

    void
    test_game_tag_at_get_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        scid_game* game = nullptr;
        assert(scid_game_create_blank(pos, &game) == SCID_OK);
        assert(game != nullptr);

        char        name[64];
        char        value[64];
        std::size_t name_size = 0;
        std::size_t value_size = 0;

        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_game_tag_at_get(
            game, 0, name, sizeof(name), &name_size, value, sizeof(value), &value_size);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_game_free(game);
        scid_position_free(pos);
    }

    void
    test_game_final_position_get_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        const char* pgn = "1. e4 e5 2. Nf3 Nc6 1-0\n";
        scid_game*  game = nullptr;
        assert(scid_game_create(pos, pgn, std::strlen(pgn), &game, nullptr, 0, nullptr) == SCID_OK);
        assert(game != nullptr);

        scid_position* final_pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &final_pos) == SCID_OK);
        assert(final_pos != nullptr);

        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_game_final_position_get(game, final_pos);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_position_free(final_pos);
        scid_game_free(game);
        scid_position_free(pos);
    }

    void
    test_game_merge_moves_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        const char* pgn1 = "1. e4 e5 *\n";
        const char* pgn2 = "1. e4 e5 2. Nf3 Nc6 *\n";

        scid_game* target_game = nullptr;
        scid_game* source_game = nullptr;
        assert(
            scid_game_create(pos, pgn1, std::strlen(pgn1), &target_game, nullptr, 0, nullptr) ==
            SCID_OK);
        assert(
            scid_game_create(pos, pgn2, std::strlen(pgn2), &source_game, nullptr, 0, nullptr) ==
            SCID_OK);
        assert(target_game != nullptr && source_game != nullptr);

        scid_game_cursor* cursor = nullptr;
        assert(scid_game_cursor_create(target_game, &cursor) == SCID_OK);
        assert(cursor != nullptr);

        scid_game_cursor* out_cursor = nullptr;
        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_game_merge_moves(
            target_game, cursor, source_game, SCID_GAME_MERGE_MOVES_INSERT_VARIATION, &out_cursor);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);
        if (res == SCID_OK && out_cursor != nullptr)
        {
            scid_game_cursor_free(out_cursor);
        }

        scid_game_cursor_free(cursor);
        scid_game_free(source_game);
        scid_game_free(target_game);
        scid_position_free(pos);
    }

    void
    test_game_free_null()
    {
        scid_game_free(nullptr);
    }

} // namespace

void
test_game_exceptions()
{
    test_game_free_null();
    test_game_create_blank_allocation_failure();
    test_game_create_from_pgn_allocation_failure();
    test_game_tag_set_allocation_failure();
    test_game_tag_at_get_allocation_failure();
    test_game_final_position_get_allocation_failure();
    test_game_merge_moves_allocation_failure();
}
