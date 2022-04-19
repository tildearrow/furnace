#include "../nestypes.h"
#include "s_logtbl.h"
#include "s_opltbl.h"

// https://docs.google.com/Doc?docid=0Aeywjj51RsmGZGQ4a3FuOWZfMTNjcWprZjRncA&hl=en&pli=1 から取ってきた。
// よくこんなことできるなぁ・・・

Uint32 sintable_base[] = {
	2137,
	1731,
	1543,
	1419,
	1326,
	1252,
	1190,
	1137,
	1091,
	1050,
	1013,
	979,
	949,
	920,
	894,
	869,
	846,
	825,
	804,
	785,
	767,
	749,
	732,
	717,
	701,
	687,
	672,
	659,
	646,
	633,
	621,
	609,
	598,
	587,
	576,
	566,
	556,
	546,
	536,
	527,
	518,
	509,
	501,
	492,
	484,
	476,
	468,
	461,
	453,
	446,
	439,
	432,
	425,
	418,
	411,
	405,
	399,
	392,
	386,
	380,
	375,
	369,
	363,
	358,
	352,
	347,
	341,
	336,
	331,
	326,
	321,
	316,
	311,
	307,
	302,
	297,
	293,
	289,
	284,
	280,
	276,
	271,
	267,
	263,
	259,
	255,
	251,
	248,
	244,
	240,
	236,
	233,
	229,
	226,
	222,
	219,
	215,
	212,
	209,
	205,
	202,
	199,
	196,
	193,
	190,
	187,
	184,
	181,
	178,
	175,
	172,
	169,
	167,
	164,
	161,
	159,
	156,
	153,
	151,
	148,
	146,
	143,
	141,
	138,
	136,
	134,
	131,
	129,
	127,
	125,
	122,
	120,
	118,
	116,
	114,
	112,
	110,
	108,
	106,
	104,
	102,
	100,
	98,
	96,
	94,
	92,
	91,
	89,
	87,
	85,
	83,
	82,
	80,
	78,
	77,
	75,
	74,
	72,
	70,
	69,
	67,
	66,
	64,
	63,
	62,
	60,
	59,
	57,
	56,
	55,
	53,
	52,
	51,
	49,
	48,
	47,
	46,
	45,
	43,
	42,
	41,
	40,
	39,
	38,
	37,
	36,
	35,
	34,
	33,
	32,
	31,
	30,
	29,
	28,
	27,
	26,
	25,
	24,
	23,
	23,
	22,
	21,
	20,
	20,
	19,
	18,
	17,
	17,
	16,
	15,
	15,
	14,
	13,
	13,
	12,
	12,
	11,
	10,
	10,
	9,
	9,
	8,
	8,
	7,
	7,
	7,
	6,
	6,
	5,
	5,
	5,
	4,
	4,
	4,
	3,
	3,
	3,
	2,
	2,
	2,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

Uint32 exptable_base[] = {
	0,
	3,
	6,
	8,
	11,
	14,
	17,
	20,
	22,
	25,
	28,
	31,
	34,
	37,
	40,
	42,
	45,
	48,
	51,
	54,
	57,
	60,
	63,
	66,
	69,
	72,
	75,
	78,
	81,
	84,
	87,
	90,
	93,
	96,
	99,
	102,
	105,
	108,
	111,
	114,
	117,
	120,
	123,
	126,
	130,
	133,
	136,
	139,
	142,
	145,
	148,
	152,
	155,
	158,
	161,
	164,
	168,
	171,
	174,
	177,
	181,
	184,
	187,
	190,
	194,
	197,
	200,
	204,
	207,
	210,
	214,
	217,
	220,
	224,
	227,
	231,
	234,
	237,
	241,
	244,
	248,
	251,
	255,
	258,
	262,
	265,
	268,
	272,
	276,
	279,
	283,
	286,
	290,
	293,
	297,
	300,
	304,
	308,
	311,
	315,
	318,
	322,
	326,
	329,
	333,
	337,
	340,
	344,
	348,
	352,
	355,
	359,
	363,
	367,
	370,
	374,
	378,
	382,
	385,
	389,
	393,
	397,
	401,
	405,
	409,
	412,
	416,
	420,
	424,
	428,
	432,
	436,
	440,
	444,
	448,
	452,
	456,
	460,
	464,
	468,
	472,
	476,
	480,
	484,
	488,
	492,
	496,
	501,
	505,
	509,
	513,
	517,
	521,
	526,
	530,
	534,
	538,
	542,
	547,
	551,
	555,
	560,
	564,
	568,
	572,
	577,
	581,
	585,
	590,
	594,
	599,
	603,
	607,
	612,
	616,
	621,
	625,
	630,
	634,
	639,
	643,
	648,
	652,
	657,
	661,
	666,
	670,
	675,
	680,
	684,
	689,
	693,
	698,
	703,
	708,
	712,
	717,
	722,
	726,
	731,
	736,
	741,
	745,
	750,
	755,
	760,
	765,
	770,
	774,
	779,
	784,
	789,
	794,
	799,
	804,
	809,
	814,
	819,
	824,
	829,
	834,
	839,
	844,
	849,
	854,
	859,
	864,
	869,
	874,
	880,
	885,
	890,
	895,
	900,
	906,
	911,
	916,
	921,
	927,
	932,
	937,
	942,
	948,
	953,
	959,
	964,
	969,
	975,
	980,
	986,
	991,
	996,
	1002,
	1007,
	1013,
	1018
};

#if STATIC_TABLES

static void OplTableRelease(void *ctx)
{
}

static KMIF_OPLTABLE opl_static_tables = {
	&opl_static_tables;
	OplTableRelease,
//#include "s_oplt.h"
};

KMIF_OPLTABLE *OplTableAddRef(void)
{
	opl_static_tables.release = OplTableRelease;
	return &opl_static_tables;
}

#else

#include <math.h>
#ifndef M_PI
#ifdef PI
#define M_PI PI
#else
#define M_PI			3.14159265358979323846
#endif
#endif

#define AM_DEPTH1 4.875  /* dB */
#define AM_DEPTH2 1.0  /* dB */
#define PM_DEPTH1 13.75 /* cent */
#define PM_DEPTH2  7.0 /* cent */
#define LOG_KEYOFF (15 << (LOG_BITS + 1))
#define TLL_POW 1.7

#define DB0375_TO_LOG(x) ((Uint32)(0.375 * (1 << (LOG_BITS + x)) / 10))

#define AR_OFF (128 << ARTBL_SHIFT)
#define AR_MAX (127 << ARTBL_SHIFT)


static volatile Uint32 opl_tables_mutex = 0;
static Uint32 opl_tables_refcount = 0;
static KMIF_OPLTABLE *opl_tables = 0;

#define DEBUGX2 0
#if DEBUGX2
#include <stdio.h>
void dox(Uint32 *sint)
{
	static int i = 0;
	static FILE *fp = NULL;
	static int stop = 0;
	if (!fp) fp = fopen("c:\\kmz80d.out", "wb");
	if (fp) { 
	for (i = 0; i < (1 << SINTBL_BITS); i++)
	{
		fprintf(fp,"%08X\r\n",sint[i]);
	}
	if (fp) { fclose(fp); fp = 0; }
	}
}
#endif


static void OplTableRelease(void *ctx)
{
	++opl_tables_mutex;
	while (opl_tables_mutex != 1)
	{
		XSLEEP(0);
	}
	opl_tables_refcount--;
	if (!opl_tables_refcount)
	{
		XFREE(ctx);
		opl_tables = 0;
	}
	--opl_tables_mutex;
}

#define dB2(x) ((x)*2)


static void OplTableCalc(KMIF_OPLTABLE *tbl)
{
	Uint32 u, u2, i;

	for (i = 0 ;i < (1 << (SINTBL_BITS - 2)); i++)
	{
		Uint32 d;
		d = sintable_base[i]<<4;
		if (d > (LOG_KEYOFF >> 1)) d = (LOG_KEYOFF >> 1);
		tbl->sin_table[0][i] = d << 1;
		tbl->sin_table[0][(1 <<(SINTBL_BITS - 1)) -i -1] = d << 1;
		tbl->sin_table[0][(i + (1 << (SINTBL_BITS - 1)))] = (((Uint32)d) << 1) + 1;
		tbl->sin_table[0][(1 << (SINTBL_BITS)) -i -1] = (((Uint32)d) << 1) + 1;
	}
	tbl->sin_table[0][0] = tbl->sin_table[0][1 << (SINTBL_BITS - 1)] = LOG_KEYOFF;
/*
	for (i = 1 ;i < (1 << (SINTBL_BITS - 1)); i++)
	{
		Uint32 d;
		d = (Uint32)((1 << LOG_BITS) * -(log(sin(2.0 * M_PI * ((double)i) / (1 << SINTBL_BITS))) / log(2)));
		//d = (d / 256) * 256;
		if (d > (LOG_KEYOFF >> 1)) d = (LOG_KEYOFF >> 1);
		tbl->sin_table[0][i] = ((Uint32)d) << 1;
		tbl->sin_table[0][(i + (1 << (SINTBL_BITS - 1)))] = (((Uint32)d) << 1) + 1;
	}
*/
	//dox(tbl->sin_table[0]);
	for (i = 0 ;i < (1 << SINTBL_BITS); i++)
	{
		tbl->sin_table[1][i] = (tbl->sin_table[0][i] & 1) ? tbl->sin_table[0][0] : tbl->sin_table[0][i];
		tbl->sin_table[2][i] = tbl->sin_table[0][i] & ~1;
		tbl->sin_table[3][i] =  (i & (1 << (SINTBL_BITS - 2))) ? LOG_KEYOFF : tbl->sin_table[2][i];
	}
	for (i = 0; i < (1 << TLLTBL_BITS); i++)//i / 0.63 - log((1 << TLLTBL_BITS) - i)*4.5
	{
//		tbl->tll2log_table[i] = ((Uint32)(i * 0.375 * DB0375_TO_LOG(0) / (10-(i/((1 << TLLTBL_BITS)/3.0))))) << 1;
//		tbl->tll2log_table[i] = ((Uint32)((pow(i,TLL_POW)* 0.375 - log(pow(i,TLL_POW))) * DB0375_TO_LOG(0) 
//			/ pow(TLL_POW*10.875,TLL_POW))) << 1;
//		tbl->tll2log_table[i] = ((Uint32)((sin(M_PI/4*((double)i/(1 << TLLTBL_BITS)))*128 *0.375 - log(pow(i,TLL_POW))) * DB0375_TO_LOG(0) 
//			/ 3.725)) << 1;
//		tbl->tll2log_table[i] = ((Uint32)(LinToLog(logtbl,128-i*2))) << 1;
//		double a,logval;
//		a = (128 - (i<64?i*2:127)) * (1 << (LOG_LIN_BITS - LIN_BITS));
//		logval = (Uint32)((LOG_LIN_BITS - (log(a) / log(2))) * (1 << LOG_BITS));

//		tbl->tll2log_table[i] = (Uint32)((i) * (1 << LOG_BITS) * 0.625 / 10) << 1;
//		tbl->tll2log_table[i] = (Uint32)((i) * (1 << LOG_BITS) * 0.355 / 10) << 1;
		tbl->tll2log_table[i] = (Uint32)((1 << (LOG_BITS-5)) * exptable_base[i] * 0.675 ) << 1;
//		tbl->tll2log_table[i] = (Uint32)((1 << (LOG_BITS-5)) * exptable_base[i] * 0.4 ) << 1;
//		tbl->tll2log_table[i] = (i * DB0375_TO_LOG(0)) << 1;
	}
	for (i = 0; i < (1 << AMTBL_BITS); i++)
	{
		u  = (Uint32)((1 + sin(2 * M_PI * ((double)i) / (1 << AMTBL_BITS))) * ((1 << LOG_BITS) * AM_DEPTH1 / 20.0));
		u2 = (Uint32)((1 + sin(2 * M_PI * ((double)i) / (1 << AMTBL_BITS))) * ((1 << LOG_BITS) * AM_DEPTH2 / 20.0));
		//u2 = u;
		tbl->am_table1[i] = u << 1;
		tbl->am_table2[i] = u2 << 1;
	}
	for (i = 0; i < (1 << PMTBL_BITS); i++)
	{
		u  = (Uint32)((1 << PM_SHIFT) * pow(2, sin(2 * M_PI * ((double)i) / (1 << PMTBL_BITS)) * PM_DEPTH1 / 1200.0));
		u2 = (Uint32)((1 << PM_SHIFT) * pow(2, sin(2 * M_PI * ((double)i) / (1 << PMTBL_BITS)) * PM_DEPTH2 / 1200.0));
		//u2 = u;
		tbl->pm_table1[i] = u;
		tbl->pm_table2[i] = u2;
	}

	for (i = 0; i < (1 << ARTBL_BITS); i++)
	{
		u = (Uint32)(((double)AR_MAX) * (1 - log(1 + i) / log(1 << ARTBL_BITS)));
//	    AR_ADJUST_TABLE[i] = (uint32)((double)(1<<EG_BITS) - 1 - (1<<EG_BITS) * log(i) / log(128)) ; 
//		u = (Uint32)(((double)AR_MAX) - 1 - ((double)AR_MAX) * log(i) / log(128));
		tbl->ar_tablelog[i] = u;
#if 1
		u = (Uint32)(((double)AR_MAX) * (pow(1 - i / (double)(1 << ARTBL_BITS), 8)));
		tbl->ar_tablepow[i] = u;
#endif
	}
	tbl->ar_tablelog[0] = AR_MAX;
}

KMIF_OPLTABLE *OplTableAddRef()
{
	++opl_tables_mutex;
	while (opl_tables_mutex != 1)
	{
		XSLEEP(0);
	}
	if (!opl_tables_refcount)
	{
		opl_tables = XMALLOC(sizeof(KMIF_OPLTABLE));
		if (opl_tables)
		{
			opl_tables->ctx = opl_tables;
			opl_tables->release = OplTableRelease;
			OplTableCalc(opl_tables);
		}
	}
	if (opl_tables) opl_tables_refcount++;
	--opl_tables_mutex;
	return opl_tables;
}

#endif
