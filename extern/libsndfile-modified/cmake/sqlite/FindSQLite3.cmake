# - Find SQLite3
# Find the native SQLite3 includes and libraries
#
#  SQLite3_INCLUDE_DIRS - where to find sqlite3.h, etc.
#  SQLite3_LIBRARIES    - List of libraries when using SQLite3.
#  SQLite3_FOUND        - True if SQLite3 found.

if (SQLite3_INCLUDE_DIR)
	# Already in cache, be silent
	set (SQLite3_FIND_QUIETLY TRUE)
endif ()

find_package (PkgConfig QUIET)
pkg_check_modules (PC_SQLite3 QUIET sqlite3)

set (SQLite3_VERSION ${PC_SQLite3_VERSION})

find_path (SQLite3_INCLUDE_DIR sqlite3.h
	HINTS
		${PC_SQLite3_INCLUDEDIR}
		${PC_SQLite3_INCLUDE_DIRS}
		${SQLite3_ROOT}
	)

find_library (SQLite3_LIBRARY
	NAMES
		sqlite3
	HINTS
		${PC_SQLite3_LIBDIR}
		${PC_SQLite3_LIBRARY_DIRS}
		${SQLite3_ROOT}
	)

include (FindPackageHandleStandardArgs)

find_package_handle_standard_args (SQLite3
	REQUIRED_VARS
		SQLite3_LIBRARY
		SQLite3_INCLUDE_DIR
	VERSION_VAR
		SQLite3_VERSION
	)

if (SQLite3_FOUND)
	set (SQLite3_INCLUDE_DIRS ${SQLite3_INCLUDE_DIR})
	set (SQLite3_LIBRARIES ${SQLite3_LIBRARY})
	if (NOT TARGET SQLite::SQLite3)
		add_library (SQLite::SQLite3 UNKNOWN IMPORTED)
		set_target_properties (SQLite::SQLite3 PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${SQLite3_INCLUDE_DIRS}"
			IMPORTED_LOCATION "${SQLite3_LIBRARIES}"
		)
	endif ()
endif ()

mark_as_advanced (SQLite3_INCLUDE_DIR SQLite3_LIBRARY)
