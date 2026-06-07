cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-060-spelling
    LANGUAGES CXX )

include( CTest )

find_package( libscid-cpp CONFIG REQUIRED )

add_executable(
    libscid-example-060-spelling
    main.cpp )

target_compile_features(
    libscid-example-060-spelling
    PRIVATE cxx_std_20 )

set_target_properties(
    libscid-example-060-spelling
    PROPERTIES
        CXX_EXTENSIONS OFF )

target_compile_definitions(
    libscid-example-060-spelling
    PRIVATE
        LIBSCID_EXAMPLE_SPELLING_FILE="${CMAKE_CURRENT_SOURCE_DIR}/../fixtures/mini.ssp" )

target_link_libraries(
    libscid-example-060-spelling
    PRIVATE
        LibScidCpp::Spelling )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-060-spelling
        COMMAND libscid-example-060-spelling )
endif()
