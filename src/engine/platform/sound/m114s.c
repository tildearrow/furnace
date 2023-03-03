// license:BSD-3-Clause

/**********************************************************************************************
 *
 *   SGS-Thomson Microelectronics M114S/M114A/M114AF Digital Sound Generator
 *   by Steve Ellenoff
 *   09/02/2004
 *
 *   Thanks to R.Belmont for Support & Agreeing to read through that nasty data sheet with me..
 *   Big thanks to Destruk for help in tracking down the data sheet.. Could have never done it
 *   without it!!
 *
 *   A note about the word "table" as used by the M114S datasheet. A table refers to rom data
 *   representing 1 full period/cycle of a sound. The chip always reads from 2 different tables
 *   and intermixes them during playback for smoother sound. Different table lengths simply allow
 *   the chip to play lower frequencies due to the fact that the table represents 1 full period,ie
 *   1 full sine wave, if that's what the rom data happens to contain.
 *
 *   It would seem that the chip comes in two versions - the M114A 4Mhz & M114AF 6Mhz versions. Unlike other sound chips which
 *   allow for clock variations on the same chip, this chip uses hard coded frequency tables
 *   in an internal ROM based on which version of the chip is being used.
 *
 *   Some ideas were taken from the source from BSMT2000 & AY8910
 *
 *   Heavily modified/extended by Carsten Waechter later-on
 *
 *   The mixer can optionally mix every (internal) channel into a separate stream for testing (set M114S_OUTPUT_CHANNELS to 16)
 *
 *   TODO: - No full/tested support for the M114AF 6Mhz version of the chip (but seems at least to work 'good enough')
 *         - Maybe also low pass filter heavily/some channels only?? (some channels sound too high frequency heavy (clicks, pops), for example on Dakar)
 **********************************************************************************************/

#include "driver.h"
#include "m114s.h"
#include <stdbool.h>

#if defined(_MSC_VER) && (_MSC_VER <= 1500)
 #define llabs _abs64
#endif

#define SAMPLE_RATE 48000

// by default mix all 4 HW channels into 1 stream for efficiency reasons:
#define M114S_OUTPUT_CHANNELS 1/*4*//*16*/ // HW uses 4, but if set to 16 instead, we simply output each (internal) channel independently (which is not how the real chip works, as that one mixes the 16 down to 4 outputs)

#define USE_FREQTABLE_FROM_MANUAL // undef to use a generated freqtable with 'correct' semitones (i.e. unlike the real chip) for the M114A 4MHz

//!! unsure how the actual vol envelope actually works internally and at what precision -> spec says 'either immediate' (=no envelope) or 'gradual increments of 1/256 of maximum amplitude'
// BUT THEN also mentions: only 8 MSBits from the 10 of v_linear are influenced! -> simple increase/decrease of the 8 MSBits with same frequency as table read (if diff > 128, every 2nd read if diff > 64, every 4th read if diff > 32 or every 8th read if diff <= 32)
#define USE_VOL_ENVELOPE // to test no volume envelope (=no smooth blending in of new volume that is set), set to 0

#define DO_FULL_PRECISION_MIXING // unclear how the mixing of channels and interpolation of channels works internally, so this allows to switch between two modes (use highest precision vs a lower precision/interpretation of the datasheet text)
#define USE_LERP_FOR_REPEATED_SAMPLES // unclear how repeated table reads really work, datasheet can be interpreted either way

#define MR_GAME_VOLUME_HACK // rather do this for specific machines only!? (i.e. is this only due to the filter network of output channels of Mr.Game??!)

#if 0
#include <stdio.h>
#include <stdlib.h>
#define LOG(x) printf x
#else
#define LOG(x) logerror x
#endif

/**********************************************************************************************

     CONSTANTS

***********************************************************************************************/

#define M114S_CHANNELS			16				// Chip has 16 internal channels for sound output

#define FRAC_BITS				16
#define FRAC_ONE				(1 << FRAC_BITS)
#define FRAC_MASK				(FRAC_ONE - 1)

/**********************************************************************************************

     INTERNAL DATA STRUCTURES

***********************************************************************************************/

/* struct describing a single table */
struct M114STable
{
		UINT8			reread;						/* # of times to re-read each byte from table */
		//UINT32			position;					/* current reading position for table */
		UINT32			start_address;				/* start address (offset into ROM) for table */
		//UINT32			stop_address;				/* stop address (offset into ROM) for table */
		UINT16			length;						/* length in bytes of the table */
		UINT16			total_length;				/* total length in bytes of the table (including repetitions) */
};

/* struct describing the registers for a single channel */
struct M114SChannelRegs
{
		UINT8			atten;							/* Attenuation Register */
		UINT8			outputs;						/* Output Pin Register */
		UINT8			table1_addr;					/* Table 1 MSB Starting Address Register */
		UINT8			table2_addr;					/* Table 2 MSB Starting Address Register */
		UINT8			table_len;						/* Table Length Register */
		UINT8			read_meth;						/* Read Method Register */
		UINT8			interp;							/* Interpolation Register */
		bool			env_enable;						/* Envelope Enable Register */
		bool			oct_divisor;					/* Octave Divisor Register */
		UINT8			frequency;						/* Frequency Register */
};

/* struct describing a single playing channel */
struct M114SChannel
{
	/* registers */
		struct M114SChannelRegs regs;					/* register data for the channel */
	/* internal state */
		bool			active;							/* is the channel active */
		INT16			output[4096];					/* Holds output samples mixed from table 1 & 2 */
		UINT32			outpos;							/* Index into output samples */
		int				prev_volume;					/* Holds the last volume code set for the channel */
		int				current_volume;					/* Volume to apply to each sample output */
		int				target_volume;					/* Target to match for volume envelope */
		int				step_rate_volume_env;			/* Volume step for volume envelope */
		//int				end_of_table;					/* End of Table Flag */
		struct M114STable table1;						/* Table 1 Data */
		struct M114STable table2;						/* Table 2 Data */
		UINT32			incr;							/* Current Sample Rate/Step size increase to play back table */
};

/* struct describing the entire M114S chip */
struct M114SChip
{
	/* vars to be reset when the chip is reset */
	int							bytes_read;					/* # of bytes read */
	int							channel;					/* Which channel is being programmed via the data bus */
	struct M114SChannelRegs		tempch_regs;				/* temporary channel register data for gathering the data programming */
	struct M114SChannel			channels[M114S_CHANNELS];	/* All the chip's internal channels */
	int							channel_volume[4];			/* scale for the HW/chip outputs */
	/* static vars, ie do not reset values */
	int							stream;						/* which stream are we using */
	INT8 *						region_base;				/* pointer to the base of the ROM region */
	struct M114Sinterface*		intf;						/* Pointer to the interface */
	double						reset_cycles;				/* # of cycles that must pass between programming bytes to auto reset the chip */
	int							cpu_num;					/* # of the cpu controlling the M114S */
	bool 						is_M114A;					/* M114A 4MHz or M114AF 6MHz */
};



/**********************************************************************************************

     GLOBALS

***********************************************************************************************/
static struct M114SChip m114schip[MAX_M114S];		//Each M114S chip

/* Table 1 & Table 2 Repetition Values based on Mode */
static const int mode_to_rep[8][2] = {
  {2,2},	//Mode 0
  {1,1},	//Mode 1
  {4,4},	//Mode 2
  {1,1},	//Mode 3
  {1,2},	//Mode 4
  {1,1},	//Mode 5
  {1,4},	//Mode 6
  {1,1}		//Mode 7
};

/* Table 1 Length Values based on Mode */ // these are always larger as table 2 values! (thus mixing code below makes sense to not handle the other case)
static const int mode_to_len_t1[8][8] = {
{16,32,64,128,256,512,1024,2048 },	//Mode 0
{16,32,64,128,256,512,1024,2048 },	//Mode 1
{16,32,64,128,256,512,1024,1024 },	//Mode 2
{16,32,64,128,256,512,1024,2048 },	//Mode 3
{16,32,64,128,256,512,1024,2048 },	//Mode 4
{16,32,64,128,256,512,1024,2048 },	//Mode 5
{16,32,64,128,256,512,1024,2048 },	//Mode 6
{16,32,64,128,256,512,1024,2048 },	//Mode 7
};

/* Table 2 Length Values based on Mode */
static const int mode_to_len_t2[8][8] = {
{16,32,64,128,256,512,1024,2048 },	//Mode 0 //datasheet has last value = 1048 //!! -> try 1024 instead??
{16,32,64,128,256,512,1024,2048 },	//Mode 1
{16,32,64,128,256,512,1024,1024 },	//Mode 2
{16,32,64,128,256,512,1024,2048 },	//Mode 3
{ 8,16,32, 64,128,256, 512,1024 },	//Mode 4
{16,16,16, 32, 64,128, 256, 512 },	//Mode 5
{ 4, 8,16, 32, 64,128, 256, 512 },	//Mode 6
{16,16,16, 16, 32, 64, 128, 256 },	//Mode 7
};

/* Attenuation Table */
static const int v_linear[32] = {
1023,939,863,791,727,667,611,559,515,471,431,
 395,363,335,307,283,259,235,215,199,183,166,
 152,140,128,117,107, 98, 90, 83, 76, 69
};
// a_decibel = 20 * log((v_linear+1)/1024)
/*static const double a_decibel[32] = {
 0.00, 0.74, 1.48, 2.23, 2.96, 3.71, 4.47, 5.24, 5.95,
 6.73, 7.50, 8.25, 8.98, 9.68,10.43,11.14,11.91,12.75,
13.52,14.19,14.91,15.75,16.51,17.22,17.99,18.77,19.54,
20.29,21.03,21.72,22.48,23.30
};*/

#ifdef USE_FREQTABLE_FROM_MANUAL
/* Frequency Table for a 4Mhz Clocked Chip */
static const double freqtable4Mhz[256] = {
 1016.78,1021.45,1026.69,1031.46,1036.27,1041.67,1044.39,1045.48,
 1046.57,1047.67,1048.77,1051.52,1056.52,1061.57,1066.67,1071.81,	//0x00 - 0x0F
 1077.01,1082.25,1087.55,1092.90,1098.30,1103.14,1106.81,1107.42,
 1108.65,1109.88,1111.11,1114.21,1119.19,1124.86,1130.58,1135.72,	//0x10 - 0x1F
 1140.90,1146.79,1152.07,1158.08,1163.47,1168.91,1172.33,1173.71,
 1174.40,1175.78,1177.16,1180.64,1186.24,1191.90,1197.60,1203.37,	//0x20 - 0x2F
 1209.19,1215.07,1221.00,1226.99,1232.29,1238.39,1242.24,1243.78,
 1244.56,1245.33,1246.88,1250.78,1256.28,1262.63,1269.04,1274.70,	//0x30 - 0x3F
 1281.23,1287.00,1293.66,1299.55,1305.48,1312.34,1315.79,1317.52,
 1318.39,1319.26,1321.00,1324.50,1331.56,1337.79,1344.09,1350.44,	//0x40 - 0x4F
 1356.85,1363.33,1369.86,1376.46,1383.13,1389.85,1393.73,1395.67,
 1396.65,1397.62,1398.60,1403.51,1410.44,1417.43,1424.50,1430.62,	//0x50 - 0x5F
 1437.81,1445.09,1451.38,1458.79,1466.28,1472.75,1478.20,1479.29,
 1480.38,1481.48,1482.58,1486.99,1494.77,1501.50,1508.30,1516.30,	//0x60 - 0x6F
 1523.23,1530.22,1538.46,1545.60,1552.80,1560.06,1564.95,1566.17,
 1567.40,1568.63,1569.86,1576.04,1583.53,1591.09,1598.72,1606.43,	//0x70 - 0x7F
 1614.21,1622.06,1629.99,1638.00,1644.74,1652.89,1658.37,1659.75,
 1661.13,1662.51,1663.89,1669.45,1677.85,1684.92,1693.48,1702.13,	//0x80 - 0x8F
 1709.40,1718.21,1727.12,1734.61,1743.68,1751.31,1757.47,1759.01,	//(2nd entry in manual shows 1781.21 but that's cleary wrong)
 1760.56,1762.11,1763.89,1768.35,1777.78,1785.71,1793.72,1803.43,	//0x90 - 0x9F
 1811.59,1819.84,1829.83,1838.24,1846.72,1855.29,1860.47,1862.20,
 1863.93,1865.67,1867.41,1874.41,1883.24,1892.15,1901.14,1910.22,	//0xA0 - 0xAF
 1919.39,1928.64,1937.98,1947.42,1956.95,1966.57,1972.39,1974.33,
 1976.28,1978.24,1980.20,1984.13,1994.02,2004.01,2014.10,2024.29,	//0xB0 - 0xBF
 2032.52,2042.90,2053.39,2063.98,2072.54,2083.33,2087.68,2089.86,
 2092.05,2094.24,2096.44,2103.05,2114.16,2123.14,2134.47,2143.62,	//0xC0 - 0xCF
 2155.17,2164.50,2176.28,2185.79,2195.39,2207.51,2212.39,2214.84,
 2217.29,2219.76,2222.22,2227.17,2239.64,2249.72,2259.89,2272.73,	//0xD0 - 0xDF
 2283.11,2293.58,2304.15,2314.81,2325.58,2339.18,2344.67,2347.42,
 2350.18,2352.94,2355.71,2361.28,2372.48,2383.79,2395.21,2406.74,	//0xE0 - 0xEF
 0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,													//0xF0 - 0xFF (Special codes)
};

/* Frequency Table for a 6Mhz Clocked Chip */
static const double freqtable6Mhz[256] = {
 1523.78,1523.79,1538.64,1545.79,1552.99,1561.08,1565.16,1566.80,
 1568.44,1570.08,1571.73,1575.86,1583.35,1590.91,1598.55,1606.26,	//0x00 - 0x0F
 1614.04,1621.90,1629.84,1637.86,1645.95,1653.22,1658.71,1659.62,
 1661.46,1663.31,1665.16,1669.79,1677.27,1685.76,1694.34,1702.03,	//0x10 - 0x1F
 1709.80,1718.62,1726.54,1735.54,1743.62,1751.77,1758.91,1758.97,
 1760.00,1762.07,1764.14,1769.35,1777.75,1786.22,1794.78,1803.42,	//0x20 - 0x2F
 1812.14,1820.95,1829.84,1838.82,1846.75,1855.90,1861.66,1863.98,
 1865.14,1866.30,1868.63,1874.47,1882.71,1892.22,1901.83,1910.31,	//0x30 - 0x3F
 1920.10,1928.75,1938.73,1947.55,1956.45,1966.72,1971.89,1974.49,
 1975.79,1977.10,1979.71,1984.95,1995.53,2004.87,2014.30,2023.82,	//0x40 - 0x4F
 2033.43,2043.14,2052.93,2062.82,2072.82,2082.89,2088.70,2091.61,
 2093.07,2094.54,2096.00,2103.35,2113.74,2124.22,2134.81,2143.98,	//0x50 - 0x5F
 2154.77,2165.66,2175.09,2186.20,2197.42,2207.13,2215.28,2216.92,
 2218.56,2220.21,2221.85,2228.46,2240.12,2250.21,2260.39,2272.39,	//0x60 - 0x6F
 2282.77,2293.25,2305.60,2316.29,2327.08,2337.97,2345.29,2347.13,
 2348.97,2350.81,2352.65,2361.92,2373.14,2384.47,2395.91,2407.45,	//0x70 - 0x7F
 2419.11,2430.88,2442.77,2454.77,2464.87,2477.09,2485.31,2487.37,
 2489.44,2491.50,2493.58,2501.90,2514.50,2525.09,2537.92,2550.88,	//0x80 - 0x8F
 2561.78,2574.98,2588.32,2599.55,2613.15,2624.59,2633.81,2636.13,
 2638.45,2640.78,2643.10,2650.11,2664.25,2676.14,2688.14,2702.69,	//0x90 - 0x9F
 2714.93,2727.28,2742.25,2754.85,2767.57,2780.41,2788.17,2790.76,
 2793.06,2795.97,2798.58,2809.07,2822.30,2835.65,2849.13,2862.73,	//0xA0 - 0xAF
 2876.47,2890.34,2904.34,2918.48,2932.76,2947.18,2955.90,2958.82,
 2961.74,2964.67,2967.60,2973.49,2988.31,3003.29,3018.41,3033.68,	//0xB0 - 0xBF
 3046.02,3061.57,3077.29,3093.17,3105.99,3122.17,3128.68,3131.95,
 3135.23,3138.51,3141.80,3151.71,3168.37,3181.83,3198.80,3212.52,	//0xC0 - 0xCF
 3229.83,3243.81,3261.46,3275.72,3290.10,3308.26,3315.58,3319.25,
 3322.93,3326.61,3330.31,3337.73,3356.42,3371.52,3386.76,3406.00,	//0xD0 - 0xDF
 3421.55,3437.25,3453.09,3469.07,3485.21,3505.59,3513.81,3517.93,
 3522.07,3526.21,3530.37,3538.70,3555.49,3572.44,3589.56,3606.84,	//0xE0 - 0xEF
 0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,													//0xF0 - 0xFF (Special codes)
};
#else
/* Frequency Table for a 4Mhz Clocked Chip
   Note: Only the 1st set of frequencies came from the manual, the rest were calculated using 1 semitone increments
   using a program called Test Tone Generator 3.91
*/
static const double freqtable4Mhz[256] = {
 1016.78,1021.45,1026.69,1031.46,1036.27,1041.67,1044.39,1045.48,
 1046.57,1047.67,1048.77,1051.52,1056.52,1061.57,1066.67,1071.81,	//0x00 - 0x0F
 1077.24,1082.19,1087.74,1092.79,1097.89,1103.61,1106.49,1107.65,
 1108.80,1109.97,1111.13,1114.05,1119.34,1124.69,1130.10,1135.54,	//0x10 - 0x1F
 1141.29,1146.54,1152.42,1157.77,1163.17,1169.24,1172.29,1173.51,
 1174.74,1175.97,1177.20,1180.29,1185.90,1191.57,1197.30,1203.07,	//0x20 - 0x2F
 1209.16,1214.72,1220.95,1226.62,1232.34,1238.76,1242.00,1243.29,
 1244.59,1245.90,1247.20,1250.47,1256.42,1262.43,1268.49,1274.60,	//0x30 - 0x3F
 1281.06,1286.95,1293.55,1299.56,1305.62,1312.42,1315.85,1317.22,
 1318.60,1319.98,1321.37,1324.83,1331.13,1337.49,1343.92,1350.40,	//0x40 - 0x4F
 1357.24,1363.47,1370.46,1376.83,1383.25,1390.46,1394.09,1395.55,
 1397.00,1398.47,1399.94,1403.61,1410.29,1417.03,1423.83,1430.70,	//0x50 - 0x5F
 1437.94,1444.55,1451.96,1457.07,1465.51,1473.14,1477.00,1478.53,
 1480.07,1481.63,1483.18,1486.99,1494.14,1501.29,1508.50,1515.77,	//0x60 - 0x6F
 1523.45,1530.45,1538.30,1545.44,1552.65,1560.74,1564.82,1566.45,
 1568.08,1569.73,1571.38,1575.50,1583.00,1590.56,1598.20,1605.90,	//0x70 - 0x7F
 1614.04,1621.45,1629.77,1637.34,1644.98,1653.55,1657.87,1659.60,
 1661.33,1663.07,1664.82,1669.18,1677.12,1685.14,1693.23,1701.39,	//0x80 - 0x8F
 1710.01,1717.87,1726.69,1734.70,1742.79,1751.87,1756.45,1758.28,
 1760.11,1761.96,1763.81,1768.44,1776.85,1785.34,1793.92,1802.56,	//0x90 - 0x9F
 1811.70,1820.02,1829.35,1837.85,1846.42,1856.05,1860.89,1862.83,
 1864.78,1866.74,1868.70,1873.60,1882.50,1891.50,1900.59,1909.75,	//0xA0 - 0xAF
 1919.43,1928.24,1938.13,1947.14,1956.22,1966.41,1971.55,1973.60,
 1975.66,1977.74,1979.81,1985.01,1994.44,2003.98,2013.61,2023.31,	//0xB0 - 0xBF
 2033.56,2042.90,2053.38,2062.92,2072.54,2083.34,2088.78,2090.96,
 2093.14,2095.34,2097.54,2103.04,2113.04,2123.14,2133.34,2143.62,	//0xC0 - 0xCF
 2154.48,2164.38,2175.48,2185.59,2195.78,2207.22,2212.99,2215.30,
 2217.60,2219.94,2222.27,2228.09,2238.69,2249.39,2260.20,2271.09,	//0xD0 - 0xDF
 2282.59,2293.08,2304.84,2315.55,2326.35,2338.47,2344.58,2347.02,
 2349.47,2351.94,2354.41,2360.58,2371.80,2383.14,2394.59,2406.13,	//0xE0 - 0xEF
 0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,													//0xF0 - 0xFF (Special codes)
};
#endif

/**********************************************************************************************

	 read_sample -- returns 1 sample from the channel's output buffer, but upsamples/interpolates the data
	 as necessary to match the Machine driver's output sample rate.

***********************************************************************************************/
static INT16 read_sample(struct M114SChannel * const channel, const UINT32 length)
{
	const UINT32 pos = channel->outpos >> FRAC_BITS;
	if (pos < length)
	{
		const INT32 frac = channel->outpos & FRAC_MASK;

		// interpolate
		const INT16 val1 = channel->output[pos];
		const INT16 val2 = channel->output[pos+1 < length ? pos : 0]; // wrap around to 0 (see code below!)?
		const INT16 sample = (val1 * ((INT32)FRAC_ONE - frac) + val2 * frac) >> FRAC_BITS;

		channel->outpos += channel->incr;
		return sample;
	}
	else {
		//LOG(("End of Table\n"));
		//channel->end_of_table++;
		channel->outpos = 0; // related to cyclic reading of tables //!! rather wrap over including exact FRAC_BITS instead of just nulling it?
		return channel->output[0]; //!! dto. would need interpolation then
	}
}

/**********************************************************************************************

	 read_table -- Reads the two tables of rom data into a temporary buffer,
	 mixes the samples using the chip's internal interpolation equation,
	 applies the volume, and writes the single mixed sample to the output buffer for the channel.
	 It processes the entire table1 length of data.

	 Note: Eventually this should flag an End of Table, and should process new table data

***********************************************************************************************/
static void read_table(struct M114SChip * const chip, struct M114SChannel * const channel) // get rid of this and write directly to output buffer?!
{
	int i;
#ifndef USE_LERP_FOR_REPEATED_SAMPLES
	int j;
#endif
	const INT8 * const rom = &chip->region_base[0];
	const int t1start = channel->table1.start_address;
	const int t2start = channel->table2.start_address;
	const int lent1 = channel->table1.length;
	const int lent2 = channel->table2.length;
	const int rep1 = channel->table1.reread;
	const int rep2 = channel->table2.reread;
	const int intp = channel->regs.interp;

	INT8 tb1[4096];	// Temp copy buffer for Table 1 // long enough to hold max sizes of 2048*2 or 1024*4
	INT8 tb2[4096];	// Temp copy buffer for Table 2 // dto.
	memset(&tb1,0,sizeof(tb1));
	memset(&tb2,0,sizeof(tb2));

	//LOG(("t1s = %d t2s = %d, l1=%d l2=%d, r1=%d, r2=%d, int = %d\n",t1start,t2start,lent1,lent2,rep1,rep2,intp));

	// datasheet says: multiple reading permits interpolation between two adjoining samples on the same table, so do we really need to lerp instead of just repeating same values here?
#ifdef USE_LERP_FOR_REPEATED_SAMPLES
	//Scan Table 1
	if(rep1 == 1)
		memcpy(tb1,&rom[t1start],lent1);
	else if(rep1 == 2)
		for(i=0; i<lent1; i++)
		{
			tb1[i*2+0] =       rom[i+t1start];
			tb1[i*2+1] = ((int)rom[i+t1start] + (int)rom[t1start + ((i < lent1-1) ? i+1 : i)])/2;
		}
	else //if(rep1 == 4)
		for(i=0; i<lent1; i++)
		{
			int val1 = (int)rom[i+t1start];
			int val2 = (int)rom[t1start + ((i < lent1-1) ? i+1 : i)];
			tb1[i*4+0] =    rom[i+t1start];
			tb1[i*4+1] = (val1*3 + val2  )/4;
			tb1[i*4+2] = (val1   + val2  )/2;
			tb1[i*4+3] = (val1   + val2*3)/4;
		}
		
	//Scan Table 2
	if (rep2 == 1)
		memcpy(tb2,&rom[t2start+0x2000],lent2); //A13 is toggled high on Table 2 reading (Implementation specific - ie, Mr. Game)
	else if(rep2 == 2)
		for(i=0; i<lent2; i++)
		{
			tb2[i*2+0] =       rom[i+t2start+0x2000];
			tb2[i*2+1] = ((int)rom[i+t2start+0x2000] + (int)rom[t2start+0x2000 + ((i < lent2-1) ? i+1 : i)])/2;
		}
	else //if(rep2 == 4)
		for(i=0; i<lent2; i++)
		{
			int val1 = (int)rom[i+t2start+0x2000];
			int val2 = (int)rom[t2start+0x2000 + ((i < lent2-1) ? i+1 : i)];
			tb2[i*4+0] =    rom[i+t2start+0x2000];
			tb2[i*4+1] = (val1*3 + val2  )/4;
			tb2[i*4+2] = (val1   + val2  )/2;
			tb2[i*4+3] = (val1   + val2*3)/4;
		}
#else
	//Scan Table 1
	for(i=0; i<lent1; i++)
		for(j=0; j<rep1; j++)
			tb1[j+(i*rep1)] = rom[i+t1start];
	//Scan Table 2
	for(i=0; i<lent2; i++)
		for(j=0; j<rep2; j++)
			tb2[j+(i*rep2)] = rom[i+t2start+0x2000];		//A13 is toggled high on Table 2 reading (Implementation specific - ie, Mr. Game)
#endif

	// Table1 is always larger, so use that as the size

	// How to make up difference (table 2 can be shorter than table 1)? -> was memset above already, so zero for now
	/*for(i=lent2*rep2; i<lent1*rep1; i++)
		tb2[i] = ?;*/

	// Now Mix based on Interpolation Bits
	for(i=0; i<lent1*rep1; i++)	{
		int l;
		//Apply volume - If envelope - inc/dec volume to calculate sample volume (only 8 most significant bits from the 10 bits, thus +/-4), otherwise, apply directly
#ifdef USE_VOL_ENVELOPE
		if(channel->regs.env_enable && ((i & (abs(channel->step_rate_volume_env)-1)) == abs(channel->step_rate_volume_env)/2)) { // check if we match the frequency that the value must be updated
			channel->current_volume += channel->step_rate_volume_env > 0 ? 4 : -4; // dependent on sign inc/dec by 4
			if(channel->step_rate_volume_env > 0)
			{
				if(channel->current_volume > channel->target_volume)
					channel->current_volume = channel->target_volume;
			}
			else if(channel->current_volume < channel->target_volume)
				channel->current_volume = channel->target_volume;
		}
#endif
		//write to output buffer
#ifdef DO_FULL_PRECISION_MIXING
		l = (int)tb1[i] * (intp + 1) + (int)tb2[i] * (15 - intp);
		channel->output[i] = (INT16)(0x6f * l * (channel->current_volume + 1) / (1024*16)); // Max Volume would be 256 for an INT16 value (so why was 0x6f chosen??)
#else
		l = ((int)tb1[i] * (intp + 1) / 16) + ((int)tb2[i] * (15 - intp) / 16); // formula seen in datasheet, but unclear what this means precision wise (i.e. is this only meant as real number pseudo code?)
		channel->output[i] = (INT16)(0x6f * l * (channel->current_volume + 1) / 1024); // Max Volume would be 256 for an INT16 value (so why was 0x6f chosen??) //!! do 0x6f scale AFTER division??
#endif
	}
}

/**********************************************************************************************

     m114s_update -- update the sound chip so that it is in sync with CPU execution

***********************************************************************************************/
//Seems this sometimes still produces some static, but I don't know why!
static void m114s_update(int num,
#if M114S_OUTPUT_CHANNELS == 1
	INT16 *buffer,
#else
	INT16 **buffer,
#endif
	int samples)
{
	struct M114SChip * const chip = &m114schip[num];

	while (samples > 0)
	{
#if M114S_OUTPUT_CHANNELS == 1
		INT32 accum = 0;
#else
		INT32 accum[M114S_OUTPUT_CHANNELS];
#endif
		int c;

#if M114S_OUTPUT_CHANNELS != 1
		/* clear accum */
		for(c = 0; c < M114S_OUTPUT_CHANNELS; c++)
			accum[c] = 0;
#endif

		/* loop over channels */
		for (c = 0; c < M114S_CHANNELS; c++)
		{
			struct M114SChannel * const channel = &chip->channels[c];
			/* Grab the next sample from the table data if the channel is active */
			if (channel->active)
			{
				//We use Table 1 to drive everything, as Table 2 is really for mixing into Table 1..
				INT32 sample = read_sample(channel, channel->table1.total_length); //!! INT16 if MR_GAME_VOLUME_HACK would be off

#ifdef MR_GAME_VOLUME_HACK
				sample = sample*chip->channel_volume[channel->regs.outputs] / 100; // boost percussion on Dakar, penalty some of the other instruments
#endif
				//Mix the output of this channel to the appropriate output channel
#if M114S_OUTPUT_CHANNELS == 1
				accum
#elif M114S_OUTPUT_CHANNELS == 4
				accum[channel->regs.outputs]
#else
				accum[c]
#endif
				+= sample;
			}
		}

		/* Update the buffer & Ensure we don't clip */
#if M114S_OUTPUT_CHANNELS == 1
		accum /= (INT32)4;
		*buffer++ = (accum < -32768) ? -32768 : ((accum > 32767) ? 32767 : accum);
#else
		for (c = 0; c < M114S_OUTPUT_CHANNELS; c++)
			*buffer[c]++ = (accum[c] < -32768) ? -32768 : ((accum[c] > 32767) ? 32767 : accum[c]);
#endif

		samples--;
	}
}

/**********************************************************************************************

     M114S_sh_start -- start emulation of the M114S

***********************************************************************************************/

INLINE void init_channel(struct M114SChannel * const channel)
{
	//set all internal registers to 0!
	channel->active = 0;
	channel->outpos = 0;
	channel->prev_volume = 0;
	memset(&channel->output,0,sizeof(channel->output));
	memset(&channel->regs,  0,sizeof(channel->regs));
	memset(&channel->table1,0,sizeof(channel->table1));
	memset(&channel->table2,0,sizeof(channel->table2));
}


INLINE void init_all_channels(struct M114SChip * const chip)
{
	int i;

	/* init the channels */
	for (i = 0; i < M114S_CHANNELS; i++)
		init_channel(&chip->channels[i]);

	//Chip init stuff
	memset(&chip->tempch_regs,0,sizeof(chip->tempch_regs));
	chip->channel = 0;
	chip->bytes_read = 0;
}


int M114S_sh_start(const struct MachineSound *msound)
{
	const struct M114Sinterface *intf = msound->sound_interface;
#if M114S_OUTPUT_CHANNELS == 1 
	char stream_name[40];
#else
	char stream_name[M114S_OUTPUT_CHANNELS][40];
	const char *stream_name_ptrs[M114S_OUTPUT_CHANNELS];
	int vol[M114S_OUTPUT_CHANNELS];
#endif
	int i,j;

	/* initialize the chips */
	memset(&m114schip, 0, sizeof(m114schip));
	for (i = 0; i < intf->num; i++)
	{
		/* Chip specific setup based on clock speed */
		switch(intf->baseclock[i]) {
			// M114A 4 Mhz
			case 4000000:
				m114schip[i].reset_cycles = 4000000 * 0.000128;	// Chip resets in 128us (microseconds)
				m114schip[i].is_M114A = 1;
				break;
			// M114AF 6 Mhz
			case 6000000:
			case 5994560: // from datasheet: 5.99456 MHz
				m114schip[i].reset_cycles = intf->baseclock[i] * 0.000085;	// Chip resets in 85us (microseconds)
				m114schip[i].is_M114A = 0;
				LOG(("M114S Chip #%d - 6Mhz chip clock not fully supported/tested at this time!\n", i));
				return 1;
			default:
				LOG(("M114S Chip #%d - Invalid Base Clock value specified! Only 4Mhz & 6Mhz values allowed!\n",i));
				return 1;
		}

		/* generate the name and create the stream */
#if M114S_OUTPUT_CHANNELS == 1
		sprintf(stream_name, "%s #%d", sound_name(msound), i);
		m114schip[i].stream = stream_init(stream_name, 100, SAMPLE_RATE, i, m114s_update);
#else
		for (j = 0; j<M114S_OUTPUT_CHANNELS; j++) {
			sprintf(stream_name[j], "%s #%d Ch%d", sound_name(msound), i, j);
			stream_name_ptrs[j] = stream_name[j];
			vol[j] = 100 / 4;
		}
		m114schip[i].stream = stream_init_multi(M114S_OUTPUT_CHANNELS, stream_name_ptrs, vol, SAMPLE_RATE, i, m114s_update);
#endif
		if (m114schip[i].stream == -1)
			return 1;

		/* initialize the region & interface info */
		m114schip[i].cpu_num = intf->cpunum[i];
		m114schip[i].region_base = (INT8 *)memory_region(intf->region[i]);
		m114schip[i].intf = (struct M114Sinterface *)intf;

		/* init the channels */
		init_all_channels(&m114schip[i]);
		for(j = 0; j < 4; j++)
			m114schip[i].channel_volume[j] = intf->mixing_level[i][j];
	}

	/* success */
	return 0;
}

/**********************************************************************************************

     M114S_sh_stop -- stop emulation of the M114S

***********************************************************************************************/

void M114S_sh_stop(void)
{
}

/**********************************************************************************************

     M114S_sh_reset -- reset emulation of the M114S

***********************************************************************************************/

void M114S_sh_reset(void)
{
	int i;
	for (i = 0; i < MAX_M114S; i++) {
		/* reset all channels */
		init_all_channels(&m114schip[i]);
	}
}

/**********************************************************************************************

     process_freq_codes -- There are up to 16 special values for frequency that signify a code

***********************************************************************************************/
static void process_freq_codes(struct M114SChip * const chip)
{
	//Grab pointer to channel being programmed
	struct M114SChannel * const channel = &chip->channels[chip->channel];
	switch(channel->regs.frequency)
	{
		//ROMID - ROM Identification  (Are you kidding me?)
		case 0xf8:
			LOG(("* * Channel: %02d: Frequency Code: %02x - ROMID * * \n",chip->channel,channel->regs.frequency));
			break;
		//SSG - Set Syncro Global
		case 0xf9:
			LOG(("* * Channel: %02d: Frequency Code: %02x - SSG * * \n",chip->channel,channel->regs.frequency));
			break;
		//RSS - Reverse Syncro Status
		case 0xfa:
			// check if used, as this will force frequency changes to wait until table ends
			LOG(("* * Channel: %02d: Frequency Code: %02x - RSS * * \n",chip->channel,channel->regs.frequency));
			break;
		//RSG - Reset Syncro Global
		case 0xfb:
			LOG(("* * Channel: %02d: Frequency Code: %02x - RSG * * \n",chip->channel,channel->regs.frequency));
			break;
		//PSF - Previously Selected Frequency
		case 0xfc:
			// seems to be unused by Mr.Game
			LOG(("* * Channel: %02d: Frequency Code: %02x - PSF * * \n",chip->channel,channel->regs.frequency));
			break;
		//FFT - Forced Table Termination
		case 0xff:
			//Stop whatever output from playing by simulating an end of table event!
			channel->outpos = 0; // but this will just put the cyclic counter back to the beginning!?!
			//LOG(("* * Channel: %02d: Frequency Code: %02x - FFT * * \n",chip->channel,channel->regs.frequency));
			break;
		default:
			LOG(("* * Channel: %02d: Frequency Code: %02x - UNKNOWN * * \n",chip->channel,channel->regs.frequency));
			break;
	}
}

/**********************************************************************************************

     process_channel_data -- complete programming for a channel now exists, process it!

***********************************************************************************************/

static void process_channel_data(struct M114SChip * const chip)
{
	//Grab pointer to channel being programmed
	struct M114SChannel * const channel = &chip->channels[chip->channel];

	/* Force stream to update */
	stream_update(chip->stream, 0);

	//Reset # of bytes for next group
	chip->bytes_read = 0;

	//Copy data to the appropriate channel registers from our temp channel registers
	memcpy(&channel->regs,&chip->tempch_regs,sizeof(chip->tempch_regs));

	//Look for the 16 special frequency codes
	if(channel->regs.frequency >= 0xf0) {
			process_freq_codes(chip);
			//FFT & PSF are the only codes that should continue to process channel data AFAIK
			if(channel->regs.frequency != 0xff && channel->regs.frequency != 0xfc)
				return;
	}

	//If Attenuation set to 0x3F - The channel becomes inactive
	if(channel->regs.atten == 0x3f) {
		channel->active = 0;
		return;
	}
	else
	//Process this channel
	{
		//Calculate # of repetitions for Table 1 & Table 2
		const int rep1 = mode_to_rep[channel->regs.read_meth][0];
		const int rep2 = mode_to_rep[channel->regs.read_meth][1];
		//Calculate Table Length for Table 1 & Table 2
		const int lent1 = mode_to_len_t1[channel->regs.read_meth][channel->regs.table_len];
		const int lent2 = mode_to_len_t2[channel->regs.read_meth][channel->regs.table_len];
		//Start & Stop Address - Note the special case for table length of 16 - Bit 5 always 1 in this case // but also for special cases 8 and 4 for table 2?!?
		//Calculate Table 1 Start & End Address in ROM
		const int t1start = ((channel->regs.table1_addr<<5) & (~(lent1-1)&0x1fff)) | (lent1 == 16 ? 0x10 : 0); //T1 Addr is only upper 8 bits, but masked by length
		//const int t1end = t1start | (lent1-1);
		//Calculate Table 2 Start & End Address in ROM
		const int t2start = ((channel->regs.table2_addr<<5) & (~(lent2-1)&0x1fff)) | (lent2 <= 16 ? 0x10 : 0); //T2 Addr is only upper 8 bits, but masked by length
		//const int t2end = t2start | (lent2-1);

		//Calculate initial frequency of both tables
		double freq = chip->is_M114A ? freqtable4Mhz[channel->regs.frequency] : freqtable6Mhz[channel->regs.frequency];

		//Calculate new volume
		channel->target_volume = channel->regs.atten < 32 ? v_linear[channel->regs.atten] : 0; // channel must be kept active, but is it really 0? Or is the last value used (v_linear[31])? Or something inbetween??

		//Adjust frequency if octave divisor set
		if (channel->regs.oct_divisor)
			freq /= 2.; // maybe new frequency must be clamped to freqtable[0]??

		//Channel is now active!
		channel->active = 1;

		// Setup Sample Rate/Step size increase
		channel->incr = (UINT32)(freq * ((double)(16u << FRAC_BITS) / SAMPLE_RATE));

		//Assign start & stop address offsets to ROM
		channel->table1.start_address = t1start;
		channel->table2.start_address = t2start;
		//channel->table1.stop_address = t1end;
		//channel->table2.stop_address = t2end;

		//Assign # of times to re-read & Length
		channel->table1.reread = rep1;
		channel->table2.reread = rep2;
		channel->table1.length = lent1;
		channel->table2.length = lent2;
		channel->table1.total_length = lent1*rep1;
		channel->table2.total_length = lent2*rep2;

		//Calculate Sample Volume
		//- If Envelope should be used, Take the difference in new volume & current volume, and break it into the # of samples in the table
		//- If No Envelope should be used, Volume is simply based on the Volume Table
#ifdef USE_VOL_ENVELOPE
		if(channel->regs.env_enable) {
			const int diff = channel->target_volume - channel->prev_volume;
			channel->step_rate_volume_env = abs(diff) > 128 ? 1 : abs(diff) > 64 ? 2 : abs(diff) > 32 ? 4 : 8;
			if(channel->target_volume < channel->prev_volume)
				channel->step_rate_volume_env = -channel->step_rate_volume_env;
			channel->current_volume = channel->prev_volume;
		}
		else
#endif
		{
			channel->current_volume = channel->target_volume;
		}
		//Update Last Volume
		channel->prev_volume = channel->target_volume;

		//Temp hack to ensure we only generate the ouput data 1x - this is WRONG and needs to be addressed eventually!
		//if(channel->output[0] == 0)
			read_table(chip,channel);

#if 0
//if(chip->channel == 2) {
if(channel->regs.outputs == 2) {
	if(channel->regs.frequency == 0x70)
	{
		LOG(("orig: = %0f, freq = %0f, incr = %0d \n",freqtable4Mhz[channel->regs.frequency],freq,channel->incr));
	}
	//LOG(("EOT=%d\n",channel->end_of_table));
	LOG(("C:%02d V:%02d FQ:%03x TS1:%02x TS2:%02x T1L:%04d T1R:%01d T2L:%04d T2R:%01d OD=%01d I:%02d E:%01d\n",
		chip->channel,
		channel->regs.atten,
		channel->regs.frequency,
		t1start,t2start,
		lent1,rep1,
		lent2,rep2,
		channel->regs.oct_divisor,
		channel->regs.interp,
		channel->regs.env_enable
		));
}
#endif

	}
}

/**********************************************************************************************

     m114s_data_write -- handle a write to the data bus of the M114S

     The chip has a data bus width of 6 bits, and must be fed 8 consecutive bytes
	 - thus 48 bits of programming! All data must be fed in order, so we make a few assumptions.

***********************************************************************************************/
static void m114s_data_write(struct M114SChip * const chip, data8_t data)
{
	/* Check if the chip needs to 'auto-reset' - this occurs if during the programming sequence (ie before all 8 bytes read)
	   a certain amount of time elapses without receiving another byte of programming...
	   128us in the 4Mhz chip, 85us in the 6Mhz chip.
	*/
	static UINT64 last_totcyc = 0;
	UINT64 curr_totcyc = cpu_gettotalcycles64(chip->cpu_num);
	double diff = (double)llabs((INT64)(curr_totcyc-last_totcyc));
	last_totcyc = curr_totcyc;
	if(chip->bytes_read && diff > chip->reset_cycles) {
		LOG(("M114S: Auto Reset - bytes read=%0d - data=%0x, elapsed cycles = %f\n",chip->bytes_read,data&0x3f,diff));
		M114S_sh_reset();
	}

	data &= 0x3f;						//Strip off bits 7-8 (only 6 bits for the data bus to the chip)
	chip->bytes_read++;
	switch(chip->bytes_read)
	{
	/*  BYTE #1 -
	    Bits 0-5: Attenuation Value (0-63) - 0 = No Attenuation, 3E = Max, 3F = Silence active channel */
		case 1:
			chip->tempch_regs.atten = data;
			break;

	/*  BYTE #2 -
		Bits 0-1: Table 2 Address (Bits 6-7)
		Bits 2-3: Table 1 Address (Bits 6-7)
		Bits 4-5: Output Pin Selection (0-3)  */
		case 2:
			chip->tempch_regs.table2_addr = (data & 0x03)<<6;
			chip->tempch_regs.table1_addr = (data & 0x0c)<<4;
			chip->tempch_regs.outputs = (data & 0x30)>>4;
			break;

	/*  BYTE #3 -
		Bits 0-5: Table 2 Address (Bits 0-5) */
		case 3:
			chip->tempch_regs.table2_addr |= data;
			break;

	/*	BYTE #4 -
		Bits 0-5: Table 1 Address (Bits 0-5) */
		case 4:
			chip->tempch_regs.table1_addr |= data;
			break;

	/*  BYTE #5 -
		Bits 0-2: Reading Method
		Bits 3-5: Table Length */
		case 5:
			chip->tempch_regs.read_meth = data & 0x07;
			chip->tempch_regs.table_len = (data & 0x38)>>3;
			break;

	/*	BYTE #6 -
		Bits 0  : Octave Divisor
		Bits 1  : Envelope Enable/Disable
		Bits 2-5: Interpolation Value (0-15) */
		case 6:
			chip->tempch_regs.oct_divisor = (data & 0x01);
			chip->tempch_regs.env_enable = (data & 0x02);
			chip->tempch_regs.interp = (data & 0x3c)>>2;
			break;

	/*	BYTE #7 -
		Bits 0-1: Frequency (Bits 0-1)
		Bits 2-5: Channel */
		case 7:
			chip->tempch_regs.frequency = (data & 0x03);
			chip->channel = (data & 0x3c)>>2;
			break;

	/*	BYTE #8 -
		Bits 0-5: Frequency (Bits 2-7) */
		case 8:
			chip->tempch_regs.frequency |= (data<<2);
			/* Process the channel data */
			process_channel_data(chip);
			break;

		default:
			LOG(("M114S.C - logic error - too many bytes processed: %x\n",chip->bytes_read));
			break;
	}
}



/**********************************************************************************************

     M114S_data_0_w -- handle a write to the current register

***********************************************************************************************/
WRITE_HANDLER( M114S_data_w )
{
	m114s_data_write(&m114schip[offset], data);
}
