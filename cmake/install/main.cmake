include( CMakePackageConfigHelpers )

set( LIBSCID_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/libscid-cpp" )
set(
    LIBSCID_INSTALL_DOCDIR
    "${CMAKE_INSTALL_DATAROOTDIR}/doc/libscid-cpp"
    CACHE PATH "Documentation install directory for the libscid-cpp package." )

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
    EXPORT LibScidCppTargets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    FILE_SET public_headers DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" )

install(
    TARGETS
        scid
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    FILE_SET public_headers DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" )

install(
    EXPORT LibScidCppTargets
    NAMESPACE LibScidCpp::
    FILE libscid-cppTargets.cmake
    DESTINATION "${LIBSCID_INSTALL_CMAKEDIR}" )

configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/../package/libscid-cppConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/libscid-cppConfig.cmake"
    INSTALL_DESTINATION "${LIBSCID_INSTALL_CMAKEDIR}" )

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/libscid-cppConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion )

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/libscid-cppConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/libscid-cppConfigVersion.cmake"
    DESTINATION "${LIBSCID_INSTALL_CMAKEDIR}" )

install(
    FILES
        "${PROJECT_SOURCE_DIR}/COPYING"
        "${PROJECT_SOURCE_DIR}/README.md"
    DESTINATION "${LIBSCID_INSTALL_DOCDIR}" )

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/examples/"
    DESTINATION "${LIBSCID_INSTALL_DOCDIR}/examples" )

if( LIBSCID_BUILD_DOCS )
    install(
        DIRECTORY "${LIBSCID_DOXYGEN_OUTPUT_DIR}/html/"
        DESTINATION "${LIBSCID_INSTALL_DOCDIR}/html" )
endif()
