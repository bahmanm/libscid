#include "test_libscid.h"

#if defined(_WIN32)
#include <crtdbg.h>
#include <cstdlib>
#include <windows.h>

static void
disable_interactive_crash_dialogs()
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
main()
{
#if defined(_WIN32)
    disable_interactive_crash_dialogs();
#endif
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
