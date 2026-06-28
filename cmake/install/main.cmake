include( CMakePackageConfigHelpers )

set( LIBSCID_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/libscid" )
set(
    LIBSCID_INSTALL_DOCDIR
    "${CMAKE_INSTALL_DATAROOTDIR}/doc/libscid"
    CACHE PATH "Documentation install directory for the libscid package." )

set_property( TARGET scid PROPERTY EXPORT_NAME LibScid )

install(
    TARGETS
        scid
    EXPORT LibScidTargets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    FILE_SET public_headers DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" )

install(
    EXPORT LibScidTargets
    NAMESPACE LibScid::
    FILE libscidTargets.cmake
    DESTINATION "${LIBSCID_INSTALL_CMAKEDIR}" )

configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/../package/libscidConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/libscidConfig.cmake"
    INSTALL_DESTINATION "${LIBSCID_INSTALL_CMAKEDIR}" )

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/libscidConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion )

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/libscidConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/libscidConfigVersion.cmake"
    DESTINATION "${LIBSCID_INSTALL_CMAKEDIR}" )

install(
    FILES
        "${PROJECT_SOURCE_DIR}/COPYING"
        "${PROJECT_SOURCE_DIR}/README.md"
    DESTINATION "${LIBSCID_INSTALL_DOCDIR}" )

install(
    FILES "${PROJECT_SOURCE_DIR}/examples/CMakeLists.txt"
    DESTINATION "${LIBSCID_INSTALL_DOCDIR}/examples" )

install(
    DIRECTORY
        "${PROJECT_SOURCE_DIR}/examples/libscid"
        "${PROJECT_SOURCE_DIR}/examples/fixtures"
    DESTINATION "${LIBSCID_INSTALL_DOCDIR}/examples" )

if( LIBSCID_BUILD_DOCS )
    install(
        DIRECTORY "${LIBSCID_DOXYGEN_OUTPUT_DIR}/html/"
        DESTINATION "${LIBSCID_INSTALL_DOCDIR}/html" )
endif()
