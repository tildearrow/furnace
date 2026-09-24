#[=======================================================================[.rst:
Findmpg123
-------

Finds the mpg123 library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``MPG123::mpg123``
  The mpg123 library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``mpg123_FOUND``
  True if the system has the mpg123 package.
``mpg123_VERSION``
	The version of mpg123 that was found on the system.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``mpg123_INCLUDE_DIR``
  The directory containing ``mpg123.h``.
``mpg123_LIBRARY``
  The path to the mpg123 library.

#]=======================================================================]

if (mpg123_INCLUDE_DIR)
    # Already in cache, be silent
    set(mpg123_FIND_QUIETLY TRUE)
endif ()

# tildearrow: don't use pkg-config for vendored build
find_path (mpg123_INCLUDE_DIR mpg123.h
	HINTS
		${mpg123_ROOT}
	)

# MSVC built mpg123 may be named mpg123_static.
# The provided project files name the library with the lib prefix.

find_library (mpg123_LIBRARY
	NAMES
		mpg123
		mpg123_static
		libmpg123
		libmpg123_static
	HINTS
		${mpg123_ROOT}
	)

if (mpg123_INCLUDE_DIR)
	#file (READ "${mpg123_INCLUDE_DIR}/mpg123.h" _mpg123_h)
	#string (REGEX MATCH "[0-9]+.[0-9]+.[0-9]+" _mpg123_version_re "${_mpg123_h}")
	#set (mpg123_VERSION "${_mpg123_version_re}")
        set (mpg123_VERSION "NO")
endif ()

# Handle the QUIETLY and REQUIRED arguments and set mpg123_FOUND
# to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (mpg123
	REQUIRED_VARS
		mpg123_LIBRARY
		mpg123_INCLUDE_DIR
	)

if (mpg123_FOUND AND NOT TARGET MPG123::mpg123)
	add_library (MPG123::mpg123 UNKNOWN IMPORTED)
	set_target_properties (MPG123::mpg123
		PROPERTIES 
			IMPORTED_LOCATION "${mpg123_LIBRARY}"
			INTERFACE_INCLUDE_DIRECTORIES "${mpg123_INCLUDE_DIR}"
		)
endif ()

mark_as_advanced(mpg123_INCLUDE_DIR mpg123_LIBRARY)
