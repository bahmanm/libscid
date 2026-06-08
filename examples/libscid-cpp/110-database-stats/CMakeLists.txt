cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-110-database-stats
    LANGUAGES CXX )

include( CTest )

find_package( libscid-cpp CONFIG REQUIRED )

add_executable(
    libscid-example-110-database-stats
    main.cpp )

target_compile_features(
    libscid-example-110-database-stats
    PRIVATE cxx_std_20 )

set_target_properties(
    libscid-example-110-database-stats
    PROPERTIES
        CXX_EXTENSIONS OFF )

target_compile_definitions(
    libscid-example-110-database-stats
    PRIVATE
        LIBSCID_EXAMPLE_DATABASE="${CMAKE_CURRENT_SOURCE_DIR}/../../fixtures/dr-lasker/dr-lasker" )

target_link_libraries(
    libscid-example-110-database-stats
    PRIVATE
        LibScidCpp::Database )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-110-database-stats
        COMMAND libscid-example-110-database-stats )

    set_tests_properties(
        libscid-example-110-database-stats
        PROPERTIES
            SKIP_RETURN_CODE 77 )
endif()
