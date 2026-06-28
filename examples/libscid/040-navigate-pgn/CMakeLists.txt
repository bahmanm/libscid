cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-040-navigate-pgn
    LANGUAGES C )

include( CTest )

find_package( libscid CONFIG REQUIRED )

add_executable(
    libscid-example-040-navigate-pgn
    main.c )

set_target_properties(
    libscid-example-040-navigate-pgn
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF )

target_link_libraries(
    libscid-example-040-navigate-pgn
    PRIVATE LibScid::LibScid )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-040-navigate-pgn
        COMMAND libscid-example-040-navigate-pgn )

    set_tests_properties(
        libscid-example-040-navigate-pgn
        PROPERTIES LABELS "c;libscid" )
endif()
