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

#pragma once

typedef struct
{
    int mclk;
    int address;
    int data_i;
    int ic;
    int cs;
    int rd;
    int wr;
} fmopl2_input_t;

typedef struct
{
    fmopl2_input_t input;

    int mclk1;
    int mclk2;
    int clk1;
    int clk2;

    int prescaler_reset_l[2];
    int prescaler_cnt[2];
    int prescaler_l1[2];
    int prescaler_l2[2];

    int reset1;

    int fsm_reset_l[2];
    int fsm_reset; // wire
    int fsm_cnt1[2];
    int fsm_cnt2[2];
    int fsm_cnt1_of; // wire
    int fsm_cnt2_of; // wire
    int fsm_sel[13];
    int fsm_cnt; // wire
    int fsm_ch_out;
    int fsm_do_fb;
    int fsm_load_fb;
    int fsm_l1[2];
    int fsm_l2[2];
    int fsm_l3[2];
    int fsm_l4[2];
    int fsm_l5[2];
    int fsm_l6[2];
    int fsm_out[16];

    int io_rd;
    int io_wr;
    int io_cs;
    int io_a0;

    int io_read0;
    int io_read1;
    int io_write;
    int io_write0;
    int io_write1;
    int io_dir;
    int io_data;

    int data_latch;

    int write0;
    int write0_sr;
    int write0_latch[6];
    int write1;
    int write1_sr;
    int write1_latch[6];

    int reg_sel1;
    int reg_sel2;
    int reg_sel3;
    int reg_sel4;
    int reg_sel8;
    int reg_selbd;
    int reg_test;
    int reg_timer1;
    int reg_timer2;
    int reg_notesel;
    int reg_csm;
    int reg_da;
    int reg_dv;
    int rhythm;
    int reg_rh_kon;
    int reg_sel4_wr; // wire
    int reg_sel4_rst; // wire
    int reg_t1_mask;
    int reg_t2_mask;
    int reg_t1_start;
    int reg_t2_start;
    int reg_mode_b3;
    int reg_mode_b4;

    int t1_cnt[2];
    int t2_cnt[2];
    int t1_of[2];
    int t2_of[2];
    int t1_status;
    int t2_status;
    int unk_status1;
    int unk_status2;
    int timer_st_load_l;
    int timer_st_load;
    int t1_start;
    int t1_start_l[2];
    int t2_start_l[2];
    int t1_load; // wire
    int csm_load_l;
    int csm_load;
    int csm_kon;
    int rh_sel0;
    int rh_sel[2];

    int keyon_comb;
    int address;
    int address_valid;
    int address_valid_l[2];
    int address_valid2;
    int data;
    int slot_cnt1[2];
    int slot_cnt2[2];
    int slot_cnt;
    int sel_ch;

    int ch_fnum[10][2];
    int ch_block[3][2];
    int ch_keyon[2];
    int ch_connect[2];
    int ch_fb[3][2];
    int op_multi[4][2];
    int op_ksr[2];
    int op_egt[2];
    int op_vib[2];
    int op_am[2];
    int op_tl[6][2];
    int op_ksl[2][2];
    int op_ar[4][2];
    int op_dr[4][2];
    int op_sl[4][2];
    int op_rr[4][2];
    int op_wf[2][2];
    int op_mod[2];
    int op_value; // wire

    int eg_load1_l;
    int eg_load1;
    int eg_load2_l;
    int eg_load2;
    int eg_load3_l;
    int eg_load3;

    int trem_carry[2];
    int trem_value[2];
    int trem_dir[2];
    int trem_step;
    int trem_out;
    int trem_of[2];

    int eg_timer[2];
    int eg_timer_masked[2];
    int eg_carry[2];
    int eg_mask[2];
    int eg_subcnt[2];
    int eg_subcnt_l[2];
    int eg_sync_l[2];
    int eg_timer_low;
    int eg_shift;
    int eg_state[2][2];
    int eg_level[9][2];
    int eg_out[2];
    int eg_dokon; // wire
    int eg_mute[2];

    int block;
    int fnum;
    int keyon;
    int connect;
    int connect_l[2];
    int fb;
    int fb_l[2][2];
    int multi;
    int ksr;
    int egt;
    int vib;
    int am;
    int tl;
    int ksl;
    int ar;
    int dr;
    int sl;
    int rr;
    int wf;

    int lfo_cnt[2];
    int t1_step; // wire
    int t2_step; // wire
    int am_step; // wire
    int vib_step; // wire
    int vib_cnt[2];
    int pg_phase[19][2];
    int dbg_serial[2];

    int noise_lfsr[2];

    int hh_load;
    int tc_load;
    int hh_bit2;
    int hh_bit3;
    int hh_bit7;
    int hh_bit8;
    int tc_bit3;
    int tc_bit5;
    int op_logsin[2];
    int op_shift[2];
    int op_pow[2];
    int op_mute[2];
    int op_sign[2];
    int op_fb[2][13][2];

    int pg_out; // wire
    int pg_out_rhy; // wire

    int accm_value[2];
    int accm_shifter[2];
    int accm_load1_l;
    int accm_load1;
    int accm_clamplow;
    int accm_clamphigh;
    int accm_top;
    int accm_sel[2];
    int accm_mo[2];

    int o_sh;
    int o_mo;
    int o_irq_pull;
    int o_sy;

    int data_o;
    int data_z;

    int o_clk1;
    int o_clk2;
    int o_reset1;
    int o_write0;
    int o_write1;
    int o_data_latch;

} fmopl2_t;

// modification
void FMOPL2_Clock(fmopl2_t *chip);
