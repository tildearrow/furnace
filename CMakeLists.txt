cmake_minimum_required(VERSION 3.0)
project(furnace)

set(CMAKE_CXX_STANDARD 11)

if (WIN32)
  add_subdirectory(SDL)
  set(HAVE_SDL2 SDL2-static)
else()
  find_library(HAVE_SDL2 SDL2)
  find_library(HAVE_JACK jack)
endif()

include_directories(include)

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

src/engine/blip_buf.c
src/engine/safeReader.cpp
src/engine/engine.cpp
src/engine/playback.cpp
src/engine/platform/abstract.cpp
src/engine/platform/genesis.cpp
src/engine/platform/sms.cpp
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

target_link_libraries(furnace ${HAVE_SDL2} z)

if (HAVE_JACK)
  target_link_libraries(furnace ${HAVE_JACK})
endif()

if (WIN32)
  target_link_libraries(furnace SDL2main)
endif()
