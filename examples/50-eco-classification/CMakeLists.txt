cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-50-eco-classification
    LANGUAGES CXX )

include( CTest )

find_package( LibScid CONFIG REQUIRED )

add_executable(
    libscid-example-50-eco-classification
    main.cpp )

target_compile_features(
    libscid-example-50-eco-classification
    PRIVATE cxx_std_20 )

set_target_properties(
    libscid-example-50-eco-classification
    PROPERTIES
        CXX_EXTENSIONS OFF )

target_compile_definitions(
    libscid-example-50-eco-classification
    PRIVATE
        LIBSCID_EXAMPLE_ECO_FILE="${CMAKE_CURRENT_SOURCE_DIR}/../fixtures/mini.eco" )

target_link_libraries(
    libscid-example-50-eco-classification
    PRIVATE
        LibScid::Eco )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-50-eco-classification
        COMMAND libscid-example-50-eco-classification )
endif()
