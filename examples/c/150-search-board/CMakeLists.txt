cmake_minimum_required( VERSION 3.28 )

project(
    libscid-example-150-search-board
    LANGUAGES C )

include( CTest )

find_package( libscid CONFIG REQUIRED )

add_executable(
    libscid-example-150-search-board
    main.c )

set_target_properties(
    libscid-example-150-search-board
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF )

target_link_libraries(
    libscid-example-150-search-board
    PRIVATE LibScid::LibScid )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-150-search-board
        COMMAND libscid-example-150-search-board )

    set_tests_properties(
        libscid-example-150-search-board
        PROPERTIES LABELS "c;libscid" )
endif()
