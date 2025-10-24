cmake_minimum_required(VERSION 3.0...3.19)

include(CheckCSourceCompiles)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckTypeSize)
include(CMakeDependentOption)
include(TestBigEndian)

option(PORTABLE_API "Only build portable library API (no off_t, no internal I/O." OFF)

check_include_file("arpa/inet.h" HAVE_ARPA_INET_H)
check_include_file("dirent.h" HAVE_DIRENT_H)
check_include_file("dlfcn.h" HAVE_DLFCN_H)
check_include_file("langinfo.h" HAVE_LANGINFO_H)
check_include_file("locale.h" HAVE_LOCALE_H)
check_include_file("netdb.h" HAVE_NETDB_H)
check_include_file("netinet/in.h" HAVE_NETINET_IN_H)
check_include_file("sched.h" HAVE_SCHED_H)
check_include_file("signal.h" HAVE_SIGNAL_H)
check_include_file("strings.h" HAVE_STRINGS_H)
check_include_file("sys/ioctl.h" HAVE_SYS_IOCTL_H)
check_include_file("sys/ipc.h" HAVE_SYS_IPC_H)
check_include_file("sys/param.h" HAVE_SYS_PARAM_H)
check_include_file("sys/resource.h" HAVE_SYS_RESOURCE_H)
check_include_file("sys/select.h" HAVE_SYS_SELECT_H)
check_include_file("sys/shm.h" HAVE_SYS_SHM_H)
check_include_file("sys/signal.h" HAVE_SYS_SIGNAL_H)
check_include_file("sys/socket.h" HAVE_SYS_SOCKET_H)
check_include_file("sys/stat.h" HAVE_SYS_STAT_H)
check_include_file("sys/time.h" HAVE_SYS_TIME_H)
check_include_file("sys/types.h" HAVE_SYS_TYPES_H)
check_include_file("sys/wait.h" HAVE_SYS_WAIT_H)
check_include_file("termios.h" HAVE_TERMIOS)
check_include_file("unistd.h" HAVE_UNISTD_H)
check_include_file("windows.h" HAVE_WINDOWS_H)
function(check_m)
    set(CMAKE_REQUIRED_LIBRARIES m)
    check_function_exists(sin HAVE_M)
endfunction(check_m)
check_m()
# Cargo coding the check if there is a library rt, possibly providing
# a canary symbol. I am not sure if this logic really mirrors what
# autotools does.
function(check_rt)
    set(CMAKE_REQUIRED_LIBRARIES rt)
    check_function_exists(clock_gettime HAVE_RT)
endfunction(check_rt)
check_rt()

check_function_exists(atoll HAVE_ATOLL)
check_function_exists(getaddrinfo HAVE_IPV6)
check_function_exists(mkfifo HAVE_MKFIFO)
check_function_exists(mmap HAVE_MMAP)
check_function_exists(nl_langinfo HAVE_NL_LANGINFO)
check_function_exists(random HAVE_RANDOM)
check_function_exists(setenv HAVE_SETENV)
check_function_exists(unsetenv HAVE_UNSETENV)
check_function_exists(setlocale HAVE_SETLOCALE)
check_function_exists(uselocale HAVE_USELOCALE)
check_function_exists(setpriority HAVE_SETPRIORITY)
check_function_exists(shmget HAVE_SHMGET)
check_function_exists(shmat HAVE_SHMAT)
check_function_exists(shmdt HAVE_SHMDT)
check_function_exists(shmctl HAVE_SHMCTL)
check_function_exists(strerror HAVE_STRERROR)
check_function_exists(strerror_l HAVE_STRERROR_L)
check_function_exists(strtok_r HAVE_STRTOK_R)
check_function_exists(strtok_s HAVE_STRTOK_S)
check_function_exists(fork HAVE_FORK)
check_function_exists(execvp HAVE_EXECVP)
check_function_exists(ctermid HAVE_CTERMID)
check_function_exists(clock_gettime HAVE_CLOCK_GETTIME)

check_type_size(off_t SIZEOF_OFF_T)

if(SIZEOF_OFF_T LESS 8)
check_function_exists(lseek64 LFS_LARGEFILE_64)

if(LFS_LARGEFILE_64)
check_c_source_compiles(
    "    #define _LARGEFILE64_SOURCE
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <fcntl.h>

    int main(){ return open(\"/foo\", O_RDONLY|O_LARGEFILE); }
    " HAVE_O_LARGEFILE)
endif()
endif()

search_libs(gethostbyname GETHOSTBYNAME_LIB nsl socket network)
search_libs(socket SOCKET_LIB socket)

test_big_endian(WORDS_BIGENDIAN)

check_c_source_compiles(
    "int main() { __asm__(\".balign 4\"); return 0; }"
    ASMALIGN_BALIGN)
check_c_source_compiles(
    "int main() { __asm__(\".align 3\"); return 0; }"
    ASMALIGN_EXP)
if(NOT ASMALIGN_EXP)
    set(ASMALIGN_BYTE ON)
endif()

check_c_source_compiles(
    "int main() { __attribute__((aligned(16))) float var; return 0; }"
    CCALIGN)

if(NOT LFS_INSENSITIVE)
    check_c_source_compiles("
        #include <sys/types.h>
          /* Check that off_t can represent 2**63 - 1 correctly.
            We can't simply define LARGE_OFF_T to be 9223372036854775807,
            since some C++ compilers masquerading as C compilers
            incorrectly reject 9223372036854775807.  */
        #define LARGE_OFF_T (((off_t) 1 << 62) - 1 + ((off_t) 1 << 62))
        int off_t_is_large[
            (LARGE_OFF_T % 2147483629 == 721 && LARGE_OFF_T % 2147483647 == 1) ? 1 : -1];
        int main() { return 0; }"
        LFS_INSENSITIVE)
endif()
if(NOT LFS_INSENSITIVE)
    check_c_source_compiles("
        #define _FILE_OFFSET_BITS 64
        #include <sys/types.h>
         /* Check that off_t can represent 2**63 - 1 correctly.
            We can't simply define LARGE_OFF_T to be 9223372036854775807,
            since some C++ compilers masquerading as C compilers
            incorrectly reject 9223372036854775807.  */
        #define LARGE_OFF_T (((off_t) 1 << 62) - 1 + ((off_t) 1 << 62))
        int off_t_is_large[
            (LARGE_OFF_T % 2147483629 == 721 && LARGE_OFF_T % 2147483647 == 1) ? 1 : -1];
        int main() { return 0; }"
        LFS_SENSITIVE)
endif()

if(WIN32 AND HAVE_WINDOWS_H)
    check_function_exists(GetThreadErrorMode HAVE_GETTHREADERRORMODE)
    if(HAVE_GETTHREADERRORMODE)
        check_c_source_compiles("
            #include <windows.h>

            int main() {
              LoadLibraryW(0);
              GetProcAddress(0, 0);
              FreeLibrary(0);
            }"
            HAVE_WIN_DL)
    else()
        set(HAVE_WIN_DL OFF)
    endif()
else()
    function(check_dl)
        set(CMAKE_REQUIRED_LIBRARIES dl)
        check_symbol_exists(dlopen "dlfcn.h" HAVE_UNIX_DL)
    endfunction(check_dl)
    check_dl()
endif()

set(DYNAMIC_BUILD ${BUILD_SHARED_LIBS})
if(MSVC)
    set(STDERR_FILENO "(_fileno(stderr))")
    set(STDIN_FILENO "(_fileno(stdin))")
    set(STDOUT_FILENO "(_fileno(stdout))")
endif()
set(HAVE_WIN32_FIFO ${WIN32})
set(WANT_WIN32_UNICODE ${WIN32})
set(WITH_SEEKTABLE 1000)

if(NOT WITH_SEEKTABLE EQUAL 0)
    set(FRAME_INDEX 1)
endif()

if(WIN32)
    set(HAVE_MKFIFO ON)
endif()

if(HAVE_NETDB_H AND HAVE_SYS_PARAM_H AND HAVE_SYS_SOCKET_H AND HAVE_NETINET_IN_H AND HAVE_ARPA_INET_H)
    set(HAVE_NETWORK ON)
endif()

if(NO_MESSAGES)
    set(NO_WARNING ON)
    set(NO_ERRORMSG ON)
    set(NO_ERETURN ON)
endif()

include(../cmake/CheckCPUArch.cmake)
check_cpu_arch_x86(ARCH_IS_X86)
check_cpu_arch_x64(ARCH_IS_X64)
check_cpu_arch_arm32(ARCH_IS_ARM32)
check_cpu_arch_arm64(ARCH_IS_ARM64)

if(WIN32 OR (ARCH_IS_ARM64 AND APPLE))
    set(HAVE_FPU 1)
else()
    cmake_host_system_information(RESULT HAVE_FPU QUERY HAS_FPU)
endif()

if(NOT HAVE_FPU)
    set(NO_SYNTH32 ON)
endif()

# Modules
# tildearrow: removed libout123

option(ACCURATE_ROUNDING "use rounding instead of fast truncation for integer output, where possible" ON)
cmake_dependent_option(FIFO "FIFO support for control interface" ON "HAVE_MKFIFO" OFF)
option(GAPLESS "enable gapless" ON)
option(IEEE_FLOAT "use special hackery relying on IEEE 754 floating point storage format (to accurately round to 16 bit integer at bit more efficiently in generic decoder, enabled by default, disable in case you have a very special computer)" ON)
cmake_dependent_option(IPV6 "IPv6 support (actually any protocol your libc does with getaddrinfo)" ON "HAVE_IPV6" OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    option(NAGGING "turn on GCC's pedantic nagging with error on warnings" OFF)
    if(NAGGING)
        set(CMAKE_C_STANDARD 99)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -pedantic")
    endif()
endif()
cmake_dependent_option(NETWORK "network support (http streams / webradio)" ON "HAVE_NETWORK" OFF)
option(NEWOLD_WRITE_SAMPLE "enable new/old WRITE_SAMPLE macro for non-accurate 16 bit output, faster on certain CPUs (default on on x86-32)" OFF)
cmake_dependent_option(NO_BUFFER "enable audio buffer code (default uses system whitelist... proper checks later)" OFF "HAVE_MMAP OR HAVE_SYS_IPC_H AND HAVE_SYS_SHM_H AND HAVE_SHMGET AND HAVE_SHMAT AND HAVE_SHMDT AND HAVE_SHMCTL" ON)
option(NO_DOWNSAMPLE "no downsampled decoding" OFF)
option(NO_EQUALIZER "no equalizer support" OFF)
option(NO_FEEDER "no feeder decoding, no buffered readers" OFF)
option(NO_ICY "ICY metainfo parsing/conversion" OFF)
option(NO_LAYER1 "no layer I decoding" OFF)
option(NO_LAYER2 "no layer II decoding" OFF)
option(NO_LAYER3 "no layer III decoding" OFF)
option(NO_MESSAGES "no error/warning messages on the console" OFF)
option(NO_MOREINFO "no extra information for frame analyzers" OFF)
option(NO_NTOM "no flexible resampling" OFF)
option(NO_16BIT "no 16 bit integer output" OFF)
option(NO_32BIT "no 32 bit integer output (also 24 bit)" OFF)
cmake_dependent_option(NO_8BIT "no 8 bit integer output" OFF "NOT NO_16BIT" ON)
option(NO_REAL "no real (floating point) output" OFF)
option(NO_STRING "no string API (this will disable ID3v2; main mpg123 won't build anymore)" OFF)
cmake_dependent_option(NO_ID3V2 "no ID3v2 parsing" OFF "NOT NO_STRING" ON)
option(SYN123_NO_CASES "include special cases for likely parameter values (channel count, encoding sizes in libsyn123 routines) in the hope of better optimization at the expense of some code bloat (default enabled)" OFF)
cmake_dependent_option(USE_MODULES "dynamically loadable output modules" OFF "BUILD_SHARED_LIBS;HAVE_WIN_DL OR HAVE_UNIX_DL" OFF)
option(USE_NEW_HUFFTABLE "use new huffman decoding scheme by Taihei (faster on modern CPUs at least, so on by default)" ON)


# again
set(TARGET compat)
add_library(${TARGET} OBJECT
    "compat/compat.c"
    "compat/compat_str.c")
if(BUILD_SHARED_LIBS)
    set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

add_library(${TARGET}_dl OBJECT
    "compat/compat_dl.c")
if(BUILD_SHARED_LIBS)
    set_target_properties(${TARGET}_dl PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

add_library(${TARGET}_str OBJECT
    "compat/compat_str.c")
if(BUILD_SHARED_LIBS)
    set_target_properties(${TARGET}_str PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

target_include_directories(${TARGET} PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}")
#    "${CMAKE_CURRENT_SOURCE_DIR}"

# a motherfiretrucking explosion because I NEED the include directory
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
add_library(${TARGET} STATIC
    "libmpg123/parse.c"
    "libmpg123/frame.c"
    "libmpg123/format.c"
    "libmpg123/dct64.c"
    "libmpg123/equalizer.c"
    "libmpg123/id3.c"
    "libmpg123/optimize.c"
    "libmpg123/readers.c"
    "libmpg123/tabinit.c"
    "libmpg123/libmpg123.c"
    "libmpg123/index.c"
    "$<$<NOT:$<BOOL:${NO_ICY}>>:libmpg123/icy.c>"
    "$<$<NOT:$<BOOL:${NO_ICY}>>:libmpg123/icy2utf8.c>"
    "$<$<NOT:$<BOOL:${NO_LAYER1}>>:libmpg123/layer1.c>"
    "$<$<NOT:$<OR:$<BOOL:${NO_LAYER1}>,$<BOOL:${NO_LAYER2}>>>:libmpg123/layer2.c>"
    "$<$<NOT:$<BOOL:${NO_LAYER3}>>:libmpg123/layer3.c>"
    "$<$<NOT:$<BOOL:${NO_NTOM}>>:libmpg123/ntom.c>"
    "$<$<NOT:$<BOOL:${NO_8BIT}>>:libmpg123/synth_8bit.c>"
    "$<$<NOT:$<BOOL:${NO_16BIT}>>:libmpg123/synth.c>"
    "$<$<AND:$<BOOL:${HAVE_FPU}>,$<NOT:$<BOOL:${NO_32BIT}>>>:libmpg123/synth_s32.c>"
    "$<$<AND:$<BOOL:${HAVE_FPU}>,$<NOT:$<BOOL:${NO_REAL}>>>:libmpg123/synth_real.c>"
    "$<$<NOT:$<BOOL:${NO_STRING}>>:libmpg123/stringbuf.c>"
    "$<$<NOT:$<BOOL:${NO_FEATURE_REPORT}>>:libmpg123/feature.c>"
    "$<$<NOT:$<BOOL:${PORTABLE_API}>>:libmpg123/lfs_wrap.c>"
    $<TARGET_OBJECTS:compat>)

add_library(MPG123::mpg123 ALIAS mpg123)

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
            "libmpg123/dct36_x86_64.S"
            "libmpg123/dct64_x86_64_float.S"
            "libmpg123/synth_x86_64_float.S"
            "libmpg123/synth_x86_64_s32.S"
            "libmpg123/synth_stereo_x86_64_float.S"
            "libmpg123/synth_stereo_x86_64_s32.S"
            "libmpg123/dct36_avx.S"
            "libmpg123/dct64_avx_float.S"
            "libmpg123/synth_stereo_avx_float.S"
            "libmpg123/synth_stereo_avx_s32.S"
            "libmpg123/getcpuflags_x86_64.S")
        target_sources(${TARGET} PRIVATE
            "libmpg123/dither.c")
        if(ACCURATE_ROUNDING)
            list(APPEND PLATFORM_SOURCES
                "libmpg123/synth_x86_64_accurate.S"
                "libmpg123/synth_stereo_x86_64_accurate.S"
                "libmpg123/synth_stereo_avx_accurate.S")
        else()
            list(APPEND PLATFORM_SOURCES
                "libmpg123/dct64_x86_64.S"
                "libmpg123/synth_x86_64.S"
                "libmpg123/synth_stereo_x86_64.S"
                "libmpg123/synth_stereo_avx.S"
                "libmpg123/dct64_avx.S")
        endif()
    else()
        set(PLATFORM_DEFINITIONS OPT_GENERIC)
    endif()
elseif(MACHINE STREQUAL "x86")
    if(TRUE)
        set(PLATFORM_DEFINITIONS OPT_I386)
        target_sources(${TARGET} PRIVATE
            "libmpg123/dct64_i386.c")
    endif()

    cmake_host_system_information(RESULT HAVE_SSE QUERY HAS_SSE)
    if(CMAKE_SYSTEM_NAME STREQUAL "Android")
        set(HAVE_SSE ${WITH_SSE})
    endif()
    if(HAVE_SSE)
        set(PLATFORM_DEFINITIONS OPT_SSE)
        set(PLATFORM_SOURCES
            "libmpg123/tabinit_mmx.S"
            "libmpg123/dct36_sse.S"
            "libmpg123/dct64_sse_float.S"
            "libmpg123/synth_sse_float.S"
            "libmpg123/synth_stereo_sse_float.S"
            "libmpg123/synth_sse_s32.S"
            "libmpg123/synth_stereo_sse_s32.S")
        if(ACCURATE_ROUNDING)
            list(APPEND PLATFORM_SOURCES
                "libmpg123/synth_sse_accurate.S"
                "libmpg123/synth_stereo_sse_accurate.S")
        else()
            list(APPEND PLATFORM_SOURCES
                "libmpg123/dct64_sse.S"
                "libmpg123/synth_sse.S")
        endif()
    endif()
elseif(MACHINE STREQUAL "arm64")
        set(PLATFORM_DEFINITIONS OPT_MULTI OPT_GENERIC OPT_GENERIC_DITHER OPT_NEON64)
        set(PLATFORM_SOURCES
            "libmpg123/dct36_neon64.S"
            "libmpg123/dct64_neon64_float.S"
            "libmpg123/synth_neon64_float.S"
            "libmpg123/synth_neon64_s32.S"
            "libmpg123/synth_stereo_neon64_float.S"
            "libmpg123/synth_stereo_neon64_s32.S"
            "libmpg123/check_neon.S")
        if(ACCURATE_ROUNDING)
            list(APPEND PLATFORM_SOURCES
                "libmpg123/synth_neon64_accurate.S"
                "libmpg123/synth_stereo_neon64_accurate.S")
        else()
            list(APPEND PLATFORM_SOURCES
                "libmpg123/synth_neon64.S"
                "libmpg123/synth_stereo_neon64.S")
        endif()
        target_sources(${TARGET} PRIVATE
            "libmpg123/dither.c"
            "libmpg123/getcpuflags_arm.c")
elseif(MACHINE STREQUAL "arm32")
    if(HAVE_FPU)
            set(PLATFORM_DEFINITIONS OPT_MULTI OPT_GENERIC OPT_GENERIC_DITHER OPT_NEON)
            set(PLATFORM_SOURCES
                "libmpg123/dct36_neon.S"
                "libmpg123/dct64_neon_float.S"
                "libmpg123/synth_neon_float.S"
                "libmpg123/synth_neon_s32.S"
                "libmpg123/synth_stereo_neon_float.S"
                "libmpg123/synth_stereo_neon_s32.S"
                "libmpg123/check_neon.S")
            target_sources(${TARGET} PRIVATE
                "libmpg123/getcpuflags_arm.c")
            if(ACCURATE_ROUNDING)
                list(APPEND PLATFORM_SOURCES
                    "libmpg123/synth_neon_accurate.S"
                    "libmpg123/synth_stereo_neon_accurate.S")
            else()
                list(APPEND PLATFORM_SOURCES
                    "libmpg123/dct64_neon.S"
                    "libmpg123/synth_neon.S"
                    "libmpg123/synth_stereo_neon.S")
            endif()
            target_sources(${TARGET} PRIVATE ${PLATFORM_SOURCES})
            target_sources(${TARGET} PRIVATE
                "libmpg123/dither.c")
    else()
            set(PLATFORM_DEFINITIONS OPT_ARM)
            if(ACCURATE_ROUNDING)
                set(PLATFORM_SOURCES
                    "libmpg123/synth_arm_accurate.S")
            else()
                set(PLATFORM_SOURCES
                    "libmpg123/synth_arm.S")
            endif()
            target_sources(${TARGET} PRIVATE ${PLATFORM_SOURCES})
    endif()
elseif(MACHINE STREQUAL "generic")
    set(PLATFORM_DEFINITIONS OPT_GENERIC)
endif()

#set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME mpg123)

target_compile_definitions(${TARGET} PRIVATE
    $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS>
    $<$<BOOL:${NO_BUFFER}>:NOXFERMEM>
    $<$<BOOL:${NEWOLD_WRITE_SAMPLE}>:NEWOLD_WRITE_SAMPLE>)

target_compile_options(${TARGET} PRIVATE
    $<$<BOOL:${MSVC}>:/wd4996>)

target_compile_definitions(${TARGET} PRIVATE
    $<$<BOOL:$<TARGET_PROPERTY:POSITION_INDEPENDENT_CODE>>:PIC>)

target_compile_definitions(${TARGET} PRIVATE
    ${PLATFORM_DEFINITIONS}
    $<$<BOOL:${HAVE_FPU}>:REAL_IS_FLOAT>
    $<$<NOT:$<BOOL:${HAVE_FPU}>>:REAL_IS_FIXED>)

if(MSVC AND MACHINE MATCHES "x86|amd64" AND YASM_ASSEMBLER)
    list(TRANSFORM PLATFORM_DEFINITIONS PREPEND /D)
    list(TRANSFORM PLATFORM_SOURCES PREPEND "${PROJECT_SOURCE_DIR}/src/")
    #message("PLATFORM_SOURCES: ${PLATFORM_SOURCES}")
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
#target_include_directories(${TARGET} INTERFACE
#    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
#    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
# what a hack
target_include_directories(${TARGET} PUBLIC
    "include"
    PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}")
#    "${CMAKE_CURRENT_SOURCE_DIR}"



configure_file(config.cmake.h.in config.h)

set(LIBMPG123_LIBS "${LIBMPG123_LIBS}" PARENT_SCOPE)
#set(LIBSYN123_LIBS "${LIBSYN123_LIBS}" PARENT_SCOPE)
