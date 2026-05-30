cmake_minimum_required( VERSION 3.23 )

project(
    libscid-examples
    LANGUAGES CXX )

enable_testing()

add_subdirectory( 00-minimal-consumer )
add_subdirectory( 10-pgn-roundtrip )
add_subdirectory( 20-position-and-moves )
add_subdirectory( 30-build-game )
add_subdirectory( 40-open-database )
add_subdirectory( 50-eco-classification )
add_subdirectory( 60-spelling )
add_subdirectory( 70-list-database )
