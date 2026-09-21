#include "test_libscid.h"

#include <cstdio>
#include <cstdlib>
#include <exception>

#if defined(_WIN32)
#include <crtdbg.h>
#include <windows.h>

static int
custom_report_hook(
    int   report_type,
    char* message,
    int*  return_value)
{
    std::fprintf(
        stderr, "\n=== CRT REPORT [type=%d] ===\n%s\n", report_type, message ? message : "(null)");
    std::fflush(stderr);
    if (return_value != nullptr)
    {
        *return_value = 0;
    }
    return 1;
}

static LONG WINAPI
custom_seh_filter(EXCEPTION_POINTERS* info)
{
    const DWORD code = (info && info->ExceptionRecord) ? info->ExceptionRecord->ExceptionCode : 0;
    void* const addr =
        (info && info->ExceptionRecord) ? info->ExceptionRecord->ExceptionAddress : nullptr;
    std::fprintf(
        stderr, "\n=== UNHANDLED WIN32 EXCEPTION: code=0x%08lX at address=%p ===\n", code, addr);
    std::fflush(stderr);
    return EXCEPTION_EXECUTE_HANDLER;
}

static void
disable_interactive_crash_dialogs()
{
    _CrtSetReportHook2(_CRT_RPTHOOK_INSTALL, custom_report_hook);

    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);

    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);

    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    SetUnhandledExceptionFilter(custom_seh_filter);
}
#endif

int
main()
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::setvbuf(stderr, nullptr, _IONBF, 0);

    std::set_terminate([]() {
        std::fprintf(stderr, "\n=== std::terminate called ===\n");
        const std::exception_ptr e = std::current_exception();
        if (e)
        {
            try
            {
                std::rethrow_exception(e);
            }
            catch (const std::exception& ex)
            {
                std::fprintf(stderr, "Exception what(): %s\n", ex.what());
            }
            catch (...)
            {
                std::fprintf(stderr, "Unknown non-standard exception thrown.\n");
            }
        }
        else
        {
            std::fprintf(
                stderr, "No active exception (std::current_exception() is null).\n"
                        "Cause: noexcept specification violation or throwing destructor during "
                        "unwinding.\n");
        }
        std::fflush(stderr);
        std::abort();
    });

#if defined(_WIN32)
    disable_interactive_crash_dialogs();
#endif

    std::fprintf(stderr, "[SUITE 01/11] test_support\n");
    std::fflush(stderr);
    test_support();

    std::fprintf(stderr, "[SUITE 02/11] test_primitives_exceptions\n");
    std::fflush(stderr);
    test_primitives_exceptions();

    std::fprintf(stderr, "[SUITE 03/11] test_movespec_exceptions\n");
    std::fflush(stderr);
    test_movespec_exceptions();

    std::fprintf(stderr, "[SUITE 04/11] test_position_exceptions\n");
    std::fflush(stderr);
    test_position_exceptions();

    std::fprintf(stderr, "[SUITE 05/11] test_game_exceptions\n");
    std::fflush(stderr);
    test_game_exceptions();

    std::fprintf(stderr, "[SUITE 06/11] test_game_cursor_exceptions\n");
    std::fflush(stderr);
    test_game_cursor_exceptions();

    std::fprintf(stderr, "[SUITE 07/11] test_game_pgn_exceptions\n");
    std::fflush(stderr);
    test_game_pgn_exceptions();

    std::fprintf(stderr, "[SUITE 08/11] test_eco_exceptions\n");
    std::fflush(stderr);
    test_eco_exceptions();

    std::fprintf(stderr, "[SUITE 09/11] test_database_exceptions\n");
    std::fflush(stderr);
    test_database_exceptions();

    std::fprintf(stderr, "[SUITE 10/11] test_database_filter_exceptions\n");
    std::fflush(stderr);
    test_database_filter_exceptions();

    std::fprintf(stderr, "[SUITE 11/11] test_database_search_exceptions\n");
    std::fflush(stderr);
    test_database_search_exceptions();

    std::fprintf(stderr, "[SUITE DONE] All C++ CAPI exception suites completed successfully.\n");
    std::fflush(stderr);

    return 0;
}
