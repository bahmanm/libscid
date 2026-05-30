set( SCID_PACKAGE_PLATFORM "" CACHE STRING "Platform label used in libscid package file names." )

if( SCID_PACKAGE_PLATFORM STREQUAL "" )
    set( _scid_package_platform "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}" )
else()
    set( _scid_package_platform "${SCID_PACKAGE_PLATFORM}" )
endif()

string( TOLOWER "${_scid_package_platform}" _scid_package_platform )
string( REGEX REPLACE "[^a-z0-9_.+-]+" "-" _scid_package_platform "${_scid_package_platform}" )
string( REGEX REPLACE "^-+|-+$" "" _scid_package_platform "${_scid_package_platform}" )

if( _scid_package_platform STREQUAL "" )
    set( _scid_package_platform "unknown" )
endif()

set( CPACK_PACKAGE_NAME "libscid" )
set( CPACK_PACKAGE_VENDOR "libscid contributors" )
set( CPACK_PACKAGE_VERSION "${PROJECT_VERSION}" )
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Standalone C++ library extracted from Scid chess database functionality." )
set( CPACK_PACKAGE_FILE_NAME "libscid__${PROJECT_VERSION}__${_scid_package_platform}" )
set( CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}" )
set( CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/COPYING" )

set( CPACK_GENERATOR "TGZ" )
set( CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON )
set( CPACK_VERBATIM_VARIABLES ON )

include( CPack )
