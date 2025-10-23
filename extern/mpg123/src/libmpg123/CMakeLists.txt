cmake_minimum_required(VERSION 3.0...3.19)

option(NO_FEATURE_REPORT "Disable feature report function" OFF)
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    option(WITH_SSE "x86: build with SSE support if detected. Turn OFF for older devices where autodetection is misleading." ON)
endif()

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

set(TARGET mpg123)
add_library(${TARGET}
    "parse.c"
    "frame.c"
    "format.c"
    "dct64.c"
    "equalizer.c"
    "id3.c"
    "optimize.c"
    "readers.c"
    "tabinit.c"
    "libmpg123.c"
    "index.c"
    "$<$<NOT:$<BOOL:${NO_ICY}>>:icy.c>"
    "$<$<NOT:$<BOOL:${NO_ICY}>>:icy2utf8.c>"
    "$<$<NOT:$<BOOL:${NO_LAYER1}>>:layer1.c>"
    "$<$<NOT:$<OR:$<BOOL:${NO_LAYER1}>,$<BOOL:${NO_LAYER2}>>>:layer2.c>"
    "$<$<NOT:$<BOOL:${NO_LAYER3}>>:layer3.c>"
    "$<$<NOT:$<BOOL:${NO_NTOM}>>:ntom.c>"
    "$<$<NOT:$<BOOL:${NO_8BIT}>>:synth_8bit.c>"
    "$<$<NOT:$<BOOL:${NO_16BIT}>>:synth.c>"
    "$<$<AND:$<BOOL:${HAVE_FPU}>,$<NOT:$<BOOL:${NO_32BIT}>>>:synth_s32.c>"
    "$<$<AND:$<BOOL:${HAVE_FPU}>,$<NOT:$<BOOL:${NO_REAL}>>>:synth_real.c>"
    "$<$<NOT:$<BOOL:${NO_STRING}>>:stringbuf.c>"
    "$<$<NOT:$<BOOL:${NO_FEATURE_REPORT}>>:feature.c>"
    "$<$<NOT:$<BOOL:${PORTABLE_API}>>:lfs_wrap.c>"
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
            "dct36_x86_64.S"
            "dct64_x86_64_float.S"
            "synth_x86_64_float.S"
            "synth_x86_64_s32.S"
            "synth_stereo_x86_64_float.S"
            "synth_stereo_x86_64_s32.S"
            "dct36_avx.S"
            "dct64_avx_float.S"
            "synth_stereo_avx_float.S"
            "synth_stereo_avx_s32.S"
            "getcpuflags_x86_64.S")
        target_sources(${TARGET} PRIVATE
            "dither.c")
        if(ACCURATE_ROUNDING)
            list(APPEND PLATFORM_SOURCES
                "synth_x86_64_accurate.S"
                "synth_stereo_x86_64_accurate.S"
                "synth_stereo_avx_accurate.S")
        else()
            list(APPEND PLATFORM_SOURCES
                "dct64_x86_64.S"
                "synth_x86_64.S"
                "synth_stereo_x86_64.S"
                "synth_stereo_avx.S"
                "dct64_avx.S")
        endif()
    else()
        set(PLATFORM_DEFINITIONS OPT_GENERIC)
    endif()
elseif(MACHINE STREQUAL "x86")
    if(TRUE)
        set(PLATFORM_DEFINITIONS OPT_I386)
        target_sources(${TARGET} PRIVATE
            "dct64_i386.c")
    endif()

    cmake_host_system_information(RESULT HAVE_SSE QUERY HAS_SSE)
    if(CMAKE_SYSTEM_NAME STREQUAL "Android")
        set(HAVE_SSE ${WITH_SSE})
    endif()
    if(HAVE_SSE)
        set(PLATFORM_DEFINITIONS OPT_SSE)
        set(PLATFORM_SOURCES
            "tabinit_mmx.S"
            "dct36_sse.S"
            "dct64_sse_float.S"
            "synth_sse_float.S"
            "synth_stereo_sse_float.S"
            "synth_sse_s32.S"
            "synth_stereo_sse_s32.S")
        if(ACCURATE_ROUNDING)
            list(APPEND PLATFORM_SOURCES
                "synth_sse_accurate.S"
                "synth_stereo_sse_accurate.S")
        else()
            list(APPEND PLATFORM_SOURCES
                "dct64_sse.S"
                "synth_sse.S")
        endif()
    endif()
elseif(MACHINE STREQUAL "arm64")
        set(PLATFORM_DEFINITIONS OPT_MULTI OPT_GENERIC OPT_GENERIC_DITHER OPT_NEON64)
        set(PLATFORM_SOURCES
            "dct36_neon64.S"
            "dct64_neon64_float.S"
            "synth_neon64_float.S"
            "synth_neon64_s32.S"
            "synth_stereo_neon64_float.S"
            "synth_stereo_neon64_s32.S"
            "check_neon.S")
        if(ACCURATE_ROUNDING)
            list(APPEND PLATFORM_SOURCES
                "synth_neon64_accurate.S"
                "synth_stereo_neon64_accurate.S")
        else()
            list(APPEND PLATFORM_SOURCES
                "synth_neon64.S"
                "synth_stereo_neon64.S")
        endif()
        target_sources(${TARGET} PRIVATE
            "dither.c"
            "getcpuflags_arm.c")
elseif(MACHINE STREQUAL "arm32")
    if(HAVE_FPU)
            set(PLATFORM_DEFINITIONS OPT_MULTI OPT_GENERIC OPT_GENERIC_DITHER OPT_NEON)
            set(PLATFORM_SOURCES
                "dct36_neon.S"
                "dct64_neon_float.S"
                "synth_neon_float.S"
                "synth_neon_s32.S"
                "synth_stereo_neon_float.S"
                "synth_stereo_neon_s32.S"
                "check_neon.S")
            target_sources(${TARGET} PRIVATE
                "getcpuflags_arm.c")
            if(ACCURATE_ROUNDING)
                list(APPEND PLATFORM_SOURCES
                    "synth_neon_accurate.S"
                    "synth_stereo_neon_accurate.S")
            else()
                list(APPEND PLATFORM_SOURCES
                    "dct64_neon.S"
                    "synth_neon.S"
                    "synth_stereo_neon.S")
            endif()
            target_sources(${TARGET} PRIVATE ${PLATFORM_SOURCES})
            target_sources(${TARGET} PRIVATE
                "dither.c")
    else()
            set(PLATFORM_DEFINITIONS OPT_ARM)
            if(ACCURATE_ROUNDING)
                set(PLATFORM_SOURCES
                    "synth_arm_accurate.S")
            else()
                set(PLATFORM_SOURCES
                    "synth_arm.S")
            endif()
            target_sources(${TARGET} PRIVATE ${PLATFORM_SOURCES})
    endif()
elseif(MACHINE STREQUAL "generic")
    set(PLATFORM_DEFINITIONS OPT_GENERIC)
endif()

#set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME mpg123)

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
            /I"${PROJECT_SOURCE_DIR}/src" /I"${PROJECT_BINARY_DIR}/src" /P /Fi${FILENAME}.asm /Tc "${FILE}"
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
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../src/include/mpg123.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../src/include/fmt123.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

# tildearrow: necessary
export(EXPORT targets NAMESPACE MPG123:: FILE mpg123Targets.cmake)
