cmake_minimum_required(VERSION 3.5...3.19)
project(klattsch-cpp
  VERSION 0.1.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(KLATTSCH_BUILD_TEXT_COMPILER "Build the text-to-phoneme compiler" ON)

set(KLATTSCH_SOURCES
  src/klattsch.cpp
  src/phoneme_banks.cpp
  src/banks_registry.cpp
)
if(KLATTSCH_BUILD_TEXT_COMPILER)
  list(APPEND KLATTSCH_SOURCES src/klattsch_text.cpp)
endif()

add_library(klattsch ${KLATTSCH_SOURCES})
add_library(klattsch::klattsch ALIAS klattsch)

target_include_directories(klattsch PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if(MSVC)
  target_compile_options(klattsch PRIVATE /W4)
else()
  target_compile_options(klattsch PRIVATE -Wall -Wextra -Wpedantic)
endif()
