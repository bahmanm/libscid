if( NOT DEFINED LIBSCID_SOURCE_ROOT )
    message( FATAL_ERROR "LIBSCID_SOURCE_ROOT must point at the repository root." )
endif()

get_filename_component( LIBSCID_SOURCE_ROOT "${LIBSCID_SOURCE_ROOT}" REALPATH )

include( GNUInstallDirs )
include( "${LIBSCID_SOURCE_ROOT}/etc/cmake/test/runtime-dlls.cmake" )
include( "${LIBSCID_SOURCE_ROOT}/etc/cmake/qc/sanitisers.cmake" )

set( CMAKE_C_VISIBILITY_PRESET hidden )
set( CMAKE_CXX_VISIBILITY_PRESET hidden )
set( CMAKE_VISIBILITY_INLINES_HIDDEN ON )


option( LIBSCID_INSTALL "Install libscid targets and CMake package files." "${PROJECT_IS_TOP_LEVEL}" )
option( LIBSCID_BUILD_DOCS "Build libscid API documentation." OFF )

if( PROJECT_IS_TOP_LEVEL )
    set(
        CMAKE_EXPORT_COMPILE_COMMANDS
        ON
        CACHE BOOL "Generate compile_commands.json for static analysis tools."
        FORCE )

    if( NOT DEFINED BUILD_TESTING )
        set( BUILD_TESTING OFF CACHE BOOL "Build tests" )
    endif()
    include( CTest )
    include( "${LIBSCID_SOURCE_ROOT}/etc/cmake/qc/clang-tidy.cmake" )
    include( "${LIBSCID_SOURCE_ROOT}/etc/cmake/qc/cppcheck.cmake" )
    include( "${LIBSCID_SOURCE_ROOT}/etc/cmake/qc/format.cmake" )
    include( "${LIBSCID_SOURCE_ROOT}/etc/cmake/qc/include-cleaner.cmake" )
endif()
