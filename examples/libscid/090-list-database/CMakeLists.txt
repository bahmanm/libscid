cmake_minimum_required( VERSION 3.28 )

project(
    libscid-example-090-list-database
    LANGUAGES C )

include( CTest )

find_package( libscid CONFIG REQUIRED )

add_executable(
    libscid-example-090-list-database
    main.c )

set_target_properties(
    libscid-example-090-list-database
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF )

target_link_libraries(
    libscid-example-090-list-database
    PRIVATE LibScid::LibScid )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-090-list-database
        COMMAND libscid-example-090-list-database )

    set_tests_properties(
        libscid-example-090-list-database
        PROPERTIES LABELS "c;libscid" )
endif()
