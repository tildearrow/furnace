/*
 * mzpokeysnd.c - POKEY sound chip emulation, v1.6
 *
 * Copyright (C) 2002 Michael Borisov
 * Copyright (C) 2002-2014 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

// additional modifications for Furnace by tildearrow

#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <math.h>

#include "pokey.h"
#include "mzpokeysnd.h"

#define CONSOLE_VOL 8
#ifdef NONLINEAR_MIXING
static const double pokeymix[61+CONSOLE_VOL] = { /* Nonlinear POKEY mixing array */
0.000000, 5.169146, 10.157015, 15.166247,
20.073793, 24.927443, 29.728237, 34.495266,
39.181262, 43.839780, 48.429508, 52.932530,
57.327319, 61.586304, 65.673220, 69.547672,
73.207846, 76.594474, 79.739231, 82.631161,
85.300361, 87.750638, 90.020656, 92.108334,
94.051256, 95.848478, 97.521287, 99.080719,
100.540674, 101.902750, 103.185339, 104.375596,
105.491149, 106.523735, 107.473511, 108.361458,
109.185669, 109.962251, 110.685574, 111.367150,
112.008476, 112.612760, 113.185603, 113.722735,
114.227904, 114.712206, 115.171007, 115.605730,
116.024396, 116.416097, 116.803169, 117.155108,
117.532921, 117.835494, 118.196180, 118.502785,
118.825177, 119.138170, 119.421378, 119.734493,
/* need to add CONSOLE_VOL extra copies of the last val */
120.000000,120.0,120.0,120.0,120.0,120.0,120.0,120.0,120.0};
#endif

/* Poly tables */
static int poly4tbl[15];
static int poly5tbl[31];
static unsigned char poly17tbl[131071];
static int poly9tbl[511];

/* Forward declarations for ResetPokeyState */

int readout0_normal(PokeyState* ps);
void event0_pure(PokeyState* ps, int p5v, int p4v, int p917v);

int readout1_normal(PokeyState* ps);
void event1_pure(PokeyState* ps, int p5v, int p4v, int p917v);

int readout2_normal(PokeyState* ps);
void event2_pure(PokeyState* ps, int p5v, int p4v, int p917v);

int readout3_normal(PokeyState* ps);
void event3_pure(PokeyState* ps, int p5v, int p4v, int p917v);

void ResetPokeyState(PokeyState* ps)
{
    /* Poly positions */
    ps->poly4pos = 0;
    ps->poly5pos = 0;
    ps->poly9pos = 0;
    ps->poly17pos = 0;

    /* Global Pokey controls */
    ps->mdivk = 28;

    ps->selpoly9 = 0;
    ps->c0_hf = 0;
    ps->c1_f0 = 0;
    ps->c2_hf = 0;
    ps->c3_f2 = 0;

    /* SKCTL for two-tone mode */
    ps->skctl = 0;

    ps->outvol_all = 0;
    ps->forcero = 0;

    /* Channel 0 state */
    ps->readout_0 = readout0_normal;
    ps->event_0 = event0_pure;

    ps->c0divpos = 1000;
    ps->c0divstart = 1000;
    ps->c0divstart_p = 1000;
    ps->c0diva = 255;

    ps->c0t1 = 0;
    ps->c0t2 = 0;
    ps->c0t3 = 0;

    ps->c0sw1 = 0;
    ps->c0sw2 = 0;
    ps->c0sw3 = 0;
    ps->c0sw4 = 0;
    ps->c0vo = 1;

#ifndef NONLINEAR_MIXING
    ps->c0stop = 1;
#endif

    ps->vol0 = 0;

    ps->outvol_0 = 0;


    /* Channel 1 state */
    ps->readout_1 = readout1_normal;
    ps->event_1 = event1_pure;

    ps->c1divpos = 1000;
    ps->c1divstart = 1000;
    ps->c1diva = 255;

    ps->c1t1 = 0;
    ps->c1t2 = 0;
    ps->c1t3 = 0;

    ps->c1sw1 = 0;
    ps->c1sw2 = 0;
    ps->c1sw3 = 0;
    ps->c1sw4 = 0;
    ps->c1vo = 1;

#ifndef NONLINEAR_MIXING
    ps->c1stop = 1;
#endif

    ps->vol1 = 0;

    ps->outvol_1 = 0;

    /* Channel 2 state */
    ps->readout_2 = readout2_normal;
    ps->event_2 = event2_pure;

    ps->c2divpos = 1000;
    ps->c2divstart = 1000;
    ps->c2divstart_p = 1000;
    ps->c2diva = 255;

    ps->c2t1 = 0;
    ps->c2t2 = 0;

    ps->c2sw1 = 0;
    ps->c2sw2 = 0;
    ps->c2sw3 = 0;

    ps->c2vo = 0;

#ifndef NONLINEAR_MIXING
    ps->c2stop = 1;
#endif

    ps->vol2 = 0;

    ps->outvol_2 = 0;

    /* Channel 3 state */
    ps->readout_3 = readout3_normal;
    ps->event_3 = event3_pure;

    ps->c3divpos = 1000;
    ps->c3divstart = 1000;
    ps->c3diva = 255;

    ps->c3t1 = 0;
    ps->c3t2 = 0;

    ps->c3sw1 = 0;
    ps->c3sw2 = 0;
    ps->c3sw3 = 0;

    ps->c3vo = 0;

#ifndef NONLINEAR_MIXING
    ps->c3stop = 1;
#endif

    ps->vol3 = 0;

    ps->outvol_3 = 0;
}

static void build_poly4(void)
{
    unsigned char c;
    unsigned char i;
    unsigned char poly4=1;

    for(i=0; i<15; i++)
    {
        poly4tbl[i] = ~poly4;
        c = ((poly4>>2)&1) ^ ((poly4>>3)&1);
        poly4 = ((poly4<<1)&15) + c;
    }
}

static void build_poly5(void)
{
  unsigned char c;
  unsigned char i;
  unsigned char poly5 = 1;

  for(i = 0; i < 31; i++) {
    poly5tbl[i] = ~poly5; /* Inversion! Attention! */
    c = ((poly5 >> 2) ^ (poly5 >> 4)) & 1;
    poly5 = ((poly5 << 1) & 31) + c;
  }
}

static void build_poly17(void)
{
  unsigned int c;
  unsigned int i;
  unsigned int poly17 = 1;

  for(i = 0; i < 131071; i++) {
    poly17tbl[i] = (unsigned char) poly17;
    c = ((poly17 >> 11) ^ (poly17 >> 16)) & 1;
    poly17 = ((poly17 << 1) & 131071) + c;
  }
}

static void build_poly9(void)
{
  unsigned int c;
  unsigned int i;
  unsigned int poly9 = 1;

  for(i = 0; i < 511; i++) {
    poly9tbl[i] = (unsigned char) poly9;
    c = ((poly9 >> 3) ^ (poly9 >> 8)) & 1;
    poly9 = ((poly9 << 1) & 511) + c;
  }
}

void advance_polies(PokeyState* ps, int tacts)
{
    ps->poly4pos = (tacts + ps->poly4pos) % 15;
    ps->poly5pos = (tacts + ps->poly5pos) % 31;
    ps->poly17pos = (tacts + ps->poly17pos) % 131071;
    ps->poly9pos = (tacts + ps->poly9pos) % 511;
}

/***********************************

   READ OUTPUT 0

  ************************************/

int readout0_vo(PokeyState* ps)
{
    return ps->vol0;
}

int readout0_hipass(PokeyState* ps)
{
    if(ps->c0t2 ^ ps->c0t3)
        return ps->vol0;
    else return 0;
}

int readout0_normal(PokeyState* ps)
{
    if(ps->c0t2)
        return ps->vol0;
    else return 0;
}

/***********************************

   READ OUTPUT 1

  ************************************/

int readout1_vo(PokeyState* ps)
{
    return ps->vol1;
}

int readout1_hipass(PokeyState* ps)
{
    if(ps->c1t2 ^ ps->c1t3)
        return ps->vol1;
    else return 0;
}

int readout1_normal(PokeyState* ps)
{
    if(ps->c1t2)
        return ps->vol1;
    else return 0;
}

/***********************************

   READ OUTPUT 2

  ************************************/

int readout2_vo(PokeyState* ps)
{
    return ps->vol2;
}

int readout2_normal(PokeyState* ps)
{
    if(ps->c2t2)
        return ps->vol2;
    else return 0;
}

/***********************************

   READ OUTPUT 3

  ************************************/

int readout3_vo(PokeyState* ps)
{
    return ps->vol3;
}

int readout3_normal(PokeyState* ps)
{
    if(ps->c3t2)
        return ps->vol3;
    else return 0;
}


/***********************************

   EVENT CHANNEL 0

  ************************************/

void event0_pure(PokeyState* ps, int p5v, int p4v, int p917v)
{
    ps->c0t2 = !ps->c0t2;
    ps->c0t1 = p5v;
}

void event0_p5(PokeyState* ps, int p5v, int p4v, int p917v)
{
    if(ps->c0t1)
        ps->c0t2 = !ps->c0t2;
    ps->c0t1 = p5v;
}

void event0_p4(PokeyState* ps, int p5v, int p4v, int p917v)
{
    ps->c0t2 = p4v;
    ps->c0t1 = p5v;
}

void event0_p917(PokeyState* ps, int p5v, int p4v, int p917v)
{
    ps->c0t2 = p917v;
    ps->c0t1 = p5v;
}

void event0_p4_p5(PokeyState* ps, int p5v, int p4v, int p917v)
{
    if(ps->c0t1)
        ps->c0t2 = p4v;
    ps->c0t1 = p5v;
}

void event0_p917_p5(PokeyState* ps, int p5v, int p4v, int p917v)
{
    if(ps->c0t1)
        ps->c0t2 = p917v;
    ps->c0t1 = p5v;
}

/***********************************

   EVENT CHANNEL 1

  ************************************/

void event1_pure(PokeyState* ps, int p5v, int p4v, int p917v)
{
    ps->c1t2 = !ps->c1t2;
    ps->c1t1 = p5v;
}

void event1_p5(PokeyState* ps, int p5v, int p4v, int p917v)
{
    if(ps->c1t1)
        ps->c1t2 = !ps->c1t2;
    ps->c1t1 = p5v;
}

void event1_p4(PokeyState* ps, int p5v, int p4v, int p917v)
{
    ps->c1t2 = p4v;
    ps->c1t1 = p5v;
}

void event1_p917(PokeyState* ps, int p5v, int p4v, int p917v)
{
    ps->c1t2 = p917v;
    ps->c1t1 = p5v;
}

void event1_p4_p5(PokeyState* ps, int p5v, int p4v, int p917v)
{
    if(ps->c1t1)
        ps->c1t2 = p4v;
    ps->c1t1 = p5v;
}

void event1_p917_p5(PokeyState* ps, int p5v, int p4v, int p917v)
{
    if(ps->c1t1)
        ps->c1t2 = p917v;
    ps->c1t1 = p5v;
}

/***********************************

   EVENT CHANNEL 2

  ************************************/

void event2_pure(PokeyState* ps, int p5v, int p4v, int p917v)
{
    ps->c2t2 = !ps->c2t2;
    ps->c2t1 = p5v;
    /* high-pass clock for channel 0 */
    ps->c0t3 = ps->c0t2;
}

void event2_p5(PokeyState* ps, int p5v, int p4v, int p917v)
{
    if(ps->c2t1)
        ps->c2t2 = !ps->c2t2;
    ps->c2t1 = p5v;
    /* high-pass clock for channel 0 */
    ps->c0t3 = ps->c0t2;
}

void event2_p4(PokeyState* ps, int p5v, int p4v, int p917v)
{
    ps->c2t2 = p4v;
    ps->c2t1 = p5v;
    /* high-pass clock for channel 0 */
    ps->c0t3 = ps->c0t2;
}

void event2_p917(PokeyState* ps, int p5v, int p4v, int p917v)
{
    ps->c2t2 = p917v;
    ps->c2t1 = p5v;
    /* high-pass clock for channel 0 */
    ps->c0t3 = ps->c0t2;
}

void event2_p4_p5(PokeyState* ps, int p5v, int p4v, int p917v)
{
    if(ps->c2t1)
        ps->c2t2 = p4v;
    ps->c2t1 = p5v;
    /* high-pass clock for channel 0 */
    ps->c0t3 = ps->c0t2;
}

void event2_p917_p5(PokeyState* ps, int p5v, int p4v, int p917v)
{
    if(ps->c2t1)
        ps->c2t2 = p917v;
    ps->c2t1 = p5v;
    /* high-pass clock for channel 0 */
    ps->c0t3 = ps->c0t2;
}

/***********************************

   EVENT CHANNEL 3

  ************************************/

void event3_pure(PokeyState* ps, int p5v, int p4v, int p917v)
{
    ps->c3t2 = !ps->c3t2;
    ps->c3t1 = p5v;
    /* high-pass clock for channel 1 */
    ps->c1t3 = ps->c1t2;
}

void event3_p5(PokeyState* ps, int p5v, int p4v, int p917v)
{
    if(ps->c3t1)
        ps->c3t2 = !ps->c3t2;
    ps->c3t1 = p5v;
    /* high-pass clock for channel 1 */
    ps->c1t3 = ps->c1t2;
}

void event3_p4(PokeyState* ps, int p5v, int p4v, int p917v)
{
    ps->c3t2 = p4v;
    ps->c3t1 = p5v;
    /* high-pass clock for channel 1 */
    ps->c1t3 = ps->c1t2;
}

void event3_p917(PokeyState* ps, int p5v, int p4v, int p917v)
{
    ps->c3t2 = p917v;
    ps->c3t1 = p5v;
    /* high-pass clock for channel 1 */
    ps->c1t3 = ps->c1t2;
}

void event3_p4_p5(PokeyState* ps, int p5v, int p4v, int p917v)
{
    if(ps->c3t1)
        ps->c3t2 = p4v;
    ps->c3t1 = p5v;
    /* high-pass clock for channel 1 */
    ps->c1t3 = ps->c1t2;
}

void event3_p917_p5(PokeyState* ps, int p5v, int p4v, int p917v)
{
    if(ps->c3t1)
        ps->c3t2 = p917v;
    ps->c3t1 = p5v;
    /* high-pass clock for channel 1 */
    ps->c1t3 = ps->c1t2;
}

void advance_ticks(PokeyState* ps, int ticks)
{
    int ta,tbe, tbe0, tbe1, tbe2, tbe3;
    int p5v,p4v,p917v;

    qev_t outvol_new;
    int need0=0;
    int need1=0;
    int need2=0;
    int need3=0;

    int need=0;

    if (ticks <= 0) return;
    if(ps->forcero)
    {
        ps->forcero = 0;
#ifdef NONLINEAR_MIXING
        outvol_new = pokeymix[ps->outvol_0 + ps->outvol_1 + ps->outvol_2 + ps->outvol_3];
#else
        outvol_new = ps->outvol_0 + ps->outvol_1 + ps->outvol_2 + ps->outvol_3;
#endif /* NONLINEAR_MIXING */
        if(outvol_new != ps->outvol_all)
        {
            ps->outvol_all = outvol_new;
        }
    }

    while(ticks>0)
    {
        tbe0 = ps->c0divpos;
        tbe1 = ps->c1divpos;
        tbe2 = ps->c2divpos;
        tbe3 = ps->c3divpos;

        tbe = ticks+1;

#ifdef NONLINEAR_MIXING
        if(tbe0 < tbe)
            tbe = tbe0;
        if(tbe1 < tbe)
            tbe = tbe1;
        if(tbe2 < tbe)
            tbe = tbe2;
        if(tbe3 < tbe)
            tbe = tbe3;
#else
        if(!ps->c0stop && tbe0 < tbe)
            tbe = tbe0;
        if(!ps->c1stop && tbe1 < tbe)
            tbe = tbe1;
        if(!ps->c2stop && tbe2 < tbe)
            tbe = tbe2;
        if(!ps->c3stop && tbe3 < tbe)
            tbe = tbe3;
#endif

        if(tbe>ticks)
            ta = ticks;
        else
        {
            ta = tbe;
            need = 1;
        }

        ticks -= ta;

#ifdef NONLINEAR_MIXING
        ps->c0divpos -= ta;
        ps->c1divpos -= ta;
        ps->c2divpos -= ta;
        ps->c3divpos -= ta;
#else
        if(!ps->c0stop) ps->c0divpos -= ta;
        if(!ps->c1stop) ps->c1divpos -= ta;
        if(!ps->c2stop) ps->c2divpos -= ta;
        if(!ps->c3stop) ps->c3divpos -= ta;
#endif

        advance_polies(ps,ta);

        if(need)
        {
            p5v = poly5tbl[ps->poly5pos] & 1;
            p4v = poly4tbl[ps->poly4pos] & 1;
            if(ps->selpoly9)
                p917v = poly9tbl[ps->poly9pos] & 1;
            else
                p917v = poly17tbl[ps->poly17pos] & 1;

#ifdef NONLINEAR_MIXING
            if(ta == tbe0)
#else
            if(!ps->c0stop && ta == tbe0)
#endif
            {
                ps->event_0(ps,p5v,p4v,p917v);
                ps->c0divpos = ps->c0divstart;
                need0 = 1;
            }
#ifdef NONLINEAR_MIXING
            if(ta == tbe1)
#else
            if(!ps->c1stop && ta == tbe1)
#endif
            {
                ps->event_1(ps,p5v,p4v,p917v);
                ps->c1divpos = ps->c1divstart;
                if(ps->c1_f0)
                    ps->c0divpos = ps->c0divstart_p;
                need1 = 1;
                /*two-tone filter*/
                /*use if send break is on and two-tone mode is on*/
                /*reset channel 1 if channel 2 changed*/
                if((ps->skctl & 0x88) == 0x88) {
                    ps->c0divpos = ps->c0divstart;
                    /* it doesn't change the output state */
                    /*need0 = 1;*/
                }
            }
#ifdef NONLINEAR_MIXING
            if(ta == tbe2)
#else
            if(!ps->c2stop && ta == tbe2)
#endif
            {
                ps->event_2(ps,p5v,p4v,p917v);
                ps->c2divpos = ps->c2divstart;
                need2 = 1;
                if(ps->c0sw4)
                    need0 = 1;
            }
#ifdef NONLINEAR_MIXING
            if(ta == tbe3)
#else
            if(!ps->c3stop && ta == tbe3)
#endif
            {
                ps->event_3(ps,p5v,p4v,p917v);
                ps->c3divpos = ps->c3divstart;
                if(ps->c3_f2)
                    ps->c2divpos = ps->c2divstart_p;
                need3 = 1;
                if(ps->c1sw4)
                    need1 = 1;
            }

            if(need0)
            {
#ifdef NONLINEAR_MIXING
                ps->outvol_0 = ps->readout_0(ps);
#else
                ps->outvol_0 = 2*ps->readout_0(ps);
#endif
            }
            if(need1)
            {
#ifdef NONLINEAR_MIXING
                ps->outvol_1 = ps->readout_1(ps);
#else
                ps->outvol_1 = 2*ps->readout_1(ps);
#endif
            }
            if(need2)
            {
#ifdef NONLINEAR_MIXING
                ps->outvol_2 = ps->readout_2(ps);
#else
                ps->outvol_2 = 2*ps->readout_2(ps);
#endif
            }
            if(need3)
            {
#ifdef NONLINEAR_MIXING
                ps->outvol_3 = ps->readout_3(ps);
#else
                ps->outvol_3 = 2*ps->readout_3(ps);
#endif
            }

#ifdef NONLINEAR_MIXING
            outvol_new = pokeymix[ps->outvol_0 + ps->outvol_1 + ps->outvol_2 + ps->outvol_3];
#else
            outvol_new = ps->outvol_0 + ps->outvol_1 + ps->outvol_2 + ps->outvol_3;
#endif /* NONLINEAR_MIXING */
            if(outvol_new != ps->outvol_all)
            {
                ps->outvol_all = outvol_new;
            }
        }
    }
}

double generate_sample(PokeyState* ps)
{
    advance_ticks(ps, 1);
    return ps->outvol_all;
}

/*****************************************************************************/
/* Module:  MZPOKEYSND_Init()                                                */
/* Purpose: to handle the power-up initialization functions                  */
/*          these functions should only be executed on a cold-restart        */
/*                                                                           */
/* Authors: Michael Borisov, Krzystof Nikiel                                 */
/*                                                                           */
/* Inputs:  freq17 - the value for the '1.79MHz' Pokey audio clock           */
/*          playback_freq - the playback frequency in samples per second     */
/*                                                                           */
/* Outputs: Adjusts local globals - no return value                          */
/*                                                                           */
/*****************************************************************************/

int MZPOKEYSND_Init(PokeyState* ps)
{
    build_poly4();
    build_poly5();
    build_poly9();
    build_poly17();
    ResetPokeyState(ps);
  return 0; /* OK */
}


void Update_readout_0(PokeyState* ps)
{
    if(ps->c0vo)
        ps->readout_0 = readout0_vo;
    else if(ps->c0sw4)
        ps->readout_0 = readout0_hipass;
    else
        ps->readout_0 = readout0_normal;
}

void Update_readout_1(PokeyState* ps)
{
    if(ps->c1vo)
        ps->readout_1 = readout1_vo;
    else if(ps->c1sw4)
        ps->readout_1 = readout1_hipass;
    else
        ps->readout_1 = readout1_normal;
}

void Update_readout_2(PokeyState* ps)
{
    if(ps->c2vo)
        ps->readout_2 = readout2_vo;
    else
        ps->readout_2 = readout2_normal;
}

void Update_readout_3(PokeyState* ps)
{
    if(ps->c3vo)
        ps->readout_3 = readout3_vo;
    else
        ps->readout_3 = readout3_normal;
}

void Update_event0(PokeyState* ps)
{
    if(ps->c0sw3)
    {
        if(ps->c0sw2)
            ps->event_0 = event0_pure;
        else
        {
            if(ps->c0sw1)
                ps->event_0 = event0_p4;
            else
                ps->event_0 = event0_p917;
        }
    }
    else
    {
        if(ps->c0sw2)
            ps->event_0 = event0_p5;
        else
        {
            if(ps->c0sw1)
                ps->event_0 = event0_p4_p5;
            else
                ps->event_0 = event0_p917_p5;
        }
    }
}

void Update_event1(PokeyState* ps)
{
    if(ps->c1sw3)
    {
        if(ps->c1sw2)
            ps->event_1 = event1_pure;
        else
        {
            if(ps->c1sw1)
                ps->event_1 = event1_p4;
            else
                ps->event_1 = event1_p917;
        }
    }
    else
    {
        if(ps->c1sw2)
            ps->event_1 = event1_p5;
        else
        {
            if(ps->c1sw1)
                ps->event_1 = event1_p4_p5;
            else
                ps->event_1 = event1_p917_p5;
        }
    }
}

void Update_event2(PokeyState* ps)
{
    if(ps->c2sw3)
    {
        if(ps->c2sw2)
            ps->event_2 = event2_pure;
        else
        {
            if(ps->c2sw1)
                ps->event_2 = event2_p4;
            else
                ps->event_2 = event2_p917;
        }
    }
    else
    {
        if(ps->c2sw2)
            ps->event_2 = event2_p5;
        else
        {
            if(ps->c2sw1)
                ps->event_2 = event2_p4_p5;
            else
                ps->event_2 = event2_p917_p5;
        }
    }
}

void Update_event3(PokeyState* ps)
{
    if(ps->c3sw3)
    {
        if(ps->c3sw2)
            ps->event_3 = event3_pure;
        else
        {
            if(ps->c3sw1)
                ps->event_3 = event3_p4;
            else
                ps->event_3 = event3_p917;
        }
    }
    else
    {
        if(ps->c3sw2)
            ps->event_3 = event3_p5;
        else
        {
            if(ps->c3sw1)
                ps->event_3 = event3_p4_p5;
            else
                ps->event_3 = event3_p917_p5;
        }
    }
}

void Update_c0divstart(PokeyState* ps)
{
    if(ps->c1_f0)
    {
        if(ps->c0_hf)
        {
            ps->c0divstart = 256;
            ps->c0divstart_p = ps->c0diva + 7;
        }
        else
        {
            ps->c0divstart = 256 * ps->mdivk;
            ps->c0divstart_p = (ps->c0diva+1)*ps->mdivk;
        }
    }
    else
    {
        if(ps->c0_hf)
            ps->c0divstart = ps->c0diva + 4;
        else
            ps->c0divstart = (ps->c0diva+1) * ps->mdivk;
    }
}

void Update_c1divstart(PokeyState* ps)
{
    if(ps->c1_f0)
    {
        if(ps->c0_hf)
            ps->c1divstart = ps->c0diva + 256*ps->c1diva + 7;
        else
            ps->c1divstart = (ps->c0diva + 256*ps->c1diva + 1) * ps->mdivk;
    }
    else
        ps->c1divstart = (ps->c1diva + 1) * ps->mdivk;
}

void Update_c2divstart(PokeyState* ps)
{
    if(ps->c3_f2)
    {
        if(ps->c2_hf)
        {
            ps->c2divstart = 256;
            ps->c2divstart_p = ps->c2diva + 7;
        }
        else
        {
            ps->c2divstart = 256 * ps->mdivk;
            ps->c2divstart_p = (ps->c2diva+1)*ps->mdivk;
        }
    }
    else
    {
        if(ps->c2_hf)
            ps->c2divstart = ps->c2diva + 4;
        else
            ps->c2divstart = (ps->c2diva+1) * ps->mdivk;
    }
}

void Update_c3divstart(PokeyState* ps)
{
    if(ps->c3_f2)
    {
        if(ps->c2_hf)
            ps->c3divstart = ps->c2diva + 256*ps->c3diva + 7;
        else
            ps->c3divstart = (ps->c2diva + 256*ps->c3diva + 1) * ps->mdivk;
    }
    else
        ps->c3divstart = (ps->c3diva + 1) * ps->mdivk;
}

void Update_audctl(PokeyState* ps, unsigned char val)
{
    int nc0_hf,nc2_hf,nc1_f0,nc3_f2,nc0sw4,nc1sw4,new_divk;
    int recalc0=0;
    int recalc1=0;
    int recalc2=0;
    int recalc3=0;

    unsigned int cnt0 = 0;
    unsigned int cnt1 = 0;
    unsigned int cnt2 = 0;
    unsigned int cnt3 = 0;

    nc0_hf = (val & 0x40) != 0;
    nc2_hf = (val & 0x20) != 0;
    nc1_f0 = (val & 0x10) != 0;
    nc3_f2 = (val & 0x08) != 0;
    nc0sw4 = (val & 0x04) != 0;
    nc1sw4 = (val & 0x02) != 0;
    if(val & 0x01)
        new_divk = 114;
    else
        new_divk = 28;

    if(new_divk != ps->mdivk)
    {
        recalc0 = recalc1 = recalc2 = recalc3 = 1;
    }
    if(nc1_f0 != ps->c1_f0)
    {
        recalc0 = recalc1 = 1;
    }
    if(nc3_f2 != ps->c3_f2)
    {
        recalc2 = recalc3 = 1;
    }
    if(nc0_hf != ps->c0_hf)
    {
        recalc0 = 1;
        if(nc1_f0)
            recalc1 = 1;
    }
    if(nc2_hf != ps->c2_hf)
    {
        recalc2 = 1;
        if(nc3_f2)
            recalc3 = 1;
    }

    if(recalc0)
    {
        if(ps->c0_hf)
            cnt0 = ps->c0divpos;
        else
            cnt0 = ps->c0divpos/ps->mdivk;
    }
    if(recalc1)
    {
        if(ps->c1_f0)
        {
            if(ps->c0_hf)
                cnt1 = ps->c1divpos/256;
            else
                cnt1 = ps->c1divpos/256/ps->mdivk;
        }
        else
        {
            cnt1 = ps->c1divpos/ps->mdivk;
        }
    }
    if(recalc2)
    {
        if(ps->c2_hf)
            cnt2 = ps->c2divpos;
        else
            cnt2 = ps->c2divpos/ps->mdivk;
    }
    if(recalc3)
    {
        if(ps->c3_f2)
        {
            if(ps->c2_hf)
                cnt3 = ps->c3divpos/256;
            else
                cnt3 = ps->c3divpos/256/ps->mdivk;
        }
    }

    if(recalc0)
    {
        if(nc0_hf)
            ps->c0divpos = cnt0;
        else
            ps->c0divpos = cnt0*new_divk;
    }
    if(recalc1)
    {
        if(nc1_f0)
        {
            if(nc0_hf)
                ps->c1divpos = cnt1*256+cnt0;
            else
                ps->c1divpos = (cnt1*256+cnt0)*new_divk;
        }
        else
        {
            ps->c1divpos = cnt1*new_divk;
        }
    }

    if(recalc2)
    {
        if(nc2_hf)
            ps->c2divpos = cnt2;
        else
            ps->c2divpos = cnt2*new_divk;
    }
    if(recalc3)
    {
        if(nc3_f2)
        {
            if(nc2_hf)
                ps->c3divpos = cnt3*256+cnt2;
            else
                ps->c3divpos = (cnt3*256+cnt2)*new_divk;
        }
    }

    ps->c0_hf = nc0_hf;
    ps->c2_hf = nc2_hf;
    ps->c1_f0 = nc1_f0;
    ps->c3_f2 = nc3_f2;
    ps->c0sw4 = nc0sw4;
    ps->c1sw4 = nc1sw4;
    ps->mdivk = new_divk;
}

/* SKCTL for two-tone mode */
void Update_skctl(PokeyState* ps, unsigned char val)
{
    ps->skctl = val;
}

/* if using nonlinear mixing, don't stop ultrasounds */
#ifdef NONLINEAR_MIXING
void Update_c0stop(PokeyState* ps)
{
    ps->outvol_0 = ps->readout_0(ps);
}
void Update_c1stop(PokeyState* ps)
{
    ps->outvol_1 = ps->readout_1(ps);
}
void Update_c2stop(PokeyState* ps)
{
    ps->outvol_2 = ps->readout_2(ps);
}
void Update_c3stop(PokeyState* ps)
{
    ps->outvol_3 = ps->readout_3(ps);
}
#else
void Update_c0stop(PokeyState* ps)
{
    int lim = 1;

    int hfa = 0;
    ps->c0stop = 0;

    if(ps->c0vo || ps->vol0 == 0)
        ps->c0stop = 1;
    else if(!ps->c0sw4 && ps->c0sw3 && ps->c0sw2) /* If channel 0 is a pure tone... */
    {
        if(ps->c1_f0)
        {
            if(ps->c1divstart <= lim)
            {
                ps->c0stop = 1;
                hfa = 1;
            }
        }
        else
        {
            if(ps->c0divstart <= lim)
            {
                ps->c0stop = 1;
                hfa = 1;
            }
        }
    }
    else if(!ps->c0sw4 && ps->c0sw3 && !ps->c0sw2 && ps->c0sw1) /* if channel 0 is poly4... */
    {
        /* period for poly4 signal is 15 cycles */
        if(ps->c1_f0)
        {
            if(ps->c1divstart <= lim*2/15) /* all poly4 signal is above Nyquist */
            {
                ps->c0stop = 1;
                hfa = 1;
            }
        }
        else
        {
            if(ps->c0divstart <= lim*2/15)
            {
                ps->c0stop = 1;
                hfa = 1;
            }
        }
    }

    ps->outvol_0 = 2*ps->readout_0(ps);
    if(hfa)
        ps->outvol_0 = ps->vol0;
}

void Update_c1stop(PokeyState* ps)
{
    int lim = 1;

    int hfa = 0;
    ps->c1stop = 0;

    if(!ps->c1_f0 && (ps->c1vo || ps->vol1 == 0))
        ps->c1stop = 1;
    else if(!ps->c1sw4 && ps->c1sw3 && ps->c1sw2 && ps->c1divstart <= lim) /* If channel 1 is a pure tone */
    {
        ps->c1stop = 1;
        hfa = 1;
    }
    else if(!ps->c1sw4 && ps->c1sw3 && !ps->c1sw2 && ps->c1sw1 && ps->c1divstart <= lim*2/15)  /* all poly4 signal is above Nyquist */
    {
        ps->c1stop = 1;
        hfa = 1;
    }

    ps->outvol_1 = 2*ps->readout_1(ps);
    if(hfa)
        ps->outvol_1 = ps->vol1;
}

void Update_c2stop(PokeyState* ps)
{
    int lim = 1;

    int hfa = 0;
    ps->c2stop = 0;

    if(!ps->c0sw4 && (ps->c2vo || ps->vol2 == 0))
        ps->c2stop = 1;
    /* If channel 2 is a pure tone and no filter for c0... */
    else if(ps->c2sw3 && ps->c2sw2 && !ps->c0sw4)
    {
        if(ps->c3_f2)
        {
            if(ps->c3divstart <= lim)
            {
                ps->c2stop = 1;
                hfa = 1;
            }
        }
        else
        {
            if(ps->c2divstart <= lim)
            {
                ps->c2stop = 1;
                hfa = 1;
            }
        }
    }
    else if(ps->c2sw3 && !ps->c2sw2 && ps->c2sw1 && !ps->c0sw4) /* if channel 2 is poly4 and no filter for c0... */
    {
        /* period for poly4 signal is 15 cycles */
        if(ps->c3_f2)
        {
            if(ps->c3divstart <= lim*2/15) /* all poly4 signal is above Nyquist */
            {
                ps->c2stop = 1;
                hfa = 1;
            }
        }
        else
        {
            if(ps->c2divstart <= lim*2/15)
            {
                ps->c2stop = 1;
                hfa = 1;
            }
        }
    }

    ps->outvol_2 = 2*ps->readout_2(ps);
    if(hfa)
        ps->outvol_2 = ps->vol2;
}

void Update_c3stop(PokeyState* ps)
{
    int lim = 1;
    int hfa = 0;
    ps->c3stop = 0;

    if(!ps->c1sw4 && !ps->c3_f2 && (ps->c3vo || ps->vol3 == 0))
        ps->c3stop = 1;
    /* If channel 3 is a pure tone */
    else if(ps->c3sw3 && ps->c3sw2 && !ps->c1sw4 && ps->c3divstart <= lim)
    {
        ps->c3stop = 1;
        hfa = 1;
    }
    else if(ps->c3sw3 && !ps->c3sw2 && ps->c3sw1 && !ps->c1sw4 && ps->c3divstart <= lim*2/15)  /* all poly4 signal is above Nyquist */
    {
        ps->c3stop = 1;
        hfa = 1;
    }

    ps->outvol_3 = 2*ps->readout_3(ps);
    if(hfa)
        ps->outvol_3 = ps->vol3;
}
#endif /*NONLINEAR_MIXING*/

/*****************************************************************************/
/* Function: Update_pokey_sound_mz()                                         */
/*                                                                           */
/* Inputs:  addr - the address of the parameter to be changed                */
/*          val - the new value to be placed in the specified address        */
/*          chip - chip # for stereo                                         */
/*          gain - specified as an 8-bit fixed point number - use 1 for no   */
/*                 amplification (output is multiplied by gain)              */
/*                                                                           */
/* Outputs: Adjusts local globals - no return value                          */
/*                                                                           */
/*****************************************************************************/
void Update_pokey_sound_mz(PokeyState* ps, unsigned short addr, unsigned char val, unsigned char gain)
{
    switch(addr & 0x0f)
    {
    case POKEY_OFFSET_AUDF1:
        ps->c0diva = val;
        Update_c0divstart(ps);
        if(ps->c1_f0)
        {
            Update_c1divstart(ps);
            Update_c1stop(ps);
        }
        Update_c0stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_AUDC1:
        ps->c0sw1 = (val & 0x40) != 0;
        ps->c0sw2 = (val & 0x20) != 0;
        ps->c0sw3 = (val & 0x80) != 0;
        ps->vol0 = (val & 0xF);
        ps->c0vo = (val & 0x10) != 0;
        Update_readout_0(ps);
        Update_event0(ps);
        Update_c0stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_AUDF2:
        ps->c1diva = val;
        Update_c1divstart(ps);
        if(ps->c1_f0)
        {
            Update_c0divstart(ps);
            Update_c0stop(ps);
        }
        Update_c1stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_AUDC2:
        ps->c1sw1 = (val & 0x40) != 0;
        ps->c1sw2 = (val & 0x20) != 0;
        ps->c1sw3 = (val & 0x80) != 0;
        ps->vol1 = (val & 0xF);
        ps->c1vo = (val & 0x10) != 0;
        Update_readout_1(ps);
        Update_event1(ps);
        Update_c1stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_AUDF3:
        ps->c2diva = val;
        Update_c2divstart(ps);
        if(ps->c3_f2)
        {
            Update_c3divstart(ps);
            Update_c3stop(ps);
        }
        Update_c2stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_AUDC3:
        ps->c2sw1 = (val & 0x40) != 0;
        ps->c2sw2 = (val & 0x20) != 0;
        ps->c2sw3 = (val & 0x80) != 0;
        ps->vol2 = (val & 0xF);
        ps->c2vo = (val & 0x10) != 0;
        Update_readout_2(ps);
        Update_event2(ps);
        Update_c2stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_AUDF4:
        ps->c3diva = val;
        Update_c3divstart(ps);
        if(ps->c3_f2)
        {
            Update_c2divstart(ps);
            Update_c2stop(ps);
        }
        Update_c3stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_AUDC4:
        ps->c3sw1 = (val & 0x40) != 0;
        ps->c3sw2 = (val & 0x20) != 0;
        ps->c3sw3 = (val & 0x80) != 0;
        ps->vol3 = val & 0xF;
        ps->c3vo = (val & 0x10) != 0;
        Update_readout_3(ps);
        Update_event3(ps);
        Update_c3stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_AUDCTL:
        ps->selpoly9 = (val & 0x80) != 0;
        Update_audctl(ps,val);
        Update_readout_0(ps);
        Update_readout_1(ps);
        Update_readout_2(ps);
        Update_readout_3(ps);
        Update_c0divstart(ps);
        Update_c1divstart(ps);
        Update_c2divstart(ps);
        Update_c3divstart(ps);
        Update_c0stop(ps);
        Update_c1stop(ps);
        Update_c2stop(ps);
        Update_c3stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_STIMER:
        if(ps->c1_f0)
            ps->c0divpos = ps->c0divstart_p;
        else
            ps->c0divpos = ps->c0divstart;
        ps->c1divpos = ps->c1divstart;
        if(ps->c3_f2)
            ps->c2divpos = ps->c2divstart_p;
        else
            ps->c2divpos = ps->c2divstart;

        ps->c3divpos = ps->c3divstart;
        /*Documentation is wrong about which voices are on after STIMER*/
        /*It is 3&4 which are on, tested on a real atari*/
        ps->c0t2 = 0;
        ps->c1t2 = 0;
        ps->c2t2 = 1;
        ps->c3t2 = 1;
        break;
    case POKEY_OFFSET_SKCTL:
        Update_skctl(ps,val);
        break;
    }
}


/**************************************************************

           Master gain and DC offset calculation
                 by Michael Borisov

 In order to use the available 8-bit or 16-bit dynamic range
 to full extent, reducing the influence of quantization
 noise while simultaneously avoiding overflows, gain
 and DC offset should be set to appropriate value.

 All Pokey-generated sounds have maximal amplitude of 15.
 When all four channels sound simultaneously and in the
 same phase, amplidudes would add up to 60.

 If Pokey is generating a 'pure tone', it always has a DC
 offset of half its amplitude. For other signals (produced
 by poly generators) DC offset varies, but it is always near
 to half amplitude and never exceeds this value.

 In case that pure tone base frequency is outside of audible
 range (ultrasound frequency for high sample rates and above-
 Nyquist frequency for low sample rates), to speed up the engine,
 the generator is stopped while having only DC offset on the
 output (half of corresponding AUDV value). In order that this
 DC offset can be always represented as integer, AUDV values
 are multiplied by 2 when the generator works.

 Therefore maximum integer value before resampling filters
 would be 60*2 = 120 while having maximum DC offset of 60.
 Resampling does not change the DC offset, therefore we may
 subtract it from the signal either before or after resampling.
 In mzpokeysnd, DC offset is subtracted after resampling, however
 for better understanding in further measurements I assume
 subtracting DC before. So, input range for the resampler
 becomes [-60 .. 60].

 Resampling filter removes some harmonics from the signal as if
 the rectangular wave was Fourier transformed forth-and-back,
 while zeroing all harmonics above cutoff frequency. In case
 of high-frequency pure tone (above samplerate/8), only first
 harmonic of the Fourier transofm will remain. As it
 is known, Fourier-transform of the rectangular function of
 amplitude 1 has first oscillation component of amplitude 4/M_PI.
 Therefore, maximum sample values for filtered rectangular
 signal may exceed the amplitude  of rectangular signal
 by up to 4/M_PI times.

 Since our range before resampler is -60 .. 60, taking into
 account mentioned effect with band limiting, range of values
 on the resampler output appears to be in the following bounds:
 [-60*4/M_PI .. 60*4/M_PI]

 In order to map this into signed 8-bit range [-128 .. 127], we
 should multiply the resampler output by 127/60/4*M_PI.

 As it is common for sound hardware to have 8-bit sound unsigned,
 additional DC offset of 128 must be added.

 For 16-bit case the output range is [-32768 .. 32767], and
 we should multiply the resampler output by 32767/60/4*M_PI

 To make some room for numerical errors, filter ripples and
 quantization noise, so that they do not cause overflows in
 quantization, dynamic range is reduced in mzpokeysnd by
 multiplying the output amplitude with 0.95, reserving 5%
 of the total range for such effects, which is about 0.51db.

 Mentioned gain and DC values were tested with 17kHz audio
 playing synchronously on 4 channels, which showed to be
 utilizing 95% of the sample values range.

 Since any other gain value will be not optimal, I removed
 user gain setting and hard-coded the gain into mzpokeysnd

 ---

 A note from Piotr Fusik:
 I've added support for the key click sound generated by GTIA. Its
 volume seems to be pretty much like 8 on single POKEY's channel.
 So, the volumes now can sum up to 136 (4 channels * 15 * 2
 + 8 * 2 for GTIA), not 120.

 A note from Mark Grebe:
 I've added back in the console and sio sounds from the old
 pokey version.  So, now the volumes can sum up to 152
 (4 channesl * 15 * 2 + 8 * 4 for old sound), not 120 or 136.

 ******************************************************************/


/******************************************************************

          Quantization effects and dithering
              by Michael Borisov

 Quantization error in the signal has an expectation value of half
 the LSB, when the rounding is performed properly. Sometimes they
 express quantization error as a random function with even
 distribution over the range [-0.5 to 0.5]. Spectrum of this function
 is flat, because it's a white noise.

 Power of a discrete signal (including noise) is calculated as
 mean square of its samples. For the mentioned above noise
 this is approximately 0.33. Therefore, in decibels for 8-bit case,
 our noise will have power of 10*log10(0.33/256/256) = -53dB

 Because noise is white, this power of -53dB will be evenly
 distributed over the whole signal band upto Nyquist frequency.
 The larger the band is (higher sampling frequency), less
 is the quantisation noise floor. For 8000Hz noise floor is
 10*log10(0.33/256/256/4000) = -89dB/Hz, and for 44100Hz noise
 floor is 10*log10(0.33/256/256/22050) = -96.4dB/Hz.
 This shows that higher sampling rates are better in sense of
 quantization noise. Moreover, as large part of quantization noise
 in case of 44100Hz will fall into ultrasound and hi-frequency
 area 10-20kHz where human ear is less sensitive, this will
 show up as great improvement in quantization noise performance
 compared to 8000Hz.

 I was plotting spectral analysis for sounds produced by mzpokeysnd
 to check these measures. And it showed up that in 8-bit case
 there is no expected flat noise floor of -89db/Hz for 8000Hz,
 but some distortion spectral peaks had higher amplitude than
 the aliasing peaks in 16-bit case. This was a proof to another
 statement which says that quantization noise tends to become
 correlated with the signal. Correlation is especially strong
 for simple signals generated by Pokey. Correlation means that
 the noise power of -53db is no longer evenly distributed
 across the whole frequency range, but concentrates in larger
 peaks at locations which depend on the Pokey signal.

 To decorrelate quantization distortion and make it again
 white noise, which would improve the sound spectrum, since
 the maximum distortion peaks will have less amplitude,
 dithering is used. Another white noise is added to the signal
 before quantization. Since this added noise is not correlated
 with the signal, it shows itself as a flat noise floor.
 Quantization noise now tries to correlate with the dithering
 noise, but this does not lead to appearance of sharp
 spectral peaks any more :)

 Another thing is that for listening, white noise is better than
 distortion. This is because human hearing has some 'noise
 reduction' system which makes it easier to percept sounds
 on the white noise background.

 From the other point of view, if a signal has high and low
 spectral peaks, it is desirable that there is no distortion
 component with peaks of amplitude comparable to those of
 the true signal. Otherwise, perception of background low-
 amplitude signals will be disrupted. That's why they say
 that dithering extends dynamic range.

 Dithering does not eliminate correlation of quantization noise
 completely. Degree of reduction of this effect depends on
 the dithering noise power. The higher is dithering noise,
 the more quantization noise is decorrelated. But this also
 leads to increase of noise percepted by the listener. So, an
 optimum value should be selected. My experiments show that
 unbiased rand() noise of amplitude 0.25 LSB is doing well.

 Test spectral pictures for 8-bit sound, 8kHz sampling rate,
 dithered, show a noise floor of approx. -87dB/Hz.

 ******************************************************************/

#define MAX_SAMPLE 152

void mzpokeysnd_process_16(PokeyState* ps, void* sndbuffer, int sndn)
{
    int i;
    int nsam = sndn;
    short *buffer = (short *) sndbuffer;

    /* if there are two pokeys, then the signal is stereo
       we assume even sndn */
    while(nsam >= (int) 1)
    {
        buffer[0] = (short)floor(generate_sample(ps)
         * (65535.0 / 2 / MAX_SAMPLE / 4 * M_PI * 0.95));
        buffer += 1;
        nsam -= 1;
    }
}
