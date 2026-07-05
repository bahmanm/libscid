cmake_minimum_required( VERSION 3.28 )

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
    LibScidInternal::Spelling
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
    PUBLIC cxx_std_23 )

set_target_properties(
    scid_spelling
    PROPERTIES
        CXX_EXTENSIONS OFF
        POSITION_INDEPENDENT_CODE ON )

target_include_directories(
    scid_spelling
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>" )
target_link_libraries(
    scid_spelling
    PUBLIC LibScidInternal::Database )

option( LIBSCID_SPELLING_VALIDATE "Verify the integrity of spelling files" OFF )
if( LIBSCID_SPELLING_VALIDATE )
    target_compile_definitions( scid_spelling PRIVATE SCID_SPELLING_VALIDATE )
endif()

if( PROJECT_IS_TOP_LEVEL )
    include( CTest )
endif()

if( BUILD_TESTING )
    if( NOT TARGET gtest_main )
        include( FetchContent )
        set( INSTALL_GTEST OFF CACHE BOOL "Disable installation rules for googletest." FORCE )
        set( BUILD_GMOCK OFF CACHE BOOL "Do not build GoogleMock; spelling tests use GoogleTest only." FORCE )
        set( gtest_force_shared_crt ON CACHE BOOL "Always use msvcrt.dll" FORCE )

        fetchcontent_declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
            URL_HASH SHA256=65fab701d9829d38cb77c14acdc431d2108bfdbf8979e40eb8ae567edf10b27c )
        fetchcontent_makeavailable( googletest )
    endif()

    file(
        GLOB SCID_SPELLING_TEST_SOURCES
        CONFIGURE_DEPENDS
        "${LIBSCID_SOURCE_ROOT}/tests/internal/spelling/cxx/*.cpp" )
    add_executable(
        scid_tests_spelling_cpp
        ${SCID_SPELLING_TEST_SOURCES} )
    target_link_libraries(
        scid_tests_spelling_cpp
        PRIVATE LibScidInternal::Spelling gtest_main )
    libscid_enable_test_runtime_dlls( scid_tests_spelling_cpp )
    add_executable(
        LibScidInternal::Tests::Spelling::Cpp
        ALIAS scid_tests_spelling_cpp )

    add_test(
        NAME spelling_cpp_test
        COMMAND $<TARGET_FILE:LibScidInternal::Tests::Spelling::Cpp> )
    set_tests_properties(
        spelling_cpp_test
        PROPERTIES LABELS "cpp;spelling" )
endif()
