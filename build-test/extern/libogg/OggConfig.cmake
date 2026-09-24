
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was OggConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

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

set(Ogg_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")
set(OGG_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")
set(Ogg_INCLUDE_DIRS "${PACKAGE_PREFIX_DIR}/include")
set(OGG_INCLUDE_DIRS "${PACKAGE_PREFIX_DIR}/include")

include(${CMAKE_CURRENT_LIST_DIR}/OggTargets.cmake)

set(Ogg_LIBRARY Ogg::ogg)
set(OGG_LIBRARY Ogg::ogg)
set(Ogg_LIBRARIES Ogg::ogg)
set(OGG_LIBRARIES Ogg::ogg)

check_required_components(Ogg)
set(OGG_FOUND 1)
