# Inspiration: https://github.com/ros2-dotnet/Fast-RTPS

macro (SETUP_ABI_VERSIONS)

    file (STRINGS ${PROJECT_SOURCE_DIR}/configure.ac CONFIGURE_AC_CONTENT)
    file (STRINGS
        configure.ac
        SNDFILE_LT_CURRENT_TMP
        REGEX "^m4_define\\(\\[?lt_current\\]?, *\\[?[0-9]+\\]?\\)"
        )
    string (REGEX REPLACE "m4_define\\(\\[?lt_current\\]?, *\\[?([0-9]+)\\]?\\)"
        "\\1"
        SNDFILE_LT_CURRENT
        ${SNDFILE_LT_CURRENT_TMP}
        )

    file (STRINGS
        configure.ac
        SNDFILE_LT_REVISION_TMP
        REGEX "^m4_define\\(\\[?lt_revision\\]?, *\\[?[0-9]+\\]?\\)"
        )
    string (REGEX REPLACE "m4_define\\(\\[?lt_revision\\]?, *\\[?([0-9]+)\\]?\\)"
        "\\1"
        SNDFILE_LT_REVISION
        ${SNDFILE_LT_REVISION_TMP}
        )

    file (STRINGS
        configure.ac
        SNDFILE_LT_AGE_TMP
        REGEX "^m4_define\\(\\[?lt_age\\]?, *\\[?[0-9]+\\]?\\)"
        )
    string (REGEX REPLACE "m4_define\\(\\[?lt_age\\]?, *\\[?([0-9]+)\\]?\\)"
        "\\1"
        SNDFILE_LT_AGE
        ${SNDFILE_LT_AGE_TMP}
        )

    #
    # Calculate CMake compatible ABI version from libtool version.
    #

    math (EXPR SNDFILE_ABI_VERSION_MAJOR "${SNDFILE_LT_CURRENT} - ${SNDFILE_LT_AGE}")
    set (SNDFILE_ABI_VERSION_MINOR ${SNDFILE_LT_AGE})
    set (SNDFILE_ABI_VERSION_PATCH ${SNDFILE_LT_REVISION})
    set (SNDFILE_ABI_VERSION "${SNDFILE_ABI_VERSION_MAJOR}.${SNDFILE_ABI_VERSION_MINOR}.${SNDFILE_ABI_VERSION_PATCH}")

    #
    # Apple platform current and compatibility versions.
    #

    math (EXPR SNDFILE_MACHO_CURRENT_VERSION_MAJOR "${SNDFILE_ABI_VERSION_MAJOR} + ${SNDFILE_ABI_VERSION_MINOR} + 1")
    set (SNDFILE_MACHO_CURRENT_VERSION "${SNDFILE_MACHO_CURRENT_VERSION_MAJOR}.${SNDFILE_ABI_VERSION_PATCH}.0")
    set (SNDFILE_MACHO_COMPATIBILITY_VERSION "${SNDFILE_MACHO_CURRENT_VERSION_MAJOR}.0.0")

endmacro (SETUP_ABI_VERSIONS)
