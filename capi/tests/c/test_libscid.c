#include "test_libscid.h"

int
main(void)
{
    test_primitives();
    test_primitives_nullguards();
    test_movespec();
    test_movespec_nullguards();
    test_position();
    test_position_nullguards();
    test_game();
    test_game_nullguards();
    test_game_cursor();
    test_game_cursor_nullguards();
    test_game_pgn();
    test_game_pgn_nullguards();
    test_database();
    test_database_nullguards();
    test_database_filter();
    test_database_filter_nullguards();
    test_database_search();
    test_database_search_nullguards();
    test_eco();
    test_eco_nullguards();

    return 0;
}
