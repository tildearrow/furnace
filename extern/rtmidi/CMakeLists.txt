# 2018 (c) Juan G. Victores, Bartek Łukawski, Stephen Sinclair
# CopyPolicy: RtMidi license.

# Set minimum CMake required version for this project.
#
# This floor is set by the Android backend and arrived with Android support in
# a30eef6, without an explanation at the time.  Older CMake fails the Android
# cross-compile because find_package(JNI) resolves JAVA_AWT_LIBRARY and
# JAVA_JVM_LIBRARY to NOTFOUND; newer CMake knows an NDK supplies the JNI
# headers without the desktop Java libraries.  Every other target still
# configures, builds and passes ctest with CMake 3.19.
#
# tildearrow: lowered minimum version so we can support Ubuntu 16.04.
if (ANDROID)
  cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
else()
  cmake_minimum_required(VERSION 3.0...3.24)
endif()

# Define a C++ project.
project(RtMidi LANGUAGES CXX C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# standards version
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Check for Jack (any OS)
#
# When cross-compiling, confine both probes to the target.  pkg-config
# otherwise answers from the host's PKG_CONFIG_PATH and find_library searches
# host paths, so a JACK installed on the build machine switches the backend on
# for a target whose sysroot has no jack/jack.h (issue #383).
find_package(PkgConfig)
if(CMAKE_CROSSCOMPILING)
  # Only consult .pc files under the target sysroot.  If the toolchain has not
  # named one, there is nothing target-specific to find and JACK stays off.
  set(_rtmidi_jack_pkg_libdir)
  foreach(_root ${CMAKE_FIND_ROOT_PATH} ${CMAKE_SYSROOT})
    if(_root)
      list(APPEND _rtmidi_jack_pkg_libdir
           "${_root}/usr/lib/pkgconfig" "${_root}/usr/share/pkgconfig")
    endif()
  endforeach()
  if(_rtmidi_jack_pkg_libdir)
    string(REPLACE ";" ":" _rtmidi_jack_pkg_libdir "${_rtmidi_jack_pkg_libdir}")
    set(ENV{PKG_CONFIG_LIBDIR} "${_rtmidi_jack_pkg_libdir}")
    unset(ENV{PKG_CONFIG_PATH})
    pkg_check_modules(jack jack)
  endif()
  find_library(JACK_LIB jack ONLY_CMAKE_FIND_ROOT_PATH)
else()
  find_library(JACK_LIB jack)
  pkg_check_modules(jack jack)
endif()
if(JACK_LIB OR jack_FOUND)
  set(HAVE_JACK TRUE)
endif()

# Necessary for Windows
if(WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Standard CMake options
option(BUILD_SHARED_LIBS "Build as shared library" ON)

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release;RelWithDebInfo;MinSizeRel")
endif()
if(WINDOWS)
  set(CMAKE_DEBUG_POSTFIX d CACHE STRING "Postfix for debug version of library")
endif()

# Build Options
set(RTMIDI_TARGETNAME_UNINSTALL "rt_uninstall" CACHE STRING "Name of 'uninstall' build target")

# API Options
option(RTMIDI_API_JACK "Compile with JACK support." ${HAVE_JACK})
option(RTMIDI_API_WINMM "Compile with WINMM support." ${WIN32})
option(RTMIDI_API_WMS "Compile with Windows MIDI Services support (the in-box Windows.Devices.Midi2 API, Windows 11 25H2 and later; unavailable elsewhere at run time)." OFF)
option(RTMIDI_API_CORE "Compile with CoreMIDI support." ${APPLE})
# ALSA defaults on when its development files are found, like JACK above.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT ANDROID)
  find_package(ALSA QUIET)
endif()
option(RTMIDI_API_ALSA "Compile with ALSA support." ${ALSA_FOUND})
option(RTMIDI_API_AMIDI "Compile with Android support." ${ANDROID})

# Module options
option(RTMIDI_BUILD_MODULES "Build C++ modules for RtMidi" OFF)
option(RTMIDI_USE_NAMESPACE "Force usage of namespace rt::midi for modules" ON)

# Read libtool version info from configure.ac
set(R "m4_define\\(\\[lt_([a-z]+)\\], ([0-9]+)\\)")
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/configure.ac" CONFIGAC
  REGEX ${R})
foreach(_S ${CONFIGAC})
  string(REGEX REPLACE ${R} "\\1" k ${_S})
  string(REGEX REPLACE ${R} "\\2" v ${_S})
  set(SO_${k} ${v})
endforeach()
math(EXPR SO_current_minus_age "${SO_current} - ${SO_age}")
set(SO_VER "${SO_current_minus_age}")
set(FULL_VER "${SO_current_minus_age}.${SO_age}.${SO_revision}")

# Read package version info from configure.ac
set(R "AC_INIT\\(RtMidi, ([0-9\\.]+),.*\\)")
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/configure.ac" CONFIGAC
  REGEX ${R})
string(REGEX REPLACE ${R} "\\1" PACKAGE_VERSION ${CONFIGAC})

# Init variables
set(rtmidi_SOURCES RtMidi.cpp RtMidi.h rtmidi_c.cpp rtmidi_c.h)
set(LINKLIBS)
set(PUBLICLINKLIBS)
set(INCDIRS)
set(LINKDIRS)
set(PKGCONFIG_REQUIRES)
set(LIBS_REQUIRES)
set(API_DEFS)
# The old names, for rtmidi.pc only, until 7.0.0 removes them (#133).
set(API_DEFS_OLD)
set(API_LIST)
set(PACKAGE_DEPENDENCIES)

# Tweak API-specific configuration.

# Jack
if(RTMIDI_API_JACK)
  if (NOT HAVE_JACK)
    message(FATAL_ERROR "Jack API requested but no Jack dev libraries found")
  endif()
  set(NEED_PTHREAD ON)
  list(APPEND PKGCONFIG_REQUIRES "jack")
  list(APPEND API_DEFS "-DRTMIDI_API_JACK")
  list(APPEND API_DEFS_OLD "-D__UNIX_JACK__")
  list(APPEND API_LIST "jack")
  if(jack_FOUND)
    # jack_LIBRARIES is the bare name ("jack"), so on an installation that
    # keeps libjack somewhere the linker does not search by default -- the
    # pipewire-jack layout, /usr/lib*/pipewire-0.3/jack -- the link fails even
    # though pkg-config reported the right -L.  Carry the directories too
    # (issue #319).
    list(APPEND LINKLIBS ${jack_LIBRARIES})
    list(APPEND LINKDIRS ${jack_LIBRARY_DIRS})
    list(APPEND INCDIRS ${jack_INCLUDE_DIRS})
  else()
    list(APPEND LINKLIBS ${JACK_LIB})
  endif()

  # Check for jack_port_rename
  include(CheckSymbolExists)
  set(tmp_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
  list(APPEND CMAKE_REQUIRED_LIBRARIES jack)
  # Look the header up on the include path rather than building an absolute
  # path from jack_INCLUDEDIR, which is empty when JACK was found by
  # find_library rather than pkg-config.
  set(tmp_CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES})
  list(APPEND CMAKE_REQUIRED_INCLUDES ${jack_INCLUDE_DIRS})
  check_symbol_exists(jack_port_rename jack/jack.h JACK_HAS_PORT_RENAME)
  set(CMAKE_REQUIRED_INCLUDES ${tmp_CMAKE_REQUIRED_INCLUDES})
  set(CMAKE_REQUIRED_LIBRARIES ${tmp_CMAKE_REQUIRED_LIBRARIES})
  if(JACK_HAS_PORT_RENAME)
    list(APPEND API_DEFS "-DRTMIDI_JACK_HAS_PORT_RENAME")
    list(APPEND API_DEFS_OLD "-DJACK_HAS_PORT_RENAME")
  endif()

  # POSIX semaphores let closePort() wait for queued output, as configure.ac does. Check for
  # sem_timedwait(): macOS has sem_init() (it fails at run time) but no sem_timedwait().
  include(CheckIncludeFile)
  include(CheckLibraryExists)
  check_include_file(semaphore.h HAVE_SEMAPHORE_H)
  if(HAVE_SEMAPHORE_H)
    check_library_exists(pthread sem_timedwait "" HAVE_SEMAPHORE)
  endif()
  if(NOT HAVE_SEMAPHORE)
    message(WARNING "POSIX semaphore support not found; data may be lost on closePort")
  endif()
endif()

# ALSA
if(RTMIDI_API_ALSA)
  find_package(ALSA)
  if(NOT ALSA_FOUND)
    message(FATAL_ERROR "RTMIDI_API_ALSA is ON but ALSA was not found. Install its development files (e.g. libasound2-dev or alsa-lib-devel).")
  endif()
  set(NEED_PTHREAD ON)
  list(APPEND INCDIRS ${ALSA_INCLUDE_DIR})
  list(APPEND LINKLIBS ALSA::ALSA)
  list(APPEND PKGCONFIG_REQUIRES "alsa")
  list(APPEND API_DEFS "-DRTMIDI_API_ALSA")
  list(APPEND API_DEFS_OLD "-D__LINUX_ALSA__")
  list(APPEND API_LIST "alsa")
  list(APPEND PACKAGE_DEPENDENCIES "find_dependency(ALSA)")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT ANDROID AND NOT ALSA_FOUND)
  message(WARNING "Building without ALSA: its development files were not found (e.g. libasound2-dev or alsa-lib-devel).")
endif()

# WinMM
if(RTMIDI_API_WINMM)
  list(APPEND API_DEFS "-DRTMIDI_API_WINMM")
  list(APPEND API_DEFS_OLD "-D__WINDOWS_MM__")
  list(APPEND API_LIST "winmm")
  list(APPEND LINKLIBS winmm)
endif()

# Windows MIDI Services
if(RTMIDI_API_WMS)
  if(NOT WIN32)
    message(FATAL_ERROR "RTMIDI_API_WMS is only supported on Windows")
  endif()
  if(NOT MSVC)
    message(FATAL_ERROR "RTMIDI_API_WMS requires the MSVC toolset (C++/WinRT)")
  endif()

  # C++/WinRT needs C++20 coroutines; VS 2026 rejects the experimental ones
  if(CMAKE_CXX_STANDARD VERSION_LESS 20)
    set(CMAKE_CXX_STANDARD 20)
  endif()

  find_package(CppWinRT REQUIRED)
  find_package(WindowsMidiServices REQUIRED)

  list(APPEND INCDIRS "${CppWinRT_INCLUDE_DIR}")

  # A Windows SDK that includes the API already provides its projection;
  # otherwise generate it from Windows.Devices.Midi2.winmd.
  if(NOT WindowsMidiServices_SDK_PROJECTION)
    set(RTMIDI_WMS_PROJECTION_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/winrt")
    set(RTMIDI_WMS_GENERATED_HEADER "${RTMIDI_WMS_PROJECTION_DIR}/winrt/Windows.Devices.Midi2.h")

    add_custom_command(
      OUTPUT "${RTMIDI_WMS_GENERATED_HEADER}"
      COMMAND ${CMAKE_COMMAND} -E make_directory "${RTMIDI_WMS_PROJECTION_DIR}"
      COMMAND "${CppWinRT_EXECUTABLE}" -input "${WindowsMidiServices_WINMD}" -reference "${CppWinRT_WINDOWS_WINMD}" -output "${RTMIDI_WMS_PROJECTION_DIR}"
      DEPENDS "${WindowsMidiServices_WINMD}" "${CppWinRT_WINDOWS_WINMD}"
      COMMENT "Generating Windows MIDI Services C++/WinRT projections"
      VERBATIM)

    add_custom_target(rtmidi_generate_wms_projection DEPENDS "${RTMIDI_WMS_GENERATED_HEADER}")
    list(APPEND INCDIRS "${RTMIDI_WMS_PROJECTION_DIR}")
  endif()

  list(APPEND API_DEFS "-DRTMIDI_API_WMS")
  list(APPEND API_DEFS_OLD "-D__WINDOWS_MIDI_SERVICES__")
  list(APPEND API_LIST "wms")
  list(APPEND LINKLIBS windowsapp runtimeobject ole32)

  # Optional: use the zero-allocation COM raw send path instead of WinRT
  option(RTMIDI_USE_WMS_COM_RAW
    "Use IMidiEndpointConnectionRaw (COM, zero-allocation) instead of WinRT SendMultipleMessagesWordArray"
    OFF)
  if(RTMIDI_USE_WMS_COM_RAW)
    list(APPEND API_DEFS "-DRTMIDI_USE_WMS_COM_RAW")
  endif()
endif()

# CoreMIDI
if(RTMIDI_API_CORE)
  find_library(CORESERVICES_LIB CoreServices)
  find_library(COREAUDIO_LIB CoreAudio)
  find_library(COREMIDI_LIB CoreMIDI)
  find_library(COREFOUNDATION_LIB CoreFoundation)
  list(APPEND API_DEFS "-DRTMIDI_API_CORE")
  list(APPEND API_DEFS_OLD "-D__MACOSX_CORE__")
  list(APPEND API_LIST "coremidi")
  list(APPEND LINKLIBS ${CORESERVICES_LIB} ${COREAUDIO_LIB} ${COREMIDI_LIB} ${COREFOUNDATION_LIB})
  list(APPEND LIBS_REQUIRES "-framework CoreServices -framework CoreAudio -framework CoreMIDI -framework CoreFoundation")
  list(APPEND LINKFLAGS "-Wl,-F/Library/Frameworks")
endif()

# Android AMIDI
if(RTMIDI_API_AMIDI)
  set(NEED_PTHREAD ON)
  list(APPEND API_DEFS "-DRTMIDI_API_AMIDI")
  list(APPEND API_DEFS_OLD "-D__AMIDI__")
  list(APPEND API_LIST "amidi")
  # The JVM is reached through dlsym() rather than by linking libnativehelper,
  # which the NDK ships only from API 31 while AMidi is available from API 29.
  list(APPEND LINKLIBS log amidi dl)
endif()

# pthread
if(NEED_PTHREAD)
  find_package(Threads REQUIRED
    CMAKE_THREAD_PREFER_PTHREAD
    THREADS_PREFER_PTHREAD_FLAG)
  list(APPEND PUBLICLINKLIBS Threads::Threads)
  list(APPEND PACKAGE_DEPENDENCIES "find_dependency(Threads)")
endif()

# Create library targets.
set(LIB_TARGETS)

# Use RTMIDI_BUILD_SHARED_LIBS / RTMIDI_BUILD_STATIC_LIBS if they
# are defined, otherwise default to standard BUILD_SHARED_LIBS.
if(DEFINED RTMIDI_BUILD_SHARED_LIBS AND NOT RTMIDI_BUILD_SHARED_LIBS STREQUAL "")
  if(RTMIDI_BUILD_SHARED_LIBS)
    add_library(rtmidi SHARED ${rtmidi_SOURCES})
  else()
    add_library(rtmidi STATIC ${rtmidi_SOURCES})
  endif()
elseif(DEFINED RTMIDI_BUILD_STATIC_LIBS AND NOT RTMIDI_BUILD_STATIC_LIBS STREQUAL "")
  if(RTMIDI_BUILD_STATIC_LIBS)
    add_library(rtmidi STATIC ${rtmidi_SOURCES})
  else()
    add_library(rtmidi SHARED ${rtmidi_SOURCES})
  endif()
else()
  add_library(rtmidi ${rtmidi_SOURCES})
endif()
list(APPEND LIB_TARGETS rtmidi)

if(TARGET rtmidi_generate_wms_projection)
  add_dependencies(rtmidi rtmidi_generate_wms_projection)
endif()

# Add -Wall if possible
if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  target_compile_options(rtmidi PRIVATE -Wall)
endif()

# Add debug flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DRTMIDI_DEBUG)
  if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_compile_options(rtmidi PRIVATE -Werror)
  endif()
endif()

# Add headers destination for install rule.
set_property(TARGET rtmidi PROPERTY PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/RtMidi.h" "${CMAKE_CURRENT_SOURCE_DIR}/rtmidi_c.h")
set_target_properties(rtmidi PROPERTIES
  SOVERSION ${SO_VER}
  VERSION ${FULL_VER})

# Set standard installation directories.
include(GNUInstallDirs)

# Set include paths, populate target interface.
target_include_directories(rtmidi PRIVATE ${INCDIRS}
                                  PUBLIC
                                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                                    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/rtmidi>)

# Set compile-time definitions
target_compile_definitions(rtmidi PRIVATE ${API_DEFS})
target_compile_definitions(rtmidi PRIVATE RTMIDI_EXPORT)
if(HAVE_SEMAPHORE)
  target_compile_definitions(rtmidi PRIVATE HAVE_SEMAPHORE)
endif()
if(LINKDIRS)
  target_link_directories(rtmidi PRIVATE ${LINKDIRS})
endif()
target_link_libraries(rtmidi PUBLIC ${PUBLICLINKLIBS}
                             PRIVATE ${LINKLIBS})

if(RTMIDI_BUILD_MODULES)
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.28)
    message(STATUS "Building rtmidi C++ module")
    add_subdirectory(modules)
  else()
    message(WARNING "Skipping rtmidi C++ module (requires CMake 3.28+, found ${CMAKE_VERSION})")
  endif()
endif()

# Compile and run hotplug regression tests whenever the ALSA library is built.
if("alsa" IN_LIST API_LIST)
  include(tests/AlsaHotplug.cmake)
  rtmidi_add_hotplug_test(rtmidi "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# Add tests if requested.
option(RTMIDI_BUILD_TESTING "Build test programs" ON)
if (NOT DEFINED RTMIDI_BUILD_TESTING OR RTMIDI_BUILD_TESTING STREQUAL "")
  set(RTMIDI_BUILD_TESTING ${BUILD_TESTING})
endif()
if(RTMIDI_BUILD_TESTING)
  include(CTest)
  add_executable(cmidiin    tests/cmidiin.cpp)
  add_executable(midiclock  tests/midiclock.cpp)
  add_executable(midiout    tests/midiout.cpp)
  add_executable(midiprobe  tests/midiprobe.cpp)
  add_executable(qmidiin    tests/qmidiin.cpp)
  add_executable(sysextest  tests/sysextest.cpp)
  add_executable(sysexchunked tests/sysexchunked.cpp)
  add_executable(apinames   tests/apinames.cpp)
  add_executable(testcapi   tests/testcapi.c)
  add_executable(cgetmessage tests/cgetmessage.c)
  add_executable(sysexinterleaved tests/sysexinterleaved.cpp)
  list(GET LIB_TARGETS 0 LIBRTMIDI)
  set_target_properties(cmidiin midiclock midiout midiprobe qmidiin sysextest sysexchunked apinames testcapi cgetmessage sysexinterleaved
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY tests
               INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}
               LINK_LIBRARIES ${LIBRTMIDI})
  add_test(NAME apinames COMMAND apinames)
  # Skipped (exit 77) where there is no MIDI loopback; see tests/cgetmessage.c.
  add_test(NAME cgetmessage COMMAND cgetmessage)
  set_tests_properties(cgetmessage PROPERTIES SKIP_RETURN_CODE 77)
  add_test(NAME sysexinterleaved COMMAND sysexinterleaved)
  set_tests_properties(sysexinterleaved PROPERTIES SKIP_RETURN_CODE 77)
endif()

# Set standard installation directories.
include(GNUInstallDirs)

# Message
string(REPLACE ";" " " apilist "${API_LIST}")
message(STATUS "Compiling with support for: ${apilist}")

# PkgConfig file
string(REPLACE ";" " " req "${PKGCONFIG_REQUIRES}")
string(REPLACE ";" " " req_libs "${LIBS_REQUIRES}")
string(REPLACE ";" " " api "${API_DEFS}")
string(REPLACE ";" " " api_old "${API_DEFS_OLD}")
set(prefix ${CMAKE_INSTALL_PREFIX})
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/rtmidi.pc.in" "rtmidi.pc" @ONLY)

# Install rules.  A project that vendors RtMidi as a subdirectory usually does
# not want RtMidi's files in its own `make install`, so they can be turned off
# wholesale.  On by default, so a standalone build is unchanged.
option(RTMIDI_INSTALL "Generate install rules for RtMidi" ON)

if(RTMIDI_INSTALL)
  install(TARGETS ${LIB_TARGETS}
          EXPORT RtMidiTargets
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
          PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rtmidi)
endif()

# Set installation path for CMake files.
set(RTMIDI_CMAKE_DESTINATION share/rtmidi)

# Store the package in the user registry, and export the target set for the
# build tree.  Both describe the set created by install(TARGETS ... EXPORT)
# above, so they follow the same option: exporting a set that was never
# installed is a configure error.
if(RTMIDI_INSTALL)
  export(PACKAGE RtMidi)

  # Export library target (build-tree).
  export(EXPORT RtMidiTargets
         NAMESPACE RtMidi::)
endif()

# Export library target (install-tree).  This exports the set created by the
# install(TARGETS ... EXPORT RtMidiTargets) above, so it has to follow the same
# option: exporting a set that was never installed is a configure error.
if(RTMIDI_INSTALL)
  install(EXPORT RtMidiTargets
          DESTINATION ${RTMIDI_CMAKE_DESTINATION}
          NAMESPACE RtMidi::)
endif()

# Configure uninstall target.
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/RtMidiConfigUninstall.cmake.in"
    "${CMAKE_BINARY_DIR}/RtMidiConfigUninstall.cmake" @ONLY)

# Create uninstall target.
# Guard against the target already existing: when RtMidi is added as a
# subproject alongside another library that defines its own 'uninstall'
# target, redefining it is an error (fatal as of CMake 4.1).
if(NOT TARGET ${RTMIDI_TARGETNAME_UNINSTALL})
  add_custom_target(${RTMIDI_TARGETNAME_UNINSTALL}
      COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/RtMidiConfigUninstall.cmake)
endif()

if(RTMIDI_INSTALL)
  install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/rtmidi.pc
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

# Set up CMake package
include(CMakePackageConfigHelpers)

# Write cmake package version file
# The package version is the release (6.1.0), not the library's SO version.
write_basic_package_version_file(
    rtmidi-config-version.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMajorVersion
)

string(REPLACE ";" "\n" package_dependencies "${PACKAGE_DEPENDENCIES}")

# Write cmake package config file
configure_package_config_file(
    cmake/rtmidi-config.cmake.in
    rtmidi-config.cmake
    INSTALL_DESTINATION "${RTMIDI_CMAKE_DESTINATION}"
)

# Install package files
if(RTMIDI_INSTALL)
  install(
      FILES
          "${CMAKE_CURRENT_BINARY_DIR}/rtmidi-config.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/rtmidi-config-version.cmake"
      DESTINATION
          "${RTMIDI_CMAKE_DESTINATION}"
  )
endif()
