cmake_minimum_required( VERSION 3.28 )

project(
    libscid-example-130-search-headers
    LANGUAGES C )

include( CTest )

find_package( libscid CONFIG REQUIRED )

add_executable(
    libscid-example-130-search-headers
    main.c )

set_target_properties(
    libscid-example-130-search-headers
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF )

target_link_libraries(
    libscid-example-130-search-headers
    PRIVATE LibScid::LibScid )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-130-search-headers
        COMMAND libscid-example-130-search-headers )

    set_tests_properties(
        libscid-example-130-search-headers
        PROPERTIES LABELS "c;libscid" )
endif()
