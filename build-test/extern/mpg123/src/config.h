// Define to only include portable API in libraries.
/* #undef PORTABLE_API */

// Define to use proper rounding.
#define ACCURATE_ROUNDING 1

// Define if .balign is present.
#define ASMALIGN_BALIGN 1

// Define if .align takes 3 for alignment of 2^3=8 bytes instead of 8.
/* #undef ASMALIGN_EXP */

// Define if .align just takes byte count.
#define ASMALIGN_BYTE 1

// Define if __attribute__((aligned(16))) shall be used
#define CCALIGN 1

#define DEFAULT_OUTPUT_MODULE ""

/* #undef DEBUG */
/* #undef DYNAMIC_BUILD */

// Define if FIFO support is enabled.
#define FIFO 1

// Define if frame index should be used.
#define FRAME_INDEX 1

#define GAPLESS 1
#define HAVE_ATOLL 1
#define HAVE_DIRENT_H 1
#define HAVE_DLFCN_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_LANGINFO_H 1
#define HAVE_LOCALE_H 1
#define HAVE_NL_LANGINFO 1
#define HAVE_RANDOM 1
#define HAVE_SCHED_H 1
#define HAVE_SETLOCALE 1
#define HAVE_USELOCALE 1
#define HAVE_SETPRIORITY 1
#define HAVE_SIGNAL_H 1
#define HAVE_STRERROR 1
#define HAVE_STRERROR_L 1
#define HAVE_STRTOK_R 1
/* #undef HAVE_STRTOK_S */
#define HAVE_FORK 1
#define HAVE_EXECVP 1
#define HAVE_CTERMID 1
#define HAVE_CLOCK_GETTIME 1
#define HAVE_STRINGS_H 1
#define HAVE_SYS_IOCTL_H 1
#define HAVE_SYS_RESOURCE_H 1
#define HAVE_SYS_SELECT_H 1
#define HAVE_SYS_SIGNAL_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_WAIT_H 1

// Define this if you have the POSIX termios library
#define HAVE_TERMIOS 1

#define HAVE_UNISTD_H 1
/* #undef HAVE_WINDOWS_H */

// Define to indicate that float storage follows IEEE754.
#define IEEE_FLOAT 1

#define INDEX_SIZE 1000

// Define if IPV6 support is enabled.
#define IPV6 1

#define LT_MODULE_EXT ".so"

// Define if network support is enabled.
#define NETWORK 1
// Define for new-style network code.
/* #undef NET123 */
/* #undef NET123_EXEC */
/* #undef NET123_WINHTTP */
/* #undef NET123_WININET */

// Define to disable downsampled decoding.
/* #undef NO_DOWNSAMPLE */

// Define to disable equalizer.
/* #undef NO_EQUALIZER */

// Define to disable error messages in combination with a return value (the return is left intact).
/* #undef NO_ERETURN */

// Define to disable error messages.
/* #undef NO_ERRORMSG */

// no feeder decoding, no buffered readers
/* #undef NO_FEEDER */

// Define to disable ICY handling.
/* #undef NO_ICY */

// Define to disable ID3v2 parsing.
/* #undef NO_ID3V2 */

// Define to disable layer I.
/* #undef NO_LAYER1 */

// Define to disable layer II.
/* #undef NO_LAYER2 */

// Define to disable layer III.
/* #undef NO_LAYER3 */

// Define to disable analyzer info.
/* #undef NO_MOREINFO */

// Define to disable ntom resampling.
/* #undef NO_NTOM */

// Define to disable 8 bit integer output.
/* #undef NO_8BIT */

// Define to disable 16 bit integer output.
/* #undef NO_16BIT */

// Define to disable 32 bit and 24 bit integer output.
/* #undef NO_32BIT */

// Define to disable real output.
/* #undef NO_REAL */

// Define to disable string functions.
/* #undef NO_STRING */

// Define for post-processed 32 bit formats.
/* #undef NO_SYNTH32 */

// Define to disable warning messages.
/* #undef NO_WARNING */

#define PACKAGE_NAME "mpg123"
#define PACKAGE_VERSION "1.33.3"

#define PKGLIBDIR "lib/mpg123"

// CMake leaves it emtpy for non-existing type. Autoconf sets it to 0.
#define SIZEOF_OFF_T (8+0)

/* #undef STDERR_FILENO */
/* #undef STDIN_FILENO */
/* #undef STDOUT_FILENO */

// Define to not duplicate some code for likely cases in libsyn123.
/* #undef SYN123_NO_CASES */

/* #undef USE_MODULES */

// Define for new Huffman decoding scheme.
#define USE_NEW_HUFFTABLE 1

// Define to use Unicode for Windows
/* #undef WANT_WIN32_UNICODE */

#ifdef WANT_WIN32_UNICODE
# define strcasecmp _stricmp
# define strncasecmp _strnicmp
#endif

// Define to use Win32 named pipes
/* #undef WANT_WIN32_FIFO */

/* #undef WORDS_BIGENDIAN */

/* #undef LFS_LARGEFILE_64 */
/* #undef LFS_SENSITIVE */
/* #undef HAVE_O_LARGEFILE */
