cmake_minimum_required(VERSION 3.12)

option(NO_FEATURE_REPORT "Disable feature report function" OFF)
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    option(WITH_SSE "x86: build with SSE support if detected. Turn OFF for older devices where autodetection is misleading." ON)
endif()

include_directories("${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/")

if(HAVE_STDLIB_H)
    set(INCLUDE_STDLIB_H "#include <stdlib.h>")
else()
    set(INCLUDE_STDLIB_H "/* #include <stdlib.h> is not available on this system */")
endif()
if(HAVE_SYS_TYPES_H)
    set(INCLUDE_SYS_TYPE_H "#include <sys/types.h>")
else()
    set(INCLUDE_SYS_TYPE_H "/* #include <sys/types.h> is not available on this system */")
endif()



# PPC with AltiVec is missing. But probably obsolete enough for CMake users.

if(ARCH_IS_X64)
    set(MACHINE amd64)
elseif(ARCH_IS_X86)
    set(MACHINE x86)
elseif(ARCH_IS_ARM64)
    set(MACHINE arm64)
elseif(ARCH_IS_ARM32)
    set(MACHINE arm32)
else()
    message(WARNING "Unknown processor. Using generic optimizations.")
    set(MACHINE generic)
endif()
message(STATUS "Detected machine: ${MACHINE}")

set(TARGET lib${PROJECT_NAME})
add_library(${TARGET}
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/parse.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/frame.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/format.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dct64.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/equalizer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/id3.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/optimize.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/readers.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/tabinit.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/libmpg123.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/index.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/$<$<NOT:$<BOOL:${NO_ICY}>>:icy.c>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/$<$<NOT:$<BOOL:${NO_ICY}>>:icy2utf8.c>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/$<$<NOT:$<BOOL:${NO_LAYER1}>>:layer1.c>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/$<$<NOT:$<OR:$<BOOL:${NO_LAYER1}>,$<BOOL:${NO_LAYER2}>>>:layer2.c>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/$<$<NOT:$<BOOL:${NO_LAYER3}>>:layer3.c>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/$<$<NOT:$<BOOL:${NO_NTOM}>>:ntom.c>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/$<$<NOT:$<BOOL:${NO_8BIT}>>:synth_8bit.c>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/$<$<NOT:$<BOOL:${NO_16BIT}>>:synth.c>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/$<$<AND:$<BOOL:${HAVE_FPU}>,$<NOT:$<BOOL:${NO_32BIT}>>>:synth_s32.c>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/$<$<AND:$<BOOL:${HAVE_FPU}>,$<NOT:$<BOOL:${NO_REAL}>>>:synth_real.c>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/$<$<NOT:$<BOOL:${NO_STRING}>>:stringbuf.c>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/$<$<NOT:$<BOOL:${NO_FEATURE_REPORT}>>:feature.c>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/$<$<NOT:$<BOOL:${PORTABLE_API}>>:lfs_wrap.c>"
    $<TARGET_OBJECTS:compat>)

if(MSVC)
    if(MACHINE MATCHES "x86|amd64")
      find_program(YASM_ASSEMBLER yasm)
      if(NOT YASM_ASSEMBLER)
          message(WARNING "Couldn't find yasm assembler for optimizded decoders. Please set YASM_ASSEMBLER variable")
          set(MACHINE generic)
      endif()
    else()
      message(WARNING "Need work to support non-x86 assembly optimizations with MSVC.")
      set(MACHINE generic)
    endif()
endif()

if(MACHINE STREQUAL "amd64")
    if(HAVE_FPU)
        set(PLATFORM_DEFINITIONS OPT_MULTI OPT_X86_64 OPT_AVX OPT_GENERIC OPT_GENERIC_DITHER)
        set(PLATFORM_SOURCES
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dct36_x86_64.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dct64_x86_64_float.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_x86_64_float.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_x86_64_s32.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_x86_64_float.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_x86_64_s32.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dct36_avx.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dct64_avx_float.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_avx_float.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_avx_s32.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/getcpuflags_x86_64.S")
        target_sources(${TARGET} PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dither.c")
        if(ACCURATE_ROUNDING)
            list(APPEND PLATFORM_SOURCES
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_x86_64_accurate.S"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_x86_64_accurate.S"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_avx_accurate.S")
        else()
            list(APPEND PLATFORM_SOURCES
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dct64_x86_64.S"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_x86_64.S"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_x86_64.S"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_avx.S"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dct64_avx.S")
        endif()
    else()
        set(PLATFORM_DEFINITIONS OPT_GENERIC)
    endif()
elseif(MACHINE STREQUAL "x86")
    if(TRUE)
        set(PLATFORM_DEFINITIONS OPT_I386)
        target_sources(${TARGET} PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dct64_i386.c")
    endif()

    cmake_host_system_information(RESULT HAVE_SSE QUERY HAS_SSE)
    if(CMAKE_SYSTEM_NAME STREQUAL "Android")
        set(HAVE_SSE ${WITH_SSE})
    endif()
    if(HAVE_SSE)
        set(PLATFORM_DEFINITIONS OPT_SSE)
        set(PLATFORM_SOURCES
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/tabinit_mmx.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dct36_sse.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dct64_sse_float.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_sse_float.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_sse_float.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_sse_s32.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_sse_s32.S")
        if(ACCURATE_ROUNDING)
            list(APPEND PLATFORM_SOURCES
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_sse_accurate.S"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_sse_accurate.S")
        else()
            list(APPEND PLATFORM_SOURCES
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dct64_sse.S"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_sse.S")
        endif()
    endif()
elseif(MACHINE STREQUAL "arm64")
        set(PLATFORM_DEFINITIONS OPT_MULTI OPT_GENERIC OPT_GENERIC_DITHER OPT_NEON64)
        set(PLATFORM_SOURCES
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dct36_neon64.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dct64_neon64_float.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_neon64_float.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_neon64_s32.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_neon64_float.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_neon64_s32.S"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/check_neon.S")
        if(ACCURATE_ROUNDING)
            list(APPEND PLATFORM_SOURCES
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_neon64_accurate.S"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_neon64_accurate.S")
        else()
            list(APPEND PLATFORM_SOURCES
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_neon64.S"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_neon64.S")
        endif()
        target_sources(${TARGET} PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dither.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/getcpuflags_arm.c")
elseif(MACHINE STREQUAL "arm32")
    if(HAVE_FPU)
            set(PLATFORM_DEFINITIONS OPT_MULTI OPT_GENERIC OPT_GENERIC_DITHER OPT_NEON)
            set(PLATFORM_SOURCES
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dct36_neon.S"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dct64_neon_float.S"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_neon_float.S"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_neon_s32.S"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_neon_float.S"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_neon_s32.S"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/check_neon.S")
            target_sources(${TARGET} PRIVATE
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/getcpuflags_arm.c")
            if(ACCURATE_ROUNDING)
                list(APPEND PLATFORM_SOURCES
                    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_neon_accurate.S"
                    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_neon_accurate.S")
            else()
                list(APPEND PLATFORM_SOURCES
                    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dct64_neon.S"
                    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_neon.S"
                    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_stereo_neon.S")
            endif()
            target_sources(${TARGET} PRIVATE ${PLATFORM_SOURCES})
            target_sources(${TARGET} PRIVATE
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/dither.c")
    else()
            set(PLATFORM_DEFINITIONS OPT_ARM)
            if(ACCURATE_ROUNDING)
                set(PLATFORM_SOURCES
                    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_arm_accurate.S")
            else()
                set(PLATFORM_SOURCES
                    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libmpg123/synth_arm.S")
            endif()
            target_sources(${TARGET} PRIVATE ${PLATFORM_SOURCES})
    endif()
elseif(MACHINE STREQUAL "generic")
    set(PLATFORM_DEFINITIONS OPT_GENERIC)
endif()

set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME mpg123)

target_compile_definitions(${TARGET} PRIVATE
    $<$<BOOL:$<TARGET_PROPERTY:POSITION_INDEPENDENT_CODE>>:PIC>)

target_compile_definitions(${TARGET} PRIVATE
    ${PLATFORM_DEFINITIONS}
    $<$<BOOL:${HAVE_FPU}>:REAL_IS_FLOAT>
    $<$<NOT:$<BOOL:${HAVE_FPU}>>:REAL_IS_FIXED>)

if(MSVC AND MACHINE MATCHES "x86|amd64" AND YASM_ASSEMBLER)
    list(TRANSFORM PLATFORM_DEFINITIONS PREPEND /D)
    foreach(FILE ${PLATFORM_SOURCES})
        get_filename_component(FILENAME ${FILE} NAME)
        add_custom_command(
            OUTPUT ${FILENAME}.asm
            COMMAND ${CMAKE_C_COMPILER} /DASMALIGN_BALIGN ${PLATFORM_DEFINITIONS}
            /I"${PROJECT_SOURCE_DIR}/../../src" /I"${PROJECT_BINARY_DIR}/src" /P /Fi${FILENAME}.asm /Tc "${FILE}"
            DEPENDS ${FILE}
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
        add_custom_command(
            OUTPUT ${FILENAME}.obj
            COMMAND ${YASM_ASSEMBLER} -a x86 -m ${MACHINE} -p gas -r raw -f win32 -g null -o ${FILENAME}.obj ${FILENAME}.asm
            DEPENDS ${FILENAME}.asm
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
        target_sources(${TARGET} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}.obj")
    endforeach()
else()
    target_sources(${TARGET} PRIVATE ${PLATFORM_SOURCES})
endif()

if(HAVE_M)
    string(APPEND LIBMPG123_LIBS " -lm")
endif()
if(WANT_WIN32_UNICODE)
    string(APPEND LIBMPG123_LIBS " -lshlwapi")
endif()
set(LIBMPG123_LIBS "${LIBMPG123_LIBS}" PARENT_SCOPE)
target_link_libraries(${TARGET} PRIVATE
    $<$<BOOL:${HAVE_M}>:m>
    $<$<BOOL:${WANT_WIN32_UNICODE}>:shlwapi>)
target_include_directories(${TARGET} INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

install(TARGETS ${TARGET} EXPORT targets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/include/mpg123.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/include/fmt123.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

