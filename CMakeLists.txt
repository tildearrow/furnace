cmake_minimum_required(VERSION 3.0)

if (APPLE)
  set(MACOSX_DEPLOYMENT_TARGET 10.9)
endif()

project(furnace)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_PROJECT_VERSION_MAJOR 0)
set(CMAKE_PROJECT_VERSION_MINOR 5)
set(CMAKE_PROJECT_VERSION_PATCH 1)

if (ANDROID)
  set(BUILD_GUI_DEFAULT OFF)
else()
  set(BUILD_GUI_DEFAULT ON)
endif()

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(JACK jack)
  set(WITH_JACK_DEFAULT ${JACK_FOUND})
else()
  set(WITH_JACK_DEFAULT OFF)
endif()

option(BUILD_GUI "Build the tracker (disable to build only a headless player)" ${BUILD_GUI_DEFAULT})
option(WITH_JACK "Whether to build with JACK support. Auto-detects if JACK is available" ${WITH_JACK_DEFAULT})
option(SYSTEM_FMT "Use a system-installed version of fmt instead of the vendored one" OFF)
option(SYSTEM_LIBSNDFILE "Use a system-installed version of libsndfile instead of the vendored one" OFF)
option(SYSTEM_ZLIB "Use a system-installed version of zlib instead of the vendored one" OFF)
option(SYSTEM_SDL2 "Use a system-installed version of SDL2 instead of the vendored one" OFF)
option(WARNINGS_ARE_ERRORS "Whether warnings in furnace's C++ code should be treated as errors (ignored on MSVC)" OFF)

set(DEPENDENCIES_CFLAGS "")
set(DEPENDENCIES_LDFLAGS "")

find_package(Threads REQUIRED)
list(APPEND DEPENDENCIES_LDFLAGS ${CMAKE_THREAD_LIBS_INIT})

if (SYSTEM_FMT)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(FMT fmt)
    if (FMT_FOUND)
      list(APPEND DEPENDENCIES_CFLAGS ${FMT_CFLAGS})
      list(APPEND DEPENDENCIES_LDFLAGS ${FMT_LDFLAGS})
    endif()
  endif()
  if (NOT FMT_FOUND)
    find_package(fmt REQUIRED)
    list(APPEND DEPENDENCIES_LDFLAGS fmt::fmt)
  endif()
  message(STATUS "Using system-installed fmt")
else()
  add_subdirectory(extern/fmt EXCLUDE_FROM_ALL)
  include_directories(extern/fmt/include)
  list(APPEND DEPENDENCIES_LDFLAGS fmt)
  message(STATUS "Using vendored fmt")
endif()

if (SYSTEM_LIBSNDFILE)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(LIBSNDFILE REQUIRED sndfile)
  list(APPEND DEPENDENCIES_CFLAGS ${LIBSNDFILE_CFLAGS})
  list(APPEND DEPENDENCIES_LDFLAGS ${LIBSNDFILE_LDFLAGS})
  message(STATUS "Using system-installed libsndfile")
else()
  set(BUILD_TESTING OFF CACHE BOOL "aaaaaa" FORCE)
  set(BUILD_PROGRAMS OFF CACHE BOOL "aaa" FORCE)
  set(BUILD_EXAMPLES OFF CACHE BOOL "a" FORCE)
  set(ENABLE_EXTERNAL_LIBS OFF CACHE BOOL "come on" FORCE)
  set(ENABLE_MPEG OFF CACHE BOOL "come on" FORCE)
  add_subdirectory(extern/libsndfile EXCLUDE_FROM_ALL)
  list(APPEND DEPENDENCIES_LDFLAGS sndfile)
  message(STATUS "Using vendored libsndfile")
endif()

if (SYSTEM_ZLIB)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(ZLIB REQUIRED zlib)
  list(APPEND DEPENDENCIES_CFLAGS ${ZLIB_CFLAGS})
  list(APPEND DEPENDENCIES_LDFLAGS ${ZLIB_LDFLAGS})
  message(STATUS "Using system-installed zlib")
else()
  set(BUILD_TESTING OFF CACHE BOOL "aaaaaa" FORCE)
  set(BUILD_PROGRAMS OFF CACHE BOOL "aaa" FORCE)
  set(BUILD_EXAMPLES OFF CACHE BOOL "a" FORCE)
  set(ENABLE_EXTERNAL_LIBS OFF CACHE BOOL "come on" FORCE)
  set(ENABLE_MPEG OFF CACHE BOOL "come on" FORCE)
  add_subdirectory(extern/zlib EXCLUDE_FROM_ALL)
  include_directories(extern/zlib ${CMAKE_CURRENT_BINARY_DIR}/extern/zlib)
  list(APPEND DEPENDENCIES_LDFLAGS zlibstatic)
  message(STATUS "Using vendored zlib")
endif()

if (SYSTEM_SDL2)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(SDL sdl>=2.0.18)
    if (SDL_FOUND)
      list(APPEND DEPENDENCIES_CFLAGS ${SDL_CFLAGS})
      list(APPEND DEPENDENCIES_LDFLAGS ${SDL_LDFLAGS})
    endif()
  endif()
  if (NOT SDL_FOUND)
    find_package(SDL 2.0.18)
    if (SDL_FOUND)
      include_directories(${SDL_INCLUDE_DIR})
      list(APPEND DEPENDENCIES_LDFLAGS ${SDL_LIBRARY})
    else()
      if (PKG_CONFIG_FOUND)
        pkg_check_modules(SDL2 sdl2>=2.0.18)
        if (SDL2_FOUND)
          list(APPEND DEPENDENCIES_CFLAGS ${SDL2_CFLAGS})
          list(APPEND DEPENDENCIES_LDFLAGS ${SDL2_LDFLAGS})
        endif()
      endif()
      if (NOT SDL2_FOUND)
        find_package(SDL2 2.0.18 REQUIRED)
        include_directories(${SDL2_INCLUDE_DIRS})
        list(APPEND DEPENDENCIES_LDFLAGS ${SDL2_LIBRARIES})
      endif()
    endif()
  endif()
  message(STATUS "Using system-installed SDL2")
else()
  set(SDL_SHARED OFF)
  set(SDL_STATIC ON)
  add_subdirectory(extern/SDL EXCLUDE_FROM_ALL)
  include_directories(extern/SDL/include)
  list(APPEND DEPENDENCIES_LDFLAGS SDL2-static)
  message(STATUS "Using vendored SDL2")
endif()

if(BUILD_GUI)
  include_directories(extern/imgui extern/IconFontCppHeaders extern/imgui/backends extern/igfd)
  message(STATUS "Building GUI")
else()
  message(STATUS "Building headless")
endif()

set(AUDIO_SOURCES src/audio/abstract.cpp src/audio/sdl.cpp)

if(WITH_JACK)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(JACK REQUIRED jack)
  list(APPEND DEPENDENCIES_CFLAGS ${JACK_CFLAGS})
  list(APPEND DEPENDENCIES_LDFLAGS ${JACK_LDFLAGS})
  list(APPEND AUDIO_SOURCES src/audio/jack.cpp)
  message(STATUS "Building with JACK support")
else()
  message(STATUS "Building without JACK support")
endif()

if(NOT MSVC)
  set(WARNING_FLAGS -Wall -Wextra -Wno-unused-parameter)
  if (WARNINGS_ARE_ERRORS)
    list(APPEND WARNING_FLAGS -Werror)
  endif()
  # Nicer but cannot be narrowed down to just C++
  # target_compile_options(furnace PRIVATE ${WARNING_FLAGS})
  string(REPLACE ";" " " WARNING_FLAGS_STRING "${WARNING_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS_STRING}")
endif()

set(ENGINE_SOURCES
src/log.cpp
src/fileutils.cpp
src/utfutils.cpp

extern/Nuked-OPN2/ym3438.c
extern/opm/opm.c
src/engine/platform/sound/sn76496.cpp
src/engine/platform/sound/ay8910.cpp
src/engine/platform/sound/saa1099.cpp
src/engine/platform/sound/gb/apu.c
src/engine/platform/sound/gb/timing.c
src/engine/platform/sound/pce_psg.cpp
src/engine/platform/sound/nes/apu.c

src/engine/platform/sound/c64/sid.cc
src/engine/platform/sound/c64/voice.cc
src/engine/platform/sound/c64/wave.cc
src/engine/platform/sound/c64/envelope.cc
src/engine/platform/sound/c64/filter.cc
src/engine/platform/sound/c64/extfilt.cc
src/engine/platform/sound/c64/pot.cc
src/engine/platform/sound/c64/version.cc

src/engine/platform/sound/c64/wave6581_PS_.cc
src/engine/platform/sound/c64/wave6581_PST.cc
src/engine/platform/sound/c64/wave6581_P_T.cc
src/engine/platform/sound/c64/wave6581__ST.cc
src/engine/platform/sound/c64/wave8580_PS_.cc
src/engine/platform/sound/c64/wave8580_PST.cc
src/engine/platform/sound/c64/wave8580_P_T.cc
src/engine/platform/sound/c64/wave8580__ST.cc

src/engine/platform/sound/tia/TIASnd.cpp

src/engine/platform/sound/ymfm/ymfm_adpcm.cpp
src/engine/platform/sound/ymfm/ymfm_opm.cpp
src/engine/platform/sound/ymfm/ymfm_opn.cpp
src/engine/platform/sound/ymfm/ymfm_ssg.cpp

src/engine/platform/ym2610Interface.cpp

src/engine/blip_buf.c
src/engine/safeReader.cpp
src/engine/safeWriter.cpp
src/engine/dispatchContainer.cpp
src/engine/engine.cpp
src/engine/instrument.cpp
src/engine/macroInt.cpp
src/engine/pattern.cpp
src/engine/playback.cpp
src/engine/sample.cpp
src/engine/song.cpp
src/engine/wavetable.cpp
src/engine/platform/abstract.cpp
src/engine/platform/genesis.cpp
src/engine/platform/genesisext.cpp
src/engine/platform/sms.cpp
src/engine/platform/gb.cpp
src/engine/platform/pce.cpp
src/engine/platform/nes.cpp
src/engine/platform/c64.cpp
src/engine/platform/arcade.cpp
src/engine/platform/ym2610.cpp
src/engine/platform/ym2610ext.cpp
src/engine/platform/ay.cpp
src/engine/platform/ay8930.cpp
src/engine/platform/tia.cpp
src/engine/platform/saa.cpp
src/engine/platform/amiga.cpp
src/engine/platform/dummy.cpp)

set(GUI_SOURCES
extern/imgui/imgui.cpp
extern/imgui/imgui_draw.cpp
extern/imgui/imgui_tables.cpp
extern/imgui/imgui_widgets.cpp
extern/imgui/backends/imgui_impl_sdlrenderer.cpp
extern/imgui/backends/imgui_impl_sdl.cpp
extern/imgui/misc/cpp/imgui_stdlib.cpp
extern/igfd/ImGuiFileDialog.cpp

src/gui/plot_nolerp.cpp
src/gui/font_exo.cpp
src/gui/font_liberationSans.cpp
src/gui/font_mononoki.cpp
src/gui/font_plexMono.cpp
src/gui/font_plexSans.cpp
src/gui/font_proggyClean.cpp
src/gui/font_ptMono.cpp
src/gui/font_unifont.cpp
src/gui/font_icon.cpp
src/gui/fonts.cpp
src/gui/debug.cpp
src/gui/gui.cpp)

if (NOT WIN32 AND NOT APPLE)
  list(APPEND GUI_SOURCES src/gui/icon.c)
endif()

if (WIN32)
  list(APPEND ENGINE_SOURCES src/engine/winStuff.cpp)
  list(APPEND ENGINE_SOURCES res/furnace.rc)
endif()

set(USED_SOURCES ${ENGINE_SOURCES} ${AUDIO_SOURCES} src/main.cpp)

if (BUILD_GUI)
  list(APPEND USED_SOURCES ${GUI_SOURCES})
  if (MSVC)
    add_executable(furnace WIN32 ${USED_SOURCES})
  else()
    add_executable(furnace ${USED_SOURCES})
  endif()
  target_compile_definitions(furnace PUBLIC HAVE_GUI)
else()
  add_executable(furnace ${USED_SOURCES})
endif()

target_compile_options(furnace PRIVATE ${DEPENDENCIES_CFLAGS})
target_link_libraries(furnace ${DEPENDENCIES_LDFLAGS})

if (WITH_JACK)
  target_compile_definitions(furnace PUBLIC HAVE_JACK)
endif()

if (WIN32)
  target_link_libraries(furnace shlwapi)
  if (NOT MSVC)
    target_link_libraries(furnace -static)
  endif()
endif()

install(TARGETS furnace RUNTIME DESTINATION bin)

if (NOT WIN32 AND NOT APPLE)
  include(GNUInstallDirs)
  install(FILES res/furnace.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
  install(FILES res/furnace.appdata.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
  install(DIRECTORY papers DESTINATION ${CMAKE_INSTALL_DOCDIR})
  install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses)
  install(DIRECTORY demos DESTINATION ${CMAKE_INSTALL_DATADIR}/furnace)
  install(FILES res/logo.png RENAME furnace.png DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps)
endif()

set(CPACK_PACKAGE_NAME "Furnace")
set(CPACK_PACKAGE_VENDOR "tildearrow")
set(CPACK_PACKAGE_DESCRIPTION "free and open-source chiptune tracker")

if (APPLE)
  set(CPACK_GENERATOR Bundle)
  set(CPACK_DMG_SLA_DIR ${CMAKE_SOURCE_DIR}/res/macLicense)
  set(CPACK_DMG_SLA_LANGUAGES en)
  set(CPACK_BUNDLE_NAME "Furnace")
  set(CPACK_DMG_VOLUME_NAME "Furnace")
  set(CPACK_BUNDLE_PLIST ${CMAKE_SOURCE_DIR}/res/Info.plist)
  set(CPACK_BUNDLE_ICON ${CMAKE_SOURCE_DIR}/res/icon.icns)
  set(CPACK_BUNDLE_STARTUP_COMMAND "furnace")
endif()

include(CPack)
