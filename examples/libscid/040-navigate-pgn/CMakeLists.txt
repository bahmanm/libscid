cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-040-navigate-pgn
    LANGUAGES C )

include( CTest )

find_path(
    LIBSCID_C_INCLUDE_DIR
    NAMES scid/scid.h )

find_library(
    LIBSCID_C_LIBRARY
    NAMES scid libscid )

if( NOT LIBSCID_C_INCLUDE_DIR OR NOT LIBSCID_C_LIBRARY )
    message(
        FATAL_ERROR
        "Could not find libscid C ABI. Set CMAKE_PREFIX_PATH to the install root." )
endif()

add_executable(
    libscid-example-040-navigate-pgn
    main.c )

set_target_properties(
    libscid-example-040-navigate-pgn
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF )

target_include_directories(
    libscid-example-040-navigate-pgn
    PRIVATE "${LIBSCID_C_INCLUDE_DIR}" )

target_link_libraries(
    libscid-example-040-navigate-pgn
    PRIVATE "${LIBSCID_C_LIBRARY}" )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-040-navigate-pgn
        COMMAND libscid-example-040-navigate-pgn )

    set_tests_properties(
        libscid-example-040-navigate-pgn
        PROPERTIES LABELS "c;libscid" )
endif()
