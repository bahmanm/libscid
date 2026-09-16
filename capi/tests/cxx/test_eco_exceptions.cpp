#include "_support/fault_injector.h"
#include "test_libscid.h"

#include "scid/scid.h"

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>

namespace
{

    void
    write_test_eco_file(const char* path)
    {
        std::FILE* file = std::fopen(path, "w");
        assert(file != nullptr);
        std::fputs(
            "# Test ECO file\n"
            "A00a \"Start position\" *\n"
            "B20 \"Sicilian Defence\" 1.e4 c5 *\n"
            "C50a \"Italian Game\" 1.e4 e5 2.Nf3 Nc6 3.Bc4 *\n",
            file);
        assert(std::fclose(file) == 0);
    }

    void
    test_eco_code_to_string_allocation_failure()
    {
        scid_eco_code code = 0;
        assert(scid_eco_code_from_string("B91a4", &code) == SCID_OK);

        char        text[32];
        std::size_t text_size = 0;
        scid::test::enable_allocation_failure(1);
        const scid_error res =
            scid_eco_code_to_string(code, SCID_ECO_FORMAT_EXTENDED, text, sizeof(text), &text_size);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);
    }

    void
    test_eco_book_load_allocation_failure()
    {
        const char* path = "test_eco_load_alloc_fail.eco";
        write_test_eco_file(path);

        scid_eco_book* book = nullptr;
        scid::test::assert_allocation_resilience([&]() {
            book = scid::test::dirty_pointer<scid_eco_book>();
            const scid_error res = scid_eco_book_load(path, &book);
            if (res == SCID_ERROR_NO_MEMORY)
            {
                assert(book == nullptr);
            }
            return res;
        });

        assert(book != nullptr);
        scid_eco_book_free(book);
        std::remove(path);
    }

    void
    test_eco_book_free_null()
    {
        scid_eco_book_free(nullptr);
    }

    void
    test_eco_book_name_find_allocation_failure()
    {
        const char* path = "test_eco_find_alloc_fail.eco";
        write_test_eco_file(path);

        scid_eco_book* book = nullptr;
        assert(scid_eco_book_load(path, &book) == SCID_OK);
        assert(book != nullptr);

        scid_position* pos = nullptr;
        assert(
            scid_position_create_from_fen(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &pos) == SCID_OK);
        assert(pos != nullptr);

        char        text[128];
        std::size_t text_size = 0;

        scid::test::enable_allocation_failure(1);
        const scid_error res = scid_eco_book_name_find(book, pos, text, sizeof(text), &text_size);
        scid::test::disable_allocation_failure();

        assert(res == SCID_OK || res == SCID_ERROR_NO_MEMORY);

        scid_position_free(pos);
        scid_eco_book_free(book);
        std::remove(path);
    }

} // namespace

void
test_eco_exceptions()
{
    test_eco_book_free_null();
    test_eco_code_to_string_allocation_failure();
    test_eco_book_load_allocation_failure();
    test_eco_book_name_find_allocation_failure();
}
