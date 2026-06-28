cmake_minimum_required( VERSION 3.28 )

project(
    libscid-example-010-edit-pgn
    LANGUAGES C )

include( CTest )

find_package( libscid CONFIG REQUIRED )

add_executable(
    libscid-example-010-edit-pgn
    main.c )

set_target_properties(
    libscid-example-010-edit-pgn
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF )

target_link_libraries(
    libscid-example-010-edit-pgn
    PRIVATE LibScid::LibScid )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-010-edit-pgn
        COMMAND libscid-example-010-edit-pgn )

    set_tests_properties(
        libscid-example-010-edit-pgn
        PROPERTIES LABELS "c;libscid" )
endif()
