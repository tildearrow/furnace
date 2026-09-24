# - Find lame
# Find the native lame includes and libraries
#
#  MP3LAME_INCLUDE_DIRS - where to find lame.h, etc.
#  MP3LAME_LIBRARIES    - List of libraries when using lame.
#  MP3LAME_FOUND        - True if Lame found.

if (MP3LAME_INCLUDE_DIR)
    # Already in cache, be silent
    set(MP3LAME_FIND_QUIETLY TRUE)
endif ()

find_path (MP3LAME_INCLUDE_DIR lame/lame.h
	HINTS
		${LAME_ROOT}
	)

# MSVC built lame may be named mp3lame_static.
# The provided project files name the library with the lib prefix.

find_library (MP3LAME_LIBRARY
	NAMES
		mp3lame
		mp3lame_static
		libmp3lame
		libmp3lame_static
		libmp3lame-static
	HINTS
		${MP3LAME_ROOT}
	)

find_library (MP3LAME_HIP_LIBRARY
	NAMES
		mpghip-static
		libmpghip-static
	HINTS
		${MP3LAME_ROOT}
	)

# Handle the QUIETLY and REQUIRED arguments and set LAME_FOUND
# to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (mp3lame
	REQUIRED_VARS
		MP3LAME_LIBRARY
		MP3LAME_INCLUDE_DIR
	)

if (MP3LAME_FOUND)
	set (MP3LAME_LIBRARIES ${MP3LAME_LIBRARY} ${MP3LAME_HIP_LIBRARY})
	set (MP3LAME_INCLUDE_DIRS ${MP3LAME_INCLUDE_DIR})

	if (NOT TARGET mp3lame::mp3lame)
		add_library (mp3lame::mp3lame UNKNOWN IMPORTED)
		set_target_properties (mp3lame::mp3lame PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${MP3LAME_INCLUDE_DIRS}"
			IMPORTED_LOCATION "${MP3LAME_LIBRARY}"
		)
		if (MP3LAME_HIP_LIBRARY AND (NOT TARGET mp3lame::mpghip))
			add_library (mp3lame::mpghip STATIC IMPORTED)
			set_property (mp3lame::mpghip PROPERTY IMPORTED_LOCATION "${MP3LAME_HIP_LIBRARY}")
			set_property (TARGET mp3lame::mp3lame PROPERTY INTERFACE_LINK_LIBRARIES "mp3lame::mpghip")
		endif ()
	endif ()
endif ()

mark_as_advanced(MP3LAME_INCLUDE_DIR MP3LAME_LIBRARY MP3LAME_HIP_LIBRARY)
