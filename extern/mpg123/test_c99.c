// A little test program to group compile tests for standard C99 features
// we expect for mpg123 code. It is silly testing all headers and types
// individually, and unnecessarily slow. We mention all of the expected
// ones here and still fail early with a useful error for the user at
// configure stage.

/* This also nicely documents which features are expected. */

// Basic stuff. exit()
#include <stdlib.h>

// For the fixed-width integer types.
#include <stdint.h>
// For PRI macros.
#include <inttypes.h>
// ptrdiff_t, size_t
#include <stddef.h>
// _MAX/_MIN
#include <limits.h>
// printf
#include <stdio.h>

// strcmp, strlen
// remember: strcasecmp is POSIX, strings.h (sometimes)
#include <string.h>

int main()
{
	const char *s = "some test string";
	// Mention some types we want to use.
	size_t    st = 1337;
	ptrdiff_t pt = 0x3443; // used instead of non-standard ssize_t
	uint16_t u16 = 42;
	uint32_t u32 = 23<<20;
	int16_t i16  = 33;
	int32_t i32  = -1*(21L<<20);
	int64_t i64  = -1*(34L<<40);
	uintmax_t um = 304203;
	intmax_t  im = -23434;
	printf( "formats: "
		"%s"  " %s"    " %s"   " %s"   " %s"   " %s"   " %s"   " %s"    " %s " "\n"
	,	"zu", PRIiMAX, PRIu16, PRIu32, PRIi16, PRIi32, PRIi64, PRIuMAX, PRIiMAX );
	printf( "values:"
		" %zu" " %"PRIiMAX " %"PRIu16 " %"PRIu32 " %"PRIi16 " %"PRIi32 " %"PRIi64 " %"PRIuMAX " %"PRIiMAX "\n"
	,	  st,    pt,         u16,       u32,       i16,       i32,       i64,       um,         im );
	printf("%ld <= long <= %ld\n", LONG_MIN, LONG_MAX);
	printf( "The test string is %zu bytes long and compares to frick as %d\n"
	,	strlen(s), strcmp(s, "frick") );
	exit(EXIT_SUCCESS);
}
