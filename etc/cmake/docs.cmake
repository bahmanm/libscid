find_package( Doxygen REQUIRED )
find_package( Python3 REQUIRED COMPONENTS Interpreter )
find_program( DOXYBOOK2_EXECUTABLE NAMES doxybook2 DOC "Path to doxybook2 executable." )
find_program( UV_EXECUTABLE NAMES uv DOC "Path to uv executable." )
find_program( MKDOCS_EXECUTABLE NAMES mkdocs HINTS "${LIBSCID_SOURCE_ROOT}/capi/docs/.venv/bin" DOC "Path to mkdocs executable." )

set( LIBSCID_DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/doxygen" )
set( LIBSCID_DOXYBOOK2_CONFIG "${LIBSCID_SOURCE_ROOT}/capi/docs/doxybook2.json" )
set( LIBSCID_MKDOCS_CONFIG "${LIBSCID_SOURCE_ROOT}/capi/docs/mkdocs.yml" )
set( LIBSCID_REFERENCE_DOCS_DIR "${LIBSCID_SOURCE_ROOT}/docs/reference" )
set( LIBSCID_SITE_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/site" )
set( LIBSCID_PLANTUML_JAR_PATH "" CACHE FILEPATH "Path to plantuml.jar for Doxygen diagram rendering." )
set( LIBSCID_PLANTUML_DIAGRAM_DIR "${LIBSCID_SOURCE_ROOT}/docs/diagrams" )

configure_file(
    "${LIBSCID_SOURCE_ROOT}/docs/Doxyfile.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    @ONLY )

set( LIBSCID_DOCS_COMMANDS
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${LIBSCID_REFERENCE_DOCS_DIR}"
    COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" )

if( DOXYBOOK2_EXECUTABLE )
    list( APPEND LIBSCID_DOCS_COMMANDS
        COMMAND "${DOXYBOOK2_EXECUTABLE}" -i "${LIBSCID_DOXYGEN_OUTPUT_DIR}/xml" -o "${LIBSCID_REFERENCE_DOCS_DIR}" -c "${LIBSCID_DOXYBOOK2_CONFIG}" )
else()
    message( WARNING "doxybook2 executable not found; C ABI Markdown reference generation will be skipped." )
endif()

if( UV_EXECUTABLE )
    list( APPEND LIBSCID_DOCS_COMMANDS
        COMMAND "${UV_EXECUTABLE}" run --project "${LIBSCID_SOURCE_ROOT}/capi/docs" mkdocs build -f "${LIBSCID_MKDOCS_CONFIG}" -d "${LIBSCID_SITE_OUTPUT_DIR}" )
elseif( MKDOCS_EXECUTABLE )
    list( APPEND LIBSCID_DOCS_COMMANDS
        COMMAND "${MKDOCS_EXECUTABLE}" build -f "${LIBSCID_MKDOCS_CONFIG}" -d "${LIBSCID_SITE_OUTPUT_DIR}" )
else()
    message( WARNING "Neither 'uv' nor 'mkdocs' executable found; MkDocs HTML site generation will be skipped." )
endif()

add_custom_target(
    api-docs
    ALL
    ${LIBSCID_DOCS_COMMANDS}
    WORKING_DIRECTORY "${LIBSCID_SOURCE_ROOT}"
    COMMENT "Generating libscid C ABI reference documentation"
    VERBATIM )

add_custom_target(
    docs
    DEPENDS api-docs )
