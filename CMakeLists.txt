cmake_minimum_required( VERSION 3.28 )

if( DEFINED LIBSCID_VERSION AND LIBSCID_VERSION MATCHES "^[0-9]+(\\.[0-9]+)?(\\.[0-9]+)?(\\.[0-9]+)?$" )
    unset( LIBSCID_VERSION CACHE )
    unset( LIBSCID_VERSION )
endif()
set( LIBSCID_PROJECT_VERSION "0.0.0" CACHE STRING "Numeric libscid version used for CMake package compatibility." )

project(
    libscid
    VERSION "${LIBSCID_PROJECT_VERSION}"
    LANGUAGES CXX )

include( GNUInstallDirs )
include( cmake/test/runtime-dlls.cmake )

set( LIBSCID_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" )

set( _libscid_default_version "snapshot" )
find_package( Git QUIET )
if( Git_FOUND )
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_CURRENT_SOURCE_DIR}" rev-parse --short=12 HEAD
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
    include( cmake/cppcheck.cmake )
    include( cmake/format.cmake )
endif()

add_subdirectory( src/internal/core )
add_subdirectory( src/internal/eco )
add_subdirectory( src/internal/database )
add_subdirectory( src/internal/spelling )
add_subdirectory( src/libscid )

if( LIBSCID_BUILD_DOCS )
    include( cmake/docs.cmake )
endif()

if( LIBSCID_INSTALL )
    include( cmake/install/main.cmake )
    include( cmake/packaging.cmake )
endif()
