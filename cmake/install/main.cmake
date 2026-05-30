include( CMakePackageConfigHelpers )

set( SCID_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/Scid" )

set_property( TARGET scid_core PROPERTY EXPORT_NAME Core )
set_property( TARGET scid_database PROPERTY EXPORT_NAME Database )
set_property( TARGET scid_eco PROPERTY EXPORT_NAME Eco )
set_property( TARGET scid_spelling PROPERTY EXPORT_NAME Spelling )

install(
    TARGETS
        scid_core
        scid_database
        scid_eco
        scid_spelling
    EXPORT ScidTargets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    FILE_SET public_headers DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" )

install(
    EXPORT ScidTargets
    NAMESPACE Scid::
    DESTINATION "${SCID_INSTALL_CMAKEDIR}" )

configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/../package/ScidConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ScidConfig.cmake"
    INSTALL_DESTINATION "${SCID_INSTALL_CMAKEDIR}" )

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ScidConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion )

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ScidConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/ScidConfigVersion.cmake"
    DESTINATION "${SCID_INSTALL_CMAKEDIR}" )

install(
    FILES
        "${PROJECT_SOURCE_DIR}/COPYING"
        "${PROJECT_SOURCE_DIR}/README.md"
    DESTINATION "${CMAKE_INSTALL_DOCDIR}" )
