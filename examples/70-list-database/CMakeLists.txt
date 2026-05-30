cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-70-list-database
    LANGUAGES CXX )

include( CTest )

find_package( LibScid CONFIG REQUIRED )

add_executable(
    libscid-example-70-list-database
    main.cpp )

target_compile_features(
    libscid-example-70-list-database
    PRIVATE cxx_std_20 )

set_target_properties(
    libscid-example-70-list-database
    PROPERTIES
        CXX_EXTENSIONS OFF )

target_compile_definitions(
    libscid-example-70-list-database
    PRIVATE
        LIBSCID_EXAMPLE_DATABASE="${CMAKE_CURRENT_SOURCE_DIR}/../40-open-database/data/dr-lasker" )

target_link_libraries(
    libscid-example-70-list-database
    PRIVATE
        LibScid::Database )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-70-list-database
        COMMAND libscid-example-70-list-database )

    set_tests_properties(
        libscid-example-70-list-database
        PROPERTIES
            SKIP_RETURN_CODE 77 )
endif()
