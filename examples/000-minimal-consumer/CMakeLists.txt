cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-000-minimal-consumer
    LANGUAGES CXX )

include( CTest )

find_package( LibScid CONFIG REQUIRED )

add_executable(
    libscid-example-000-minimal-consumer
    main.cpp )

target_compile_features(
    libscid-example-000-minimal-consumer
    PRIVATE cxx_std_20 )

set_target_properties(
    libscid-example-000-minimal-consumer
    PROPERTIES
        CXX_EXTENSIONS OFF )

target_link_libraries(
    libscid-example-000-minimal-consumer
    PRIVATE
        LibScid::Core )

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-000-minimal-consumer
        COMMAND libscid-example-000-minimal-consumer )
endif()
