cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-90-opening-tree
    LANGUAGES CXX )

include( CTest )

find_package( LibScid CONFIG REQUIRED )

add_executable(
    libscid-example-90-opening-tree
    main.cpp )

target_compile_features(
    libscid-example-90-opening-tree
    PRIVATE cxx_std_20 )

set_target_properties(
    libscid-example-90-opening-tree
    PROPERTIES
        CXX_EXTENSIONS OFF )

target_compile_definitions(
    libscid-example-90-opening-tree
    PRIVATE
        LIBSCID_EXAMPLE_DATABASE="${CMAKE_CURRENT_SOURCE_DIR}/../40-open-database/data/dr-lasker" )

target_link_libraries(
    libscid-example-90-opening-tree
    PRIVATE
        LibScid::Database )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-90-opening-tree
        COMMAND libscid-example-90-opening-tree )

    set_tests_properties(
        libscid-example-90-opening-tree
        PROPERTIES
            SKIP_RETURN_CODE 77 )
endif()
