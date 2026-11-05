# tildearrow: CMake port of Autotools build thingy.
# ported from Makefile.am

cmake_minimum_required(VERSION 3.0...3.20)
project(lame)

include(GNUInstallDirs)

set(LAME_DEFINES STDC_HEADERS HAVE_STRCHR HAVE_MEMCPY HAVE_CONFIG_H)

# mpglib

add_library(libmpgdecoder STATIC
mpglib/common.c
mpglib/dct64_i386.c
mpglib/decode_i386.c
mpglib/interface.c
mpglib/layer1.c
mpglib/layer2.c
mpglib/layer3.c
mpglib/tabinit.c
)

target_compile_definitions(libmpgdecoder PRIVATE ${LAME_DEFINES})
target_include_directories(libmpgdecoder PRIVATE config_ta include libmp3lame)

# libmp3lame

add_library(mp3lame STATIC
libmp3lame/VbrTag.c
libmp3lame/bitstream.c
libmp3lame/encoder.c
libmp3lame/fft.c
libmp3lame/gain_analysis.c
libmp3lame/id3tag.c
libmp3lame/lame.c
libmp3lame/newmdct.c
libmp3lame/presets.c
libmp3lame/psymodel.c
libmp3lame/quantize.c
libmp3lame/quantize_pvt.c
libmp3lame/reservoir.c
libmp3lame/set_get.c
libmp3lame/tables.c
libmp3lame/takehiro.c
libmp3lame/util.c
libmp3lame/vbrquantize.c
libmp3lame/version.c
libmp3lame/mpglib_interface.c

include/lame/lame.h
)

target_compile_definitions(mp3lame PRIVATE ${LAME_DEFINES})
target_include_directories(mp3lame PUBLIC include PRIVATE config_ta)

add_library(mp3lame::mp3lame ALIAS mp3lame)
