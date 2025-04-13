/*
 * Copyright (C) 2023-2024 nukeykt
 *
 * This file is part of YM2608-LLE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  YM2608/YM2610/YM2612 emulator.
 *  Thanks:
 *      Raki (ika-musume):
 *          YM2608B decap
 *      John McMaster (siliconpr0n.org):
 *          YM2610 decap
 *      HardWareMan:
 *          YM2612 decap
 *
 */
#include <string.h>
#include "fmopna_impl.h"
#ifdef FMOPNA_YM2608
#include "fmopna_rom.h"
#endif

enum {
    eg_state_attack = 0,
    eg_state_decay,
    eg_state_sustain,
    eg_state_release
};

static const int fm_algorithm[4][6][8] = {
    {
        { 1, 1, 1, 1, 1, 1, 1, 1 }, /* OP1_0         */
        { 1, 1, 1, 1, 1, 1, 1, 1 }, /* OP1_1         */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* OP2           */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* Last operator */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* Last operator */
        { 0, 0, 0, 0, 0, 0, 0, 1 }  /* Out           */
    },
    {
        { 0, 1, 0, 0, 0, 1, 0, 0 }, /* OP1_0         */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* OP1_1         */
        { 1, 1, 1, 0, 0, 0, 0, 0 }, /* OP2           */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* Last operator */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* Last operator */
        { 0, 0, 0, 0, 0, 1, 1, 1 }  /* Out           */
    },
    {
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* OP1_0         */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* OP1_1         */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* OP2           */
        { 1, 0, 0, 1, 1, 1, 1, 0 }, /* Last operator */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* Last operator */
        { 0, 0, 0, 0, 1, 1, 1, 1 }  /* Out           */
    },
    {
        { 0, 0, 1, 0, 0, 1, 0, 0 }, /* OP1_0         */
        { 0, 0, 0, 0, 0, 0, 0, 0 }, /* OP1_1         */
        { 0, 0, 0, 1, 0, 0, 0, 0 }, /* OP2           */
        { 1, 1, 0, 1, 1, 0, 0, 0 }, /* Last operator */
        { 0, 0, 1, 0, 0, 0, 0, 0 }, /* Last operator */
        { 1, 1, 1, 1, 1, 1, 1, 1 }  /* Out           */
    }
};


#ifdef FMOPNA_YM2608
void FMOPNA_Clock(fmopna_t* chip, int clk)
#elif defined (FMOPNA_YM2610)
void FMOPNA_2610_Clock(fmopna_2610_t* chip, int clk)
#else
void FMOPNA_2612_Clock(fmopna_2612_t* chip, int clk)
#endif
{
    int i;

    chip->input.clk = clk;

    chip->mclk1 = !chip->input.clk;
    chip->mclk2 = chip->input.clk;

    chip->ic = !chip->input.ic;


    if (chip->mclk1)
    {
        int prescaler_of = 0;
        int dclk = 1;
#ifdef FMOPNA_YM2608
        switch (chip->prescaler_sel[1])
        {
            case 2:
                prescaler_of = (chip->prescaler_latch[1] & 0x7ff) == 0;
                dclk = (chip->prescaler_latch[1] & 0xffc) == 0;
                break;
            case 0:
                prescaler_of = (chip->prescaler_latch[1] & 7) == 0;
                dclk = (chip->prescaler_latch[1] & 7) != 0;
                break;
            case 3:
                prescaler_of = (chip->prescaler_latch[1] & 0x1f) == 0;
                dclk = (chip->prescaler_latch[1] & 0x1f) == 0;
                break;

        }
#endif
#ifdef FMOPNA_YM2610
        prescaler_of = (chip->prescaler_latch[1] & 0x7ff) == 0;
        dclk = (chip->prescaler_latch[1] & 0xffc) == 0;
#endif
#ifdef FMOPNA_YM2612
        prescaler_of = (chip->prescaler_latch[1] & 0x1f) == 0;
#endif
        chip->ic_latch1[0] = chip->ic;

        chip->ic_latch2[0] = (chip->ic_latch2[1] << 1) | chip->ic_latch1[1];
        chip->ic_latch3[0] = (chip->ic_latch3[1] << 1) | chip->ic_check1;

        chip->prescaler_latch[0] = chip->prescaler_latch[1] << 1;
        chip->prescaler_latch[0] |= !chip->ic_check1 && prescaler_of;

#ifdef FMOPNA_YM2608
        chip->pssel_l[0][0] = (chip->prescaler_latch[1] & 0x861) != 0;
        chip->pssel_l[1][0] = (chip->prescaler_latch[1] & 0x30c) != 0;
        chip->pssel_l[2][0] = (chip->prescaler_latch[1] & 0x12) != 0;
        chip->pssel_l[3][0] = (chip->prescaler_latch[1] & 0x24) != 0;
        chip->pssel_l[4][0] = chip->pssel_l[4][1] << 1;
        chip->pssel_l[4][0] |= (chip->prescaler_latch[1] & 0x5) != 0;
        chip->pssel_l[5][0] = (chip->prescaler_latch[1] & 0x3c) != 0;
        chip->pssel_l[6][0] = (chip->prescaler_latch[1] & 0xf00) != 0;
        chip->pssel_l[7][0] = (chip->prescaler_latch[1] & 0xc) != 0;
        chip->pssel_l[8][0] = (chip->prescaler_latch[1] & 0x21) != 0;
        chip->pssel_l[9][0] = (chip->prescaler_latch[1] & 0x2) != 0;
        chip->pssel_l[10][0] = (chip->prescaler_latch[1] & 0x1) != 0;
        chip->pssel_l[11][0] = (chip->prescaler_latch[1] & 0x924) != 0;
        chip->pssel_l[12][0] = (chip->prescaler_latch[1] & 0x249) != 0;
        chip->pssel_l[13][0] = chip->pssel_l[13][1] << 1;
        chip->pssel_l[13][0] |= (chip->prescaler_latch[1] & 9) == 0;
        chip->pssel_l[14][0] = (chip->prescaler_latch[1] & 0x555) != 0;

        chip->dclk = dclk;
#endif
#ifdef FMOPNA_YM2610
        chip->pssel_l[0][0] = (chip->prescaler_latch[1] & 0x861) != 0;
        chip->pssel_l[1][0] = (chip->prescaler_latch[1] & 0x30c) != 0;
        chip->pssel_l[5][0] = (chip->prescaler_latch[1] & 0x3c) != 0;
        chip->pssel_l[6][0] = (chip->prescaler_latch[1] & 0xf00) != 0;
        chip->pssel_l[11][0] = (chip->prescaler_latch[1] & 0x924) != 0;
        chip->pssel_l[12][0] = (chip->prescaler_latch[1] & 0x249) != 0;
        chip->pssel_l[14][0] = (chip->prescaler_latch[1] & 0x555) != 0;

        chip->dclk = dclk;
#endif
#ifdef FMOPNA_YM2612
        chip->pssel_l[0][0] = (chip->prescaler_latch[1] & 0x21) != 0;
        chip->pssel_l[1][0] = (chip->prescaler_latch[1] & 0xc) != 0;
#endif

    }
    if (chip->mclk2)
    {
        chip->ic_latch1[1] = chip->ic_latch1[0];
        chip->ic_latch2[1] = chip->ic_latch2[0];
        chip->ic_latch3[1] = chip->ic_latch3[0];

#ifndef FMOPNA_YM2612
        chip->ic_check1 = chip->ic_latch1[1] && (chip->ic_latch2[1] & 0x20000) == 0;
        chip->ic_check2 = (chip->ic_latch3[1] & 4) != 0;
#else
        chip->ic_check1 = chip->ic_latch1[1] && (chip->ic_latch2[1] & 0x800) == 0;
#endif
        chip->ic_check3 = (chip->ic_latch3[1] & 8) != 0;



        chip->prescaler_latch[1] = chip->prescaler_latch[0];

#ifndef FMOPNA_YM2612
        for (i = 0; i < 15; i++)
            chip->pssel_l[i][1] = chip->pssel_l[i][0];
#else
        chip->pssel_l[0][1] = chip->pssel_l[0][0];
        chip->pssel_l[1][1] = chip->pssel_l[1][0];
#endif
    }

    if (chip->clk1)
    {
        chip->ic_latch_fm[0] = chip->ic;
    }
    if (chip->clk2)
    {
        chip->ic_latch_fm[1] = chip->ic_latch_fm[0];
    }

    {
#ifndef FMOPNA_YM2612
        int clk1 = 1;
        int clk2 = 1;
        int aclk1 = 1;
        int aclk2 = 1;
        int bclk1 = 1;
        int bclk2 = 1;
#ifdef FMOPNA_YM2608
        switch (chip->prescaler_sel[1])
        {
            case 2:
                clk1 = chip->pssel_l[0][1];
                clk2 = chip->pssel_l[1][1];
                aclk1 = chip->pssel_l[5][1];
                aclk2 = chip->pssel_l[6][1];
                bclk1 = chip->pssel_l[11][1];
                bclk2 = chip->pssel_l[12][1];
                break;
            case 0:
                clk1 = (chip->pssel_l[4][1] & 2) != 0 && (chip->pssel_l[4][0] & 4) != 0;
                clk2 = (chip->pssel_l[4][1] & 1) != 0 && (chip->pssel_l[4][0] & 2) != 0;
                aclk1 = chip->pssel_l[9][1];
                aclk2 = chip->pssel_l[10][1];
                bclk1 = !chip->mclk2;
                bclk2 = !chip->mclk1;
                break;
            case 3:
                clk1 = chip->pssel_l[2][1];
                clk2 = chip->pssel_l[3][1];
                aclk1 = chip->pssel_l[7][1];
                aclk2 = chip->pssel_l[8][1];
                bclk1 = ((chip->pssel_l[13][1] & 4) == 0 && (chip->pssel_l[13][0] & 8) != 0)
                    || ((chip->pssel_l[13][1] & 1) == 0 && (chip->pssel_l[13][0] & 2) == 0);
                bclk2 = ((chip->pssel_l[13][1] & 1) == 0 && (chip->pssel_l[13][0] & 2) != 0)
                    || ((chip->pssel_l[13][1] & 2) == 0 && (chip->pssel_l[13][0] & 4) == 0);
                break;

        }
#endif
#ifdef FMOPNA_YM2610
        clk1 = chip->pssel_l[0][1];
        clk2 = chip->pssel_l[1][1];
        aclk1 = chip->pssel_l[5][1];
        aclk2 = chip->pssel_l[6][1];
        bclk1 = chip->pssel_l[11][1];
        bclk2 = chip->pssel_l[12][1];
#endif


        chip->clk1 = clk1;
        chip->clk2 = clk2;

        chip->aclk1 = aclk1;
        chip->aclk2 = aclk2;

        chip->bclk1 = bclk1;
        chip->bclk2 = bclk2;

        chip->cclk1 = !chip->pssel_l[14][1];
        chip->cclk2 = chip->pssel_l[14][1];
#else
        chip->clk1 = chip->pssel_l[0][1];
        chip->clk2 = chip->pssel_l[1][1];
#endif
    }

    // if (clk)
    //     return;

    {
        int read = !chip->ic && !chip->input.rd && !chip->input.cs;
        chip->read0 = !chip->ic && !chip->input.rd && !chip->input.cs && !chip->input.a1 && !chip->input.a0;
        int write = !chip->input.wr && !chip->input.cs;
        int writeaddr = chip->ic || (!chip->input.wr && !chip->input.cs && !chip->input.a0);
        int writedata = !chip->ic && !chip->input.wr && !chip->input.cs && chip->input.a0;
#ifndef FMOPNA_YM2612
        int read1 = !chip->ic && !chip->input.rd && !chip->input.cs && !chip->input.a1 && chip->input.a0;
        chip->read2 = !chip->ic && !chip->input.rd && !chip->input.cs && chip->input.a1 && !chip->input.a0;
#ifdef FMOPNA_YM2608
        chip->read3 = !chip->ic && !chip->input.rd && !chip->input.cs && chip->input.a1 && chip->input.a0;
        chip->write2 = !chip->ic && !chip->input.wr && !chip->input.cs && chip->input.a1 && !chip->input.a0;
        chip->write3 = !chip->ic && !chip->input.wr && !chip->input.cs && chip->input.a1 && chip->input.a0;
#endif
#ifdef FMOPNA_YM2610
        chip->write2 = !chip->ic && !chip->input.wr && !chip->input.cs && !chip->input.a1 && !chip->input.a0;
        chip->write3 = !chip->ic && !chip->input.wr && !chip->input.cs && !chip->input.a1 && chip->input.a0;
#endif
        chip->ssg_write0 = writeaddr && !chip->input.a1;
        chip->ssg_write1 = (writedata && !chip->input.a1) || chip->ic;
        chip->ssg_read1 = read1;
#endif


        chip->o_data_d = !read;

        if (writeaddr)
            chip->write0_trig0 = 1;
        else if (chip->write0_l[0])
            chip->write0_trig0 = 0;
        if (chip->clk1)
        {
            chip->write0_trig1 = chip->write0_trig0;
            chip->write0_l[1] = chip->write0_l[0];
        }
        if (chip->clk2)
        {
            chip->write0_l[0] = chip->write0_trig1;
            chip->write0_l[2] = chip->write0_l[1];
        }
        chip->write0_en = chip->write0_l[0] && !chip->write0_l[2];

        if (writedata)
            chip->write1_trig0 = 1;
        else if (chip->write1_l[0])
            chip->write1_trig0 = 0;
        if (chip->clk1)
        {
            chip->write1_trig1 = chip->write1_trig0;
            chip->write1_l[1] = chip->write1_l[0];
        }
        if (chip->clk2)
        {
            chip->write1_l[0] = chip->write1_trig1;
            chip->write1_l[2] = chip->write1_l[1];
        }
        chip->write1_en = chip->write1_l[0] && !chip->write1_l[2];

#ifdef FMOPNA_YM2608
        if (writeaddr)
            chip->write2_trig0 = 1;
        else if (chip->write2_l[0])
            chip->write2_trig0 = 0;
        if (chip->mclk1)
        {
            chip->write2_trig1 = chip->write2_trig0;
            chip->write2_l[1] = chip->write2_l[0];
        }
        if (chip->mclk2)
        {
            chip->write2_l[0] = chip->write2_trig1;
            chip->write2_l[2] = chip->write2_l[1];
        }
        chip->write2_en = chip->write2_l[0] && !chip->write2_l[2];
#endif

#ifndef FMOPNA_YM2612
        if (writedata)
            chip->write3_trig0 = 1;
        else if (chip->write3_l[0])
            chip->write3_trig0 = 0;
#endif

        if (write)
            chip->data_l = (chip->input.data & 255) | (chip->input.a1 << 8);

#ifndef FMOPNA_YM2612
        if (chip->ic)
            chip->data_bus2 |= 0x2f;
        else
#endif
        if (!read && !chip->ic)
            chip->data_bus2 = chip->data_l ^ 0x1ff;

        if (chip->ic_latch_fm[1])
        {
            chip->data_bus1 &= ~255;
        }
        else if (!read && !chip->ic)
        {
            chip->data_bus1 = chip->data_l;
        }
#ifdef FMOPNA_YM2608
        else if (read1 && chip->addr_ff[1])
        {
            chip->data_bus1 &= ~255;
            chip->data_bus1 |= 1;
        }
#endif

#define ADDRESS_MATCH(x) ((chip->data_bus2 & x) == 0 && (chip->data_bus1 & (x^511)) == 0)

#ifdef FMOPNA_YM2608
        if (chip->mclk1)
        {
            int addr2d = chip->write2_en && ADDRESS_MATCH(0x2d);
            int addr2e = chip->write2_en && ADDRESS_MATCH(0x2e);
            int addr2f = chip->write2_en && ADDRESS_MATCH(0x2f);
            chip->prescaler_sel[0] = chip->prescaler_sel[1];
            if (addr2f)
                chip->prescaler_sel[0] = 0;
            if (chip->ic)
                chip->prescaler_sel[0] = 2;
            if (addr2d)
                chip->prescaler_sel[0] |= 2;
            if (addr2e)
                chip->prescaler_sel[0] |= 1;
        }
        if (chip->mclk2)
        {
            chip->prescaler_sel[1] = chip->prescaler_sel[0];
        }
#endif

        if (chip->clk1)
        {
            int is_fm = (chip->data_bus1 & 0xf0) != 0;

            chip->write_fm_address[0] = chip->write0_en ? is_fm : chip->write_fm_address[1];

            if (chip->ic)
                chip->fm_address[0] = 0;
            else if (is_fm && chip->write0_en)
                chip->fm_address[0] = chip->data_bus1;
            else
                chip->fm_address[0] = chip->fm_address[1];

            if (chip->ic)
                chip->fm_data[0] = 0;
            else if (chip->write_fm_address[1] && chip->write1_en)
                chip->fm_data[0] = chip->data_bus1 & 255;
            else
                chip->fm_data[0] = chip->fm_data[1];

            chip->write_fm_data[0] = (chip->write_fm_address[1] && chip->write1_en) || (chip->write_fm_data[1] && !chip->write0_en);

#ifdef FMOPNA_YM2608
            chip->addr_10[0] = chip->write0_en ? ADDRESS_MATCH(0x10) : chip->addr_10[1];
            chip->addr_10h[0] = chip->write0_en ? ADDRESS_MATCH(0x110) : chip->addr_10h[1];
            chip->addr_12[0] = chip->write0_en ? ADDRESS_MATCH(0x12) : chip->addr_12[1];
            chip->addr_29[0] = chip->write0_en ? ADDRESS_MATCH(0x29) : chip->addr_29[1];
            chip->addr_ff[0] = chip->write0_en ? ADDRESS_MATCH(0xff) : chip->addr_ff[1];
            int write10 = chip->addr_10h[1] && (chip->data_bus1 & 0x100) != 0 && chip->write1_en;
            int irq_rst = write10 && (chip->data_bus2 & 0x80) == 0;

#endif
#ifdef FMOPNA_YM2610
            chip->addr_00[0] = chip->write0_en ? ADDRESS_MATCH(0x100) : chip->addr_00[1];
            chip->addr_02[0] = chip->write0_en ? ADDRESS_MATCH(0x102) : chip->addr_02[1];
            chip->addr_1c[0] = chip->write0_en ? ADDRESS_MATCH(0x11c) : chip->addr_1c[1];
#endif
#ifdef FMOPNA_YM2612
            chip->addr_2a[0] = chip->write0_en ? ADDRESS_MATCH(0x2a) : chip->addr_2a[1];
            chip->addr_2b[0] = chip->write0_en ? ADDRESS_MATCH(0x2b) : chip->addr_2b[1];
            chip->addr_2c[0] = chip->write0_en ? ADDRESS_MATCH(0x2c) : chip->addr_2c[1];
#endif
            chip->addr_21[0] = chip->write0_en ? ADDRESS_MATCH(0x21) : chip->addr_21[1];
            chip->addr_22[0] = chip->write0_en ? ADDRESS_MATCH(0x22) : chip->addr_22[1];
            chip->addr_24[0] = chip->write0_en ? ADDRESS_MATCH(0x24) : chip->addr_24[1];
            chip->addr_25[0] = chip->write0_en ? ADDRESS_MATCH(0x25) : chip->addr_25[1];
            chip->addr_26[0] = chip->write0_en ? ADDRESS_MATCH(0x26) : chip->addr_26[1];
            chip->addr_27[0] = chip->write0_en ? ADDRESS_MATCH(0x27) : chip->addr_27[1];
            chip->addr_28[0] = chip->write0_en ? ADDRESS_MATCH(0x28) : chip->addr_28[1];

            if (chip->ic)
            {
                chip->reg_test_21[0] = 0;
                chip->reg_lfo[0] = 0;
                chip->reg_timer_a[0] = 0;
                chip->reg_timer_b[0] = 0;
                chip->reg_ch3[0] = 0;
                chip->reg_timer_a_load[0] = 0;
                chip->reg_timer_b_load[0] = 0;
                chip->reg_timer_a_enable[0] = 0;
                chip->reg_timer_b_enable[0] = 0;
                chip->reg_kon_operator[0] = 0;
                chip->reg_kon_channel[0] = 0;
#ifdef FMOPNA_YM2608
                chip->reg_test_12[0] = 0;
                chip->reg_sch[0] = 0;
                chip->reg_irq[0] = 31;
                chip->reg_mask[0] = 28;
#endif
#ifdef FMOPNA_YM2610
                chip->reg_test_12[0] = 0;
#endif
#ifdef FMOPNA_YM2612
                chip->reg_dac_en[0] = 0;
                chip->reg_dac_data[0] = 0x80;
                chip->reg_test_2c[0] = 0;
#endif
            }
            else
            {
#ifdef FMOPNA_YM2608
                int write10r = write10 && (chip->data_bus2 & 0x80) != 0;
                if (write10r)
                {
                    chip->reg_mask[0] = chip->data_bus1 & 3;
                    chip->reg_mask[0] |= (chip->data_bus2 ^ 28) & 28;
                }
                else
                {
                    chip->reg_mask[0] = chip->reg_mask[1];
                }
                if (chip->addr_12[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_test_12[0] = chip->data_bus1 & 252;
                }
                else
                {
                    chip->reg_test_12[0] = chip->reg_test_12[1];
                }
#endif
#ifdef FMOPNA_YM2610
                if (chip->addr_1c[1] && (chip->data_bus1 & 0x100) != 0 && chip->write1_en)
                {
                    chip->reg_flags[0] = chip->data_bus1 & 0xbf;
                }
                else
                {
                    chip->reg_flags[0] = chip->reg_flags[1];
                }
                if (chip->addr_02[1] && (chip->data_bus1 & 0x100) != 0 && chip->write1_en)
                {
                    chip->reg_test_12[0] = chip->data_bus1 & 0xb8;
                }
                else
                {
                    chip->reg_test_12[0] = chip->reg_test_12[1];
                }
#endif
                if (chip->addr_21[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_test_21[0] = chip->data_bus1 & 255;
                }
                else
                {
                    chip->reg_test_21[0] = chip->reg_test_21[1];
                }
                if (chip->addr_22[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_lfo[0] = chip->data_bus1 & 15;
                }
                else
                {
                    chip->reg_lfo[0] = chip->reg_lfo[1];
                }
                chip->reg_timer_a[0] = 0;
                if (chip->addr_24[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_timer_a[0] |= (chip->data_bus1 & 255) << 2;
                }
                else
                {
                    chip->reg_timer_a[0] |= chip->reg_timer_a[1] & 0x3fc;
                }
                if (chip->addr_25[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_timer_a[0] |= chip->data_bus1 & 3;
                }
                else
                {
                    chip->reg_timer_a[0] |= chip->reg_timer_a[1] & 3;
                }
                if (chip->addr_26[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_timer_b[0] = chip->data_bus1 & 255;
                }
                else
                {
                    chip->reg_timer_b[0] = chip->reg_timer_b[1];
                }
                if (chip->addr_27[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_ch3[0] = (chip->data_bus1 >> 6) & 3;
                    chip->reg_timer_a_load[0] = (chip->data_bus1 >> 0) & 1;
                    chip->reg_timer_b_load[0] = (chip->data_bus1 >> 1) & 1;
                    chip->reg_timer_a_enable[0] = (chip->data_bus1 >> 2) & 1;
                    chip->reg_timer_b_enable[0] = (chip->data_bus1 >> 3) & 1;
                }
                else
                {
                    chip->reg_ch3[0] = chip->reg_ch3[1];
                    chip->reg_timer_a_load[0] = chip->reg_timer_a_load[1];
                    chip->reg_timer_b_load[0] = chip->reg_timer_b_load[1];
                    chip->reg_timer_a_enable[0] = chip->reg_timer_a_enable[1];
                    chip->reg_timer_b_enable[0] = chip->reg_timer_b_enable[1];
                }
                if (chip->addr_28[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_kon_operator[0] = (chip->data_bus1 >> 4) & 15;
                    chip->reg_kon_channel[0] = (chip->data_bus1 >> 0) & 7;
                }
                else
                {
                    chip->reg_kon_operator[0] = chip->reg_kon_operator[1];
                    chip->reg_kon_channel[0] = chip->reg_kon_channel[1];
                }
#ifdef FMOPNA_YM2608
                if (chip->addr_29[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_sch[0] = (chip->data_bus1 >> 7) & 1;
                    chip->reg_irq[0] = (chip->data_bus2 & 31) ^ 31;
                }
                else
                {
                    chip->reg_sch[0] = chip->reg_sch[1];
                    chip->reg_irq[0] = chip->reg_irq[1];
                }
#endif
#ifdef FMOPNA_YM2612
                if (chip->addr_2a[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_dac_data[0] = chip->data_bus1 & 255;
                }
                else
                {
                    chip->reg_dac_data[0] = chip->reg_dac_data[1];
                }
                if (chip->addr_2b[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_dac_en[0] = (chip->data_bus1 & 128) != 0;
                }
                else
                {
                    chip->reg_dac_en[0] = chip->reg_dac_en[1];
                }
                if (chip->addr_2c[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en)
                {
                    chip->reg_test_2c[0] = chip->data_bus1 & 0xf8;
                }
                else
                {
                    chip->reg_test_2c[0] = chip->reg_test_2c[1];
                }
#endif
            }

            chip->reg_timer_a_reset[0] = chip->addr_27[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en && ((chip->data_bus1 >> 4) & 1) != 0;
            chip->reg_timer_b_reset[0] = chip->addr_27[1] && (chip->data_bus1 & 0x100) == 0 && chip->write1_en && ((chip->data_bus1 >> 5) & 1) != 0;

            int rst1 = chip->reg_cnt_sync || chip->ic;
            int of = (chip->reg_cnt1[1] & 2) != 0;

            chip->reg_cnt1[0] = (rst1 || of) ? 0 : ((chip->reg_cnt1[1] + 1) & 3);
            chip->reg_cnt2[0] = rst1 ? 0 : ((chip->reg_cnt2[1] + of) & 3);

#ifdef FMOPNA_YM2608
            int rst2 = chip->reg_cnt_sync || chip->reg_cnt_rss_of;
            chip->reg_cnt_rss[0] = rst2 ? 0 : ((chip->reg_cnt_rss[1] + 1) & 15);
#endif

            int of2 = (chip->reg_key_cnt1[1] & 2) != 0;
            chip->reg_key_cnt1[0] = (rst1 || of2) ? 0 : ((chip->reg_key_cnt1[1] + 1) & 3);
            chip->reg_key_cnt2[0] = rst1 ? 0 : ((chip->reg_key_cnt2[1] + of2) & 7);

            chip->reg_kon[0][0] = chip->reg_kon[0][1] << 1;
            chip->reg_kon[1][0] = chip->reg_kon[1][1] << 1;
            chip->reg_kon[2][0] = chip->reg_kon[2][1] << 1;
            chip->reg_kon[3][0] = chip->reg_kon[3][1] << 1;

            if (chip->reg_kon_match)
            {
                chip->reg_kon[0][0] |= (chip->reg_kon_operator[1] >> 0) & 1;
                chip->reg_kon[1][0] |= (chip->reg_kon_operator[1] >> 3) & 1;
                chip->reg_kon[2][0] |= (chip->reg_kon_operator[1] >> 1) & 1;
                chip->reg_kon[3][0] |= (chip->reg_kon_operator[1] >> 2) & 1;
            }
            else
            {
                if (!chip->ic)
                    chip->reg_kon[0][0] |= (chip->reg_kon[3][1] >> 5) & 1;
                chip->reg_kon[1][0] |= (chip->reg_kon[0][1] >> 5) & 1;
                chip->reg_kon[2][0] |= (chip->reg_kon[1][1] >> 5) & 1;
                chip->reg_kon[3][0] |= (chip->reg_kon[2][1] >> 5) & 1;
            }

            chip->reg_sync_timer_l[0] = chip->reg_sync_timer;

            int time = chip->timer_a_cnt[1];
            time += (chip->reg_test_21[1] & 4) != 0 || (chip->timer_a_reg_load && chip->reg_sync_timer);

            chip->timer_a_cnt[0] = chip->timer_a_load ? chip->reg_timer_a[1] : (!chip->timer_a_reg_load ? 0 : (time & 1023));
            chip->timer_a_of[0] = (time & 1024) != 0;

            chip->timer_a_reg_load_l[0] = chip->timer_a_reg_load;

            int rst_a = chip->reg_timer_a_reset[1] || chip->ic;
            int rst_a2 = rst_a
#ifdef FMOPNA_YM2608
                || irq_rst || (chip->reg_mask[1] & 1) != 0
#endif
                ;

            if (rst_a2)
                chip->timer_a_status[0] = 0;
            else
                chip->timer_a_status[0] = chip->timer_a_status[1];

            chip->timer_a_status[0] |= !rst_a
#ifdef FMOPNA_YM2608
                && (chip->reg_mask[1] & 1) == 0
#endif
                && chip->reg_timer_a_enable[1] && chip->timer_a_of[1];

            int subcnt = chip->timer_b_subcnt[1] + chip->reg_sync_timer;
            chip->timer_b_subcnt[0] = chip->ic ? 0 : subcnt & 15;
            chip->timer_b_subcnt_of[0] = (subcnt & 16) != 0;

            time = chip->timer_b_cnt[1];
            time += (chip->reg_test_21[1] & 4) != 0 || (chip->timer_b_reg_load && chip->timer_b_subcnt_of[1]);

            chip->timer_b_cnt[0] = chip->timer_b_load ? chip->reg_timer_b[1] : (!chip->timer_b_reg_load ? 0 : (time & 255));
            chip->timer_b_of[0] = (time & 256) != 0;

            chip->timer_b_reg_load_l[0] = chip->timer_b_reg_load;

            int rst_b = chip->reg_timer_b_reset[1] || chip->ic;
            int rst_b2 = rst_b
#ifdef FMOPNA_YM2608
                || irq_rst || (chip->reg_mask[1] & 2) != 0
#endif
                ;

            if (rst_b2)
                chip->timer_b_status[0] = 0;
            else
                chip->timer_b_status[0] = chip->timer_b_status[1];

            chip->timer_b_status[0] |= !rst_b
#ifdef FMOPNA_YM2608
                && (chip->reg_mask[1] & 2) == 0
#endif
                && chip->reg_timer_b_enable[1] && chip->timer_b_of[1];

            memcpy(&chip->reg_freq[0][1], &chip->reg_freq[1][0], 5 * sizeof(unsigned short));
            memcpy(&chip->reg_freq_3ch[0][1], &chip->reg_freq_3ch[1][0], 5 * sizeof(unsigned short));
            memcpy(&chip->reg_connect_fb[0][1], &chip->reg_connect_fb[1][0], 5 * sizeof(unsigned char));
            memcpy(&chip->reg_b4[0][1], &chip->reg_b4[1][0], 5 * sizeof(unsigned char));
#ifdef FMOPNA_YM2608
            memcpy(&chip->reg_rss[0][1], &chip->reg_rss[1][0], 5 * sizeof(unsigned char));
#endif
            memcpy(&chip->op_multi_dt[0][1][0], &chip->op_multi_dt[1][0][0], 11 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_tl[0][1][0], &chip->op_tl[1][0][0], 11 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_ar_ks[0][1][0], &chip->op_ar_ks[1][0][0], 11 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_dr_a[0][1][0], &chip->op_dr_a[1][0][0], 11 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_sr[0][1][0], &chip->op_sr[1][0][0], 11 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_rr_sl[0][1][0], &chip->op_rr_sl[1][0][0], 11 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_ssg[0][1][0], &chip->op_ssg[1][0][0], 11 * 2 * sizeof(unsigned char));

            if (chip->ic)
            {
                chip->reg_a4[0] = 0;
                chip->reg_freq[0][0] = 0;
                chip->reg_ac[0] = 0;
                chip->reg_freq_3ch[0][0] = 0;
                chip->reg_connect_fb[0][0] = 0;
                chip->reg_b4[0][0] = 0xc0;
#ifdef FMOPNA_YM2608
                chip->reg_rss[0][0] = 0;
#endif

                chip->op_multi_dt[0][0][0] = 0;
                chip->op_multi_dt[0][0][1] = 0;
                chip->op_tl[0][0][0] = 0;
                chip->op_tl[0][0][1] = 0;
                chip->op_ar_ks[0][0][0] = 0;
                chip->op_ar_ks[0][0][1] = 0;
                chip->op_dr_a[0][0][0] = 0;
                chip->op_dr_a[0][0][1] = 0;
                chip->op_sr[0][0][0] = 0;
                chip->op_sr[0][0][1] = 0;
                chip->op_rr_sl[0][0][0] = 0;
                chip->op_rr_sl[0][0][1] = 0;
                chip->op_ssg[0][0][0] = 0;
                chip->op_ssg[0][0][1] = 0;
            }
            else
            {
                chip->reg_a4[0] = chip->fm_isa4 ? (chip->fm_data[1] & 0x3f) : chip->reg_a4[1];
                chip->reg_freq[0][0] = chip->fm_isa0 ? (chip->fm_data[1] & 0xff) | (chip->reg_a4[1] << 8) : chip->reg_freq[1][5];
                chip->reg_ac[0] = chip->fm_isac ? (chip->fm_data[1] & 0x3f) : chip->reg_ac[1];
                chip->reg_freq_3ch[0][0] = chip->fm_isa8 ? (chip->fm_data[1] & 0xff) | (chip->reg_ac[1] << 8) : chip->reg_freq_3ch[1][5];
                chip->reg_connect_fb[0][0] = chip->fm_isb0 ? (chip->fm_data[1] & 0x3f) : chip->reg_connect_fb[1][5];
                chip->reg_b4[0][0] = chip->fm_isb4 ? (chip->fm_data[1] & 0xf7) : chip->reg_b4[1][5];
#ifdef FMOPNA_YM2608
                chip->reg_rss[0][0] = chip->rss_18 ? (chip->fm_data[1] & 0xdf) : chip->reg_rss[1][5];
#endif

                int bank = (chip->fm_address[1] & 8) != 0;
                chip->op_multi_dt[0][0][0] = (chip->fm_is30 && !bank) ? (chip->fm_data[1] & 0x7f) : chip->op_multi_dt[1][11][0];
                chip->op_multi_dt[0][0][1] = (chip->fm_is30 && bank) ? (chip->fm_data[1] & 0x7f) : chip->op_multi_dt[1][11][1];
                chip->op_tl[0][0][0] = (chip->fm_is40 && !bank) ? (chip->fm_data[1] & 0x7f) : chip->op_tl[1][11][0];
                chip->op_tl[0][0][1] = (chip->fm_is40 && bank) ? (chip->fm_data[1] & 0x7f) : chip->op_tl[1][11][1];
                chip->op_ar_ks[0][0][0] = (chip->fm_is50 && !bank) ? (chip->fm_data[1] & 0xdf) : chip->op_ar_ks[1][11][0];
                chip->op_ar_ks[0][0][1] = (chip->fm_is50 && bank) ? (chip->fm_data[1] & 0xdf) : chip->op_ar_ks[1][11][1];
                chip->op_dr_a[0][0][0] = (chip->fm_is60 && !bank) ? (chip->fm_data[1] & 0x9f) : chip->op_dr_a[1][11][0];
                chip->op_dr_a[0][0][1] = (chip->fm_is60 && bank) ? (chip->fm_data[1] & 0x9f) : chip->op_dr_a[1][11][1];
                chip->op_sr[0][0][0] = (chip->fm_is70 && !bank) ? (chip->fm_data[1] & 0x1f) : chip->op_sr[1][11][0];
                chip->op_sr[0][0][1] = (chip->fm_is70 && bank) ? (chip->fm_data[1] & 0x1f) : chip->op_sr[1][11][1];
                chip->op_rr_sl[0][0][0] = (chip->fm_is80 && !bank) ? (chip->fm_data[1] & 0xff) : chip->op_rr_sl[1][11][0];
                chip->op_rr_sl[0][0][1] = (chip->fm_is80 && bank) ? (chip->fm_data[1] & 0xff) : chip->op_rr_sl[1][11][1];
                chip->op_ssg[0][0][0] = (chip->fm_is90 && !bank) ? (chip->fm_data[1] & 0xf) : chip->op_ssg[1][11][0];
                chip->op_ssg[0][0][1] = (chip->fm_is90 && bank) ? (chip->fm_data[1] & 0xf) : chip->op_ssg[1][11][1];
            }

            int rst_cc = chip->ic || chip->ch_cnt_sync;
            int cc_of = (chip->ch_cnt1[1] & 2) != 0;

            chip->ch_cnt1[0] = (rst_cc || cc_of) ? 0 : (chip->ch_cnt1[1] + 1) & 3;
            chip->ch_cnt2[0] = rst_cc ? 0 : (chip->ch_cnt2[1] + cc_of) & 7;

            int cc_cnt = (chip->ch_cnt2[1] << 2) | chip->ch_cnt1[1];


            int freq;
            if (chip->ch3_en && cc_cnt == 1)
                freq = chip->reg_freq_3ch[1][5];
            else if (chip->ch3_en && cc_cnt == 9)
                freq = chip->reg_freq_3ch[1][0];
            else if (chip->ch3_en && cc_cnt == 17)
                freq = chip->reg_freq_3ch[1][4];
            else
                freq = chip->reg_freq[1][4];

            chip->fnum[0] = freq & 0x7ff;
            chip->kcode[0] = ((freq >> 11) & 7) << 2;
            if (freq & 0x400)
            {
                chip->kcode[0] |= 2;
                if ((freq & 0x380) != 0)
                    chip->kcode[0] |= 1;
            }
            else
            {
                if ((freq & 0x380) == 0x380)
                    chip->kcode[0] |= 1;
            }

            chip->fnum[2] = chip->fnum[1];
            chip->kcode[2] = chip->kcode[1];
        }
        if (chip->clk2)
        {
            chip->fm_address[1] = chip->fm_address[0];
            chip->write_fm_address[1] = chip->write_fm_address[0];
            chip->fm_data[1] = chip->fm_data[0];
            chip->write_fm_data[1] = chip->write_fm_data[0];

            chip->addr_21[1] = chip->addr_21[0];
            chip->addr_22[1] = chip->addr_22[0];
            chip->addr_24[1] = chip->addr_24[0];
            chip->addr_25[1] = chip->addr_25[0];
            chip->addr_26[1] = chip->addr_26[0];
            chip->addr_27[1] = chip->addr_27[0];
            chip->addr_28[1] = chip->addr_28[0];
#ifdef FMOPNA_YM2608
            chip->addr_10[1] = chip->addr_10[0];
            chip->addr_10h[1] = chip->addr_10h[0];
            chip->addr_12[1] = chip->addr_12[0];
            chip->addr_29[1] = chip->addr_29[0];
            chip->addr_ff[1] = chip->addr_ff[0];
#endif
#ifdef FMOPNA_YM2610
            chip->addr_00[1] = chip->addr_00[0];
            chip->addr_02[1] = chip->addr_02[0];
            chip->addr_1c[1] = chip->addr_1c[0];
#endif
#ifdef FMOPNA_YM2612
            chip->addr_2a[1] = chip->addr_2a[0];
            chip->addr_2b[1] = chip->addr_2b[0];
            chip->addr_2c[1] = chip->addr_2c[0];
#endif

#ifdef FMOPNA_YM2608
            chip->reg_mask[1] = chip->reg_mask[0];
            chip->reg_sch[1] = chip->reg_sch[0];
            chip->reg_irq[1] = chip->reg_irq[0];
            chip->reg_test_12[1] = chip->reg_test_12[0];
#endif
#ifdef FMOPNA_YM2610
            chip->reg_flags[1] = chip->reg_flags[0];
            chip->reg_test_12[1] = chip->reg_test_12[0];
#endif
#ifdef FMOPNA_YM2612
            chip->reg_dac_en[1] = chip->reg_dac_en[0];
            chip->reg_dac_data[1] = chip->reg_dac_data[0];
            chip->reg_test_2c[1] = chip->reg_test_2c[0];
#endif
            chip->reg_test_21[1] = chip->reg_test_21[0];
            chip->reg_lfo[1] = chip->reg_lfo[0];
            chip->reg_timer_a[1] = chip->reg_timer_a[0];
            chip->reg_timer_b[1] = chip->reg_timer_b[0];
            chip->reg_ch3[1] = chip->reg_ch3[0];
            chip->reg_timer_a_load[1] = chip->reg_timer_a_load[0];
            chip->reg_timer_b_load[1] = chip->reg_timer_b_load[0];
            chip->reg_timer_a_enable[1] = chip->reg_timer_a_enable[0];
            chip->reg_timer_b_enable[1] = chip->reg_timer_b_enable[0];
            chip->reg_timer_a_reset[1] = chip->reg_timer_a_reset[0];
            chip->reg_timer_b_reset[1] = chip->reg_timer_b_reset[0];
            chip->reg_kon_operator[1] = chip->reg_kon_operator[0];
            chip->reg_kon_channel[1] = chip->reg_kon_channel[0];

            int op_match = chip->write_fm_data[0] && (chip->reg_cnt1[0] == (chip->fm_address[0] & 3))
                && (chip->reg_cnt2[0] & 1) == ((chip->fm_address[0] >> 8) & 1)
                && ((chip->reg_cnt2[0] >> 1) & 1) == ((chip->fm_address[0] >> 2) & 1);
            int ch_match = chip->write_fm_data[0] && (chip->reg_cnt1[0] == (chip->fm_address[0] & 3))
                && (chip->reg_cnt2[0] & 1) == ((chip->fm_address[0] >> 8) & 1);

            chip->fm_is30 = op_match && (chip->fm_address[0] & 0xf0) == 0x30;
            chip->fm_is40 = op_match && (chip->fm_address[0] & 0xf0) == 0x40;
            chip->fm_is50 = op_match && (chip->fm_address[0] & 0xf0) == 0x50;
            chip->fm_is60 = op_match && (chip->fm_address[0] & 0xf0) == 0x60;
            chip->fm_is70 = op_match && (chip->fm_address[0] & 0xf0) == 0x70;
            chip->fm_is80 = op_match && (chip->fm_address[0] & 0xf0) == 0x80;
            chip->fm_is90 = op_match && (chip->fm_address[0] & 0xf0) == 0x90;
            chip->fm_isa0 = ch_match && (chip->fm_address[0] & 0xfc) == 0xa0;
            chip->fm_isa4 = ch_match && (chip->fm_address[0] & 0xfc) == 0xa4;
            chip->fm_isa8 = ch_match && (chip->fm_address[0] & 0xfc) == 0xa8;
            chip->fm_isac = ch_match && (chip->fm_address[0] & 0xfc) == 0xac;
            chip->fm_isb0 = ch_match && (chip->fm_address[0] & 0xfc) == 0xb0;
            chip->fm_isb4 = ch_match && (chip->fm_address[0] & 0xfc) == 0xb4;

            chip->reg_cnt1[1] = chip->reg_cnt1[0];
            chip->reg_cnt2[1] = chip->reg_cnt2[0];
            chip->reg_cnt_sync = chip->fsm_sel23[1];

#ifdef FMOPNA_YM2608
            chip->reg_cnt_rss[1] = chip->reg_cnt_rss[0];

            chip->reg_cnt_rss_of = (chip->reg_cnt_rss[0] & 11) == 11;

            chip->rss_18 = chip->write_fm_data[0]
                && chip->reg_cnt_rss[0] == (chip->fm_address[0] & 7) && (chip->fm_address[0] & 6) != 6
                && (chip->fm_address[0] & 0x1f8) == 0x18;
#endif

            chip->reg_kon_match = chip->reg_key_cnt1[0] == (chip->reg_kon_channel[0] & 3)
                && chip->reg_key_cnt2[0] == (((chip->reg_kon_channel[0] >> 2) & 1)
#ifdef FMOPNA_YM2608
                    && chip->reg_sch[0]
#endif
                    )
#ifdef FMOPNA_YM2610
                // tildearrow: changed to allow YM2610B emulation
                && (chip->ym2610b || (chip->reg_kon_channel[0] & 3) != 0) || chip->input.ym2610b)
#endif
                ;

            chip->reg_key_cnt1[1] = chip->reg_key_cnt1[0];
            chip->reg_key_cnt2[1] = chip->reg_key_cnt2[0];

            chip->reg_kon[0][1] = chip->reg_kon[0][0];
            chip->reg_kon[1][1] = chip->reg_kon[1][0];
            chip->reg_kon[2][1] = chip->reg_kon[2][0];
            chip->reg_kon[3][1] = chip->reg_kon[3][0];

            chip->reg_ch3_sel = chip->fsm_sel_ch3[1];

            chip->ch3_en = chip->reg_ch3[0] != 0;
            chip->ch3_csm = chip->reg_ch3[0] == 2;

            chip->timer_a_cnt[1] = chip->timer_a_cnt[0];
            chip->timer_a_of[1] = chip->timer_a_of[0];

            chip->reg_sync_timer = chip->fsm_sel1[1];
            chip->reg_sync_timer_l[1] = chip->reg_sync_timer_l[0];

            chip->timer_a_reg_load_l[1] = chip->timer_a_reg_load_l[0];

            chip->reg_sync_timer_load = chip->reg_sync_timer_l[0] && chip->reg_sync_timer_l[1];
            if (chip->reg_sync_timer_load)
            {
                chip->timer_a_reg_load = chip->reg_timer_a_load[1];
                chip->timer_b_reg_load = chip->reg_timer_b_load[1];
            }

            chip->timer_a_load = chip->timer_a_of[1] || (!chip->timer_a_reg_load_l[1] && chip->timer_a_reg_load);

            if (chip->reg_sync_timer_load)
            {
                chip->ch3_csm_load = chip->ch3_csm && chip->timer_a_load;
            }

            chip->kon_comb = (chip->reg_kon[3][1] >> 5) & 1;
            chip->kon_comb |= chip->reg_ch3_sel && chip->ch3_csm_load;

            chip->timer_a_status[1] = chip->timer_a_status[0];

            chip->timer_b_subcnt[1] = chip->timer_b_subcnt[0];
            chip->timer_b_subcnt_of[1] = chip->timer_b_subcnt_of[0];

            chip->timer_b_cnt[1] = chip->timer_b_cnt[0];
            chip->timer_b_of[1] = chip->timer_b_of[0];

            chip->timer_b_reg_load_l[1] = chip->timer_b_reg_load_l[0];

            chip->timer_b_load = chip->timer_b_of[1] || (!chip->timer_b_reg_load_l[1] && chip->timer_b_reg_load);

            chip->timer_b_status[1] = chip->timer_b_status[0];

#ifndef FMOPNA_YM2612
            chip->irq_eos_l = chip->eos_repeat;
#endif


            memcpy(&chip->reg_freq[1][0], &chip->reg_freq[0][0], 6 * sizeof(unsigned short));
            memcpy(&chip->reg_freq_3ch[1][0], &chip->reg_freq_3ch[0][0], 6 * sizeof(unsigned short));
            memcpy(&chip->reg_connect_fb[1][0], &chip->reg_connect_fb[0][0], 6 * sizeof(unsigned char));
            memcpy(&chip->reg_b4[1][0], &chip->reg_b4[0][0], 6 * sizeof(unsigned char));
#ifdef FMOPNA_YM2608
            memcpy(&chip->reg_rss[1][0], &chip->reg_rss[0][0], 6 * sizeof(unsigned char));
#endif
            memcpy(&chip->op_multi_dt[1][0][0], &chip->op_multi_dt[0][0][0], 12 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_tl[1][0][0], &chip->op_tl[0][0][0], 12 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_ar_ks[1][0][0], &chip->op_ar_ks[0][0][0], 12 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_dr_a[1][0][0], &chip->op_dr_a[0][0][0], 12 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_sr[1][0][0], &chip->op_sr[0][0][0], 12 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_rr_sl[1][0][0], &chip->op_rr_sl[0][0][0], 12 * 2 * sizeof(unsigned char));
            memcpy(&chip->op_ssg[1][0][0], &chip->op_ssg[0][0][0], 12 * 2 * sizeof(unsigned char));

            chip->reg_a4[1] = chip->reg_a4[0];
            chip->reg_ac[1] = chip->reg_ac[0];

            chip->ch_cnt_sync = chip->fsm_sel23[1];

            chip->ch_cnt1[1] = chip->ch_cnt1[0];
            chip->ch_cnt2[1] = chip->ch_cnt2[0];

            chip->fnum[1] = chip->fnum[0];
            chip->fnum[3] = chip->fnum[2];
            chip->kcode[1] = chip->kcode[0];
            chip->kcode[3] = chip->kcode[2];
        }

#ifdef FMOPNA_YM2608
        {
            int write10 = chip->addr_10h[1] && (chip->data_bus1 & 0x100) != 0 && chip->write1_en;
            int irq_rst = write10 && (chip->data_bus2 & 0x80) == 0;
            chip->irq_mask_eos = (chip->reg_mask[1] & 4) != 0 || chip->irq_eos_l || irq_rst;
            chip->irq_mask_brdy = (chip->reg_mask[1] & 8) != 0 || irq_rst;
            chip->irq_mask_zero = (chip->reg_mask[1] & 16) != 0 || irq_rst;
        }
#endif

    }

    {
        if (chip->clk1)
        {
            int fsm_cnt;
            int of1 = (chip->fsm_cnt1[1] & 2) != 0;
            chip->fsm_cnt1[0] = (chip->ic_check3 || of1) ? 0 : (chip->fsm_cnt1[1] + 1) & 3;
            chip->fsm_cnt2[0] = chip->ic_check3 ? 0 : (chip->fsm_cnt2[1] + of1) & 7;

            fsm_cnt = (chip->fsm_cnt2[0] << 2) | chip->fsm_cnt1[0];

            chip->fsm_out[0] = (fsm_cnt & 30) == 30;
            chip->fsm_out[1] = (fsm_cnt & 28) == 0;
            chip->fsm_out[2] = (fsm_cnt & 30) == 4;
            chip->fsm_out[3] = (fsm_cnt & 30) == 22;
            chip->fsm_out[4] = (fsm_cnt & 28) == 24;
            chip->fsm_out[5] = (fsm_cnt & 30) == 28;
            chip->fsm_out[6] = (fsm_cnt & 30) == 14;
            chip->fsm_out[7] = (fsm_cnt & 28) == 16;
            chip->fsm_out[8] = (fsm_cnt & 30) == 20;
            chip->fsm_out[9] = (fsm_cnt & 30) == 6;
            chip->fsm_out[10] = (fsm_cnt & 28) == 8;
            chip->fsm_out[11] = (fsm_cnt & 30) == 12;
            chip->fsm_out[12] = (fsm_cnt & 30) == 30;
            chip->fsm_out[13] = fsm_cnt == 0;
            chip->fsm_out[14] = fsm_cnt == 1;
#ifndef FMOPNA_YM2612
            chip->fsm_out[15] = fsm_cnt == 13;
#endif
            chip->fsm_out[16] = fsm_cnt == 29;
            chip->fsm_out[17] = (fsm_cnt & 7) == 1;
            chip->fsm_out[18] = (fsm_cnt & 28) == 4;
            chip->fsm_out[19] = fsm_cnt == 8;
#ifndef FMOPNA_YM2612
            chip->fsm_out[20] = (fsm_cnt & 28) == 20;
            chip->fsm_out[21] = fsm_cnt == 24;
#endif
#ifdef FMOPNA_YM2612
            chip->fsm_out[20] = (fsm_cnt & 15) == 14;
            chip->fsm_out[21] = (fsm_cnt & 15) == 4;
            chip->fsm_out[22] = (fsm_cnt & 15) == 9;
            chip->fsm_out[23] = fsm_cnt == 14;
            chip->fsm_out[24] = (fsm_cnt & 24) == 16;
            chip->fsm_out[25] = (fsm_cnt & 28) == 24;
            chip->fsm_out[26] = (fsm_cnt & 30) == 28;
#endif

            chip->alg_mod_op1_0_l = 0;
            chip->alg_mod_op1_1_l = 0;
            chip->alg_mod_op2_l = 0;
            chip->alg_mod_prev_0_l = 0;
            chip->alg_mod_prev_1_l = 0;
            chip->alg_output_l = 0;

            if (chip->fsm_op2_sel_l)
            {
                chip->alg_mod_op1_0_l |= fm_algorithm[0][0][chip->fsm_connect];
                chip->alg_mod_op1_1_l |= fm_algorithm[0][1][chip->fsm_connect];
                chip->alg_mod_op2_l |= fm_algorithm[0][2][chip->fsm_connect];
                chip->alg_mod_prev_0_l |= fm_algorithm[0][3][chip->fsm_connect];
                chip->alg_mod_prev_1_l |= fm_algorithm[0][4][chip->fsm_connect];
                chip->alg_output_l |= fm_algorithm[2][5][chip->fsm_connect];
            }
            if (chip->fsm_op4_sel_l)
            {
                chip->alg_mod_op1_0_l |= fm_algorithm[1][0][chip->fsm_connect];
                chip->alg_mod_op1_1_l |= fm_algorithm[1][1][chip->fsm_connect];
                chip->alg_mod_op2_l |= fm_algorithm[1][2][chip->fsm_connect];
                chip->alg_mod_prev_0_l |= fm_algorithm[1][3][chip->fsm_connect];
                chip->alg_mod_prev_1_l |= fm_algorithm[1][4][chip->fsm_connect];
                chip->alg_output_l |= fm_algorithm[3][5][chip->fsm_connect];
            }
            if (chip->fsm_op1_sel_l)
            {
                chip->alg_mod_op1_0_l |= fm_algorithm[2][0][chip->fsm_connect];
                chip->alg_mod_op1_1_l |= fm_algorithm[2][1][chip->fsm_connect];
                chip->alg_mod_op2_l |= fm_algorithm[2][2][chip->fsm_connect];
                chip->alg_mod_prev_0_l |= fm_algorithm[2][3][chip->fsm_connect];
                chip->alg_mod_prev_1_l |= fm_algorithm[2][4][chip->fsm_connect];
                chip->alg_output_l |= fm_algorithm[0][5][chip->fsm_connect];
            }
            if (chip->fsm_op3_sel_l)
            {
                chip->alg_mod_op1_0_l |= fm_algorithm[3][0][chip->fsm_connect];
                chip->alg_mod_op1_1_l |= fm_algorithm[3][1][chip->fsm_connect];
                chip->alg_mod_op2_l |= fm_algorithm[3][2][chip->fsm_connect];
                chip->alg_mod_prev_0_l |= fm_algorithm[3][3][chip->fsm_connect];
                chip->alg_mod_prev_1_l |= fm_algorithm[3][4][chip->fsm_connect];
                chip->alg_output_l |= fm_algorithm[1][5][chip->fsm_connect];
            }

            chip->alg_do_fb[1] = chip->alg_do_fb[0];

            chip->alg_load_fb = chip->fsm_op1_sel_l;

            chip->fsm_sel0[1] = chip->fsm_sel0[0];
            chip->fsm_sel1[1] = chip->fsm_sel1[0];
            chip->fsm_sel2[1] = chip->fsm_sel2[0];
            chip->fsm_sel23[1] = chip->fsm_sel23[0];
            chip->fsm_sel_ch3[1] = chip->fsm_sel_ch3[0];
#ifndef FMOPNA_YM2612
            chip->fsm_sel11[1] = chip->fsm_sel11[0];
            chip->fsm_sh1[1] = chip->fsm_sh1[0];
            chip->fsm_sh2[1] = chip->fsm_sh2[0];

            chip->fsm_rss2 = !chip->fsm_out[16] && !chip->fsm_sel23[1];
#endif
#ifdef FMOPNA_YM2612
            chip->fsm_dac_ch6[1] = chip->fsm_dac_ch6[0];
            chip->fsm_dac_load[1] = chip->fsm_dac_load[0];
            chip->fsm_dac_out_sel[1] = chip->fsm_dac_out_sel[0];
#endif
        }
        if (chip->clk2)
        {
            chip->fsm_cnt1[1] = chip->fsm_cnt1[0];
            chip->fsm_cnt2[1] = chip->fsm_cnt2[0];

            chip->fsm_op4_sel_l = chip->fsm_out[0] || chip->fsm_out[1] || chip->fsm_out[2];
            chip->fsm_op2_sel_l = chip->fsm_out[3] || chip->fsm_out[4] || chip->fsm_out[5];
            chip->fsm_op3_sel_l = chip->fsm_out[6] || chip->fsm_out[7] || chip->fsm_out[8];
            chip->fsm_op1_sel_l = chip->fsm_out[9] || chip->fsm_out[10] || chip->fsm_out[11];

            chip->fsm_connect = chip->reg_connect_fb[0][4] & 7;

            chip->alg_do_fb[0] = chip->alg_mod_op1_1_l;

            chip->fsm_sel0[0] = chip->fsm_out[12];
            chip->fsm_sel1[0] = chip->fsm_out[13];
            chip->fsm_sel2[0] = chip->fsm_out[14];
            chip->fsm_sel23[0] = chip->fsm_out[16];
            chip->fsm_sel_ch3[0] = chip->fsm_out[17];
#ifndef FMOPNA_YM2612
            chip->fsm_sel11[0] = chip->fsm_out[15];
            chip->fsm_sh1[0] = chip->fsm_out[18] || chip->fsm_out[19];
            chip->fsm_sh2[0] = chip->fsm_out[20] || chip->fsm_out[21];

            chip->fsm_rss = (chip->fsm_cnt2[0] & 2) != 0;
#endif
#ifdef FMOPNA_YM2612
            chip->fsm_dac_ch6[0] = chip->fsm_out[18] || chip->fsm_out[19];
            chip->fsm_dac_load[0] = chip->fsm_out[20] || chip->fsm_out[21] || chip->fsm_out[22];
            chip->fsm_dac_out_sel[0] = chip->fsm_out[23] || chip->fsm_out[24] || chip->fsm_out[25] || chip->fsm_out[26];
#endif
        }
    }

    {
        if (chip->clk1)
        {
            int inc = chip->lfo_sync[0]
#ifndef FMOPNA_YM2610
                || (chip->reg_test_21[1] & 2) != 0
#endif
                ;
            int subcnt_rst = chip->ic || chip->lfo_subcnt_of;

            chip->lfo_subcnt[0] = subcnt_rst ? 0 : (chip->lfo_subcnt[1] + inc) & 127;

            int cnt = chip->lfo_cnt[1] + chip->lfo_subcnt_of;

#ifdef FMOPNA_YM2608
            chip->lfo_cnt_of = (cnt & 128) != 0 && chip->lfo_mode;
#endif
            chip->lfo_cnt[0] = chip->lfo_cnt_rst ? 0 : cnt & 127;

            chip->lfo_sync[1] = chip->lfo_sync[0];
            chip->lfo_sync[3] = chip->lfo_sync[2];

            // LFO shift
            static const int pg_lfo_sh1[8][8] = {
                { 7, 7, 7, 7, 7, 7, 7, 7 },
                { 7, 7, 7, 7, 7, 7, 7, 7 },
                { 7, 7, 7, 7, 7, 7, 1, 1 },
                { 7, 7, 7, 7, 1, 1, 1, 1 },
                { 7, 7, 7, 1, 1, 1, 1, 0 },
                { 7, 7, 1, 1, 0, 0, 0, 0 },
                { 7, 7, 1, 1, 0, 0, 0, 0 },
                { 7, 7, 1, 1, 0, 0, 0, 0 }
            };

#if 0
            // YM2608/YM2610 decap, doesn't match YM2608 nor YM2610 hardware tests though
            static const int pg_lfo_sh2[8][8] = {
                { 7, 7, 7, 7, 7, 7, 7, 7 },
                { 7, 7, 7, 7, 2, 2, 2, 2 },
                { 7, 7, 7, 2, 2, 2, 7, 7 },
                { 7, 7, 2, 2, 7, 7, 2, 2 },
                { 7, 2, 2, 7, 7, 2, 2, 7 },
                { 7, 2, 7, 2, 7, 2, 2, 1 },
                { 7, 2, 7, 2, 7, 2, 2, 1 },
                { 7, 2, 7, 2, 7, 2, 2, 1 }
            };
#endif
#if 1
            // YM2612/YM3438 decap, matches to YM2608 and YM2610 hardware tests O_O
            static const int pg_lfo_sh2[8][8] = {
                { 7, 7, 7, 7, 7, 7, 7, 7 },
                { 7, 7, 7, 7, 2, 2, 2, 2 },
                { 7, 7, 7, 2, 2, 2, 7, 7 },
                { 7, 7, 2, 2, 7, 7, 2, 2 },
                { 7, 7, 2, 7, 7, 7, 2, 7 },
                { 7, 7, 7, 2, 7, 7, 2, 1 },
                { 7, 7, 7, 2, 7, 7, 2, 1 },
                { 7, 7, 7, 2, 7, 7, 2, 1 }
            };
#endif
            int pms = chip->reg_b4[1][5] & 7;
            int lfo = (chip->lfo_cnt_load >> 2) & 7;
            if (chip->lfo_cnt_load & 32)
                lfo ^= 7;
            int fnum_h = chip->fnum[1] >> 4;

            chip->lfo_fnum1 = fnum_h >> pg_lfo_sh1[pms][lfo];
            chip->lfo_fnum2 = fnum_h >> pg_lfo_sh2[pms][lfo];

            chip->lfo_shift = 2;
            if (pms > 5)
                chip->lfo_shift = 7 - pms;

            chip->lfo_sign = (chip->lfo_cnt_load >> 6) & 1;

            chip->lfo_fnum = ((chip->fnum[3] << 1) + chip->lfo_pm) & 0xfff;
        }
        if (chip->clk2)
        {
            static const int lfo_cycles[8] = {
                108, 77, 71, 67, 62, 44, 8, 5
            };
            chip->lfo_sync[0] = chip->fsm_sel23[1];
            chip->lfo_sync[2] = chip->lfo_sync[1];

            chip->lfo_subcnt[1] = chip->lfo_subcnt[0];

            int of = (chip->lfo_subcnt[0] & lfo_cycles[chip->reg_lfo[0] & 7]) == lfo_cycles[chip->reg_lfo[0] & 7];

#ifdef FMOPNA_YM2608
            chip->lfo_mode = chip->ad_sample_l[2] || (chip->ad_reg_rec && chip->ad_start_l[1]);
            chip->lfo_subcnt_of = chip->lfo_mode ? (chip->lfo_subcnt[0] & 127) == 127 : of;
            chip->lfo_cnt_rst = chip->lfo_mode ? chip->ad_ad_quiet : (chip->reg_lfo[0] & 8) == 0;
#else
            chip->lfo_subcnt_of = of;
            chip->lfo_cnt_rst = (chip->reg_lfo[0] & 8) == 0;
#endif

            chip->lfo_cnt[1] = chip->lfo_cnt[0];

            if (!chip->lfo_sync[3] && chip->lfo_sync[2])
            {
                chip->lfo_cnt_load = chip->lfo_cnt[1];
            }

            if (chip->lfo_cnt_load & 64)
                chip->lfo_am = chip->lfo_cnt_load & 63;
            else
                chip->lfo_am = (chip->lfo_cnt_load & 63) ^ 63;

            chip->lfo_pm = (chip->lfo_fnum1 + chip->lfo_fnum2) >> chip->lfo_shift;
            if (chip->lfo_sign)
                chip->lfo_pm = -chip->lfo_pm;
        }
    }


    {
        if (chip->clk1)
        {
            chip->pg_block = chip->kcode[3] >> 2;

            chip->pg_dt_multi = (chip->reg_key_cnt2[1] & 4) == 0 ? chip->op_multi_dt[1][11][0]
                : chip->op_multi_dt[1][11][1];

            chip->dt_note[1] = chip->dt_note[0];
            chip->dt_blockmax[1] = chip->dt_blockmax[0];

            chip->dt_enable[1] = chip->dt_enable[0];

            chip->dt_sign[1] = chip->dt_sign[0];

            chip->dt_sum = chip->dt_add1 + chip->dt_add2 + 1;

            chip->pg_freqdt[0] = (chip->pg_freq + chip->pg_dt_add) & 0x1ffff;

            chip->pg_multi[1] = chip->pg_multi[0];
            chip->pg_multi[3] = chip->pg_multi[2];

            chip->pg_add[0] = chip->pg_multi[4] ? chip->pg_freqdt[1] * chip->pg_multi[4] :
                (chip->pg_freqdt[1] >> 1);
            chip->pg_add[2] = chip->pg_add[1];
            chip->pg_add[4] = chip->pg_add[3];

            chip->pg_reset[1] = chip->pg_reset[0];
            chip->pg_reset[3] = chip->pg_reset[2] || (chip->reg_test_21[1] & 8) != 0;

            memcpy(&chip->pg_phase[0][1], &chip->pg_phase[1][0], 22 * sizeof(int));

            chip->pg_phase2[0] = chip->pg_phase[1][22];

            chip->pg_phase[0][0] = (chip->pg_phase2[1] + chip->pg_add[5]) & 0xfffff;

            chip->pg_dbg[0] = chip->pg_dbg[1] >> 1;
            if (chip->pg_dbgsync)
                chip->pg_dbg[0] |= chip->pg_phase[1][22] & 1023;
#ifdef FMOPNA_YM2610
            chip->pg_dbgsync_l[1] = chip->pg_dbgsync_l[0];
#endif

        }
        if (chip->clk2)
        {
            chip->pg_freq = (chip->lfo_fnum << chip->pg_block) >> 2;

            chip->dt_note[0] = chip->kcode[2] & 3;
            chip->dt_blockmax[0] = (chip->kcode[2] & 28) == 28;
            chip->dt_add1 = (chip->kcode[2] >> 2) & 7;
            if ((chip->pg_dt_multi & 0x30) != 0)
                chip->dt_add1 |= 8;
            chip->dt_add2 = 0;
            if ((chip->pg_dt_multi & 0x30) == 0x30)
                chip->dt_add2 |= 1;
            if (chip->pg_dt_multi & 0x20)
                chip->dt_add2 |= 2;

            chip->dt_enable[0] = (chip->pg_dt_multi & 0x30) != 0;

            chip->dt_sign[0] = (chip->pg_dt_multi & 0x40) != 0;

            int dt_l = (chip->dt_sum & 1) << 2;
            if (!chip->dt_blockmax[1])
                dt_l |= chip->dt_note[1];
            int dt_h = chip->dt_sum >> 1;

            static const int pg_detune[8] = { 16, 17, 19, 20, 22, 24, 27, 29 };

            int dt_freq = pg_detune[dt_l] >> (9 - dt_h);

            if (chip->dt_sign[1])
                dt_freq = -dt_freq;

            chip->pg_dt_add = dt_freq;

            chip->pg_multi[0] = chip->pg_dt_multi & 15;
            chip->pg_multi[2] = chip->pg_multi[1];
            chip->pg_multi[4] = chip->pg_multi[3];

            chip->pg_freqdt[1] = chip->pg_freqdt[0];

            chip->pg_add[1] = chip->pg_add[0];
            chip->pg_add[3] = chip->pg_reset[1] ? 0 : chip->pg_add[2];
            chip->pg_add[5] = chip->pg_add[4];

            chip->pg_reset[0] = (chip->eg_pg_reset[0] & 2) != 0;
            chip->pg_reset[2] = chip->pg_reset[1];
            memcpy(&chip->pg_phase[1][0], &chip->pg_phase[0][0], 23 * sizeof(int));

            chip->pg_out = chip->pg_phase[1][18] >> 10;

            chip->pg_phase2[1] = chip->pg_reset[3] ? 0 : chip->pg_phase2[0];

#ifdef FMOPNA_YM2610
            chip->pg_dbgsync_l[0] = chip->fsm_sel2[1];
            chip->pg_dbgsync = chip->pg_dbgsync_l[1];
#else
            chip->pg_dbgsync = chip->fsm_sel2[1];
#endif

            chip->pg_dbg[1] = chip->pg_dbg[0];
        }
    }

    {
        if (chip->clk1)
        {
            chip->eg_prescaler_clock_l[0] = chip->eg_sync;
            chip->eg_prescaler[0] = (chip->eg_prescaler[1] + chip->eg_sync) & 3;
            if (((chip->eg_prescaler[1] & 2) != 0 && chip->eg_sync) || chip->ic)
                chip->eg_prescaler[0] = 0;
            chip->eg_step[0] = chip->eg_prescaler[1] >> 1;
            chip->eg_step[2] = chip->eg_step[1];
            chip->eg_timer_step[1] = chip->eg_timer_step[0];

            chip->eg_ic[0] = chip->ic;

            chip->eg_clock_delay[0] = (chip->eg_clock_delay[1] << 1) | chip->eg_prescaler_clock_l[1];


            int sum = (chip->eg_timer[1] >> 10) & 1;
            int add = chip->eg_timer_carry[1];
            if ((chip->eg_prescaler[1] & 2) != 0 && chip->eg_prescaler_clock_l[1])
                add = 1;
            sum += add;

            chip->eg_timer[0] = (chip->eg_timer[1] << 1) | chip->eg_timer_sum[1];

            chip->eg_timer_carry[0] = sum >> 1;
            chip->eg_timer_sum[0] = sum & 1;
            chip->eg_timer_test = (chip->reg_test_21[1] & 32) != 0;
            chip->eg_timer_test_bit[0] = chip->input.test;

            int timer_bit = chip->eg_timer_sum[1] ||
                (!chip->eg_timer_test_bit[1]
#ifdef FMOPNA_YM2612
                    && (chip->reg_test_2c[1] & 64) != 0
#endif
                    );

            chip->eg_timer_mask[0] = timer_bit | chip->eg_timer_mask[1];
            if (chip->eg_prescaler_clock_l[1] || ((chip->eg_clock_delay[1] >> 11) & 1) != 0 || chip->eg_ic[1])
                chip->eg_timer_mask[0] = 0;

            int timer_bit_masked = timer_bit && !chip->eg_timer_mask[1];

            chip->eg_timer_masked[0] = (chip->eg_timer_masked[1] << 1) | timer_bit_masked;

            if (chip->eg_timer_step[0] && chip->eg_timer_step[1])
            {
                int b0, b1, b2, b3;
                b0 = (chip->eg_timer[0] >> 11) & 1;
                b1 = (chip->eg_timer[0] >> 10) & 1;
                chip->eg_timer_low_lock = b1 * 2 + b0;

                b0 = (chip->eg_timer_masked[0] & 0xaaa) != 0;
                b1 = (chip->eg_timer_masked[0] & 0x666) != 0;
                b2 = (chip->eg_timer_masked[0] & 0x1e1) != 0;
                b3 = (chip->eg_timer_masked[0] & 0x1f) != 0;
                chip->eg_shift_lock = b3 * 8 + b2 * 4 + b1 * 2 + b0;
            }

            int bank = (chip->reg_key_cnt2[1] & 4) != 0;

            chip->eg_rate_ar = chip->op_ar_ks[1][11][bank] & 0x1f;
            chip->eg_ks = (chip->op_ar_ks[1][11][bank] >> 6) & 3;
            chip->eg_rate_dr = chip->op_dr_a[1][11][bank] & 0x1f;
            chip->eg_rate_sr = chip->op_sr[1][11][bank] & 0x1f;
            chip->eg_rate_rr = chip->op_rr_sl[1][11][bank] & 0xf;

            chip->eg_rate_nonzero[1] = chip->eg_rate_nonzero[0];

            chip->eg_rate2 = (chip->eg_rate << 1) + chip->eg_ksv;

            chip->eg_maxrate[1] = chip->eg_maxrate[0];

            int inc1 = 0;
            if (chip->eg_rate_slow && chip->eg_rate_nonzero[2])
            {
                switch (chip->eg_rate_sum)
                {
                case 12:
                    inc1 = chip->eg_ratenz;
                    break;
                case 13:
                    inc1 = (chip->eg_rate_low >> 1) & 1;
                    break;
                case 14:
                    inc1 = chip->eg_rate_low & 1;
                    break;
                }
            }

            chip->eg_incsh0[0] = inc1;
            chip->eg_incsh3[0] = chip->eg_rate15;
            if (!chip->eg_inc2)
            {
                chip->eg_incsh0[0] |= chip->eg_rate12;
                chip->eg_incsh1[0] = chip->eg_rate13;
                chip->eg_incsh2[0] = chip->eg_rate14;
            }
            else
            {
                chip->eg_incsh1[0] = chip->eg_rate12;
                chip->eg_incsh2[0] = chip->eg_rate13;
                chip->eg_incsh3[0] |= chip->eg_rate14;
            }

            chip->eg_kon_latch[0] = (chip->eg_kon_latch[1] << 1) | chip->kon_comb;
            int csm_kon = chip->reg_ch3_sel && chip->ch3_csm_load;
            chip->eg_kon_csm[0] = (chip->eg_kon_csm[1] << 1) | csm_kon;

            int kon = (chip->eg_kon_latch[1] >> 1) & 1;
            int okon = (chip->eg_key[1] >> 23) & 1;
            int pg_reset = (kon && !okon) || (chip->eg_ssg_pgreset[1] & 2) != 0;
            chip->eg_pg_reset[0] = (chip->eg_pg_reset[1] << 1) | pg_reset;
            chip->eg_kon_event = (kon && !okon) || (okon && (chip->eg_ssg_egrepeat[1] & 2) != 0);

            chip->eg_key[0] = (chip->eg_key[1] << 1) | kon;

            int okon2 = (chip->eg_key[1] >> 21) & 1;


            chip->eg_ssg_sign[0] = (chip->eg_level[1][18] & 0x200) != 0;

            int ssg_eg = chip->op_ssg[1][11][bank] & 15;
            int ssg_enable = (ssg_eg & 8) != 0;
            chip->eg_ssg_enable[0] = (chip->eg_ssg_enable[1] << 1) | ssg_enable;
            int ssg_inv_e = ssg_enable && (ssg_eg & 4) != 0;
            int ssg_holdup = ssg_enable && ((ssg_eg & 7) == 3 || (ssg_eg & 7) == 5) && chip->kon_comb;
            chip->eg_ssg_holdup[0] = (chip->eg_ssg_holdup[1] << 1) | ssg_holdup;
            int ssg_pgreset = ssg_enable && chip->eg_ssg_sign[1] && (ssg_eg & 3) == 0;
            chip->eg_ssg_pgreset[0] = (chip->eg_ssg_pgreset[1] << 1) | ssg_pgreset;
            int ssg_egrepeat = ssg_enable && chip->eg_ssg_sign[1] && (ssg_eg & 1) == 0;
            chip->eg_ssg_egrepeat[0] = (chip->eg_ssg_egrepeat[1] << 1) | ssg_egrepeat;

            chip->eg_rate_sel = (okon2 ? ssg_egrepeat : chip->kon_comb) ? eg_state_attack : chip->eg_state[1][20];

            int ssg_odir = (chip->eg_ssg_dir[1] >> 23) & 1;
            int ssg_dir = ssg_enable && okon2 &&
                ((ssg_odir ^ ((ssg_eg & 3) == 2 && chip->eg_ssg_sign[1])) || ((ssg_eg & 3) == 3 && chip->eg_ssg_sign[1]));
            chip->eg_ssg_dir[0] = (chip->eg_ssg_dir[1] << 1) | ssg_dir;

            int ssg_inv = okon2 && (ssg_odir ^ ssg_inv_e);

            chip->eg_ssg_inv = ssg_inv;

            chip->eg_level_ssg[0] = chip->eg_output;
            int eg_output = chip->eg_output;

            if (chip->reg_test_21[1] & 32)
                eg_output = 0;

            int sl = (chip->op_rr_sl[1][11][bank] >> 4) & 15;

            if (sl == 15)
                sl |= 16;

            chip->eg_sl[0][0] = sl;
            chip->eg_sl[1][0] = chip->eg_sl[0][1];
            chip->eg_tl[0][0] = chip->op_tl[1][11][bank];
            chip->eg_tl[1][0] = chip->eg_tl[0][1];
            chip->eg_tl[2][0] = chip->eg_tl[1][1];

            int level = (okon && !kon) ? chip->eg_level_ssg[1] : chip->eg_level[1][21];

            chip->eg_off = (chip->eg_ssg_enable[1] & 2) != 0 ? (level & 512) != 0 : (level & 0x3f0) == 0x3f0;
            chip->eg_slreach = (level >> 4) == (chip->eg_sl[1][1] << 1);
            chip->eg_zeroreach = level == 0;
            
            chip->eg_level_l[0] = level;

            chip->eg_state_l = chip->eg_state[1][22];

            memcpy(&chip->eg_state[0][1], &chip->eg_state[1][0], 22 * sizeof(unsigned char));
            chip->eg_state[0][0] = chip->eg_nextstate;

            int inc_total = 0;
            if (chip->eg_exp)
            {
                if (chip->eg_incsh0[1])
                    inc_total |= ~chip->eg_level_l[1] >> 4;
                if (chip->eg_incsh1[1])
                    inc_total |= ~chip->eg_level_l[1] >> 3;
                if (chip->eg_incsh2[1])
                    inc_total |= ~chip->eg_level_l[1] >> 2;
                if (chip->eg_incsh3[1])
                    inc_total |= ~chip->eg_level_l[1] >> 1;
            }
            if (chip->eg_linear)
            {
                if (chip->eg_ssg_enable[1] & 4)
                {
                    if (chip->eg_incsh0[1])
                        inc_total |= 4;
                    if (chip->eg_incsh1[1])
                        inc_total |= 8;
                    if (chip->eg_incsh2[1])
                        inc_total |= 16;
                    if (chip->eg_incsh3[1])
                        inc_total |= 32;
                }
                else
                {
                    if (chip->eg_incsh0[1])
                        inc_total |= 1;
                    if (chip->eg_incsh1[1])
                        inc_total |= 2;
                    if (chip->eg_incsh2[1])
                        inc_total |= 4;
                    if (chip->eg_incsh3[1])
                        inc_total |= 8;
                }
            }

            chip->eg_inc_total = inc_total;

            int nextlevel = 0;

            if (!chip->eg_istantattack)
                nextlevel |= chip->eg_level_l[1];

            if (chip->eg_kon_csm[1] & 4)
                nextlevel |= chip->eg_tl[2][1] << 3;

            if (chip->eg_mute || chip->ic)
                nextlevel |= 0x3ff;

            chip->eg_nextlevel[0] = nextlevel;

            memcpy(&chip->eg_level[0][1], &chip->eg_level[1][0], 21 * sizeof(unsigned short));
            chip->eg_level[0][0] = chip->eg_nextlevel[1];

            chip->eg_am_l[0] = chip->lfo_am;
            chip->eg_am_shift[0] = (chip->op_dr_a[1][11][bank] & 0x80) != 0 ? (chip->reg_b4[1][5] >> 4) & 3 : 0;
            static const int eg_am_shift[4] = {
                7, 3, 1, 0
            };

            int lfo_add = (chip->eg_am_l[1] << 1) >> eg_am_shift[chip->eg_am_shift[1]];

            eg_output += lfo_add;

            chip->eg_of1 = (eg_output & 1024) != 0;
            chip->eg_output_lfo = eg_output & 1023;

            chip->eg_ch3_l[1] = chip->eg_ch3_l[0];

            chip->eg_csm_tl = (chip->ch3_csm && (chip->eg_ch3_l[0] & 2) != 0) ? 0 : chip->eg_tl[1][0];

            chip->eg_debug[0] = chip->eg_debug[1] << 1;

            if (chip->eg_dbg_sync)
            {
                chip->eg_debug[0] |= chip->eg_out;
            }

            chip->eg_debug_inc = chip->eg_incsh0[1] || chip->eg_incsh1[1] || chip->eg_incsh2[1] || chip->eg_incsh3[1];
        }
        if (chip->clk2)
        {
            chip->eg_sync = chip->fsm_sel0[1];
            chip->eg_prescaler_clock_l[1] = chip->eg_prescaler_clock_l[0];
            chip->eg_prescaler[1] = chip->eg_prescaler[0];
            chip->eg_step[1] = chip->eg_step[0];
            chip->eg_timer_step[0] = chip->eg_step[0] && chip->eg_prescaler_clock_l[0];

            chip->eg_ic[1] = chip->eg_ic[0];

            chip->eg_timer_test_bit[1] = chip->eg_timer_test_bit[0];

            chip->eg_timer_sum[1] = chip->eg_timer_sum[0] && !chip->eg_ic[0] && !chip->eg_timer_test;
            chip->eg_timer[1] = chip->eg_timer[0];
            chip->eg_clock_delay[1] = chip->eg_clock_delay[0];
            chip->eg_timer_carry[1] = chip->eg_timer_carry[0];
            chip->eg_timer_mask[1] = chip->eg_timer_mask[0];
            chip->eg_timer_masked[1] = chip->eg_timer_masked[0];

            int rate = 0;
            switch (chip->eg_rate_sel)
            {
                case eg_state_attack:
                    rate = chip->eg_rate_ar;
                    break;
                case eg_state_decay:
                    rate = chip->eg_rate_dr;
                    break;
                case eg_state_sustain:
                    rate = chip->eg_rate_sr;
                    break;
                case eg_state_release:
                    rate = (chip->eg_rate_rr * 2) | 1;
                    break;
            }

            chip->eg_rate_nonzero[0] = rate != 0;
            chip->eg_rate_nonzero[2] = chip->eg_rate_nonzero[1];
            chip->eg_rate = rate;
            chip->eg_ksv = chip->kcode[2] >> (chip->eg_ks ^ 3);

            int rate2 = chip->eg_rate2;
            if (rate2 & 64)
                rate2 = 63;

            rate2 &= 63;

            static const int eg_stephi[4][4] = {
                { 0, 0, 0, 0 },
                { 1, 0, 0, 0 },
                { 1, 0, 1, 0 },
                { 1, 1, 1, 0 }
            };

            chip->eg_inc2 = eg_stephi[rate2 & 3][chip->eg_timer_low_lock];
            chip->eg_ratenz = rate2 != 0;
            chip->eg_rate12 = (rate2 & 60) == 48;
            chip->eg_rate13 = (rate2 & 60) == 52;
            chip->eg_rate14 = (rate2 & 60) == 56;
            chip->eg_rate15 = (rate2 & 60) == 60;
            chip->eg_maxrate[0] = (rate2 & 62) == 62;
            chip->eg_rate_low = rate2 & 3;
            chip->eg_rate_slow = (rate2 & 48) != 48;

            chip->eg_rate_sum = (chip->eg_shift_lock + (rate2 >> 2)) & 15;

            chip->eg_incsh0[1] = chip->eg_step[2] && chip->eg_incsh0[0];
            chip->eg_incsh1[1] = chip->eg_step[2] && chip->eg_incsh1[0];
            chip->eg_incsh2[1] = chip->eg_step[2] && chip->eg_incsh2[0];
            chip->eg_incsh3[1] = chip->eg_step[2] && chip->eg_incsh3[0];

            chip->eg_kon_latch[1] = chip->eg_kon_latch[0];
            chip->eg_key[1] = chip->eg_key[0];

            chip->eg_level_ssg[1] = chip->eg_level_ssg[0];

            chip->eg_pg_reset[1] = chip->eg_pg_reset[0];

            chip->eg_ssg_sign[1] = chip->eg_ssg_sign[0];

            chip->eg_ssg_enable[1] = chip->eg_ssg_enable[0];
            chip->eg_ssg_dir[1] = chip->eg_ssg_dir[0];
            chip->eg_ssg_holdup[1] = chip->eg_ssg_holdup[0];
            chip->eg_ssg_pgreset[1] = chip->eg_ssg_pgreset[0];
            chip->eg_ssg_egrepeat[1] = chip->eg_ssg_egrepeat[0];

            chip->eg_sl[0][1] = chip->eg_sl[0][0];
            chip->eg_sl[1][1] = chip->eg_sl[1][0];
            chip->eg_tl[0][1] = chip->eg_tl[0][0];
            chip->eg_tl[1][1] = chip->eg_tl[1][0];
            chip->eg_tl[2][1] = chip->eg_tl[2][0];

            chip->eg_nextlevel[1] = chip->eg_nextlevel[0] + chip->eg_inc_total;

            chip->eg_kon_csm[1] = chip->eg_kon_csm[0];

            int inv = ((chip->eg_level[0][20] ^ 1023) + 513) & 1023;

            chip->eg_output = chip->eg_ssg_inv ? inv : chip->eg_level[0][20];

            chip->eg_level_l[1] = chip->eg_level_l[0];


            int nextstate = eg_state_attack;

            int eg_mute = !chip->eg_kon_event && chip->eg_off && (chip->eg_ssg_holdup[0] & 4) == 0 && chip->eg_state_l != eg_state_attack;
            chip->eg_mute = eg_mute;

            if (eg_mute)
            {
                nextstate |= eg_state_release;
            }

            if (!chip->eg_kon_event && chip->eg_state_l == eg_state_sustain)
            {
                nextstate |= eg_state_sustain;
            }

            if (!chip->eg_kon_event && chip->eg_state_l == eg_state_decay && !chip->eg_slreach)
            {
                nextstate |= eg_state_decay;
            }
            if (!chip->eg_kon_event && chip->eg_state_l == eg_state_decay && chip->eg_slreach)
            {
                nextstate |= eg_state_sustain;
            }

            if ((chip->eg_kon_latch[0] & 4) == 0 && !chip->eg_kon_event)
            {
                nextstate |= eg_state_release;
            }
            if (!chip->eg_kon_event && chip->eg_state_l == eg_state_release)
            {
                nextstate |= eg_state_release;
            }

            if (!chip->eg_kon_event && chip->eg_state_l == eg_state_attack && chip->eg_zeroreach)
            {
                nextstate |= eg_state_decay;
            }
            if (chip->eg_ic[0])
            {
                nextstate |= eg_state_release;
            }

            chip->eg_nextstate = nextstate;
            memcpy(&chip->eg_state[1][0], &chip->eg_state[0][0], 23 * sizeof(unsigned char));

            chip->eg_exp = (chip->eg_kon_latch[0] & 4) != 0 && (chip->eg_state_l == eg_state_attack) && !chip->eg_maxrate[1] && !chip->eg_zeroreach;
            chip->eg_linear = !chip->eg_kon_event && !chip->eg_off && (chip->eg_state_l == eg_state_sustain || chip->eg_state_l == eg_state_release);
            chip->eg_linear |= !chip->eg_kon_event && !chip->eg_off && !chip->eg_slreach && chip->eg_state_l == eg_state_decay;

            chip->eg_istantattack = chip->eg_maxrate[1] && (!chip->eg_maxrate[1] || chip->eg_kon_event);

            memcpy(&chip->eg_level[1][0], &chip->eg_level[0][0], 22 * sizeof(unsigned short));


            chip->eg_am_l[1] = chip->eg_am_l[0];
            chip->eg_am_shift[1] = chip->eg_am_shift[0];

            chip->eg_ch3_l[0] = (chip->eg_ch3_l[1] << 1) | chip->fsm_sel_ch3[1];

            int levelsum = chip->eg_output_lfo + (chip->eg_csm_tl << 3);

            int eg_of = (levelsum & 1024) != 0;

            if (eg_of || chip->eg_of1)
                levelsum = 1023;

            chip->eg_out = levelsum;

            chip->eg_dbg_sync = chip->fsm_sel2[1];

            chip->eg_debug[1] = chip->eg_debug[0];
        }
    }

    {
        if (chip->clk1)
        {
            chip->op_phase1 = chip->pg_out;
            chip->op_phase2 = chip->op_mod[1][5];

            chip->op_sign[1] = chip->op_sign[0];
            static const int logsin[128] = {
                0x6c3, 0x58b, 0x4e4, 0x471, 0x41a, 0x3d3, 0x398, 0x365, 0x339, 0x311, 0x2ed, 0x2cd, 0x2af, 0x293, 0x279, 0x261,
                0x24b, 0x236, 0x222, 0x20f, 0x1fd, 0x1ec, 0x1dc, 0x1cd, 0x1be, 0x1b0, 0x1a2, 0x195, 0x188, 0x17c, 0x171, 0x166,
                0x15b, 0x150, 0x146, 0x13c, 0x133, 0x129, 0x121, 0x118, 0x10f, 0x107, 0x0ff, 0x0f8, 0x0f0, 0x0e9, 0x0e2, 0x0db,
                0x0d4, 0x0cd, 0x0c7, 0x0c1, 0x0bb, 0x0b5, 0x0af, 0x0a9, 0x0a4, 0x09f, 0x099, 0x094, 0x08f, 0x08a, 0x086, 0x081,
                0x07d, 0x078, 0x074, 0x070, 0x06c, 0x068, 0x064, 0x060, 0x05c, 0x059, 0x055, 0x052, 0x04e, 0x04b, 0x048, 0x045,
                0x042, 0x03f, 0x03c, 0x039, 0x037, 0x034, 0x031, 0x02f, 0x02d, 0x02a, 0x028, 0x026, 0x024, 0x022, 0x020, 0x01e,
                0x01c, 0x01a, 0x018, 0x017, 0x015, 0x014, 0x012, 0x011, 0x00f, 0x00e, 0x00d, 0x00c, 0x00a, 0x009, 0x008, 0x007,
                0x007, 0x006, 0x005, 0x004, 0x004, 0x003, 0x002, 0x002, 0x001, 0x001, 0x001, 0x001, 0x000, 0x000, 0x000, 0x000
            };
            static const int logsin_d[128] = {
                0x196, 0x07c, 0x04a, 0x035, 0x029, 0x022, 0x01d, 0x019, 0x015, 0x013, 0x012, 0x00f, 0x00e, 0x00d, 0x00d, 0x00c,
                0x00b, 0x00a, 0x00a, 0x009, 0x009, 0x009, 0x008, 0x007, 0x007, 0x007, 0x007, 0x006, 0x007, 0x006, 0x006, 0x005,
                0x005, 0x005, 0x005, 0x005, 0x004, 0x005, 0x004, 0x004, 0x005, 0x004, 0x004, 0x003, 0x004, 0x003, 0x003, 0x003,
                0x003, 0x004, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x002, 0x003, 0x003, 0x003, 0x003, 0x002, 0x002,
                0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x001, 0x002, 0x002, 0x002, 0x001,
                0x001, 0x001, 0x002, 0x002, 0x001, 0x001, 0x002, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001,
                0x001, 0x001, 0x001, 0x000, 0x001, 0x000, 0x001, 0x000, 0x001, 0x001, 0x000, 0x000, 0x001, 0x001, 0x001, 0x001,
                0x000, 0x000, 0x000, 0x001, 0x000, 0x000, 0x001, 0x000, 0x001, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000
            };
            static const int pow[128] = {
                0x3f5, 0x3ea, 0x3df, 0x3d4, 0x3c9, 0x3bf, 0x3b4, 0x3a9, 0x39f, 0x394, 0x38a, 0x37f, 0x375, 0x36a, 0x360, 0x356,
                0x34c, 0x342, 0x338, 0x32e, 0x324, 0x31a, 0x310, 0x306, 0x2fd, 0x2f3, 0x2e9, 0x2e0, 0x2d6, 0x2cd, 0x2c4, 0x2ba,
                0x2b1, 0x2a8, 0x29e, 0x295, 0x28c, 0x283, 0x27a, 0x271, 0x268, 0x25f, 0x257, 0x24e, 0x245, 0x23c, 0x234, 0x22b,
                0x223, 0x21a, 0x212, 0x209, 0x201, 0x1f9, 0x1f0, 0x1e8, 0x1e0, 0x1d8, 0x1d0, 0x1c8, 0x1c0, 0x1b8, 0x1b0, 0x1a8,
                0x1a0, 0x199, 0x191, 0x189, 0x181, 0x17a, 0x172, 0x16b, 0x163, 0x15c, 0x154, 0x14d, 0x146, 0x13e, 0x137, 0x130,
                0x129, 0x122, 0x11b, 0x114, 0x10c, 0x106, 0x0ff, 0x0f8, 0x0f1, 0x0ea, 0x0e3, 0x0dc, 0x0d6, 0x0cf, 0x0c8, 0x0c2,
                0x0bb, 0x0b5, 0x0ae, 0x0a8, 0x0a1, 0x09b, 0x094, 0x08e, 0x088, 0x082, 0x07b, 0x075, 0x06f, 0x069, 0x063, 0x05d,
                0x057, 0x051, 0x04b, 0x045, 0x03f, 0x039, 0x033, 0x02d, 0x028, 0x022, 0x01c, 0x016, 0x011, 0x00b, 0x006, 0x000,
            };
            static const int pow_d[128] = {
                0x005, 0x005, 0x005, 0x006, 0x006, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x006, 0x005, 0x005,
                0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x004, 0x005,
                0x004, 0x004, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x005, 0x004, 0x004, 0x004, 0x005, 0x004, 0x005,
                0x004, 0x004, 0x004, 0x005, 0x004, 0x004, 0x005, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
                0x004, 0x003, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x003, 0x004, 0x004, 0x004,
                0x003, 0x003, 0x003, 0x003, 0x004, 0x003, 0x003, 0x003, 0x003, 0x003, 0x004, 0x004, 0x003, 0x003, 0x004, 0x003,
                0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x004, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003,
                0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x002, 0x003, 0x003, 0x003, 0x003, 0x003, 0x002, 0x003,
            };

            chip->op_logsin_base = logsin[(chip->op_phase_index >> 1) & 127];
            chip->op_logsin_delta = (chip->op_phase_index & 1) != 0 ? 0 : logsin_d[(chip->op_phase_index >> 1) & 127];

            chip->op_eglevel = chip->eg_out;

            chip->op_shift[0] = chip->op_att >> 8;

            chip->op_pow_base = pow[(chip->op_att >> 1) & 127];
            chip->op_pow_delta = (chip->op_att & 1) != 0 ? 0 : pow_d[(chip->op_att >> 1) & 127];

            int output = chip->op_pow;
            output = (output << 2) >> chip->op_shift[1];

            if (chip->reg_test_21[1] & 16)
                output ^= 1 << 13;

            if (chip->op_sign[0] & 4)
                output ^= 0x3fff;

            chip->op_output[0] = output;
            chip->op_output[2] = chip->op_output[1];

            memcpy(&chip->op_op1_0[0][1], &chip->op_op1_0[1][0], 5 * sizeof(unsigned short));
            memcpy(&chip->op_op1_1[0][1], &chip->op_op1_1[1][0], 5 * sizeof(unsigned short));
            memcpy(&chip->op_op2[0][1], &chip->op_op2[1][0], 5 * sizeof(unsigned short));

            chip->op_op1_0[0][0] = chip->op_loadfb ? chip->op_output[3] : chip->op_op1_0[1][5];
            chip->op_op1_1[0][0] = chip->op_loadfb ? chip->op_op1_0[1][5] : chip->op_op1_1[1][5];
            chip->op_op2[0][0] = chip->op_loadop2 ? chip->op_output[3] : chip->op_op2[1][5];

            int mod1 = 0;
            int mod2 = 0;

            if (chip->op_mod_op1_0)
            {
                mod2 |= chip->op_op1_0[1][5];
            }
            if (chip->op_mod_op1_1)
            {
                mod1 |= chip->op_op1_1[1][5];
            }
            if (chip->op_mod_op2)
            {
                mod1 |= chip->op_op2[1][5];
            }
            if (chip->op_mod_prev_0)
            {
                mod2 |= chip->op_output[3];
            }
            if (chip->op_mod_prev_1)
            {
                mod1 |= chip->op_output[3];
            }
            if (mod1 & (1 << 13))
                mod1 |= 1 << 14;
            if (mod2 & (1 << 13))
                mod2 |= 1 << 14;
            chip->op_mod1 = mod1;
            chip->op_mod2 = mod2;

            int mod;

            if (chip->op_do_fb)
            {
                if (!chip->op_fb)
                    mod = 0;
                else
                {
                    mod = chip->op_mod_sum;
                    if (mod & (1 << 13))
                        mod |= ~0x3fff;

                    mod = mod >> (9 - chip->op_fb);
                }
            }
            else
                mod = chip->op_mod_sum;

            memcpy(&chip->op_mod[0][1], &chip->op_mod[1][0], 5 * sizeof(unsigned short));
            chip->op_mod[0][0] = mod;
        }
        if (chip->clk2)
        {

            int phase = chip->op_phase1 + chip->op_phase2;

            int phase2 = phase & 255;
            if (phase & 256)
                phase2 ^= 255;

            chip->op_phase_index = phase2;

            chip->op_sign[0] = chip->op_sign[1] << 1;
            if (phase & 512)
                chip->op_sign[0] |= 1;

            //chip->op_eglevel = 256; //
            int level = chip->op_logsin_base + chip->op_logsin_delta + (chip->op_eglevel << 2);
            if (level & 4096)
                level = 4095;
            chip->op_att = level;

            chip->op_shift[1] = chip->op_shift[0];

            chip->op_pow = (chip->op_pow_base + chip->op_pow_delta) | 0x400;

            int output = chip->op_output[0];

            if (chip->op_sign[1] & 4)
                output++;

            chip->op_output[1] = output & 0x3fff;
            chip->op_output[3] = chip->op_output[2];

            chip->op_loadfb = chip->alg_load_fb;
            chip->op_loadop2 = chip->alg_mod_op1_1_l;
            chip->op_mod_op1_0 = chip->alg_mod_op1_0_l;
            chip->op_mod_op1_1 = chip->alg_mod_op1_1_l;
            chip->op_mod_op2 = chip->alg_mod_op2_l;
            chip->op_mod_prev_0 = chip->alg_mod_prev_0_l;
            chip->op_mod_prev_1 = chip->alg_mod_prev_1_l;
            chip->op_do_fb = chip->alg_do_fb[1];

            chip->op_fb = (chip->reg_connect_fb[0][0] >> 3) & 7;

            memcpy(&chip->op_op1_0[1][0], &chip->op_op1_0[0][0], 6 * sizeof(unsigned short));
            memcpy(&chip->op_op1_1[1][0], &chip->op_op1_1[0][0], 6 * sizeof(unsigned short));
            memcpy(&chip->op_op2[1][0], &chip->op_op2[0][0], 6 * sizeof(unsigned short));

            int mod = (chip->op_mod1 + chip->op_mod2) >> 1;
            mod &= 0x3fff;
            chip->op_mod_sum = mod;
            memcpy(&chip->op_mod[1][0], &chip->op_mod[0][0], 6 * sizeof(unsigned short));
        }
    }

#ifndef FMOPNA_YM2612
    {
#ifdef FMOPNA_YM2608
        if (!chip->mclk2)
        {
            int addr2d = chip->write2_en && ADDRESS_MATCH(0x2d);
            int addr2e = chip->write2_en && ADDRESS_MATCH(0x2e);
            int addr2f = chip->write2_en && ADDRESS_MATCH(0x2f);
            chip->ssg_prescaler1[0] = (chip->ssg_prescaler1[1] && !addr2f) || addr2e;
            chip->ssg_prescaler2[0] = (chip->ssg_prescaler2[1] && !addr2f) || addr2d || chip->ic;
        }
        else
        {
            chip->ssg_prescaler1[1] = chip->ssg_prescaler1[0] && !chip->ic;
            chip->ssg_prescaler2[1] = chip->ssg_prescaler2[0];
        }
#endif
        if (!chip->mclk2)
        {
            chip->ssg_div1[0] = !chip->ssg_div1[1];
        }
        else
        {
            chip->ssg_div1[1] = chip->ssg_div1[0];
        }
        if (!chip->ssg_div1[0])
        {
            chip->ssg_div2[0] = !chip->ssg_div2[1];
        }
        else
        {
            chip->ssg_div2[1] = chip->ssg_div2[0];
        }
        if (!chip->ssg_div2[0])
        {
            chip->ssg_div3[0] = !chip->ssg_div3[1];
        }
        else
        {
            chip->ssg_div3[1] = chip->ssg_div3[0];
        }
        if (chip->ic_check2)
        {
            chip->ssg_div1[0] = 0;
            chip->ssg_div1[1] = 0;
            chip->ssg_div2[0] = 0;
            chip->ssg_div2[1] = 0;
            chip->ssg_div3[0] = 0;
            chip->ssg_div3[1] = 0;
        }

#ifdef FMOPNA_YM2608
        if (chip->ssg_prescaler2[1])
        {
            if (chip->ssg_prescaler1[1])
            {
                chip->ssg_clk = chip->ssg_div2[0];
            }
            else
            {
                chip->ssg_clk = chip->ssg_div3[0];
            }
        }
        else
            chip->ssg_clk = chip->ssg_div1[0];
#else
        chip->ssg_clk = chip->ssg_div3[0];
#endif

        chip->ssg_clk1 = !chip->ssg_clk;
        chip->ssg_clk2 = chip->ssg_clk;

        if (chip->ic)
            chip->ssg_ssg_addr = 0;
        else if (chip->ssg_write0)
            chip->ssg_ssg_addr = (chip->data_bus1 & 0xf0) == 0;

        if (chip->ic)
            chip->ssg_address = 0;
        else if (chip->ssg_write0 && (chip->data_bus1 & 0xe0) == 0)
            chip->ssg_address = chip->data_bus1 & 0x1f;
        
        int ssg_access = chip->ssg_ssg_addr && (chip->ssg_write1 || chip->ssg_read1);

        if (chip->ssg_egtrig_rst)
            chip->ssg_egtrig = 0;

        if (chip->ic)
        {
            chip->ssg_freq_a[0] = chip->data_bus1 & 255;
            chip->ssg_freq_a[1] = chip->data_bus1 & 15;
            chip->ssg_freq_b[0] = chip->data_bus1 & 255;
            chip->ssg_freq_b[1] = chip->data_bus1 & 15;
            chip->ssg_freq_c[0] = chip->data_bus1 & 255;
            chip->ssg_freq_c[1] = chip->data_bus1 & 15;
            chip->ssg_noise = chip->data_bus1 & 31;
            chip->ssg_mode = chip->data_bus1 & 255;
            chip->ssg_level_a = chip->data_bus1 & 31;
            chip->ssg_level_b = chip->data_bus1 & 31;
            chip->ssg_level_c = chip->data_bus1 & 31;
            chip->ssg_env[0] = chip->data_bus1 & 255;
            chip->ssg_env[1] = chip->data_bus1 & 255;
            chip->ssg_envmode = chip->data_bus1 & 15;
#ifdef FMOPNA_YM2608
            chip->o_gpio_a = chip->data_bus1 & 255;
            chip->o_gpio_b = chip->data_bus1 & 255;
#endif

            chip->ssg_egtrig = 1;
        }
        else if (chip->ssg_ssg_addr && chip->ssg_write1)
        {
            switch (chip->ssg_address)
            {
                case 0x0:
                    chip->ssg_freq_a[0] = chip->data_bus1 & 255;
                    break;
                case 0x1:
                    chip->ssg_freq_a[1] = chip->data_bus1 & 15;
                    break;
                case 0x2:
                    chip->ssg_freq_b[0] = chip->data_bus1 & 255;
                    break;
                case 0x3:
                    chip->ssg_freq_b[1] = chip->data_bus1 & 15;
                    break;
                case 0x4:
                    chip->ssg_freq_c[0] = chip->data_bus1 & 255;
                    break;
                case 0x5:
                    chip->ssg_freq_c[1] = chip->data_bus1 & 15;
                    break;
                case 0x6:
                    chip->ssg_noise = chip->data_bus1 & 31;
                    break;
                case 0x7:
                    chip->ssg_mode = chip->data_bus1 & 255;
                    break;
                case 0x8:
                    chip->ssg_level_a = chip->data_bus1 & 31;
                    break;
                case 0x9:
                    chip->ssg_level_b = chip->data_bus1 & 31;
                    break;
                case 0xa:
                    chip->ssg_level_c = chip->data_bus1 & 31;
                    break;
                case 0xb:
                    chip->ssg_env[0] = chip->data_bus1 & 255;
                    break;
                case 0xc:
                    chip->ssg_env[1] = chip->data_bus1 & 255;
                    break;
                case 0xd:
                    chip->ssg_envmode = chip->data_bus1 & 15;
                    chip->ssg_egtrig = 1;
                    break;
#ifdef FMOPNA_YM2608
                case 0xe:
                    chip->o_gpio_a = chip->data_bus1 & 255;
                    break;
                case 0xf:
                    chip->o_gpio_b = chip->data_bus1 & 255;
                    break;
#endif
            }
        }
        if (chip->ssg_ssg_addr && chip->ssg_read1)
        {
            switch (chip->ssg_address)
            {
                case 0x0:
                    chip->data_bus1 &= ~255;
                    chip->data_bus1 |= chip->ssg_freq_a[0] & 255;
                    break;
                case 0x1:
                    chip->data_bus1 &= ~15;
                    chip->data_bus1 |= chip->ssg_freq_a[1] & 15;
                    break;
                case 0x2:
                    chip->data_bus1 &= ~255;
                    chip->data_bus1 |= chip->ssg_freq_b[0] & 255;
                    break;
                case 0x3:
                    chip->data_bus1 &= ~15;
                    chip->data_bus1 |= chip->ssg_freq_b[1] & 15;
                    break;
                case 0x4:
                    chip->data_bus1 &= ~255;
                    chip->data_bus1 |= chip->ssg_freq_c[0] & 255;
                    break;
                case 0x5:
                    chip->data_bus1 &= ~15;
                    chip->data_bus1 |= chip->ssg_freq_c[1] & 15;
                    break;
                case 0x6:
                    chip->data_bus1 &= ~31;
                    chip->data_bus1 |= chip->ssg_noise & 31;
                    break;
                case 0x7:
                    chip->data_bus1 &= ~255;
                    chip->data_bus1 |= chip->ssg_mode & 255;
                    break;
                case 0x8:
                    chip->data_bus1 &= ~31;
                    chip->data_bus1 |= chip->ssg_level_a & 31;
                    break;
                case 0x9:
                    chip->data_bus1 &= ~31;
                    chip->data_bus1 |= chip->ssg_level_b & 31;
                    break;
                case 0xa:
                    chip->data_bus1 &= ~31;
                    chip->data_bus1 |= chip->ssg_level_c & 31;
                    break;
                case 0xb:
                    chip->data_bus1 &= ~255;
                    chip->data_bus1 |= chip->ssg_env[0] & 255;
                    break;
                case 0xc:
                    chip->data_bus1 &= ~255;
                    chip->data_bus1 |= chip->ssg_env[1] & 255;
                    break;
                case 0xd:
                    chip->data_bus1 &= ~15;
                    chip->data_bus1 |= chip->ssg_envmode & 15;
                    break;
#ifdef FMOPNA_YM2608
                case 0xe:
                    chip->data_bus1 &= ~255;
                    chip->data_bus1 |= chip->input.gpio_a & 255;
                    break;
                case 0xf:
                    chip->data_bus1 &= ~255;
                    chip->data_bus1 |= chip->input.gpio_b & 255;
                    break;
#endif
            }
        }

        if (chip->ssg_clk1)
        {
            chip->ssg_envadd = (chip->ssg_eg_of[1] & 4) != 0 && (chip->ssg_sel[1] & 8) != 0;
            chip->ssg_envcnt[1] = chip->ssg_hold[0] ? 31 : chip->ssg_envcnt[0];
            chip->ssg_dir[1] = chip->ssg_dir[0];
            chip->ssg_hold[1] = chip->ssg_hold[0];
            chip->ssg_t2[1] = chip->ssg_t2[0];

            chip->ssg_egtrig_s = chip->ssg_egtrig;

            chip->ssg_eg_sel_l = (chip->ssg_sel[1] & 8) != 0;

            chip->ssg_sel[0] = chip->ssg_sel[1] << 1;
            chip->ssg_sel[0] |= (chip->ssg_sel[1] & 7) == 0 && !chip->ic;

            chip->ssg_sel_freq = 0;
            if (chip->ssg_sel[1] & 1)
                chip->ssg_sel_freq |= (chip->ssg_freq_c[1] << 8) | chip->ssg_freq_c[0];
            if (chip->ssg_sel[1] & 2)
                chip->ssg_sel_freq |= (chip->ssg_freq_b[1] << 8) | chip->ssg_freq_b[0];
            if (chip->ssg_sel[1] & 4)
                chip->ssg_sel_freq |= (chip->ssg_freq_a[1] << 8) | chip->ssg_freq_a[0];
            if (chip->ssg_sel[1] & 8)
                chip->ssg_sel_freq |= (chip->ssg_env[1] << 8) | chip->ssg_env[0];

            int cnt = chip->ssg_freq_cnt[7] + 1;
            int of = (cnt & 0x1000) != 0;
            chip->ssg_freq_cnt[0] = cnt & 0xfff;
            chip->ssg_freq_cnt[4] = chip->ssg_freq_cnt[3];
            chip->ssg_freq_cnt[6] = chip->ssg_freq_cnt[5];

            chip->ssg_cnt_of[1] = chip->ssg_cnt_of[0];

            if (chip->ssg_sel[1] & 8)
                chip->ssg_sign_toggle = chip->ssg_cnt_of[0] & 7;
            else
                chip->ssg_sign_toggle = 0;

            chip->ssg_freq_cnt2[1] = chip->ssg_freq_cnt2[0];
            chip->ssg_freq_cnt2[3] = chip->ssg_freq_cnt2[2];
            chip->ssg_freq_cnt2[5] = chip->ssg_freq_cnt2[4];
            chip->ssg_freq_cnt2[7] = chip->ssg_freq_cnt2[6];

            chip->ssg_sign[1] = chip->ssg_sign[0];

            int cnt_of = !chip->ssg_cnt_reload && (chip->ssg_sel_freq_l - chip->ssg_cnt_of_l < chip->ssg_freq_cnt2[0]);

            chip->ssg_cnt2_add = (chip->ssg_sel[1] & 8) != 0 && of;

            chip->ssg_sel_eg_l[0] = (chip->ssg_sel[1] & 8) != 0;

            chip->ssg_eg_of[0] = (chip->ssg_eg_of[1] << 1) | cnt_of;

            int fr_rst = chip->ssg_ch_of || ((cnt_of || chip->ssg_cnt_reload) && chip->ssg_sel_eg_l[1]);

            chip->ssg_freq_cnt[2] = fr_rst ? 0 : chip->ssg_freq_cnt[1];

            chip->ssg_fr_rst_l = fr_rst;

            chip->ssg_noise_add = (chip->ssg_sel[1] & 1) != 0;

            chip->ssg_noise_cnt[1] = chip->ssg_noise_cnt[0];

            int noise_of = chip->ssg_noise <= (chip->ssg_noise_cnt[0] >> 1);

            chip->ssg_noise_of = noise_of && chip->ssg_noise_of_low;

            chip->ssg_test = chip->input.test;

            chip->ssg_noise_step = chip->ssg_noise_of || !chip->ssg_test;
        }
        if (chip->ssg_clk2)
        {
            int sum = chip->ssg_envcnt[1] + chip->ssg_envadd;
            int of = (sum & 32) != 0;
            chip->ssg_envcnt[0] = chip->ssg_egtrig_s ? 0 : sum & 31;

            int dir = of && (chip->ssg_envmode & 2) != 0 && !chip->ssg_hold[1];
            chip->ssg_dir[0] = chip->ssg_egtrig_s ? 0 : chip->ssg_dir[1] ^ dir;

            chip->ssg_hold[0] = chip->ssg_egtrig_s ? 0 : chip->ssg_hold[1] || (of && (chip->ssg_envmode & 1) != 0);

            chip->ssg_t2[0] = chip->ssg_egtrig_s ? 0 : chip->ssg_t2[1] || of;

            chip->ssg_egtrig_rst = chip->ssg_egtrig_s && chip->ssg_eg_sel_l;


            chip->ssg_sel[1] = chip->ssg_sel[0];

            chip->ssg_freq_cnt[1] = chip->ssg_freq_cnt[0];
            chip->ssg_freq_cnt[3] = chip->ssg_freq_cnt[2];
            chip->ssg_freq_cnt[5] = chip->ssg_freq_cnt[4];
            chip->ssg_freq_cnt[7] = chip->ssg_freq_cnt[6];

            int cnt_of = (chip->ssg_sel_freq & 0xfff) <= chip->ssg_freq_cnt[0];

            chip->ssg_cnt_of_l = cnt_of;
            chip->ssg_sel_freq_l = (chip->ssg_sel_freq >> 12) & 0xf;

            chip->ssg_cnt_of[0] = (chip->ssg_cnt_of[1] << 1) | cnt_of;

            if (chip->ic)
                chip->ssg_sign[0] = 0;
            else
                chip->ssg_sign[0] = chip->ssg_sign[1] ^ chip->ssg_sign_toggle;

            int cnt = chip->ssg_freq_cnt2[7] + chip->ssg_cnt2_add;
            chip->ssg_freq_cnt2[0] = cnt & 0xf;
            chip->ssg_freq_cnt2[2] = chip->ssg_fr_rst_l ? 0 : chip->ssg_freq_cnt2[1];
            chip->ssg_freq_cnt2[4] = chip->ssg_freq_cnt2[3];
            chip->ssg_freq_cnt2[6] = chip->ssg_freq_cnt2[5];

            chip->ssg_sel_eg_l[1] = chip->ssg_sel_eg_l[0];

            chip->ssg_ch_of = (!chip->ssg_sel_eg_l[0] && cnt_of) || chip->ic;

            chip->ssg_cnt_reload = chip->ssg_sel_eg_l[0] && chip->ssg_egtrig_s;

            chip->ssg_eg_of[1] = chip->ssg_eg_of[0];


            chip->ssg_noise_cnt[0] = chip->ssg_noise_step ? 0 : ((chip->ssg_noise_cnt[1] + chip->ssg_noise_add) & 63);

            chip->ssg_noise_of_low = (chip->ssg_noise_cnt[1] & 1) != 0 && chip->ssg_noise_add;

        }

        if (!chip->ssg_noise_step)
        {
            int bit = ((chip->ssg_noise_lfsr[1] >> 16) ^ (chip->ssg_noise_lfsr[1] >> 13)) & 1;

            if ((chip->ssg_noise_lfsr[1] & 0x1ffff) == 0)
                bit |= 1;

            chip->ssg_noise_lfsr[0] = (chip->ssg_noise_lfsr[1] << 1) | bit;
        }
        else
        {
            chip->ssg_noise_lfsr[1] = chip->ic ? 0 : chip->ssg_noise_lfsr[0];
        }

        if (chip->ssg_clk2)
            chip->ssg_noise_bit = (chip->ssg_noise_lfsr[1] >> 16) & 1;

        int envlevel = chip->ssg_hold[0] ? 31 : chip->ssg_envcnt[0];
        envlevel = (chip->ssg_dir[0] ^ ((chip->ssg_envmode >> 2) & 1)) == 0 ? (envlevel ^ 31) : envlevel;
        envlevel = (chip->ssg_t2[0] && (chip->ssg_envmode & 8) == 0) ? 0 : envlevel;

        int vol_a = (chip->ssg_level_a & 0x10) != 0 ? envlevel : (((chip->ssg_level_a & 15) << 1) | 1);
        int vol_b = (chip->ssg_level_b & 0x10) != 0 ? envlevel : (((chip->ssg_level_b & 15) << 1) | 1);
        int vol_c = (chip->ssg_level_c & 0x10) != 0 ? envlevel : (((chip->ssg_level_c & 15) << 1) | 1);

        int sign_a = ((chip->ssg_mode & 1) == 0 && (chip->ssg_sign[0] & 1) != 0) || ((chip->ssg_mode & 8) == 0 && chip->ssg_noise_bit);
        int sign_b = ((chip->ssg_mode & 2) == 0 && (chip->ssg_sign[0] & 2) != 0) || ((chip->ssg_mode & 16) == 0 && chip->ssg_noise_bit);
        int sign_c = ((chip->ssg_mode & 4) == 0 && (chip->ssg_sign[0] & 4) != 0) || ((chip->ssg_mode & 32) == 0 && chip->ssg_noise_bit);

        static const float volume_lut[32] = {
            0.0000, 0.0000, 0.0049, 0.0075, 0.0105, 0.0131, 0.0156, 0.0183,
            0.0228, 0.0276, 0.0321, 0.0367, 0.0448, 0.0535, 0.0626, 0.0713,
            0.0884, 0.1057, 0.1225, 0.1392, 0.1691, 0.2013, 0.2348, 0.2670,
            0.3307, 0.3951, 0.4573, 0.5196, 0.6316, 0.7528, 0.8787, 1.0000
        };


        // tildearrow: per-channel oscilloscope for SSG
        chip->o_analog = 0;
        chip->o_analog += chip->o_analog_ch[0] = volume_lut[sign_a ? 0 : vol_a];
        chip->o_analog += chip->o_analog_ch[1] = volume_lut[sign_b ? 0 : vol_b];
        chip->o_analog += chip->o_analog_ch[2] = volume_lut[sign_c ? 0 : vol_c];

        //please work
        chip->o_gpio_a_d = (chip->ssg_mode & 64) == 0;
        chip->o_gpio_b_d = (chip->ssg_mode & 128) == 0;
    }

    {
        if (chip->mclk2)
        {
            chip->rss_dclk_l = chip->dclk;
        }
        if (chip->clk2)
        {
            chip->rss_cnt1_sync = !chip->fsm_rss2;
        }
        if (chip->aclk1)
        {
            int cnt1_rst = chip->rss_cnt1[1] == 5 || chip->rss_cnt1_sync;
            chip->rss_cnt1[0] = cnt1_rst ? 0 : (chip->rss_cnt1[1] + 1) & 7;

            chip->rss_eclk1_l = chip->rss_cnt1[1] == 2 || chip->rss_cnt1[1] == 3;
            chip->rss_eclk2_l = chip->rss_cnt1[1] == 5 || chip->rss_cnt1[1] == 0;
            chip->rss_fclk_sel[0] = chip->rss_cnt1[1] == 5 || chip->rss_cnt1[1] == 0 || chip->rss_cnt1[1] == 1;
        }

        if (chip->aclk2)
        {
            chip->rss_cnt1[1] = chip->rss_cnt1[0];

            chip->rss_eclk1 = chip->rss_eclk1_l;
            chip->rss_eclk2 = chip->rss_eclk2_l;

            chip->rss_fclk_sel[1] = chip->rss_fclk_sel[0];
        }

        chip->rss_fclk1 = !chip->rss_dclk_l && chip->aclk1 && chip->rss_fclk_sel[1];
        chip->rss_fclk2 = !chip->rss_dclk_l && chip->aclk2 && chip->rss_fclk_sel[1];

        if (chip->clk1)
        {
            int clk_rst = chip->rss_fmcnt_of || chip->rss_fmcnt_sync;
            chip->rss_fmcnt[0] = clk_rst ? 0 : (chip->rss_fmcnt[1] + 1) & 15;

#ifdef FMOPNA_YM2608
            chip->rss_tl_sel[0] = chip->write0_en ? chip->data_bus1 == 0x11 : chip->rss_tl_sel[1];
#else
            chip->rss_tl_sel[0] = chip->write0_en ? chip->data_bus1 == 0x101 : chip->rss_tl_sel[1];
#endif

            if (chip->ic)
                chip->rss_tl[0] = 0;
            else
                chip->rss_tl[0] = chip->rss_tl_sel[1]
#ifdef FMOPNA_YM2608
                && (chip->data_bus1 & 0x100) == 0
#else
                && (chip->data_bus1 & 0x100) != 0
#endif
                && chip->write1_en ? chip->data_bus1 & 0x3f : chip->rss_tl[1];
        }
        if (chip->clk2)
        {
            chip->rss_fmcnt[1] = chip->rss_fmcnt[0];
            chip->rss_fmcnt_of = (chip->rss_fmcnt[0] & 11) == 11;

            chip->rss_fmcnt_sync = chip->fsm_sel23[1];

            chip->rss_tl_sel[1] = chip->rss_tl_sel[0];

            chip->rss_tl[1] = chip->rss_tl[0];
        }

        if (chip->rss_eclk1)
        {
            int cnt2_rst = chip->ic || chip->rss_cnt2[1] == 5 || (chip->fsm_rss && chip->rss_cnt2[0] == 0);
            chip->rss_cnt2[0] = cnt2_rst ? 0 : (chip->rss_cnt2[1] + 1) & 7;
        }
        if (chip->rss_eclk2)
        {
            chip->rss_cnt2[1] = chip->rss_cnt2[0];
        }

#ifdef FMOPNA_YM2610
        if (chip->clk1)
        {
            if (chip->write0_en && (chip->data_bus1 & 0x3c) != 0)
            {
                chip->rss_address[0] = chip->data_bus1;
            }
            else
            {
                chip->rss_address[0] = chip->rss_address[1];
            }

            if (chip->rss_address_wr[1] && chip->write1_en)
            {
                chip->rss_data[0] = chip->data_bus1 & 0xff;
            }
            else
            {
                chip->rss_data[0] = chip->rss_data[1];
            }

            chip->rss_address_wr[0] = chip->write0_en ? (chip->data_bus1 & 0x3c) != 0 : chip->rss_address_wr[1];

            chip->rss_data_wr[0] = (chip->write1_en && chip->rss_address_wr[1])
                || (!chip->write0_en && chip->rss_data_wr[1]);

            memcpy(&chip->rss_reg_pan_tl[0][1], &chip->rss_reg_pan_tl[1][0], 5 * sizeof(unsigned char));
            memcpy(&chip->rss_reg_start_l[0][1], &chip->rss_reg_start_l[1][0], 5 * sizeof(unsigned char));
            memcpy(&chip->rss_reg_start_h[0][1], &chip->rss_reg_start_h[1][0], 5 * sizeof(unsigned char));
            memcpy(&chip->rss_reg_stop_l[0][1], &chip->rss_reg_stop_l[1][0], 5 * sizeof(unsigned char));
            memcpy(&chip->rss_reg_stop_h[0][1], &chip->rss_reg_stop_h[1][0], 5 * sizeof(unsigned char));

            if (chip->ic)
            {
                chip->rss_reg_pan_tl[0][0] = 0;
                chip->rss_reg_start_l[0][0] = 0;
                chip->rss_reg_start_h[0][0] = 0;
                chip->rss_reg_stop_l[0][0] = 0;
                chip->rss_reg_stop_h[0][0] = 0;
            }
            else
            {
                if (chip->rss_write08)
                {
                    chip->rss_reg_pan_tl[0][0] = chip->rss_data[1] & 0xdf;
                }
                else
                {
                    chip->rss_reg_pan_tl[0][0] = chip->rss_reg_pan_tl[1][5];
                }
                if (chip->rss_write10)
                {
                    chip->rss_reg_start_l[0][0] = chip->rss_data[1];
                }
                else
                {
                    chip->rss_reg_start_l[0][0] = chip->rss_reg_start_l[1][5];
                }
                if (chip->rss_write18)
                {
                    chip->rss_reg_start_h[0][0] = chip->rss_data[1];
                }
                else
                {
                    chip->rss_reg_start_h[0][0] = chip->rss_reg_start_h[1][5];
                }
                if (chip->rss_write20)
                {
                    chip->rss_reg_stop_l[0][0] = chip->rss_data[1];
                }
                else
                {
                    chip->rss_reg_stop_l[0][0] = chip->rss_reg_stop_l[1][5];
                }
                if (chip->rss_write28)
                {
                    chip->rss_reg_stop_h[0][0] = chip->rss_data[1] & 0xf;
                }
                else
                {
                    chip->rss_reg_stop_h[0][0] = chip->rss_reg_stop_h[1][5];
                }
            }
        }
        if (chip->clk2)
        {
            if (chip->ic)
            {
                chip->rss_address[1] = 0;
                chip->rss_data[1] = 0;
            }
            else
            {
                chip->rss_address[1] = chip->rss_address[0];
                chip->rss_data[1] = chip->rss_data[0];
            }
            chip->rss_address_wr[1] = chip->rss_address_wr[0];
            chip->rss_data_wr[1] = chip->rss_data_wr[0];

            int addr_match = (chip->rss_fmcnt[0] == (chip->rss_address[0] & 7) && (chip->rss_address[0] & 6) != 6)
                && chip->rss_data_wr[0] && (chip->rss_address[0] & 0x100) != 0;

            chip->rss_write08 = addr_match && (chip->rss_address[0] & 0xf8) == 0x08;
            chip->rss_write10 = addr_match && (chip->rss_address[0] & 0xf8) == 0x10;
            chip->rss_write18 = addr_match && (chip->rss_address[0] & 0xf8) == 0x18;
            chip->rss_write20 = addr_match && (chip->rss_address[0] & 0xf8) == 0x20;
            chip->rss_write28 = addr_match && (chip->rss_address[0] & 0xf8) == 0x28;
            memcpy(&chip->rss_reg_pan_tl[1][0], &chip->rss_reg_pan_tl[0][0], 6 * sizeof(unsigned char));
            memcpy(&chip->rss_reg_start_l[1][0], &chip->rss_reg_start_l[0][0], 6 * sizeof(unsigned char));
            memcpy(&chip->rss_reg_start_h[1][0], &chip->rss_reg_start_h[0][0], 6 * sizeof(unsigned char));
            memcpy(&chip->rss_reg_stop_l[1][0], &chip->rss_reg_stop_l[0][0], 6 * sizeof(unsigned char));
            memcpy(&chip->rss_reg_stop_h[1][0], &chip->rss_reg_stop_h[0][0], 6 * sizeof(unsigned char));
        }
#endif

        int rss_fm_match = chip->rss_fmcnt[1] == chip->rss_cnt2[1];

#ifdef FMOPNA_YM2608
        if (chip->clk1 && rss_fm_match)
        {
            chip->rss_params[0] = chip->reg_rss[1][5];
        }
        if (chip->rss_cnt1[1] == 3)
        {
            chip->rss_params[1] = chip->rss_params[0];
        }
#endif
#ifdef FMOPNA_YM2610
        if (chip->clk1)
            chip->rss_fm_match_l = rss_fm_match;
        if (rss_fm_match && !chip->rss_fm_match_l)
        {
            chip->rss_params[0] = chip->rss_reg_pan_tl[1][5];
            chip->rss_params_start[0] = chip->rss_reg_start_l[1][4] | (chip->rss_reg_start_h[1][4] << 8);
            chip->rss_params_stop[0] = chip->rss_reg_stop_l[1][4] | (chip->rss_reg_stop_h[1][4] << 8);
        }
        if (chip->rss_cnt1[1] == 1)
        {
            chip->rss_params_start_h = (chip->rss_params_start[2] >> 12) & 15;
        }
        if (chip->rss_cnt1[1] == 3)
        {
            chip->rss_params[1] = chip->rss_params[0];
            chip->rss_params_start[1] = chip->rss_params_start[0];
            chip->rss_params_stop[1] = chip->rss_params_stop[0];
        }
        if (chip->rss_cnt1[1] == 5)
        {
            chip->rss_params_stop[2] = chip->rss_params_stop[1];
            chip->rss_params_start[2] = chip->rss_params_start[1] & 0xfff;
        }
#endif
        if (chip->rss_cnt1[1] == 3)
        {
            chip->rss_tl_l = chip->rss_tl[1];

            chip->rss_tl_shift[1] = chip->rss_tl_shift[0];
        }
        int tl = (((chip->rss_params[1] & 0x1f) | 0x20) + chip->rss_tl_l + 1) ^ 63;
        int key = (chip->rss_key[1] & 0x20) != 0;
        if ((tl & 64) == 0 || !key)
            tl = 63;
        else
            tl &= 63;
        if (chip->rss_cnt1[1] == 5)
        {
            chip->rss_tl_shift[0] = (tl >> 3);
            chip->rss_tl_shift[2] = chip->rss_tl_shift[1];
        }

        int dbg_load = ((chip->reg_test_12[1] & 16) != 0 && chip->rss_cnt1[1] == 5)
            || ((chip->reg_test_12[1] & 8) != 0 && chip->rss_cnt1[1] == 1);

        if (chip->aclk1)
        {
            int load = chip->rss_cnt1[1] == 5 && tl != 63;
            chip->rss_multi_ctrl[0] = chip->rss_multi_ctrl[1] >> 1;
            if (load)
            {
                chip->rss_multi_ctrl[0] |= (15 - (tl & 7)) << 1;
            }

            chip->rss_sample_load = chip->rss_cnt1[1] == 5;
            chip->rss_sample_shift_load = chip->rss_cnt1[1] == 0;

            int add1 = 0;
            if (chip->rss_multi_ctrl[1] & 1)
            {
                add1 = chip->rss_sample;
                if (add1 & 0x800)
                    add1 |= 0x1000;
            }
            int add2 = 0;
            if (chip->rss_cnt1[1] != 5)
            {
                add2 = chip->rss_multi_accum[1] >> 1;
                if (add2 & 0x800)
                    add2 |= 0x1000;
            }
            chip->rss_multi_accum[0] = (add1 + add2) & 0x1fff;

            chip->rss_dbg_load = dbg_load;
        }
        if (chip->aclk2)
        {
            chip->rss_multi_ctrl[1] = chip->rss_multi_ctrl[0];

            chip->rss_multi_accum[1] = chip->rss_multi_accum[0];
        }

        if (chip->rss_cnt1[1] == 5 && !chip->rss_sample_load)
        {
            chip->rss_sample = chip->rss_regs[1][16] & 0xfff;

            chip->rss_multi_accum_load = chip->rss_multi_accum[1];
        }

        if (chip->rss_cnt1[1] == 0 && !chip->rss_sample_shift_load)
        {
            int sample = chip->rss_multi_accum_load;
            if (sample & 0x1000)
                sample |= ~0x1fff;
            chip->rss_sample_shift = sample >> chip->rss_tl_shift[2];
        }

        if (dbg_load && !chip->rss_dbg_load)
        {
#ifdef FMOPNA_YM2608
            chip->rss_dbg_data = chip->rss_regs[1][16] | (chip->rss_isend << 15);
#else
            chip->rss_dbg_data = chip->rss_regs[1][16] & 0xfff;
#endif
        }


        if (chip->rss_eclk1)
        {
            chip->write3_trig1 = chip->write3_trig0;
            chip->write3_l[1] = chip->write3_l[0];
        }
        if (chip->rss_eclk2)
        {
            chip->write3_l[0] = chip->write3_trig1;
            chip->write3_l[2] = chip->write3_l[1];
        }
        chip->write3_en = chip->write3_l[0] && !chip->write3_l[2];

        if (chip->rss_eclk1)
        {
            if (chip->ic)
            {
                chip->rss_keydm[0] = 0;
                chip->rss_keymask[0] = 0;
            }
            else
            {
#ifdef FMOPNA_YM2608
                int key_write = chip->write3_en && chip->addr_10[1] && (chip->data_bus1 & 0x100) == 0;
#else
                int key_write = chip->write3_en && chip->addr_00[1] && (chip->data_bus1 & 0x100) != 0;
#endif
                if (key_write)
                {
                    chip->rss_keydm[0] = (chip->data_bus1 & 0x80) != 0;
                    chip->rss_keymask[0] = chip->data_bus1 & 0x3f;
                }
                else
                {
                    chip->rss_keydm[0] = chip->rss_keydm[1];
                    chip->rss_keymask[0] = chip->rss_keymask[1];
                }
            }

            int mask = chip->rss_keymask[1] & (1 << chip->rss_cnt2[1]);

            int key_event = mask != 0;

            chip->rss_keymask[0] &= ~mask;

            int kon_event = key_event && !chip->rss_keydm[1];
            int koff_event = key_event && chip->rss_keydm[1];

            chip->rss_ic[0] = (chip->rss_ic[1] << 1) | chip->ic;

            int ic = chip->ic || (chip->rss_ic[1] & 0x3f) != 0;

            int key = (chip->rss_key[1] & 0x20) != 0;

            int key_next = !ic && !koff_event && (key || kon_event);

            chip->rss_key[0] = (chip->rss_key[1] << 1) | key_next;

            int stop = (chip->rss_stop[1] & 0x20) != 0;

            int eos = chip->rss_eos_l && !(chip->reg_test_12[1] & 16);

            int stop_next = !kon_event && (stop || ic || koff_event || eos);

            chip->rss_stop[0] = (chip->rss_stop[1] << 1) | stop_next;


#ifdef FMOPNA_YM2610

            chip->rss_stop_flag[0] = 0;
            if (!stop && eos)
                chip->rss_stop_flag[0] |= 1 << chip->rss_cnt2[1];
#endif

        }
        if (chip->rss_eclk2)
        {
            chip->rss_keydm[1] = chip->rss_keydm[0];
            chip->rss_keymask[1] = chip->rss_keymask[0];

            chip->rss_ic[1] = chip->rss_ic[0];

            chip->rss_key[1] = chip->rss_key[0];

            chip->rss_stop[1] = chip->rss_stop[0];
#ifdef FMOPNA_YM2610
            chip->rss_stop_flag[1] = chip->rss_stop_flag[0];
#endif
        }

#ifdef FMOPNA_YM2608
        if (chip->rss_cnt1[1] == 0)
        {
            chip->rss_eos_l = chip->rss_isend;
            chip->rss_step = (chip->rss_ix & 1) == 0;
        }
#endif
#ifdef FMOPNA_YM2610
        if (chip->aclk1)
            chip->rss_eos_load = chip->rss_cnt1[1] == 0;
        if (chip->rss_cnt1[1] == 0 && !chip->rss_eos_load)
        {
            chip->rss_eos_l = (chip->rss_ix >> 1) == ((chip->rss_params_stop[2] << 8) | 0xff);

            chip->rss_nibble = chip->rss_rad_bus;

            if (chip->rss_ix & 1)
                chip->rss_nibble &= 15;
            else
                chip->rss_nibble >>= 4;
        }
#endif

        static const int rss_delta[64] = {
            16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66,
            73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
            337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411,
            1552, 1552, 1552, 1552, 1552, 1552, 1552, 1552, 1552, 1552, 1552, 1552, 1552, 1552, 1552, 1552
        };

        static const int rss_adjust[8] = {
            63, 63, 63, 63, 2, 5, 7, 9
        };

#ifdef FMOPNA_YM2608
        static const int rss_start[8] = {
            0, 0x1c0, 0x440, 0x1b80, 0x1d00, 0x1f80, 0, 0
        };
#endif

        {

            if (chip->aclk1)
            {

                int mask = chip->rss_keymask[1] & (1 << chip->rss_cnt2[1]);

                int key_event = mask != 0;

                int kon_event = key_event && !chip->rss_keydm[1];
                int koff_event = key_event && chip->rss_keydm[1];

                int eos = chip->rss_eos_l && !(chip->reg_test_12[1] & 16);

                int stop = (chip->rss_stop[1] & 0x20) != 0;

                int nibble = chip->rss_nibble;

                int adjust = rss_adjust[chip->rss_nibble & 7];

                if (chip->reg_test_12[1] & 16)
                {
                    nibble = 4;
                    adjust = 1;
                }

                int c5 = chip->rss_cnt1[1] == 2;
                int c6 = chip->rss_cnt1[1] == 3 || chip->rss_cnt1[1] == 4;
                int c8 = (!kon_event && chip->rss_cnt1[1] == 0) || (!kon_event && chip->rss_cnt1[1] == 1)
                    || (chip->rss_cnt1[1] == 5 && !eos && !kon_event && !(chip->reg_test_12[1] & 16));

                int c0 = !kon_event && !eos && (nibble & 8) != 0 && chip->rss_cnt1[1] == 5
#ifdef FMOPNA_YM2608
                    && chip->rss_step
#endif
                    ;
                int c1 = !kon_event && !eos && (nibble & 8) == 0 && chip->rss_cnt1[1] == 5
#ifdef FMOPNA_YM2608
                    && chip->rss_step
#endif
                    ;
                int c2 = !kon_event && chip->rss_cnt1[1] == 1
#ifdef FMOPNA_YM2608
                    && chip->rss_step
#endif
                    ;
                int c3 = kon_event && chip->rss_cnt1[1] == 0;
                int c4 = !kon_event && !eos && !stop && chip->rss_cnt1[1] == 0;
                int c7 = (chip->rss_cnt1[1] == 2 && (nibble & 1) != 0) || (chip->rss_cnt1[1] == 3 && (nibble & 2) != 0)
                    || (chip->rss_cnt1[1] == 4 && (nibble & 4) != 0);

                int delta = rss_delta[chip->rss_delta_ix];

                // c5 c6 c8
                // c0 c1 c2 c3 c4 c7

                int carry = c4 || c0;
                int add1 = 0;
                int add2 = 0;
                if (c7)
                {
                    add1 |= delta;
                }
                if (c1)
                {
                    add1 |= chip->rss_accum[1] & 0xfff;
                }
                if (c0)
                {
                    add1 |= (chip->rss_accum[1] & 0xfff) ^ 0xfff;
                }
                if (c2)
                {
                    add1 |= adjust;
                }
                if (c3)
                {
#ifdef FMOPNA_YM2608
                    add1 |= rss_start[chip->rss_cnt2[1]] << 2;
#else
                    add1 |= (chip->rss_params_start[2] & 0xfff) << 9;
#endif
                }
#ifdef FMOPNA_YM2608
                if (c4)
                {
                    if (chip->rss_cnt2[1] == 0 || chip->rss_cnt2[1] == 1 || chip->rss_cnt2[1] == 2 || chip->rss_cnt2[1] == 3)
                        add1 |= 1;
                }
#endif

                if (c8)
                {
                    add2 |= chip->rss_regs[1][16];
                }
                if (c6)
                {
                    add2 |= (chip->rss_accum[1] >> 1) & 0x7ff;
                }
                if (c5)
                {
                    add2 |= delta >> 1;
                }

#ifdef FMOPNA_YM2608
                chip->rss_accum[0] = (add1 + add2 + carry) & 0x7fff;
#else
                chip->rss_accum[0] = (add1 + add2 + carry) & 0x1fffff;
#endif

                chip->rss_delta_ix_load = chip->rss_cnt1[1] == 5;
                chip->rss_ix_load = chip->rss_cnt1[1] == 1;

                chip->rss_cnt1_is1 = chip->rss_cnt1[1] == 1;
            }
            if (chip->aclk2)
            {
                chip->rss_accum[1] = chip->rss_accum[0];
                if (chip->rss_cnt1_is1)
                {
                    if ((chip->rss_accum[0] & 0x3f) == 0x3f)
                        chip->rss_accum[1] &= ~0x3f;
                    if ((chip->rss_accum[0] & 0x38) == 0x30 || (chip->rss_accum[0] & 0x3c) == 0x38)
                        chip->rss_accum[1] &= ~0xf;
                }
            }

            if (chip->rss_cnt1[1] == 5 && !chip->rss_delta_ix_load)
            {
                chip->rss_delta_ix = chip->rss_regs[1][14] & 63;
            }

#ifdef FMOPNA_YM2608
            if (chip->rss_cnt1[1] == 1 && !chip->rss_ix_load)
            {
                chip->rss_ix = chip->rss_regs[1][14] & 0x7fff;

                int ix = chip->rss_ix >> 2;

                chip->rss_isend = ix == 0x1bf || ix == 0x43f ||
                    ix == 0x1b7f || ix == 0x1cff ||
                    ix == 0x1f7f || ix == 0x1fff;
            }

            if (chip->rss_cnt1[1] == 0)
            {
                chip->rss_nibble = rss_rom[(chip->rss_ix >> 2) & 0x1fff];
                if (chip->rss_ix & 2)
                    chip->rss_nibble >>= 4;
                else
                    chip->rss_nibble &= 15;
            }
#endif
#ifdef FMOPNA_YM2610
            if (chip->rss_cnt1[1] == 1 && !chip->rss_ix_load)
            {
                chip->rss_ix = chip->rss_regs[1][14] & 0x1fffff;
            }
#endif

            if (chip->rss_fclk1)
            {
                memcpy(&chip->rss_regs[0][1], &chip->rss_regs[1][0], 16 * sizeof(int));
                chip->rss_regs[0][0] = chip->rss_accum[1];
            }
            if (chip->rss_fclk2)
            {
                memcpy(&chip->rss_regs[1][0], &chip->rss_regs[0][0], 17 * sizeof(int));
            }

        }
        if (chip->rss_cnt1[1] == 5)
        {
            chip->rss_pan[0] = (chip->rss_params[1] >> 6) & 3;
            chip->rss_pan[2] = chip->rss_pan[1];
        }
        if (chip->rss_cnt1[1] == 3)
        {
            chip->rss_pan[1] = chip->rss_pan[0];
        }
#ifdef FMOPNA_YM2610
        int is1_2 = chip->rss_cnt1[1] == 1 || chip->rss_cnt1[1] == 2;
        if (chip->aclk1)
        {
            chip->rss_rmpx[0] = is1_2;
            chip->rss_roe[0] = chip->rss_rmpx[1];
            chip->rss_roe[2] = chip->rss_roe[1];
        }
        if (chip->aclk2)
        {
            chip->rss_rmpx[1] = chip->rss_rmpx[0];
            chip->rss_roe[1] = chip->rss_roe[0];
            chip->rss_roe[3] = chip->rss_roe[2];
        }

        chip->o_rmpx = chip->rss_rmpx[1];
        chip->o_roe = !chip->rss_roe[1];
        int io_dir = is1_2 || chip->rss_roe[1];
        if (io_dir)
        {
            chip->rss_rad_bus = is1_2 ? (chip->rss_ix >> 1) & 255 : (chip->rss_ix >> 11) & 255;
        }
        else
        {
            chip->rss_rad_bus = chip->input.rad;
        }
        chip->o_ra8 = is1_2 ? (chip->rss_ix >> 9) & 3 : (chip->rss_ix >> 19) & 3;
        chip->o_ra20 = chip->rss_params_start_h;
        chip->o_rad = chip->rss_rad_bus;
        chip->o_rad_d = !io_dir;
#endif
    }

    // adpcm-b
    {
        if (chip->write2)
        {
#ifdef FMOPNA_YM2608
            chip->ad_is0 = (chip->data_bus1 & 0xff) == 0x0;
            chip->ad_is1 = (chip->data_bus1 & 0xff) == 0x1;
            chip->ad_is2 = (chip->data_bus1 & 0xff) == 0x2;
            chip->ad_is3 = (chip->data_bus1 & 0xff) == 0x3;
            chip->ad_is4 = (chip->data_bus1 & 0xff) == 0x4;
            chip->ad_is5 = (chip->data_bus1 & 0xff) == 0x5;
            chip->ad_is6 = (chip->data_bus1 & 0xff) == 0x6;
            chip->ad_is7 = (chip->data_bus1 & 0xff) == 0x7;
            chip->ad_is8 = (chip->data_bus1 & 0xff) == 0x8;
            chip->ad_is9 = (chip->data_bus1 & 0xff) == 0x9;
            chip->ad_isa = (chip->data_bus1 & 0xff) == 0xa;
            chip->ad_isb = (chip->data_bus1 & 0xff) == 0xb;
            chip->ad_isc = (chip->data_bus1 & 0xff) == 0xc;
            chip->ad_isd = (chip->data_bus1 & 0xff) == 0xd;
            chip->ad_ise = (chip->data_bus1 & 0xff) == 0xe;
            chip->ad_isf = (chip->data_bus1 & 0xff) == 0xf;
#else
            chip->ad_is0 = (chip->data_bus1 & 0xff) == 0x10;
            chip->ad_is1 = (chip->data_bus1 & 0xff) == 0x11;
            chip->ad_is2 = (chip->data_bus1 & 0xff) == 0x12;
            chip->ad_is3 = (chip->data_bus1 & 0xff) == 0x13;
            chip->ad_is4 = (chip->data_bus1 & 0xff) == 0x14;
            chip->ad_is5 = (chip->data_bus1 & 0xff) == 0x15;
            chip->ad_is9 = (chip->data_bus1 & 0xff) == 0x19;
            chip->ad_isa = (chip->data_bus1 & 0xff) == 0x1a;
            chip->ad_isb = (chip->data_bus1 & 0xff) == 0x1b;
#endif
        }

        if (chip->ic)
        {
#ifdef FMOPNA_YM2608
            chip->ad_reg_spoff = 0;
            chip->ad_reg_memdata = 0;
            chip->ad_reg_rec = 0;
            chip->ad_reg_rom = 0;
            chip->ad_reg_ramtype = 0;
            chip->ad_reg_da_ad = 0;
            chip->ad_reg_sample = 0;
#endif
            chip->ad_reg_reset = 0;
            chip->ad_reg_repeat = 0;
            chip->ad_reg_start = 0;
            chip->ad_reg_r = 0;
            chip->ad_reg_l = 0;
        }
        else if (chip->write3)
        {
            if (chip->ad_is0)
            {
#ifdef FMOPNA_YM2608
                chip->ad_reg_spoff = (chip->data_bus1 >> 3) & 1;
                chip->ad_reg_memdata = (chip->data_bus1 >> 5) & 1;
                chip->ad_reg_rec = (chip->data_bus1 >> 6) & 1;
#endif
                chip->ad_reg_reset = (chip->data_bus1 >> 0) & 1;
                chip->ad_reg_repeat = (chip->data_bus1 >> 4) & 1;
                chip->ad_reg_start = (chip->data_bus1 >> 7) & 1;
            }
            if (chip->ad_is1)
            {
#ifdef FMOPNA_YM2608
                chip->ad_reg_rom = (chip->data_bus1 >> 0) & 1;
                chip->ad_reg_ramtype = (chip->data_bus1 >> 1) & 1;
                chip->ad_reg_da_ad = (chip->data_bus1 >> 2) & 1;
                chip->ad_reg_sample = (chip->data_bus1 >> 3) & 1;
#endif
                chip->ad_reg_r = (chip->data_bus1 >> 6) & 1;
                chip->ad_reg_l = (chip->data_bus1 >> 7) & 1;
            }
        }

        if (chip->write3)
        {
            if (chip->ad_is9)
            {
                chip->ad_reg_delta_l = chip->data_bus1 & 255;
            }
            if (chip->ad_isa)
            {
                chip->ad_reg_delta_h = chip->data_bus1 & 255;
            }
            if (chip->ad_is2)
            {
                chip->ad_reg_start_l = chip->data_bus1 & 255;
            }
            if (chip->ad_is3)
            {
                chip->ad_reg_start_h = chip->data_bus1 & 255;
            }
            if (chip->ad_is4)
            {
                chip->ad_reg_stop_l = chip->data_bus1 & 255;
            }
            if (chip->ad_is5)
            {
                chip->ad_reg_stop_h = chip->data_bus1 & 255;
            }
            if (chip->ad_isb)
            {
                chip->ad_reg_level = chip->data_bus1 & 255;
            }
#ifdef FMOPNA_YM2608
            if (chip->ad_isc)
            {
                chip->ad_reg_limit_l = chip->data_bus1 & 255;
            }
            if (chip->ad_isd)
            {
                chip->ad_reg_limit_h = chip->data_bus1 & 255;
            }
            if (chip->ad_is6)
            {
                chip->ad_reg_prescale_l = chip->data_bus1 & 255;
            }
            if (chip->ad_is7)
            {
                chip->ad_reg_prescale_h = chip->data_bus1 & 7;
            }
            if (chip->ad_ise)
            {
                chip->ad_da_data = (chip->data_bus1 & 255) ^ 128;
            }
#endif
        }

        int reset = chip->ad_reg_reset || chip->ic;

#ifdef FMOPNA_YM2608
        int mode1 = chip->ad_reg_memdata && chip->ad_reg_rec && !chip->ad_start_l[2]
            && (chip->ad_w2_l[1] & 16) != 0
            && (chip->ad_write_port_l[1] & 4) != 0; // cpu->mem
        int mode2 = !chip->ad_start_l[2] && (chip->ad_w2_l[1] & 16) != 0;
        int mode3 = !chip->ad_reg_memdata && !chip->ad_start_l[2];
        int mode4 = chip->ad_reg_memdata && chip->ad_start_l[2]
            && (chip->ad_w2_l[1] & 16) != 0;
        int mode5 = chip->ad_reg_memdata && chip->ad_reg_rec && chip->ad_start_l[2]; // enc->mem
        int mode6 = chip->ad_reg_memdata && !chip->ad_reg_rec && chip->ad_start_l[2]; // mem->dec
        int mode7 = !chip->ad_reg_ramtype && chip->ad_reg_rom && chip->ad_reg_memdata
            && !chip->ad_reg_rec && !chip->ad_start_l[2] && (chip->ad_w2_l[1] & 16) != 0
            && (chip->ad_read_port_l[1] & 4) != 0; // mem(rom)->cpu
        int mode8 = !chip->ad_reg_memdata && !chip->ad_reg_rec && chip->ad_start_l[2]
            && (chip->ad_write_port_l[1] & 4) != 0; // cpu->dec
        int mode9 = chip->ad_reg_ramtype && !chip->ad_reg_rom && chip->ad_reg_memdata
            && !chip->ad_reg_rec && !chip->ad_start_l[2] && (chip->ad_w2_l[1] & 16) != 0
            && (chip->ad_read_port_l[1] & 4) != 0; // mem(8bit)->cpu
        int mode10 = !chip->ad_reg_memdata && chip->ad_reg_rec && chip->ad_start_l[2]
            && (chip->ad_read_port_l[1] & 4) != 0; // enc->cpu
        int mode11 = !chip->ad_reg_ramtype && !chip->ad_reg_rom && chip->ad_reg_memdata
            && !chip->ad_reg_rec && !chip->ad_start_l[2] && (chip->ad_w2_l[1] & 16) != 0
            && (chip->ad_read_port_l[1] & 4) != 0; // mem(1bit)->cpu
#else
        int mode6 = chip->ad_start_l[2];
#endif

        int code_end = chip->ad_code_end[1] && (chip->ad_w2_l[1] & 16) != 0;

        int repeat = chip->ad_reg_repeat && code_end && !chip->ad_code_ed_end[1];

#ifdef FMOPNA_YM2608
        int w28 = repeat || (!chip->ad_mode6_l[1] && mode6);

        int w6 = mode7 || mode9 || (w28 &&
            ((!chip->ad_reg_rom && chip->ad_reg_ramtype)
                || (!chip->ad_reg_ramtype && chip->ad_reg_rom)));

        int sync = chip->ad_mem_sync_run || !chip->ad_mem_w20[1];
        int w16 = !chip->ad_mem_w10[1] && sync;

        int w18 = !(chip->ad_mem_w17[1] && (sync || !chip->ad_reg_memdata));

        int p5 = (chip->ad_code_ctrl_l & 1) != 0 && !chip->ad_reg_rec && chip->ad_start_l[2] && (w16 || !chip->ad_reg_memdata);

        int w19 = !(w18 && chip->ad_start_l[2] && !p5);

        int p1 = chip->ad_reg_rec && w16 && w19 && chip->ad_reg_memdata && chip->ad_mem_w15[1];
        int p2 = !chip->ad_reg_rec && w16 && w19 && chip->ad_reg_memdata && chip->ad_mem_w15[1];
        int p3 = chip->ad_reg_rec && !w18;
        int p4 = (!chip->ad_reg_rec && (w16 || !chip->ad_reg_memdata) && !w18) || p5;

        int w23 = p1 || p2;

        int w24 = (p4 && !chip->ad_reg_memdata) || (!chip->ad_start_l[2] && p1);

        int w25 = p2 || p3;

        int w26 = mode5 && !chip->ad_rec_start_l[1];
        int w27 = mode11 || (!chip->ad_reg_rom && w28 && !chip->ad_reg_ramtype);

        int w29 = w28 || w26 || mode10 || mode8; // start dsp
#else
        int w28 = repeat || (!chip->ad_mode6_l[1] && mode6);
        int w16 = !chip->ad_mem_w10[1];

        int w18 = !chip->ad_mem_w17[1];

        int p5 = (chip->ad_code_ctrl_l & 1) != 0 && chip->ad_start_l[2] && w16;

        int w19 = !(w18 && !p5);

        int p2 = w16 && w19 && chip->ad_mem_w15[1];
        int p4 = (w16 && !w18) || p5;

        int w23 = p2;

        int w29 = w28; // start dsp
#endif

        if (chip->cclk1)
        {
#ifdef FMOPNA_YM2608
            chip->ad_sample_l[1] = chip->ad_sample_l[0];

            int read_sample = chip->read3 && chip->ad_is8;
            int write_sample = chip->write3 && chip->ad_is8;
            chip->ad_read_port_l[0] = (chip->ad_read_port_l[1] << 1) | read_sample;
            chip->ad_write_port_l[0] = (chip->ad_write_port_l[1] << 1) | write_sample;

            chip->ad_rw_l[1] = chip->ad_rw_l[0];

            int w3 = chip->ad_addr_isend_l && chip->ad_reg_rec;
#endif
            chip->ad_start_l[1] = chip->ad_start_l[0];

            int t1 = !chip->ad_addr_isend_l2[1] || chip->ad_addr_isend_l;

#ifdef FMOPNA_YM2608
            chip->ad_w2[0] = (chip->ad_w2[1] && t1) || (chip->ad_rw_en && chip->ad_w4[1])
                || (t1 && (reset || w3 || (chip->ad_w4[1] && chip->ad_start_l[2])));

            chip->ad_w4[0] = (chip->ad_w4[1] && t1) || w3 || (chip->ad_addr_isend_l && p2);
#else
            chip->ad_w2[0] = (chip->ad_w2[1] && t1)
                || (t1 && (reset || (chip->ad_w4[1] && chip->ad_start_l[2])));

            chip->ad_w4[0] = (chip->ad_w4[1] && t1) || (chip->ad_addr_isend_l && p2);
#endif

            chip->ad_w2_l[0] = (chip->ad_w2_l[1] << 1) | chip->ad_w2[1];

            chip->ad_addr_isend_l2[0] = chip->ad_addr_isend_l;

#ifdef FMOPNA_YM2608
            int t2 = mode2 || mode3 || mode4;
#else
            int t2 = (chip->ad_w2_l[1] & 16) != 0;
#endif

            chip->ad_w12[0] = chip->ad_w12[1] << 1;
            chip->ad_w12[0] |= t2;

            chip->ad_w13[0] = (chip->ad_w12[1] & 3) == 1;

            chip->ad_mode6_l[0] = mode6;

            chip->ad_code_ed_end[0] = code_end;
            
        }
        if (chip->cclk2)
        {
#ifdef FMOPNA_YM2608
            chip->ad_sample_l[0] = chip->ad_reg_sample;
            chip->ad_sample_l[2] = chip->ad_sample_l[1];

            chip->ad_read_port_l[1] = chip->ad_read_port_l[0];
            chip->ad_write_port_l[1] = chip->ad_write_port_l[0];

            chip->ad_rw_l[0] = chip->ad_rw_l[1] << 1;
            chip->ad_rw_l[0] |= (chip->ad_read_port_l[0] & 1) != 0 || (chip->ad_write_port_l[0] & 1) != 0;

            chip->ad_rw_en = (chip->ad_rw_l[0] & 1) == 0 && (chip->ad_rw_l[0] & 2) != 0;

            chip->ad_rec_start_l[1] = chip->ad_rec_start_l[0];
#endif

            chip->ad_start_l[0] = chip->ad_reg_start;
            chip->ad_start_l[2] = chip->ad_start_l[1];

            chip->ad_w2[1] = chip->ad_w2[0];

            chip->ad_w4[1] = chip->ad_w4[0];

            chip->ad_w2_l[1] = chip->ad_w2_l[0];

            chip->ad_addr_isend_l = chip->ad_stop_match2[0];

            chip->ad_addr_isend_l2[1] = chip->ad_addr_isend_l2[0];

            chip->ad_w12[1] = chip->ad_w12[0];

            chip->ad_w13[1] = chip->ad_w13[0];
            chip->ad_mode6_l[1] = chip->ad_mode6_l[0];

            chip->ad_code_ed_end[1] = chip->ad_code_ed_end[0];
        }
#ifdef FMOPNA_YM2608
        int cond = 0;

        if (chip->cclk1)
        {
            if (chip->ic || (chip->ad_mem_ctrl_l & 64) != 0)
                chip->ad_mem_sync[0] = 0;
            else
                chip->ad_mem_sync[0] = (chip->ad_mem_sync[1] + chip->ad_mem_sync_run) & 63;

            int next_ptr = 0;
            chip->ad_mem_ctrl = 0;

            int cond_next = 0;

            if (mode1)
            {
                cond_next |= 1;
            }
            if (w26)
            {
                cond_next |= 2;
            }
            if (w27)
            {
                cond_next |= 4;
            }
            if (w6)
            {
                cond_next |= 8;
            }
            if (w23)
            {
                cond_next |= 16;
            }

            if (sync)
            {
                cond = chip->ad_mem_cond[1];
            }
            else
            {
                cond_next |= chip->ad_mem_cond[1];
            }

            if ((cond & 15) == 0)
                cond |= 32;

            chip->ad_mem_cond[0] = cond_next;

            int store_addr = 0;

#if 0
            if ((chip->ad_mem_code_ptr[1] & 0x2f) == 0xf)
            {
                next_ptr |= 0;
                chip->ad_mem_ctrl |= 0;
            }
#endif
            // common code
            switch (chip->ad_mem_code_ptr[1] & 0xf)
            {
                case 0x0:
                    chip->ad_mem_ctrl |= 0b00100000000000;
                    break;
                case 0x1:
                    chip->ad_mem_ctrl |= 0b00100000000000;
                    break;
                case 0x2:
                    chip->ad_mem_ctrl |= 0b00100000000000;
                    break;
                case 0x3:
                    chip->ad_mem_ctrl |= 0b10011000000000;
                    break;
                case 0x4:
                    chip->ad_mem_ctrl |= 0b10111000010000;
                    break;
                case 0x5:
                    chip->ad_mem_ctrl |= 0b11010000000000;
                    break;
                case 0x6:
                    chip->ad_mem_ctrl |= 0b11110000000000;
                    break;
                case 0x7:
                    chip->ad_mem_ctrl |= 0b11000000000001;
                    break;
                case 0x8:
                    chip->ad_mem_ctrl |= 0b11100000000001;
                    break;
            }

            switch (chip->ad_mem_code_ptr[1])
            {
                case 0x9:
                    chip->ad_mem_ctrl |= 0b11100000000010;
                    break;
                case 0xa:
                    chip->ad_mem_ctrl |= 0b11000010000000;
                    break;
                case 0xb:
                    chip->ad_mem_ctrl |= 0b00000110000000;
                    break;
                case 0xd:
                    if (sync)
                    {
                        if (chip->ad_stop_match2[1])
                        {
                            next_ptr |= 0x3f|0x40;
                        }
                        if (!chip->ad_mem_w22)
                        {
                            next_ptr |= 0x4|0x40;
                            chip->ad_mem_ctrl |= 0b10011000000000;
                        }
                        if ((cond & 16) == 0 && chip->ad_mem_w22)
                        {
                            next_ptr |= 0xd|0x40;
                        }
                        if ((cond & 16) != 0 && chip->ad_mem_w22)
                        {
                            next_ptr |= 0x4|0x40;
                            chip->ad_mem_ctrl |= 0b10011000000000;
                        }
                    }
                    else
                    {
                        next_ptr = 0x3a | 0x40;
                        chip->ad_mem_ctrl |= 0b00100000000000;
                        store_addr = 1;
                    }
                    break;

                case 0x19:
                    chip->ad_mem_ctrl |= 0b00100100001000;
                    break;
                case 0x1b:
                    if (sync)
                    {
                        if (chip->ad_stop_match2[1])
                        {
                            next_ptr |= 0x3f|0x40;
                        }
                        if (!chip->ad_mem_w22)
                        {
                            next_ptr |= 0x14|0x40;
                            chip->ad_mem_ctrl |= 0b10011000000000;
                        }
                        if ((cond & 16) == 0 && chip->ad_mem_w22)
                        {
                            next_ptr |= 0x1b|0x40;
                        }
                        if ((cond & 16) != 0 && chip->ad_mem_w22)
                        {
                            next_ptr |= 0x14|0x40;
                            chip->ad_mem_ctrl |= 0b10011000000000;
                        }
                    }
                    else
                    {
                        next_ptr = 0x3a | 0x40;
                        chip->ad_mem_ctrl |= 0b00100000000000;
                        store_addr = 1;
                    }
                    break;

                case 0x29:
                    chip->ad_mem_ctrl |= 0b00100000000100;
                    break;
                case 0x2b:
                    if (sync)
                    {
                        if (chip->ad_stop_match2[1])
                        {
                            next_ptr |= 0x3f|0x40;
                        }
                        if ((cond & 16) == 0)
                        {
                            next_ptr |= 0x2b|0x40;
                        }
                        if ((cond & 16) != 0)
                        {
                            next_ptr |= 0x24|0x40;
                            chip->ad_mem_ctrl |= 0b10011000000000;
                        }
                    }
                    else
                    {
                        next_ptr = 0x3a | 0x40;
                        chip->ad_mem_ctrl |= 0b00100000000000;
                        store_addr = 1;
                    }
                    break;

                case 0x3a:
                    chip->ad_mem_ctrl |= 0b00100000000000;
                    break;
                case 0x3b:
                    chip->ad_mem_ctrl |= 0b00100000000000;
                    break;
                case 0x3c:
                    chip->ad_mem_ctrl |= 0b10011000000000;
                    break;
                case 0x3d:
                    chip->ad_mem_ctrl |= 0b00111001000000;
                    break;

                case 0x2f:
                    if (sync)
                    {
                        if ((cond & 16) == 0)
                        {
                            next_ptr = 0x2f | 0x40;
                        }
                        else
                        {
                            next_ptr = 0x0 | 0x40;
                            chip->ad_mem_ctrl |= 0b00100000100000;
                        }
                    }
                    else
                    {
                        next_ptr = 0x3a | 0x40;
                        chip->ad_mem_ctrl |= 0b00100000000000;
                        store_addr = 1;
                    }
                    break;

                case 0x3f:
                    if (sync)
                    {
                        if (cond & 1)
                        {
                            next_ptr |= 0x0 | 0x40;
                            chip->ad_mem_ctrl = 0b00100000100000;
                        }
                        if (cond & 2)
                        {
                            next_ptr |= 0x2f | 0x40;
                        }
                        if (cond & 4)
                        {
                            next_ptr |= 0x10 | 0x40;
                            chip->ad_mem_ctrl = 0b00100000100000;
                        }
                        if (cond & 8)
                        {
                            next_ptr |= 0x20 | 0x40;
                            chip->ad_mem_ctrl = 0b00100000100000;
                        }
                        if (cond & 32)
                        {
                            next_ptr |= 0x3f | 0x40;
                        }
                    }
                    else
                    {
                        next_ptr = 0x3a | 0x40;
                        chip->ad_mem_ctrl |= 0b00100000000000;
                        store_addr = 1;
                    }
                    break;
            }

            if ((chip->ad_mem_ctrl_l & 64) != 0)
            {
                next_ptr |= chip->ad_mem_ptr_store | 0x40;
            }

            if (chip->ic)
            {
                next_ptr |= 0x3f | 0x40;
            }

            if (next_ptr & 64)
                chip->ad_mem_code_ptr[0] = next_ptr & 63;
            else
                chip->ad_mem_code_ptr[0] = (chip->ad_mem_code_ptr[1] + 1) & 63;

            if (store_addr)
                chip->ad_mem_ptr_store = chip->ad_mem_code_ptr[1];
        }
        if (chip->cclk2)
        {
            chip->ad_mem_code_ptr[1] = chip->ad_mem_code_ptr[0];
            chip->ad_mem_ctrl_l = chip->ad_mem_ctrl;

            chip->ad_mem_cond[1] = chip->ad_mem_cond[0];

            chip->ad_mem_sync[1] = chip->ad_mem_sync[0];

            chip->ad_mem_sync_run = (chip->ad_mem_sync[0] & 40) != 40;
        }

        if (chip->cclk1)
        {
            chip->ad_end_sel[0] = chip->ad_end_sel[1];

            int stop_val = 0;
            int limit_val = 0;
            if (chip->ad_end_sel[1] & 1)
            {
                stop_val |= ((chip->ad_reg_stop_l & 15) << 5) | 31;
                limit_val |= ((chip->ad_reg_limit_l & 15) << 5) | 31;
            }
            if (chip->ad_end_sel[1] & 4)
            {
                stop_val |= ((chip->ad_reg_stop_h & 31) << 4) | ((chip->ad_reg_stop_l >> 4) & 15);
                limit_val |= ((chip->ad_reg_limit_h & 31) << 4) | ((chip->ad_reg_limit_l >> 4) & 15);
            }
            if (chip->ad_end_sel[1] & 8)
            {
                stop_val |= (chip->ad_reg_stop_h >> 5) & 7;
                limit_val |= (chip->ad_reg_limit_h >> 5) & 7;
            }

            chip->ad_stop_match[0] = chip->ad_stop_match[1] << 1;
            if (stop_val == chip->ad_address_cnt[3][1])
                chip->ad_stop_match[0] |= 1;
            chip->ad_limit_match[0] = chip->ad_limit_match[1] << 1;
            if (limit_val == chip->ad_address_cnt[3][1])
                chip->ad_limit_match[0] |= 1;

            int rst = (chip->ad_mem_cond[1] & 2) != 0 || (chip->ad_start_sel[1] & 1) != 0;

            int stop_match = (chip->ad_stop_match[0] & 11) == 11 && (chip->ad_end_sel[1] & 8) != 0;

            chip->ad_stop_match2[0] = ((stop_match || chip->ad_stop_match2[1])
                && !rst) || reset;

            chip->ad_limit_match2[0] = (chip->ad_limit_match[0] & 11) == 11 && (chip->ad_end_sel[1] & 8) != 0;

            chip->ad_start_sel[0] = chip->ad_start_sel[1];

            int start_val = 0;
            if (chip->ad_start_sel[1] & 1)
                start_val |= ((chip->ad_reg_start_l & 15) << 5);
            if (chip->ad_start_sel[1] & 2)
                start_val |= ((chip->ad_reg_start_h & 31) << 4) | ((chip->ad_reg_start_l >> 4) & 15);
            if (chip->ad_start_sel[1] & 4)
                start_val |= (chip->ad_reg_start_h >> 5) & 7;

            chip->ad_address_cnt[0][0] = 0;
            chip->ad_address_cnt[1][0] = 0;
            chip->ad_address_cnt[2][0] = 0;
            chip->ad_address_cnt[3][0] = 0;

            int add = chip->ad_address_cnt[3][1];

            if ((chip->ad_mem_ctrl_l & 0x200) != 0 || chip->ad_address_carry[1])
                add++;

            int carry = ((add >> 9) & 1) != 0 && (chip->ad_mem_ctrl_l & 0x40) == 0;
            add &= 0x1ff;

            if (!chip->ad_limit_match2[1])
            {
                if (chip->ad_mem_ctrl_l & 0x800)
                {
                    if ((chip->ad_start_sel[1] & 7) != 0)
                        chip->ad_address_cnt[0][0] = start_val;
                    else
                        chip->ad_address_cnt[0][0] = add;
                    chip->ad_address_cnt[1][0] = chip->ad_address_cnt[0][1];
                    chip->ad_address_cnt[2][0] = chip->ad_address_cnt[1][1];
                }
                else if (!chip->ic)
                {
                    chip->ad_address_cnt[0][0] = chip->ad_address_cnt[0][1];
                    chip->ad_address_cnt[1][0] = chip->ad_address_cnt[1][1];
                    chip->ad_address_cnt[2][0] = chip->ad_address_cnt[2][1];
                }
            }
            if (chip->ad_mem_ctrl_l & 0x800)
            {
                chip->ad_address_cnt[3][0] = chip->ad_address_cnt[2][1];
            }
            else if (!chip->ic)
            {
                chip->ad_address_cnt[3][0] = chip->ad_address_cnt[3][1];
            }

            if (chip->ad_mem_ctrl_l & 0x800)
                chip->ad_address_carry[0] = carry;
            else
                chip->ad_address_carry[0] = chip->ad_address_carry[1];
        }
        if (chip->cclk2)
        {
            chip->ad_end_sel[1] = chip->ad_end_sel[0] << 1;
            if (chip->ad_mem_ctrl & 16)
                chip->ad_end_sel[1] |= 1;
            chip->ad_start_sel[1] = chip->ad_start_sel[0] << 1;
            if (chip->ad_mem_ctrl & 32)
                chip->ad_start_sel[1] |= 1;
            chip->ad_stop_match[1] = chip->ad_stop_match[0];
            chip->ad_limit_match[1] = chip->ad_limit_match[0];
            chip->ad_stop_match2[1] = chip->ad_stop_match2[0];
            chip->ad_limit_match2[1] = chip->ad_limit_match2[0];

            chip->ad_address_cnt[0][1] = chip->ad_address_cnt[0][0];
            chip->ad_address_cnt[1][1] = chip->ad_address_cnt[1][0];
            chip->ad_address_cnt[2][1] = chip->ad_address_cnt[2][0];
            chip->ad_address_cnt[3][1] = chip->ad_address_cnt[3][0];

            chip->ad_address_carry[1] = chip->ad_address_carry[0];
        }

        if (chip->cclk1)
        {

            if (chip->ad_mem_ctrl_l & 1)
                chip->ad_mem_addr_bank = chip->ad_address_cnt[3][1] & 7;

            int t1 = 0;
            if (chip->ad_mem_ctrl_l & 1)
            {
                t1 |= chip->ad_mem_bus & 254;
                t1 |= chip->input.dt0;
            }
            if (chip->ad_mem_ctrl_l & 2)
            {
                if (chip->ad_reg_ramtype)
                {
                    t1 |= chip->ad_mem_data_l2;
                }
                else
                {
                    if (chip->ad_mem_data_l2 & (1 << chip->ad_mem_bit_cnt[1]))
                        t1 |= 255;
                }
            }
            if (((chip->ad_mem_ctrl_l & 2) != 0 && chip->ad_reg_ramtype) || (chip->ad_mem_ctrl_l & 1) != 0)
            {
                chip->ad_mem_data_l1 = t1;
            }
            else if (chip->ad_mem_ctrl_l & 2)
            {
                int mask = 1 << chip->ad_mem_addr_bank;
                chip->ad_mem_data_l1 &= ~mask;
                chip->ad_mem_data_l1 |= mask & t1;
            }

            if (chip->ad_mem_ctrl_l & 32)
                chip->ad_mem_bit_cnt[0] = 0;
            else
            {
                int add = !chip->ad_reg_ramtype && (chip->ad_mem_ctrl_l & 0x100) != 0;
                chip->ad_mem_bit_cnt[0] = (chip->ad_mem_bit_cnt[1] + add) & 7;
            }

            int t2 = 0;
            if (p1)
            {
                t2 |= chip->ad_mem_data_bus;
            }
            if (chip->ad_mem_ctrl_l & 4)
            {
                t2 |= chip->ad_mem_data_l1;
            }
            if (chip->ad_mem_ctrl_l & 8)
            {
                if (chip->ad_mem_data_l1 & (1 << chip->ad_mem_addr_bank))
                    t2 |= 255;
            }

            if ((chip->ad_mem_ctrl_l & 4) != 0 || p1)
            {
                chip->ad_mem_data_l2 = t2;
            }
            else if (chip->ad_mem_ctrl_l & 8)
            {
                int mask = 1 << chip->ad_mem_bit_cnt[1];
                chip->ad_mem_data_l2 &= ~mask;
                chip->ad_mem_data_l2 |= mask & t2;
            }

            int t4;
            if (p4)
                t4 = chip->ad_mem_data_bus;
            else if (chip->ad_mem_w7[1])
                t4 = ((chip->ad_mem_data_l4[1] & 127) << 1) | !chip->ad_dsp_enc_bit;
            else
                t4 = chip->ad_mem_data_l4[1];

            chip->ad_mem_data_l4[0] = t4;

            chip->ad_mem_w7[0] = (chip->ad_code_ctrl_l & 0x1000) != 0;
            chip->ad_mem_w8[0] = (chip->ad_code_ctrl_l & 0x800) != 0;

            int add = chip->ad_mem_shift_cnt[1] + chip->ad_mem_w7[1];
            int of = (add & 8) != 0;

            if (!chip->ad_start_l[2])
                chip->ad_mem_shift_cnt[0] = 0;
            else
                chip->ad_mem_shift_cnt[0] = add & 7;

            chip->ad_mem_shift_cnt0_l[0] = (chip->ad_mem_shift_cnt[1] & 3) == 0;

            int mem_en = chip->ad_start_l[2] && chip->ad_reg_memdata;

            chip->ad_mem_mem_en_l[0] = mem_en;

            int w9 = (mem_en && !chip->ad_mem_mem_en_l[1] && !chip->ad_reg_rec) ||
                (!chip->ad_reg_memdata && chip->ad_start_l[2]) ||
                repeat ||
                w23;

            chip->ad_mem_mem_stop[0] = !mem_en && chip->ad_mem_mem_en_l[1];

            int w11 = reset || chip->ad_mem_mem_stop[1] || chip->ad_w13[1] || chip->ad_mem_w21;

            chip->ad_mem_w10[0] = w9 || (chip->ad_mem_w10[1] && !w11);

            int w14 = reset || chip->ad_mem_mem_stop[1] || chip->ad_w13[1] || w16 || (!chip->ad_reg_memdata && (p3 || p4));

            chip->ad_mem_w15[0] = (mem_en || chip->ad_rw_en) || (chip->ad_mem_w15[1] && !w14);

            chip->ad_brdy_set_l[0] = chip->ad_reg_memdata ? (!chip->ad_mem_w15[1] && w16)
                : !chip->ad_mem_w15[1];

            chip->ad_mem_w17[0] = of || (chip->ad_mem_w17[1] && w18);

            chip->ad_mem_rw_en[0] = chip->ad_mem_rw_en[1] << 1;
            chip->ad_mem_rw_en[0] |= chip->ad_rw_en;

            chip->ad_mem_w20[0] = ((chip->ad_mem_rw_en[1] & 2) != 0 || chip->ad_start_l[2] || reset) ||
                (chip->ad_mem_w20[1] && (cond & 32) != 0);

            if (chip->ad_mem_ctrl_l & 32)
                chip->ad_mem_ucnt[0] = 0;
            else
            {
                int inc = !chip->ad_reg_ramtype && (chip->ad_mem_ctrl_l & 0x100) != 0;
                chip->ad_mem_ucnt[0] = (chip->ad_mem_ucnt[1] + inc) & 7;
            }

            chip->ad_mem_we[0] = (chip->ad_mem_ctrl_l & 0x80) != 0;
            chip->ad_mem_cas[0] = (chip->ad_mem_ctrl_l & 0x1000) != 0;
            chip->ad_mem_ras[0] = (chip->ad_mem_ctrl_l & 0x2000) != 0;
        }
        if (chip->cclk2)
        {
            chip->ad_mem_bit_cnt[1] = chip->ad_mem_bit_cnt[0];

            chip->ad_mem_data_l4[1] = chip->ad_mem_data_l4[0];

            chip->ad_mem_w7[1] = chip->ad_mem_w7[0];
            chip->ad_mem_w8[1] = chip->ad_mem_w8[0];

            chip->ad_mem_shift_cnt[1] = chip->ad_mem_shift_cnt[0];

            chip->ad_mem_shift_cnt0_l[1] = chip->ad_mem_shift_cnt0_l[0];

            chip->ad_mem_mem_en_l[1] = chip->ad_mem_mem_en_l[0];

            chip->ad_mem_mem_stop[1] = chip->ad_mem_mem_stop[0];

            chip->ad_mem_w10[1] = chip->ad_mem_w10[0];

            chip->ad_mem_w15[1] = chip->ad_mem_w15[0];

            chip->ad_brdy_set_l[1] = chip->ad_brdy_set_l[0];

            chip->ad_mem_w17[1] = chip->ad_mem_w17[0];

            chip->ad_mem_rw_en[1] = chip->ad_mem_rw_en[0];

            chip->ad_mem_ucnt[1] = chip->ad_mem_ucnt[0];

            chip->ad_mem_w20[1] = chip->ad_mem_w20[0];

            chip->ad_mem_w21 = (chip->ad_mem_ctrl & 4) != 0 ||
                (chip->ad_reg_ramtype && (chip->ad_mem_ctrl & 0x100) != 0) ||
                (!chip->ad_reg_ramtype && (chip->ad_mem_ctrl & 0x100) != 0 && chip->ad_mem_ucnt[0] == 7);

            chip->ad_mem_w22 = chip->ad_mem_ucnt[0] == 0;

            chip->ad_mem_we[1] = chip->ad_mem_we[0];
            chip->ad_mem_cas[1] = chip->ad_mem_cas[0];
            chip->ad_mem_ras[1] = chip->ad_mem_ras[0];

            chip->ad_mem_dir = (chip->ad_mem_ctrl & 0x80) != 0 || (chip->ad_mem_ctrl & 0x400) != 0;
        }

        {

            int nibble_load;
            if (chip->ad_reg_rec)
                nibble_load = (chip->ad_mem_shift_cnt[1] & 3) == 0 && !chip->ad_mem_shift_cnt0_l[1];
            else
                nibble_load = (chip->ad_mem_shift_cnt[1] & 3) == 0 && chip->ad_mem_w7[1];

            if (chip->cclk1)
                chip->ad_mem_nibble_load = nibble_load;

            if (nibble_load && !chip->ad_mem_nibble_load)
            {
                int nibble;
                if (chip->ad_reg_rec)
                    nibble = (chip->ad_mem_data_l4[1] & 15) ^ 8;
                else
                    nibble = (chip->ad_mem_data_l4[1] >> 4) & 15;
                chip->ad_mem_nibble = nibble;
            }

            chip->ad_mem_nibble_msb = chip->ad_mem_w8[1] && (chip->ad_mem_nibble & 8) != 0;
        }

        int write8 = chip->ad_is8 && chip->write3;
        int read8 = chip->ad_is8 && chip->read3;
        if (write8 || w25)
        {
            int t3 = 0;
            if (write8)
                t3 |= chip->data_bus1 & 255;
            if (w25)
                t3 |= chip->ad_mem_data_bus;
            chip->ad_mem_data_l3 = t3;
        }
        if (read8)
        {
            chip->data_bus1 &= ~255;
            chip->data_bus1 |= chip->ad_mem_data_l3 & 255;
        }

        if (!chip->ad_mem_dir)
            chip->ad_mem_bus = chip->input.dm;

        if (chip->ad_mem_ctrl_l & 0x400)
            chip->ad_mem_bus = chip->ad_address_cnt[3][1];
        if (chip->ad_mem_ctrl_l & 0x80)
        {
            chip->ad_mem_bus &= ~255;
            chip->ad_mem_bus = chip->ad_mem_data_l1;
        }
        if (p2)
            chip->ad_mem_data_bus = chip->ad_mem_data_l2;
        if (w24)
            chip->ad_mem_data_bus = chip->ad_mem_data_l3;
        if (p3)
            chip->ad_mem_data_bus = chip->ad_mem_data_l4[1] ^ 0x88;
#else
        if (chip->cclk1)
        {
            int next_ptr = 0;
            chip->ad_mem_ctrl = 0;

            chip->ad_mem_cond[0] = 0;
            if (p2)
                chip->ad_mem_cond[0] |= 1;
            if (w29)
                chip->ad_mem_cond[0] |= 2;

            switch (chip->ad_mem_code_ptr[1])
            {
                case 0x0:
                    chip->ad_mem_ctrl = 0b00100000;
                    break;
                case 0x1:
                    chip->ad_mem_ctrl = 0b01010000;
                    break;
                case 0x2:
                    chip->ad_mem_ctrl = 0b01110100;
                    break;
                case 0x3:
                    chip->ad_mem_ctrl = 0b00010000;
                    break;
                case 0x4:
                    chip->ad_mem_ctrl = 0b00110000;
                    break;
                case 0x5:
                    chip->ad_mem_ctrl = 0b10000000;
                    break;
                case 0x6:
                    chip->ad_mem_ctrl = 0b10000001;
                    break;
                case 0x7:
                    chip->ad_mem_ctrl = 0b00000010;
                    break;
                case 0x9:
                    if (chip->ad_stop_match2[1])
                        next_ptr = 0xf | 0x10;
                    if ((chip->ad_mem_cond[1] & 1) == 0)
                        next_ptr |= 0x9 | 0x10;
                    else
                    {
                        chip->ad_mem_ctrl = 0b01010000;
                        next_ptr |= 0x2 | 0x10;
                    }
                    break;
                case 0xf:
                    if ((chip->ad_mem_cond[1] & 2) != 0)
                    {
                        chip->ad_mem_ctrl = 0b00101000;
                        next_ptr = 0x0 | 0x10;
                    }
                    else
                        next_ptr = 0xf | 0x10;
                    break;
            }
            if (chip->ic)
            {
                next_ptr |= 0xf | 0x10;
            }

            if (next_ptr & 0x10)
                chip->ad_mem_code_ptr[0] = next_ptr & 0xf;
            else
                chip->ad_mem_code_ptr[0] = (chip->ad_mem_code_ptr[1] + 1) & 0xf;
        }
        if (chip->cclk2)
        {
            chip->ad_mem_code_ptr[1] = chip->ad_mem_code_ptr[0];
            chip->ad_mem_ctrl_l = chip->ad_mem_ctrl;

            chip->ad_mem_cond[1] = chip->ad_mem_cond[0];
        }

        if (chip->cclk1)
        {
            chip->ad_end_sel[0] = chip->ad_end_sel[1];
            int stop_val;
            if (chip->ad_end_sel[1] & 1)
            {
                stop_val = ((chip->ad_reg_stop_l & 15) << 8) | 0xff;
            }
            else
            {
                stop_val = (chip->ad_reg_stop_h << 4) | ((chip->ad_reg_stop_l >> 4) & 15);
            }
            chip->ad_stop_match[0] = chip->ad_stop_match[1] << 1;
            if (stop_val == chip->ad_address_cnt[1][1])
                chip->ad_stop_match[0] |= 1;

            int rst = (chip->ad_start_sel[1] & 1) != 0;

            int stop_match = (chip->ad_stop_match[0] & 5) == 5 && (chip->ad_end_sel[1] & 4) != 0;

            chip->ad_stop_match2[0] = ((stop_match || chip->ad_stop_match2[1])
                && !rst) || reset;

            chip->ad_start_sel[0] = chip->ad_start_sel[1];

            if (chip->ad_mem_ctrl_l & 0x20)
            {
                int add = chip->ad_address_cnt[1][1];

                if ((chip->ad_mem_ctrl_l & 0x40) != 0 || chip->ad_address_carry[1])
                    add++;

                int carry = ((add >> 12) & 1) != 0;
                add &= 0xfff;

                int start_val;
                if (chip->ad_start_sel[1] & 1)
                    start_val = ((chip->ad_reg_start_l & 15) << 8);
                else if (chip->ad_start_sel[1] & 2)
                    start_val = (chip->ad_reg_start_h << 4) | ((chip->ad_reg_start_l >> 4) & 15);
                else
                    start_val = add;

                chip->ad_address_cnt[0][0] = start_val;
                chip->ad_address_cnt[1][0] = chip->ad_address_cnt[0][1];
                chip->ad_address_carry[0] = carry;
            }
            else
            {
                chip->ad_address_cnt[0][0] = chip->ad_address_cnt[0][1];
                chip->ad_address_cnt[1][0] = chip->ad_address_cnt[1][1];
                chip->ad_address_carry[0] = chip->ad_address_carry[1];
            }

        }
        if (chip->cclk2)
        {
            chip->ad_end_sel[1] = chip->ad_end_sel[0] << 1;
            if (chip->ad_mem_ctrl & 4)
                chip->ad_end_sel[1] |= 1;
            chip->ad_start_sel[1] = chip->ad_start_sel[0] << 1;
            if (chip->ad_mem_ctrl & 8)
                chip->ad_start_sel[1] |= 1;
            chip->ad_stop_match[1] = chip->ad_stop_match[0];
            chip->ad_stop_match2[1] = chip->ad_stop_match2[0];

            chip->ad_address_cnt[0][1] = chip->ad_address_cnt[0][0];
            chip->ad_address_cnt[1][1] = chip->ad_address_cnt[1][0];

            chip->ad_address_carry[1] = chip->ad_address_carry[0];
        }

        if (chip->cclk1)
        {
            int t4;
            if (p4)
                t4 = chip->ad_mem_data_l3;
            else if (chip->ad_mem_w7[1])
                t4 = (chip->ad_mem_data_l4[1] & 127) << 1;
            else
                t4 = chip->ad_mem_data_l4[1];

            chip->ad_mem_data_l4[0] = t4;

            chip->ad_mem_w7[0] = (chip->ad_code_ctrl_l & 0x1000) != 0;
            chip->ad_mem_w8[0] = (chip->ad_code_ctrl_l & 0x800) != 0;

            int add = chip->ad_mem_shift_cnt[1] + chip->ad_mem_w7[1];
            int of = (add & 8) != 0;

            if (!chip->ad_start_l[2])
                chip->ad_mem_shift_cnt[0] = 0;
            else
                chip->ad_mem_shift_cnt[0] = add & 7;

            int mem_en = chip->ad_start_l[2];

            chip->ad_mem_mem_en_l[0] = mem_en;

            int w9 = (mem_en && !chip->ad_mem_mem_en_l[1]) ||
                repeat ||
                w23;

            chip->ad_mem_mem_stop[0] = !mem_en && chip->ad_mem_mem_en_l[1];

            int w11 = reset || chip->ad_mem_mem_stop[1] || chip->ad_w13[1] || chip->ad_mem_w21;

            chip->ad_mem_w10[0] = w9 || (chip->ad_mem_w10[1] && !w11);

            int w14 = reset || chip->ad_mem_mem_stop[1] || chip->ad_w13[1] || w16;

            chip->ad_mem_w15[0] = mem_en || (chip->ad_mem_w15[1] && !w14);

            chip->ad_mem_w17[0] = of;

            chip->ad_mem_pmpx[0] = (chip->ad_mem_ctrl_l & 0x40) != 0;
            chip->ad_mem_poe[0] = (chip->ad_mem_ctrl_l & 0x80) != 0;
        }
        if (chip->cclk2)
        {
            chip->ad_mem_data_l4[1] = chip->ad_mem_data_l4[0];

            chip->ad_mem_w7[1] = chip->ad_mem_w7[0];
            chip->ad_mem_w8[1] = chip->ad_mem_w8[0];

            chip->ad_mem_shift_cnt[1] = chip->ad_mem_shift_cnt[0];

            chip->ad_mem_mem_en_l[1] = chip->ad_mem_mem_en_l[0];

            chip->ad_mem_mem_stop[1] = chip->ad_mem_mem_stop[0];

            chip->ad_mem_w10[1] = chip->ad_mem_w10[0];

            chip->ad_mem_w15[1] = chip->ad_mem_w15[0];

            chip->ad_mem_w17[1] = chip->ad_mem_w17[0];

            chip->ad_mem_w21 = (chip->ad_mem_ctrl & 2) != 0;

            chip->ad_mem_dir = (chip->ad_mem_ctrl & 0x10) != 0;

            chip->ad_mem_pmpx[1] = chip->ad_mem_pmpx[0];
            chip->ad_mem_poe[1] = chip->ad_mem_poe[0];
        }

        {

            int nibble_load = (chip->ad_mem_shift_cnt[0] & 3) == 0 && chip->ad_mem_w7[0];

            if (chip->cclk2)
                chip->ad_mem_nibble_load = nibble_load;

            if (nibble_load && !chip->ad_mem_nibble_load)
                chip->ad_mem_nibble = (chip->ad_mem_data_l4[1] >> 4) & 15;;

            chip->ad_mem_nibble_msb = chip->ad_mem_w8[1] && (chip->ad_mem_nibble & 8) != 0;
        }

        if (chip->ad_mem_ctrl_l & 0x10)
            chip->ad_mem_bus = chip->ad_address_cnt[1][1] & 0xff;
        if (!chip->ad_mem_dir)
            chip->ad_mem_bus = chip->input.pad;
        if ((chip->ad_mem_ctrl & 1) != 0 && (chip->ad_mem_ctrl_l & 1) == 0)
            chip->ad_mem_data_l3 = chip->ad_mem_bus;

#endif

        if (chip->cclk1)
        {
#ifdef FMOPNA_YM2608
#define PLAYBACK_BIT 0x40
#else
#define PLAYBACK_BIT 0x00
#endif
            int w44 = (chip->ad_dsp_cnt1_run[1] & 3) == 0;

            int next_ptr = 0;
            chip->ad_code_ctrl = 0;
            int carry_mode = 0;
            int vol_o = 0;

            switch (chip->ad_code_ptr[1])
            {
#ifdef FMOPNA_YM2608
                case 0x3f:
#endif
                case 0x3f|PLAYBACK_BIT:
                    if (w29)
                    {
                        chip->ad_code_ctrl = 0b000100000011110011001;
                        next_ptr = 0x32;
                    }
                    else
                        next_ptr = 0x3f;
                    break;
#ifdef FMOPNA_YM2608
                case 0x32:
#endif
                case 0x32|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000100000001110111001;
                    break;
#ifdef FMOPNA_YM2608
                case 0x33:
#endif
                case 0x33|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000100000000110111101;
                    break;
#ifdef FMOPNA_YM2608
                case 0x34:
#endif
                case 0x34|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000100000000110011011;
                    break;
#ifdef FMOPNA_YM2608
                case 0x35:
#endif
                case 0x35|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000100000000110111011;
                    break;
#ifdef FMOPNA_YM2608
                case 0x36:
#endif
                case 0x36|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000100000000010011100;
                    break;
#ifdef FMOPNA_YM2608
                case 0x3b:
                    next_ptr = 0x31;
                    break;
#endif
                case 0x37|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000100000000110011111;
                    break;
                case 0x38|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000100000000110111111;
                    break;
                case 0x39|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000000000000110011001;
                    break;
                case 0x3a|PLAYBACK_BIT:
                    if (!p5)
                    {
                        next_ptr = 0x3a;
                        chip->ad_code_ctrl = 0b000000000000110011001;
                    }
                    break;
                case 0x3b|PLAYBACK_BIT:
                    if (chip->ad_code_sync[2])
                    {
                        chip->ad_code_ctrl = 0b000000100010110010000;
                        next_ptr = 0x7;
                    }
                    else
                        next_ptr = 0x3b;
                    break;
                    
#ifdef FMOPNA_YM2608
                case 0x01:
                    chip->ad_code_ctrl = 0b000011000010000000000;
                    break;
                case 0x02:
                    chip->ad_code_ctrl = 0b000010001000110010000;
                    break;
                case 0x03:
                    chip->ad_code_ctrl = 0b001110000000000000010;
                    break;
                case 0x04:
                    chip->ad_code_ctrl = 0b000110000000000100010;
                    break;
                case 0x05:
                    if (chip->ad_dsp_enc_bit_l[1])
                        chip->ad_code_ctrl = 0b100011000000010000010;
                    else
                    {
                        chip->ad_code_ctrl = 0b100010100000000000010;
                        next_ptr = 0xa;
                    }
                    break;
                case 0x06:
                    chip->ad_code_ctrl = 0b100010000000000100010;
                    break;
                case 0x07:
                    chip->ad_code_ctrl = 0b001110000000000000010;
                    break;
                case 0x08:
                    chip->ad_code_ctrl = 0b000110000000000100010;
                    break;
                case 0x09:
                    chip->ad_code_ctrl = 0b100010100000000000010;
                    break;
                case 0x0a:
                    chip->ad_code_ctrl = 0b100010000000000100010;
                    break;
                case 0x0b:
                    chip->ad_code_ctrl = 0b100011000000000000100;
                    break;
                case 0x0c:
                    chip->ad_code_ctrl = 0b100010001000000100100;
                    break;
                case 0x0e:
                    chip->ad_code_ctrl = 0b000000000000010000000;
                    break;
                case 0x0f:
                    if (chip->ad_dsp_enc_bit_l[1])
                    {
                        next_ptr = 0x13;
                        chip->ad_code_ctrl = 0b100000100000000000000;
                    }
                    break;
                case 0x10:
                    chip->ad_code_ctrl = 0b011100100000000000000;
                    break;
                case 0x11:
                    chip->ad_code_ctrl = 0b000100000000010110000;
                    break;
                case 0x12:
                    next_ptr = 0x15;
                    break;
                case 0x13:
                    chip->ad_code_ctrl = 0b100000000000010110000;
                    break;
                case 0x15:
                    chip->ad_code_ctrl = 0b011001000000000000000;
                    break;
                case 0x17:
                    chip->ad_code_ctrl = 0b100010100000100010010;
                    break;
                case 0x18:
                    chip->ad_code_ctrl = 0b100010001000100110010;
                    break;
                case 0x1a:
                    chip->ad_code_ctrl = 0b000000000000010000000;
                    break;
                case 0x1b:
                    if (chip->ad_dsp_enc_bit_l[1])
                    {
                        next_ptr = 0x20;
                        chip->ad_code_ctrl = 0b100001000000000000100;
                    }
                    else
                        chip->ad_code_ctrl = 0b000000000000100010000;
                    break;
                case 0x1c:
                    chip->ad_code_ctrl = 0b000000000000100010000;
                    break;
                case 0x1d:
                    chip->ad_code_ctrl = 0b001100000000100010000;
                    break;
                case 0x1e:
                    chip->ad_code_ctrl = 0b000100000000000100000;
                    break;
                case 0x1f:
                    chip->ad_code_ctrl = 0b100001000000000000100;
                    break;
                case 0x20:
                    chip->ad_code_ctrl = 0b100000000000000100100;
                    break;
                case 0x21:
                    if (!chip->ad_dsp_alu_mask[1] && chip->ad_dsp_alu_shift == 2)
                        chip->ad_code_ctrl = 0b000000000000000011000;
                    else
                    {
                        next_ptr = 0x13;
                        chip->ad_code_ctrl = 0b100000100000000000000;
                    }
                    break;
                case 0x22:
                    chip->ad_code_ctrl = 0b100000000000010010100;
                    break;
                case 0x23:
                    chip->ad_code_ctrl = 0b100000000000000100100;
                    break;
                case 0x24:
                    chip->ad_code_ctrl = 0b100000100000000000000;
                    break;
                case 0x25:
                    chip->ad_code_ctrl = 0b100000000000000100000;
                    break;
                case 0x26:
                    chip->ad_code_ctrl = 0b001100000000000000000;
                    break;
                case 0x27:
                    chip->ad_code_ctrl = 0b000100000000000100000;
                    break;
                case 0x28:
                    chip->ad_code_ctrl = 0b100001000000110010000;
                    break;
                case 0x29:
                    chip->ad_code_ctrl = 0b100000000100000100000;
                    break;
                case 0x2a:
                    chip->ad_code_ctrl = 0b000000100110100011000;
                    break;
                case 0x2b:
                    chip->ad_code_ctrl = 0b000000000100000000000;
                    break;
                case 0x2c:
                    chip->ad_code_ctrl = 0b001000000111000000000;
                    break;
                case 0x2d:
                    chip->ad_code_ctrl = 0b000000000001000000000;
                    break;
                case 0x2e:
                    chip->ad_code_ctrl = 0b000100000000001000100;
                    break;
                case 0x2f:
                    chip->ad_code_ctrl = 0b000100000000000100100;
                    break;
                case 0x31:
                    if (chip->ad_w12[1] & 1)
                        next_ptr = 0x3f;
                    if (chip->ad_ad_w56)
                    {
                        next_ptr |= 1;
                        chip->ad_code_ctrl = 0b000010010000100001000;
                    }
                    else
                        next_ptr |= 0x31;
                    break;
#endif

                case 0x01|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b100000000000110110010;
                    break;
                case 0x02|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000001000000000001000;
                    break;
                case 0x03|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000000000000100000000;
                    break;
                case 0x04|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b001100000000100000010;
                    break;
                case 0x05|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000100000000100100010;
                    break;
                case 0x06|PLAYBACK_BIT:
                    if (chip->ad_dsp_alu_of)
                        chip->ad_code_ctrl = 0b000000100010110010000;
                    else
                    {
                        next_ptr = 0x24;
                        chip->ad_code_ctrl = 0b100000000000010101010;
                    }
                    break;
                case 0x08|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b100001000000000000000;
                    break;
                case 0x09|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b100000000000000100000;
                    break;
                case 0x0a|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b011000001011000000000;
                    break;
                case 0x0b|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000000000001010000000;
                    break;
                case 0x0c|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000000000000000011000;
                    break;
                case 0x0d|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b100001001000000000100;
                    if (chip->ad_mem_data_l4[0] & 128)
                        chip->ad_code_ctrl |= 0b000000100000000000000;
                    break;
                case 0x0e|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b100000000000010110100;
                    break;
                case 0x0f|PLAYBACK_BIT:
                    if ((chip->ad_mem_data_l4[0] & 128) == 0)
                        chip->ad_code_ctrl = 0b000000000000000010000;
                    break;
                case 0x10|PLAYBACK_BIT:
                    if ((chip->ad_mem_data_l4[0] & 128) == 0)
                        chip->ad_code_ctrl = 0b000000000000000010000;
                    break;
                case 0x11|PLAYBACK_BIT:
                case 0x15|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b011000101000100011000;
                    break;
                case 0x12|PLAYBACK_BIT:
                case 0x16|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000000000000010010000;
                    break;
                case 0x13|PLAYBACK_BIT:
                    if ((chip->ad_mem_data_l4[0] & 128) == 0)
                        chip->ad_code_ctrl = 0b000000000000000010000;
                    break;
                case 0x14|PLAYBACK_BIT:
                    if ((chip->ad_mem_data_l4[0] & 128) == 0)
                        chip->ad_code_ctrl = 0b000000000000000010000;
                    break;
                case 0x18|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b011001000000010010000;
                    break;
                case 0x1a|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b100000100100010010110;
                    carry_mode = 1;
                    break;
                case 0x1b|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b100000000100000100110;
                    carry_mode = 1;
                    break;
                case 0x1c|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b001100000100000000000;
                    carry_mode = 1;
                    break;
                case 0x1d|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000100000100000100000;
                    carry_mode = 1;
                    break;
                case 0x1e|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000000000100010000000;
                    break;
                case 0x1f|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b001100000100000000110;
                    break;
                case 0x20|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000100000000000100110;
                    break;
                case 0x21|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000100000000001000100;
                    break;
                case 0x22|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000100000000000100100;
                    break;
                case 0x23|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b100000000000010101010;
                    break;
                case 0x24|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b100000000000000000000;
                    break;
                case 0x25|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b100000000000000100000;
                    break;
                case 0x26|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000000100010000000000;
                    break;
                case 0x29|PLAYBACK_BIT:
                    if (!w44)
                        next_ptr = 0x29;
                    else
                        chip->ad_code_ctrl = 0b000001000000111010000;
                    break;
                case 0x2b|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b000000000000110001000;
                    vol_o = 1;
                    break;
                case 0x2c|PLAYBACK_BIT:
                    chip->ad_code_ctrl = 0b001000000000000000000;
                    break;
                case 0x2f|PLAYBACK_BIT:
                    if (chip->ad_w12[1] & 1)
                        next_ptr = 0x3f;
                    if (chip->ad_code_sync[2])
                    {
                        next_ptr |= 1;
                        chip->ad_code_ctrl = 0b100000100000000000010;
                    }
                    else
                        next_ptr |= 0x2f;
                    break;
            }

#ifdef FMOPNA_YM2608
            if (!chip->ad_reg_rec)
                next_ptr |= 64; // dec/enc
#endif

            if (reset)
                next_ptr |= 63; // reset

            if (next_ptr & 63)
                chip->ad_code_ptr[0] = next_ptr;
            else
#ifdef FMOPNA_YM2608
                chip->ad_code_ptr[0] = (chip->ad_code_ptr[1] + 1) & 127;
#else
                chip->ad_code_ptr[0] = (chip->ad_code_ptr[1] + 1) & 63;
#endif

            chip->ad_code_end[0] = (chip->ad_code_ptr[1] & 63) == 63;

            chip->ad_dsp_carry_mode[0] = carry_mode;

#ifdef FMOPNA_YM2608
            chip->ad_dsp_enc_bit_l[0] = chip->ad_dsp_enc_bit;
#endif

            chip->ad_dsp_vol_o[0] = vol_o;
        }
        if (chip->cclk2)
        {
            chip->ad_code_ptr[1] = chip->ad_code_ptr[0];
            chip->ad_code_end[1] = chip->ad_code_end[0];
            chip->ad_code_ctrl_l = chip->ad_code_ctrl;

            chip->ad_dsp_ctrl = 0;
            if (chip->ad_code_ctrl & 8)
                chip->ad_dsp_ctrl |= 1;
            if (chip->ad_code_ctrl & 16)
                chip->ad_dsp_ctrl |= 2;
            if (chip->ad_code_ctrl & 0x80)
                chip->ad_dsp_ctrl |= 4;
            if (chip->ad_code_ctrl & 0x100)
                chip->ad_dsp_ctrl |= 8;

            chip->ad_dsp_carry_mode[1] = chip->ad_dsp_carry_mode[0];

#ifdef FMOPNA_YM2608
            chip->ad_dsp_enc_bit_l[1] = chip->ad_dsp_enc_bit_l[0];
#endif

            chip->ad_code_reg_id = 0;
            if (chip->ad_code_ctrl & 32)
                chip->ad_code_reg_id |= 1;
            if (chip->ad_code_ctrl & 4)
                chip->ad_code_reg_id |= 2;
            if (chip->ad_code_ctrl & 2)
                chip->ad_code_reg_id |= 4;

            chip->ad_dsp_vol_o[1] = chip->ad_dsp_vol_o[0];
        }

        int w30 = chip->ad_dsp_ctrl == 5 || chip->ad_dsp_ctrl == 13;
        int w31 = chip->ad_dsp_ctrl == 5 || chip->ad_dsp_ctrl == 13
            || chip->ad_dsp_ctrl == 3;

        int w35 = chip->ad_dsp_w36[1];
        if ((chip->ad_dsp_w31_l[1] & 2) != 0)
            w35 |= chip->ad_dsp_w33;

        int cnt1;
        if ((chip->ad_dsp_w31_l[1] & 2) != 0)
            cnt1 = 0;
        else
        {
            int inc = (chip->ad_dsp_cnt1_run[1] & 1) != 0;
            cnt1 = (chip->ad_dsp_cnt1[1] + inc) & 7;
        }

        if (chip->ad_dsp_ctrl == 1)
            chip->ad_dsp_bus = chip->ad_reg_delta_l;
        if (chip->ad_dsp_delta_sel[1])
            chip->ad_dsp_bus = chip->ad_reg_delta_h;
        if ((chip->ad_code_ctrl_l & 0x40) != 0)
            chip->ad_dsp_bus = chip->ad_dsp_w45 & 255;
        if (chip->ad_dsp_w46[1])
            chip->ad_dsp_bus = (chip->ad_dsp_w45 >> 8) & 255;
        if (chip->ad_dsp_ctrl == 3)
        {
            static const int adjust[8] = {
                57, 57, 57, 57, 77, 102, 128, 153
            };

            chip->ad_dsp_bus = adjust[chip->ad_mem_nibble & 7];
        }

        if ((chip->ad_code_ctrl_l & 0x40000) != 0)
            chip->ad_dsp_bus = chip->ad_dsp_alu_res & 255;
        if (chip->ad_dsp_read_res[1])
            chip->ad_dsp_bus = (chip->ad_dsp_alu_res >> 8) & 255;

#ifdef FMOPNA_YM2608
        if ((chip->ad_code_ctrl_l & 0x2000) != 0)
            chip->ad_dsp_bus = chip->ad_ad_buf;
#endif
        if (chip->ad_dsp_vol_o[1])
            chip->ad_dsp_bus = chip->ad_reg_level;
        if ((chip->ad_code_ctrl_l & 0x200) == 0 && chip->ad_dsp_w69[1])
            chip->ad_dsp_bus = chip->ad_dsp_sregs2[0][1];
        if ((chip->ad_code_ctrl_l & 0x200) == 0 && (chip->ad_code_ctrl_l & 0x400) != 0)
            chip->ad_dsp_bus = chip->ad_dsp_sregs2[1][1];
        if (chip->ad_dsp_ctrl == 7)
            chip->ad_dsp_bus = 127;
        if (chip->ad_dsp_ctrl == 15)
            chip->ad_dsp_bus = 0;

        if (chip->cclk1)
        {
            chip->ad_dsp_delta_sel[0] = chip->ad_dsp_ctrl == 1;

            chip->ad_dsp_w30_l[0] = chip->ad_dsp_w30_l[1] << 1;
            chip->ad_dsp_w30_l[0] |= w30;

            chip->ad_dsp_w31_l[0] = chip->ad_dsp_w31_l[1] << 1;
            chip->ad_dsp_w31_l[0] |= w31;

            chip->ad_dsp_w36[0] = w35 >> 1;
            chip->ad_dsp_w35_l = w35 & 1;

            chip->ad_dsp_cnt1[0] = cnt1;

            chip->ad_dsp_cnt1_run[0] = chip->ad_dsp_cnt1_run[1];

            int accm2 = chip->ad_dsp_mul_accm2_add1 + chip->ad_dsp_mul_accm2_add2 + chip->ad_dsp_mul_accm1_c;

            chip->ad_dsp_mul_accm1_add1 = (w35 & 1) != 0 ? chip->ad_dsp_w34 : 0;
            chip->ad_dsp_mul_accm1_add2 = cnt1 != 0 ? (chip->ad_dsp_mul_accm1[0] >> 1) | ((accm2 & 1) << 7) : 0;

            chip->ad_dsp_mul_accm1[1] = chip->ad_dsp_mul_accm1[0];

            chip->ad_dsp_w32_l = chip->ad_dsp_w32;

            chip->ad_dsp_mul_accm2_load = cnt1 != 0;

            chip->ad_dsp_mul_accm2 = accm2 & 0x1ff;

            chip->ad_dsp_w38[0] = chip->ad_dsp_w38[1] << 1;
            if (cnt1 != 0 && (chip->ad_dsp_mul_accm1[0] & 1) != 0)
                chip->ad_dsp_w38[0] |= 1;

            chip->ad_dsp_w41[0] = chip->ad_dsp_w41[1] << 1;
            if (chip->ad_dsp_ctrl == 11)
                chip->ad_dsp_w41[0] |= 1;

            chip->ad_dsp_w43[0] = (chip->ad_dsp_w43[1] << 1) | ((chip->ad_dsp_cnt1_run[1] & 3) == 2);

            chip->ad_dsp_w46[0] = (chip->ad_code_ctrl_l & 0x40) != 0;

            if (chip->ad_dsp_ctrl == 14)
                chip->ad_dsp_cnt2[0] = 0;
            else
            {
                int add = chip->ad_dsp_ctrl == 6;
                chip->ad_dsp_cnt2[0] = (chip->ad_dsp_cnt2[1] + add) & 3;
            }

#ifdef FMOPNA_YM2608
            chip->ad_dsp_ctrl10_l = chip->ad_dsp_ctrl == 10;
#endif

            chip->ad_dsp_load_alu1[0] = chip->ad_dsp_load_alu1[1] << 1;
            chip->ad_dsp_load_alu2[0] = chip->ad_dsp_load_alu2[1] << 1;

            chip->ad_dsp_load_alu1_h = (chip->ad_dsp_load_alu1[1] & 1) != 0
#ifdef FMOPNA_YM2608
                || chip->ad_dsp_ctrl == 9
#endif
                ;

            if ((chip->ad_code_ctrl_l & 0x4000) != 0)
                chip->ad_dsp_load_alu1[0] |= 1;
            if ((chip->ad_code_ctrl_l & 0x8000) != 0)
                chip->ad_dsp_load_alu2[0] |= 1;

            chip->ad_dsp_load_res[0] = (chip->ad_code_ctrl_l & 0x80000) != 0;

            chip->ad_dsp_alu_mask[0] = chip->ad_dsp_ctrl == 2;

#ifdef FMOPNA_YM2608
            chip->ad_dsp_alu_neg[0] = (chip->ad_code_ctrl_l & 0x10000) != 0;
#endif

            chip->ad_dsp_read_res[0] = (chip->ad_code_ctrl_l & 0x40000) != 0;
            chip->ad_dsp_w52[0] = (chip->ad_code_ctrl_l & 0x2000) != 0;

            chip->ad_dsp_sregs2[0][0] = (chip->ad_dsp_w69[1] && (chip->ad_code_ctrl_l & 0x200) != 0) ? chip->ad_dsp_bus : chip->ad_dsp_sregs2[0][1];
            chip->ad_dsp_sregs2[1][0] = ((chip->ad_code_ctrl_l & 0x400) != 0 && (chip->ad_code_ctrl_l & 0x200) != 0) ? chip->ad_dsp_bus : chip->ad_dsp_sregs2[1][1];

            chip->ad_dsp_w69[0] = (chip->ad_code_ctrl_l & 0x400) != 0;
        }
        if (chip->cclk2)
        {
            chip->ad_dsp_delta_sel[1] = chip->ad_dsp_delta_sel[0];
            chip->ad_dsp_w30_l[1] = chip->ad_dsp_w30_l[0];
            chip->ad_dsp_w31_l[1] = chip->ad_dsp_w31_l[0];

            chip->ad_dsp_w36[1] = chip->ad_dsp_w36[0];

            chip->ad_dsp_cnt1[1] = chip->ad_dsp_cnt1[0];

            chip->ad_dsp_cnt1_run[1] = chip->ad_dsp_cnt1_run[0] << 1;
            if (chip->ad_dsp_cnt1[0] != 7)
                chip->ad_dsp_cnt1_run[1] |= 1;

            int accm1 = chip->ad_dsp_mul_accm1_add1 + chip->ad_dsp_mul_accm1_add2;

            chip->ad_dsp_mul_accm1_c = (accm1 & 256) != 0;

            chip->ad_dsp_mul_accm1[0] = accm1 & 255;

            int w37 = (chip->ad_dsp_w38[0] & 2) == 0 && (chip->ad_dsp_mul_accm1[1] & 63) == 0;
            int w39 = (chip->ad_dsp_mul_accm1[1] & 192) == 0 && (chip->ad_dsp_mul_accm2 & 511) == 0;

            int b8 = (chip->ad_dsp_w32_l & 0x100) == 0 && !(w37 && w39);

            chip->ad_dsp_mul_accm2_add1 = chip->ad_dsp_w35_l ? (chip->ad_dsp_w32_l ^ 0x100) : 0;
            chip->ad_dsp_mul_accm2_add2 = chip->ad_dsp_mul_accm2_load ? (chip->ad_dsp_mul_accm2 >> 1) | (b8 << 8) : 0;


            chip->ad_dsp_w38[1] = chip->ad_dsp_w38[0];

            chip->ad_dsp_w41[1] = chip->ad_dsp_w41[0];

            int w42 = (chip->ad_dsp_mul_accm2 & 0x1c0) != 0x00 || (chip->ad_dsp_mul_accm2 & 0x30) == 0x30;

            if (chip->ad_dsp_w41[0] & 2)
            {
                chip->ad_dsp_w40 = 0;
                if (!w42)
                {
                    if ((chip->ad_dsp_w38[0] & 2) != 0)
                        chip->ad_dsp_w40 |= 1;
                    chip->ad_dsp_w40 |= (chip->ad_dsp_mul_accm1[1] & 255) << 1;
                    chip->ad_dsp_w40 |= (chip->ad_dsp_mul_accm2 & 15) << 9;
                }
                chip->ad_dsp_w40 |= (chip->ad_dsp_mul_accm2 & 0x30) << 9;
                if (w42)
                    chip->ad_dsp_w40 |= 0x6000;
                if (w39)
                    chip->ad_dsp_w40 |= 127;
            }
            else
            {
                chip->ad_dsp_w40 = (chip->ad_dsp_mul_accm1[1] >> 1) & 127;
                chip->ad_dsp_w40 |= (chip->ad_dsp_mul_accm2 & 511) << 7;
            }

            chip->ad_dsp_w43[1] = chip->ad_dsp_w43[0];

            chip->ad_dsp_w46[1] = chip->ad_dsp_w46[0];

            chip->ad_dsp_cnt2[1] = chip->ad_dsp_cnt2[0];

            chip->ad_dsp_load_alu1[1] = chip->ad_dsp_load_alu1[0];
            chip->ad_dsp_load_alu2[1] = chip->ad_dsp_load_alu2[0];

            chip->ad_dsp_load_res[1] = chip->ad_dsp_load_res[0];

            chip->ad_dsp_alu_mask[1] = chip->ad_dsp_alu_mask[0];

            chip->ad_dsp_alu_shift =
#ifdef FMOPNA_YM2608
                chip->ad_dsp_ctrl10_l ? 0 :
#endif
                chip->ad_dsp_cnt2[0];

#ifdef FMOPNA_YM2608
            chip->ad_dsp_alu_neg[1] = chip->ad_dsp_alu_neg[0];
#endif

            chip->ad_dsp_read_res[1] = chip->ad_dsp_read_res[0];

            chip->ad_dsp_w52[1] = chip->ad_dsp_w52[0];

            chip->ad_dsp_sregs2[0][1] = chip->ad_dsp_sregs2[0][0];
            chip->ad_dsp_sregs2[1][1] = chip->ad_dsp_sregs2[1][0];

            chip->ad_dsp_w69[1] = chip->ad_dsp_w69[0];
        }

        {
            int w47 = (chip->ad_code_ctrl_l & 0x100000) != 0;
            int w48 = (chip->ad_code_ctrl & 0x100000) != 0 && chip->cclk2;
            int w49 = (chip->ad_code_ctrl & 0x100000) == 0 && (chip->ad_code_ctrl_l & 0x20000) != 0;
            int w51 = (chip->ad_code_ctrl_l & 0x100000) != 0 && !chip->cclk2;

            int reg_id = 0;


            if (w49)
            {
                if (!w51)
                {
                    if (chip->ad_code_reg_id == 0)
                    {
                        if (((chip->ad_code_ctrl_l & 0x20000) != 0 || (chip->ad_code_ctrl_l & 0x100000) != 0))
                            chip->ad_dsp_regs[0] = chip->ad_dsp_bus;
                    }
                    else
                        chip->ad_dsp_regs[chip->ad_code_reg_id] = chip->ad_dsp_bus;
                }
            }
            if (chip->ad_dsp_w52[1])
            {
                chip->ad_dsp_regs[0] = 0;
                chip->ad_dsp_regs[1] = 0;
            }
            if (w48)
            {
                if (!w51)
                {
                    chip->ad_dsp_regs_o = chip->ad_dsp_regs[chip->ad_code_reg_id];
                }
            }
            if (w47)
            {
                chip->ad_dsp_bus = chip->ad_dsp_regs_o;
            }
        }

#ifdef FMOPNA_YM2608
        // FIXME: guesswork
        if (chip->ad_ad_w57[2])
            chip->ad_ad_cmp_i = chip->ad_comp_da < chip->ad_ad_input;
        else
            chip->ad_comp_da = chip->input.da;

        int w54 = (chip->ad_ad_shift & 1) == 0 && !chip->ad_ad_w53[1];

        int w64 = w54 ? (chip->ad_ad_w65_l ^ 127) : chip->ad_ad_w65_l;
        int w63 = (w64 + (w54 && chip->ad_ad_w55)) & 127;

        if (chip->cclk1)
        {
            if (chip->ad_ad_cnt3_of[1])
                chip->ad_ad_cnt1[0] = 0;
            else
            {
                int inc = chip->ad_ad_w57[2];
                chip->ad_ad_cnt1[0] = (chip->ad_ad_cnt1[1] + inc) & 15;
            }

            chip->ad_ad_w53[0] = chip->ad_ad_w57[2]
                ? !(w54 || ((chip->ad_ad_shift & 1) != 0 && !chip->ad_ad_cmp_i))
                : chip->ad_ad_w53[1];

            chip->ad_ad_w55_l[0] = chip->ad_ad_w55;

            int rst = chip->ad_ad_w57[2] || chip->ad_ad_cnt3_of[1];

            if (rst)
                chip->ad_ad_cnt2[0] = 0;
            else
            {
                int inc = chip->ad_ad_w55;
                chip->ad_ad_cnt2[0] = (chip->ad_ad_cnt2[1] + inc) & 31;
            }

            chip->ad_ad_w62[0] = rst;

            chip->ad_ad_w57[1] = chip->ad_ad_w57[0];
            chip->ad_ad_w58[1] = chip->ad_ad_w58[0];

            chip->ad_ad_w61[0] = chip->ad_ad_w61[1] >> 1;
            if (chip->ad_ad_w62[1])
            {
                chip->ad_ad_w61[0] |= w63;
                if (!w54)
                    chip->ad_ad_w61[0] |= 128;
            }

            int w59 = w54 ^ chip->ad_ad_w60;

            chip->ad_ad_w66[0] = chip->ad_ad_w57[2] ? (w59 ? chip->ad_ad_w65_l : chip->ad_ad_w68)
                : chip->ad_ad_w66[1];

            int inc3 = !chip->ad_ad_cnt3_of[1] && chip->ad_ad_cnt3_en[1];
            int cnt3 = chip->ad_ad_cnt3[1] + inc3;
            int en = chip->ad_sample_l[2] || chip->ad_start_l[2];
            chip->ad_ad_cnt3[0] = cnt3 & 0x7ff;
            chip->ad_ad_cnt3_of[0] = (cnt3 & 0x800) != 0;
            chip->ad_ad_cnt3_en[0] = en;
            chip->ad_ad_cnt3_load = chip->ad_ad_cnt3_of[1] || (en && !chip->ad_ad_cnt3_en[1]);
            chip->ad_ad_cnt3_load_val = (chip->ad_reg_prescale_l | (chip->ad_reg_prescale_h << 8)) ^ 0x7ff;
        }
        if (chip->cclk2)
        {
            int cnt1 = chip->ad_ad_cnt1[0];
            if (chip->ic)
                cnt1 |= 8;

            int shift = 0;
            int shift2 = 0;
            if ((cnt1 & 8) == 0)
            {
                shift = 1 << (cnt1 & 7);
                shift2 = 128 >> (cnt1 & 7);
            }
            chip->ad_ad_shift = shift;

            chip->ad_ad_cnt1[1] = cnt1;

            chip->ad_ad_w53[1] = chip->ad_ad_w53[0];

            chip->ad_ad_w55 = (cnt1 & 8) == 0;

            chip->ad_ad_w55_l[1] = chip->ad_ad_w55_l[0];

            chip->ad_ad_cnt2[1] = chip->ad_ad_cnt2[0];

            chip->ad_ad_w57[0] = chip->ad_ad_cnt2[0] == 23;
            chip->ad_ad_w57[2] = chip->ad_ad_w57[1];

            chip->ad_ad_w58[0] = (chip->ad_ad_cnt2[0] & 24) == 0;
            chip->ad_ad_w58[2] = chip->ad_ad_w58[1];

            chip->ad_ad_w61[1] = chip->ad_ad_w61[0];

            chip->ad_ad_w62[1] = chip->ad_ad_w62[0];

            int w67 = chip->ad_ad_w66[0];
            if (shift & 1)
                w67 = 0;

            int sum = shift2 + w67;
            chip->ad_ad_w65_l = sum & 127;

            chip->ad_ad_w66[1] = chip->ad_ad_w66[0];

            chip->ad_ad_w68 = w67;

            chip->ad_ad_cnt3[1] = chip->ad_ad_cnt3[0];
            if (chip->ad_ad_cnt3_load)
                chip->ad_ad_cnt3[1] |= chip->ad_ad_cnt3_load_val;

            chip->ad_ad_cnt3_of[1] = chip->ad_ad_cnt3_of[0];

            chip->ad_ad_cnt3_en[1] = chip->ad_ad_cnt3_en[0];
        }

        if (chip->ad_ad_w57[2])
            chip->ad_ad_w60 = chip->ad_ad_cmp_i;

        if (!chip->ad_ad_w55)
            chip->ad_ad_input = chip->input.ad;

        chip->ad_ad_w56 = !chip->ad_ad_w55 && chip->ad_ad_w55_l[1];

        if (chip->ad_ad_w56)
        {
            chip->ad_ad_buf = w63;
            if (w54)
                chip->ad_ad_buf |= 128;
        }

        if (chip->ad_isf && chip->read3)
        {
            chip->data_bus1 &= ~255;
            chip->data_bus1 |= chip->ad_ad_buf & 255;
        }
#endif


        chip->ad_set_eos = chip->ad_w13[1]
#ifdef FMOPNA_YM2608
            || (chip->ad_ad_w56 && chip->ad_sample_l[2])
#endif
            ;


        if (w31 && (chip->ad_dsp_w31_l[0] & 1) == 0)
            chip->ad_dsp_w33 = chip->ad_dsp_bus;
        if ((chip->ad_dsp_w31_l[1] & 1) != 0 && (chip->ad_dsp_w31_l[0] & 2) == 0)
            chip->ad_dsp_w34 = chip->ad_dsp_bus;
        if ((chip->ad_dsp_w31_l[1] & 2) != 0 && (chip->ad_dsp_w31_l[0] & 4) == 0)
        {
            chip->ad_dsp_w32 = chip->ad_dsp_bus;
            if ((chip->ad_dsp_w32 & 128) == 0 || (chip->ad_dsp_w30_l[1] & 2) == 0)
                chip->ad_dsp_w32 |= 256;
        }

        if ((chip->ad_dsp_w43[1] & 1) != 0 && (chip->ad_dsp_w43[0] & 2) == 0)
            chip->ad_dsp_w45 = chip->ad_dsp_w40;

        if ((chip->ad_dsp_w45 >> 3) == 0x1fff || ((chip->ad_dsp_w45 >> 3) & 0x1ffe) == 0)
            chip->ad_output = 0;
        else
            chip->ad_output = chip->ad_dsp_w45 >> 3;

        if ((chip->ad_code_ctrl_l & 0x4000) != 0 && (chip->ad_dsp_load_alu1[0] & 1) == 0)
        {
            chip->ad_dsp_alu_in1 &= ~255;
            chip->ad_dsp_alu_in1 |= chip->ad_dsp_bus;
        }
        if ((chip->ad_code_ctrl_l & 0x8000) != 0 && (chip->ad_dsp_load_alu2[0] & 1) == 0)
        {
            chip->ad_dsp_alu_in2 &= ~255;
            chip->ad_dsp_alu_in2 |= chip->ad_dsp_bus;
        }

        if (((chip->ad_dsp_load_alu1[1] & 1) != 0
#ifdef FMOPNA_YM2608
            || chip->ad_dsp_ctrl == 9
#endif
            ) && !chip->ad_dsp_load_alu1_h)
        {
            chip->ad_dsp_alu_in1 &= ~0xff00;
            chip->ad_dsp_alu_in1 |= chip->ad_dsp_bus << 8;
        }
        if ((chip->ad_dsp_load_alu2[1] & 1) != 0 && (chip->ad_dsp_load_alu2[0] & 2) == 0)
        {
            chip->ad_dsp_alu_in2 &= ~0xff00;
            chip->ad_dsp_alu_in2 |= chip->ad_dsp_bus << 8;
        }

        if (chip->ad_dsp_ctrl == 4)
            chip->ad_dsp_alu_in1 = 0;
#ifdef FMOPNA_YM2608
        if (chip->ad_dsp_ctrl == 9)
            chip->ad_dsp_alu_in1 &= ~255;
#endif

        // alu
        {
            int i1 = chip->ad_dsp_alu_in1;
            int i2;
            if (chip->ad_dsp_alu_mask[1])
                i2 = 0;
            else
                i2 = chip->ad_dsp_alu_in2 >> chip->ad_dsp_alu_shift;

            int c1 = chip->ad_dsp_carry_mode[1] && (chip->ad_dsp_alu_in2 & 1) != 0;
            int c2 = chip->ad_mem_nibble_msb;

            int neg =
#ifdef FMOPNA_YM2608
                chip->ad_dsp_alu_neg[1] ||
#endif
                c2;

            int b15_i1 = (i1 & 0x8000) != 0;
            int b15_i2 = (i2 & 0x8000) != 0;

            if (neg)
                i2 = i2 ^ 0xffff;

            int carry = !(!neg && (c2 || !c1)) && (!c2 || !c1);
            int sum = i1 + i2 + carry;
            int of = (sum >> 16) & 1;
            sum &= 0xffff;
            sum ^= 0x8000;


            int b15_s = (sum & 0x8000) != 0;

            if ((chip->ad_code_ctrl_l & 0x80000) == 0 && chip->ad_dsp_load_res[1] == 0)
            {
                int clip_h = (!b15_s && chip->ad_dsp_ctrl != 8 && !neg && !b15_i2 && !b15_i1)
                    || (!b15_s && chip->ad_dsp_ctrl != 8 && neg && b15_i2 && !b15_i1);
                int clip_l = (b15_s && chip->ad_dsp_ctrl != 8 && !neg && b15_i2 && b15_i1)
                    || (b15_s && chip->ad_dsp_ctrl != 8 && neg && !b15_i2 && b15_i1);

                chip->ad_dsp_alu_res = clip_h ? 0xffff : (clip_l ? 0 : sum);
                chip->ad_dsp_alu_res ^= 0x8000;
            }

            chip->ad_dsp_alu_of = of;

#ifdef FMOPNA_YM2608
            chip->ad_dsp_enc_bit = (b15_i1 && !b15_i2) || (!b15_i1 && !of && !b15_i2)
                || (b15_i1 && !of && b15_i2);
#endif
        }

        if (chip->clk2)
            chip->ad_code_sync[0] = chip->fsm_sel23[1];
        if (chip->cclk1)
            chip->ad_code_sync[1] = chip->ad_code_sync[0];
        if (chip->cclk2)
            chip->ad_code_sync[2] = chip->ad_code_sync[1];

#ifdef FMOPNA_YM2608
        chip->ad_ad_quiet = (chip->ad_ad_buf & 0xf8) == 0 || (chip->ad_ad_buf & 0xf8) == 0xf8;

        chip->o_spoff = chip->ad_reg_spoff;
        chip->o_a8 = (chip->ad_mem_bus & 0x100) != 0;

        chip->o_romcs = !(chip->ad_reg_rom && (chip->ad_mem_ctrl_l & 1) != 0);
        chip->o_mden = !chip->ad_reg_rom && (chip->ad_mem_ctrl_l & 1) != 0;

        chip->o_we = !chip->ad_mem_we[1];
        chip->o_cas = !chip->ad_mem_cas[1];
        chip->o_ras = !chip->ad_mem_ras[1];

        chip->o_dm = chip->ad_mem_bus & 0xff;
        chip->o_dm_d = !chip->ad_mem_dir;
#else
        chip->o_pad = chip->ad_mem_bus & 0xff;
        chip->o_pad_d = !chip->ad_mem_dir;

        chip->o_pa8 = (chip->ad_address_cnt[1][1] >> 8) & 15;

        chip->o_pmpx = chip->ad_mem_pmpx[1];
        chip->o_poe = !chip->ad_mem_poe[1];
#endif
    }


    // accumulator
    {
        if (chip->clk1)
        {
#ifdef FMOPNA_YM2608
            chip->ac_da_w70[0] = (!chip->ac_da_sync && chip->ac_da_w70[1]) || (chip->ad_ise && chip->write1_en);
#endif

            chip->ac_da_sync3[0] = chip->ac_da_sync;

            int accm1 = chip->ac_rss_sum_l;
            if (chip->ac_rss_sum_l & 0x8000)
                accm1 |= 0x30000;
            if (
#ifdef FMOPNA_YM2608
                !chip->ad_reg_rec &&
#endif
                chip->ad_reg_l && chip->ad_start_l[2])
                accm1 += chip->ac_ad_output;
            chip->ac_fm_accm1[0] = chip->ac_da_sync2 ? accm1 : chip->ac_fm_accm1[1];
            if ((chip->ac_fm_pan & 2) != 0 && chip->ac_fm_output_en)
                chip->ac_fm_accm1[0] += chip->ac_fm_output;
            chip->ac_fm_accm1[0] &= 0x3ffff;

            int accm2 = chip->ac_rss_sum_r;
            if (chip->ac_rss_sum_r & 0x8000)
                accm2 |= 0x30000;
            if (
#ifdef FMOPNA_YM2608
                !chip->ad_reg_rec &&
#endif
                chip->ad_reg_r && chip->ad_start_l[2])
                accm2 += chip->ac_ad_output;
            chip->ac_fm_accm2[0] = chip->ac_da_sync ? accm2 : chip->ac_fm_accm2[1];
            if ((chip->ac_fm_pan & 1) != 0 && chip->ac_fm_output_en)
                chip->ac_fm_accm2[0] += chip->ac_fm_output;
            chip->ac_fm_accm2[0] &= 0x3ffff;

            chip->ac_shifter_load_l = chip->ac_da_sync2;
            chip->ac_shifter_load_r = chip->ac_da_sync;
        }
        if (chip->clk2)
        {
#ifdef FMOPNA_YM2608
            chip->ac_da_w70[1] = chip->ac_da_w70[0];
#endif
            chip->ac_da_sync = chip->fsm_sel23[1];

            chip->ac_fm_output = (chip->op_output[2] & 0x1fff) >> 1;
            if (chip->op_output[2] & 0x2000)
                chip->ac_fm_output |= 0x3f000;

            chip->ac_fm_output_en = chip->alg_output_l;
            chip->ac_fm_pan = (chip->reg_b4[0][5] >> 6) & 3;

            chip->ac_da_sync2 = chip->fsm_sel11[1];

            chip->ac_da_sync3[1] = chip->ac_da_sync3[0];

            chip->ac_fm_accm1[1] = chip->ac_fm_accm1[0];
            chip->ac_fm_accm2[1] = chip->ac_fm_accm2[0];
        }

        if (chip->rss_eclk1)
        {
            int rss_sample = (chip->rss_sample_shift & 0xfff) << 2;
            if (chip->rss_sample_shift & 0x1000)
                rss_sample |= 0xc000;
            int accm1 = chip->rss_cnt2[1] == 0 ? 0 : chip->ac_rss_accm1[1];
            if (chip->rss_pan[2] & 2)
                accm1 += rss_sample;
            chip->ac_rss_accm1[0] = accm1 & 0xffff;

            int accm2 = chip->rss_cnt2[1] == 0 ? 0 : chip->ac_rss_accm2[1];
            if (chip->rss_pan[2] & 1)
                accm2 += rss_sample;
            chip->ac_rss_accm2[0] = accm2 & 0xffff;

            chip->ac_rss_load = chip->rss_cnt2[1] == 0;

            // tildearrow: per-channel osc
            chip->last_rss_sample = rss_sample & 0xffff;
        }
        if (chip->rss_eclk2)
        {
            chip->ac_rss_accm1[1] = chip->ac_rss_accm1[0];
            chip->ac_rss_accm2[1] = chip->ac_rss_accm2[0];
        }

        if (chip->rss_cnt2[1] == 0 && !chip->ac_rss_load)
        {
            chip->ac_rss_sum_l = chip->ac_rss_accm1[1];
            chip->ac_rss_sum_r = chip->ac_rss_accm2[1];
        }

        if (chip->ac_da_sync3[1])
        {
            chip->ac_ad_output = (chip->ad_output & 0xfff) << 2;
            if (chip->ad_output & 0x1000)
                chip->ac_ad_output |= 0x3c000;
        }

        int load_l = chip->ac_da_sync2 && !chip->ac_shifter_load_l;
        int load_r = chip->ac_da_sync && !chip->ac_shifter_load_r;

        if (load_l)
            chip->ac_shifter_top = (chip->ac_fm_accm1[1] >> 15) & 7;
        else if (load_r)
            chip->ac_shifter_top = (chip->ac_fm_accm2[1] >> 15) & 7;

        if (chip->bclk1)
        {
#ifdef FMOPNA_YM2608
            chip->ac_da_shift[1] = chip->ac_da_shift[0];

            chip->ac_da_set[1] = chip->ac_da_set[0];

            chip->opo_da[1] = chip->opo_da[0];
#endif

            chip->ac_shifter[0] = chip->ac_shifter[1] >> 1;

            chip->ac_shifter_bit = chip->ac_shifter[1] & 1;

            chip->opo_fm = chip->ac_opo;
        }
        if (chip->bclk2)
        {
#ifdef FMOPNA_YM2608
            int set = chip->ac_da_w70[1] && chip->ac_da_sync;

            chip->opo_da[0] = chip->ac_da_shift[1] & 1;

            int bit = !chip->ic && (chip->ac_da_shift[1] & 1) != 0;
            chip->ac_da_shift[0] = (chip->ac_da_shift[1] >> 1) | (bit << 23);
            if (set && !chip->ac_da_set[1])
            {
                chip->ac_da_shift[0] &= ~(0xffff << 8);
                chip->ac_da_shift[0] |= chip->ad_da_data << 8;
            }

            chip->ac_da_set[0] = set;
#endif

            chip->ac_shifter[1] = chip->ac_shifter[0];

            if (load_l || load_r)
            {
                int sample = 0;
                if (load_l)
                    sample = chip->ac_fm_accm1[1];
                else if (load_r)
                    sample = chip->ac_fm_accm2[1];

                int sample16 = sample & 0x7fff;
                if ((sample & 0x20000) == 0)
                    sample16 |= 0x8000;

                chip->ac_shifter[1] |= sample16;
            }

            int clipl = chip->ac_shifter_top == 6 || (chip->ac_shifter_top & 6) == 4;
            int cliph = chip->ac_shifter_top == 1 || (chip->ac_shifter_top & 6) == 2;

            chip->ac_opo = cliph ? 1 : (clipl ? 0 : chip->ac_shifter_bit);
        }

        if (chip->clk2)
        {
            chip->sh1_l = chip->fsm_sh1[1];
            chip->sh2_l = chip->fsm_sh2[1];
        }

#ifdef FMOPNA_YM2608
        if (chip->cclk2)
        {
            chip->opo_ad = chip->ad_ad_w61[0] & 1;
        }
#endif


#ifdef FMOPNA_YM2608
        int dac_damode = chip->ad_reg_da_ad && chip->ad_sample_l[2];

        int dac_admode = (chip->ad_reg_rec && chip->ad_start_l[2]) || (!chip->ad_reg_da_ad && chip->ad_sample_l[2]);

        if (dac_damode)
            chip->o_sh1 = chip->ad_reg_l && chip->sh1_l;
        else if (dac_admode)
            chip->o_sh1 = 0;
        else
            chip->o_sh1 = chip->sh1_l;

        if (dac_damode)
            chip->o_sh2 = chip->ad_reg_r && chip->sh2_l;
        else if (dac_admode)
            chip->o_sh2 = chip->ad_ad_w58[2];
        else
            chip->o_sh2 = chip->sh2_l;

        if (dac_damode)
            chip->o_opo = chip->opo_da[1];
        else if (dac_admode)
            chip->o_opo = chip->opo_ad;
        else
            chip->o_opo = chip->opo_fm;

        if (dac_admode)
            chip->o_s = chip->cclk2;
        else
            chip->o_s = chip->bclk1;
#else
        chip->o_sh1 = chip->sh1_l;
        chip->o_sh2 = chip->sh2_l;
        chip->o_opo = chip->opo_fm;
        chip->o_s = chip->bclk1;
#endif

    }
#endif

    {
        if (chip->clk1)
        {
            int inc = chip->busy_cnt_en[1];
            int sum = chip->busy_cnt[1] + inc;
            int of = (sum & 32) != 0;
            if (chip->ic)
                chip->busy_cnt[0] = 0;
            else
                chip->busy_cnt[0] = sum & 31;

            chip->busy_cnt_en[0] = chip->write1_en || (chip->busy_cnt_en[1] && !(of || chip->ic));

#ifndef FMOPNA_YM2612
            chip->eos_l[0] = chip->eos_flag;

            chip->eos_repeat = chip->eos_l[1] && chip->ad_set_eos && chip->ad_reg_repeat;
#endif
        }
        if (chip->clk2)
        {
            chip->busy_cnt[1] = chip->busy_cnt[0];
            chip->busy_cnt_en[1] = chip->busy_cnt_en[0];

#ifdef FMOPNA_YM2608
            chip->zero_set = chip->lfo_cnt_of;
#endif

#ifndef FMOPNA_YM2612
            chip->eos_l[1] = chip->eos_l[0];
#endif
            chip->eg_dbg = chip->reg_test_21[0] ? (chip->eg_debug[0] & 0x200) != 0 :
                chip->eg_debug_inc;
        }

#ifdef FMOPNA_YM2608
        if (chip->irq_mask_eos)
            chip->eos_flag = 0;
        else
            chip->eos_flag |= chip->ad_set_eos;

        if (chip->irq_mask_brdy)
            chip->brdy_flag = 0;
        else
            chip->brdy_flag |= chip->ad_brdy_set_l[1];

        if (chip->irq_mask_zero)
            chip->zero_flag = 0;
        else
            chip->zero_flag |= chip->zero_set;

        if (!chip->read0 && !chip->read2)
        {
            chip->status_timer_a = chip->timer_a_status[1];
            chip->status_timer_b = chip->timer_b_status[1];
            chip->status_eos = chip->eos_flag;
            chip->status_brdy = chip->brdy_flag;
            chip->status_zero = chip->zero_flag;
        }
#endif
#ifdef FMOPNA_YM2610
        if (chip->ic || (chip->reg_flags[1] & 1) != 0)
            chip->flag_rss_0 = 0;
        else if (chip->rss_stop_flag[1] & 1)
            chip->flag_rss_0 = 1;
        if (chip->ic || (chip->reg_flags[1] & 2) != 0)
            chip->flag_rss_1 = 0;
        else if (chip->rss_stop_flag[1] & 2)
            chip->flag_rss_1 = 1;
        if (chip->ic || (chip->reg_flags[1] & 4) != 0)
            chip->flag_rss_2 = 0;
        else if (chip->rss_stop_flag[1] & 4)
            chip->flag_rss_2 = 1;
        if (chip->ic || (chip->reg_flags[1] & 8) != 0)
            chip->flag_rss_3 = 0;
        else if (chip->rss_stop_flag[1] & 8)
            chip->flag_rss_3 = 1;
        if (chip->ic || (chip->reg_flags[1] & 16) != 0)
            chip->flag_rss_4 = 0;
        else if (chip->rss_stop_flag[1] & 16)
            chip->flag_rss_4 = 1;
        if (chip->ic || (chip->reg_flags[1] & 32) != 0)
            chip->flag_rss_5 = 0;
        else if (chip->rss_stop_flag[1] & 32)
            chip->flag_rss_5 = 1;
        if (chip->ic || (chip->reg_flags[1] & 128) != 0 || chip->irq_eos_l)
            chip->eos_flag = 0;
        else if (chip->ad_set_eos)
            chip->eos_flag = 1;
        if (!chip->read0 && !chip->read2)
        {
            chip->status_timer_a = chip->timer_a_status[1];
            chip->status_timer_b = chip->timer_b_status[1];

            chip->status_rss_0 = chip->flag_rss_0;
            chip->status_rss_1 = chip->flag_rss_1;
            chip->status_rss_2 = chip->flag_rss_2;
            chip->status_rss_3 = chip->flag_rss_3;
            chip->status_rss_4 = chip->flag_rss_4;
            chip->status_rss_5 = chip->flag_rss_5;
            chip->status_eos = chip->eos_flag;
        }
#endif
#ifdef FMOPNA_YM2612
        if (!chip->read0)
        {
            chip->status_timer_a = chip->timer_a_status[1];
            chip->status_timer_b = chip->timer_b_status[1];
        }
#endif

#ifdef FMOPNA_YM2608
        chip->o_irq_pull = 0;
        if (chip->reg_irq[1] & 1)
            chip->o_irq_pull |= chip->status_timer_a;
        if (chip->reg_irq[1] & 2)
            chip->o_irq_pull |= chip->status_timer_b;
        if (chip->reg_irq[1] & 4)
            chip->o_irq_pull |= chip->status_eos;
        if (chip->reg_irq[1] & 8)
            chip->o_irq_pull |= chip->status_brdy;
        if (chip->reg_irq[1] & 16)
            chip->o_irq_pull |= chip->status_zero;
#else
        chip->o_irq_pull = chip->status_timer_a | chip->status_timer_b;
#endif

        chip->read_bus = 0; // FIXME
#ifndef FMOPNA_YM2612
        if (chip->ssg_read1
#ifdef FMOPNA_YM2608
            || chip->read3
#endif
            )
            chip->read_bus = chip->data_bus1 & 255;
#endif
        if ((chip->reg_test_21[1] & 0x40) == 0
#ifndef FMOPNA_YM2612
            && (chip->reg_test_12[1] & 0x20) == 0
#endif
            )
        {
            if (chip->read0)
            {
                if (chip->busy_cnt_en[1])
                    chip->read_bus |= 128;
                if (chip->status_timer_a)
                    chip->read_bus |= 1;
                if (chip->status_timer_b)
                    chip->read_bus |= 2;
            }
#ifdef FMOPNA_YM2608
            if (chip->read2)
            {
                if (chip->busy_cnt_en[1])
                    chip->read_bus |= 128;
                if (chip->status_timer_a)
                    chip->read_bus |= 1;
                if (chip->status_timer_b)
                    chip->read_bus |= 2;
                if (chip->status_eos)
                    chip->read_bus |= 4;
                if (chip->status_brdy)
                    chip->read_bus |= 8;
                if (chip->status_zero)
                    chip->read_bus |= 16;
                if (chip->ad_start_l[0])
                    chip->read_bus |= 32;
            }
#endif
#ifdef FMOPNA_YM2610
            if (chip->read2)
            {
                if (chip->status_rss_0)
                    chip->read_bus |= 1;
                if (chip->status_rss_1)
                    chip->read_bus |= 2;
                if (chip->status_rss_2)
                    chip->read_bus |= 4;
                if (chip->status_rss_3)
                    chip->read_bus |= 8;
                if (chip->status_rss_4)
                    chip->read_bus |= 16;
                if (chip->status_rss_5)
                    chip->read_bus |= 32;
                if (chip->status_eos)
                    chip->read_bus |= 128;
            }
#endif
        }
#ifndef FMOPNA_YM2612
        if ((chip->reg_test_12[1] & 0x20) != 0 && chip->read0)
        {
#ifdef  FMOPNA_YM2608
            if ((chip->reg_test_12[1] & 0x40) == 0)
            {
                // FIXME
                chip->read_bus = rss_rom[(chip->rss_ix >> 2) & 0x1fff];
            }
            else
#endif
            if ((chip->reg_test_12[1] & 0x80) == 0)
            {
                chip->read_bus = (chip->rss_dbg_data >> 8) & 255;
            }
            else
            {
                chip->read_bus = chip->rss_dbg_data & 255;
            }
        }
#endif
        if ((chip->reg_test_21[1] & 0x40) != 0 && chip->read0)
        {
            int testdata = 0;
#ifndef FMOPNA_YM2612
            testdata = chip->op_output[3] & 0x3fff;
#endif
#ifdef FMOPNA_YM2612
            if (chip->reg_test_2c[1] & 16)
                testdata |= chip->ch_dbg[1] & 0x1ff;
            else
                testdata |= chip->op_output[3] & 0x3fff;
#endif

            testdata |= (chip->pg_dbg[1] & 1) << 15;
            testdata |= chip->eg_dbg << 14;

            if ((chip->reg_test_21[1] & 0x80) == 0)
            {
                chip->read_bus = (testdata >> 8) & 255;
            }
            else
            {
                chip->read_bus = testdata & 255;
            }
        }

        chip->o_data = chip->read_bus;
    }
#ifdef FMOPNA_YM2612
    // YM2612 channel accumulator
    {
        int test_dac = (chip->reg_test_2c[1] & 32) != 0;
        if (chip->clk1)
        {
            int add = (!test_dac && chip->ch_op_add) ? chip->ch_op_output : 0;
            int load = chip->ch_accm_load || test_dac;
            int clear = !test_dac && load;
            int acc = clear ? 0 : chip->ch_accm[1][5];
            int sum1 = (add + acc + test_dac) & 0x1ff;

            if ((acc & 0x100) == 0 && (add & 0x100) == 0 && (sum1 & 0x100) != 0)
                sum1 = 255;
            else if ((acc & 0x100) != 0 && (add & 0x100) != 0 && (sum1 & 0x100) == 0)
                sum1 = 256;

            chip->ch_accm[0][0] = sum1;
            memcpy(&chip->ch_accm[0][1], &chip->ch_accm[1][0], 5 * sizeof(short));
            memcpy(&chip->ch_buf[0][1], &chip->ch_buf[1][0], 5 * sizeof(short));

            chip->ch_buf[0][0] = load ? chip->ch_accm[1][5] : chip->ch_buf[1][5];

            chip->ch_load[1] = chip->ch_load[0];

            chip->ch_dbg[0] = chip->ch_output;
        }
        if (chip->clk2)
        {
            chip->ch_op_output = (chip->op_output[2] >> 5) & 0x1ff;
            chip->ch_op_add = chip->alg_output_l;
            chip->ch_accm_load = chip->alg_mod_op1_1_l;
            memcpy(&chip->ch_accm[1][0], &chip->ch_accm[0][0], 6 * sizeof(short));
            memcpy(&chip->ch_buf[1][0], &chip->ch_buf[0][0], 6 * sizeof(short));

            chip->ch_load[0] = chip->fsm_dac_load[1];

            chip->ch_sel = chip->fsm_dac_out_sel[1];

            chip->ch_ch6 = chip->fsm_dac_ch6[1];

            chip->ch_dbg[1] = chip->ch_dbg[0];

            chip->o_test = chip->fsm_sel23[1];
        }

        {
            if (test_dac || (chip->ch_load[0] && !chip->ch_load[1]))
            {
                int sel = test_dac || chip->ch_sel;
                chip->ch_output = sel ? chip->ch_buf[1][5] : chip->ch_buf[1][4];
            }

            if (chip->ch_load[0] && !chip->ch_load[1])
            {
                int sel = chip->ch_sel;
                if (sel)
                    chip->ch_pan = (chip->reg_b4[1][5] >> 6) & 3;
                else
                    chip->ch_pan = (chip->reg_b4[1][4] >> 6) & 3;
            }

            int dac_sel = test_dac || (chip->reg_dac_en[1] && chip->ch_ch6);

            int ch_val;

            if (dac_sel)
            {
                ch_val = (chip->reg_dac_data[1] ^ 0x80) << 1;
                ch_val |= (chip->reg_test_2c[1] & 8) != 0;
            }
            else
                ch_val = chip->ch_output;

            int do_out = test_dac || chip->ch_load[0];
            int sign;

            if (ch_val & 256)
            {
                sign = -1;
                ch_val |= ~0x1ff;
            }
            else
            {
                sign = 1;
                ch_val++;
            }

            if (do_out && (chip->ch_pan & 2) != 0)
                chip->o_mol = ch_val;
            else
                chip->o_mol = sign;
            if (do_out && (chip->ch_pan & 1) != 0)
                chip->o_mor = ch_val;
            else
                chip->o_mor = sign;
        }

        chip->o_test_d = (chip->reg_test_2c[1] & 128) == 0;
    }
#endif

#undef ADDRESS_MATCH
}
