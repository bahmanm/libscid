cmake_minimum_required( VERSION 3.23 )

project(
    libscid-examples
    LANGUAGES CXX )

enable_testing()

add_subdirectory( 000-minimal-consumer )
add_subdirectory( 010-pgn-roundtrip )
add_subdirectory( 020-position-and-moves )
add_subdirectory( 030-build-game )
add_subdirectory( 040-open-database )
add_subdirectory( 050-eco-classification )
add_subdirectory( 060-spelling )
add_subdirectory( 070-list-database )
add_subdirectory( 080-search-position )
add_subdirectory( 090-opening-tree )
add_subdirectory( 100-import-pgn )
add_subdirectory( 110-database-stats )
