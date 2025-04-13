/*
 * Copyright (C) 2023 nukeykt
 *
 * This file is part of YMF262-LLE.
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
 *  YMF262 emulator
 *  Thanks:
 *      John McMaster (siliconpr0n.org):
 *          YMF262 decap and die shot
 *
 */

#include "fmopl3.h"


enum {
    eg_state_attack = 0,
    eg_state_decay,
    eg_state_sustain,
    eg_state_release
};

void FMOPL3_Clock(fmopl3_t *chip)
{
    int i;

    chip->mclk1 = !chip->input.mclk;
    chip->mclk2 = chip->input.mclk;

    chip->io_rd = !chip->input.rd;
    chip->io_wr = !chip->input.wr;
    chip->io_cs = !chip->input.cs;
    chip->io_a0 = chip->input.address & 1;
    chip->io_a1 = (chip->input.address & 2) != 0;

    if (chip->mclk1)
    {
        chip->ic_latch[0] = (chip->ic_latch[1] << 1) | (!chip->input.ic);
    }
    if (chip->mclk2)
    {
        chip->ic_latch[1] = chip->ic_latch[0];

        chip->reset0 = (chip->ic_latch[1] & 2) != 0;
    }

    chip->io_read = !chip->reset0 && chip->io_cs && chip->io_rd && !chip->io_a0 && !chip->io_a1;
    chip->io_write = !chip->reset0 && chip->io_cs && chip->io_wr;
    chip->io_write0 = !chip->reset0 && chip->io_cs && chip->io_wr && !chip->io_a0;
    chip->io_write1 = !chip->reset0 && chip->io_cs && chip->io_wr && chip->io_a0;

    if (chip->reset0)
        chip->data_latch = 0;
    else if (chip->io_write)
        chip->data_latch = chip->input.data_i & 255;
    if (chip->reset0)
        chip->bank_latch = 0;
    else if (chip->io_write0)
        chip->bank_latch = chip->io_a1;

    if (chip->mclk2)
    {
        chip->prescaler1_reset[1] = chip->prescaler1_reset[0];
        chip->prescaler1_cnt[1] = chip->prescaler1_cnt[0];
    }

    int prescaler1_clk = chip->input.mclk; /* Temp: disable prescaler for performance reasons */

    chip->aclk1 = !prescaler1_clk;
    chip->aclk2 = prescaler1_clk;

    if (chip->aclk2)
    {
        chip->prescaler2_reset_l[1] = chip->prescaler2_reset_l[0];
        chip->prescaler2_cnt[1] = chip->prescaler2_cnt[0];
        chip->prescaler2_l1[1] = chip->prescaler2_l1[0];
        chip->prescaler2_l3[1] = chip->prescaler2_l3[0];
        chip->prescaler2_l5[1] = chip->prescaler2_l5[0];
        chip->prescaler2_l6[1] = chip->prescaler2_l6[0];
    }


    chip->clk1 = chip->prescaler2_l1[1] && !chip->prescaler2_l2;
    chip->clk2 = chip->prescaler2_l3[1] && !chip->prescaler2_l4;

    chip->rclk1 = chip->prescaler2_l6[1];
    chip->rclk2 = chip->prescaler2_l5[1];

    if (chip->aclk1)
    {

        int ga = (chip->data_latch & 0xe0) != 0;

        int write0 = ga && chip->write0 && (chip->reg_test1 & 16) != 0;
        int write = chip->write1 || write0;

            chip->ra_w1_l1 = write;
    }

    if (chip->clk2)
    {
        chip->write0_l[1] = chip->write0_l[0];
        chip->write0_l[3] = chip->write0_l[2];

        chip->write1_l[1] = chip->write1_l[0];
        chip->write1_l[3] = chip->write1_l[2];

        chip->write0 = chip->write0_l[3] && !chip->write0_l[1];
        chip->write1 = chip->write1_l[3] && !chip->write1_l[1];
    }

    //////////////////////

    //if (chip->o_clk1 == chip->clk1 && chip->o_clk2 == chip->clk2 && chip->o_rclk1 == chip->rclk1 && chip->o_rclk2 == chip->rclk2 && chip->o_reset0 == chip->reset0
    //    && chip->o_ra_w1_l1 == chip->ra_w1_l1 && chip->o_bank_latch == chip->bank_latch && chip->o_data_latch == chip->data_latch)
    //    goto end; // opt

    chip->o_clk1 = chip->clk1;
    chip->o_clk2 = chip->clk2;
    chip->o_rclk1 = chip->rclk1;
    chip->o_rclk2 = chip->rclk2;
    chip->o_reset0 = chip->reset0;
    chip->o_data_latch = chip->data_latch;
    chip->o_bank_latch = chip->bank_latch;
    chip->o_ra_w1_l1 = chip->ra_w1_l1;

    if (chip->reset0)
    {
        chip->reg_sel1 = 0;
        chip->reg_sel2 = 0;
        chip->reg_sel3 = 0;
        chip->reg_sel4 = 0;
        chip->reg_sel5 = 0;
        chip->reg_sel8 = 0;
        chip->reg_selbd = 0;
    }
    else if (chip->write0)
    {
        chip->reg_sel1 = chip->data_latch == 1;
        chip->reg_sel2 = chip->data_latch == 2;
        chip->reg_sel3 = chip->data_latch == 3;
        chip->reg_sel4 = chip->data_latch == 4;
        chip->reg_sel5 = chip->data_latch == 5;
        chip->reg_sel8 = chip->data_latch == 8;
        chip->reg_selbd = chip->data_latch == 0xbd;
    }

    if (chip->reset0)
        chip->reg_new = 0;
    else if (chip->write1 && chip->bank_latch && chip->reg_sel5)
        chip->reg_new = chip->data_latch & 1;

    int bank_masked = chip->reg_new && chip->bank_latch;

    if (chip->reset0)
    {
        chip->reg_test0 = 0;
        chip->reg_test1 = 0;
        chip->reg_timer1 = 0;
        chip->reg_timer2 = 0;
        chip->reg_notesel = 0;
        chip->rhythm = 0;
        chip->reg_rh_kon = 0;
        chip->reg_da = 0;
        chip->reg_dv = 0;
    }
    else if (chip->write1)
    {
        if (chip->reg_sel1 && !bank_masked)
            chip->reg_test0 = chip->data_latch & 255;
        if (chip->reg_sel2 && !bank_masked)
            chip->reg_timer1 = chip->data_latch & 255;
        if (chip->reg_sel3 && !bank_masked)
            chip->reg_timer2 = chip->data_latch & 255;
        if (chip->reg_sel8 && !bank_masked)
        {
            chip->reg_notesel = (chip->data_latch & 64) != 0;
        }
        if (chip->reg_selbd && !bank_masked)
        {
            chip->reg_rh_kon = chip->data_latch & 31;
            chip->rhythm = (chip->data_latch & 32) != 0;
            chip->reg_dv = (chip->data_latch & 64) != 0;
            chip->reg_da = (chip->data_latch & 128) != 0;
        }
        if (chip->reg_sel1 && bank_masked)
            chip->reg_test1 = chip->data_latch & 255;
        if (chip->reg_sel4 && bank_masked)
            chip->reg_4op = chip->data_latch & 63;
    }
    int reg_sel4_wr = chip->write1 && chip->reg_sel4 && !bank_masked && (chip->data_latch & 128) == 0;
    int reg_sel4_rst = (chip->write1 && chip->reg_sel4 && !bank_masked && (chip->data_latch & 128) != 0) || chip->reset0;

    if (chip->reset0)
    {
        chip->reg_t1_mask = 0;
        chip->reg_t2_mask = 0;
        chip->reg_t1_start = 0;
        chip->reg_t2_start = 0;
    }
    else if (reg_sel4_wr)
    {
        chip->reg_t1_mask = (chip->data_latch & 64) != 0;
        chip->reg_t2_mask = (chip->data_latch & 32) != 0;
        chip->reg_t1_start = (chip->data_latch & 1) != 0;
        chip->reg_t2_start = (chip->data_latch & 2) != 0;
    }

    chip->reset1 = chip->reset0 || (chip->reg_test1 & 0xc0) == 0xc0;

    {
        //int bclk = !prescaler2_reset && chip->prescaler2_l7 && (chip->prescaler2_cnt[1] & 1) == 0;

        int ga = (chip->data_latch & 0xe0) != 0;

        if (chip->reset1)
            chip->ra_address_latch = 0;
        else if (chip->write0 && ga)
            chip->ra_address_latch = (bank_masked << 8) | chip->data_latch;
        if (chip->reset1)
            chip->ra_address_good = 0;
        else if (chip->write0)
            chip->ra_address_good = ga;
        if (chip->reset1)
            chip->ra_data_latch = 0;
        else if (chip->write1 && chip->ra_address_good)
            chip->ra_data_latch = chip->data_latch;

        int write0 = ga && chip->write0 && (chip->reg_test1 & 16) != 0;
        int write = chip->write1 || write0;

        if (chip->aclk1)
            chip->ra_w1_l1 = write;
        chip->ra_write = (write && !chip->ra_w1_l1) || (chip->reset1 && chip->clk2);
        if (chip->clk1)
            chip->ra_w1_l2 = write;
        chip->ra_write_a = write && !chip->ra_w1_l2;

        if (chip->clk1)
        {
            chip->ra_rst_l[0] = chip->reset1;
            int rst = (chip->reset1 && !chip->ra_rst_l[1]) || chip->fsm_out[5];

            int of1 = (chip->ra_cnt1[1] & 5) == 5;
            int of2 = (chip->ra_cnt2[1] & 2) == 2 && of1;
            int of4 = (chip->ra_cnt4[1] & 2) == 2;
            if (rst || of1)
                chip->ra_cnt1[0] = 0;
            else
                chip->ra_cnt1[0] = (chip->ra_cnt1[1] + 1) & 7;
            if (rst || of2)
                chip->ra_cnt2[0] = 0;
            else
                chip->ra_cnt2[0] = (chip->ra_cnt2[1] + of1) & 3;
            if (rst)
                chip->ra_cnt3[0] = 0;
            else
                chip->ra_cnt3[0] = (chip->ra_cnt3[1] + of2) & 1;

            if (rst || of4 || of1)
                chip->ra_cnt4[0] = 0;
            else
                chip->ra_cnt4[0] = (chip->ra_cnt4[1] + 1) & 3;

        }
        if (chip->clk2)
        {
            chip->ra_rst_l[1] = chip->ra_rst_l[0];
            chip->ra_cnt1[1] = chip->ra_cnt1[0];
            chip->ra_cnt2[1] = chip->ra_cnt2[0];
            chip->ra_cnt3[1] = chip->ra_cnt3[0];
            chip->ra_cnt4[1] = chip->ra_cnt4[0];
            chip->ra_cnt = (chip->ra_cnt3[1] << 5) | (chip->ra_cnt2[1] << 3) | chip->ra_cnt1[1];
        }

        if (chip->ra_write || chip->clk1)
        {
            static const int ch_map[32] = {
                0, 1, 2, -1,
                3, 4, 5, -1,
                6, 7, 8, -1,
                -1, -1, -1, -1,
                9, 10, 11, -1,
                12, 13, 14, -1,
                15, 16, 17, -1,
                -1, -1, -1, -1
            };

            static const int op_map[64] = {
                0, 1, 2, 3, 4, 5, -1, -1,
                6, 7, 8, 9, 10, 11, -1, -1,
                12, 13, 14, 15, 16, 17, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1,
                18, 19, 20, 21, 22, 23, -1, -1,
                24, 25, 26, 27, 28, 29, -1, -1,
                30, 31, 32, 33, 34, 35, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1
            };

            int bank = (chip->ra_address_latch & 0x100) != 0;
            int op_address = chip->ra_write_a ? ((chip->ra_address_latch & 0x1f) | (bank << 5)) : chip->ra_cnt;
            int idx = op_map[op_address];
            if (chip->ra_write && idx != -1)
            {
                if ((chip->ra_address_latch & 0xe0) == 0x20 || write0 || chip->reset1)
                {
                    chip->ra_multi[idx] = chip->ra_data_latch & 15;
                    chip->ra_ksr[idx] = (chip->ra_data_latch >> 4) & 1;
                    chip->ra_egt[idx] = (chip->ra_data_latch >> 5) & 1;
                    chip->ra_vib[idx] = (chip->ra_data_latch >> 6) & 1;
                    chip->ra_am[idx] = (chip->ra_data_latch >> 7) & 1;
                }
                if ((chip->ra_address_latch & 0xe0) == 0x40 || write0 || chip->reset1)
                {
                    chip->ra_tl[idx] = chip->ra_data_latch & 63;
                    chip->ra_ksl[idx] = (chip->ra_data_latch >> 6) & 3;
                }
                if ((chip->ra_address_latch & 0xe0) == 0x60 || write0 || chip->reset1)
                {
                    chip->ra_dr[idx] = chip->ra_data_latch & 15;
                    chip->ra_ar[idx] = (chip->ra_data_latch >> 4) & 15;
                }
                if ((chip->ra_address_latch & 0xe0) == 0x80 || write0 || chip->reset1)
                {
                    chip->ra_rr[idx] = chip->ra_data_latch & 15;
                    chip->ra_sl[idx] = (chip->ra_data_latch >> 4) & 15;
                }
                if ((chip->ra_address_latch & 0xe0) == 0xe0 || write0 || chip->reset1)
                {
                    int data = chip->ra_data_latch & 3;
                    if (chip->reg_new)
                        data |= chip->ra_data_latch & 4;
                    chip->ra_wf[idx] = data;
                }
            }
            int ch_address_write = chip->ra_address_latch & 15;
            int add = 0;
            if (ch_address_write == 3 || ch_address_write == 4 || ch_address_write == 5)
                add |= 1;
            if (ch_address_write == 6 || ch_address_write == 7 || ch_address_write == 8)
                add |= 2;
            int ch_address_mapped = (ch_address_write & 1) + (add & 1);
            ch_address_mapped |= add & 2;
            ch_address_mapped += ch_address_write & 14;
            ch_address_mapped &= 15;
            ch_address_mapped |= bank << 4;
            int ch_address_mapped2 = ch_address_mapped & 3;
            if ((ch_address_mapped & 12) == 8)
                ch_address_mapped2 |= 4;
            if ((ch_address_mapped & 12) == 0)
                ch_address_mapped2 |= 8;
            if ((ch_address_mapped & 28) == 0 || (ch_address_mapped & 28) == 20 || (ch_address_mapped & 28) == 24)
                ch_address_mapped2 |= 16;


            int ch_address_read = (chip->ra_cnt4[1] & 3) | (chip->ra_cnt2[1] << 2) | (chip->ra_cnt3[1] << 4);
            int ch_address = chip->ra_write_a ? ch_address_mapped : ch_address_read;
            int ch_address_read_4op = ch_address_read;
            if ((chip->ra_cnt2[1] & 2) == 0)
            {
                switch (chip->ra_cnt3[1] * 4 + chip->ra_cnt4[1])
                {
                    case 0: // 0, 3, 6, 9
                        if (chip->reg_4op & 1)
                            ch_address_read_4op &= ~4;
                        break;
                    case 1: // 1, 4, 7, 10
                        if (chip->reg_4op & 2)
                            ch_address_read_4op &= ~4;
                        break;
                    case 2: // 2, 5, 8, 11
                        if (chip->reg_4op & 4)
                            ch_address_read_4op &= ~4;
                        break;
                    case 4: // 0, 3, 6, 9
                        if (chip->reg_4op & 8)
                            ch_address_read_4op &= ~4;
                        break;
                    case 5: // 1, 4, 7, 10
                        if (chip->reg_4op & 16)
                            ch_address_read_4op &= ~4;
                        break;
                    case 6: // 2, 5, 8, 11
                        if (chip->reg_4op & 32)
                            ch_address_read_4op &= ~4;
                        break;
                }
            }
            int ch_address_4op = chip->ra_write_a ? ch_address_mapped : ch_address_read_4op;
            int ch_address_fb = chip->ra_write_a ? ch_address_mapped2 : ch_address_read;
            
            int idx1 = ch_map[ch_address];
            int idx2 = ch_map[ch_address_4op];
            int idx3 = ch_map[ch_address_fb];
            if (chip->ra_write && idx1 != -1)
            {
                if ((chip->ra_address_latch & 0xf0) == 0xc0 || write0 || chip->reset1)
                {
                    chip->ra_connect[idx1] = chip->ra_data_latch & 1;
                    int pan_data = 0;
                    if (!chip->reg_new || chip->reset1)
                        pan_data |= 3;
                    if (chip->reg_new)
                        pan_data |= (chip->ra_data_latch >> 4) & 15;
                    chip->ra_pan[idx1] = pan_data;
                }
            }
            if (chip->ra_write && idx2 != -1)
            {
                if ((chip->ra_address_latch & 0xf0) == 0xa0 || write0 || chip->reset1)
                {
                    chip->ra_fnum[idx2] &= 0x300;
                    chip->ra_fnum[idx2] |= chip->ra_data_latch & 0xff;
                }
                if ((chip->ra_address_latch & 0xf0) == 0xb0 || write0 || chip->reset1)
                {
                    chip->ra_fnum[idx2] &= 0xff;
                    chip->ra_fnum[idx2] |= (chip->ra_data_latch & 3) << 8;
                    chip->ra_block[idx2] = (chip->ra_data_latch >> 2) & 7;
                    chip->ra_keyon[idx2] = (chip->ra_data_latch >> 5) & 1;
                }
            }
            if (chip->ra_write && idx3 != -1)
            {
                if ((chip->ra_address_latch & 0xf0) == 0xc0 || write0 || chip->reset1)
                {
                    chip->ra_connect_pair[idx3] = chip->ra_data_latch & 1;
                    chip->ra_fb[idx3] = (chip->ra_data_latch >> 1) & 7;
                }
            }

            if (chip->clk1)
            {
                if (idx != -1)
                {
                    chip->multi[0] = chip->ra_multi[idx];
                    chip->ksr[0] = chip->ra_ksr[idx];
                    chip->egt[0] = chip->ra_egt[idx];
                    chip->am[0] = chip->ra_am[idx];
                    chip->vib[0] = chip->ra_vib[idx];
                    chip->tl[0] = chip->ra_tl[idx];
                    chip->ksl[0] = chip->ra_ksl[idx];
                    chip->ar[0] = chip->ra_ar[idx];
                    chip->dr[0] = chip->ra_dr[idx];
                    chip->sl[0] = chip->ra_sl[idx];
                    chip->rr[0] = chip->ra_rr[idx];
                    chip->wf[0] = chip->ra_wf[idx];
                }
                if (idx1 != -1)
                {
                    chip->connect[0] = chip->ra_connect[idx1];
                    chip->pan[0] = chip->ra_pan[idx1];
                }
                if (idx2 != -1)
                {
                    chip->fnum[0] = chip->ra_fnum[idx2];
                    chip->block[0] = chip->ra_block[idx2];
                    chip->keyon[0] = chip->ra_keyon[idx2];
                }
                if (idx3 != -1)
                {
                    chip->connect_pair[0] = chip->ra_connect_pair[idx3];
                    chip->fb[0] = chip->ra_fb[idx3];
                }
            }
        }
        if (chip->clk2)
        {
            chip->multi[1] = chip->multi[0];
            chip->ksr[1] = chip->ksr[0];
            chip->egt[1] = chip->egt[0];
            chip->am[1] = chip->am[0];
            chip->vib[1] = chip->vib[0];
            chip->tl[1] = chip->tl[0];
            chip->ksl[1] = chip->ksl[0];
            chip->ar[1] = chip->ar[0];
            chip->dr[1] = chip->dr[0];
            chip->sl[1] = chip->sl[0];
            chip->rr[1] = chip->rr[0];
            chip->wf[1] = chip->wf[0];

            chip->connect[1] = chip->connect[0];
            chip->pan[1] = chip->pan[0];

            chip->fnum[1] = chip->fnum[0];
            chip->block[1] = chip->block[0];
            chip->keyon[1] = chip->keyon[0];

            chip->connect_pair[1] = chip->connect_pair[0];
            chip->fb[1] = chip->fb[0];
        }
    }

    if (chip->clk1)
    {
        chip->connect_l[0] = (chip->connect_l[1] << 1) | chip->connect[1];
        chip->connect_pair_l[0] = (chip->connect_pair_l[1] << 1) | chip->connect_pair[1];
        chip->fb_l[0][0] = chip->fb[1];
        chip->fb_l[1][0] = chip->fb_l[0][1];
        chip->pan_l[0][0] = chip->pan[1];
        chip->pan_l[1][0] = chip->pan_l[0][1];
    }
    if (chip->clk2)
    {
        chip->connect_l[1] = chip->connect_l[0];
        chip->connect_pair_l[1] = chip->connect_pair_l[0];
        chip->fb_l[0][1] = chip->fb_l[0][0];
        chip->fb_l[1][1] = chip->fb_l[1][0];
        chip->pan_l[0][1] = chip->pan_l[0][0];
        chip->pan_l[1][1] = chip->pan_l[1][0];
    }
    
    {
        int fsm_4op = 0;
        switch (chip->fsm_cnt)
        {
        case 5: // 5
            fsm_4op = (chip->reg_4op & 1) != 0;
            break;
        case 8: // 6
            fsm_4op = (chip->reg_4op & 2) != 0;
            break;
        case 9: // 7
            fsm_4op = (chip->reg_4op & 4) != 0;
            break;
        case 37: // 23
            fsm_4op = (chip->reg_4op & 8) != 0;
            break;
        case 40: // 24
            fsm_4op = (chip->reg_4op & 16) != 0;
            break;
        case 41: // 25
            fsm_4op = (chip->reg_4op & 32) != 0;
            break;
        }
        int con_4op = fsm_4op && (chip->fsm_l10[1] & 4) != 0; // 01 connect

        if (chip->clk1)
        {
            int fsm_reset = (chip->fsm_reset_l[1] & 2) == 0 && chip->reset1;
            chip->fsm_reset_l[0] = (chip->fsm_reset_l[1] << 1) | chip->reset1;

            int fsm_of1 = (chip->fsm_cnt1[1] & 5) == 5;
            int fsm_of2 = (chip->fsm_cnt2[1] & 2) == 2 && fsm_of1;

            if (fsm_reset || fsm_of1)
                chip->fsm_cnt1[0] = 0;
            else
                chip->fsm_cnt1[0] = (chip->fsm_cnt1[1] + 1) & 7;

            if (fsm_reset || fsm_of2)
                chip->fsm_cnt2[0] = 0;
            else
                chip->fsm_cnt2[0] = (chip->fsm_cnt2[1] + fsm_of1) & 3;

            if (fsm_reset)
                chip->fsm_cnt3[0] = 0;
            else
                chip->fsm_cnt3[0] = (chip->fsm_cnt3[1] + fsm_of2) & 1;

            chip->fsm_l1[0] = chip->fsm_cnt == 53;
            chip->fsm_l2[0] = chip->fsm_cnt == 16;
            chip->fsm_l3[0] = chip->fsm_cnt == 20;
            chip->fsm_l4[0] = chip->fsm_cnt == 52;
            chip->fsm_l5[0] = (chip->fsm_l5[1] << 1) | ((chip->fsm_cnt & 56) == 0);
            chip->fsm_l6[0] = (chip->fsm_l6[1] << 1) | ((chip->fsm_cnt & 56) == 8 || (chip->fsm_cnt & 62) == 16);
            chip->fsm_l7[0] = (chip->fsm_l7[1] << 1) | ((chip->fsm_cnt & 56) == 40 || (chip->fsm_cnt & 62) == 48);
            chip->fsm_l8[0] = (chip->fsm_l8[1] << 1) | ((chip->fsm_cnt & 48) == 16);
            chip->fsm_l9[0] = (chip->fsm_l9[1] << 1) | con_4op;
            chip->fsm_l10[0] = (chip->fsm_l10[1] << 1) | ((chip->connect_l[1] & 2) == 0 && (chip->connect_pair_l[1] & 2) != 0);
        }
        if (chip->clk2)
        {
            chip->fsm_reset_l[1] = chip->fsm_reset_l[0];
            chip->fsm_cnt1[1] = chip->fsm_cnt1[0];
            chip->fsm_cnt2[1] = chip->fsm_cnt2[0];
            chip->fsm_cnt3[1] = chip->fsm_cnt3[0];

            chip->fsm_cnt = (chip->fsm_cnt3[1] << 5) | (chip->fsm_cnt2[1] << 3) | chip->fsm_cnt1[1];

            chip->fsm_l1[1] = chip->fsm_l1[0];
            chip->fsm_l2[1] = chip->fsm_l2[0];
            chip->fsm_l3[1] = chip->fsm_l3[0];
            chip->fsm_l4[1] = chip->fsm_l4[0];
            chip->fsm_l5[1] = chip->fsm_l5[0];
            chip->fsm_l6[1] = chip->fsm_l6[0];
            chip->fsm_l7[1] = chip->fsm_l7[0];
            chip->fsm_l8[1] = chip->fsm_l8[0];
            chip->fsm_l9[1] = chip->fsm_l9[0];
            chip->fsm_l10[1] = chip->fsm_l10[0];
        }
        {
            chip->fsm_out[0] = chip->fsm_l1[1]; // 0
            chip->fsm_out[1] = chip->fsm_cnt == 16; // 12
            chip->fsm_out[2] = chip->fsm_l2[1]; // 13
            chip->fsm_out[3] = chip->fsm_cnt == 20; // 16
            chip->fsm_out[4] = chip->fsm_l3[1]; // 17
            chip->fsm_out[5] = chip->fsm_cnt == 52; // 34
            chip->fsm_out[6] = chip->fsm_l4[1]; // 35
            chip->fsm_out[7] = (chip->fsm_l5[1] & 4) != 0 || ((chip->fsm_cnt & 56) == 0); // 0-8
            chip->fsm_out[8] = (chip->fsm_cnt & 32) == 0;
            chip->fsm_out[9] = (chip->fsm_l6[1] & 2) != 0;
            chip->fsm_out[10] = (chip->fsm_l7[1] & 2) != 0;
            chip->fsm_out[11] = chip->rhythm && (chip->fsm_l8[1] & 2) != 0; // r 14, 15, 16, 17, 18, 19

            int fsm_mc = !((chip->fsm_cnt & 5) == 4 || (chip->fsm_cnt & 2) != 0);
            int fsm_mc_4op = fsm_mc && !fsm_4op;
            int rhy_19_20 = chip->rhythm && (chip->fsm_cnt == 19 || chip->fsm_cnt == 20);

            chip->fsm_out[12] = fsm_mc_4op && !(chip->rhythm && (chip->fsm_cnt == 16 || chip->fsm_cnt == 17)); // feedback
            chip->fsm_out[14] = con_4op || (!fsm_4op && !(chip->fsm_l9[1] & 4) && (chip->connect_l[1] & 2) != 0); // connect
            chip->fsm_out[13] = !(chip->rhythm && chip->fsm_cnt == 18) && (fsm_mc_4op || rhy_19_20 || chip->fsm_out[14]); // output
            chip->fsm_out[15] = !fsm_mc && !rhy_19_20; // load fb
            chip->fsm_out[16] = !fsm_mc_4op && !rhy_19_20; // modulate
        }
    }

    if (chip->clk1)
        chip->timer_st_load_l = chip->fsm_out[6];
    chip->timer_st_load = chip->fsm_out[6] && !chip->timer_st_load_l;

    if (chip->timer_st_load)
    {
        chip->t1_start = chip->reg_t1_start;
        chip->t2_start = chip->reg_t2_start;
    }

    if (chip->clk1)
    {
        int lfo = chip->lfo_cnt[1];
        int add = chip->fsm_out[6];
        int reset = (chip->reg_test0 & 2) != 0 || chip->reset1;

        chip->lfo_cnt[0] = reset ? 0 : (lfo + add) & 1023;
        chip->vib_cnt[0] = reset ? 0 : (chip->vib_cnt[1] + chip->vib_step) & 7;
    }
    if (chip->clk2)
    {
        chip->lfo_cnt[1] = chip->lfo_cnt[0];
        chip->vib_cnt[1] = chip->vib_cnt[0];
    }

    {
        int lfo = chip->lfo_cnt[1];
        int add = chip->fsm_out[6];

        chip->t1_step = (((lfo & 3) + add) & 4) != 0;
        chip->t2_step = (((lfo & 15) + add) & 16) != 0;
        chip->am_step = (((lfo & 63) + add) & 64) != 0;
        chip->vib_step = (((lfo & 1023) + add) & 1024) != 0;
        chip->vib_step |= (chip->reg_test0 & 16) != 0 && add;
    }

    if (chip->clk1)
    {
        int value = (chip->t1_of[1] || (chip->t1_start_l[1] & 3) == 1) ? chip->reg_timer1 : chip->t1_cnt[1];
        value += ((chip->t1_start_l[1] & 1) != 0 && chip->t1_step) || (chip->reg_test1 & 8) != 0;
        chip->t1_of[0] = (value & 256) != 0;
        chip->t1_cnt[0] = (chip->t1_start_l[1] & 1) == 0 ? 0 : (value & 255);
        
        value = (chip->t2_of[1] || (chip->t2_start_l[1] & 3) == 1) ? chip->reg_timer2 : chip->t2_cnt[1];
        value += ((chip->t2_start_l[1] & 1) != 0 && chip->t2_step) || (chip->reg_test1 & 8) != 0;
        chip->t2_of[0] = (value & 256) != 0;
        chip->t2_cnt[0] = (chip->t2_start_l[1] & 1) == 0 ? 0 : (value & 255);

        chip->t1_start_l[0] = (chip->t1_start_l[1] << 1) | chip->t1_start;
        chip->t2_start_l[0] = (chip->t2_start_l[1] << 1) | chip->t2_start;
    }
    if (chip->clk2)
    {
        chip->t1_cnt[1] = chip->t1_cnt[0];
        chip->t1_of[1] = chip->t1_of[0];
        chip->t2_cnt[1] = chip->t2_cnt[0];
        chip->t2_of[1] = chip->t2_of[0];

        chip->t1_start_l[1] = chip->t1_start_l[0];
        chip->t2_start_l[1] = chip->t2_start_l[0];
    }

    if (reg_sel4_rst || chip->reg_t1_mask)
        chip->t1_status = 0;
    else if (chip->t1_of[1])
        chip->t1_status = 1;

    if (reg_sel4_rst || chip->reg_t2_mask)
        chip->t2_status = 0;
    else if (chip->t2_of[1])
        chip->t2_status = 1;

    chip->rh_sel0 = chip->rhythm && chip->fsm_out[1];

    if (chip->clk1)
    {
        chip->rh_sel[0] = (chip->rh_sel[1] << 1) | chip->rh_sel0;
    }
    if (chip->clk2)
    {
        chip->rh_sel[1] = chip->rh_sel[0];
    }
    
    chip->keyon_comb = chip->keyon[1]
        || (chip->rh_sel0 && (chip->reg_rh_kon & 16) != 0) // bd0
        || ((chip->rh_sel[1] & 1) != 0 && (chip->reg_rh_kon & 1) != 0) // hh
        || ((chip->rh_sel[1] & 2) != 0 && (chip->reg_rh_kon & 4) != 0) // tom
        || ((chip->rh_sel[1] & 4) != 0 && (chip->reg_rh_kon & 16) != 0) // bd1
        || ((chip->rh_sel[1] & 8) != 0 && (chip->reg_rh_kon & 8) != 0) // sd
        || ((chip->rh_sel[1] & 16) != 0 && (chip->reg_rh_kon & 2) != 0); // tc


    if (chip->clk1)
    {
        chip->trem_load_l = chip->fsm_out[0];
        chip->trem_st_load_l = chip->fsm_out[6];
        chip->eg_load_l = chip->eg_load_l1[1];
    }
    chip->trem_load = !chip->trem_load_l && chip->fsm_out[0];
    chip->trem_st_load = !chip->trem_st_load_l && chip->fsm_out[6];
    chip->eg_load = !chip->eg_load_l && chip->eg_load_l1[1];

    {
        if (chip->trem_st_load)
            chip->trem_step = chip->am_step;
        if (chip->trem_load)
            chip->trem_out = chip->trem_value[1] & 127;

        if (chip->clk1)
        {
            int bit = chip->trem_value[1] & 1;
            int reset = chip->reset1 || (chip->reg_test0 & 2) != 0;

            int step = ((chip->trem_step || (chip->reg_test0 & 16) != 0) && (chip->fsm_out[0] || chip->trem_dir[1]))
                && chip->fsm_out[7];
            int carry = chip->fsm_out[7] && chip->trem_carry[1];

            bit += step + carry;

            int of = (chip->trem_out == 0) || (chip->trem_out & 105) == 105;

            chip->trem_carry[0] = (bit & 2) != 0;
            chip->trem_value[0] = (chip->trem_value[1] >> 1) & 255;
            if (!reset)
                chip->trem_value[0] |= (bit & 1) << 8;
            chip->trem_of[0] = of;

            if (reset)
                chip->trem_dir[0] = 0;
            else
                chip->trem_dir[0] = chip->trem_dir[1] ^ (of && !chip->trem_of[1]);
        }
        if (chip->clk2)
        {
            chip->trem_carry[1] = chip->trem_carry[0];
            chip->trem_value[1] = chip->trem_value[0];
            chip->trem_of[1] = chip->trem_of[0];
            chip->trem_dir[1] = chip->trem_dir[0];
        }
    }

    {

        if (chip->reset1)
        {
            chip->eg_timer_low = 0;
            chip->eg_shift = 0;
        }
        else if (chip->eg_load)
        {
            chip->eg_timer_low = chip->eg_timer_o[3] | (chip->eg_timer_o[1] << 1);
            chip->eg_shift = 0;
            if (chip->eg_timer_masked[1] & 0x1555)
                chip->eg_shift |= 1;
            if (chip->eg_timer_masked[1] & 0x666)
                chip->eg_shift |= 2;
            if (chip->eg_timer_masked[1] & 0x1878)
                chip->eg_shift |= 4;
            if (chip->eg_timer_masked[1] & 0x1f80)
                chip->eg_shift |= 8;
        }

        if (chip->clk1)
        {
            int bit = chip->eg_timer_o[3];
            int bit2;
            int carry = chip->eg_carry[1] || (chip->eg_subcnt[1] && chip->eg_sync_l[1]);
            bit += carry;

            int rst = chip->reset1 || (chip->reg_test1 & 8) != 0;

            if (rst)
                bit2 = 0;
            else
                bit2 = bit & 1;

            chip->eg_timer_i = bit2;
            chip->eg_carry[0] = (bit & 2) != 0;
            chip->eg_sync_l[0] = chip->fsm_out[6];
            chip->eg_mask[0] = (rst || chip->fsm_out[6]) ? 0 :
                (chip->eg_mask[1] || bit2);
            chip->eg_timer_masked[0] = (chip->eg_timer_masked[1] >> 1) & 0x7ffffffffLL;
            if (!chip->eg_mask[1])
                chip->eg_timer_masked[0] |= ((int64_t)bit2) << 35;
            if (!chip->eg_timer_dbg[1] && (chip->reg_test0 & 64) != 0)
                chip->eg_timer_masked[0] |= 1LL << 35;

            if (chip->reset1)
                chip->eg_subcnt[0] = 0;
            else
                chip->eg_subcnt[0] = chip->eg_subcnt[1] ^ chip->fsm_out[6];

            chip->eg_load_l1[0] = chip->eg_subcnt[1] && chip->fsm_out[6];

            chip->eg_timer_dbg[0] = (chip->reg_test0 & 64) != 0;
        }
        if (chip->clk2)
        {
            chip->eg_carry[1] = chip->eg_carry[0];
            chip->eg_sync_l[1] = chip->eg_sync_l[0];
            chip->eg_mask[1] = chip->eg_mask[0];
            chip->eg_timer_masked[1] = chip->eg_timer_masked[0];
            chip->eg_subcnt[1] = chip->eg_subcnt[0];
            chip->eg_load_l1[1] = chip->eg_load_l1[0];
            chip->eg_timer_dbg[1] = chip->eg_timer_dbg[0];
        }
    }

    if (chip->clk1)
    {
        static const int eg_stephi[4][4] = {
            { 0, 0, 0, 0 },
            { 1, 0, 0, 0 },
            { 1, 0, 1, 0 },
            { 1, 1, 1, 0 }
        };

        int rst = chip->reset1 || (chip->reg_test1 & 32) != 0;

        int state = chip->eg_state_o[3];
        int dokon = state == eg_state_release && chip->keyon_comb;
        int rate_sel = dokon ? eg_state_attack : state;
        int rate = 0;
        int ksr;
        if (rate_sel == 0)
            rate |= chip->ar[1];
        if (rate_sel == 1)
            rate |= chip->dr[1];
        if (rate_sel == 3 || (rate_sel == 2 && !chip->egt[1]))
            rate |= chip->rr[1];

        int sl = chip->sl[1];
        if (chip->sl[1] == 15)
            sl |= 16;

        int ns = chip->reg_notesel ? (chip->fnum[1] & 256) != 0 : (chip->fnum[1] & 512) != 0;

        if (chip->ksr[1])
            ksr = (chip->block[1] << 1) | ns;
        else
            ksr = chip->block[1] >> 1;

        int rate_hi = rate + (ksr >> 2);
        if (rate_hi & 16)
            rate_hi = 15;

        int maxrate = rate_hi == 15;

        int rate12 = rate_hi == 12;
        int rate13 = rate_hi == 13;
        int rate14 = rate_hi == 14;

        int inclow = 0;

        if (rate_hi < 12 && rate != 0 && chip->eg_subcnt[1])
        {
            int sum = (rate_hi + chip->eg_shift) & 15;
            switch (sum)
            {
                case 12:
                    inclow = 1;
                    break;
                case 13:
                    inclow = (ksr & 2) != 0;
                    break;
                case 14:
                    inclow = (ksr & 1) != 0;
                    break;
            }
        }

        int stephi = eg_stephi[ksr & 3][chip->eg_timer_low];

        int step1 = 0;
        int step2 = 0;
        int step3 = 0;

        switch (rate_hi)
        {
            case 12:
                step1 = stephi || chip->eg_subcnt[1];
                break;
            case 13:
                if (stephi)
                    step2 = 1;
                else
                    step1 = 1;
                break;
            case 14:
                if (stephi)
                    step3 = 1;
                else
                    step2 = 1;
                break;
            case 15:
                step3 = 1;
                break;
        }

        step1 |= inclow;

        int level = chip->eg_level_o[3];
        int slreach = (level >> 4) == sl;
        int zeroreach = level == 0;
        int silent = (level & 0x1f8) == 0x1f8;

        static const int eg_ksltable[16] = {
            0, 32, 40, 45, 48, 51, 53, 55, 56, 58, 59, 60, 61, 62, 63, 64
        };

        int nextstate = eg_state_attack;

        if (rst)
            nextstate = eg_state_release;
        else if (dokon)
            nextstate = eg_state_attack;
        else
        {
            if (!chip->keyon_comb)
                nextstate = eg_state_release;
            else if (state == eg_state_attack)
                nextstate = zeroreach ? eg_state_decay : eg_state_attack;
            else if (state == eg_state_decay)
                nextstate = slreach ? eg_state_sustain : eg_state_decay;
            else if (state == eg_state_sustain)
                nextstate = eg_state_sustain;
            else if (state == eg_state_release)
                nextstate = eg_state_release;
        }

        int linear = !dokon && !silent && ((state & 2) != 0 || (state == eg_state_decay && !slreach));
        int exponent = state == eg_state_attack && chip->keyon_comb && !maxrate && !zeroreach;
        int instantattack = (dokon && maxrate) || (chip->reg_test0 & 16) != 0;
        int mute = rst || (state != eg_state_attack && silent && !dokon && !(chip->reg_test0 & 16));

        int level2 = mute ? 0x1ff : (instantattack ? 0 : level);

        int add = 0;
        int addshift = 0;

        if (exponent)
            add |= (level >> 1) ^ 0xff;
        if (linear)
            add |= 4;

        if (exponent && (step1 || step2 || step3))
            addshift |= 256;

        if (step1)
            addshift |= (add >> 2) | (exponent << 6) | (exponent << 7) | linear;
        if (step2)
            addshift |= (add >> 1) | (exponent << 7) | (linear << 1);
        if (step3)
            addshift |= (add >> 0) | (linear << 2);

        int levelnext = level2 + addshift;

        int ksl;
        ksl = eg_ksltable[chip->fnum[1] >> 6];
        int ksl_hi = (ksl & 64) != 0;

        ksl = (ksl & 63) + (chip->block[1] << 3);
        if (ksl_hi || (ksl & 64) != 0)
            ksl = ksl & 63;
        else
            ksl = 0;

        static int eg_kslshift[4] = {
            31, 1, 2, 0
        };

        ksl = (ksl << 2) >> eg_kslshift[chip->ksl[1]];

        int ksltl = ksl + (chip->tl[1] << 2);

        int tremolo;

        if (!chip->am[1])
            tremolo = 0;
        else if (chip->reg_da)
            tremolo = chip->trem_out >> 2;
        else
            tremolo = chip->trem_out >> 4;

        int ksltltrem = ksltl + tremolo;
        int levelof = 0;

        if (ksltltrem & 0x200)
            levelof = 1;

        int totallevel = level + (ksltltrem & 0x1ff);
        if (totallevel & 0x200)
            levelof = 1;

        int totallevelclamp = (chip->reg_test0 & 1) != 0 ? 0 : (levelof ? 0x1ff : (totallevel & 0x1ff));

        chip->eg_out[0] = totallevelclamp;

        chip->eg_dbg[0] = chip->eg_dbg[1] >> 1;

        if ((chip->reg_test0 & 32) != 0 && !chip->eg_dbg_load_l[1])
        {
            chip->eg_dbg[0] |= chip->eg_out[1];
        }
        chip->eg_dbg_load_l[0] = (chip->reg_test0 & 32) != 0;

        if (chip->fsm_out[4] || chip->fsm_out[6])
            chip->eg_index[0] = 0;
        else
            chip->eg_index[0] = chip->eg_index[1] + 1;

        if (chip->eg_index[1] < 18)
        {
            int index1 = chip->eg_index[1];
            int index2 = (index1 + 17) % 18;
            chip->eg_cells[index2] = (nextstate & 3) | ((levelnext & 511) << 2) | (chip->eg_timer_i << 11);
            chip->eg_cells[index2 + 18] = chip->eg_cells[index1];
            chip->eg_state_o[0] = chip->eg_cells[18 + index1] & 3;
            chip->eg_level_o[0] = (chip->eg_cells[18 + index1] >> 2) & 511;
            chip->eg_timer_o[0] = (chip->eg_cells[18 + index1] >> 11) & 1;
        }
        chip->eg_state_o[2] = chip->eg_state_o[1];
        chip->eg_level_o[2] = chip->eg_level_o[1];
        chip->eg_timer_o[2] = chip->eg_timer_o[1];
    }
    if (chip->clk2)
    {
        chip->eg_out[1] = chip->eg_out[0];
        chip->eg_dbg[1] = chip->eg_dbg[0];
        chip->eg_dbg_load_l[1] = chip->eg_dbg_load_l[0];

        chip->eg_index[1] = chip->eg_index[0];
        chip->eg_state_o[1] = chip->eg_state_o[0];
        chip->eg_state_o[3] = chip->eg_state_o[2];
        chip->eg_level_o[1] = chip->eg_level_o[0];
        chip->eg_level_o[3] = chip->eg_level_o[2];
        chip->eg_timer_o[1] = chip->eg_timer_o[0];
        chip->eg_timer_o[3] = chip->eg_timer_o[2];
    }

    if (chip->clk1)
    {
        static const int pg_multi[16] = {
            1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30
        };
        int fnum = chip->fnum[1];
        int freq;
        int pg_add;
        int vib_sel1 = (chip->vib_cnt[1] & 3) == 2;
        int vib_sel2 = (chip->vib_cnt[1] & 1) == 1;
        int vib_sh0 = chip->reg_dv && chip->vib[1] && vib_sel1;
        int vib_sh1 = (chip->reg_dv && chip->vib[1] && vib_sel2)
            || (!chip->reg_dv && chip->vib[1] && vib_sel1);
        int vib_sh2 = !chip->reg_dv && chip->vib[1] && vib_sel2;
        int vib_sign = (chip->vib_cnt[1] & 4) != 0 && chip->vib[1];
        int vib_add = 0;
        int phase;
        if (vib_sh0)
            vib_add |= (chip->fnum[1] >> 7) & 7;
        if (vib_sh1)
            vib_add |= (chip->fnum[1] >> 8) & 3;
        if (vib_sh2)
            vib_add |= (chip->fnum[1] >> 9) & 1;
        if (vib_sign)
        {
            vib_add ^= 1023;
        }
        fnum += vib_add;
        fnum += vib_sign;
        if (vib_sign)
            fnum &= 1023;

        freq = (fnum << chip->block[1]) >> 1;

        pg_add = (freq * pg_multi[chip->multi[1]]) >> 1;

        int state = chip->eg_state_o[3];
        int dokon = state == eg_state_release && chip->keyon_comb;

        phase = ((dokon || (chip->reg_test0 & 4) != 0 || chip->reset1) ? 0 : chip->pg_phase_o[3]) + pg_add;

        if (chip->fsm_out[4] || chip->fsm_out[6])
            chip->pg_index[0] = 0;
        else
            chip->pg_index[0] = chip->pg_index[1] + 1;

        if (chip->pg_index[1] < 18)
        {
            int index1 = chip->pg_index[1];
            int index2 = (index1 + 17) % 18;
            chip->pg_cells[index2] = phase;
            chip->pg_cells[index2 + 18] = chip->pg_cells[index1];
            chip->pg_phase_o[0] = chip->pg_cells[18 + index1];
        }
        chip->pg_phase_o[2] = chip->pg_phase_o[1];
    }
    if (chip->clk2)
    {
        chip->pg_index[1] = chip->pg_index[0];
        chip->pg_phase_o[1] = chip->pg_phase_o[0];
        chip->pg_phase_o[3] = chip->pg_phase_o[2];
    }

    if (chip->rclk1)
    {
        int noise_bit;

        noise_bit = ((chip->noise_lfsr[1] >> 22) ^ (chip->noise_lfsr[1] >> 8)) & 1;

        if ((chip->noise_lfsr[1] & 0x7fffff) == 0)
            noise_bit |= 1;

        noise_bit |= (chip->reg_test0 & 2) != 0;

        if (chip->reset1)
            noise_bit = 0;

        chip->noise_lfsr[0] = (chip->noise_lfsr[1] << 1) | noise_bit;
    }
    if (chip->rclk2)
    {
        chip->noise_lfsr[1] = chip->noise_lfsr[0];
    }

    {
        int pg_out = chip->pg_phase_o[3] >> 9;
        int hh = chip->fsm_out[2] && chip->rhythm;
        int sd = chip->fsm_out[3] && chip->rhythm;
        int tc = chip->fsm_out[4] && chip->rhythm;
        int rhy = (chip->fsm_out[2] || chip->fsm_out[3] || chip->fsm_out[4]) && chip->rhythm;
        if (chip->clk1)
            chip->hh_load = chip->fsm_out[2];
        if (!chip->hh_load && chip->fsm_out[2])
        {
            chip->hh_bit2 = (pg_out >> 2) & 1;
            chip->hh_bit3 = (pg_out >> 3) & 1;
            chip->hh_bit7 = (pg_out >> 7) & 1;
            chip->hh_bit8 = (pg_out >> 8) & 1;
        }
        if (chip->clk1)
            chip->tc_load = tc;
        if (!chip->tc_load && tc)
        {
            chip->tc_bit3 = (pg_out >> 3) & 1;
            chip->tc_bit5 = (pg_out >> 5) & 1;
        }

        if (chip->clk1) // opt
        {
            int rm_bit;
            int noise = (chip->noise_lfsr[1] >> 22) & 1;

            rm_bit = (chip->hh_bit2 ^ chip->hh_bit7)
                | (chip->tc_bit5 ^ chip->hh_bit3)
                | (chip->tc_bit5 ^ chip->tc_bit3);

            chip->pg_out_rhy = 0;
            if (!rhy)
                chip->pg_out_rhy |= pg_out;
            if (hh)
            {
                chip->pg_out_rhy |= rm_bit << 9;
                if (noise ^ rm_bit)
                    chip->pg_out_rhy |= 0xd0;
                else
                    chip->pg_out_rhy |= 0x34;
            }
            if (sd)
                chip->pg_out_rhy |= (chip->hh_bit8 << 9) | ((noise ^ chip->hh_bit8) << 8);
            if (tc)
                chip->pg_out_rhy |= (rm_bit << 9) | 0x80;
        }

        if (chip->clk1)
        {
            chip->pg_dbg[0] = chip->pg_dbg[1] >> 1;

            chip->pg_dbg_load_l[0] = (chip->reg_test0 & 8) != 0;

            if ((chip->reg_test0 & 8) != 0 && !chip->pg_dbg_load_l[1])
            {
                chip->pg_dbg[0] |= chip->pg_phase_o[3] & 0x1ff;
                chip->pg_dbg[0] |= (chip->pg_out_rhy & 0x3ff) << 9;
            }
        }
        if (chip->clk2)
        {
            chip->pg_dbg_load_l[1] = chip->pg_dbg_load_l[0];

            chip->pg_dbg[1] = chip->pg_dbg[0];
        }
    }

    {

        if (chip->clk1)
        {
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
            int wf = chip->wf[1];
            int phase = chip->pg_out_rhy + chip->op_mod[1];
            int square = wf == 6;
            int sawtooth = wf == 7;

            int phase2;
            if (wf == 4 || wf == 5)
                phase2 = phase << 1;
            else
                phase2 = phase;
            phase2 &= 1023;

            if (wf == 7 ? (phase2 & 512) != 0 : (phase2 & 256) != 0)
                phase2 ^= 511;

            int mute = ((phase & 512) != 0 && (wf == 1 || wf == 4 || wf == 5)) || ((phase & 256) != 0 && wf == 3);
            int sign = (wf == 2 || wf == 3 || wf == 5) ? 0 : (phase2 & 512) != 0;

            int index = square ? 255 : (phase2 & 255);

            int ls = logsin[index >> 1];
            if ((index & 1) == 0)
                ls += logsin_d[index >> 1];

            chip->op_logsin[0] = ls;
            chip->op_saw[0] = sawtooth;
            chip->op_saw_phase[0] = phase2 & 511;

            int att = (chip->op_saw[1] ? (chip->op_saw_phase[1] << 3) : chip->op_logsin[1]) + (chip->eg_out[1] << 3);
            if (att & 4096)
                att = 4095;

            int pw = pow[(att >> 1) & 127];
            if ((att & 1) == 0)
                pw += pow_d[(att >> 1) & 127];

            chip->op_shift[0] = (att >> 8) & 15;
            chip->op_pow[0] = pw;

            chip->op_mute[0] = (chip->op_mute[1] << 1) | mute;
            chip->op_sign[0] = (chip->op_sign[1] << 1) | sign;

            int value = 0;

            if ((chip->op_mute[1] & 2) == 0)
            {
                value = ((chip->op_pow[1] | 0x400) << 1) >> chip->op_shift[1];
            }

            if ((chip->op_mute[1] & 2) == 0 && (chip->op_sign[1] & 2) != 0)
                value ^= 8191;

            for (i = 0; i < 13; i++)
            {
                int bit;
                chip->op_fb[0][i][0] = chip->op_fb[0][i][1] << 1;
                if (chip->fsm_out[15])
                    bit = (value >> i) & 1;
                else
                    bit = (chip->op_fb[0][i][1] >> 8) & 1;
                chip->op_fb[0][i][0] |= bit;
                chip->op_fb[1][i][0] = chip->op_fb[1][i][1] << 1;
                if (chip->fsm_out[15])
                    bit = (chip->op_fb[0][i][1] >> 8) & 1;
                else
                    bit = (chip->op_fb[1][i][1] >> 8) & 1;
                chip->op_fb[1][i][0] |= bit;
                chip->op_fb[2][i][0] = chip->op_fb[2][i][1] << 1;
                if (chip->fsm_out[15])
                    bit = (chip->op_fb[1][i][1] >> 8) & 1;
                else
                    bit = (chip->op_fb[2][i][1] >> 8) & 1;
                chip->op_fb[2][i][0] |= bit;
                chip->op_fb[3][i][0] = chip->op_fb[3][i][1] << 1;
                if (chip->fsm_out[15])
                    bit = (chip->op_fb[2][i][1] >> 8) & 1;
                else
                    bit = (chip->op_fb[3][i][1] >> 8) & 1;
                chip->op_fb[3][i][0] |= bit;
            }

            int fb1 = 0;
            int fb2 = 0;
            for (i = 0; i < 14; i++)
            {
                int j = i;
                if (i == 13)
                    j = 12;
                fb1 |= ((chip->op_fb[1][j][1] >> 5) & 1) << i;
                fb2 |= ((chip->op_fb[3][j][1] >> 5) & 1) << i;
            }
            int fb_sum = fb1 + fb2;
            fb_sum &= 16383;
            if (fb_sum & 8192)
                fb_sum |= ~8191;

            int mod = 0;

            if (chip->fsm_out[16] && !chip->fsm_out[14])
                mod |= value & 1023;
            if (chip->fsm_out[12])
            {
                if (chip->fb_l[1][1])
                {
                    mod |= (fb_sum >> (9 - chip->fb_l[1][1])) & 1023;
                }
            }

            chip->op_mod[0] = mod & 1023;

            chip->op_value = value;
        }
        if (chip->clk2)
        {
            chip->op_logsin[1] = chip->op_logsin[0];
            chip->op_saw[1] = chip->op_saw[0];
            chip->op_saw_phase[1] = chip->op_saw_phase[0];
            chip->op_shift[1] = chip->op_shift[0];
            chip->op_pow[1] = chip->op_pow[0];
            chip->op_mute[1] = chip->op_mute[0];
            chip->op_sign[1] = chip->op_sign[0];

            for (i = 0; i < 13; i++)
            {
                chip->op_fb[0][i][1] = chip->op_fb[0][i][0];
                chip->op_fb[1][i][1] = chip->op_fb[1][i][0];
                chip->op_fb[2][i][1] = chip->op_fb[2][i][0];
                chip->op_fb[3][i][1] = chip->op_fb[3][i][0];
            }
            chip->op_mod[1] = chip->op_mod[0];
        }
    }

    {
        if (chip->clk1)
        {
            chip->accm_load_ac_l = chip->fsm_out[6];
            chip->accm_load_bd_l = chip->fsm_out[4];
        }
        chip->accm_load_ac = !chip->accm_load_ac_l && chip->fsm_out[6];
        chip->accm_load_bd = !chip->accm_load_bd_l && chip->fsm_out[4];

        if (chip->accm_load_ac)
        {
            chip->accm_a_sign = (chip->accm_a[1] & 0x40000) == 0;
            chip->accm_a_of = !((chip->accm_a[1] & 0x78000) == 0 || (chip->accm_a[1] & 0x78000) == 0x78000);
            chip->accm_c_sign = (chip->accm_c[1] & 0x40000) == 0;
            chip->accm_c_of = !((chip->accm_c[1] & 0x78000) == 0 || (chip->accm_c[1] & 0x78000) == 0x78000);
        }

        if (chip->accm_load_bd)
        {
            chip->accm_b_sign = (chip->accm_b[1] & 0x40000) == 0;
            chip->accm_b_of = !((chip->accm_b[1] & 0x78000) == 0 || (chip->accm_b[1] & 0x78000) == 0x78000);
            chip->accm_d_sign = (chip->accm_d[1] & 0x40000) == 0;
            chip->accm_d_of = !((chip->accm_d[1] & 0x78000) == 0 || (chip->accm_d[1] & 0x78000) == 0x78000);
        }

        if (chip->clk1)
        {
            int value = 0;

            if (chip->fsm_out[13])
            {
                if (chip->fsm_out[11])
                    value = chip->op_value << 1;
                else
                {
                    value = chip->op_value;
                    if (chip->op_value & 0x1000)
                        value |= 0x2000;
                }
            }
            if (value & 0x2000)
            {
                value |= 0x7c000;
            }

            int sign;

            chip->op_value_debug = chip->op_value;

            int accm_a = chip->fsm_out[6] ? 0 : chip->accm_a[1];
            accm_a += (chip->pan_l[1][1] & 1) != 0 ? value : 0;
            chip->accm_a[0] = accm_a;
            sign = (chip->accm_a[1] & 0x40000) == 0;
            chip->accm_shift_a[0] = (chip->accm_shift_a[1] >> 1);
            if (chip->fsm_out[6])
            {
                chip->accm_shift_a[0] |= chip->accm_a[1] & 0x7fff;
                if (sign)
                    chip->accm_shift_a[0] |= 0x8000;
            }

            int accm_b = chip->fsm_out[4] ? 0 : chip->accm_b[1];
            accm_b += (chip->pan_l[1][1] & 2) != 0 ? value : 0;
            chip->accm_b[0] = accm_b;
            sign = (chip->accm_b[1] & 0x40000) == 0;
            chip->accm_shift_b[0] = (chip->accm_shift_b[1] >> 1);
            if (chip->fsm_out[4])
            {
                chip->accm_shift_b[0] |= chip->accm_b[1] & 0x7fff;
                if (sign)
                    chip->accm_shift_b[0] |= 0x8000;
            }

            int accm_c = chip->fsm_out[6] ? 0 : chip->accm_c[1];
            accm_c += (chip->pan_l[1][1] & 4) != 0 ? value : 0;
            chip->accm_c[0] = accm_c;
            sign = (chip->accm_c[1] & 0x40000) == 0;
            chip->accm_shift_c[0] = (chip->accm_shift_c[1] >> 1);
            if (chip->fsm_out[6])
            {
                chip->accm_shift_c[0] |= chip->accm_c[1] & 0x7fff;
                if (sign)
                    chip->accm_shift_c[0] |= 0x8000;
            }

            int accm_d = chip->fsm_out[4] ? 0 : chip->accm_d[1];
            accm_d += (chip->pan_l[1][1] & 8) != 0 ? value : 0;
            chip->accm_d[0] = accm_d;
            sign = (chip->accm_d[1] & 0x40000) == 0;
            chip->accm_shift_d[0] = (chip->accm_shift_d[1] >> 1);
            if (chip->fsm_out[4])
            {
                chip->accm_shift_d[0] |= chip->accm_d[1] & 0x7fff;
                if (sign)
                    chip->accm_shift_d[0] |= 0x8000;
            }
        }
        if (chip->clk2)
        {
            chip->accm_a[1] = chip->accm_a[0];
            chip->accm_b[1] = chip->accm_b[0];
            chip->accm_c[1] = chip->accm_c[0];
            chip->accm_d[1] = chip->accm_d[0];

            chip->accm_shift_a[1] = chip->accm_shift_a[0];
            chip->accm_shift_b[1] = chip->accm_shift_b[0];
            chip->accm_shift_c[1] = chip->accm_shift_c[0];
            chip->accm_shift_d[1] = chip->accm_shift_d[0];
        }

        if (chip->fsm_out[8])
        {
            chip->o_doab = chip->accm_a_of ? chip->accm_a_sign : chip->accm_shift_a[1] & 1;
            chip->o_docd = chip->accm_c_of ? chip->accm_c_sign : chip->accm_shift_c[1] & 1;
        }
        else
        {
            chip->o_doab = chip->accm_b_of ? chip->accm_b_sign : chip->accm_shift_b[1] & 1;
            chip->o_docd = chip->accm_d_of ? chip->accm_d_sign : chip->accm_shift_d[1] & 1;
        }
    }

    chip->o_sy = chip->clk2;
    chip->o_smpac = chip->fsm_out[10];
    chip->o_smpbd = chip->fsm_out[9];
    chip->o_irq_pull = chip->t1_status || chip->t2_status;

    if (chip->io_read)
    {
        chip->data_o = 0;
        if (chip->t1_status || chip->t2_status)
            chip->data_o |= 128;
        if (chip->t1_status)
            chip->data_o |= 64;
        if (chip->t2_status)
            chip->data_o |= 32;
        chip->data_z = 0;
    }
    else
        chip->data_z = 1;

    {
        if (chip->clk1)
        {
            chip->ra_dbg1[0] = chip->ra_dbg1[1] >> 1;
            chip->ra_dbg2[0] = chip->ra_dbg2[1] >> 1;
            if ((chip->reg_test0 & 128) != 0 && !chip->ra_dbg_load[1])
            {
                chip->ra_dbg1[0] |= (int64_t)chip->multi[1] << 0;
                chip->ra_dbg1[0] |= (int64_t)chip->ksr[1] << 4;
                chip->ra_dbg1[0] |= (int64_t)chip->egt[1] << 5;
                chip->ra_dbg1[0] |= (int64_t)chip->vib[1] << 6;
                chip->ra_dbg1[0] |= (int64_t)chip->am[1] << 7;
                chip->ra_dbg1[0] |= (int64_t)chip->tl[1] << 8;
                chip->ra_dbg1[0] |= (int64_t)chip->ksl[1] << 14;
                chip->ra_dbg1[0] |= (int64_t)chip->dr[1] << 16;
                chip->ra_dbg1[0] |= (int64_t)chip->ar[1] << 20;
                chip->ra_dbg1[0] |= (int64_t)chip->rr[1] << 24;
                chip->ra_dbg1[0] |= (int64_t)chip->sl[1] << 28;
                chip->ra_dbg1[0] |= (int64_t)chip->wf[1] << 32;
                chip->ra_dbg2[0] |= (int64_t)chip->fnum[1] << 0;
                chip->ra_dbg2[0] |= (int64_t)chip->block[1] << 10;
                chip->ra_dbg2[0] |= (int64_t)chip->keyon[1] << 13;
                chip->ra_dbg2[0] |= (int64_t)chip->connect[1] << 14;
                chip->ra_dbg2[0] |= (int64_t)chip->pan[1] << 15;
                chip->ra_dbg2[0] |= (int64_t)chip->connect_pair[1] << 19;
                chip->ra_dbg2[0] |= (int64_t)chip->fb[1] << 20;
            }
            chip->ra_dbg_load[0] = (chip->reg_test0 & 128) != 0;
        }
        if (chip->clk2)
        {
            chip->ra_dbg1[1] = chip->ra_dbg1[0];
            chip->ra_dbg2[1] = chip->ra_dbg2[0];
            chip->ra_dbg_load[1] = chip->ra_dbg_load[0];
        }
    }

    switch (chip->reg_test1 & 7)
    {
        case 0:
            chip->o_test = 0;
            break;
        case 1:
            chip->o_test = (chip->ra_dbg1[1] & 1) != 0;
            break;
        case 2:
            chip->o_test = (chip->ra_dbg2[1] & 1) != 0;
            break;
        case 3:
            chip->o_test = (chip->pg_dbg[1] & 1) != 0;
            break;
        case 4:
            chip->o_test = (chip->eg_dbg[1] & 1) != 0;
            break;
    }

end:

    if (chip->io_write0)
        chip->write0_sr = 1;
    else if (chip->reset0 || chip->write0_l[1])
        chip->write0_sr = 0;

    if (chip->io_write1)
        chip->write1_sr = 1;
    else if (chip->reset0 || chip->write1_l[1])
        chip->write1_sr = 0;

    if (chip->clk1)
    {
        chip->write0_l[0] = chip->write0_sr;
        chip->write0_l[2] = chip->write0_l[1];

        chip->write1_l[0] = chip->write1_sr;
        chip->write1_l[2] = chip->write1_l[1];
    }

    if (chip->mclk1)
    {
        chip->prescaler1_reset[0] = (chip->prescaler1_reset[1] << 1) | chip->reset1;

        if (!(chip->prescaler1_reset[1] & 2) && chip->reset1)
            chip->prescaler1_cnt[0] = 0;
        else
            chip->prescaler1_cnt[0] = (chip->prescaler1_cnt[1] + 1) & 3;
    }

    if (chip->aclk1)
    {
        int prescaler2_reset = !(chip->prescaler2_reset_l[1] & 2) && chip->reset1;
        chip->prescaler2_reset_l[0] = (chip->prescaler2_reset_l[1] << 1) | chip->reset1;

        if (prescaler2_reset)
            chip->prescaler2_cnt[0] = 0;
        else
            chip->prescaler2_cnt[0] = (chip->prescaler2_cnt[1] + 1) & 3;

        chip->prescaler2_l1[0] = !prescaler2_reset && (chip->prescaler2_cnt[1] & 1) == 0;
        chip->prescaler2_l2 = chip->prescaler2_l1[1];

        chip->prescaler2_l3[0] = !prescaler2_reset && (chip->prescaler2_cnt[1] & 1) != 0;
        chip->prescaler2_l4 = chip->prescaler2_l3[1];

        chip->prescaler2_l5[0] = !prescaler2_reset && chip->prescaler2_cnt[1] == 3;

        chip->prescaler2_l6[0] = !prescaler2_reset && chip->prescaler2_cnt[1] == 1;

        chip->prescaler2_l7 = (chip->prescaler2_cnt[1] & 1) == 0;
    }
}
