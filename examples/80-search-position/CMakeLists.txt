cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-80-search-position
    LANGUAGES CXX )

include( CTest )

find_package( LibScid CONFIG REQUIRED )

add_executable(
    libscid-example-80-search-position
    main.cpp )

target_compile_features(
    libscid-example-80-search-position
    PRIVATE cxx_std_20 )

set_target_properties(
    libscid-example-80-search-position
    PROPERTIES
        CXX_EXTENSIONS OFF )

target_compile_definitions(
    libscid-example-80-search-position
    PRIVATE
        LIBSCID_EXAMPLE_DATABASE="${CMAKE_CURRENT_SOURCE_DIR}/../fixtures/dr-lasker/dr-lasker" )

target_link_libraries(
    libscid-example-80-search-position
    PRIVATE
        LibScid::Database )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-80-search-position
        COMMAND libscid-example-80-search-position )

    set_tests_properties(
        libscid-example-80-search-position
        PROPERTIES
            SKIP_RETURN_CODE 77 )
endif()
