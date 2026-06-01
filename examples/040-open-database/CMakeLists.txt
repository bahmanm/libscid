cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-040-open-database
    LANGUAGES CXX )

include( CTest )

find_package( LibScid CONFIG REQUIRED )

add_executable(
    libscid-example-040-open-database
    main.cpp )

target_compile_features(
    libscid-example-040-open-database
    PRIVATE cxx_std_20 )

set_target_properties(
    libscid-example-040-open-database
    PROPERTIES
        CXX_EXTENSIONS OFF )

target_compile_definitions(
    libscid-example-040-open-database
    PRIVATE
        LIBSCID_EXAMPLE_DATABASE="${CMAKE_CURRENT_SOURCE_DIR}/../fixtures/dr-lasker/dr-lasker" )

target_link_libraries(
    libscid-example-040-open-database
    PRIVATE
        LibScid::Database )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-040-open-database
        COMMAND libscid-example-040-open-database )

    set_tests_properties(
        libscid-example-040-open-database
        PROPERTIES
            SKIP_RETURN_CODE 77 )
endif()
