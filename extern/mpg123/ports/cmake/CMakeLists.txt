cmake_minimum_required(VERSION 3.12)

include(cmake/read_api_version.cmake)
read_api_version(MPG123_VERSION)

project(mpg123 VERSION ${MPG123_VERSION} LANGUAGES C ASM)
set(CMAKE_C_STANDARD 99)

option(BUILD_LIBOUT123 "build libout123 (prerequisite for included programs)" ON)
if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
    message(WARNING "Output module is not implemented for '${CMAKE_SYSTEM_NAME}' platform.")
    message(WARNING "To prevent build errors 'BUILD_LIBOUT123' option is set to OFF.")
    set(BUILD_LIBOUT123 OFF)
endif()

include(cmake/search_libs.cmake)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(PACKAGE_VERSION ${PROJECT_VERSION})
add_subdirectory("src")

install(
    EXPORT targets
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    NAMESPACE MPG123::)

configure_package_config_file(
    mpg123-config.cmake.in mpg123-config.cmake
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
write_basic_package_version_file(
    mpg123-config-version.cmake COMPATIBILITY AnyNewerVersion)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/mpg123-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/mpg123-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

set(prefix "${CMAKE_INSTALL_PREFIX}")
set(exec_prefix "${CMAKE_INSTALL_PREFIX}")
set(libdir "${CMAKE_INSTALL_FULL_LIBDIR}")
set(includedir "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/../../libmpg123.pc.in" libmpg123.pc @ONLY)
if(BUILD_LIBOUT123)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/../../libout123.pc.in" libout123.pc @ONLY)
endif()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/../../libsyn123.pc.in" libsyn123.pc @ONLY)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/libmpg123.pc"
        "${CMAKE_CURRENT_BINARY_DIR}/libsyn123.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
if(BUILD_LIBOUT123)
    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/libout123.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif()
