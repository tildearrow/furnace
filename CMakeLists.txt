cmake_minimum_required(VERSION 3.0)
project(furnace)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_PROJECT_VERSION_MAJOR 0)
set(CMAKE_PROJECT_VERSION_MINOR 4)
set(CMAKE_PROJECT_VERSION_PATCH 6)

if (ANDROID)
  set(BUILD_GUI OFF)
else()
  set(BUILD_GUI ON)
endif()

option(DEVENDOR_LIBRARIES "Use local versions of dependencies instead of the ones provided by submodules" OFF)

if (NOT DEVENDOR_LIBRARIES)
  add_subdirectory(extern/fmt EXCLUDE_FROM_ALL)
endif()

if (NOT DEVENDOR_LIBRARIES)
  set(BUILD_TESTING OFF CACHE BOOL "aaaaaa" FORCE)
  set(BUILD_PROGRAMS OFF CACHE BOOL "aaa" FORCE)
  set(BUILD_EXAMPLES OFF CACHE BOOL "a" FORCE)
  set(ENABLE_EXTERNAL_LIBS OFF CACHE BOOL "come on" FORCE)
  set(ENABLE_MPEG OFF CACHE BOOL "come on" FORCE)
  add_subdirectory(extern/libsndfile EXCLUDE_FROM_ALL)
  add_subdirectory(extern/zlib EXCLUDE_FROM_ALL)

  include_directories(extern/zlib ${CMAKE_CURRENT_BINARY_DIR}/extern/zlib)
else()
  find_library(HAVE_Z z)
endif()

if (DEVENDOR_LIBRARIES)
  find_package(SDL)
  if (SDL_FOUND)
    set(HAVE_SDL2 ${SDL_LIBRARY})
    include_directories(extern/imgui extern/IconFontCppHeaders extern/imgui/backends extern/igfd ${SDL_INCLUDE_DIR})
  else()
    find_package(SDL2 REQUIRED)
    set(HAVE_SDL2 ${SDL2_LIBRARIES})
    include_directories(extern/imgui extern/IconFontCppHeaders extern/imgui/backends extern/igfd ${SDL2_INCLUDE_DIRS})
  endif()
  find_library(HAVE_JACK jack)
else()
  if (WIN32)
    set(SDL_SHARED OFF)
    set(SDL_STATIC ON)
    add_subdirectory(extern/SDL EXCLUDE_FROM_ALL)
    set(HAVE_SDL2 SDL2-static)
    set(HAVE_Z zlibstatic)
    include_directories(extern/SDL/include extern/imgui extern/IconFontCppHeaders extern/imgui/backends extern/igfd extern/fmt/include)
  else()
    if (BUILD_GUI)
      set(SDL_SHARED ON)
      add_subdirectory(extern/SDL EXCLUDE_FROM_ALL)
      include_directories(extern/SDL/include extern/imgui extern/IconFontCppHeaders extern/imgui/backends extern/igfd extern/fmt/include)
      set(HAVE_SDL2 SDL2-static)
    else()
      find_library(HAVE_SDL2 SDL2)
    endif()
    if (NOT APPLE)
      find_library(HAVE_JACK jack)
    endif()
    find_library(HAVE_SNDFILE sndfile)
    set(HAVE_Z zlibstatic)
  endif()
endif()

set(AUDIO_SOURCES src/audio/abstract.cpp)
if (HAVE_SDL2)
  list(APPEND AUDIO_SOURCES src/audio/sdl.cpp)
endif()
if (HAVE_JACK)
  list(APPEND AUDIO_SOURCES src/audio/jack.cpp)
endif()

set(ENGINE_SOURCES
src/log.cpp
src/fileutils.cpp

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
src/gui/gui.cpp)

if (NOT WIN32 AND NOT APPLE)
  list(APPEND GUI_SOURCES src/gui/icon.c)
endif()

if (WIN32)
  list(APPEND ENGINE_SOURCES src/utfutils.cpp)
  list(APPEND ENGINE_SOURCES src/engine/winStuff.cpp)
  list(APPEND ENGINE_SOURCES res/furnace.rc)
endif()

if (BUILD_GUI)
  add_executable(furnace ${ENGINE_SOURCES} ${AUDIO_SOURCES} ${GUI_SOURCES} src/main.cpp)
  target_compile_definitions(furnace PUBLIC HAVE_GUI)
else()
  add_executable(furnace ${ENGINE_SOURCES} ${AUDIO_SOURCES} src/main.cpp)
endif()

if (DEVENDOR_LIBRARIES)
  target_compile_definitions(furnace PUBLIC DEVENDOR_LIBRARIES)
endif()

target_link_libraries(furnace ${HAVE_SDL2} ${HAVE_Z} sndfile fmt)

if (HAVE_JACK)
  target_link_libraries(furnace ${HAVE_JACK})
  target_compile_definitions(furnace PUBLIC HAVE_JACK)
endif()

if (WIN32)
  target_link_libraries(furnace shlwapi -static)
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
