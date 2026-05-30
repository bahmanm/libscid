cmake_minimum_required( VERSION 3.23 )

project(
    libscid-examples
    LANGUAGES CXX )

enable_testing()

add_subdirectory( 00-minimal-consumer )
add_subdirectory( 10-pgn-roundtrip )
add_subdirectory( 20-position-and-moves )
