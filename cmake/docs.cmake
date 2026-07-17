find_package( Doxygen REQUIRED )
find_package( Python3 REQUIRED COMPONENTS Interpreter )

set( LIBSCID_DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/doxygen" )
set( LIBSCID_PLANTUML_JAR_PATH "" CACHE FILEPATH "Path to plantuml.jar for Doxygen diagram rendering." )
set( LIBSCID_DOXYGEN_INPUT_DIRS
    "${LIBSCID_SOURCE_ROOT}/docs/index.md"
    "${LIBSCID_SOURCE_ROOT}/docs/api-surface.md"
    "${LIBSCID_SOURCE_ROOT}/docs/quick-start.md"
    "${LIBSCID_SOURCE_ROOT}/docs/installation.md"
    "${LIBSCID_SOURCE_ROOT}/docs/examples-recipes.md"
    "${LIBSCID_SOURCE_ROOT}/src/libscid/include/scid" )
set( LIBSCID_PLANTUML_DIAGRAM_DIR "${LIBSCID_SOURCE_ROOT}/docs/diagrams" )

configure_file(
    "${LIBSCID_SOURCE_ROOT}/docs/Doxyfile.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    @ONLY )

add_custom_target(
    api-docs
    ALL
    COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    COMMAND "${Python3_EXECUTABLE}" "${LIBSCID_SOURCE_ROOT}/docs/doxygen/patch_navigation.py" "${LIBSCID_DOXYGEN_OUTPUT_DIR}/html"
    BYPRODUCTS "${LIBSCID_DOXYGEN_OUTPUT_DIR}/html/index.html"
    WORKING_DIRECTORY "${LIBSCID_SOURCE_ROOT}"
    COMMENT "Generating libscid API reference"
    VERBATIM )

add_custom_target(
    docs
    DEPENDS api-docs )
