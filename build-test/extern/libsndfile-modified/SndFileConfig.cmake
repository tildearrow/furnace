set(SndFile_VERSION 1.2.2)
set(SndFile_VERSION_MAJOR 1)
set(SndFile_VERSION_MINOR 2)
set(SndFile_VERSION_PATCH 2)

set (SndFile_WITH_EXTERNAL_LIBS 1)
set (SndFile_WITH_MPEG 1)


####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was SndFileConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include (CMakeFindDependencyMacro)

if (NOT OFF)
	list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
endif ()

if (SndFile_WITH_EXTERNAL_LIBS AND NOT OFF)
	find_dependency (Ogg 1.3)
	find_dependency (Vorbis)
	find_dependency (FLAC)
	find_dependency (Opus)
endif ()

if (SndFile_WITH_MPEG AND NOT OFF)
	find_dependency (mp3lame)
	find_dependency (mpg123)
endif ()

if (NOT OFF)
	list (REMOVE_ITEM CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
endif ()

include (${CMAKE_CURRENT_LIST_DIR}/SndFileTargets.cmake)

set_and_check (SndFile_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")
set (SNDFILE_INCLUDE_DIR ${SndFile_INCLUDE_DIR})

set (SndFile_LIBRARY SndFile::sndfile)
set (SNDFILE_LIBRARY SndFile::sndfile)
set (SndFile_LIBRARIES SndFile::sndfile)
set (SNDFILE_LIBRARIES SndFile::sndfile)


check_required_components(SndFile)

set (SNDFILE_FOUND 1)
