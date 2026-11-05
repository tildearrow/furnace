/*
	calctables: compute fixed decoder table values

	copyright ?-2021 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Michael Hipp (as tabinit.c, and parts of layer2.c and layer3.c),
	then printout added by Thomas Orgis

	This is supposed to compute the supposedly fixed tables that used to be computed
	live on library startup in mpg123_init().
*/

#define CALCTABLES
#include "mpg123lib_intern.h"
#include "../common/debug.h"

#define ASIZE(a) (sizeof(a)/sizeof(*a))

// The variable definitions here are redundant with the runtime part of
// the init_* headers, but that is fine, as those headers are generated
// by this very program.

static double cos64[16],cos32[8],cos16[4],cos8[2],cos4[1];
#include "init_costabs.h"

// layer I+II tables
static double layer12_table[27][64];

// Storing only values 0 to 26, so char is fine.
// The size of those might be reduced ... 
static unsigned char grp_3tab[32 * 3] = { 0, };   /* used: 27 */
static unsigned char grp_5tab[128 * 3] = { 0, };  /* used: 125 */
static unsigned char grp_9tab[1024 * 3] = { 0, }; /* used: 729 */

#include "init_layer12.h"

// layer III

static double ispow[8207]; // scale with SCALE_POW43
static double aa_ca[8],aa_cs[8];
static double win[4][36];
static double win1[4][36];
double INT123_COS9[9]; /* INT123_dct36_3dnow wants to use that */
static double COS6_1,COS6_2;
double INT123_tfcos36[9]; /* INT123_dct36_3dnow wants to use that */
static double tfcos12[3];
static double cos9[3],cos18[3];

// scale with SCALE_15
static double tan1_1[16],tan2_1[16],tan1_2[16],tan2_2[16];
// This used to be [2][32] initially, but already Taihei noticed that
// _apparently_ only [2][16] is used for the integer tables. But
// this is misleading: Accesses beyond that happen, at least in pathologic
// cases. Taihei's fixed-point decoding introduced read-only buffer
// overflows:-/ Those are rather harmless, though, as only bad numbers
// enter calculation. Nothing about pointers or code.
static double pow1_1[2][32],pow2_1[2][32],pow1_2[2][32],pow2_2[2][32];


// That needs special handling for storing the pointers
// in map and mapend.
//    map[j][N] = mapbufN[j];
// mapend[j][N] = ... varies let's use
// mapbufN[j]+(mapend[j][N]-map[j][N])
static short mapbuf0[9][152];
static short mapbuf1[9][156];
static short mapbuf2[9][44];
static short *map[9][3];
static short *mapend[9][3];

static unsigned short n_slen2[512]; /* MPEG 2.0 slen for 'normal' mode */
static unsigned short i_slen2[256]; /* MPEG 2.0 slen for intensity stereo */

static real gainpow2[256+118+4];

#include "l3bandgain.h"
#include "init_layer3.h"

// helpers

static void print_char_array( const char *indent, const char *name
,	size_t count, unsigned char tab[], int calc )
{
	if(calc)
	{
		if(name)
			printf("static unsigned char %s[%zu];\n", name, count);
		return;
	}
	size_t block = 72/4;
	size_t i = 0;
	if(name)
		printf("static const unsigned char %s[%zu] = \n", name, count);
	printf("%s{\n", indent);
	while(i<count)
	{
		size_t line = block > count-i ? count-i : block;
		printf("%s", indent);
		for(size_t j=0; j<line; ++j, ++i)
			printf("%s%c%3u", i ? "," : "", j ? ' ' : '\t', tab[i]);
		printf("\n");
	}
	printf("%s}%s\n", indent, name ? ";" : "");
}

static void print_value( int fixed, double fixed_scale
,	const char *name, double val, int calc )
{
	if(calc)
	{
		if(name)
			printf("static real %s;\n", name);
		return;
	}
	if(name)
		printf("static const real %s = ", name);
	if(fixed)
		printf("%ld;\n", (long)(DOUBLE_TO_REAL(fixed_scale*val)));
	else
		printf("%15.8ef;\n", val);
}

// I feal uneasy about inf appearing as literal.
// Do all C99 implementations support it the same?
// An unreasonably big value should also just work.
static double limit_val(double val)
{
	if(val > 1e38)
		return 1e38;
	if(val < -1e38)
		return -1e38;
	return val;
}

static void print_array( int statick, int fixed, double fixed_scale
,	const char *indent, const char *name
,	size_t count, double tab[], int calc )
{
	if(calc)
	{
		if(!fixed && name)
			printf( "%sALIGNED(16) real %s[%zu];\n"
			,	statick ? "static " : "", name, count );
		return;
	}
	size_t block = 72/18;
	size_t i = 0;
	if(name)
		printf( "%sconst%s real %s[%zu] = \n", statick ? "static " : ""
		,	fixed ? "" : " ALIGNED(16)", name, count );
	printf("%s{\n", indent);
	while(i<count)
	{
		size_t line = block > count-i ? count-i : block;
		printf("%s", indent);
		if(fixed) for(size_t j=0; j<line; ++j, ++i)
			printf( "%s%c%11ld", i ? "," : "", j ? ' ' : '\t'
			,	(long)(DOUBLE_TO_REAL(fixed_scale*tab[i])) );
		else for(size_t j=0; j<line; ++j, ++i)
			printf("%s%c%15.8ef", i ? "," : "", j ? ' ' : '\t', limit_val(tab[i]));
		printf("\n");
	}
	printf("%s}%s\n", indent, name ? ";" : "");
}

// C99 allows passing VLA with the fast dimensions first.
static void print_array2d( int fixed, double fixed_scale
,	const char *name, size_t x, size_t y
, double tab[][y], int calc )
{
	if(calc)
	{
		if(!fixed)
			printf("static ALIGNED(16) real %s[%zu][%zu];\n", name, x, y);
		return;
	}
	printf( "static const%s real %s[%zu][%zu] = \n{\n", fixed ? "" : " ALIGNED(16)"
	,	name, x, y );
	for(size_t i=0; i<x; ++i)
	{
		if(i)
			printf(",");
		print_array(1, fixed, fixed_scale, "\t", NULL, y, tab[i], calc);
	}
	printf("};\n");
}

static void print_short_array( const char *indent, const char *name
,	size_t count, short tab[], int calc )
{
	if(calc)
	{
		if(name)
			printf("static short %s[%zu];\n", name, count);
		return;
	}
	size_t block = 72/8;
	size_t i = 0;
	if(name)
		printf("static const short %s[%zu] = \n", name, count);
	printf("%s{\n", indent);
	while(i<count)
	{
		size_t line = block > count-i ? count-i : block;
		printf("%s", indent);
		for(size_t j=0; j<line; ++j, ++i)
			printf("%s%c%6d", i ? "," : "", j ? ' ' : '\t', tab[i]);
		printf("\n");
	}
	printf("%s}%s\n", indent, name ? ";" : "");
}

static void print_fixed_array( const char *indent, const char *name
,	size_t count, real tab[] )
{
	size_t block = 72/13;
	size_t i = 0;
	if(name)
		printf("static const real %s[%zu] = \n", name, count);
	printf("%s{\n", indent);
	while(i<count)
	{
		size_t line = block > count-i ? count-i : block;
		printf("%s", indent);
		for(size_t j=0; j<line; ++j, ++i)
			printf("%s%c%11ld", i ? "," : "", j ? ' ' : '\t', (long)tab[i]);
		printf("\n");
	}
	printf("%s}%s\n", indent, name ? ";" : "");
}

static void print_ushort_array( const char *indent, const char *name
,	size_t count, unsigned short tab[], int calc )
{
	if(calc)
	{
		if(name)
			printf("static unsigned short %s[%zu];\n", name, count);
		return;
	}
	size_t block = 72/8;
	size_t i = 0;
	if(name)
		printf("static const unsigned short %s[%zu] =\n", name, count);
	printf("%s{\n", indent);
	while(i<count)
	{
		size_t line = block > count-i ? count-i : block;
		printf("%s", indent);
		for(size_t j=0; j<line; ++j, ++i)
			printf("%s%c%8u", i ? "," : "", j ? ' ' : '\t', tab[i]);
		printf("\n");
	}
	printf("%s}%s\n", indent, name ? ";" : "");
}

// C99 allows passing VLA with the fast dimensions first.
static void print_short_array2d( const char *name, size_t x, size_t y
, short tab[][y], int calc )
{
	if(calc)
	{
		printf("static short %s[%zu][%zu];\n", name, x, y);
		return;
	}
	printf( "static const short %s[%zu][%zu] =\n{\n"
	,	name, x, y );
	for(size_t i=0; i<x; ++i)
	{
		if(i)
			printf(",");
		print_short_array("\t", NULL, y, tab[i], calc);
	}
	printf("};\n");
}

void printer(int calc, char *arg)
{
	// No fixed point output for run-time calculation.
	for(int fixed=0; fixed < (calc ? 1 : 2); ++fixed)
	{
		printf("\n#ifdef %s\n\n", fixed ? "REAL_IS_FIXED" : "REAL_IS_FLOAT");
		if(!fixed)
			printf("// aligned to 16 bytes for vector instructions, e.g. AltiVec\n\n");
		if(!strcmp("cos", arg))
		{
			print_array(1, fixed, 1., "", "cos64", ASIZE(cos64), cos64, calc);
			print_array(1, fixed, 1., "", "cos32", ASIZE(cos32), cos32, calc);
			print_array(1, fixed, 1., "", "cos16", ASIZE(cos16), cos16, calc);
			print_array(1, fixed, 1., "", "cos8",  ASIZE(cos8),  cos8,  calc);
			print_array(1, fixed, 1., "", "cos4",  ASIZE(cos4),  cos4,  calc);
		}
		if(!strcmp("l12", arg))
		{
			print_array2d( fixed, SCALE_LAYER12/REAL_FACTOR, "layer12_table"
			,	27, 64, layer12_table, calc );
		}
		if(!strcmp("l3", arg))
		{
			print_array( 1, fixed, SCALE_POW43/REAL_FACTOR, "", "ispow"
			,	sizeof(ispow)/sizeof(*ispow), ispow, calc );
			print_array(1, fixed, 1., "", "aa_ca", ASIZE(aa_ca), aa_ca, calc);
			print_array(1, fixed, 1., "", "aa_cs", ASIZE(aa_cs), aa_cs, calc);
			print_array2d(fixed, 1., "win", 4, 36, win, calc);
			print_array2d(fixed, 1., "win1", 4, 36, win1, calc);
			print_array(0, fixed, 1., "", "INT123_COS9", ASIZE(INT123_COS9), INT123_COS9, calc);
			print_value(fixed, 1., "COS6_1", COS6_1, calc);
			print_value(fixed, 1., "COS6_2", COS6_2, calc);
			print_array(0, fixed, 1., "", "INT123_tfcos36", ASIZE(INT123_tfcos36), INT123_tfcos36, calc);
			print_array(1, fixed, 1., "", "tfcos12", ASIZE(tfcos12), tfcos12, calc);
			print_array(1, fixed, 1., "", "cos9", ASIZE(cos9), cos9, calc);
			print_array(1, fixed, 1., "", "cos18", ASIZE(cos18), cos18, calc);
			print_array( 1, fixed, SCALE_15/REAL_FACTOR, ""
			,	"tan1_1", ASIZE(tan1_1), tan1_1, calc );
			print_array( 1, fixed, SCALE_15/REAL_FACTOR, ""
			,	"tan2_1", ASIZE(tan2_1), tan2_1, calc );
			print_array( 1, fixed, SCALE_15/REAL_FACTOR, ""
			,	"tan1_2", ASIZE(tan1_2), tan1_2, calc );
			print_array( 1, fixed, SCALE_15/REAL_FACTOR, ""
			,	"tan2_2", ASIZE(tan2_2), tan2_2, calc );
			print_array2d( fixed, SCALE_15/REAL_FACTOR
			,	"pow1_1", 2, 32, pow1_1, calc );
			print_array2d( fixed, SCALE_15/REAL_FACTOR
			,	"pow2_1", 2, 32, pow2_1, calc );
			print_array2d( fixed, SCALE_15/REAL_FACTOR
			,	"pow1_2", 2, 32, pow1_2, calc );
			print_array2d( fixed, SCALE_15/REAL_FACTOR
			,	"pow2_2", 2, 32, pow2_2, calc );
			if(fixed)
				print_fixed_array("", "gainpow2", ASIZE(gainpow2), gainpow2);
		}
		printf("\n#endif\n");
	}
	if(!strcmp("l12", arg))
	{
		printf("\n");
		print_char_array("", "grp_3tab", ASIZE(grp_3tab), grp_3tab, calc);
		printf("\n");
		print_char_array("", "grp_5tab", ASIZE(grp_5tab), grp_5tab, calc);
		printf("\n");
		print_char_array("", "grp_9tab", ASIZE(grp_9tab), grp_9tab, calc);
	}
	if(!strcmp("l3", arg))
	{
		printf("\n");
		print_short_array2d("mapbuf0", 9, 152, mapbuf0, calc);
		print_short_array2d("mapbuf1", 9, 156, mapbuf1, calc);
		print_short_array2d("mapbuf2", 9,  44, mapbuf2, calc);
		if(calc)
		{
			printf("static short *map[9][3];\n");
			printf("static short *mapend[9][3];\n");
		}
		else
		{
			printf("static const short *map[9][3] =\n{\n");
			for(int i=0; i<9; ++i)
				printf( "%s\t{ mapbuf0[%d], mapbuf1[%d], mapbuf2[%d] }\n"
				,	(i ? "," : ""), i, i, i );
			printf("};\n");
			printf("static const short *mapend[9][3] =\n{\n");
			for(int i=0; i<9; ++i)
				printf( "%s\t{ mapbuf0[%d]+%d, mapbuf1[%d]+%d, mapbuf2[%d]+%d }\n"
				,	(i ? "," : "")
				,	i, (int)(mapend[i][0]-map[i][0])
				,	i, (int)(mapend[i][1]-map[i][1])
				,	i, (int)(mapend[i][2]-map[i][2]) );
			printf("};\n");
		}
		print_ushort_array("", "n_slen2", ASIZE(n_slen2), n_slen2, calc);
		print_ushort_array("", "i_slen2", ASIZE(i_slen2), i_slen2, calc);
	}
}

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		fprintf(stderr, "usage:\n\t%s <cos|l12|l3>\n\n", argv[0]);
		return 1;
	}
	printf("// %s MPEG decoding tables\n", argv[1]);
	printf("// output of:\n// %s", argv[0]);
	for(int i=1; i<argc; ++i)
		printf(" %s", argv[i]);
	printf("\n");

	INT123_init_costabs();
	INT123_init_layer12();
	INT123_init_layer3();

	printf("\n#if defined(RUNTIME_TABLES)\n");
	printer(1, argv[1]);
	printf("\n#else\n");
	printer(0, argv[1]);
	printf("\n#endif\n");

	return 0;
}
