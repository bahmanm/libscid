cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-070-eco-lookup
    LANGUAGES C )

include( CTest )

find_package( libscid CONFIG REQUIRED )

add_executable(
    libscid-example-070-eco-lookup
    main.c )

set_target_properties(
    libscid-example-070-eco-lookup
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF )

target_compile_definitions(
    libscid-example-070-eco-lookup
    PRIVATE
        LIBSCID_EXAMPLE_ECO_FILE="${CMAKE_CURRENT_SOURCE_DIR}/../../fixtures/mini.eco" )

target_link_libraries(
    libscid-example-070-eco-lookup
    PRIVATE LibScid::LibScid )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-070-eco-lookup
        COMMAND libscid-example-070-eco-lookup )

    set_tests_properties(
        libscid-example-070-eco-lookup
        PROPERTIES LABELS "c;libscid" )
endif()
