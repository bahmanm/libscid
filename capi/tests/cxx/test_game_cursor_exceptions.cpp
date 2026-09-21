#include "_support/fault_injector.h"
#include "test_libscid.h"

#include "scid/scid.h"

#include <cstddef>
#include <cstring>

namespace
{

    void
    test_game_cursor_create_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        scid_game* game = nullptr;
        assert(scid_game_create_blank(pos, &game) == SCID_OK);
        assert(game != nullptr);

        scid_game_cursor* cursor = nullptr;
        scid::test::assert_allocation_resilience([&]() {
            cursor = scid::test::dirty_pointer<scid_game_cursor>();
            const scid_error res = scid_game_cursor_create(game, &cursor);
            if (res == SCID_ERROR_NO_MEMORY)
            {
                assert(cursor == nullptr);
            }
            return res;
        });

        assert(cursor != nullptr);
        scid_game_cursor_free(cursor);
        scid_game_free(game);
        scid_position_free(pos);
    }

    void
    test_game_cursor_clone_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        scid_game* game = nullptr;
        assert(scid_game_create_blank(pos, &game) == SCID_OK);
        assert(game != nullptr);

        scid_game_cursor* cursor = nullptr;
        assert(scid_game_cursor_create(game, &cursor) == SCID_OK);
        assert(cursor != nullptr);

        scid_game_cursor* clone = nullptr;
        scid::test::assert_allocation_resilience([&]() {
            clone = scid::test::dirty_pointer<scid_game_cursor>();
            const scid_error res = scid_game_cursor_clone(game, cursor, &clone);
            if (res == SCID_ERROR_NO_MEMORY)
            {
                assert(clone == nullptr);
            }
            return res;
        });

        assert(clone != nullptr);
        scid_game_cursor_free(clone);
        scid_game_cursor_free(cursor);
        scid_game_free(game);
        scid_position_free(pos);
    }

    void
    test_game_cursor_navigation_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        const char* pgn = "[Event \"Test\"]\n"
                          "[Result \"*\"]\n\n"
                          "1. e4 (1. d4 d5) e5 *\n";
        scid_game*  game = nullptr;
        assert(scid_game_create(pos, pgn, std::strlen(pgn), &game, nullptr, 0, nullptr) == SCID_OK);
        assert(game != nullptr);

        scid_game_cursor* cursor = nullptr;
        assert(scid_game_cursor_create(game, &cursor) == SCID_OK);
        assert(cursor != nullptr);

        int               moved = 0;
        scid_game_cursor* next = nullptr;
        scid::test::enable_allocation_failure(1);
        scid_error res = scid_game_cursor_next(cursor, &moved, &next);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);
        if (res == SCID_OK && next != nullptr)
        {
            scid_game_cursor_free(next);
            next = nullptr;
        }

        scid_game_cursor* end_cursor = nullptr;
        scid::test::enable_allocation_failure(1);
        res = scid_game_cursor_to_end(cursor, &end_cursor);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);
        if (res == SCID_OK && end_cursor != nullptr)
        {
            scid_game_cursor_free(end_cursor);
            end_cursor = nullptr;
        }

        int               entered = 0;
        scid_game_cursor* var_cursor = nullptr;
        scid::test::enable_allocation_failure(1);
        res = scid_game_cursor_variation_enter(cursor, 0, &entered, &var_cursor);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);
        if (res == SCID_OK && var_cursor != nullptr)
        {
            scid_game_cursor_free(var_cursor);
            var_cursor = nullptr;
        }

        scid_game_cursor_free(cursor);
        scid_game_free(game);
        scid_position_free(pos);
    }

    void
    test_game_cursor_comment_set_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        scid_game* game = nullptr;
        assert(scid_game_create_blank(pos, &game) == SCID_OK);
        assert(game != nullptr);

        scid_game_cursor* cursor = nullptr;
        assert(scid_game_cursor_create(game, &cursor) == SCID_OK);
        assert(cursor != nullptr);

        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_game_cursor_comment_set(
            game, cursor,
            "An elaborate and comprehensive commentary text intentionally extending beyond small "
            "string optimisations");
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_game_cursor_free(cursor);
        scid_game_free(game);
        scid_position_free(pos);
    }

    void
    test_game_cursor_move_add_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        scid_movespec move{};
        assert(scid_movespec_create_from_san(pos, "e4", &move) == SCID_OK);

        scid_game* game = nullptr;
        assert(scid_game_create_blank(pos, &game) == SCID_OK);
        assert(game != nullptr);

        scid_game_cursor* cursor = nullptr;
        assert(scid_game_cursor_create(game, &cursor) == SCID_OK);
        assert(cursor != nullptr);

        scid_game_cursor* next = nullptr;
        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_game_cursor_move_add(game, cursor, move, &next);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);
        if (res == SCID_OK && next != nullptr)
        {
            scid_game_cursor_free(next);
        }

        scid_game_cursor_free(cursor);
        scid_game_free(game);
        scid_position_free(pos);
    }

    void
    test_game_cursor_tree_mutation_allocation_failure()
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        const char* pgn = "1. e4 e5 2. Nf3 Nc6 *\n";
        scid_game*  game = nullptr;
        assert(scid_game_create(pos, pgn, std::strlen(pgn), &game, nullptr, 0, nullptr) == SCID_OK);
        assert(game != nullptr);

        scid_game_cursor* cursor = nullptr;
        assert(scid_game_cursor_create(game, &cursor) == SCID_OK);
        assert(cursor != nullptr);

        int               added = 0;
        scid_game_cursor* var_cursor = nullptr;
        scid::test::enable_allocation_failure(1);
        scid_error res = scid_game_cursor_variation_add(game, cursor, nullptr, &added, &var_cursor);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);
        if (res == SCID_OK && var_cursor != nullptr)
        {
            scid_game_cursor_free(var_cursor);
            var_cursor = nullptr;
        }

        scid_game_cursor* next = nullptr;
        scid::test::enable_allocation_failure(1);
        res = scid_game_cursor_truncate(game, cursor, &next);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);
        if (res == SCID_OK && next != nullptr)
        {
            scid_game_cursor_free(next);
            next = nullptr;
        }

        scid_game_cursor_free(cursor);
        scid_game_free(game);
        scid_position_free(pos);
    }

    void
    test_game_cursor_free_null()
    {
        scid_game_cursor_free(nullptr);
    }

} // namespace

void
test_game_cursor_exceptions()
{
    test_game_cursor_free_null();
    test_game_cursor_create_allocation_failure();
    test_game_cursor_clone_allocation_failure();
    test_game_cursor_navigation_allocation_failure();
    test_game_cursor_comment_set_allocation_failure();
    test_game_cursor_move_add_allocation_failure();
    test_game_cursor_tree_mutation_allocation_failure();
}
