cmake_minimum_required( VERSION 3.23 )

project(
    libscid
    LANGUAGES C )

include( GNUInstallDirs )

set(
    SCID_PUBLIC_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/scid.h" )

add_library(
    scid
    INTERFACE )
add_library(
    LibScid::LibScid
    ALIAS scid )

target_sources(
    scid
    INTERFACE
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
        FILES ${SCID_PUBLIC_HEADERS} )

target_include_directories(
    scid
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>" )
