cmake_minimum_required(VERSION 3.0...3.16)

if (APPLE)
  set(MACOSX_DEPLOYMENT_TARGET 10.9)
endif()

project(furnace)

if (APPLE)
  enable_language(OBJC)
  enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_PROJECT_VERSION_MAJOR 0)
set(CMAKE_PROJECT_VERSION_MINOR 6)
set(CMAKE_PROJECT_VERSION_PATCH 8)

set(BUILD_GUI_DEFAULT ON)
set(USE_SDL2_DEFAULT ON)
set(USE_SNDFILE_DEFAULT ON)
set(SYSTEM_SDL2_DEFAULT OFF)

include(CheckIncludeFile)
include(TestBigEndian)

execute_process(COMMAND git status RESULT_VARIABLE DONT_HAVE_GIT WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
if (NOT DONT_HAVE_GIT)
  message(STATUS "Git is available")
else()
  message(WARNING "either Git is not available, or this Git repository has not been initialized.
if you have used the \"Source code\" option in the GitHub release page, you are doing it wrong! unless you manually initialize submodules, the build is guaranteed to FAIL spectacularly!
read the \"developer info\" section of README.md for more information.")
endif()

if (ANDROID)
  set(USE_RTMIDI_DEFAULT OFF)
  set(WITH_PORTAUDIO_DEFAULT OFF)
  set(USE_BACKWARD_DEFAULT OFF)
  find_library(TERMUX rt)
  if (TERMUX)
    message(STATUS "Termux detected")
  endif()
else()
  set(USE_RTMIDI_DEFAULT ON)
  set(WITH_PORTAUDIO_DEFAULT ON)
  if (WIN32 OR APPLE)
    set(USE_BACKWARD_DEFAULT ON)
  else()
    CHECK_INCLUDE_FILE(execinfo.h EXECINFO_FOUND)
    if (EXECINFO_FOUND)
      set(USE_BACKWARD_DEFAULT ON)
    else()
      set(USE_BACKWARD_DEFAULT OFF)
    endif()
  endif()
endif()

find_package(PkgConfig)
if (PKG_CONFIG_FOUND AND NOT ANDROID)
  pkg_check_modules(JACK jack)
  set(WITH_JACK_DEFAULT ${JACK_FOUND})
else()
  set(WITH_JACK_DEFAULT OFF)
endif()

set(WITH_RENDER_SDL_DEFAULT ON)
set(WITH_RENDER_OPENGL_DEFAULT ON)
if (WIN32)
  set(WITH_RENDER_DX11_DEFAULT ON)
  set(WITH_RENDER_DX9_DEFAULT ON)
else()
  set(WITH_RENDER_DX11_DEFAULT OFF)
  set(WITH_RENDER_DX9_DEFAULT OFF)
endif()
if (APPLE)
  set(WITH_RENDER_METAL_DEFAULT ON)
else()
  set(WITH_RENDER_METAL_DEFAULT OFF)
endif()

if (ANDROID)
  set(USE_GLES_DEFAULT ON)
  set(WITH_RENDER_OPENGL1_DEFAULT OFF)
else()
  set(USE_GLES_DEFAULT OFF)
  if (APPLE)
    set(WITH_RENDER_OPENGL1_DEFAULT OFF)
  else()
    set(WITH_RENDER_OPENGL1_DEFAULT ON)
  endif()
endif()

set(WITH_LOCALE_DEFAULT ON)

try_compile(HAVE_SETLOCALE ${CMAKE_BINARY_DIR}/check SOURCES ${CMAKE_SOURCE_DIR}/src/check/check_setlocale.c)
include(FindIntl)

if (HAVE_SETLOCALE AND Intl_FOUND AND NOT APPLE AND NOT WIN32)
  set(USE_MOMO_DEFAULT OFF)
else()
  set(USE_MOMO_DEFAULT ON)
endif()

option(BUILD_GUI "Build the tracker (disable to build only a headless player)" ${BUILD_GUI_DEFAULT})
option(WITH_LOCALE "Use libintl for language support" ${WITH_LOCALE_DEFAULT})
option(USE_RTMIDI "Build with MIDI support using RtMidi." ${USE_RTMIDI_DEFAULT})
option(USE_SDL2 "Build with SDL2. Required to build with GUI." ${USE_SDL2_DEFAULT})
option(USE_SNDFILE "Build with libsndfile. Required in order to work with audio files." ${USE_SNDFILE_DEFAULT})
option(USE_BACKWARD "Use backward-cpp to print a backtrace on crash/abort." ${USE_BACKWARD_DEFAULT})
option(USE_MOMO "Build a libintl implementation instead of using the system one." ${USE_MOMO_DEFAULT})
option(WITH_JACK "Whether to build with JACK support. Auto-detects if JACK is available" ${WITH_JACK_DEFAULT})
option(WITH_PORTAUDIO "Whether to build with PortAudio for audio output." ${WITH_PORTAUDIO_DEFAULT})
option(WITH_RENDER_SDL "Whether to build with the SDL_Renderer render backend." ${WITH_RENDER_SDL_DEFAULT})
option(WITH_RENDER_OPENGL "Whether to build with the OpenGL render backend." ${WITH_RENDER_OPENGL_DEFAULT})
option(WITH_RENDER_OPENGL1 "Whether to build with the OpenGL 1.1 render backend." ${WITH_RENDER_OPENGL1_DEFAULT})
option(WITH_RENDER_DX11 "Whether to build with the DirectX 11 render backend." ${WITH_RENDER_DX11_DEFAULT})
option(WITH_RENDER_DX9 "Whether to build with the DirectX 9 render backend." ${WITH_RENDER_DX9_DEFAULT})
option(WITH_RENDER_METAL "Whether to build with the Metal render backend." ${WITH_RENDER_METAL_DEFAULT})
option(USE_GLES "Use OpenGL ES for the OpenGL render backend." ${USE_GLES_DEFAULT})
option(USE_FREETYPE "Use FreeType for font rendering." ON)
option(SYSTEM_FFTW "Use a system-installed version of FFTW instead of the vendored one" OFF)
option(SYSTEM_FMT "Use a system-installed version of fmt instead of the vendored one" OFF)
option(SYSTEM_FREETYPE "Use a system-installed version of FreeType instead of the vendored one" OFF)
option(SYSTEM_LIBSNDFILE "Use a system-installed version of libsndfile instead of the vendored one" OFF)
option(SYSTEM_PORTAUDIO "Use a system-installed version of PortAudio instead of the vendored one" OFF)
option(SYSTEM_RTMIDI "Use a system-installed version of RtMidi instead of the vendored one" OFF)
option(SYSTEM_ZLIB "Use a system-installed version of zlib instead of the vendored one" OFF)
option(SYSTEM_SDL2 "Use a system-installed version of SDL2 instead of the vendored one" ${SYSTEM_SDL2_DEFAULT})
option(SUPPORT_XP "Build a Windows XP-compatible binary" OFF)
option(SUPPORT_VISTA "Build a Windows Vista-compatible binary" OFF)
option(WARNINGS_ARE_ERRORS "Whether warnings in furnace's C++ code should be treated as errors" OFF)
option(WITH_DEMOS "Install demo songs (NOTE: demo songs are NOT under GPL!)" OFF)
option(WITH_INSTRUMENTS "Install instruments" ON)
option(WITH_WAVETABLES "Install wavetables" ON)
option(SHOW_OPEN_ASSETS_MENU_ENTRY "Show option to open built-in assets directory (on supported platforms)" OFF)
option(CONSOLE_SUBSYSTEM "Build Furnace with Console subsystem on Windows" OFF)
option(FORCE_CODEVIEW "Force -gcodeview on MinGW GCC" OFF)
option(FLATPAK_WORKAROUNDS "Enable Flatpak-specific workaround for system file picker" OFF)
option(NO_INTRO "Disable intro animation entirely" OFF)
option(ORIG_NDS_CORE "Use original NDS emulation core (no acquireDirect)" OFF)
if (APPLE)
  option(FORCE_APPLE_BIN "Force enable binary installation to /bin" OFF)
  option(MAKE_BUNDLE "Make a bundle" OFF)
else()
  # not Apple - not needed
  set(FORCE_APPLE_BIN OFF)
  set(MAKE_BUNDLE OFF)
endif()

set(DEPENDENCIES_INCLUDE_DIRS extern/IconFontCppHeaders extern/blip_buf src/icon)

if (ANDROID AND NOT TERMUX)
  set(DEPENDENCIES_DEFINES "IS_MOBILE")
else()
  set(DEPENDENCIES_DEFINES "")
endif()

TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

if (IS_BIG_ENDIAN)
  list(APPEND DEPENDENCIES_DEFINES "TA_BIG_ENDIAN")
endif()

if (FLATPAK_WORKAROUNDS)
  list(APPEND DEPENDENCIES_DEFINES "FLATPAK_WORKAROUNDS")
endif()

set(DEPENDENCIES_COMPILE_OPTIONS "")
set(DEPENDENCIES_LIBRARIES "")
set(DEPENDENCIES_LIBRARY_DIRS "")
set(DEPENDENCIES_LINK_OPTIONS "")
set(DEPENDENCIES_LEGACY_LDFLAGS "")

if (BUILD_GUI AND WITH_RENDER_SDL)
  set(SYSTEM_SDL_MIN_VER 2.0.18)
else()
  set(SYSTEM_SDL_MIN_VER 2.0.0)
endif()

#list(APPEND DEPENDENCIES_INCLUDE_DIRS "extern/SAASound/include")
list(APPEND DEPENDENCIES_INCLUDE_DIRS "extern/vgsound_emu-modified")

find_package(Threads REQUIRED)
list(APPEND DEPENDENCIES_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})


if (WIN32)
  # support Windows XP
  if (SUPPORT_XP)
    list(APPEND DEPENDENCIES_DEFINES "_WIN32_WINNT=0x0500")
    list(APPEND DEPENDENCIES_DEFINES "SUPPORT_XP")
  else()
    # support Windows Vista
    if (SUPPORT_VISTA)
      list(APPEND DEPENDENCIES_DEFINES "_WIN32_WINNT=0x0600")
    endif()
  endif()
endif()

if (SYSTEM_FFTW)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(FFTW REQUIRED fftw3>=3.3)
  list(APPEND DEPENDENCIES_INCLUDE_DIRS ${FFTW_INCLUDE_DIRS})
  list(APPEND DEPENDENCIES_COMPILE_OPTIONS ${FFTW_CFLAGS_OTHER})
  list(APPEND DEPENDENCIES_LIBRARIES ${FFTW_LIBRARIES})
  list(APPEND DEPENDENCIES_LIBRARY_DIRS ${FFTW_LIBRARY_DIRS})
  list(APPEND DEPENDENCIES_LINK_OPTIONS ${FFTW_LDFLAGS_OTHER})
  list(APPEND DEPENDENCIES_LEGACY_LDFLAGS ${FFTW_LDFLAGS})
  message(STATUS "Using system-installed FFTW")
else()
  if (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(WITH_OUR_MALLOC ON CACHE BOOL "aaa" FORCE)
  endif()
  set(BUILD_TESTS OFF CACHE BOOL "come on" FORCE)
  add_subdirectory(extern/fftw EXCLUDE_FROM_ALL)
  list(APPEND DEPENDENCIES_INCLUDE_DIRS extern/fftw/api)
  list(APPEND DEPENDENCIES_LIBRARIES fftw3)
  message(STATUS "Using vendored FFTW")
endif()

if (SYSTEM_FMT)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(FMT fmt>=7.1.0)
    if (FMT_FOUND)
      list(APPEND DEPENDENCIES_INCLUDE_DIRS ${FMT_INCLUDE_DIRS})
      list(APPEND DEPENDENCIES_COMPILE_OPTIONS ${FMT_CFLAGS_OTHER})
      list(APPEND DEPENDENCIES_LIBRARIES ${FMT_LIBRARIES})
      list(APPEND DEPENDENCIES_LIBRARY_DIRS ${FMT_LIBRARY_DIRS})
      list(APPEND DEPENDENCIES_LINK_OPTIONS ${FMT_LDFLAGS_OTHER})
      list(APPEND DEPENDENCIES_LEGACY_LDFLAGS ${FMT_LDFLAGS})
    endif()
  endif()
  if (NOT FMT_FOUND)
    find_package(fmt REQUIRED)
    list(APPEND DEPENDENCIES_LIBRARIES fmt::fmt)
  endif()
  message(STATUS "Using system-installed fmt")
else()
  add_subdirectory(extern/fmt EXCLUDE_FROM_ALL)
  list(APPEND DEPENDENCIES_INCLUDE_DIRS extern/fmt/include)
  list(APPEND DEPENDENCIES_LIBRARIES fmt)
  message(STATUS "Using vendored fmt")
endif()

if (BUILD_GUI AND USE_FREETYPE)
  # TODO: FIX FIX FIX FIX FIX
  if (SYSTEM_FREETYPE)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(FREETYPE freetype2>=2.0.0)
      if (FREETYPE_FOUND)
        list(APPEND DEPENDENCIES_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS})
        list(APPEND DEPENDENCIES_COMPILE_OPTIONS ${FREETYPE_CFLAGS_OTHER})
        list(APPEND DEPENDENCIES_LIBRARIES ${FREETYPE_LIBRARIES})
        list(APPEND DEPENDENCIES_LIBRARY_DIRS ${FREETYPE_LIBRARY_DIRS})
        list(APPEND DEPENDENCIES_LINK_OPTIONS ${FREETYPE_LDFLAGS_OTHER})
        list(APPEND DEPENDENCIES_LEGACY_LDFLAGS ${FREETYPE_LDFLAGS})
      endif()
    endif()
    if (NOT FREETYPE_FOUND)
      find_package(freetype REQUIRED)
      list(APPEND DEPENDENCIES_LIBRARIES freetype::freetype)
    endif()
    message(STATUS "Using system-installed FreeType")
  else()
    set(FT_DISABLE_BROTLI ON CACHE BOOL "one" FORCE)
    set(FT_DISABLE_BZIP2 ON CACHE BOOL "two" FORCE)
    set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "three" FORCE)
    set(FT_DISABLE_PNG ON CACHE BOOL "four" FORCE)
    set(FT_DISABLE_ZLIB ON CACHE BOOL "five" FORCE)
    add_subdirectory(extern/freetype EXCLUDE_FROM_ALL)
    list(APPEND DEPENDENCIES_INCLUDE_DIRS extern/freetype/include)
    list(APPEND DEPENDENCIES_LIBRARIES freetype)
    message(STATUS "Using vendored FreeType")
  endif()
  list(APPEND DEPENDENCIES_DEFINES HAVE_FREETYPE)
endif()

if (USE_SNDFILE)
  list(APPEND DEPENDENCIES_DEFINES HAVE_SNDFILE)
  if (SYSTEM_LIBSNDFILE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBSNDFILE REQUIRED sndfile)
    list(APPEND DEPENDENCIES_INCLUDE_DIRS ${LIBSNDFILE_INCLUDE_DIRS})
    list(APPEND DEPENDENCIES_COMPILE_OPTIONS ${LIBSNDFILE_CFLAGS_OTHER})
    list(APPEND DEPENDENCIES_LIBRARIES ${LIBSNDFILE_LIBRARIES})
    list(APPEND DEPENDENCIES_LIBRARY_DIRS ${LIBSNDFILE_LIBRARY_DIRS})
    list(APPEND DEPENDENCIES_LINK_OPTIONS ${LIBSNDFILE_LDFLAGS_OTHER})
    list(APPEND DEPENDENCIES_LEGACY_LDFLAGS ${LIBSNDFILE_LDFLAGS})
    message(STATUS "Using system-installed libsndfile")
  else()
    set(BUILD_TESTING OFF CACHE BOOL "aaaaaa" FORCE)
    set(BUILD_PROGRAMS OFF CACHE BOOL "aaa" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "a" FORCE)
    set(ENABLE_EXTERNAL_LIBS OFF CACHE BOOL "come on" FORCE)
    set(ENABLE_MPEG OFF CACHE BOOL "come on" FORCE)
    add_subdirectory(extern/libsndfile-modified EXCLUDE_FROM_ALL)
    list(APPEND DEPENDENCIES_LIBRARIES sndfile)
    message(STATUS "Using vendored libsndfile")
  endif()
else()
  message(STATUS "Not using libsndfile")
endif()

if (WITH_PORTAUDIO)
  if (SYSTEM_PORTAUDIO)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
    list(APPEND DEPENDENCIES_INCLUDE_DIRS ${PORTAUDIO_INCLUDE_DIRS})
    list(APPEND DEPENDENCIES_COMPILE_OPTIONS ${PORTAUDIO_CFLAGS_OTHER})
    list(APPEND DEPENDENCIES_LIBRARIES ${PORTAUDIO_LIBRARIES})
    list(APPEND DEPENDENCIES_LIBRARY_DIRS ${PORTAUDIO_LIBRARY_DIRS})
    list(APPEND DEPENDENCIES_LINK_OPTIONS ${PORTAUDIO_LDFLAGS_OTHER})
    list(APPEND DEPENDENCIES_LEGACY_LDFLAGS ${PORTAUDIO_LDFLAGS})
    message(STATUS "Using system-installed PortAudio")
  else()
    set(PA_BUILD_SHARED_LIBS OFF CACHE BOOL "Build dynamic library" FORCE)
    # don't - Furnace has its own implementation
    set(PA_USE_JACK OFF CACHE BOOL "Enable support for JACK Audio Connection Kit" FORCE)
    add_subdirectory(extern/portaudio-modified EXCLUDE_FROM_ALL)
    list(APPEND DEPENDENCIES_LIBRARIES PortAudio)
    message(STATUS "Using vendored PortAudio")
  endif()
endif()

if (USE_RTMIDI)
  if (SYSTEM_RTMIDI)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(RTMIDI REQUIRED rtmidi)
    list(APPEND DEPENDENCIES_INCLUDE_DIRS ${RTMIDI_INCLUDE_DIRS})
    list(APPEND DEPENDENCIES_COMPILE_OPTIONS ${RTMIDI_CFLAGS_OTHER})
    list(APPEND DEPENDENCIES_LIBRARIES ${RTMIDI_LIBRARIES})
    list(APPEND DEPENDENCIES_LIBRARY_DIRS ${RTMIDI_LIBRARY_DIRS})
    list(APPEND DEPENDENCIES_LINK_OPTIONS ${RTMIDI_LDFLAGS_OTHER})
    list(APPEND DEPENDENCIES_LEGACY_LDFLAGS ${RTMIDI_LDFLAGS})
    message(STATUS "Using system-installed RtMidi")
  else()
    add_subdirectory(extern/rtmidi EXCLUDE_FROM_ALL)
    list(APPEND DEPENDENCIES_LIBRARIES rtmidi)
    message(STATUS "Using vendored RtMidi")
  endif()
endif()

if (SYSTEM_ZLIB)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(ZLIB REQUIRED zlib)
  list(APPEND DEPENDENCIES_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS})
  list(APPEND DEPENDENCIES_COMPILE_OPTIONS ${ZLIB_CFLAGS_OTHER})
  list(APPEND DEPENDENCIES_LIBRARIES ${ZLIB_LIBRARIES})
  list(APPEND DEPENDENCIES_LIBRARY_DIRS ${ZLIB_LIBRARY_DIRS})
  list(APPEND DEPENDENCIES_LINK_OPTIONS ${ZLIB_LDFLAGS_OTHER})
  list(APPEND DEPENDENCIES_LEGACY_LDFLAGS ${ZLIB_LDFLAGS})
  message(STATUS "Using system-installed zlib")
else()
  set(BUILD_TESTING OFF CACHE BOOL "aaaaaa" FORCE)
  set(BUILD_PROGRAMS OFF CACHE BOOL "aaa" FORCE)
  set(BUILD_EXAMPLES OFF CACHE BOOL "a" FORCE)
  set(ENABLE_EXTERNAL_LIBS OFF CACHE BOOL "come on" FORCE)
  set(ENABLE_MPEG OFF CACHE BOOL "come on" FORCE)
  set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "come on" FORCE)
  add_subdirectory(extern/zlib EXCLUDE_FROM_ALL)
  list(APPEND DEPENDENCIES_INCLUDE_DIRS extern/zlib ${CMAKE_CURRENT_BINARY_DIR}/extern/zlib)
  list(APPEND DEPENDENCIES_LIBRARIES zlibstatic)
  message(STATUS "Using vendored zlib")
endif()

if (USE_SDL2)
  if (SYSTEM_SDL2)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(SDL2 sdl2>=${SYSTEM_SDL_MIN_VER})
      if (SDL2_FOUND)
        list(APPEND DEPENDENCIES_DEFINES HAVE_SDL2)
        list(APPEND DEPENDENCIES_INCLUDE_DIRS ${SDL2_INCLUDE_DIRS})
        list(APPEND DEPENDENCIES_COMPILE_OPTIONS ${SDL2_CFLAGS_OTHER})
        list(APPEND DEPENDENCIES_LIBRARIES ${SDL2_LIBRARIES})
        list(APPEND DEPENDENCIES_LIBRARY_DIRS ${SDL2_LIBRARY_DIRS})
        list(APPEND DEPENDENCIES_LINK_OPTIONS ${SDL2_LDFLAGS_OTHER})
        list(APPEND DEPENDENCIES_LEGACY_LDFLAGS ${SDL2_LDFLAGS})
      endif()
    endif()
    if (NOT SDL2_FOUND)
      find_package(SDL2 ${SYSTEM_SDL_MIN_VER} REQUIRED)
      list(APPEND DEPENDENCIES_DEFINES HAVE_SDL2)
      list(APPEND DEPENDENCIES_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
      list(APPEND DEPENDENCIES_LIBRARIES ${SDL2_LIBRARY})
    endif()
    message(STATUS "Using system-installed SDL2")
  else()
    if (ANDROID AND NOT TERMUX)
      set(SDL_SHARED ON CACHE BOOL "Force no dynamically-linked SDL" FORCE)
      set(SDL_STATIC OFF CACHE BOOL "Force statically-linked SDL" FORCE)
    else()
      set(SDL_SHARED OFF CACHE BOOL "Force no dynamically-linked SDL" FORCE)
      set(SDL_STATIC ON CACHE BOOL "Force statically-linked SDL" FORCE)
    endif()
    # https://github.com/libsdl-org/SDL/issues/5535
    # disable PipeWire support due to an unfixable bug:
    #   Looks like their headers have a C90 violation... I imagine they're probably on C99 so not the craziest bug in the world. Definitely file this at the PipeWire repository as well so they know this is out there.
    set(SDL_PIPEWIRE OFF CACHE BOOL "Use Pipewire audio" FORCE)

    # https://github.com/libsdl-org/SDL/issues/1481
    # On 2014-06-22 17:15:50 +0000, Sam Lantinga wrote:
    #   If you link SDL statically, you also need to define HAVE_LIBC so it builds with the C runtime that your application uses.
    #   This should probably go in a FAQ.
    set(SDL_LIBC ON CACHE BOOL "Tell SDL that we want it to use our C runtime (required for proper static linking)" FORCE)

    # https://github.com/tildearrow/furnace/issues/1237
    # enabling this will result in SDL finding the Direct3D headers, forcing _WIN32_WINNT to an undesirable value (which makes the Wine headers define GetTickCount64)
    if (SUPPORT_XP)
      set(SDL_RENDER_D3D OFF CACHE BOOL "Enable the Direct3D render driver" FORCE)
    endif()

    CHECK_INCLUDE_FILE(GLES/gl.h GLES_GL_FOUND)
    if (NOT GLES_GL_FOUND AND NOT USE_GLES)
      set(SDL_OPENGLES OFF CACHE BOOL "Enable OpenGL ES" FORCE)
    endif()

    add_subdirectory(extern/SDL EXCLUDE_FROM_ALL)
    list(APPEND DEPENDENCIES_DEFINES HAVE_SDL2)
    list(APPEND DEPENDENCIES_INCLUDE_DIRS extern/SDL/include)
    if (ANDROID AND NOT TERMUX)
      list(APPEND DEPENDENCIES_LIBRARIES SDL2)
    else()
      list(APPEND DEPENDENCIES_LIBRARIES SDL2-static)
    endif()
    # Work around add_subdirectory'd SDL not propagating HAVE_LIBC to MSVC furnace build
    if (MSVC)
      list(APPEND DEPENDENCIES_COMPILE_OPTIONS "/DHAVE_LIBC")
    endif()
    if (WIN32)
      list(APPEND DEPENDENCIES_LIBRARIES SDL2main)
    endif()
    message(STATUS "Using vendored SDL2")
  endif()
else()
  message(STATUS "Not using SDL2")
  if (BUILD_GUI)
    message(FATAL_ERROR "SDL2 is required in order to build with GUI! Disable BUILD_GUI otherwise.")
  endif()
endif()

set(AUDIO_SOURCES
src/audio/abstract.cpp
src/audio/midi.cpp
src/audio/pipe.cpp
)

if (USE_SDL2)
  list(APPEND AUDIO_SOURCES src/audio/sdlAudio.cpp)
endif()

if (WITH_JACK)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(JACK REQUIRED jack)
  list(APPEND AUDIO_SOURCES extern/weakjack/weak_libjack.c)
  list(APPEND AUDIO_SOURCES src/audio/jack.cpp)
  list(APPEND DEPENDENCIES_INCLUDE_DIRS ${JACK_INCLUDE_DIRS})
  list(APPEND DEPENDENCIES_DEFINES HAVE_JACK)
  list(APPEND DEPENDENCIES_DEFINES USE_WEAK_JACK)
  #list(APPEND DEPENDENCIES_COMPILE_OPTIONS ${JACK_CFLAGS_OTHER})
  #list(APPEND DEPENDENCIES_LIBRARIES ${JACK_LIBRARIES})
  #list(APPEND DEPENDENCIES_LIBRARY_DIRS ${JACK_LIBRARY_DIRS})
  #list(APPEND DEPENDENCIES_LINK_OPTIONS ${JACK_LDFLAGS_OTHER})
  #list(APPEND DEPENDENCIES_LEGACY_LDFLAGS ${JACK_LDFLAGS})
  message(STATUS "Building with JACK support")
else()
  message(STATUS "Building without JACK support")
endif()

if (WITH_PORTAUDIO)
  list(APPEND AUDIO_SOURCES src/audio/pa.cpp)
  message(STATUS "Building with PortAudio")
  list(APPEND DEPENDENCIES_DEFINES HAVE_PA)
else()
  message(STATUS "Building without PortAudio")
endif()

if (USE_RTMIDI)
  list(APPEND AUDIO_SOURCES src/audio/rtmidi.cpp)
  message(STATUS "Building with RtMidi")
  list(APPEND DEPENDENCIES_DEFINES HAVE_RTMIDI)
else()
  message(STATUS "Building without RtMidi")
endif()

set(ENGINE_SOURCES
src/log.cpp
src/baseutils.cpp
src/fileutils.cpp
src/utfutils.cpp

extern/itcompress/compression.c

extern/SAASound/src/SAAAmp.cpp
extern/SAASound/src/SAADevice.cpp
extern/SAASound/src/SAAEnv.cpp
extern/SAASound/src/SAAFreq.cpp
extern/SAASound/src/SAAImpl.cpp
extern/SAASound/src/SAANoise.cpp
extern/SAASound/src/SAASndC.cpp
extern/SAASound/src/SAASound.cpp

extern/vgsound_emu-modified/vgsound_emu/src/core/vox/vox.cpp

extern/vgsound_emu-modified/vgsound_emu/src/es550x/es550x.cpp
extern/vgsound_emu-modified/vgsound_emu/src/es550x/es550x_alu.cpp
extern/vgsound_emu-modified/vgsound_emu/src/es550x/es550x_filter.cpp
extern/vgsound_emu-modified/vgsound_emu/src/es550x/es5504.cpp
extern/vgsound_emu-modified/vgsound_emu/src/es550x/es5505.cpp
extern/vgsound_emu-modified/vgsound_emu/src/es550x/es5506.cpp

extern/vgsound_emu-modified/vgsound_emu/src/k005289/k005289.cpp

extern/vgsound_emu-modified/vgsound_emu/src/k007232/k007232.cpp

extern/vgsound_emu-modified/vgsound_emu/src/k053260/k053260.cpp

extern/vgsound_emu-modified/vgsound_emu/src/msm6295/msm6295.cpp

extern/vgsound_emu-modified/vgsound_emu/src/n163/n163.cpp

extern/vgsound_emu-modified/vgsound_emu/src/scc/scc.cpp

extern/vgsound_emu-modified/vgsound_emu/src/vrcvi/vrcvi.cpp

extern/vgsound_emu-modified/vgsound_emu/src/x1_010/x1_010.cpp

extern/adpcm/bs_codec.c
extern/adpcm/oki_codec.c
extern/adpcm/yma_codec.c
extern/adpcm/ymb_codec.c
extern/adpcm/ymz_codec.c

extern/adpcm-xq-s/adpcm-lib.c

extern/opn/ym3438.c
extern/Nuked-PSG/ympsg.c
extern/opm/opm.c
extern/Nuked-OPLL/opll.c
extern/opl/opl3.c
extern/YM3812-LLE/fmopl2.c
extern/YMF262-LLE/fmopl3.c
extern/YMF276-LLE/fmopn2.c
extern/ESFMu/esfm.c
extern/ESFMu/esfm_registers.c
extern/emu2413/emu2413.c
extern/YM2608-LLE/fmopna_2608.c
extern/YM2608-LLE/fmopna_2610.c
extern/YM2608-LLE/fmopna_2612.c

extern/pwrnoise/pwrnoise.c

extern/blip_buf/blip_buf.c

src/pch.cpp

src/engine/platform/sound/sn76496.cpp
src/engine/platform/sound/ay8910.cpp
src/engine/platform/sound/saa1099.cpp
src/engine/platform/sound/namco.cpp
src/engine/platform/sound/segapcm.cpp
src/engine/platform/sound/gb/apu.c
src/engine/platform/sound/gb/timing.c
src/engine/platform/sound/pce_psg.cpp
src/engine/platform/sound/nes/apu.cpp
src/engine/platform/sound/nes/fds.cpp
src/engine/platform/sound/nes/mmc5.cpp
src/engine/platform/sound/vera_psg.c
src/engine/platform/sound/vera_pcm.c
src/engine/platform/sound/ymf278b/ymf278.cpp

src/engine/platform/sound/atomicssg/ssg.c

src/engine/platform/sound/nes_nsfplay/5e01_apu.cpp
src/engine/platform/sound/nes_nsfplay/5e01_dmc.cpp
src/engine/platform/sound/nes_nsfplay/nes_apu.cpp
src/engine/platform/sound/nes_nsfplay/nes_dmc.cpp
src/engine/platform/sound/nes_nsfplay/nes_fds.cpp
src/engine/platform/sound/nes_nsfplay/nes_mmc5.cpp
src/engine/platform/sound/nes_nsfplay/nes_n106.cpp
src/engine/platform/sound/nes_nsfplay/nes_vrc6.cpp

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

src/engine/platform/sound/c64_fp/array.cpp
src/engine/platform/sound/c64_fp/Dac.cpp
src/engine/platform/sound/c64_fp/EnvelopeGenerator.cpp
src/engine/platform/sound/c64_fp/ExternalFilter.cpp
src/engine/platform/sound/c64_fp/Filter6581.cpp
src/engine/platform/sound/c64_fp/Filter8580.cpp
src/engine/platform/sound/c64_fp/Filter.cpp
src/engine/platform/sound/c64_fp/FilterModelConfig6581.cpp
src/engine/platform/sound/c64_fp/FilterModelConfig8580.cpp
src/engine/platform/sound/c64_fp/FilterModelConfig.cpp
src/engine/platform/sound/c64_fp/Integrator6581.cpp
src/engine/platform/sound/c64_fp/Integrator8580.cpp
src/engine/platform/sound/c64_fp/OpAmp.cpp
src/engine/platform/sound/c64_fp/SID.cpp
src/engine/platform/sound/c64_fp/Spline.cpp
src/engine/platform/sound/c64_fp/WaveformCalculator.cpp
src/engine/platform/sound/c64_fp/WaveformGenerator.cpp
src/engine/platform/sound/c64_fp/resample/SincResampler.cpp

src/engine/platform/sound/c64_d/dsid.c

src/engine/platform/sound/tia/AudioChannel.cpp
src/engine/platform/sound/tia/Audio.cpp

src/engine/platform/sound/ymfm/ymfm_adpcm.cpp
src/engine/platform/sound/ymfm/ymfm_opl.cpp
src/engine/platform/sound/ymfm/ymfm_opm.cpp
src/engine/platform/sound/ymfm/ymfm_opn.cpp
src/engine/platform/sound/ymfm/ymfm_opz.cpp
src/engine/platform/sound/ymfm/ymfm_pcm.cpp
src/engine/platform/sound/ymfm/ymfm_ssg.cpp

src/engine/platform/sound/lynx/Mikey.cpp

src/engine/platform/sound/pokey/mzpokeysnd.c
src/engine/platform/sound/pokey/AltASAP.cpp

src/engine/platform/sound/qsound.c

src/engine/platform/sound/swan.c
src/engine/platform/sound/swan_mdfn.cpp

src/engine/platform/sound/su.cpp

src/engine/platform/sound/vic20sound.c

src/engine/platform/sound/ymz280b.cpp

src/engine/platform/sound/vsu.cpp

src/engine/platform/sound/t6w28/T6W28_Apu.cpp

src/engine/platform/sound/rf5c68.cpp

src/engine/platform/sound/oki/msm5232.cpp
src/engine/platform/sound/oki/okim6258.cpp

src/engine/platform/sound/snes/SPC_DSP.cpp

src/engine/platform/sound/ga20/iremga20.cpp

src/engine/platform/sound/sm8521.c

src/engine/platform/sound/supervision.c

src/engine/platform/sound/upd1771.cpp

src/engine/platform/sound/d65modified.c

src/engine/platform/sound/ted-sound.c

src/engine/platform/sound/c140_c219.c

src/engine/platform/sound/dave/dave.cpp

src/engine/platform/sound/sid2/envelope.cc
src/engine/platform/sound/sid2/filter.cc
src/engine/platform/sound/sid2/sid.cc
src/engine/platform/sound/sid2/version.cc
src/engine/platform/sound/sid2/voice.cc
src/engine/platform/sound/sid2/wave8580_PS_.cc
src/engine/platform/sound/sid2/wave8580_PST.cc
src/engine/platform/sound/sid2/wave8580_P_T.cc
src/engine/platform/sound/sid2/wave8580__ST.cc
src/engine/platform/sound/sid2/wave.cc

src/engine/platform/sound/sid3.c

src/engine/platform/oplAInterface.cpp
src/engine/platform/ym2608Interface.cpp
src/engine/platform/ym2610Interface.cpp

src/engine/fileOps/fileOpsCommon.cpp
src/engine/fileOps/dmf.cpp
src/engine/fileOps/fc.cpp
src/engine/fileOps/ftm.cpp
src/engine/fileOps/fur.cpp
src/engine/fileOps/it.cpp
src/engine/fileOps/mod.cpp
src/engine/fileOps/s3m.cpp
src/engine/fileOps/text.cpp
src/engine/fileOps/tfm.cpp
src/engine/fileOps/xm.cpp

src/engine/fileOps/p.cpp
src/engine/fileOps/p86.cpp
src/engine/fileOps/pdx.cpp
src/engine/fileOps/ppc.cpp
src/engine/fileOps/pps.cpp
src/engine/fileOps/pvi.cpp
src/engine/fileOps/pzi.cpp

src/engine/brrUtils.c
src/engine/safeReader.cpp
src/engine/safeWriter.cpp
src/engine/workPool.cpp
src/engine/cmdStream.cpp
src/engine/cmdStreamOps.cpp
src/engine/config.cpp
src/engine/configEngine.cpp
src/engine/dispatchContainer.cpp
src/engine/engine.cpp
src/engine/export.cpp
src/engine/exportDef.cpp
src/engine/fileOpsIns.cpp
src/engine/fileOpsSample.cpp
src/engine/filter.cpp
src/engine/instrument.cpp
src/engine/macroInt.cpp
src/engine/pattern.cpp
src/engine/pitchTable.cpp
src/engine/playback.cpp
src/engine/sample.cpp
src/engine/song.cpp
src/engine/sysDef.cpp
src/engine/wavetable.cpp
src/engine/waveSynth.cpp
src/engine/wavOps.cpp
src/engine/vgmOps.cpp

src/engine/platform/abstract.cpp
src/engine/platform/genesis.cpp
src/engine/platform/genesisext.cpp
src/engine/platform/sms.cpp
src/engine/platform/opll.cpp
src/engine/platform/gb.cpp
src/engine/platform/pce.cpp
src/engine/platform/mmc5.cpp
src/engine/platform/nes.cpp
src/engine/platform/c64.cpp
src/engine/platform/arcade.cpp
src/engine/platform/tx81z.cpp
src/engine/platform/ym2203.cpp
src/engine/platform/ym2203ext.cpp
src/engine/platform/ym2608.cpp
src/engine/platform/ym2608ext.cpp
src/engine/platform/ym2610.cpp
src/engine/platform/ym2610ext.cpp
src/engine/platform/ym2610b.cpp
src/engine/platform/ym2610bext.cpp
src/engine/platform/ay.cpp
src/engine/platform/ay8930.cpp
src/engine/platform/opl.cpp
src/engine/platform/fds.cpp
src/engine/platform/tia.cpp
src/engine/platform/saa.cpp
src/engine/platform/amiga.cpp
src/engine/platform/msm5232.cpp
src/engine/platform/msm6258.cpp
src/engine/platform/msm6295.cpp
src/engine/platform/pcspkr.cpp
src/engine/platform/segapcm.cpp
src/engine/platform/qsound.cpp
src/engine/platform/x1_010.cpp
src/engine/platform/pokey.cpp
src/engine/platform/lynx.cpp
src/engine/platform/su.cpp
src/engine/platform/swan.cpp
src/engine/platform/t6w28.cpp
src/engine/platform/vb.cpp
src/engine/platform/vera.cpp
src/engine/platform/zxbeeper.cpp
src/engine/platform/zxbeeperquadtone.cpp
src/engine/platform/bubsyswsg.cpp
src/engine/platform/n163.cpp
src/engine/platform/pet.cpp
src/engine/platform/pokemini.cpp
src/engine/platform/pong.cpp
src/engine/platform/vic20.cpp
src/engine/platform/vrc6.cpp
src/engine/platform/es5506.cpp
src/engine/platform/scc.cpp
src/engine/platform/ymz280b.cpp
src/engine/platform/namcowsg.cpp
src/engine/platform/rf5c68.cpp
src/engine/platform/snes.cpp
src/engine/platform/k007232.cpp
src/engine/platform/ga20.cpp
src/engine/platform/sm8521.cpp
src/engine/platform/supervision.cpp
src/engine/platform/scvtone.cpp
src/engine/platform/pv1000.cpp
src/engine/platform/k053260.cpp
src/engine/platform/ted.cpp
src/engine/platform/c140.cpp
src/engine/platform/esfm.cpp
src/engine/platform/powernoise.cpp
src/engine/platform/dave.cpp
src/engine/platform/gbadma.cpp
src/engine/platform/gbaminmod.cpp
src/engine/platform/nds.cpp
src/engine/platform/bifurcator.cpp
src/engine/platform/sid2.cpp
src/engine/platform/sid3.cpp
src/engine/platform/multipcm.cpp
src/engine/platform/pcmdac.cpp
src/engine/platform/dummy.cpp

src/engine/export/abstract.cpp
src/engine/export/amigaValidation.cpp
src/engine/export/sapr.cpp
src/engine/export/tiuna.cpp
src/engine/export/zsm.cpp
src/engine/export/ipod.cpp
src/engine/export/grub.cpp

src/engine/effect/abstract.cpp
src/engine/effect/dummy.cpp
)

if (ORIG_NDS_CORE)
  list(APPEND ENGINE_SOURCES src/engine/platform/sound/nds_unopt.cpp)
  list(APPEND DEPENDENCIES_DEFINES ORIG_NDS_CORE)
else()
  list(APPEND ENGINE_SOURCES src/engine/platform/sound/nds.cpp)
endif()

if (USE_SNDFILE)
  list(APPEND ENGINE_SOURCES src/engine/sfWrapper.cpp)
endif()

if (WIN32)
  list(APPEND ENGINE_SOURCES src/utfutils.cpp)
  list(APPEND ENGINE_SOURCES src/engine/winStuff.cpp)
  list(APPEND ENGINE_SOURCES res/furnace.rc)
endif()

if (APPLE)
  if (CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS 10.12)
    list(APPEND DEPENDENCIES_INCLUDE_DIRS extern/macports-legacy-support/include)
    list(APPEND ENGINE_SOURCES extern/macports-legacy-support/src/time.c)
  endif()
endif()

set(CLI_SOURCES
src/cli/cli.cpp
)

set(GUI_SOURCES
extern/imgui_patched/imgui.cpp
extern/imgui_patched/imgui_draw.cpp
extern/imgui_patched/imgui_tables.cpp
extern/imgui_patched/imgui_widgets.cpp
extern/imgui_patched/backends/imgui_impl_sdl2.cpp
extern/imgui_software_renderer/imgui_sw.cpp
extern/imgui_patched/misc/cpp/imgui_stdlib.cpp

src/gui/plot_nolerp.cpp

src/gui/render.cpp
src/gui/render/abstract.cpp
src/gui/render/renderSoftware.cpp

src/gui/font_exo.cpp
src/gui/font_liberationSans.cpp
src/gui/font_mononoki.cpp
src/gui/font_plexMono.cpp
src/gui/font_plexSans.cpp
src/gui/font_plexSansJP.cpp
src/gui/font_plexSansKR.cpp
src/gui/font_proggyClean.cpp
src/gui/font_ptMono.cpp
src/gui/font_unifont.cpp
src/gui/font_icon.cpp
src/gui/font_furicon.cpp
src/gui/fonts.cpp
src/gui/fontzlib.cpp

src/gui/image_icon.cpp
src/gui/image.cpp

src/gui/debug.cpp
src/gui/fileDialog.cpp
src/gui/newFilePicker.cpp

src/gui/intConst.cpp
src/gui/guiConst.cpp

src/gui/about.cpp
src/gui/channels.cpp
src/gui/chanOsc.cpp
src/gui/clock.cpp
src/gui/compatFlags.cpp
src/gui/csPlayer.cpp
src/gui/cursor.cpp
src/gui/dataList.cpp
src/gui/debugWindow.cpp
src/gui/doAction.cpp
src/gui/editing.cpp
src/gui/editControls.cpp
src/gui/effectList.cpp
src/gui/exportOptions.cpp
src/gui/findReplace.cpp
src/gui/fmPreview.cpp
src/gui/gradient.cpp
src/gui/grooves.cpp
src/gui/insEdit.cpp
src/gui/log.cpp
src/gui/memory.cpp
src/gui/mixer.cpp
src/gui/midiMap.cpp
src/gui/newSong.cpp
src/gui/commandPalette.cpp
src/gui/orders.cpp
src/gui/osc.cpp
src/gui/patManager.cpp
src/gui/pattern.cpp
src/gui/piano.cpp
src/gui/presets.cpp
src/gui/regView.cpp
src/gui/sampleEdit.cpp
src/gui/scaling.cpp
src/gui/settings.cpp
src/gui/songInfo.cpp
src/gui/songNotes.cpp
src/gui/speed.cpp
src/gui/spoiler.cpp
src/gui/stats.cpp
src/gui/subSongs.cpp
src/gui/sysConf.cpp
src/gui/sysEx.cpp
src/gui/sysManager.cpp
src/gui/sysMiscInfo.cpp
src/gui/sysPicker.cpp
src/gui/tutorial.cpp
src/gui/userPresets.cpp
src/gui/util.cpp
src/gui/waveEdit.cpp
src/gui/volMeter.cpp
src/gui/xyOsc.cpp
src/gui/gui.cpp
)

if (NOT NO_INTRO)
  list(APPEND GUI_SOURCES src/gui/introTune.cpp)
  list(APPEND GUI_SOURCES src/gui/intro.cpp)
  list(APPEND GUI_SOURCES
  src/gui/image_talogo.cpp
  src/gui/image_tachip.cpp
  src/gui/image_logo.cpp
  src/gui/image_wordmark.cpp
  src/gui/image_introbg.cpp
  src/gui/image_pat.cpp
  )
else()
  list(APPEND DEPENDENCIES_DEFINES NO_INTRO)
endif()

if (WIN32 AND NOT SUPPORT_XP)
  list(APPEND GUI_SOURCES extern/nfd-modified/src/nfd_common.cpp)
  list(APPEND GUI_SOURCES extern/nfd-modified/src/nfd_win.cpp)
endif()

if (USE_FREETYPE)
  list(APPEND GUI_SOURCES extern/imgui_patched/misc/freetype/imgui_freetype.cpp)
endif()

if (APPLE)
  list(APPEND GUI_SOURCES extern/nfd-modified/src/nfd_common.cpp)
  list(APPEND GUI_SOURCES src/gui/macstuff.m)
  list(APPEND GUI_SOURCES extern/nfd-modified/src/nfd_cocoa.mm)
endif()

if (WITH_RENDER_SDL)
  list(APPEND GUI_SOURCES src/gui/render/renderSDL.cpp)
  list(APPEND GUI_SOURCES extern/imgui_patched/backends/imgui_impl_sdlrenderer2.cpp)
  list(APPEND DEPENDENCIES_DEFINES HAVE_RENDER_SDL)
  message(STATUS "UI render backend: SDL_Renderer")
endif()

if (WITH_RENDER_OPENGL)
  list(APPEND GUI_SOURCES src/gui/render/renderGL.cpp)
  list(APPEND GUI_SOURCES extern/imgui_patched/backends/imgui_impl_opengl3.cpp)
  list(APPEND DEPENDENCIES_DEFINES HAVE_RENDER_GL)
  if (USE_GLES)
    list(APPEND DEPENDENCIES_DEFINES USE_GLES)
    list(APPEND DEPENDENCIES_DEFINES IMGUI_IMPL_OPENGL_ES2)
  endif()
  if (WIN32)
    list(APPEND DEPENDENCIES_LIBRARIES opengl32)
  elseif(APPLE)
    list(APPEND DEPENDENCIES_LIBRARIES "-framework OpenGL")
  elseif(USE_GLES)
    list(APPEND DEPENDENCIES_LIBRARIES GLESv2)
  else()
    list(APPEND DEPENDENCIES_LIBRARIES GL)
  endif()
  if (USE_GLES)
    message(STATUS "UI render backend: OpenGL ES 2.0")
  else()
    message(STATUS "UI render backend: OpenGL 3.0/2.0")
  endif()
endif()

if (WITH_RENDER_OPENGL1)
  list(APPEND GUI_SOURCES src/gui/render/renderGL1.cpp)
  list(APPEND GUI_SOURCES extern/imgui_patched/backends/imgui_impl_opengl2.cpp)
  list(APPEND DEPENDENCIES_DEFINES HAVE_RENDER_GL1)
  if (NOT WITH_RENDER_OPENGL)
    if (WIN32)
      list(APPEND DEPENDENCIES_LIBRARIES opengl32)
    else()
      list(APPEND DEPENDENCIES_LIBRARIES GL)
    endif()
  endif()
  message(STATUS "UI render backend: OpenGL 1.1")
endif()

if (WITH_RENDER_DX11)
  if (WIN32)
    if (SUPPORT_XP)
      message(FATAL_ERROR "SUPPORT_XP is on. cannot enable DirectX 11 backend.")
    else()
      list(APPEND GUI_SOURCES src/gui/render/renderDX11.cpp)
      list(APPEND GUI_SOURCES extern/imgui_patched/backends/imgui_impl_dx11.cpp)
      list(APPEND DEPENDENCIES_DEFINES HAVE_RENDER_DX11)
      list(APPEND DEPENDENCIES_LIBRARIES d3d11 dxgi)
      message(STATUS "UI render backend: DirectX 11")
    endif()
  else()
    message(FATAL_ERROR "DirectX 11 render backend only for Windows!")
  endif()
endif()

if (WITH_RENDER_DX9)
  if (WIN32)
    list(APPEND GUI_SOURCES src/gui/render/renderDX9.cpp)
    list(APPEND GUI_SOURCES extern/imgui_patched/backends/imgui_impl_dx9.cpp)
    list(APPEND DEPENDENCIES_DEFINES HAVE_RENDER_DX9)
    list(APPEND DEPENDENCIES_LIBRARIES d3d9)
    message(STATUS "UI render backend: DirectX 9")
  else()
    message(FATAL_ERROR "DirectX 9 render backend only for Windows!")
  endif()
endif()

if (WITH_RENDER_METAL)
  if (APPLE)
    list(APPEND GUI_SOURCES src/gui/render/renderMetal.mm)
    list(APPEND GUI_SOURCES extern/imgui_patched/backends/imgui_impl_metal.mm)
    list(APPEND DEPENDENCIES_DEFINES HAVE_RENDER_METAL)
    list(APPEND DEPENDENCIES_LIBRARIES "-framework Metal")
    list(APPEND DEPENDENCIES_LIBRARIES "-framework MetalKit")
    message(STATUS "UI render backend: Metal")
  else()
    message(FATAL_ERROR "Metal render backend only for Apple operating systems!")
  endif()
endif()

if (NOT WIN32 AND NOT APPLE)
  CHECK_INCLUDE_FILE(sys/io.h SYS_IO_FOUND)
  CHECK_INCLUDE_FILE(linux/input.h LINUX_INPUT_FOUND)
  CHECK_INCLUDE_FILE(linux/kd.h LINUX_KD_FOUND)
  if (SYS_IO_FOUND)
    try_compile(HAVE_INOUTB ${CMAKE_BINARY_DIR}/check SOURCES ${CMAKE_SOURCE_DIR}/src/check/check_sysIO.c)
    if (HAVE_INOUTB)
      list(APPEND DEPENDENCIES_DEFINES HAVE_SYS_IO)
      message(STATUS "PC speaker output: outb()")
    else()
      message(STATUS "sys/io.h found but inb()/outb() not present")
    endif()
  endif()
  if (LINUX_INPUT_FOUND)
    list(APPEND DEPENDENCIES_DEFINES HAVE_LINUX_INPUT)
    message(STATUS "PC speaker output: evdev")
  endif()
  if (LINUX_KD_FOUND)
    list(APPEND DEPENDENCIES_DEFINES HAVE_LINUX_KD)
    message(STATUS "PC speaker output: KIOCSOUND")
  endif()
endif()

if (NOT WIN32)
  try_compile(HAVE_DIRENT_TYPE ${CMAKE_BINARY_DIR}/check SOURCES ${CMAKE_SOURCE_DIR}/src/check/check_dirent_type.c)
  if (HAVE_DIRENT_TYPE)
    list(APPEND DEPENDENCIES_DEFINES HAVE_DIRENT_TYPE)
  endif()
endif()

if (WITH_LOCALE)
  if (HAVE_SETLOCALE)
    list(APPEND DEPENDENCIES_DEFINES HAVE_SETLOCALE)
    message(STATUS "setlocale() found")
  else()
    if (NOT USE_MOMO)
      message(FATAL_ERROR "setlocale() is not defined! This means the C library in your system does not support locale at all. Try enabling USE_MOMO.")
    else()
      message(STATUS "setlocale() is not defined")
    endif()
  endif()
  if (USE_MOMO)
    list(APPEND ENGINE_SOURCES src/momo/momo.c)
    list(APPEND DEPENDENCIES_DEFINES HAVE_LOCALE)
    list(APPEND DEPENDENCIES_DEFINES HAVE_MOMO)
    list(APPEND DEPENDENCIES_INCLUDE_DIRS src/momo)
    message(STATUS "Using libintl (Momo)")
  else()
    if ("${CMAKE_VERSION}" VERSION_LESS "3.2")
      message(FATAL_ERROR "CMake 3.2 or later required for locale support.")
    else()
      if (NOT Intl_FOUND)
        message(FATAL_ERROR "Could not find libintl! Try enabling USE_MOMO.")
      endif()
      list(APPEND DEPENDENCIES_DEFINES HAVE_LOCALE)
      list(APPEND DEPENDENCIES_DEFINES GNULIB_overrides_sprintf)
      list(APPEND DEPENDENCIES_INCLUDE_DIRS ${Intl_INCLUDE_DIRS})
      list(APPEND DEPENDENCIES_LIBRARIES ${Intl_LIBRARIES})
      message(STATUS "Using libintl (system)")
    endif()
  endif()
endif()

set(USED_SOURCES ${ENGINE_SOURCES} ${AUDIO_SOURCES} ${CLI_SOURCES} src/main.cpp)

if (USE_BACKWARD)
  list(APPEND USED_SOURCES src/backtrace.cpp)
  if (WIN32)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      include(CheckCXXCompilerFlag)
      check_cxx_compiler_flag(-gcodeview GCC_CODEVIEW)
      if (GCC_CODEVIEW OR FORCE_CODEVIEW)
        set(CMAKE_EXE_LINKER_FLAGS "-Wl,--pdb= ")
        add_compile_options(-gcodeview)
        message(STATUS "Enabling -gcodeview flag for backward-cpp.")
      else()
        message(WARNING "Could not enable -gcodeview! backward-cpp will not work.")
      endif()
      list(APPEND DEPENDENCIES_LIBRARIES dbghelp psapi)
    endif()
  endif()
  find_library(EXECINFO_IS_LIBRARY execinfo)
  if (EXECINFO_IS_LIBRARY)
    list(APPEND DEPENDENCIES_LIBRARIES execinfo)
  endif()
  message(STATUS "Using backward-cpp")
else()
  message(STATUS "Not using backward-cpp")
endif()

if (BUILD_GUI)
  list(APPEND USED_SOURCES ${GUI_SOURCES})
  list(APPEND DEPENDENCIES_INCLUDE_DIRS
    extern/imgui_patched
    extern/imgui_patched/backends
    extern/imgui_software_renderer
  )
  if (WIN32 OR APPLE)
    list(APPEND DEPENDENCIES_INCLUDE_DIRS
      extern/nfd-modified/src/include
    )
  endif()
  list(APPEND DEPENDENCIES_DEFINES HAVE_GUI)
  message(STATUS "Building GUI")
else()
  message(STATUS "Building headless")
endif()

if (WIN32)
  list(APPEND DEPENDENCIES_LIBRARIES shlwapi)
  if (NOT MSVC)
    list(APPEND DEPENDENCIES_LIBRARIES -static)
  endif()
elseif (APPLE)
  find_library(COCOA Cocoa REQUIRED)
  list(APPEND DEPENDENCIES_LIBRARIES ${COCOA})
else()
  list(APPEND DEPENDENCIES_LIBRARIES ${CMAKE_DL_LIBS})
endif()

if (NOT MSVC)
  set(WARNING_FLAGS -Wall -Wextra -Wno-unused-parameter)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0.0)
      # nothing
    else()
      list(APPEND WARNING_FLAGS -Wno-cast-function-type)
    endif()
  endif()
  if (WARNINGS_ARE_ERRORS)
    list(APPEND WARNING_FLAGS -Werror)
  endif()
else()
  add_compile_options("/utf-8")
  set(WARNING_FLAGS /W2 /D_CRT_SECURE_NO_WARNINGS)
  list(APPEND WARNING_FLAGS
    /wd4244 # implicit type conversions
    /wd4305 # truncations
    /wd4309 # truncations of constant values
  )
  if (WARNINGS_ARE_ERRORS)
    list(APPEND WARNING_FLAGS /WX)
  endif()
endif()
# Nicer but cannot be narrowed down to just C++
# target_compile_options(furnace PRIVATE ${WARNING_FLAGS})
string(REPLACE ";" " " WARNING_FLAGS_STRING "${WARNING_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS_STRING}")
if (WARNINGS_ARE_ERRORS)
  message(STATUS
    "Treating all warnings in furnace's C++ code as errors! "
    "Please report any errors you encounter on the bug tracker."
  )
endif()

if (NOT ANDROID OR TERMUX)
  if (NOT WIN32 AND NOT APPLE)
    if (NOT DONT_HAVE_GIT)
      add_custom_command(OUTPUT furnace.appdata.xml COMMAND res/make-appdata.sh ARGS ${CMAKE_SOURCE_DIR}/res/furnace.appdata.xml.in ${CMAKE_BINARY_DIR}/furnace.appdata.xml DEPENDS res/furnace.appdata.xml.in WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
      list(APPEND USED_SOURCES furnace.appdata.xml)
    endif()
  endif()
endif()

if (WIN32 AND CONSOLE_SUBSYSTEM)
  list(APPEND DEPENDENCIES_DEFINES "TA_SUBSYSTEM_CONSOLE")
endif()

if (MAKE_BUNDLE)
  set(FURNACE Furnace)
else()
  set(FURNACE furnace)
endif()

if(ANDROID AND NOT TERMUX)
  add_library(${FURNACE} SHARED ${USED_SOURCES})
elseif(WIN32 AND NOT CONSOLE_SUBSYSTEM)
  add_executable(${FURNACE} WIN32 ${USED_SOURCES})
else()
  add_executable(${FURNACE} ${USED_SOURCES})
endif()

if (MAKE_BUNDLE AND NOT FORCE_APPLE_BIN)
  set_target_properties(${FURNACE} PROPERTIES
    MACOSX_BUNDLE True
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/res/Info.plist)
endif()

target_include_directories(${FURNACE} SYSTEM PRIVATE ${DEPENDENCIES_INCLUDE_DIRS})
target_compile_options(${FURNACE} PRIVATE ${DEPENDENCIES_COMPILE_OPTIONS})
target_link_libraries(${FURNACE} PRIVATE ${DEPENDENCIES_LIBRARIES})
if (PKG_CONFIG_FOUND AND (SYSTEM_FMT OR SYSTEM_LIBSNDFILE OR SYSTEM_ZLIB OR SYSTEM_SDL2 OR SYSTEM_RTMIDI OR WITH_JACK))
  if ("${CMAKE_VERSION}" VERSION_LESS "3.13")
    message(WARNING
      "CMake version is <3.13, using old pkg-config LDFLAGS. "
      "You may encounter linking problems with these!"
    )
    target_link_libraries(${FURNACE} PRIVATE ${DEPENDENCIES_LEGACY_LDFLAGS})
  else()
    target_link_directories(${FURNACE} PRIVATE ${DEPENDENCIES_LIBRARY_DIRS})
    target_link_options(${FURNACE} PRIVATE ${DEPENDENCIES_LINK_OPTIONS})
  endif()
endif()

# why 3.16..... why not 3.0?
if (NOT "${CMAKE_VERSION}" VERSION_LESS "3.16")
  if (BUILD_GUI)
  target_precompile_headers(${FURNACE} PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h>
    $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui_patched/imgui.h>
    $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui_patched/imgui_internal.h>
  )
  else()
    target_precompile_headers(${FURNACE} PUBLIC
      $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h>
    )
  endif()
endif()

if (NOT ANDROID OR TERMUX)
  if (NOT WIN32 AND NOT APPLE)
    include(GNUInstallDirs)
    install(TARGETS ${FURNACE} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES res/furnace.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES res/mime.xml RENAME furnace.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/mime/packages)
    if (NOT DONT_HAVE_GIT)
      install(FILES ${CMAKE_BINARY_DIR}/furnace.appdata.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
    endif()
    install(DIRECTORY doc DESTINATION ${CMAKE_INSTALL_DOCDIR})
    install(DIRECTORY papers DESTINATION ${CMAKE_INSTALL_DOCDIR}/other)
    install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/furnace)
    list(APPEND DEPENDENCIES_DEFINES LOCALE_DIR="${CMAKE_INSTALL_FULL_LOCALEDIR}")
    install(DIRECTORY po/locale/ DESTINATION ${CMAKE_INSTALL_LOCALEDIR})
    if (WITH_DEMOS OR WITH_INSTRUMENTS OR WITH_WAVETABLES)
      set(FURNACE_DATADIR ${CMAKE_INSTALL_FULL_DATADIR}/furnace)
      if (WITH_DEMOS)
        install(DIRECTORY demos DESTINATION ${FURNACE_DATADIR})
      endif()
      if (WITH_INSTRUMENTS)
        install(DIRECTORY instruments DESTINATION ${FURNACE_DATADIR})
      endif()
      if (WITH_WAVETABLES)
        install(DIRECTORY wavetables DESTINATION ${FURNACE_DATADIR})
      endif()
      list(APPEND DEPENDENCIES_DEFINES FURNACE_DATADIR="${FURNACE_DATADIR}")
      if (SHOW_OPEN_ASSETS_MENU_ENTRY)
        list(APPEND DEPENDENCIES_DEFINES SHOW_OPEN_ASSETS_MENU_ENTRY)
      endif()
    endif()
    foreach(num 16 32 64 128 256 512)
      set(res ${num}x${num})
      install(FILES res/icon.iconset/icon_${res}.png RENAME furnace.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${res}/apps)
      install(FILES res/icon.iconset/icon_${res}@2x.png RENAME furnace.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${res}@2/apps)
    endforeach()
    install(FILES res/logo.png RENAME furnace.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/1024x1024/apps)
  else()
    if (MAKE_BUNDLE)
      install(TARGETS ${FURNACE} BUNDLE DESTINATION Applications RESOURCE DESTINATION Resources)
    else()
      install(TARGETS ${FURNACE} RUNTIME DESTINATION bin)
    endif()
  endif()
endif()

target_compile_definitions(${FURNACE} PRIVATE ${DEPENDENCIES_DEFINES})
