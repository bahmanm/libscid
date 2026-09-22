#include "test_libscid.h"

#if defined(_WIN32)
#include <crtdbg.h>
#include <stdlib.h>
#include <windows.h>

static void
disable_interactive_crash_dialogs(void)
{
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);

    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);

    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
}
#endif

int
main(void)
{
#if defined(_WIN32)
    disable_interactive_crash_dialogs();
#endif
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
