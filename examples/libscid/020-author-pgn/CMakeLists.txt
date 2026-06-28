cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-020-author-pgn
    LANGUAGES C )

include( CTest )

find_package( libscid CONFIG REQUIRED )

add_executable(
    libscid-example-020-author-pgn
    main.c )

set_target_properties(
    libscid-example-020-author-pgn
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF )

target_link_libraries(
    libscid-example-020-author-pgn
    PRIVATE LibScid::LibScid )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-020-author-pgn
        COMMAND libscid-example-020-author-pgn )

    set_tests_properties(
        libscid-example-020-author-pgn
        PROPERTIES LABELS "c;libscid" )
endif()
