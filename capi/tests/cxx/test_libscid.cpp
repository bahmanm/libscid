#include "test_libscid.h"

int
main()
{
    test_support();
    test_primitives_exceptions();
    test_movespec_exceptions();
    test_position_exceptions();
    test_game_exceptions();
    test_game_cursor_exceptions();
    test_game_pgn_exceptions();
    test_eco_exceptions();
    test_database_exceptions();
    test_database_filter_exceptions();
    test_database_search_exceptions();
    return 0;
}
