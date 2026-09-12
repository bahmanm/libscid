cmake_minimum_required( VERSION 3.28 )

include( "${CMAKE_CURRENT_SOURCE_DIR}/../etc/cmake/version.cmake" )

project(
    libscid-internal
    VERSION "${LIBSCID_PROJECT_VERSION}"
    LANGUAGES CXX )

include( "${LIBSCID_SOURCE_ROOT}/etc/cmake/project.cmake" )

add_subdirectory( core )
add_subdirectory( eco )
add_subdirectory( database )
add_subdirectory( spelling )
