cmake_minimum_required( VERSION 3.23 )

project(
    libscid-example-000-python-bindings
    LANGUAGES NONE )

include( CTest )
include( GNUInstallDirs )

find_package( Python3 COMPONENTS Interpreter REQUIRED )
find_package( libscid CONFIG REQUIRED )

set(
    LIBSCID_EXAMPLE_C_ABI_LIBRARY
    ""
    CACHE FILEPATH "Path to the libscid shared library used by C ABI examples." )

if( LIBSCID_EXAMPLE_C_ABI_LIBRARY STREQUAL "" AND TARGET scid )
    set( LIBSCID_EXAMPLE_C_ABI_LIBRARY "$<TARGET_FILE:scid>" )
endif()

if( LIBSCID_EXAMPLE_C_ABI_LIBRARY STREQUAL "" AND TARGET LibScid::LibScid )
    set( LIBSCID_EXAMPLE_C_ABI_LIBRARY "$<TARGET_FILE:LibScid::LibScid>" )
endif()

if( LIBSCID_EXAMPLE_C_ABI_LIBRARY STREQUAL "" )
    foreach( prefix IN LISTS CMAKE_PREFIX_PATH )
        set(
            candidate
            "${prefix}/${CMAKE_INSTALL_BINDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}scid${CMAKE_SHARED_LIBRARY_SUFFIX}" )
        if( EXISTS "${candidate}" )
            set( LIBSCID_EXAMPLE_C_ABI_LIBRARY "${candidate}" )
            break()
        endif()

        set(
            candidate
            "${prefix}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}scid${CMAKE_SHARED_LIBRARY_SUFFIX}" )
        if( EXISTS "${candidate}" )
            set( LIBSCID_EXAMPLE_C_ABI_LIBRARY "${candidate}" )
            break()
        endif()
    endforeach()
endif()

if( LIBSCID_EXAMPLE_C_ABI_LIBRARY STREQUAL "" )
    message(
        FATAL_ERROR
        "Could not find the libscid shared library. Set LIBSCID_EXAMPLE_C_ABI_LIBRARY." )
endif()

if( BUILD_TESTING )
    add_test(
        NAME libscid-example-000-python-bindings
        COMMAND
            "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/pgn_roundtrip.py"
            --library "${LIBSCID_EXAMPLE_C_ABI_LIBRARY}" )

    set_tests_properties(
        libscid-example-000-python-bindings
        PROPERTIES LABELS "python;libscid" )
endif()
