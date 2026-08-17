cmake_minimum_required( VERSION 3.28 )

project(
    libscid-example-140-search-position
    LANGUAGES C )

include( CTest )

find_package( libscid CONFIG REQUIRED )

add_executable(
    libscid-example-140-search-position
    main.c )

set_target_properties(
    libscid-example-140-search-position
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF )

target_link_libraries(
    libscid-example-140-search-position
    PRIVATE LibScid::LibScid )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-140-search-position
        COMMAND libscid-example-140-search-position )

    set_tests_properties(
        libscid-example-140-search-position
        PROPERTIES LABELS "c;libscid" )
endif()
