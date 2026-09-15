#include "_support/fault_injector.h"
#include "test_libscid.h"

#include "scid/scid.h"

#include <cassert>
#include <cstddef>
#include <cstring>

namespace
{

    void
    add_test_game(
        scid_database* database,
        const char*    pgn)
    {
        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        scid_game* game = nullptr;
        assert(scid_game_create(pos, pgn, std::strlen(pgn), &game, nullptr, 0, nullptr) == SCID_OK);
        assert(game != nullptr);
        assert(scid_database_game_add(database, game, nullptr) == SCID_OK);

        scid_game_free(game);
        scid_position_free(pos);
    }

    scid_database*
    create_test_database()
    {
        scid_database* database = nullptr;
        assert(scid_database_create_memory("filter_exc_tests", &database) == SCID_OK);
        assert(database != nullptr);

        add_test_game(
            database,
            "[Event \"F1\"]\n[White \"A\"]\n[Black \"B\"]\n[Result \"1-0\"]\n\n1. e4 e5 1-0\n");
        add_test_game(
            database,
            "[Event \"F2\"]\n[White \"C\"]\n[Black \"D\"]\n[Result \"0-1\"]\n\n1. d4 d5 0-1\n");
        return database;
    }

    void
    test_database_filter_create_allocation_failure()
    {
        scid_database* database = create_test_database();

        scid_filter_id filter_id = 0;
        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_database_filter_create(database, &filter_id);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);
        if (res == SCID_OK)
        {
            scid_database_filter_delete(database, filter_id);
        }

        scid_database_free(database);
    }

    void
    test_database_filter_game_indices_get_allocation_failure()
    {
        scid_database* database = create_test_database();

        std::size_t game_indices[2] = {0, 0};
        std::size_t count = 0;
        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_database_filter_game_indices_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, 2, game_indices, 2, &count);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_database_free(database);
    }

    void
    test_database_filter_game_index_at_row_get_allocation_failure()
    {
        scid_database* database = create_test_database();

        std::size_t game_index = 0;
        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_database_filter_game_index_at_row_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, &game_index);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_database_free(database);
    }

    void
    test_database_filter_game_row_for_index_get_allocation_failure()
    {
        scid_database* database = create_test_database();

        std::size_t row = 0;
        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_database_filter_game_row_for_index_get(
            database, SCID_FILTER_ALL_GAMES, "d+", 0, &row);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_database_free(database);
    }

} // namespace

void
test_database_filter_exceptions()
{
    test_database_filter_create_allocation_failure();
    test_database_filter_game_indices_get_allocation_failure();
    test_database_filter_game_index_at_row_get_allocation_failure();
    test_database_filter_game_row_for_index_get_allocation_failure();
}
