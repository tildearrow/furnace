cmake_minimum_required(VERSION 3.0)
project(furnace)

set(CMAKE_CXX_STANDARD 14)

set(BUILD_TESTING OFF)
add_subdirectory(extern/libsndfile)

if (WIN32)
  set(SDL_SHARED OFF)
  add_subdirectory(extern/SDL)
  add_subdirectory(extern/zlib)
  set(HAVE_SDL2 SDL2-static)
  set(HAVE_Z zlibstatic)
  include_directories(extern/zlib ${CMAKE_CURRENT_BINARY_DIR}/extern/zlib)
else()
  find_library(HAVE_SDL2 SDL2)
  if (NOT APPLE)
    find_library(HAVE_JACK jack)
  endif()
  find_library(HAVE_Z z)
  find_library(HAVE_SNDFILE sndfile)
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

extern/Nuked-OPN2/ym3438.c
extern/opm/opm.c
src/engine/platform/sound/sn76496.cpp
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

src/engine/platform/sound/ym2610/ymfm_adpcm.cpp
src/engine/platform/sound/ym2610/ymfm_opn.cpp
src/engine/platform/sound/ym2610/ymfm_ssg.cpp

src/engine/platform/ym2610Interface.cpp

src/engine/blip_buf.c
src/engine/safeReader.cpp
src/engine/engine.cpp
src/engine/macroInt.cpp
src/engine/pattern.cpp
src/engine/playback.cpp
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
src/engine/platform/dummy.cpp)

#imgui/imgui.cpp
#imgui/imgui_demo.cpp
#imgui/imgui_draw.cpp
#imgui/imgui_tables.cpp
#imgui/imgui_widgets.cpp
#imgui/backends/imgui_impl_opengl3.cpp
#imgui/backends/imgui_impl_sdl.cpp
#src/gui/main.cpp)

add_executable(furnace ${ENGINE_SOURCES} ${AUDIO_SOURCES}
src/main.cpp)

target_link_libraries(furnace ${HAVE_SDL2} ${HAVE_Z} sndfile)

if (HAVE_JACK)
  target_link_libraries(furnace ${HAVE_JACK})
  target_compile_definitions(furnace PUBLIC HAVE_JACK)
endif()

if (WIN32)
  target_link_libraries(furnace -static)
endif()
