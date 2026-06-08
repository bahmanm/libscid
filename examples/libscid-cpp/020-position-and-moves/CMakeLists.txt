cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-020-position-and-moves
    LANGUAGES CXX )

include( CTest )

find_package( libscid-cpp CONFIG REQUIRED )

add_executable(
    libscid-example-020-position-and-moves
    main.cpp )

target_compile_features(
    libscid-example-020-position-and-moves
    PRIVATE cxx_std_20 )

set_target_properties(
    libscid-example-020-position-and-moves
    PROPERTIES
        CXX_EXTENSIONS OFF )

target_link_libraries(
    libscid-example-020-position-and-moves
    PRIVATE
        LibScidCpp::Core )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-020-position-and-moves
        COMMAND libscid-example-020-position-and-moves )
endif()
