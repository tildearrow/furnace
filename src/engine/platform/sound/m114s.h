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
 *
 *   Code based largely on Aaron Gile's BSMT2000 driver.
 **********************************************************************************************/

#ifndef M114S_H
#define M114S_H
#if !defined(__GNUC__) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || (__GNUC__ >= 4)	// GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#define MAX_M114S 1

struct M114Sinterface
{
        int num;                             /* total number of chips */
        int baseclock[MAX_M114S];            /* input clock - Allowed values are 4Mhz & 6Mhz only! */
        int region[MAX_M114S];               /* memory region where the sample ROM lives */
        int mixing_level[MAX_M114S][4];      /* master volume, one for each output sample */
        int cpunum[MAX_M114S];               /* # of the cpu controlling the M114S */
};

int M114S_sh_start(const struct MachineSound *msound);
void M114S_sh_stop(void);
void M114S_sh_reset(void);

WRITE_HANDLER( M114S_data_w );

#endif
