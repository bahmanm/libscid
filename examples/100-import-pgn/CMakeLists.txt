cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-100-import-pgn
    LANGUAGES CXX )

include( CTest )

find_package( libscid-cpp CONFIG REQUIRED )

add_executable(
    libscid-example-100-import-pgn
    main.cpp )

target_compile_features(
    libscid-example-100-import-pgn
    PRIVATE cxx_std_20 )

set_target_properties(
    libscid-example-100-import-pgn
    PROPERTIES
        CXX_EXTENSIONS OFF )

target_compile_definitions(
    libscid-example-100-import-pgn
    PRIVATE
        LIBSCID_EXAMPLE_PGN_FILE="${CMAKE_CURRENT_SOURCE_DIR}/../fixtures/import.pgn" )

target_link_libraries(
    libscid-example-100-import-pgn
    PRIVATE
        LibScidCpp::Database )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-100-import-pgn
        COMMAND libscid-example-100-import-pgn )
endif()
