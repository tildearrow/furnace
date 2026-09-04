# MODIFIED version - see README.md for details.

cmake_minimum_required(VERSION 3.0)
project(lua-cmake VERSION 1.0.0 DESCRIPTION "Lua build for a project subdirectory" LANGUAGES C)

set(srcFiles
upstream/src/lapi.c
upstream/src/lauxlib.c
upstream/src/lbaselib.c
upstream/src/lcode.c
upstream/src/lcorolib.c
upstream/src/lctype.c
upstream/src/ldblib.c
upstream/src/ldebug.c
upstream/src/ldo.c
upstream/src/ldump.c
upstream/src/lfunc.c
upstream/src/lgc.c
upstream/src/linit.c
upstream/src/liolib.c
upstream/src/llex.c
upstream/src/lmathlib.c
upstream/src/lmem.c
upstream/src/loadlib.c
upstream/src/lobject.c
upstream/src/lopcodes.c
upstream/src/loslib.c
upstream/src/lparser.c
upstream/src/lstate.c
upstream/src/lstring.c
upstream/src/lstrlib.c
upstream/src/ltable.c
upstream/src/ltablib.c
upstream/src/ltm.c
upstream/src/lundump.c
upstream/src/lutf8lib.c
upstream/src/lvm.c
upstream/src/lzio.c)

set(publicHeaderFiles
upstream/src/lauxlib.h
upstream/src/lua.h
upstream/src/luaconf.h
upstream/src/lualib.h)

add_library(lua STATIC
    ${srcFiles})

set(DEFINES LUA_COMPAT_5_3)

if (LINUX)
  list(APPEND DEFINES LUA_USE_LINUX)
endif()
if (WIN32)
  list(APPEND DEFINES LUA_USE_WINDOWS)
endif()
if (APPLE)
  list(APPEND DEFINES LUA_USE_MACOSX)
endif()

target_compile_definitions(lua PRIVATE ${DEFINES})

#target_compile_options(lua
#    PRIVATE
#    $<$<OR:$<C_COMPILER_ID:AppleClang>,$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:GNU>>:
#    -Wextra -Wshadow -Wsign-compare -Wundef -Wwrite-strings -Wredundant-decls
#    -Wdisabled-optimization -Waggregate-return -Wdouble-promotion -Wdeclaration-after-statement
#    -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes -Wc++-compat -Wold-style-definition>)

set(includeDir ${CMAKE_CURRENT_BINARY_DIR}/include)
file(COPY ${publicHeaderFiles} DESTINATION ${includeDir})

#add_library(lua-header
#    INTERFACE)

#target_include_directories(lua-header
#    INTERFACE
#    ${includeDir})

#target_link_libraries(lua
#    INTERFACE
#    lua-header)

add_library(lua::lib ALIAS lua)
#add_library(lua::header ALIAS lua-header)
