cmake_minimum_required( VERSION 3.23 )

set( LIBSCID_VERSION "0.0.0" CACHE STRING "libscid package version." )

project(
    libscid
    VERSION "${LIBSCID_VERSION}"
    LANGUAGES CXX )

include( GNUInstallDirs )

option( LIBSCID_INSTALL "Install libscid targets and CMake package files." "${PROJECT_IS_TOP_LEVEL}" )

if( PROJECT_IS_TOP_LEVEL )
    if( NOT DEFINED BUILD_TESTING )
        set( BUILD_TESTING OFF CACHE BOOL "Build tests" )
    endif()
    include( CTest )
endif()

add_subdirectory( libs/core )
add_subdirectory( libs/eco )
add_subdirectory( libs/database )
add_subdirectory( libs/spelling )

if( LIBSCID_INSTALL )
    include( cmake/install/main.cmake )
    include( cmake/packaging.cmake )
endif()
