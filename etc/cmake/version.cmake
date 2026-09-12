cmake_minimum_required( VERSION 3.28 )

get_filename_component( LIBSCID_SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." REALPATH )

function( _libscid_discover_version out_var )
    set( _discovered_version "" )

    find_package( Git QUIET )
    if( Git_FOUND AND EXISTS "${LIBSCID_SOURCE_ROOT}/.git" )
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" -C "${LIBSCID_SOURCE_ROOT}" describe --tags --match "v*" --exact-match HEAD
            OUTPUT_VARIABLE _git_tag
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE )
        if( NOT _git_tag STREQUAL "" )
            set( _discovered_version "${_git_tag}" )
        else()
            execute_process(
                COMMAND "${GIT_EXECUTABLE}" -C "${LIBSCID_SOURCE_ROOT}" rev-parse --short=12 HEAD
                OUTPUT_VARIABLE _git_sha
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE )
            if( NOT _git_sha STREQUAL "" )
                set( _discovered_version "snapshot+${_git_sha}" )
            endif()
        endif()
    endif()

    if( _discovered_version STREQUAL "" )
        set( _discovered_version "snapshot" )
    endif()

    set( ${out_var} "${_discovered_version}" PARENT_SCOPE )
endfunction()

function( _libscid_parse_project_version version_string out_var )
    set( _parsed_version "0.0.0" )
    if( version_string MATCHES "^v?([0-9]+\\.[0-9]+(\\.[0-9]+)?)" )
        set( _parsed_version "${CMAKE_MATCH_1}" )
        if( NOT _parsed_version MATCHES "\\.[0-9]+\\.[0-9]+" )
            set( _parsed_version "${_parsed_version}.0" )
        endif()
    endif()
    set( ${out_var} "${_parsed_version}" PARENT_SCOPE )
endfunction()

if( NOT DEFINED LIBSCID_VERSION )
    _libscid_discover_version( _libscid_default_version )
    set( LIBSCID_VERSION "${_libscid_default_version}" CACHE STRING "Human-readable libscid version label." )
endif()

if( NOT DEFINED LIBSCID_PROJECT_VERSION )
    _libscid_parse_project_version( "${LIBSCID_VERSION}" _libscid_default_project_version )
    set( LIBSCID_PROJECT_VERSION "${_libscid_default_project_version}" CACHE STRING "Numeric libscid version used for CMake package compatibility." )
endif()

if( DEFINED CMAKE_SCRIPT_MODE_FILE )
    execute_process( COMMAND "${CMAKE_COMMAND}" -E echo "${LIBSCID_PROJECT_VERSION} ${LIBSCID_VERSION}" )
endif()
