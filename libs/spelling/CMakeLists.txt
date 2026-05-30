cmake_minimum_required( VERSION 3.23 )

project(
    libscid-spelling
    LANGUAGES CXX )

set(
    SCID_SPELLING_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/spelling.cpp" )

set(
    SCID_SPELLING_PUBLIC_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/spelling/spelling.h" )

add_library(
    scid_spelling
    ${SCID_SPELLING_SOURCES} )
add_library(
    Scid::Spelling
    ALIAS scid_spelling )
target_sources(
    scid_spelling
    PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
        FILES ${SCID_SPELLING_PUBLIC_HEADERS} )
target_compile_features(
    scid_spelling
    PUBLIC cxx_std_20 )
target_include_directories(
    scid_spelling
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
target_link_libraries(
    scid_spelling
    PUBLIC Scid::Database )

option( SCID_SPELLING_VALIDATE "Verify the integrity of spelling files" OFF )
if( SCID_SPELLING_VALIDATE )
    target_compile_definitions( scid_spelling PRIVATE SCID_SPELLING_VALIDATE )
endif()
