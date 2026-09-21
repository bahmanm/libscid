#ifndef LIBSCID_TEST_CXX_LIBSCID_H
#define LIBSCID_TEST_CXX_LIBSCID_H

#include <cassert>
#include <cstdio>
#include <cstdlib>

#define TEST_ASSERT(expr)                                                                          \
    do                                                                                             \
    {                                                                                              \
        if (!(expr))                                                                               \
        {                                                                                          \
            std::fprintf(                                                                          \
                stderr,                                                                            \
                "TEST ASSERTION FAILED: %s\n"                                                      \
                "  File: %s:%d\n"                                                                  \
                "  Function: %s()\n",                                                              \
                #expr, __FILE__, __LINE__, __func__);                                              \
            std::fflush(stderr);                                                                   \
            std::abort();                                                                          \
        }                                                                                          \
    } while (0)

#undef assert
#define assert(expr) TEST_ASSERT(expr)

void
test_support();

void
test_primitives_exceptions();

void
test_movespec_exceptions();

void
test_position_exceptions();

void
test_game_exceptions();

void
test_game_cursor_exceptions();

void
test_game_pgn_exceptions();

void
test_eco_exceptions();

void
test_database_exceptions();

void
test_database_filter_exceptions();

void
test_database_search_exceptions();

#endif // LIBSCID_TEST_CXX_LIBSCID_H
