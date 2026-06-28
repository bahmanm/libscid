cmake_minimum_required( VERSION 3.23 )

project(
    libscid-spelling
    LANGUAGES CXX )

include( GNUInstallDirs )

set(
    SCID_SPELLING_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/spelling.cpp" )

set(
    SCID_SPELLING_PUBLIC_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/spelling/spelling.h" )

add_library(
    scid_spelling
    STATIC
    ${SCID_SPELLING_SOURCES} )
add_library(
    LibScidCpp::Spelling
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

set_target_properties(
    scid_spelling
    PROPERTIES POSITION_INDEPENDENT_CODE ON )

target_include_directories(
    scid_spelling
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>" )
target_link_libraries(
    scid_spelling
    PUBLIC LibScidCpp::Database )

option( LIBSCID_SPELLING_VALIDATE "Verify the integrity of spelling files" OFF )
if( LIBSCID_SPELLING_VALIDATE )
    target_compile_definitions( scid_spelling PRIVATE SCID_SPELLING_VALIDATE )
endif()
