find_package( Doxygen REQUIRED )

set( LIBSCID_DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/doxygen" )
set( LIBSCID_PLANTUML_JAR_PATH "" CACHE FILEPATH "Path to plantuml.jar for Doxygen diagram rendering." )
set( LIBSCID_DOXYGEN_INPUT_DIRS
    "${PROJECT_SOURCE_DIR}/docs"
    "${PROJECT_SOURCE_DIR}/libs/core/include/scid/core"
    "${PROJECT_SOURCE_DIR}/libs/database/include/scid/database"
    "${PROJECT_SOURCE_DIR}/libs/database/docs"
    "${PROJECT_SOURCE_DIR}/libs/eco/include/scid/eco"
    "${PROJECT_SOURCE_DIR}/libs/spelling/include/scid/spelling" )
set( LIBSCID_PLANTUML_DIAGRAM_DIR "${PROJECT_SOURCE_DIR}/docs/diagrams" )

configure_file(
    "${PROJECT_SOURCE_DIR}/docs/Doxyfile.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    @ONLY )

add_custom_target(
    api-docs
    COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    BYPRODUCTS "${LIBSCID_DOXYGEN_OUTPUT_DIR}/html/index.html"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Generating libscid API reference"
    VERBATIM )

add_custom_target(
    docs
    DEPENDS api-docs )
