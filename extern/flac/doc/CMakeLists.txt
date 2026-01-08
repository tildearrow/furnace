option(BUILD_DOXYGEN "Enable API documentation building via Doxygen if not already present" ON)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/api")

    find_package(Doxygen)

    if (NOT DOXYGEN_FOUND)
        return()
    endif()

    if (NOT BUILD_DOXYGEN)
        return()
    endif()

    set(top_srcdir "${PROJECT_SOURCE_DIR}")
    configure_file(Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)


    add_custom_target(FLAC-doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )

    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doxytmp/html/"
            DESTINATION "${CMAKE_INSTALL_DOCDIR}/api")
else()
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/api/"
            DESTINATION "${CMAKE_INSTALL_DOCDIR}/api")
endif()
