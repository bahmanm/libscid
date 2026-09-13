#include "test_libscid.h"

int
main(void)
{
    test_primitives();
    test_primitives_nullguards();
    test_movespec();
    test_position();
    test_game();
    test_cursor();
    test_cursor_mutation();
    test_database();
    test_database_filters();
    test_database_search();
    test_eco();
    test_pgn_contract();

    return 0;
}
