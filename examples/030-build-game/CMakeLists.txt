cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-030-build-game
    LANGUAGES CXX )

include( CTest )

find_package( LibScid CONFIG REQUIRED )

add_executable(
    libscid-example-030-build-game
    main.cpp )

target_compile_features(
    libscid-example-030-build-game
    PRIVATE cxx_std_20 )

set_target_properties(
    libscid-example-030-build-game
    PROPERTIES
        CXX_EXTENSIONS OFF )

target_link_libraries(
    libscid-example-030-build-game
    PRIVATE
        LibScid::Core )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-030-build-game
        COMMAND libscid-example-030-build-game )
endif()
