cmake_minimum_required( VERSION 3.28 )

project(
    libscid-example-120-import-export-pgn
    LANGUAGES C )

include( CTest )

find_package( libscid CONFIG REQUIRED )

add_executable(
    libscid-example-120-import-export-pgn
    main.c )

set_target_properties(
    libscid-example-120-import-export-pgn
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF )

target_link_libraries(
    libscid-example-120-import-export-pgn
    PRIVATE LibScid::LibScid )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-120-import-export-pgn
        COMMAND libscid-example-120-import-export-pgn )

    set_tests_properties(
        libscid-example-120-import-export-pgn
        PROPERTIES LABELS "c;libscid" )
endif()
