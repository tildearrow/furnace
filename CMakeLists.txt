cmake_minimum_required(VERSION 3.0)
project(furnace)

set(CMAKE_CXX_STANDARD 11)

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
src/engine/platform/sound/sn76496.cpp
src/engine/platform/sound/gb/apu.c
src/engine/platform/sound/gb/timing.c
src/engine/platform/sound/pce_psg.cpp

src/engine/blip_buf.c
src/engine/safeReader.cpp
src/engine/engine.cpp
src/engine/macroInt.cpp
src/engine/playback.cpp
src/engine/platform/abstract.cpp
src/engine/platform/genesis.cpp
src/engine/platform/genesisext.cpp
src/engine/platform/sms.cpp
src/engine/platform/gb.cpp
src/engine/platform/pce.cpp
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

target_link_libraries(furnace ${HAVE_SDL2} ${HAVE_Z})

if (HAVE_JACK)
  target_link_libraries(furnace ${HAVE_JACK})
endif()

if (WIN32)
  target_link_libraries(furnace SDL2main)
endif()
