cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-60-spelling
    LANGUAGES CXX )

include( CTest )

find_package( LibScid CONFIG REQUIRED )

add_executable(
    libscid-example-60-spelling
    main.cpp )

target_compile_features(
    libscid-example-60-spelling
    PRIVATE cxx_std_20 )

set_target_properties(
    libscid-example-60-spelling
    PROPERTIES
        CXX_EXTENSIONS OFF )

target_compile_definitions(
    libscid-example-60-spelling
    PRIVATE
        LIBSCID_EXAMPLE_SPELLING_FILE="${CMAKE_CURRENT_SOURCE_DIR}/../fixtures/mini.ssp" )

target_link_libraries(
    libscid-example-60-spelling
    PRIVATE
        LibScid::Spelling )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-60-spelling
        COMMAND libscid-example-60-spelling )
endif()
