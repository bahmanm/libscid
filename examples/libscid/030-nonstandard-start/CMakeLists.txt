cmake_minimum_required( VERSION 3.28 )

project(
    libscid-example-030-nonstandard-start
    LANGUAGES C )

include( CTest )

find_package( libscid CONFIG REQUIRED )

add_executable(
    libscid-example-030-nonstandard-start
    main.c )

set_target_properties(
    libscid-example-030-nonstandard-start
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF )

target_link_libraries(
    libscid-example-030-nonstandard-start
    PRIVATE LibScid::LibScid )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-030-nonstandard-start
        COMMAND libscid-example-030-nonstandard-start )

    set_tests_properties(
        libscid-example-030-nonstandard-start
        PROPERTIES LABELS "c;libscid" )
endif()
