cmake_minimum_required(VERSION 3.12)

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

if(BUILD_LIBOUT123)
    if(CHECK_MODULES)
        list(FIND CHECK_MODULES alsa ALSA_REQUIRED)
        list(FIND CHECK_MODULES coreaudio COREAUDIO_REQUIRED)
        list(FIND CHECK_MODULES pulse PULSE_REQUIRED)
        list(FIND CHECK_MODULES jack JACK_REQUIRED)
        list(FIND CHECK_MODULES tinyalsa TINYALSA_REQUIRED)
        list(FIND CHECK_MODULES win32 WIN32_REQUIRED)
        list(FIND CHECK_MODULES win32_wasapi WIN32_WASAPI_REQUIRED)
        set(MODULE_NOT_FOUND_MESSAGE "module required but couldn't be found")
    endif()

    if(NOT CHECK_MODULES OR NOT ALSA_REQUIRED EQUAL -1)
        find_package(ALSA)
        if(TARGET ALSA::ALSA)
            list(APPEND OUTPUT_MODULES alsa)
        elseif(CHECK_MODULES AND NOT ALSA_REQUIRED EQUAL -1)
            message(FATAL_ERROR "alsa ${MODULE_NOT_FOUND_MESSAGE}")
        endif()
    endif()

    if(NOT CHECK_MODULES OR NOT COREAUDIO_REQUIRED EQUAL -1)
        if(APPLE)
            find_library(AUDIO_TOOLBOX AudioToolbox)
            list(APPEND OUTPUT_MODULES coreaudio)
        elseif(CHECK_MODULES AND NOT COREAUDIO_REQUIRED EQUAL -1)
            message(FATAL_ERROR "coreaudio ${MODULE_NOT_FOUND_MESSAGE}")
        endif()
    endif()

    find_package(PkgConfig)
    if(PKG_CONFIG_FOUND)
        if(NOT CHECK_MODULES OR NOT PULSE_REQUIRED EQUAL -1)
            pkg_search_module(PULSE libpulse-simple)
            if(PULSE_FOUND)
                list(APPEND OUTPUT_MODULES pulse)
            elseif(CHECK_MODULES AND NOT PULSE_REQUIRED EQUAL -1)
                message(FATAL_ERROR "pulse ${MODULE_NOT_FOUND_MESSAGE}")
            endif()
        endif()

        if(NOT CHECK_MODULES OR NOT JACK_REQUIRED EQUAL -1)
            pkg_search_module(JACK jack)
            if(JACK_FOUND)
                list(APPEND OUTPUT_MODULES jack)
            elseif(CHECK_MODULES AND NOT JACK_REQUIRED EQUAL -1)
                message(FATAL_ERROR "jack ${MODULE_NOT_FOUND_MESSAGE}")
            endif()
        endif()

        if(NOT CHECK_MODULES OR NOT TINYALSA_REQUIRED EQUAL -1)
            pkg_search_module(TINYALSA tinyalsa)
            if(TINYALSA_FOUND)
                list(APPEND OUTPUT_MODULES tinyalsa)
            elseif(CHECK_MODULES AND NOT TINYALSA_REQUIRED EQUAL -1)
                message(FATAL_ERROR "tinyalsa ${MODULE_NOT_FOUND_MESSAGE}")
            endif()
        endif()
    endif()

    if(NOT CHECK_MODULES OR NOT WIN32_REQUIRED EQUAL -1)
        if(HAVE_WINDOWS_H)
            set(WIN32_LIBRARIES winmm)
            list(APPEND OUTPUT_MODULES win32)
        elseif(CHECK_MODULES AND NOT WIN32_REQUIRED EQUAL -1)
            message(FATAL_ERROR "win32 ${MODULE_NOT_FOUND_MESSAGE}")
        endif()
    endif()

    if(NOT CHECK_MODULES OR NOT WIN32_WASAPI_REQUIRED EQUAL -1)
        set(WASAPI_INCLUDES "initguid.h" "audioclient.h" "mmdeviceapi.h" "avrt.h")
        check_include_files("${WASAPI_INCLUDES}" HAVE_WASAPI)
        if(HAVE_WASAPI)
            set(WIN32_WASAPI_LIBRARIES ole32 avrt)
            list(APPEND OUTPUT_MODULES win32_wasapi)
        elseif(CHECK_MODULES AND NOT WIN32_WASAPI_REQUIRED EQUAL -1)
            message(FATAL_ERROR "win32_wasapi ${MODULE_NOT_FOUND_MESSAGE}")
        endif()
    endif()

    if(CHECK_MODULES)
        list(REMOVE_AT CHECK_MODULES
            ${ALSA_REQUIRED}
            ${COREAUDIO_REQUIRED}
            ${PULSE_REQUIRED}
            ${JACK_REQUIRED}
            ${TINYALSA_REQUIRED}
            ${WIN32_REQUIRED}
            ${WIN32_WASAPI_REQUIRED})
        list(LENGTH CHECK_MODULES CHECK_MODULES_LENGTH)
        if(NOT CHECK_MODULES_LENGTH EQUAL 0)
            message(FATAL_ERROR "Dunno how to find modules: ${CHECK_MODULES}")
        endif()
    endif()

    if(NOT OUTPUT_MODULES)
        set(DEFAULT_OUTPUT_MODULE dummy)
        set(DEFAULT_OUTPUT_MODULES ${DEFAULT_OUTPUT_MODULE})
    else()
        list(GET OUTPUT_MODULES 0 _DEFAULT_OUTPUT_MODULE)
        set(DEFAULT_OUTPUT_MODULE ${_DEFAULT_OUTPUT_MODULE} CACHE STRING "Default output module")
        if(BUILD_SHARED_LIBS)
            string(REPLACE ";" "," DEFAULT_OUTPUT_MODULES "${OUTPUT_MODULES}")
        else()
            set(DEFAULT_OUTPUT_MODULES ${DEFAULT_OUTPUT_MODULE})
        endif()
        set_property(CACHE DEFAULT_OUTPUT_MODULE PROPERTY STRINGS ${OUTPUT_MODULES})
    endif()
endif()

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
cmake_dependent_option(USE_MODULES "dynamically loadable output modules" ON "BUILD_SHARED_LIBS;HAVE_WIN_DL OR HAVE_UNIX_DL" OFF)
option(USE_NEW_HUFFTABLE "use new huffman decoding scheme by Taihei (faster on modern CPUs at least, so on by default)" ON)


include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/include")

add_compile_definitions(
    $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS>
    $<$<BOOL:${NO_BUFFER}>:NOXFERMEM>
    $<$<BOOL:${NEWOLD_WRITE_SAMPLE}>:NEWOLD_WRITE_SAMPLE>)

add_compile_options(
    $<$<BOOL:${MSVC}>:/wd4996>)

add_subdirectory("compat")
add_subdirectory("libmpg123")
if(BUILD_LIBOUT123)
    add_subdirectory("libout123")
endif()
add_subdirectory("libsyn123")

if(UNIX AND NOT PORTABLE_API)
    option(BUILD_PROGRAMS "Build programs (mpg123 executable and others)" ON)

    if(BUILD_PROGRAMS)

        if(HAVE_FORK AND HAVE_EXECVP AND NETWORK)
            set(NET123 ON)
            set(NET123_EXEC ON)
        endif()

        if(HAVE_TERMIOS)
            set(TERM_POSIX ON)
        else()
            set(TERM_NONE ON)
        endif()

        add_subdirectory("tests")

        if(BUILD_LIBOUT123)
            add_executable(${PROJECT_NAME}
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/audio.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/common.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/sysutil.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/control_generic.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/equalizer.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/getlopt.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/httpget.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/resolver.c"
                $<$<BOOL:${NET123_EXEC}>:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/net123_exec.c>
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/genre.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/mpg123.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/metaprint.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/local.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/playlist.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/streamdump.c"
                $<$<BOOL:${TERM_POSIX}>:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/term_posix.c>
                $<$<BOOL:${TERM_NONE}>:${CMAKE_CURRENT_SOURCE_DIR}/../../../src/term_none.c>
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/term.c")
            target_link_libraries(${PROJECT_NAME} PRIVATE
                compat
                lib${PROJECT_NAME}
                libout123
                libsyn123
                ${GETHOSTBYNAME_LIB}
                ${SOCKET_LIB}
                $<$<BOOL:${HAVE_M}>:m>)

            add_executable(out123
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/sysutil.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/getlopt.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/filters.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/local.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/out123.c")
            target_link_libraries(out123 PRIVATE
                lib${PROJECT_NAME}
                libout123
                libsyn123)
            install(TARGETS ${PROJECT_NAME} out123
                EXPORT targets
                ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/"
                LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/"
                RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/")
        endif()

        add_executable(${PROJECT_NAME}-id3dump
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/mpg123-id3dump.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/getlopt.c")
        target_link_libraries(${PROJECT_NAME}-id3dump  PRIVATE
            lib${PROJECT_NAME})

        add_executable(${PROJECT_NAME}-strip
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/mpg123-strip.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/getlopt.c")
        target_link_libraries(${PROJECT_NAME}-strip  PRIVATE
            lib${PROJECT_NAME}
            libsyn123)

        install(TARGETS ${PROJECT_NAME}-id3dump ${PROJECT_NAME}-strip
            EXPORT targets
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/"
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/")

        install(
           FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../man1/mpg123.1"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../man1/out123.1"
           DESTINATION "${CMAKE_INSTALL_MANDIR}")

   endif()
endif()

configure_file(config.cmake.h.in config.h)

set(LIBMPG123_LIBS "${LIBMPG123_LIBS}" PARENT_SCOPE)
set(LIBOUT123_LIBS "${LIBOUT123_LIBS}" PARENT_SCOPE)
set(LIBSYN123_LIBS "${LIBSYN123_LIBS}" PARENT_SCOPE)
