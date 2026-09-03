/*
 * Copyright (C) 2025 nukeykt
 *
 * This file is part of YM2203-LLE.
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
 *  YM2203 emulator.
 *  Thanks:
 *      John McMaster (siliconpr0n.org):
 *          YM2203 decap
 *
 */
#include <string.h>
#include "fmopn.h"

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


void FMOPN_Clock(fmopn_t* chip, int clk)
{
    int i;

    chip->input.clk = clk;

    int ic = !chip->input.ic;

    chip->mclk1 = !chip->input.clk;
    chip->mclk2 = chip->input.clk;

    if (chip->mclk1)
    {
        chip->ic_latch[0] = (chip->ic_latch[1] & 63) << 1;
        chip->ic_latch[0] |= ic;
    }
    else
        chip->ic_latch[1] = chip->ic_latch[0];

    int ic2 = (chip->ic_latch[0] & 64) == 0 && (chip->ic_latch[0] & 1) != 0;

    if (chip->mclk1)
    {
        chip->ps1[0] = (chip->ps1[1] & 3) << 1;
        if (!ic2 && (chip->ps1[1] & 3) == 0)
            chip->ps1[0] |= 1;
        chip->ps1_l0 = chip->ps1[1] & 1;
        chip->ps1_l1 = (chip->ps1[1] >> 1) & 1;
    }
    else
    {
        chip->ps1[1] = chip->ps1[0];
    }

    if ((chip->prescaler_sel[1] & 2) == 0) // div 2
    {
        chip->aclk1 = !chip->mclk1;
        chip->aclk2 = !chip->mclk2;
    }
    else if ((chip->prescaler_sel[1] & 1) == 0) // div 6
    {
        chip->aclk1 = (chip->ps1[1] >> 2) & 1;
        chip->aclk2 = chip->ps1[1] & 1;
    }
    else // div 3
    {
        chip->aclk1 = (chip->mclk1 && (chip->ps1[1] & 4) != 0) || (!chip->ps1_l1 && (chip->ps1[1] & 2) != 0);
        chip->aclk2 = (chip->mclk1 && (chip->ps1[1] & 1) != 0) || (!chip->ps1_l0 && (chip->ps1[1] & 1) != 0);
    }

    chip->o_sy = chip->aclk2;

    if (ic2)
    {
        chip->ps2[0] = 0;
        chip->ps2[1] = 0;
    }
    else
    {
        if (chip->aclk1)
        {
            chip->ps2[0] = !chip->ps2[1];
        }
        if (chip->aclk2)
        {
            chip->ps2[1] = chip->ps2[0];
        }
    }

    chip->clk1 = chip->ps2[0];
    chip->clk2 = !chip->ps2[0];


    int write_0_en;
    int write_1_en;
    int write_p_en;
    int read0;
    int read1;
    int write_addr;
    int write_data;
    int read;
    {
        int write = !chip->input.wr && !chip->input.cs;
        read0 = !ic && !chip->input.rd && !chip->input.cs && !chip->input.a0;
        read1 = !ic && !chip->input.rd && !chip->input.cs && chip->input.a0;
        write_addr = ic || (!chip->input.wr && !chip->input.cs && !chip->input.a0);
        write_data = !ic && !chip->input.wr && !chip->input.cs && chip->input.a0;
        read = !ic && !chip->input.rd && !chip->input.cs;

        if (write)
            chip->data_l = chip->input.data;

        if (!read1 && !ic)
        {
            chip->data_bus1 = chip->data_l;
            chip->data_bus2 = chip->data_l ^ 255;
        }
        else
        {
            chip->data_l &= ~32; // FIXME
        }

        if (ic)
        {
            chip->data_bus1 = 0;
            chip->data_bus2 = 255;
        }

        if (chip->write0_l[5])
            chip->write0_trig = 0;
        else if (write_addr)
            chip->write0_trig = 1;

        if (chip->mclk2)
            chip->write0_l[0] = chip->write0_trig;
        else if (chip->mclk1)
            chip->write0_l[0] = chip->write0_l[0];

        if (chip->mclk1)
            chip->write0_l[1] = chip->write0_l[0];
        if (chip->mclk2)
            chip->write0_l[2] = chip->write0_l[1];

        if (chip->clk2)
            chip->write0_l[3] = chip->write0_l[2];
        else if (chip->clk1)
            chip->write0_l[3] = chip->write0_l[3];

        if (chip->clk1)
            chip->write0_l[4] = chip->write0_l[3];
        if (chip->clk2)
            chip->write0_l[5] = chip->write0_l[4];

        if (chip->write1_l[5])
            chip->write1_trig = 0;
        else if (write_data)
            chip->write1_trig = 1;

        if (chip->mclk2)
            chip->write1_l[0] = chip->write1_trig;
        else if (chip->mclk1)
            chip->write1_l[0] = chip->write1_l[0];

        if (chip->mclk1)
            chip->write1_l[1] = chip->write1_l[0];
        if (chip->mclk2)
            chip->write1_l[2] = chip->write1_l[1];

        if (chip->clk2)
            chip->write1_l[3] = chip->write1_l[2];
        else if (chip->clk1)
            chip->write1_l[3] = chip->write1_l[3];

        if (chip->clk1)
            chip->write1_l[4] = chip->write1_l[3];
        if (chip->clk2)
            chip->write1_l[5] = chip->write1_l[4];

        if (chip->writep_l[2])
            chip->writep_trig = 0;
        else if (write_addr)
            chip->writep_trig = 1;

        if (chip->mclk2)
            chip->writep_l[0] = chip->writep_trig;
        else if (chip->mclk1)
            chip->writep_l[0] = chip->writep_l[0];

        if (chip->mclk1)
            chip->writep_l[1] = chip->writep_l[0];
        if (chip->mclk2)
            chip->writep_l[2] = chip->writep_l[1];

#define ADDRESS_MATCH(x) ((chip->data_bus2 & x) == 0 && (chip->data_bus1 & (x^255)) == 0)

        write_0_en = chip->write0_l[5];
        write_1_en = chip->write1_l[5];
        write_p_en = chip->writep_l[2];

        if (chip->mclk1)
        {
            int addr2d = write_p_en && ADDRESS_MATCH(0x2d);
            int addr2e = write_p_en && ADDRESS_MATCH(0x2e);
            int addr2f = write_p_en && ADDRESS_MATCH(0x2f);
            chip->prescaler_sel[0] = chip->prescaler_sel[1];
            if (addr2f)
                chip->prescaler_sel[0] = 0;
            if (addr2d || ic)
                chip->prescaler_sel[0] |= 2;
            if (addr2e)
                chip->prescaler_sel[0] |= 1;
        }
        if (chip->mclk2)
        {
            chip->prescaler_sel[1] = chip->prescaler_sel[0];
            if (ic)
                chip->prescaler_sel[1] &= ~1;
        }
    }

#if 1
    if (chip->o_clk1 == chip->clk1
        && chip->o_clk2 == chip->clk2
        && chip->o_aclk1 == chip->aclk1
        && chip->o_aclk2 == chip->aclk2)
        return;

    chip->o_clk1 = chip->clk1;
    chip->o_clk2 = chip->clk2;
    chip->o_aclk1 = chip->aclk1;
    chip->o_aclk2 = chip->aclk2;

    if (!chip->aclk1 && !chip->aclk2)
        return;
#endif

    {
        if (chip->clk1)
        {
            int is_fm = (chip->data_bus1 & 0xe0) != 0;

            chip->write_fm_address[0] = write_0_en ? is_fm : chip->write_fm_address[1];

            if (ic)
                chip->fm_address[0] = 0;
            else if (is_fm && write_0_en)
                chip->fm_address[0] = chip->data_bus1;
            else
                chip->fm_address[0] = chip->fm_address[1];

            if (ic)
                chip->fm_data[0] = 0;
            else if (chip->write_fm_address[1] && write_1_en)
                chip->fm_data[0] = chip->data_bus1 & 255;
            else
                chip->fm_data[0] = chip->fm_data[1];

            chip->write_fm_data[0] = (chip->write_fm_address[1] && write_1_en) || (chip->write_fm_data[1] && !write_0_en);

            chip->addr_21[0] = write_0_en ? ADDRESS_MATCH(0x21) : chip->addr_21[1];
            chip->addr_24[0] = write_0_en ? ADDRESS_MATCH(0x24) : chip->addr_24[1];
            chip->addr_25[0] = write_0_en ? ADDRESS_MATCH(0x25) : chip->addr_25[1];
            chip->addr_26[0] = write_0_en ? ADDRESS_MATCH(0x26) : chip->addr_26[1];
            chip->addr_27[0] = write_0_en ? ADDRESS_MATCH(0x27) : chip->addr_27[1];
            chip->addr_28[0] = write_0_en ? ADDRESS_MATCH(0x28) : chip->addr_28[1];

            if (ic)
            {
                chip->reg_test_21[0] = 0;
                chip->reg_timer_a[0] = 0;
                chip->reg_timer_b[0] = 0;
                chip->reg_ch3[0] = 0;
                chip->reg_timer_a_load[0] = 0;
                chip->reg_timer_b_load[0] = 0;
                chip->reg_timer_a_enable[0] = 0;
                chip->reg_timer_b_enable[0] = 0;
                chip->reg_kon_operator[0] = 0;
                chip->reg_kon_channel[0] = 0;
            }
            else
            {
                if (chip->addr_21[1] && write_1_en)
                {
                    chip->reg_test_21[0] = chip->data_bus1 & 0xde;
                }
                else
                {
                    chip->reg_test_21[0] = chip->reg_test_21[1];
                }
                chip->reg_timer_a[0] = 0;
                if (chip->addr_24[1] && write_1_en)
                {
                    chip->reg_timer_a[0] |= (chip->data_bus1 & 255) << 2;
                }
                else
                {
                    chip->reg_timer_a[0] |= chip->reg_timer_a[1] & 0x3fc;
                }
                if (chip->addr_25[1] && write_1_en)
                {
                    chip->reg_timer_a[0] |= chip->data_bus1 & 3;
                }
                else
                {
                    chip->reg_timer_a[0] |= chip->reg_timer_a[1] & 3;
                }
                if (chip->addr_26[1] && write_1_en)
                {
                    chip->reg_timer_b[0] = chip->data_bus1 & 255;
                }
                else
                {
                    chip->reg_timer_b[0] = chip->reg_timer_b[1];
                }
                if (chip->addr_27[1] && write_1_en)
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
                if (chip->addr_28[1] && write_1_en)
                {
                    chip->reg_kon_operator[0] = (chip->data_bus1 >> 4) & 15;
                    chip->reg_kon_channel[0] = (chip->data_bus1 >> 0) & 3;
                }
                else
                {
                    chip->reg_kon_operator[0] = chip->reg_kon_operator[1];
                    chip->reg_kon_channel[0] = chip->reg_kon_channel[1];
                }
            }
            chip->reg_timer_a_reset[0] = chip->addr_27[1] && write_1_en && ((chip->data_bus1 >> 4) & 1) != 0;
            chip->reg_timer_b_reset[0] = chip->addr_27[1] && write_1_en && ((chip->data_bus1 >> 5) & 1) != 0;

            int rst1 = chip->reg_cnt_sync;
            int of = (chip->reg_cnt1[1] & 2) != 0;

            chip->reg_cnt1[0] = (rst1 || of) ? 0 : ((chip->reg_cnt1[1] + 1) & 3);
            chip->reg_cnt2[0] = rst1 ? 0 : ((chip->reg_cnt2[1] + of) & 3);

            int of2 = (chip->reg_key_cnt1[1] & 2) != 0;
            chip->reg_key_cnt1[0] = (rst1 || of2) ? 0 : ((chip->reg_key_cnt1[1] + 1) & 3);
            chip->reg_key_cnt2[0] = rst1 ? 0 : ((chip->reg_key_cnt2[1] + of2) & 3);

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
                if (!ic)
                    chip->reg_kon[0][0] |= (chip->reg_kon[3][1] >> 2) & 1;
                chip->reg_kon[1][0] |= (chip->reg_kon[0][1] >> 2) & 1;
                chip->reg_kon[2][0] |= (chip->reg_kon[1][1] >> 2) & 1;
                chip->reg_kon[3][0] |= (chip->reg_kon[2][1] >> 2) & 1;
            }

            chip->reg_sync_timer_l[0] = chip->reg_sync_timer;

            int time = chip->timer_a_cnt[1];
            time += (chip->reg_test_21[1] & 4) != 0 || (chip->timer_a_reg_load && chip->reg_sync_timer);

            chip->timer_a_cnt[0] = chip->timer_a_load ? chip->reg_timer_a[1] : (!chip->timer_a_reg_load ? 0 : (time & 1023));
            chip->timer_a_of[0] = (time & 1024) != 0;

            chip->timer_a_reg_load_l[0] = chip->timer_a_reg_load;

            int rst_a = chip->reg_timer_a_reset[1] || ic;

            if (rst_a)
                chip->timer_a_status[0] = 0;
            else
                chip->timer_a_status[0] = chip->timer_a_status[1];

            chip->timer_a_status[0] |= !rst_a && chip->reg_timer_a_enable[1] && chip->timer_a_of[1];

            int subcnt = chip->timer_b_subcnt[1] + chip->reg_sync_timer;
            chip->timer_b_subcnt[0] = ic ? 0 : subcnt & 15;
            chip->timer_b_subcnt_of[0] = (subcnt & 16) != 0;

            time = chip->timer_b_cnt[1];
            time += (chip->reg_test_21[1] & 4) != 0 || (chip->timer_b_reg_load && chip->timer_b_subcnt_of[1]);

            chip->timer_b_cnt[0] = chip->timer_b_load ? chip->reg_timer_b[1] : (!chip->timer_b_reg_load ? 0 : (time & 255));
            chip->timer_b_of[0] = (time & 256) != 0;

            chip->timer_b_reg_load_l[0] = chip->timer_b_reg_load;

            int rst_b = chip->reg_timer_b_reset[1] || ic;

            if (rst_b)
                chip->timer_b_status[0] = 0;
            else
                chip->timer_b_status[0] = chip->timer_b_status[1];

            chip->timer_b_status[0] |= !rst_b && chip->reg_timer_b_enable[1] && chip->timer_b_of[1];


            memcpy(&chip->reg_freq[0][1], &chip->reg_freq[1][0], 2 * sizeof(unsigned short));
            memcpy(&chip->reg_freq_3ch[0][1], &chip->reg_freq_3ch[1][0], 2 * sizeof(unsigned short));
            memcpy(&chip->reg_connect_fb[0][1], &chip->reg_connect_fb[1][0], 2 * sizeof(unsigned char));
            memcpy(&chip->op_multi_dt[0][1], &chip->op_multi_dt[1][0], 11 * sizeof(unsigned char));
            memcpy(&chip->op_tl[0][1], &chip->op_tl[1][0], 11 * sizeof(unsigned char));
            memcpy(&chip->op_ar_ks[0][1], &chip->op_ar_ks[1][0], 11 * sizeof(unsigned char));
            memcpy(&chip->op_dr_a[0][1], &chip->op_dr_a[1][0], 11 * sizeof(unsigned char));
            memcpy(&chip->op_sr[0][1], &chip->op_sr[1][0], 11 * sizeof(unsigned char));
            memcpy(&chip->op_rr_sl[0][1], &chip->op_rr_sl[1][0], 11 * sizeof(unsigned char));
            memcpy(&chip->op_ssg[0][1], &chip->op_ssg[1][0], 11 * sizeof(unsigned char));

            if (ic)
            {
                chip->reg_a4[0] = 0;
                chip->reg_freq[0][0] = 0;
                chip->reg_ac[0] = 0;
                chip->reg_freq_3ch[0][0] = 0;
                chip->reg_connect_fb[0][0] = 0;

                chip->op_multi_dt[0][0] = 0;
                chip->op_tl[0][0] = 0;
                chip->op_ar_ks[0][0] = 0;
                chip->op_dr_a[0][0] = 0;
                chip->op_sr[0][0] = 0;
                chip->op_rr_sl[0][0] = 0;
                chip->op_ssg[0][0] = 0;
            }
            else
            {
                chip->reg_a4[0] = chip->fm_isa4 ? (chip->fm_data[1] & 0x3f) : chip->reg_a4[1];
                chip->reg_freq[0][0] = chip->fm_isa0 ? (chip->fm_data[1] & 0xff) | (chip->reg_a4[1] << 8) : chip->reg_freq[1][2];
                chip->reg_ac[0] = chip->fm_isac ? (chip->fm_data[1] & 0x3f) : chip->reg_ac[1];
                chip->reg_freq_3ch[0][0] = chip->fm_isa8 ? (chip->fm_data[1] & 0xff) | (chip->reg_ac[1] << 8) : chip->reg_freq_3ch[1][2];
                chip->reg_connect_fb[0][0] = chip->fm_isb0 ? (chip->fm_data[1] & 0x3f) : chip->reg_connect_fb[1][2];

                chip->op_multi_dt[0][0] = chip->fm_is30 ? (chip->fm_data[1] & 0x7f) : chip->op_multi_dt[1][11];
                chip->op_tl[0][0] = chip->fm_is40 ? (chip->fm_data[1] & 0x7f) : chip->op_tl[1][11];
                chip->op_ar_ks[0][0] = chip->fm_is50 ? (chip->fm_data[1] & 0xdf) : chip->op_ar_ks[1][11];
                chip->op_dr_a[0][0] = chip->fm_is60 ? (chip->fm_data[1] & 0x9f) : chip->op_dr_a[1][11];
                chip->op_sr[0][0] = chip->fm_is70 ? (chip->fm_data[1] & 0x1f) : chip->op_sr[1][11];
                chip->op_rr_sl[0][0] = chip->fm_is80 ? (chip->fm_data[1] & 0xff) : chip->op_rr_sl[1][11];
                chip->op_ssg[0][0] = chip->fm_is90 ? (chip->fm_data[1] & 0xf) : chip->op_ssg[1][11];
            }


            int freq = 0;
            if (chip->ch3_en && (chip->ch3_sel[1] & 2) != 0)
                freq |= chip->reg_freq_3ch[1][2];
            if (chip->ch3_en && (chip->ch3_sel[1] & 16) != 0)
                freq |= chip->reg_freq_3ch[1][0];
            if (chip->ch3_en && (chip->ch3_sel[1] & 128) != 0)
                freq |= chip->reg_freq_3ch[1][1];
            if (chip->reg_cnt_sync || (chip->ch3_sel[1] & (1+4+8+32+64+256+512+1024)) != 0 || (!chip->ch3_en && (chip->ch3_sel[1] & (2+16+128)) != 0))
                freq |= chip->reg_freq[1][1];

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
            chip->fnum[4] = chip->fnum[3];

            chip->ch3_sel[0] = (chip->ch3_sel[1] << 1) | chip->reg_cnt_sync;
        }
        if (chip->clk2)
        {
            chip->fm_address[1] = chip->fm_address[0];
            chip->write_fm_address[1] = chip->write_fm_address[0];
            chip->fm_data[1] = chip->fm_data[0];
            chip->write_fm_data[1] = chip->write_fm_data[0];

            chip->addr_21[1] = chip->addr_21[0];
            chip->addr_24[1] = chip->addr_24[0];
            chip->addr_25[1] = chip->addr_25[0];
            chip->addr_26[1] = chip->addr_26[0];
            chip->addr_27[1] = chip->addr_27[0];
            chip->addr_28[1] = chip->addr_28[0];

            chip->reg_test_21[1] = chip->reg_test_21[0];
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
                && (chip->reg_cnt2[0] & 3) == ((chip->fm_address[0] >> 2) & 3);
            int ch_match = chip->write_fm_data[0] && (chip->reg_cnt1[0] == (chip->fm_address[0] & 3));

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

            chip->reg_cnt1[1] = chip->reg_cnt1[0];
            chip->reg_cnt2[1] = chip->reg_cnt2[0];

            chip->reg_kon_match = chip->reg_key_cnt1[0] == (chip->reg_kon_channel[0] & 3)
                && chip->reg_key_cnt2[0] == 0;

            chip->reg_key_cnt1[1] = chip->reg_key_cnt1[0];
            chip->reg_key_cnt2[1] = chip->reg_key_cnt2[0];

            chip->reg_kon[0][1] = chip->reg_kon[0][0];
            chip->reg_kon[1][1] = chip->reg_kon[1][0];
            chip->reg_kon[2][1] = chip->reg_kon[2][0];
            chip->reg_kon[3][1] = chip->reg_kon[3][0];

            chip->ch3_en = chip->reg_ch3[0] != 0;
            chip->ch3_csm = chip->reg_ch3[0] == 2;

            chip->timer_a_cnt[1] = chip->timer_a_cnt[0];
            chip->timer_a_of[1] = chip->timer_a_of[0];

            chip->reg_sync_timer = (chip->ch3_sel[0] & 2) != 0;
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

            chip->timer_a_status[1] = chip->timer_a_status[0];

            chip->timer_b_subcnt[1] = chip->timer_b_subcnt[0];
            chip->timer_b_subcnt_of[1] = chip->timer_b_subcnt_of[0];

            chip->timer_b_cnt[1] = chip->timer_b_cnt[0];
            chip->timer_b_of[1] = chip->timer_b_of[0];

            chip->timer_b_reg_load_l[1] = chip->timer_b_reg_load_l[0];

            chip->timer_b_load = chip->timer_b_of[1] || (!chip->timer_b_reg_load_l[1] && chip->timer_b_reg_load);

            chip->timer_b_status[1] = chip->timer_b_status[0];

            memcpy(&chip->reg_freq[1][0], &chip->reg_freq[0][0], 3 * sizeof(unsigned short));
            memcpy(&chip->reg_freq_3ch[1][0], &chip->reg_freq_3ch[0][0], 3 * sizeof(unsigned short));
            memcpy(&chip->reg_connect_fb[1][0], &chip->reg_connect_fb[0][0], 3 * sizeof(unsigned char));
            memcpy(&chip->op_multi_dt[1][0], &chip->op_multi_dt[0][0], 12 * sizeof(unsigned char));
            memcpy(&chip->op_tl[1][0], &chip->op_tl[0][0], 12 * sizeof(unsigned char));
            memcpy(&chip->op_ar_ks[1][0], &chip->op_ar_ks[0][0], 12 * sizeof(unsigned char));
            memcpy(&chip->op_dr_a[1][0], &chip->op_dr_a[0][0], 12 * sizeof(unsigned char));
            memcpy(&chip->op_sr[1][0], &chip->op_sr[0][0], 12 * sizeof(unsigned char));
            memcpy(&chip->op_rr_sl[1][0], &chip->op_rr_sl[0][0], 12 * sizeof(unsigned char));
            memcpy(&chip->op_ssg[1][0], &chip->op_ssg[0][0], 12 * sizeof(unsigned char));

            chip->reg_a4[1] = chip->reg_a4[0];
            chip->reg_ac[1] = chip->reg_ac[0];

            chip->fnum[1] = chip->fnum[0];
            chip->fnum[3] = chip->fnum[2];

            chip->ch3_sel[1] = chip->ch3_sel[0];

            chip->kcode[1] = chip->kcode[0];
            chip->kcode[3] = chip->kcode[2];

            chip->o_irq_pull = chip->timer_a_status[0] || chip->timer_b_status[0];

            chip->reg_ch3_sel = chip->fsm_ch3_sync[1];
        }
    }

    {
        if (chip->clk1)
        {
            chip->ic_latch[0] = chip->ic_latch[1];

            int fsm_ic = (chip->ic_latch[1] & 4) != 0;

            int fsm_cnt;
            int of1 = (chip->fsm_cnt1[1] & 2) != 0;
            chip->fsm_cnt1[0] = (fsm_ic || of1) ? 0 : (chip->fsm_cnt1[1] + 1) & 3;
            chip->fsm_cnt2[0] = fsm_ic ? 0 : (chip->fsm_cnt2[1] + of1) & 3;

            fsm_cnt = (chip->fsm_cnt2[0] << 2) | chip->fsm_cnt1[0];

            // 0 - 0000
            // 1 - 0001
            // 2 - 0010
            // 3 - 0100
            // 4 - 0101
            // 5 - 0110
            // 6 - 1000
            // 7 - 1001
            // 8 - 1010
            // 9 - 1100
            // 10 - 1101
            // 11 - 1110

            chip->fsm_out[0] = (fsm_cnt & 14) == 4; // 3, 4
            chip->fsm_out[1] = fsm_cnt == 2; // 2
            chip->fsm_out[2] = fsm_cnt == 14; // 11
            chip->fsm_out[3] = (fsm_cnt & 14) == 0; // 0, 1
            chip->fsm_out[4] = (fsm_cnt & 14) == 12; // 9, 10
            chip->fsm_out[5] = fsm_cnt == 10; // 8
            chip->fsm_out[6] = fsm_cnt == 6; // 5
            chip->fsm_out[7] = (fsm_cnt & 14) == 8; // 6, 7

            chip->fsm_out[8] = (fsm_cnt & 14) == 12; // 9, 10
            chip->fsm_out[9] = fsm_cnt == 10; // 8
            chip->fsm_out[10] = (fsm_cnt & 13) == 9; // 7
            chip->fsm_out[11] = (fsm_cnt & 3) == 1; // 1, 4, 7, 10
            chip->fsm_out[12] = (fsm_cnt & 14) == 10; // 8
            chip->fsm_out[13] = (fsm_cnt & 13) == 13; // 10

            chip->fsm_sh_l[0] = (chip->fsm_sh_l[1] << 1) | chip->fsm_sh;

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

            chip->fsm_sel_11[1] = chip->fsm_sel_11[0];

            chip->fsm_acc_sync[1] = chip->fsm_acc_sync[0];
            chip->fsm_ch3_sync[1] = chip->fsm_ch3_sync[0];
        }
        if (chip->clk2)
        {
            chip->ic_latch[1] = (chip->ic_latch[0] << 1) | ic;

            chip->fsm_cnt1[1] = chip->fsm_cnt1[0];
            chip->fsm_cnt2[1] = chip->fsm_cnt2[0];

            chip->fsm_op4_sel_l = chip->fsm_out[0] || chip->fsm_out[1]; // 2, 3, 4
            chip->fsm_op2_sel_l = chip->fsm_out[2] || chip->fsm_out[3]; // 11, 0, 1
            chip->fsm_op3_sel_l = chip->fsm_out[4] || chip->fsm_out[5]; // 8, 9, 10
            chip->fsm_op1_sel_l = chip->fsm_out[6] || chip->fsm_out[7]; // 5, 6, 7

            chip->fsm_connect = chip->reg_connect_fb[0][1] & 7;

            chip->alg_do_fb[0] = chip->alg_mod_op1_1_l;

            chip->fsm_sh = chip->fsm_out[8] || chip->fsm_out[9] || chip->fsm_out[10]; // 7, 8, 9, 10

            chip->fsm_sh_l[1] = chip->fsm_sh_l[0];

            chip->reg_cnt_sync = chip->fsm_sel_11[1];

            chip->fsm_sel_11[0] = chip->fsm_out[13];

            chip->fsm_acc_sync[0] = chip->fsm_out[12];
            chip->fsm_ch3_sync[0] = chip->fsm_out[11];
        }
    }

    chip->o_sh = (chip->fsm_sh_l[0] >> 8) & 1;


    {
        if (chip->clk1)
        {
            chip->pg_block = chip->kcode[3] >> 2;

            chip->pg_dt_multi = chip->op_multi_dt[1][11];

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

            memcpy(&chip->pg_phase[0][1], &chip->pg_phase[1][0], 10 * sizeof(int));

            chip->pg_phase2[0] = chip->pg_phase[1][10];

            chip->pg_phase[0][0] = (chip->pg_phase2[1] + chip->pg_add[5]) & 0xfffff;

            chip->pg_dbg[0] = chip->pg_dbg[1] >> 1;
            if (chip->pg_dbgsync)
                chip->pg_dbg[0] |= chip->pg_phase[1][10] & 1023;

        }
        if (chip->clk2)
        {
            chip->pg_freq = (chip->fnum[4] << chip->pg_block) >> 1;

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
            memcpy(&chip->pg_phase[1][0], &chip->pg_phase[0][0], 11 * sizeof(int));

            chip->pg_out = chip->pg_phase[1][6] >> 10;

            chip->pg_phase2[1] = chip->pg_reset[3] ? 0 : chip->pg_phase2[0];

            chip->pg_dbgsync = (chip->ch3_sel[0] & 4) != 0;

            chip->pg_dbg[1] = chip->pg_dbg[0];
        }
    }

    {
        if (chip->clk1)
        {
            chip->eg_prescaler_clock_l[0] = chip->eg_sync;
            chip->eg_prescaler[0] = (chip->eg_prescaler[1] + chip->eg_sync) & 3;
            if (((chip->eg_prescaler[1] & 2) != 0 && chip->eg_sync) || ic)
                chip->eg_prescaler[0] = 0;
            chip->eg_step[0] = chip->eg_prescaler[1] >> 1;
            chip->eg_step[2] = chip->eg_step[1];
            chip->eg_timer_step[1] = chip->eg_timer_step[0];

            chip->eg_ic[0] = ic;


            int sum = (chip->eg_timer[1] >> 10) & 1;
            int add = chip->eg_timer_carry[1];
            if ((chip->eg_prescaler[1] & 2) != 0 && chip->eg_prescaler_clock_l[1])
                add = 1;
            sum += add;

            chip->eg_timer[0] = (chip->eg_timer[1] << 1) | chip->eg_timer_sum[1];

            chip->eg_timer_carry[0] = sum >> 1;
            chip->eg_timer_sum[0] = sum & 1;

            int timer_bit = chip->eg_timer_sum[1];

            chip->eg_timer_mask[0] = timer_bit | chip->eg_timer_mask[1];
            if (chip->eg_prescaler_clock_l[1] || chip->eg_ic[1])
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

            chip->eg_rate_ar = chip->op_ar_ks[1][11] & 0x1f;
            chip->eg_ks = (chip->op_ar_ks[1][11] >> 6) & 3;
            chip->eg_rate_dr = chip->op_dr_a[1][11] & 0x1f;
            chip->eg_rate_sr = chip->op_sr[1][11] & 0x1f;
            chip->eg_rate_rr = chip->op_rr_sl[1][11] & 0xf;

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

            int kon_comb;

            kon_comb = (chip->reg_kon[3][1] >> 2) & 1;
            kon_comb |= chip->reg_ch3_sel && chip->ch3_csm_load;

            chip->eg_kon_latch[0] = (chip->eg_kon_latch[1] << 1) | kon_comb;
            int csm_kon = chip->reg_ch3_sel && chip->ch3_csm_load;
            chip->eg_kon_csm[0] = (chip->eg_kon_csm[1] << 1) | csm_kon;

            int kon = (chip->eg_kon_latch[1] >> 1) & 1;
            int okon = (chip->eg_key[1] >> 11) & 1;
            int pg_reset = (kon && !okon) || (chip->eg_ssg_pgreset[1] & 2) != 0;
            chip->eg_pg_reset[0] = (chip->eg_pg_reset[1] << 1) | pg_reset;
            chip->eg_kon_event = (kon && !okon) || (okon && (chip->eg_ssg_egrepeat[1] & 2) != 0);

            chip->eg_key[0] = (chip->eg_key[1] << 1) | kon;

            int okon2 = (chip->eg_key[1] >> 9) & 1;


            chip->eg_ssg_sign[0] = (chip->eg_level[1][6] & 0x200) != 0;

            int ssg_eg = chip->op_ssg[1][11] & 15;
            int ssg_enable = (ssg_eg & 8) != 0;
            chip->eg_ssg_enable[0] = (chip->eg_ssg_enable[1] << 1) | ssg_enable;
            int ssg_inv_e = ssg_enable && (ssg_eg & 4) != 0;
            int ssg_holdup = ssg_enable && ((ssg_eg & 7) == 3 || (ssg_eg & 7) == 5) && kon_comb;
            chip->eg_ssg_holdup[0] = (chip->eg_ssg_holdup[1] << 1) | ssg_holdup;
            int ssg_pgreset = ssg_enable && chip->eg_ssg_sign[1] && (ssg_eg & 3) == 0;
            chip->eg_ssg_pgreset[0] = (chip->eg_ssg_pgreset[1] << 1) | ssg_pgreset;
            int ssg_egrepeat = ssg_enable && chip->eg_ssg_sign[1] && (ssg_eg & 1) == 0;
            chip->eg_ssg_egrepeat[0] = (chip->eg_ssg_egrepeat[1] << 1) | ssg_egrepeat;

            chip->eg_rate_sel = (okon2 ? ssg_egrepeat : kon_comb) ? eg_state_attack : chip->eg_state[1][8];

            int ssg_odir = (chip->eg_ssg_dir[1] >> 11) & 1;
            int ssg_dir = ssg_enable && okon2 &&
                ((ssg_odir ^ ((ssg_eg & 3) == 2 && chip->eg_ssg_sign[1])) || ((ssg_eg & 3) == 3 && chip->eg_ssg_sign[1]));
            chip->eg_ssg_dir[0] = (chip->eg_ssg_dir[1] << 1) | ssg_dir;

            int ssg_inv = okon2 && (ssg_odir ^ ssg_inv_e);

            chip->eg_ssg_inv = ssg_inv;

            chip->eg_level_ssg[0] = chip->eg_output;

            int sl = (chip->op_rr_sl[1][11] >> 4) & 15;

            if (sl == 15)
                sl |= 16;

            chip->eg_sl[0][0] = sl;
            chip->eg_sl[1][0] = chip->eg_sl[0][1];
            chip->eg_tl[0][0] = chip->op_tl[1][11];
            chip->eg_tl[1][0] = chip->eg_tl[0][1];
            chip->eg_tl[2][0] = chip->eg_tl[1][1];

            int level = (okon && !kon) ? chip->eg_level_ssg[1] : chip->eg_level[1][9];

            chip->eg_off = (chip->eg_ssg_enable[1] & 2) != 0 ? (level & 512) != 0 : (level & 0x3f0) == 0x3f0;
            chip->eg_slreach = (level >> 4) == (chip->eg_sl[1][1] << 1);
            chip->eg_zeroreach = level == 0;
            
            chip->eg_level_l[0] = level;

            chip->eg_state_l = chip->eg_state[1][10];

            memcpy(&chip->eg_state[0][1], &chip->eg_state[1][0], 10 * sizeof(unsigned char));
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

            if (chip->eg_mute || ic)
                nextlevel |= 0x3ff;

            chip->eg_nextlevel[0] = nextlevel;

            memcpy(&chip->eg_level[0][1], &chip->eg_level[1][0], 9 * sizeof(unsigned short));
            chip->eg_level[0][0] = chip->eg_nextlevel[1];

            chip->eg_output2 = chip->eg_output & 1023;

            chip->eg_ch3_l[1] = chip->eg_ch3_l[0];

            chip->eg_csm_tl = (chip->ch3_csm && (chip->eg_ch3_l[0] & 2) != 0) ? 0 : chip->eg_tl[1][0];

            chip->eg_debug[0] = chip->eg_debug[1] << 1;

            if (chip->eg_dbg_sync)
            {
                chip->eg_debug[0] |= chip->eg_out;
            }
        }
        if (chip->clk2)
        {
            chip->eg_sync = (chip->ch3_sel[0] & 1) != 0;
            chip->eg_prescaler_clock_l[1] = chip->eg_prescaler_clock_l[0];
            chip->eg_prescaler[1] = chip->eg_prescaler[0];
            chip->eg_step[1] = chip->eg_step[0];
            chip->eg_timer_step[0] = chip->eg_step[0] && chip->eg_prescaler_clock_l[0];

            chip->eg_ic[1] = chip->eg_ic[0];

            chip->eg_timer_sum[1] = chip->eg_timer_sum[0] && !chip->eg_ic[0];
            chip->eg_timer[1] = chip->eg_timer[0];
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

            int inv = ((chip->eg_level[0][8] ^ 1023) + 513) & 1023;

            chip->eg_output = chip->eg_ssg_inv ? inv : chip->eg_level[0][8];

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
            memcpy(&chip->eg_state[1][0], &chip->eg_state[0][0], 11 * sizeof(unsigned char));

            chip->eg_exp = (chip->eg_kon_latch[0] & 4) != 0 && (chip->eg_state_l == eg_state_attack) && !chip->eg_maxrate[1] && !chip->eg_zeroreach;
            chip->eg_linear = !chip->eg_kon_event && !chip->eg_off && (chip->eg_state_l == eg_state_sustain || chip->eg_state_l == eg_state_release);
            chip->eg_linear |= !chip->eg_kon_event && !chip->eg_off && !chip->eg_slreach && chip->eg_state_l == eg_state_decay;

            chip->eg_istantattack = chip->eg_maxrate[1] && (!chip->eg_maxrate[1] || chip->eg_kon_event);

            memcpy(&chip->eg_level[1][0], &chip->eg_level[0][0], 10 * sizeof(unsigned short));

            chip->eg_ch3_l[0] = (chip->eg_ch3_l[1] << 1) | chip->fsm_ch3_sync[1];

            int levelsum = chip->eg_output2 + (chip->eg_csm_tl << 3);

            int eg_of = (levelsum & 1024) != 0;

            if (eg_of)
                levelsum = 1023;

            chip->eg_out = levelsum;

            chip->eg_dbg_sync = (chip->ch3_sel[0] & 4) != 0;

            chip->eg_debug[1] = chip->eg_debug[0];
        }
    }

    {
        if (chip->clk1)
        {
            chip->op_phase1 = chip->pg_out;

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

            memcpy(&chip->op_op1_0[0][1], &chip->op_op1_0[1][0], 2 * sizeof(unsigned short));
            memcpy(&chip->op_op1_1[0][1], &chip->op_op1_1[1][0], 2 * sizeof(unsigned short));
            memcpy(&chip->op_op2[0][1], &chip->op_op2[1][0], 2 * sizeof(unsigned short));

            chip->op_op1_0[0][0] = chip->op_loadfb ? chip->op_output[3] : chip->op_op1_0[1][2];
            chip->op_op1_1[0][0] = chip->op_loadfb ? chip->op_op1_0[1][2] : chip->op_op1_1[1][2];
            chip->op_op2[0][0] = chip->op_loadop2 ? chip->op_output[3] : chip->op_op2[1][2];

            int mod1 = 0;
            int mod2 = 0;

            if (chip->op_mod_op1_0)
            {
                mod2 |= chip->op_op1_0[1][2];
            }
            if (chip->op_mod_op1_1)
            {
                mod1 |= chip->op_op1_1[1][2];
            }
            if (chip->op_mod_op2)
            {
                mod1 |= chip->op_op2[1][2];
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

            chip->op_phase2 = mod;
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

            memcpy(&chip->op_op1_0[1][0], &chip->op_op1_0[0][0], 3 * sizeof(unsigned short));
            memcpy(&chip->op_op1_1[1][0], &chip->op_op1_1[0][0], 3 * sizeof(unsigned short));
            memcpy(&chip->op_op2[1][0], &chip->op_op2[0][0], 3 * sizeof(unsigned short));

            int mod = (chip->op_mod1 + chip->op_mod2) >> 1;
            mod &= 0x3fff;
            chip->op_mod_sum = mod;
        }
    }
    {
        int ssg_write1 = ic || write_data;

        if (!chip->mclk2)
        {
            int addr2d = write_p_en && ADDRESS_MATCH(0x2d);
            int addr2e = write_p_en && ADDRESS_MATCH(0x2e);
            int addr2f = write_p_en && ADDRESS_MATCH(0x2f);
            chip->ssg_prescaler1[0] = (chip->ssg_prescaler1[1] && !addr2f) || addr2e;
            chip->ssg_prescaler2[0] = (chip->ssg_prescaler2[1] && !addr2f) || addr2d || ic;
        }
        else
        {
            chip->ssg_prescaler1[1] = chip->ssg_prescaler1[0] && !ic;
            chip->ssg_prescaler2[1] = chip->ssg_prescaler2[0];
        }
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
        if (ic)
        {
            chip->ssg_div1[0] = 0;
            chip->ssg_div1[1] = 0;
            chip->ssg_div2[0] = 0;
            chip->ssg_div2[1] = 0;
        }

        if (chip->ssg_prescaler2[1])
        {
            if (chip->ssg_prescaler1[1])
            {
                chip->ssg_clk = chip->ssg_div1[0];
            }
            else
            {
                chip->ssg_clk = chip->ssg_div2[0];
            }
        }
        else
            chip->ssg_clk = chip->mclk2;

        chip->ssg_clk1 = !chip->ssg_clk;
        chip->ssg_clk2 = chip->ssg_clk;

        if (ic)
            chip->ssg_ssg_addr = 0;
        else if (write_addr)
            chip->ssg_ssg_addr = (chip->data_bus1 & 0xe0) == 0;

        if (ic)
            chip->ssg_address = 0;
        else if (write_addr && (chip->data_bus1 & 0xe0) == 0)
            chip->ssg_address = chip->data_bus1 & 0x1f;
        
        int ssg_access = chip->ssg_ssg_addr && (write_data || read1);

        if (chip->ssg_egtrig_rst)
            chip->ssg_egtrig = 0;

        if (ic)
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
            chip->o_gpio_a = chip->data_bus1 & 255;
            chip->o_gpio_b = chip->data_bus1 & 255;

            chip->ssg_egtrig = 1;
        }
        else if (ssg_access && ssg_write1)
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
                case 0xe:
                    chip->o_gpio_a = chip->data_bus1 & 255;
                    break;
                case 0xf:
                    chip->o_gpio_b = chip->data_bus1 & 255;
                    break;
            }
        }
        if (ssg_access && read1)
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
                case 0xe:
                    chip->data_bus1 &= ~255;
                    chip->data_bus1 |= chip->input.gpio_a & 255;
                    break;
                case 0xf:
                    chip->data_bus1 &= ~255;
                    chip->data_bus1 |= chip->input.gpio_b & 255;
                    break;
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
            chip->ssg_sel[0] |= (chip->ssg_sel[1] & 7) == 0 && !ic;

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

            chip->ssg_noise_step = chip->ssg_noise_of || ic;
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

            if (ic)
                chip->ssg_sign[0] = 0;
            else
                chip->ssg_sign[0] = chip->ssg_sign[1] ^ chip->ssg_sign_toggle;

            int cnt = chip->ssg_freq_cnt2[7] + chip->ssg_cnt2_add;
            chip->ssg_freq_cnt2[0] = cnt & 0xf;
            chip->ssg_freq_cnt2[2] = chip->ssg_fr_rst_l ? 0 : chip->ssg_freq_cnt2[1];
            chip->ssg_freq_cnt2[4] = chip->ssg_freq_cnt2[3];
            chip->ssg_freq_cnt2[6] = chip->ssg_freq_cnt2[5];

            chip->ssg_sel_eg_l[1] = chip->ssg_sel_eg_l[0];

            chip->ssg_ch_of = (!chip->ssg_sel_eg_l[0] && cnt_of) || ic;

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
            chip->ssg_noise_lfsr[1] = ic ? 0 : chip->ssg_noise_lfsr[0];
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


        chip->o_analog_a = volume_lut[sign_a ? 0 : vol_a];
        chip->o_analog_b = volume_lut[sign_b ? 0 : vol_b];
        chip->o_analog_c = volume_lut[sign_c ? 0 : vol_c];

        chip->o_gpio_a_d = (chip->ssg_mode & 64) == 0;
        chip->o_gpio_b_d = (chip->ssg_mode & 128) == 0;
    }

    {
        if (chip->clk1)
        {
            chip->ac_accum[0] = chip->ac_accum[1] + chip->ac_fm_output;

            chip->ac_sync[1] = chip->ac_sync[0];
        }
        if (chip->clk2)
        {
            if (chip->alg_output_l)
            {
                chip->ac_fm_output = chip->op_output[2] & 0x1fff;
                if (chip->op_output[2] & 0x2000)
                    chip->ac_fm_output |= 0x3e000;
            }
            else
                chip->ac_fm_output = 0;

            chip->ac_accum[1] = (chip->ac_sync[1] & 2) != 0 ? 0 : chip->ac_accum[0];
            chip->ac_accum_l = chip->ac_accum[0];

            chip->ac_sync[0] = (chip->ac_sync[1] << 1) | chip->fsm_acc_sync[1];
        }

        int load = (chip->ac_sync[0] & 4) != 0 && (chip->ac_sync[1] & 4) == 0;
        if (chip->aclk2)
            chip->ac_load2_l = (chip->ac_sync[0] & 1) != 0;

        int load2 = (chip->ac_sync[0] & 1) != 0 && !chip->ac_load2_l;

        if (load)
        {
            chip->ac_clip_l = (chip->ac_accum_l & 0x38000) == 0x30000
                           || (chip->ac_accum_l & 0x38000) == 0x28000
                           || (chip->ac_accum_l & 0x38000) == 0x20000;
            chip->ac_clip_h = (chip->ac_accum_l & 0x38000) == 0x18000
                           || (chip->ac_accum_l & 0x38000) == 0x10000
                           || (chip->ac_accum_l & 0x38000) == 0x8000;
        }

        if (chip->aclk2)
        {
            chip->ac_shifter1[0] = chip->ac_shifter1[1] >> 1;
            if (load)
            {
                chip->ac_shifter1[0] = chip->ac_accum_l & 0x7fff;
                if ((chip->ac_accum_l & 0x20000) == 0)
                    chip->ac_shifter1[0] |= 0x8000;
                chip->dbg_out = chip->ac_accum_l;
            }

            chip->ac_shifter2[0] = chip->ac_shifter2[1] >> 1;
            chip->ac_shifter2[0] |= (chip->ac_shifter1[1] & 1) << 24;
            if (chip->ac_clip_l)
                chip->ac_shifter2[0] |= 1 << 22;
            if (chip->ac_clip_h)
                chip->ac_shifter2[0] &= ~(1 << 22);

            int sync = (chip->ac_sync[0] & 128) != 0 && (chip->ac_sync[1] & 128) != 0;
            chip->ac_sync2[0] = (chip->ac_sync2[1] << 1) | sync;

            int bit = 0;

            if (sync)
            {
                bit |= chip->ac_sign;
            }
            if (chip->ac_sync2[1] & 1)
            {
                bit |= (chip->ac_exp & (1 + 4 + 16 + 64)) != 0;
            }
            if (chip->ac_sync2[1] & 2)
            {
                bit |= (chip->ac_exp & (2 + 4 + 32 + 64)) != 0;
            }
            if (chip->ac_sync2[1] & 4)
            {
                bit |= (chip->ac_exp & (8 + 16 + 32 + 64)) != 0;
            }
            if (!(chip->ac_sync2[1] & 7) && !sync)
            {
                if (chip->ac_exp & 1)
                    bit |= (chip->ac_shifter2[1] & 1) != 0;
                if (chip->ac_exp & 2)
                    bit |= (chip->ac_shifter2[1] & 2) != 0;
                if (chip->ac_exp & 4)
                    bit |= (chip->ac_shifter2[1] & 4) != 0;
                if (chip->ac_exp & 8)
                    bit |= (chip->ac_shifter2[1] & 8) != 0;
                if (chip->ac_exp & 16)
                    bit |= (chip->ac_shifter2[1] & 16) != 0;
                if (chip->ac_exp & 32)
                    bit |= (chip->ac_shifter2[1] & 32) != 0;
                if (chip->ac_exp & 64)
                    bit |= (chip->ac_shifter2[1] & 64) != 0;
            }
            chip->ac_out[0] = (chip->ac_out[1] << 1) | bit;
        }
        if (chip->aclk1)
        {
            chip->ac_shifter1[1] = chip->ac_shifter1[0];
            chip->ac_shifter2[1] = chip->ac_shifter2[0];

            chip->ac_sync2[1] = chip->ac_sync2[0];
            chip->ac_out[1] = chip->ac_out[0];
        }

        if (load2)
        {
            chip->ac_hi_bits = (chip->ac_shifter2[1] >> 15) & 127;
        }
        if (chip->ac_sync[0] & 8)
        {
            chip->ac_exp = 0;
            int sign = (chip->ac_hi_bits & 64) != 0;
            int ss = chip->ac_hi_bits & 63;
            if (!sign)
                ss ^= 63;
            if (ss & 32)
                chip->ac_exp |= 64;
            if ((ss & 48) == 16)
                chip->ac_exp |= 32;
            if ((ss & 56) == 8)
                chip->ac_exp |= 16;
            if ((ss & 60) == 4)
                chip->ac_exp |= 8;
            if ((ss & 62) == 2)
                chip->ac_exp |= 4;
            if (ss == 1)
                chip->ac_exp |= 2;
            if (ss == 0)
                chip->ac_exp |= 1;

            chip->ac_sign = sign;
        }

        chip->o_opo = (chip->ac_out[1] & 2) != 0;
    }
    
    {
        if (chip->clk1)
        {
            int inc = chip->busy_cnt_en[1];
            int sum = chip->busy_cnt[1] + inc;
            int of = (sum & 16) != 0;
            if (ic)
                chip->busy_cnt[0] = 0;
            else
                chip->busy_cnt[0] = sum & 15;

            chip->busy_cnt_en[0] = write_1_en || (chip->busy_cnt_en[1] && !(of || ic));

        }
        if (chip->clk2)
        {
            chip->busy_cnt[1] = chip->busy_cnt[0];
            chip->busy_cnt_en[1] = chip->busy_cnt_en[0];
        }
    }
    {
        if (chip->clk1)
        {
            int testdata;
            chip->read_test = (chip->reg_test_21[1] & 64) != 0;
            testdata = chip->op_output[3] & 0x3fff;
            testdata |= (chip->pg_dbg[1] & 1) << 15;
            if (chip->eg_debug[1] & 0x200)
                testdata |= 1 << 14;

            if (chip->reg_test_21[1] & 128)
                chip->read_op = testdata & 255;
            else
                chip->read_op = testdata >> 8;
        }

        if (chip->read_test)
        {
            chip->read_data = chip->read_op;
        }
        if (read1)
        {
            chip->read_data = chip->data_bus1;
        }
        if (!chip->read_test && !read1)
        {
            chip->read_data = 0; // FIXME: floating bus
            if (chip->busy_cnt_en[1])
                chip->read_data |= 128;
            if (chip->timer_a_status[1])
                chip->read_data |= 1;
            if (chip->timer_b_status[1])
                chip->read_data |= 2;
        }

        if (!read0)
        {
            chip->o_data = chip->read_data;
        }
    }
}
