cmake_minimum_required( VERSION 3.28 )

project(
    libscid-example-110-persistent-database
    LANGUAGES C )

include( CTest )

find_package( libscid CONFIG REQUIRED )

add_executable(
    libscid-example-110-persistent-database
    main.c )

set_target_properties(
    libscid-example-110-persistent-database
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF )

target_link_libraries(
    libscid-example-110-persistent-database
    PRIVATE LibScid::LibScid )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-110-persistent-database
        COMMAND libscid-example-110-persistent-database )

    set_tests_properties(
        libscid-example-110-persistent-database
        PROPERTIES LABELS "c;libscid" )
endif()
