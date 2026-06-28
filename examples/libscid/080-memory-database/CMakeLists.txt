cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-080-memory-database
    LANGUAGES C )

include( CTest )

find_package( libscid CONFIG REQUIRED )

add_executable(
    libscid-example-080-memory-database
    main.c )

set_target_properties(
    libscid-example-080-memory-database
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF )

target_link_libraries(
    libscid-example-080-memory-database
    PRIVATE LibScid::LibScid )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-080-memory-database
        COMMAND libscid-example-080-memory-database )

    set_tests_properties(
        libscid-example-080-memory-database
        PROPERTIES LABELS "c;libscid" )
endif()
