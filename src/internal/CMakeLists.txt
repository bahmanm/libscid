cmake_minimum_required( VERSION 3.28 )

if( DEFINED LIBSCID_VERSION AND LIBSCID_VERSION MATCHES "^[0-9]+(\\.[0-9]+)?(\\.[0-9]+)?(\\.[0-9]+)?$" )
    unset( LIBSCID_VERSION CACHE )
    unset( LIBSCID_VERSION )
endif()
set( LIBSCID_PROJECT_VERSION "0.0.0" CACHE STRING "Numeric libscid version used for CMake package compatibility." )

project(
    libscid-internal
    VERSION "${LIBSCID_PROJECT_VERSION}"
    LANGUAGES CXX )

include( "${LIBSCID_SOURCE_ROOT}/cmake/project.cmake" )

add_subdirectory( core )
add_subdirectory( eco )
add_subdirectory( database )
add_subdirectory( spelling )
