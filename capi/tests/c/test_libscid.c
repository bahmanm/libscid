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
    test_database();
    test_database_filters();
    test_database_search();
    test_eco();
    test_pgn_contract();

    return 0;
}
