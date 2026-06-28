cmake_minimum_required( VERSION 3.28 )

project(
    libscid-example-050-position-and-moves
    LANGUAGES C )

include( CTest )

find_package( libscid CONFIG REQUIRED )

add_executable(
    libscid-example-050-position-and-moves
    main.c )

set_target_properties(
    libscid-example-050-position-and-moves
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF )

target_link_libraries(
    libscid-example-050-position-and-moves
    PRIVATE LibScid::LibScid )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-050-position-and-moves
        COMMAND libscid-example-050-position-and-moves )

    set_tests_properties(
        libscid-example-050-position-and-moves
        PROPERTIES LABELS "c;libscid" )
endif()
