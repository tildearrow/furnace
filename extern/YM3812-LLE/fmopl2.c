/*
 * Copyright (C) 2023 nukeykt
 *
 * This file is part of YM3812-LLE.
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
 *  YM3812 emulator
 *  Thanks:
 *      Travis Goodspeed:
 *          YM3812 decap and die shot
 *
 */

#include "fmopl2.h"


void FMOPL2_DoShiftRegisters(fmopl2_t *chip, int sel)
{
    int j;
    int to = sel;
    int from = sel ^ 1;
    int rot = sel == 0 ? 1 : 0;
#define CH_ROTATE(x) rot ? ((x << 1) | ((x >> 8) & 1)) : x
#define OP_ROTATE(x) rot ? ((x << 1) | ((x >> 17) & 1)) : x
    // channel registers

    // fnum
    for (j = 0; j < 10; j++)
        chip->ch_fnum[j][to] = CH_ROTATE(chip->ch_fnum[j][from]);
    // block
    for (j = 0; j < 3; j++)
        chip->ch_block[j][to] = CH_ROTATE(chip->ch_block[j][from]);
    // kon
    chip->ch_keyon[to] = CH_ROTATE(chip->ch_keyon[from]);
    // connect
    chip->ch_connect[to] = CH_ROTATE(chip->ch_connect[from]);
    // feedback
    for (j = 0; j < 3; j++)
        chip->ch_fb[j][to] = CH_ROTATE(chip->ch_fb[j][from]);
    // multi
    for (j = 0; j < 4; j++)
        chip->op_multi[j][to] = OP_ROTATE(chip->op_multi[j][from]);
    // ksr
    chip->op_ksr[to] = OP_ROTATE(chip->op_ksr[from]);
    // egt
    chip->op_egt[to] = OP_ROTATE(chip->op_egt[from]);
    // vib
    chip->op_vib[to] = OP_ROTATE(chip->op_vib[from]);
    // am
    chip->op_am[to] = OP_ROTATE(chip->op_am[from]);
    // tl
    for (j = 0; j < 6; j++)
        chip->op_tl[j][to] = OP_ROTATE(chip->op_tl[j][from]);
    // ksl
    for (j = 0; j < 2; j++)
        chip->op_ksl[j][to] = OP_ROTATE(chip->op_ksl[j][from]);
    // ar
    for (j = 0; j < 4; j++)
        chip->op_ar[j][to] = OP_ROTATE(chip->op_ar[j][from]);
    // dr
    for (j = 0; j < 4; j++)
        chip->op_dr[j][to] = OP_ROTATE(chip->op_dr[j][from]);
    // sl
    for (j = 0; j < 4; j++)
        chip->op_sl[j][to] = OP_ROTATE(chip->op_sl[j][from]);
    // rr
    for (j = 0; j < 4; j++)
        chip->op_rr[j][to] = OP_ROTATE(chip->op_rr[j][from]);
    // wf
    for (j = 0; j < 2; j++)
        chip->op_wf[j][to] = OP_ROTATE(chip->op_wf[j][from]);
#undef CH_ROTATE
#undef OP_ROTATE
}

enum {
    eg_state_attack = 0,
    eg_state_decay,
    eg_state_sustain,
    eg_state_release
};

void FMOPL2_Clock(fmopl2_t *chip)
{
    int i;

    chip->mclk1 = !chip->input.mclk;
    chip->mclk2 = chip->input.mclk;

    chip->reset1 = !chip->input.ic;
    chip->io_rd = !chip->input.rd;
    chip->io_wr = !chip->input.wr;
    chip->io_cs = !chip->input.cs;
    chip->io_a0 = chip->input.address & 1;

    if (chip->mclk1)
    {
        int prescaler_reset = !(chip->prescaler_reset_l[1] & 2) && chip->reset1;
        chip->prescaler_reset_l[0] = (chip->prescaler_reset_l[1] << 1) | chip->reset1;
        if (prescaler_reset)
            chip->prescaler_cnt[0] = 0;
        else
            chip->prescaler_cnt[0] = (chip->prescaler_cnt[1] + 1) & 3;

        chip->prescaler_l1[0] = !prescaler_reset && chip->prescaler_cnt[1] == 1;
        chip->prescaler_l2[0] = chip->prescaler_cnt[1] == 3;
    }
    if (chip->mclk2)
    {
        chip->prescaler_reset_l[1] = chip->prescaler_reset_l[0];
        chip->prescaler_cnt[1] = chip->prescaler_cnt[0];
        chip->prescaler_l1[1] = chip->prescaler_l1[0];
        chip->prescaler_l2[1] = chip->prescaler_l2[0];
    }

    chip->clk1 = chip->prescaler_l1[1];
    chip->clk2 = chip->prescaler_l2[1];

    chip->io_read0 = !chip->reset1 && chip->io_cs && chip->io_rd && !chip->io_a0;
    chip->io_read1 = !chip->reset1 && chip->io_cs && chip->io_rd && chip->io_a0;
    chip->io_write = !chip->reset1 && chip->io_cs && chip->io_wr;
    chip->io_write0 = !chip->reset1 && chip->io_cs && chip->io_wr && !chip->io_a0;
    chip->io_write1 = !chip->reset1 && chip->io_cs && chip->io_wr && chip->io_a0;
    chip->io_dir = chip->io_cs && chip->io_rd;

    int irq = chip->t1_status || chip->t2_status || chip->unk_status1 || chip->unk_status2;

    if (!chip->io_dir)
        chip->io_data = chip->input.data_i;

    if (chip->io_write)
        chip->data_latch = chip->io_data;

    if (chip->write0)
        chip->write0_sr = 0;
    else if (chip->io_write0)
        chip->write0_sr = 1;

    if (chip->write1)
        chip->write1_sr = 0;
    else if (chip->io_write1)
        chip->write1_sr = 1;

    if (chip->mclk1)
    {
        chip->write0_latch[1] = chip->write0_latch[0];
        chip->write1_latch[1] = chip->write1_latch[0];
    }
    if (chip->mclk2)
    {
        chip->write0_latch[0] = chip->write0_sr;
        chip->write0_latch[2] = chip->write0_latch[1];

        chip->write1_latch[0] = chip->write1_sr;
        chip->write1_latch[2] = chip->write1_latch[1];
    }

    if (chip->clk1)
    {
        chip->write0_latch[4] = chip->write0_latch[3];
        chip->write1_latch[4] = chip->write1_latch[3];
    }
    if (chip->clk2)
    {
        chip->write0_latch[3] = chip->write0_latch[2];
        chip->write0_latch[5] = chip->write0_latch[4];

        chip->write1_latch[3] = chip->write1_latch[2];
        chip->write1_latch[5] = chip->write1_latch[4];
    }

    chip->write0 = chip->write0_latch[5];
    chip->write1 = chip->write1_latch[5];

    ////

    if (chip->o_clk1 == chip->clk1 && chip->o_clk2 == chip->clk2 && chip->o_reset1 == chip->reset1
        && chip->o_write0 == chip->write0 && chip->o_write1 == chip->write1 && chip->o_data_latch == chip->data_latch)
        goto end; // opt

    chip->o_clk1 = chip->clk1;
    chip->o_clk2 = chip->clk2;
    chip->o_reset1 = chip->reset1;
    chip->o_write0 = chip->write0;
    chip->o_write1 = chip->write1;
    chip->o_data_latch = chip->data_latch;

    if (chip->write0)
    {
        chip->reg_sel1 = chip->data_latch == 1;
        chip->reg_sel2 = chip->data_latch == 2;
        chip->reg_sel3 = chip->data_latch == 3;
        chip->reg_sel4 = chip->data_latch == 4;
        chip->reg_sel8 = chip->data_latch == 8;
        chip->reg_selbd = chip->data_latch == 0xbd;
    }

    chip->reg_sel4_wr = chip->write1 && chip->reg_sel4 && (chip->data_latch & 128) == 0;
    chip->reg_sel4_rst = (chip->write1 && chip->reg_sel4 && (chip->data_latch & 128) != 0) || chip->reset1;

    if (chip->reset1)
    {
        chip->reg_test = 0;
        chip->reg_timer1 = 0;
        chip->reg_timer2 = 0;
        chip->reg_notesel = 0;
        chip->reg_csm = 0;
        chip->rhythm = 0;
        chip->reg_rh_kon = 0;
        chip->reg_da = 0;
        chip->reg_dv = 0;
    }
    else if (chip->write1)
    {
        if (chip->reg_sel1)
            chip->reg_test = chip->data_latch & 255;
        if (chip->reg_sel2)
            chip->reg_timer1 = chip->data_latch & 255;
        if (chip->reg_sel3)
            chip->reg_timer2 = chip->data_latch & 255;
        if (chip->reg_sel8)
        {
            chip->reg_notesel = (chip->data_latch & 64) != 0;
            chip->reg_csm = (chip->data_latch & 128) != 0;
        }
        if (chip->reg_selbd)
        {
            chip->reg_rh_kon = chip->data_latch & 31;
            chip->rhythm = (chip->data_latch & 32) != 0;
            chip->reg_dv = (chip->data_latch & 64) != 0;
            chip->reg_da = (chip->data_latch & 128) != 0;
        }
    }

    if (chip->reset1)
    {
        chip->reg_t1_mask = 0;
        chip->reg_t2_mask = 0;
        chip->reg_t1_start = 0;
        chip->reg_t2_start = 0;
        chip->reg_mode_b3 = 0;
        chip->reg_mode_b4 = 0;
    }
    else if (chip->reg_sel4_wr)
    {
        chip->reg_t1_mask = (chip->data_latch & 64) != 0;
        chip->reg_t2_mask = (chip->data_latch & 32) != 0;
        chip->reg_t1_start = (chip->data_latch & 1) != 0;
        chip->reg_t2_start = (chip->data_latch & 2) != 0;
        chip->reg_mode_b3 = (chip->data_latch & 8) != 0;
        chip->reg_mode_b4 = (chip->data_latch & 16) != 0;
    }

    {
        chip->fsm_reset = !(chip->fsm_reset_l[1] & 2) && chip->reset1;
        chip->fsm_cnt1_of = (chip->fsm_cnt1[1] & 5) == 5;
        chip->fsm_cnt2_of = chip->fsm_cnt1_of && (chip->fsm_cnt2[1] & 2) != 0;

        chip->fsm_cnt = (chip->fsm_cnt2[1] << 3) | chip->fsm_cnt1[1];

        chip->fsm_sel[0] = chip->fsm_cnt == 20 && chip->rhythm;
        chip->fsm_sel[1] = chip->fsm_cnt == 19 && chip->rhythm;
        chip->fsm_sel[2] = chip->fsm_cnt == 18 && chip->rhythm;
        chip->fsm_sel[3] = chip->fsm_cnt == 17 && chip->rhythm;
        chip->fsm_sel[4] = chip->fsm_cnt == 16 && chip->rhythm;
        chip->fsm_sel[5] = chip->fsm_cnt == 20 && chip->rhythm;
        chip->fsm_sel[6] = chip->fsm_cnt == 19 && chip->rhythm;
        chip->fsm_sel[7] = (chip->fsm_cnt & 5) == 4;
        chip->fsm_sel[8] = chip->fsm_cnt == 16;
        chip->fsm_sel[9] = (chip->fsm_cnt & 29) == 5;
        chip->fsm_sel[10] = chip->fsm_cnt == 16;
        chip->fsm_sel[11] = chip->fsm_cnt == 11;
        chip->fsm_sel[12] = chip->fsm_cnt == 20;

        int fsm_mc = !(chip->fsm_sel[7] || (chip->fsm_cnt & 2) != 0);

        chip->fsm_out[0] = ((chip->connect_l[1] & 2) != 0 || chip->fsm_sel[0] || chip->fsm_sel[1] || fsm_mc) && !chip->fsm_sel[2];

        chip->fsm_out[1] = fsm_mc && !chip->fsm_sel[3] && !chip->fsm_sel[4];

        chip->fsm_out[2] = !fsm_mc && !chip->fsm_sel[5] && !chip->fsm_sel[6];

        chip->fsm_out[3] = !(chip->fsm_l1[1] && 1);

        chip->fsm_out[4] = chip->fsm_l2[1];

        chip->fsm_out[5] = chip->fsm_sel[10];

        chip->fsm_out[6] = chip->fsm_sel[11];

        chip->fsm_out[7] = chip->fsm_sel[12];

        chip->fsm_out[8] = (chip->fsm_l3[1] & 1) != 0;

        chip->fsm_out[9] = (chip->fsm_l3[1] & 2) != 0;

        chip->fsm_out[10] = (chip->fsm_l3[1] & 2) != 0;

        chip->fsm_out[11] = (chip->fsm_l4[1] & 2) != 0 && chip->rhythm;

        chip->fsm_out[12] = (chip->fsm_l5[1] & 4) != 0;

        chip->fsm_out[13] = (chip->fsm_l6[1] & 4) != 0;

        chip->fsm_out[14] = !(chip->fsm_out[12] || (chip->fsm_cnt & 16) != 0);

        chip->fsm_out[15] = !(chip->fsm_out[12] || chip->fsm_out[13]);
    }

    if (chip->clk1)
    {
        if (chip->fsm_reset || chip->fsm_cnt1_of)
            chip->fsm_cnt1[0] = 0;
        else
            chip->fsm_cnt1[0] = (chip->fsm_cnt1[1] + 1) & 7;
        if (chip->fsm_reset || chip->fsm_cnt2_of)
            chip->fsm_cnt2[0] = 0;
        else
            chip->fsm_cnt2[0] = (chip->fsm_cnt2[1] + chip->fsm_cnt1_of) & 3;

        chip->fsm_reset_l[0] = (chip->fsm_reset_l[1] << 1) | chip->reset1;

        chip->fsm_l1[0] = !chip->fsm_sel[8] && !chip->fsm_sel[9] && (chip->fsm_cnt & 8) == 0;

        chip->fsm_l2[0] = chip->fsm_sel[10];

        chip->fsm_l3[0] = (chip->fsm_l3[1] << 1) | chip->fsm_sel[12];

        chip->fsm_l4[0] = (chip->fsm_l4[1] << 1) | ((chip->fsm_cnt & 16) != 0);

        chip->fsm_l5[0] = (chip->fsm_l5[1] << 1) | ((chip->fsm_cnt & 8) != 0);

        chip->fsm_l6[0] = (chip->fsm_l6[1] << 1) | ((chip->fsm_cnt & 16) != 0);
    }
    if (chip->clk2)
    {
        chip->fsm_cnt1[1] = chip->fsm_cnt1[0];
        chip->fsm_cnt2[1] = chip->fsm_cnt2[0];
        chip->fsm_reset_l[1] = chip->fsm_reset_l[0];
        chip->fsm_l1[1] = chip->fsm_l1[0];
        chip->fsm_l2[1] = chip->fsm_l2[0];
        chip->fsm_l3[1] = chip->fsm_l3[0];
        chip->fsm_l4[1] = chip->fsm_l4[0];
        chip->fsm_l5[1] = chip->fsm_l5[0];
        chip->fsm_l6[1] = chip->fsm_l6[0];
    }

    if (chip->clk1)
        chip->timer_st_load_l = chip->fsm_out[8];
    chip->timer_st_load = chip->fsm_out[8] && !chip->timer_st_load_l;

    if (chip->timer_st_load)
        chip->t1_start = chip->reg_t1_start;

    if (chip->clk1)
    {
        int lfo = chip->lfo_cnt[1];
        int add = chip->fsm_out[8];

        chip->lfo_cnt[0] = (chip->reg_test & 128) != 0 ? 0 : (lfo + add) & 1023;
        chip->vib_cnt[0] = (chip->reg_test & 128) != 0 ? 0 : (chip->vib_cnt[1] + chip->vib_step) & 7;
    }
    if (chip->clk2)
    {
        chip->lfo_cnt[1] = chip->lfo_cnt[0];
        chip->vib_cnt[1] = chip->vib_cnt[0];
    }

    {
        int lfo = chip->lfo_cnt[1];
        int add = chip->fsm_out[8];

        chip->t1_step = (((lfo & 3) + add) & 4) != 0;
        chip->t2_step = (((lfo & 15) + add) & 16) != 0;
        chip->am_step = (((lfo & 63) + add) & 64) != 0;
        chip->vib_step = (((lfo & 1023) + add) & 1024) != 0;
        chip->vib_step |= (chip->reg_test & 8) != 0 && add;
    }

    if (chip->clk1)
    {
        int value = chip->t1_load ? chip->reg_timer1 : chip->t1_cnt[1];
        value += ((chip->t1_start_l[1] & 1) != 0 && chip->t1_step) || (chip->reg_test & 2) != 0;
        chip->t1_of[0] = (value & 256) != 0;
        chip->t1_cnt[0] = (chip->t1_start_l[1] & 1) == 0 ? 0 : (value & 255);
        
        value = (chip->t2_of[1] || (chip->t2_start_l[1] & 3) == 1) ? chip->reg_timer2 : chip->t2_cnt[1];
        value += ((chip->t2_start_l[1] & 1) != 0 && chip->t2_step) || (chip->reg_test & 2) != 0;
        chip->t2_of[0] = (value & 256) != 0;
        chip->t2_cnt[0] = (chip->t2_start_l[1] & 1) == 0 ? 0 : (value & 255);

        chip->t1_start_l[0] = (chip->t1_start_l[1] << 1) | chip->t1_start;
        chip->t2_start_l[0] = (chip->t2_start_l[1] << 1) | chip->reg_t2_start;
    }
    if (chip->clk2)
    {
        chip->t1_cnt[1] = chip->t1_cnt[0];
        chip->t1_of[1] = chip->t1_of[0];
        chip->t2_cnt[1] = chip->t2_cnt[0];
        chip->t2_of[1] = chip->t2_of[0];

        chip->t1_start_l[1] = chip->t1_start_l[0];
        chip->t2_start_l[1] = chip->t2_start_l[0];

        chip->t1_load = (chip->t1_of[1] || (chip->t1_start_l[1] & 3) == 1); // opt
    }

    if (chip->reg_sel4_rst || chip->reg_t1_mask)
        chip->t1_status = 0;
    else if (chip->t1_of[1])
        chip->t1_status = 1;

    if (chip->reg_sel4_rst || chip->reg_t2_mask)
        chip->t2_status = 0;
    else if (chip->t2_of[1])
        chip->t2_status = 1;

    if (chip->reg_sel4_rst || chip->reg_mode_b4)
        chip->unk_status1 = 0;
    else if (0)
        chip->unk_status1 = 1;

    chip->unk_status2 = 0;

    if (chip->clk1)
        chip->csm_load_l = chip->fsm_out[10];
    chip->csm_load = chip->fsm_out[10] && !chip->csm_load_l;

    if (chip->csm_load)
        chip->csm_kon = chip->reg_csm && chip->t1_load;

    chip->rh_sel0 = chip->rhythm && chip->fsm_out[5];

    if (chip->clk1)
    {
        chip->rh_sel[0] = (chip->rh_sel[1] << 1) | chip->rh_sel0;
    }
    if (chip->clk2)
    {
        chip->rh_sel[1] = chip->rh_sel[0];
    }

    //if (chip->clk1) // opt
    {
        chip->keyon_comb = chip->keyon || chip->csm_kon
            || (chip->rh_sel0 && (chip->reg_rh_kon & 16) != 0) // bd0
            || ((chip->rh_sel[1] & 1) != 0 && (chip->reg_rh_kon & 1) != 0) // hh
            || ((chip->rh_sel[1] & 2) != 0 && (chip->reg_rh_kon & 4) != 0) // tom
            || ((chip->rh_sel[1] & 4) != 0 && (chip->reg_rh_kon & 16) != 0) // bd1
            || ((chip->rh_sel[1] & 8) != 0 && (chip->reg_rh_kon & 8) != 0) // sd
            || ((chip->rh_sel[1] & 16) != 0 && (chip->reg_rh_kon & 2) != 0); // tc
    }

    if (chip->reset1)
        chip->address = 0;
    else if ((chip->data_latch & 0xe0) != 0 && chip->write0)
        chip->address = chip->data_latch;

    if (chip->write0)
        chip->address_valid = (chip->data_latch & 0xe0) != 0;

    if (chip->reset1)
        chip->data = 0;
    else if (chip->address_valid && chip->write1)
        chip->data = chip->data_latch;

    chip->address_valid2 = chip->address_valid_l[1] && !chip->write0;

    if (chip->clk1)
    {
        chip->address_valid_l[0] = (chip->address_valid && chip->write1) || chip->address_valid2;

        int slot_cnt1_of = (chip->slot_cnt1[1] & 5) == 5;

        if (chip->fsm_out[8] || slot_cnt1_of)
            chip->slot_cnt1[0] = 0;
        else
            chip->slot_cnt1[0] = (chip->slot_cnt1[1] + 1) & 7;

        if (chip->fsm_out[8] || (slot_cnt1_of && (chip->slot_cnt2[1] & 2) != 0))
            chip->slot_cnt2[0] = 0;
        else
            chip->slot_cnt2[0] = (chip->slot_cnt2[1] + slot_cnt1_of) & 3;
    }
    if (chip->clk2)
    {
        chip->address_valid_l[1] = chip->address_valid_l[0];

        chip->slot_cnt1[1] = chip->slot_cnt1[0];
        chip->slot_cnt2[1] = chip->slot_cnt2[0];

        chip->slot_cnt = (chip->slot_cnt2[1] << 3) | chip->slot_cnt1[1]; // opt
    }

    if (chip->clk1)
    {
        int sel_ch = (chip->address & 0xf0) == 0xa0 || (chip->address & 0xf0) == 0xb0 || (chip->address & 0xf0) == 0xc0;
        int addr_add = sel_ch && ((chip->address & 8) != 0 || (chip->address & 6) == 6);

        int addr_sel = chip->address & 1;

        addr_sel |= (((chip->address >> 1) + addr_add) & 7) << 1;

        if (!sel_ch)
            addr_sel |= chip->address & 16;

        int addr_match = addr_sel == chip->slot_cnt && chip->address_valid2;

        int sel_20 = (chip->address & 0xe0) == 0x20 && addr_match;
        int sel_40 = (chip->address & 0xe0) == 0x40 && addr_match;
        int sel_60 = (chip->address & 0xe0) == 0x60 && addr_match;
        int sel_80 = (chip->address & 0xe0) == 0x80 && addr_match;
        int sel_e0 = (chip->address & 0xe0) == 0xe0 && addr_match && (chip->reg_test & 32) != 0;

        int sel_a0 = (chip->address & 0xf0) == 0xa0 && addr_match;
        int sel_b0 = (chip->address & 0xf0) == 0xb0 && addr_match;
        int sel_c0 = (chip->address & 0xf0) == 0xc0 && addr_match;

        FMOPL2_DoShiftRegisters(chip, 0);

        if (chip->reset1)
        {
            for (i = 0; i < 10; i++)
                chip->ch_fnum[i][0] &= ~1;
            for (i = 0; i < 3; i++)
                chip->ch_block[i][0] &= ~1;
            chip->ch_keyon[0] &= ~1;
            chip->ch_connect[0] &= ~1;
            for (i = 0; i < 3; i++)
                chip->ch_fb[i][0] &= ~1;

            for (i = 0; i < 4; i++)
                chip->op_multi[i][0] &= ~1;
            chip->op_ksr[0] &= ~1;
            chip->op_egt[0] &= ~1;
            chip->op_vib[0] &= ~1;
            chip->op_am[0] &= ~1;
            for (i = 0; i < 6; i++)
                chip->op_tl[i][0] &= ~1;
            for (i = 0; i < 2; i++)
                chip->op_ksl[i][0] &= ~1;
            for (i = 0; i < 4; i++)
                chip->op_ar[i][0] &= ~1;
            for (i = 0; i < 4; i++)
                chip->op_dr[i][0] &= ~1;
            for (i = 0; i < 4; i++)
                chip->op_sl[i][0] &= ~1;
            for (i = 0; i < 4; i++)
                chip->op_rr[i][0] &= ~1;
            for (i = 0; i < 2; i++)
                chip->op_wf[i][0] &= ~1;
        }
        else
        {
            if (sel_a0)
            {
                for (i = 0; i < 8; i++)
                    chip->ch_fnum[i][0] &= ~1;

                for (i = 0; i < 8; i++)
                    chip->ch_fnum[i][0] |= (chip->data >> i) & 1;
            }
            if (sel_b0)
            {
                for (i = 8; i < 10; i++)
                    chip->ch_fnum[i][0] &= ~1;
                for (i = 0; i < 3; i++)
                    chip->ch_block[i][0] &= ~1;
                chip->ch_keyon[0] &= ~1;

                for (i = 8; i < 10; i++)
                    chip->ch_fnum[i][0] |= (chip->data >> (i - 8)) & 1;
                for (i = 0; i < 3; i++)
                    chip->ch_block[i][0] |= (chip->data >> (i + 2)) & 1;
                chip->ch_keyon[0] |= (chip->data >> 5) & 1;
            }
            if (sel_c0)
            {
                chip->ch_connect[0] &= ~1;
                for (i = 0; i < 3; i++)
                    chip->ch_fb[i][0] &= ~1;

                chip->ch_connect[0] |= (chip->data >> 0) & 1;
                for (i = 0; i < 3; i++)
                    chip->ch_fb[i][0] |= (chip->data >> (i + 1)) & 1;
            }
            if (sel_20)
            {
                for (i = 0; i < 4; i++)
                    chip->op_multi[i][0] &= ~1;
                chip->op_ksr[0] &= ~1;
                chip->op_egt[0] &= ~1;
                chip->op_vib[0] &= ~1;
                chip->op_am[0] &= ~1;

                for (i = 0; i < 4; i++)
                    chip->op_multi[i][0] |= (chip->data >> i) & 1;
                chip->op_ksr[0] |= (chip->data >> 4) & 1;
                chip->op_egt[0] |= (chip->data >> 5) & 1;
                chip->op_vib[0] |= (chip->data >> 6) & 1;
                chip->op_am[0] |= (chip->data >> 7) & 1;
            }
            if (sel_40)
            {
                for (i = 0; i < 6; i++)
                    chip->op_tl[i][0] &= ~1;
                for (i = 0; i < 2; i++)
                    chip->op_ksl[i][0] &= ~1;

                for (i = 0; i < 6; i++)
                    chip->op_tl[i][0] |= (chip->data >> i) & 1;
                for (i = 0; i < 2; i++)
                    chip->op_ksl[i][0] |= (chip->data >> (i + 6)) & 1;
            }
            if (sel_60)
            {
                for (i = 0; i < 4; i++)
                    chip->op_ar[i][0] &= ~1;
                for (i = 0; i < 4; i++)
                    chip->op_dr[i][0] &= ~1;

                for (i = 0; i < 4; i++)
                    chip->op_ar[i][0] |= (chip->data >> (i + 4)) & 1;
                for (i = 0; i < 4; i++)
                    chip->op_dr[i][0] |= (chip->data >> i) & 1;
            }
            if (sel_80)
            {
                for (i = 0; i < 4; i++)
                    chip->op_sl[i][0] &= ~1;
                for (i = 0; i < 4; i++)
                    chip->op_rr[i][0] &= ~1;

                for (i = 0; i < 4; i++)
                    chip->op_sl[i][0] |= (chip->data >> (i + 4)) & 1;
                for (i = 0; i < 4; i++)
                    chip->op_rr[i][0] |= (chip->data >> i) & 1;
            }
            if (sel_e0)
            {
                for (i = 0; i < 2; i++)
                    chip->op_wf[i][0] &= ~1;

                for (i = 0; i < 2; i++)
                    chip->op_wf[i][0] |= (chip->data >> i) & 1;
            }
        }
    }
    if (chip->clk2)
    {
        FMOPL2_DoShiftRegisters(chip, 1);
    }

    //if (chip->clk2) // opt
    {
        int shift = 0;

        if (chip->fsm_out[13])
            shift = 8;
        else if (chip->fsm_out[12])
            shift = 5;
        else if (chip->fsm_out[15])
            shift = 2;

        chip->block = 0;
        chip->fnum = 0;
        for (i = 0; i < 3; i++)
            chip->block |= ((chip->ch_block[i][1] >> shift) & 1) << i;
        for (i = 0; i < 10; i++)
            chip->fnum |= ((chip->ch_fnum[i][1] >> shift) & 1) << i;
        chip->keyon = (chip->ch_keyon[1] >> shift) & 1;
        chip->connect = (chip->ch_connect[1] >> shift) & 1;

        chip->fb = 0;
        if (chip->fsm_out[13])
            shift = 5;
        else if (chip->fsm_out[12])
            shift = 2;
        else if (chip->fsm_out[15])
            shift = 8;
        for (i = 0; i < 3; i++)
            chip->fb |= ((chip->ch_fb[i][1] >> shift) & 1) << i;

        chip->multi = 0;
        chip->tl = 0;
        chip->ksl = 0;
        chip->ar = 0;
        chip->dr = 0;
        chip->sl = 0;
        chip->rr = 0;
        chip->wf = 0;

        for (i = 0; i < 4; i++)
            chip->multi |= ((chip->op_multi[i][1] >> 17) & 1) << i;

        chip->ksr = (chip->op_ksr[1] >> 17) & 1;
        chip->egt = (chip->op_egt[1] >> 17) & 1;
        chip->vib = (chip->op_vib[1] >> 17) & 1;
        chip->am = (chip->op_am[1] >> 17) & 1;

        for (i = 0; i < 6; i++)
            chip->tl |= ((chip->op_tl[i][1] >> 17) & 1) << i;

        for (i = 0; i < 2; i++)
            chip->ksl |= ((chip->op_ksl[i][1] >> 17) & 1) << i;

        for (i = 0; i < 4; i++)
            chip->ar |= ((chip->op_ar[i][1] >> 17) & 1) << i;

        for (i = 0; i < 4; i++)
            chip->dr |= ((chip->op_dr[i][1] >> 17) & 1) << i;

        for (i = 0; i < 4; i++)
            chip->sl |= ((chip->op_sl[i][1] >> 17) & 1) << i;

        for (i = 0; i < 4; i++)
            chip->rr |= ((chip->op_rr[i][1] >> 17) & 1) << i;

        for (i = 0; i < 2; i++)
            chip->wf |= ((chip->op_wf[i][1] >> 17) & 1) << i;

    }

    if (chip->clk1)
    {
        chip->connect_l[0] = (chip->connect_l[1] << 1) | chip->connect;
        chip->fb_l[0][0] = chip->fb;
        chip->fb_l[1][0] = chip->fb_l[0][1];
    }
    if (chip->clk2)
    {
        chip->connect_l[1] = chip->connect_l[0];
        chip->fb_l[0][1] = chip->fb_l[0][0];
        chip->fb_l[1][1] = chip->fb_l[1][0];
    }

    if (chip->clk1)
    {
        chip->eg_load1_l = chip->fsm_out[8];
        chip->eg_load2_l = chip->fsm_out[9];
        chip->eg_load3_l = chip->eg_subcnt_l[1] && chip->eg_sync_l[1];
    }
    chip->eg_load1 = !chip->eg_load1_l && chip->fsm_out[8];
    chip->eg_load2 = !chip->eg_load2_l && chip->fsm_out[9];
    chip->eg_load3 = !chip->eg_load3_l && chip->eg_subcnt_l[1] && chip->eg_sync_l[1];

    {

        if (chip->eg_load1)
            chip->trem_step = chip->am_step;
        if (chip->eg_load2)
            chip->trem_out = chip->trem_value[1] & 127;

        if (chip->clk1)
        {
            int bit = chip->trem_value[1] & 1;
            int reset = chip->reset1 || (chip->reg_test & 128) != 0;

            int step = ((chip->trem_step || (chip->reg_test & 8) != 0) && (chip->fsm_out[9] || chip->trem_dir[1]))
                && chip->fsm_out[14];
            int carry = chip->fsm_out[14] && chip->trem_carry[1];

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

        if (chip->eg_load3)
        {
            chip->eg_timer_low = chip->eg_timer[1] & 3;
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
            int bit = chip->eg_timer[1] & 1;
            int bit2;
            int carry = chip->eg_carry[1] || (chip->eg_subcnt[1] && chip->eg_sync_l[1]);
            bit += carry;

            if (chip->reset1)
                bit2 = 0;
            else
                bit2 = bit & 1;

            chip->eg_timer[0] = (chip->eg_timer[1] >> 1) & 0x1ffff;
            chip->eg_timer[0] |= bit2 << 17;
            chip->eg_carry[0] = (bit & 2) != 0;
            chip->eg_sync_l[0] = chip->fsm_out[8];
            chip->eg_mask[0] = (chip->reset1 || chip->fsm_out[8]) ? 0 :
                (chip->eg_mask[1] || bit2);
            chip->eg_timer_masked[0] = (chip->eg_timer_masked[1] >> 1) & 0x1ffff;
            if (!chip->eg_mask[1])
                chip->eg_timer_masked[0] |= bit2 << 17;

            if (chip->reset1)
                chip->eg_subcnt[0] = 0;
            else
                chip->eg_subcnt[0] = chip->eg_subcnt[1] ^ chip->fsm_out[8];

            chip->eg_subcnt_l[0] = chip->eg_subcnt[1];
        }
        if (chip->clk2)
        {
            chip->eg_timer[1] = chip->eg_timer[0];
            chip->eg_carry[1] = chip->eg_carry[0];
            chip->eg_sync_l[1] = chip->eg_sync_l[0];
            chip->eg_mask[1] = chip->eg_mask[0];
            chip->eg_timer_masked[1] = chip->eg_timer_masked[0];
            chip->eg_subcnt[1] = chip->eg_subcnt[0];
            chip->eg_subcnt_l[1] = chip->eg_subcnt_l[0];
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

        int state = 0;
        if (chip->eg_state[0][1] & 0x20000)
            state |= 1;
        if (chip->eg_state[1][1] & 0x20000)
            state |= 2;

        int dokon = state == eg_state_release && chip->keyon_comb;
        int rate_sel = dokon ? eg_state_attack : state;
        int rate = 0;
        int ksr;
        if (rate_sel == 0)
            rate |= chip->ar;
        if (rate_sel == 1)
            rate |= chip->dr;
        if (rate_sel == 3 || (rate_sel == 2 && !chip->egt))
            rate |= chip->rr;

        int sl = chip->sl;
        if (chip->sl == 15)
            sl |= 16;

        int ns = chip->reg_notesel ? (chip->fnum & 256) != 0 : (chip->fnum & 512) != 0;

        if (chip->ksr)
            ksr = (chip->block << 1) | ns;
        else
            ksr = chip->block >> 1;

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

        int level = 0;

        for (i = 0; i < 9; i++)
        {
            level |= ((chip->eg_level[i][1] >> 17) & 1) << i;
        }

        int slreach = (level >> 4) == sl;
        int zeroreach = level == 0;
        int silent = (level & 0x1f8) == 0x1f8;

        int nextstate = eg_state_attack;

        if (chip->reset1)
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
        int instantattack = dokon && maxrate;
        int mute = chip->reset1 || (state != eg_state_attack && silent && !dokon);

        int level2 = mute ? 0x1ff : (instantattack ? 0 : level);

        int add = 0;
        int addshift = 0;

        if (exponent)
            add |= (~level) >> 1;
        if (linear)
            add |= 4;

        if (step1)
            addshift |= add >> 2;
        if (step2)
            addshift |= add >> 1;
        if (step3)
            addshift |= add >> 0;

        int levelnext = level2 + addshift;

        static const int eg_ksltable[16] = {
            0, 32, 40, 45, 48, 51, 53, 55, 56, 58, 59, 60, 61, 62, 63, 64
        };

        int ksl;
        ksl = eg_ksltable[chip->fnum >> 6] ^ 127;

        ksl += ((chip->block ^ 7) + 1) << 3;
        if (ksl & 128)
            ksl = 0;
        else
            ksl = (ksl ^ 63) & 63;

        static int eg_kslshift[4] = {
            31, 1, 2, 0
        };

        ksl = (ksl << 2) >> eg_kslshift[chip->ksl];

        int ksltl = ksl + (chip->tl << 2);

        int tremolo;

        if (!chip->am)
            tremolo = 0;
        else if (chip->reg_dv)
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

        int totallevelclamp = (chip->reg_test & 1) != 0 ? 0 : (levelof ? 0x1ff : (totallevel & 0x1ff));


        chip->eg_dokon = dokon;

        chip->eg_state[0][0] = (chip->eg_state[0][1] << 1) | ((nextstate & 1) != 0);
        chip->eg_state[1][0] = (chip->eg_state[1][1] << 1) | ((nextstate & 2) != 0);

        for (i = 0; i < 9; i++)
        {
            chip->eg_level[i][0] = (chip->eg_level[i][1] << 1) | ((levelnext >> i) & 1);
        }
        chip->eg_out[0] = totallevelclamp;

        if (chip->fsm_out[9])
        {
            for (i = 0; i < 9; i++)
            {
                if (chip->eg_out[1] & (1 << i))
                    chip->dbg_serial[0] |= 1 << (17 - i);
            }
        }

        chip->eg_mute[0] = (chip->eg_mute[1] << 1) | mute;
    }
    if (chip->clk2)
    {
        chip->eg_state[0][1] = chip->eg_state[0][0];
        chip->eg_state[1][1] = chip->eg_state[1][0];

        for (i = 0; i < 9; i++)
        {
            chip->eg_level[i][1] = chip->eg_level[i][0];
        }
        chip->eg_out[1] = chip->eg_out[0];
        chip->eg_mute[1] = chip->eg_mute[0];
    }

    if (chip->clk1)
    {
        static const int pg_multi[16] = {
            1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30
        };
        int fnum = chip->fnum;
        int freq;
        int pg_add;
        int vib_sel1 = (chip->vib_cnt[1] & 3) == 2;
        int vib_sel2 = (chip->vib_cnt[1] & 1) == 1;
        int vib_sh0 = chip->reg_dv && chip->vib && vib_sel1;
        int vib_sh1 = (chip->reg_dv && chip->vib && vib_sel2)
            || (!chip->reg_dv && chip->vib && vib_sel1);
        int vib_sh2 = !chip->reg_dv && chip->vib && vib_sel2;
        int vib_sign = (chip->vib_cnt[1] & 4) != 0 && chip->vib;
        int vib_add = 0;
        int pg_out = 0;
        int phase;
        int noise_bit;
        if (vib_sh0)
            vib_add |= (chip->fnum >> 7) & 7;
        if (vib_sh1)
            vib_add |= (chip->fnum >> 8) & 3;
        if (vib_sh2)
            vib_add |= (chip->fnum >> 9) & 1;
        if (vib_sign)
        {
            vib_add ^= 1023;
        }
        fnum += vib_add;
        fnum += vib_sign;
        if (vib_sign)
            fnum &= 1023;

        freq = (fnum << chip->block) >> 1;

        pg_add = (freq * pg_multi[chip->multi]) >> 1;

        for (i = 0; i < 19; i++)
        {
            pg_out |= ((chip->pg_phase[i][1] >> 17) & 1) << i;
        }

        phase = ((chip->eg_dokon || (chip->reg_test & 4) != 0) ? 0 : pg_out) + pg_add;

        for (i = 0; i < 19; i++)
        {
            chip->pg_phase[i][0] = chip->pg_phase[i][1] << 1;
            chip->pg_phase[i][0] |= (phase >> i) & 1;
        }

        chip->dbg_serial[0] = chip->dbg_serial[1] >> 1;

        if (chip->fsm_out[9])
        {
            chip->dbg_serial[0] |= pg_out & 511;
        }

        noise_bit = ((chip->noise_lfsr[1] >> 22) ^ (chip->noise_lfsr[1] >> 8)) & 1;

        if ((chip->noise_lfsr[1] & 0x7fffff) == 0)
            noise_bit |= 1;

        noise_bit |= (chip->reg_test & 128) != 0;

        chip->noise_lfsr[0] = (chip->noise_lfsr[1] << 1) | noise_bit;
    }
    if (chip->clk2)
    {
        for (i = 0; i < 19; i++)
        {
            chip->pg_phase[i][1] = chip->pg_phase[i][0];
        }

        chip->noise_lfsr[1] = chip->noise_lfsr[0];

        chip->pg_out = 0;
        for (i = 0; i < 10; i++)
        {
            chip->pg_out |= ((chip->pg_phase[i+9][1] >> 17) & 1) << i;
        }

        chip->dbg_serial[1] = chip->dbg_serial[0];
    }

    {
        int hh = chip->fsm_out[4] && chip->rhythm;
        int sd = chip->fsm_out[7] && chip->rhythm;
        int tc = chip->fsm_out[8] && chip->rhythm;
        int rhy = (chip->fsm_out[4] || chip->fsm_out[7] || chip->fsm_out[8]) && chip->rhythm;
        if (chip->clk1)
            chip->hh_load = chip->fsm_out[4];
        if (!chip->hh_load && chip->fsm_out[5])
        {
            chip->hh_bit2 = (chip->pg_out >> 2) & 1;
            chip->hh_bit3 = (chip->pg_out >> 3) & 1;
            chip->hh_bit7 = (chip->pg_out >> 7) & 1;
            chip->hh_bit8 = (chip->pg_out >> 8) & 1;
        }
        if (chip->clk1)
            chip->tc_load = tc;
        if (!chip->tc_load && tc)
        {
            chip->tc_bit3 = (chip->pg_out >> 3) & 1;
            chip->tc_bit5 = (chip->pg_out >> 5) & 1;
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
                chip->pg_out_rhy |= chip->pg_out;
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
                chip->pg_out_rhy |= (rm_bit << 9) | 0x100;
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
            int phase = chip->pg_out_rhy + chip->op_mod[1];
            int sign = (phase & 512) != 0;
            int quarter = (phase & 256) != 0;
            phase &= 255;
            if (quarter)
                phase ^= 255;

            int ls = logsin[phase >> 1];
            if ((phase & 1) == 0)
                ls += logsin_d[phase >> 1];

            int att = chip->op_logsin[1] + (chip->eg_out[1] << 3);
            if (att & 4096)
                att = 4095;

            int pw = pow[(att >> 1) & 127];
            if ((att & 1) == 0)
                pw += pow_d[(att >> 1) & 127];

            int value = 0;

            if (chip->op_mute[1] & 2)
            {
                value = ((chip->op_pow[1] | 0x400) << 1) >> chip->op_shift[1];
            }

            if (chip->op_sign[1] & 2)
                value ^= 8191;

            int sign_wf = sign && chip->wf == 0;
            int mute_wf = !((chip->wf == 1 && sign) || (chip->wf == 3 && quarter));

            int fb1 = 0;
            int fb2 = 0;
            for (i = 0; i < 14; i++)
            {
                int j = i;
                if (i == 13)
                    j = 12;
                fb1 |= ((chip->op_fb[0][j][1] >> 5) & 1) << i;
                fb2 |= ((chip->op_fb[1][j][1] >> 5) & 1) << i;
            }
            int fb_sum = fb1 + fb2;
            fb_sum &= 16383;
            if (fb_sum & 8192)
                fb_sum |= ~8191;

            int mod = 0;

            if (chip->fsm_out[2] && !(chip->connect_l[1] & 2))
                mod |= value & 1023;
            if (chip->fsm_out[1])
            {
                if (chip->fb_l[1][1])
                {
                    mod |= (fb_sum >> (9 - chip->fb_l[1][1])) & 1023;
                }
            }

            chip->op_logsin[0] = ls;
            chip->op_shift[0] = (att >> 8) & 15;
            chip->op_pow[0] = pw;
            chip->op_mute[0] = (chip->op_mute[1] << 1) | mute_wf;
            chip->op_sign[0] = (chip->op_sign[1] << 1) | sign_wf;

            for (i = 0; i < 13; i++)
            {
                int bit;
                chip->op_fb[0][i][0] = chip->op_fb[0][i][1] << 1;
                if (chip->fsm_out[2])
                    bit = (value >> i) & 1;
                else
                    bit = (chip->op_fb[0][i][1] >> 8) & 1;
                chip->op_fb[0][i][0] |= bit;
                chip->op_fb[1][i][0] = chip->op_fb[1][i][1] << 1;
                if (chip->fsm_out[2])
                    bit = (chip->op_fb[0][i][1] >> 8) & 1;
                else
                    bit = (chip->op_fb[1][i][1] >> 8) & 1;
                chip->op_fb[1][i][0] |= bit;
            }
            chip->op_mod[0] = mod & 1023;

            chip->op_value = value;
        }
        if (chip->clk2)
        {
            chip->op_logsin[1] = chip->op_logsin[0];
            chip->op_shift[1] = chip->op_shift[0];
            chip->op_pow[1] = chip->op_pow[0];
            chip->op_mute[1] = chip->op_mute[0];
            chip->op_sign[1] = chip->op_sign[0];

            for (i = 0; i < 13; i++)
            {
                chip->op_fb[0][i][1] = chip->op_fb[0][i][0];
                chip->op_fb[1][i][1] = chip->op_fb[1][i][0];
            }
            chip->op_mod[1] = chip->op_mod[0];
        }
    }

    {
        int accm_out = chip->fsm_out[8] ? (chip->accm_value[1] & 0x7fff) : 0;
        if (chip->fsm_out[8] && !(chip->accm_value[1] & 0x20000))
            accm_out |= 0x8000;

        int top = (chip->accm_value[1] >> 15) & 7;

        int clamplow = top == 4 || top == 5 || top == 6;
        int clamphigh = top == 3 || top == 2 || top == 1;

        if (chip->clk1)
            chip->accm_load1_l = chip->fsm_out[8];
        chip->accm_load1 = !chip->accm_load1_l && chip->fsm_out[8];

        if (chip->accm_load1)
        {
            chip->accm_clamplow = clamplow;
            chip->accm_clamphigh = clamphigh;
            chip->accm_top = (accm_out >> 9) & 127;
        }

        if (chip->clk1)
        {
            int add = 0;
            int op_out = chip->op_value;
            if (op_out & 0x1000)
                op_out |= ~0xfff;
            if (!(chip->eg_mute[1] & 2) && chip->fsm_out[0])
                add = chip->fsm_out[11] ? (op_out * 2) : op_out;

            int value = chip->fsm_out[8] ? 0 : chip->accm_value[1];
            value += add;

            chip->op_value_debug = add;

            int sign = ((chip->accm_top & 64) != 0 && !chip->accm_clamplow) || chip->accm_clamphigh;

            int top_unsigned = chip->accm_top & 63;
            if (!sign)
                top_unsigned ^= 63;

            int shift = 0;

            if (top_unsigned & 32)
                shift |= 7;
            if ((top_unsigned & 48) == 16)
                shift |= 6;
            if ((top_unsigned & 56) == 8)
                shift |= 5;
            if ((top_unsigned & 60) == 4)
                shift |= 4;
            if ((top_unsigned & 62) == 2)
                shift |= 3;
            if (top_unsigned == 1)
                shift |= 2;
            if (top_unsigned == 0)
                shift |= 1;
            if (chip->accm_clamplow)
                shift |= 7;
            if (chip->accm_clamphigh)
                shift |= 7;

            int accm_bit = 0;

            if (chip->fsm_out[6])
                accm_bit |= sign;
            if (chip->accm_sel[1] & 1)
                accm_bit |= (shift & 1) != 0;
            if (chip->accm_sel[1] & 2)
                accm_bit |= (shift & 2) != 0;
            if (chip->accm_sel[1] & 4)
                accm_bit |= (shift & 4) != 0;

            if ((chip->accm_sel[1] & 7) == 0 && !chip->fsm_out[6])
            {
                if (top_unsigned & 32)
                    accm_bit |= (chip->accm_shifter[1] >> 6) & 1;
                if ((top_unsigned & 48) == 16)
                    accm_bit |= (chip->accm_shifter[1] >> 5) & 1;
                if ((top_unsigned & 56) == 8)
                    accm_bit |= (chip->accm_shifter[1] >> 4) & 1;
                if ((top_unsigned & 60) == 4)
                    accm_bit |= (chip->accm_shifter[1] >> 3) & 1;
                if ((top_unsigned & 62) == 2)
                    accm_bit |= (chip->accm_shifter[1] >> 2) & 1;
                if (top_unsigned == 1)
                    accm_bit |= (chip->accm_shifter[1] >> 1) & 1;
                if (top_unsigned == 0)
                    accm_bit |= chip->accm_shifter[1] & 1;
                if (chip->accm_clamphigh)
                    accm_bit |= 1;
                if (chip->accm_clamplow)
                    accm_bit = 0;
            }

            chip->accm_value[0] = value & 0x3ffff;
            chip->accm_shifter[0] = (chip->accm_shifter[1] >> 1) & 0x7fff;
            if (chip->fsm_out[8])
                chip->accm_shifter[0] |= accm_out;
            chip->accm_sel[0] = (chip->accm_sel[1] << 1) | chip->fsm_out[6];
            chip->accm_mo[0] = accm_bit;
        }
        if (chip->clk2)
        {
            chip->accm_value[1] = chip->accm_value[0];
            chip->accm_shifter[1] = chip->accm_shifter[0];
            chip->accm_sel[1] = chip->accm_sel[0];
            chip->accm_mo[1] = chip->accm_mo[0];
        }
    }

end:
    chip->o_sh = chip->fsm_out[3];
    chip->o_mo = chip->accm_mo[1];
    chip->o_irq_pull = irq;
    chip->o_sy = chip->clk1;

    if (chip->io_read0)
    {
        chip->io_data &= ~6;
        if (chip->reg_test & 64)
            chip->io_data |= chip->dbg_serial[1] & 1;
        if (irq)
            chip->io_data |= 128;
        if (chip->t1_status)
            chip->io_data |= 64;
        if (chip->t2_status)
            chip->io_data |= 32;
        if (chip->unk_status1)
            chip->io_data |= 16;
        if (chip->unk_status2)
            chip->io_data |= 8;
    }

    if (chip->io_dir)
    {
        chip->data_o = chip->io_data;
        chip->data_z = 0;
    }
    else
        chip->data_z = 1;
}
