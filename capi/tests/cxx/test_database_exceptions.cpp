#include "_support/fault_injector.h"
#include "test_libscid.h"

#include "scid/scid.h"

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>

namespace
{

    void
    remove_scid5_files(const char* base_path)
    {
        const std::string path(base_path);
        std::remove((path + ".si5").c_str());
        std::remove((path + ".sg5").c_str());
        std::remove((path + ".sn5").c_str());
    }

    void
    test_database_create_memory_allocation_failure()
    {
        scid_database* db = nullptr;
        scid::test::assert_allocation_resilience([&]() {
            db = scid::test::dirty_pointer<scid_database>();
            const scid_error res = scid_database_create_memory("memdb_alloc_fail", &db);
            if (res == SCID_ERROR_NO_MEMORY)
            {
                assert(db == nullptr);
            }
            return res;
        });

        assert(db != nullptr);
        scid_database_free(db);
    }

    void
    test_database_create_scid5_allocation_failure()
    {
        const char* path = "test_scid5_create_alloc_fail";
        remove_scid5_files(path);

        scid_database* db = nullptr;
        scid::test::assert_allocation_resilience([&]() {
            db = scid::test::dirty_pointer<scid_database>();
            const scid_error res = scid_database_create_scid5(path, &db);
            if (res == SCID_ERROR_NO_MEMORY)
            {
                assert(db == nullptr);
            }
            return res;
        });

        assert(db != nullptr);
        scid_database_free(db);
        remove_scid5_files(path);
    }

    void
    test_database_free_null()
    {
        scid_database_free(nullptr);
    }

    void
    test_database_game_add_allocation_failure()
    {
        scid_database* db = nullptr;
        assert(scid_database_create_memory("memdb_game_add", &db) == SCID_OK);
        assert(db != nullptr);

        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        const char* pgn = "[Event \"Test\"]\n"
                          "[White \"Kasparov\"]\n"
                          "[Black \"Deep Blue\"]\n"
                          "[Result \"1-0\"]\n\n"
                          "1. e4 e5 2. Nf3 Nc6 1-0\n";
        scid_game*  game = nullptr;
        assert(scid_game_create(pos, pgn, std::strlen(pgn), &game, nullptr, 0, nullptr) == SCID_OK);
        assert(game != nullptr);

        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_database_game_add(db, game, "W");
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_game_free(game);
        scid_position_free(pos);
        scid_database_free(db);
    }

    void
    test_database_game_get_allocation_failure()
    {
        scid_database* db = nullptr;
        assert(scid_database_create_memory("memdb_game_get", &db) == SCID_OK);
        assert(db != nullptr);

        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        const char* pgn = "[Event \"Test\"]\n"
                          "[White \"Kasparov\"]\n"
                          "[Black \"Deep Blue\"]\n"
                          "[Result \"1-0\"]\n\n"
                          "1. e4 e5 2. Nf3 Nc6 1-0\n";
        scid_game*  game = nullptr;
        assert(scid_game_create(pos, pgn, std::strlen(pgn), &game, nullptr, 0, nullptr) == SCID_OK);
        assert(game != nullptr);
        assert(scid_database_game_add(db, game, "W") == SCID_OK);
        scid_game_free(game);
        scid_position_free(pos);

        scid_game* retrieved = nullptr;
        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_database_game_get(db, 0, &retrieved, nullptr, 0, nullptr);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);
        if (res == SCID_OK && retrieved != nullptr)
        {
            scid_game_free(retrieved);
        }

        scid_database_free(db);
    }

    void
    test_database_game_tag_get_allocation_failure()
    {
        scid_database* db = nullptr;
        assert(scid_database_create_memory("memdb_tag_get", &db) == SCID_OK);
        assert(db != nullptr);

        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        const char* pgn = "[Event \"Test\"]\n"
                          "[White \"Kasparov\"]\n"
                          "[Black \"Deep Blue\"]\n"
                          "[Result \"1-0\"]\n\n"
                          "1. e4 e5 2. Nf3 Nc6 1-0\n";
        scid_game*  game = nullptr;
        assert(scid_game_create(pos, pgn, std::strlen(pgn), &game, nullptr, 0, nullptr) == SCID_OK);
        assert(game != nullptr);
        assert(scid_database_game_add(db, game, "W") == SCID_OK);
        scid_game_free(game);
        scid_position_free(pos);

        char        text[128];
        std::size_t size = 0;
        scid::test::enable_allocation_failure(1);
        const scid_error res =
            scid_database_game_tag_get(db, 0, "White", text, sizeof(text), &size);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_database_free(db);
    }

    void
    test_database_import_pgn_allocation_failure()
    {
        scid_database* db = nullptr;
        assert(scid_database_create_memory("memdb_import_pgn", &db) == SCID_OK);
        assert(db != nullptr);

        const char* pgn = "[Event \"Imported\"]\n"
                          "[White \"Player 1\"]\n"
                          "[Black \"Player 2\"]\n"
                          "[Result \"*\"]\n\n"
                          "1. e4 e5 2. Nf3 Nc6 *\n";

        std::size_t      imported = 0;
        const scid_error res =
            scid_database_import_pgn(db, pgn, std::strlen(pgn), nullptr, 0, nullptr, &imported);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_database_free(db);
    }

} // namespace

void
test_database_exceptions()
{
    test_database_free_null();
    test_database_create_memory_allocation_failure();
    test_database_create_scid5_allocation_failure();
    test_database_game_add_allocation_failure();
    test_database_game_get_allocation_failure();
    test_database_game_tag_get_allocation_failure();
    test_database_import_pgn_allocation_failure();
}
