#include "test_libscid.h"

int main(void) {
    test_square();
    test_piece();
    test_movespec();
    test_nag();
    test_position();
    test_game();
    test_cursor();
    test_cursor_mutation();
    test_database();
    test_eco();
    test_pgn_contract();

    return 0;
}
