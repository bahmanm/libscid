find_package( Doxygen REQUIRED )
find_package( Python3 REQUIRED COMPONENTS Interpreter )

set( LIBSCID_DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/doxygen" )
set( LIBSCID_PLANTUML_JAR_PATH "" CACHE FILEPATH "Path to plantuml.jar for Doxygen diagram rendering." )
set( LIBSCID_DOXYGEN_INPUT_DIRS
    "${PROJECT_SOURCE_DIR}/docs/index.md"
    "${PROJECT_SOURCE_DIR}/docs/api-surface.md"
    "${PROJECT_SOURCE_DIR}/docs/quick-start.md"
    "${PROJECT_SOURCE_DIR}/docs/installation.md"
    "${PROJECT_SOURCE_DIR}/docs/examples-recipes.md"
    "${PROJECT_SOURCE_DIR}/libscid/include/scid" )
set( LIBSCID_PLANTUML_DIAGRAM_DIR "${PROJECT_SOURCE_DIR}/docs/diagrams" )

configure_file(
    "${PROJECT_SOURCE_DIR}/docs/Doxyfile.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    @ONLY )

add_custom_target(
    api-docs
    ALL
    COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/docs/doxygen/patch_navigation.py" "${LIBSCID_DOXYGEN_OUTPUT_DIR}/html"
    BYPRODUCTS "${LIBSCID_DOXYGEN_OUTPUT_DIR}/html/index.html"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Generating libscid API reference"
    VERBATIM )

add_custom_target(
    docs
    DEPENDS api-docs )
