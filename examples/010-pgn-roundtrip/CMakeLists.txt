cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-010-pgn-roundtrip
    LANGUAGES CXX )

include( CTest )

find_package( LibScid CONFIG REQUIRED )

add_executable(
    libscid-example-010-pgn-roundtrip
    main.cpp )

target_compile_features(
    libscid-example-010-pgn-roundtrip
    PRIVATE cxx_std_20 )

set_target_properties(
    libscid-example-010-pgn-roundtrip
    PROPERTIES
        CXX_EXTENSIONS OFF )

target_link_libraries(
    libscid-example-010-pgn-roundtrip
    PRIVATE
        LibScid::Core )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-010-pgn-roundtrip
        COMMAND libscid-example-010-pgn-roundtrip )
endif()
