if( NOT DEFINED LIBSCID_SOURCE_ROOT )
    message( FATAL_ERROR "LIBSCID_SOURCE_ROOT must point at the repository root." )
endif()

get_filename_component( LIBSCID_SOURCE_ROOT "${LIBSCID_SOURCE_ROOT}" REALPATH )

include( GNUInstallDirs )
include( "${LIBSCID_SOURCE_ROOT}/etc/cmake/test/runtime-dlls.cmake" )
include( "${LIBSCID_SOURCE_ROOT}/etc/cmake/qc/sanitisers.cmake" )

set( _libscid_default_version "snapshot" )
find_package( Git QUIET )
if( Git_FOUND )
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${LIBSCID_SOURCE_ROOT}" rev-parse --short=12 HEAD
        OUTPUT_VARIABLE _libscid_git_sha
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE )
    if( NOT _libscid_git_sha STREQUAL "" )
        set( _libscid_default_version "snapshot+${_libscid_git_sha}" )
    endif()
endif()
set( LIBSCID_VERSION "${_libscid_default_version}" CACHE STRING "Human-readable libscid version label." )

option( LIBSCID_INSTALL "Install libscid targets and CMake package files." "${PROJECT_IS_TOP_LEVEL}" )
option( LIBSCID_BUILD_DOCS "Build libscid API documentation." OFF )

if( PROJECT_IS_TOP_LEVEL )
    set(
        CMAKE_EXPORT_COMPILE_COMMANDS
        ON
        CACHE BOOL "Generate compile_commands.json for static analysis tools."
        FORCE )

    if( NOT DEFINED BUILD_TESTING )
        set( BUILD_TESTING OFF CACHE BOOL "Build tests" )
    endif()
    include( CTest )
    include( "${LIBSCID_SOURCE_ROOT}/etc/cmake/qc/clang-tidy.cmake" )
    include( "${LIBSCID_SOURCE_ROOT}/etc/cmake/qc/cppcheck.cmake" )
    include( "${LIBSCID_SOURCE_ROOT}/etc/cmake/qc/format.cmake" )
    include( "${LIBSCID_SOURCE_ROOT}/etc/cmake/qc/include-cleaner.cmake" )
endif()
