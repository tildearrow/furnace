cmake_minimum_required(VERSION 3.0)
project(emu2413)

if(MSVC)
  set(CMAKE_C_FLAGS "/Ox /W3 /wd4996")
else()
  set(CMAKE_C_FLAGS "-O3 -Wall")
endif()

add_library(emu2413 STATIC emu2413.c)
