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
        assert(scid_database_create_memory("search_exc_tests", &database) == SCID_OK);
        assert(database != nullptr);

        add_test_game(
            database, "[Event \"F1\"]\n[White \"Kasparov\"]\n[Black \"Deep Blue\"]\n[Result "
                      "\"1-0\"]\n\n1. e4 "
                      "e5 1-0\n");
        return database;
    }

    void
    test_search_header_criteria_create_allocation_failure()
    {
        scid_search_header_criteria* criteria = nullptr;
        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_search_header_criteria_create(&criteria);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);
        if (res == SCID_OK && criteria != nullptr)
        {
            scid_search_header_criteria_free(criteria);
        }
    }

    void
    test_search_header_criteria_player_set_allocation_failure()
    {
        scid_search_header_criteria* criteria = nullptr;
        assert(scid_search_header_criteria_create(&criteria) == SCID_OK);
        assert(criteria != nullptr);

        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_search_header_criteria_player_set(criteria, "Kasparov");
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_search_header_criteria_free(criteria);
    }

    void
    test_search_board_criteria_create_allocation_failure()
    {
        scid_search_board_criteria* criteria = nullptr;
        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_search_board_criteria_create(&criteria);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);
        if (res == SCID_OK && criteria != nullptr)
        {
            scid_search_board_criteria_free(criteria);
        }
    }

    void
    test_search_board_criteria_position_set_allocation_failure()
    {
        scid_search_board_criteria* criteria = nullptr;
        assert(scid_search_board_criteria_create(&criteria) == SCID_OK);
        assert(criteria != nullptr);

        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_search_board_criteria_position_set(criteria, pos);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_position_free(pos);
        scid_search_board_criteria_free(criteria);
    }

    void
    test_database_search_headers_allocation_failure()
    {
        scid_database* database = create_test_database();

        scid_search_header_criteria* criteria = nullptr;
        assert(scid_search_header_criteria_create(&criteria) == SCID_OK);
        assert(criteria != nullptr);
        assert(scid_search_header_criteria_player_set(criteria, "Kasparov") == SCID_OK);

        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_database_search_headers(
            database, SCID_FILTER_ALL_GAMES, SCID_FILTER_PRIMARY, criteria, nullptr, nullptr,
            nullptr, nullptr);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_search_header_criteria_free(criteria);
        scid_database_free(database);
    }

    void
    test_database_search_position_allocation_failure()
    {
        scid_database* database = create_test_database();

        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_database_search_position(
            database, SCID_FILTER_ALL_GAMES, SCID_FILTER_PRIMARY, pos, nullptr, nullptr, nullptr,
            nullptr);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_position_free(pos);
        scid_database_free(database);
    }

    void
    test_database_search_board_allocation_failure()
    {
        scid_database* database = create_test_database();

        scid_search_board_criteria* criteria = nullptr;
        assert(scid_search_board_criteria_create(&criteria) == SCID_OK);
        assert(criteria != nullptr);

        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);
        assert(scid_search_board_criteria_position_set(criteria, pos) == SCID_OK);

        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_database_search_board(
            database, SCID_FILTER_ALL_GAMES, SCID_FILTER_PRIMARY, criteria, nullptr, nullptr,
            nullptr, nullptr);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_position_free(pos);
        scid_search_board_criteria_free(criteria);
        scid_database_free(database);
    }

} // namespace

void
test_database_search_exceptions()
{
    test_search_header_criteria_create_allocation_failure();
    test_search_header_criteria_player_set_allocation_failure();
    test_search_board_criteria_create_allocation_failure();
    test_search_board_criteria_position_set_allocation_failure();
    test_database_search_headers_allocation_failure();
    test_database_search_position_allocation_failure();
    test_database_search_board_allocation_failure();
}
