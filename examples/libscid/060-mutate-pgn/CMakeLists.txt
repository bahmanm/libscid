cmake_minimum_required( VERSION 3.28 )

project(
    libscid-example-060-mutate-pgn
    LANGUAGES C )

include( CTest )

find_package( libscid CONFIG REQUIRED )

add_executable(
    libscid-example-060-mutate-pgn
    main.c )

set_target_properties(
    libscid-example-060-mutate-pgn
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF )

target_link_libraries(
    libscid-example-060-mutate-pgn
    PRIVATE LibScid::LibScid )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-060-mutate-pgn
        COMMAND libscid-example-060-mutate-pgn )

    set_tests_properties(
        libscid-example-060-mutate-pgn
        PROPERTIES LABELS "c;libscid" )
endif()
