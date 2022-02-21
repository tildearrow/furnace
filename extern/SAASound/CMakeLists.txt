cmake_minimum_required(VERSION 3.10)

if (APPLE)
  set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "OSX Architectures")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9")
  option(BUILD_FRAMEWORK "Build a Mac OS X framework instead of a shared library" ON)
endif ()

project(SAASound)

if (MSVC)
  add_definitions(-DUNICODE -D_UNICODE)
endif ()

set(EXTERNAL_CLK_HZ "8000000" CACHE STRING "External clock in Hz")
option(SAAFREQ_FIXED_CLOCKRATE "Use precomputed frequency table, external clock is locked" OFF)
set(SAMPLE_RATE_HZ "44100" CACHE STRING "Output sample rate in Hz")
set(DEFAULT_OVERSAMPLE "6" CACHE STRING "Default oversample factor")
set(DEFAULT_UNBOOSTED_MULTIPLIER "11.3" CACHE STRING "Default scaling factor (before volume boost), not configurable in SAAConfig.cfg")
set(DEFAULT_BOOST "1" CACHE STRING "Default volume boost multiplier, configurable in SAAConfig.cfg")
option(DEBUGSAA "Create a text log of all register:value writes as well as a raw PCM output file" OFF)
set(DEBUG_SAA_REGISTER_LOG "debugsaa.txt" CACHE STRING "Path to register log file (if DEBUGSAA)")
set(DEBUG_SAA_PCM_LOG "debugsaa.pcm" CACHE STRING "Path to raw PCM output file (if DEBUGSAA)")
option(USE_CONFIG_FILE "Compile support for loading a config file on startup (SAAsound.cfg)" ON)
set(CONFIG_FILE_PATH "SAASound.cfg" CACHE STRING "Path to config file (if USE_CONFIG_FILE)")

#include(CheckStructHasMember)
#CHECK_STRUCT_HAS_MEMBER("struct dirent" d_type dirent.h HAVE_STRUCT_DIRENT_D_TYPE LANGUAGE CXX)

if (MSVC)
  set(RESOURCES resources/SAASound.def)
endif ()

add_library(SAASound SHARED)
set(API_HEADERS include/SAASound.h)
target_sources(SAASound
    PRIVATE
        src/defns.h
        src/SAAAmp.cpp
        src/SAAAmp.h
        src/SAADevice.cpp
        src/SAADevice.h
        src/SAAEnv.cpp
        src/SAAEnv.h
        src/SAAFreq.cpp
        src/SAAFreq.h
        src/SAAImpl.cpp
        src/SAAImpl.h
        src/SAANoise.cpp
        src/SAANoise.h
        src/SAASndC.cpp
        src/SAASound.cpp
        src/types.h
        src/SAASndC.h
        ${RESOURCES}
        ${API_HEADERS}
)

if (USE_CONFIG_FILE)
target_sources(SAASound
    PRIVATE
        src/SAAConfig.cpp
        src/SAAConfig.h
        src/minIni/minIni.h
        src/minIni/minIni.c
        src/minIni/minGlue.h
)
endif ()

set_target_properties(SAASound PROPERTIES
  VERSION 3.5
  SOVERSION 3
  CXX_STANDARD 11)

target_include_directories(SAASound PRIVATE
  ${CMAKE_BINARY_DIR}
  PUBLIC
  include)

if (APPLE AND BUILD_FRAMEWORK)
  set_target_properties(SAASound PROPERTIES
    OUTPUT_NAME SAASound
    FRAMEWORK TRUE
    FRAMEWORK_VERSION C
    MACOSX_FRAMEWORK_IDENTIFIER com.beermex)
  set_source_files_properties(${API_HEADERS} PROPERTIES
    MACOSX_PACKAGE_LOCATION Headers)
endif ()

configure_file(src/config.h.in ${CMAKE_BINARY_DIR}/saasound_cmake_config.h)
target_compile_definitions(SAASound PRIVATE HAVE_CONFIG_H=1)

install(TARGETS SAASound
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  FRAMEWORK DESTINATION lib
  PUBLIC_HEADER DESTINATION include)

if (NOT APPLE OR NOT BUILD_FRAMEWORK)
  install(FILES ${API_HEADERS} DESTINATION include)
endif ()
