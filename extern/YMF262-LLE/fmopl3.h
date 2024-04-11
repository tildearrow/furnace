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

#pragma once

#include <stdint.h>

typedef struct
{
    int mclk;
    int address;
    int data_i;
    int ic;
    int cs;
    int rd;
    int wr;
} fmopl3_input_t;

typedef struct
{
    fmopl3_input_t input;

    int mclk1;
    int mclk2;
    int aclk1;
    int aclk2;
    int clk1;
    int clk2;
    int rclk1;
    int rclk2;

    int o_clk1;
    int o_clk2;
    int o_rclk1;
    int o_rclk2;
    int o_wrcheck;
    int o_data_latch;
    int o_bank_latch;
    int o_reset0;
    int o_ra_w1_l1;

    int prescaler1_reset[2];
    int prescaler1_cnt[2];

    int prescaler2_reset_l[2];
    int prescaler2_cnt[2];
    int prescaler2_reset;
    int prescaler2_l1[2];
    int prescaler2_l2;
    int prescaler2_l3[2];
    int prescaler2_l4;
    int prescaler2_l5[2];
    int prescaler2_l6[2];
    int prescaler2_l7;

    int fsm_cnt1[2];
    int fsm_cnt2[2];
    int fsm_cnt3[2];
    int fsm_cnt;

    int fsm_reset_l[2];
    int fsm_out[17];
    int fsm_l1[2];
    int fsm_l2[2];
    int fsm_l3[2];
    int fsm_l4[2];
    int fsm_l5[2];
    int fsm_l6[2];
    int fsm_l7[2];
    int fsm_l8[2];
    int fsm_l9[2];
    int fsm_l10[2];

    int ic_latch[2];

    int io_rd;
    int io_wr;
    int io_cs;
    int io_a0;
    int io_a1;

    int io_read;
    int io_write;
    int io_write0;
    int io_write1;
    int io_bank;

    int data_latch;
    int bank_latch;
    int bank_masked;

    int reg_sel1;
    int reg_sel2;
    int reg_sel3;
    int reg_sel4;
    int reg_sel5;
    int reg_sel8;
    int reg_selbd;

    int reg_test0;
    int reg_timer1;
    int reg_timer2;
    int reg_notesel;
    int rhythm;
    int reg_rh_kon;
    int reg_da;
    int reg_dv;

    int reg_test1;
    int reg_new;
    int reg_4op;

    int reg_t1_mask;
    int reg_t2_mask;
    int reg_t1_start;
    int reg_t2_start;

    int lfo_cnt[2];
    int vib_cnt[2];
    int t1_step;
    int t2_step;
    int am_step;
    int vib_step;

    int rh_sel0;
    int rh_sel[2];

    int keyon_comb;

    int ra_address_latch;
    int ra_address_good;
    int ra_data_latch;
    int ra_cnt1[2];
    int ra_cnt2[2];
    int ra_cnt3[2];
    int ra_cnt4[2];
    int ra_cnt;
    int ra_rst_l[2];
    int ra_w1_l1;
    int ra_w1_l2;
    int ra_write;
    int ra_write_a;

    int ra_multi[36];
    int ra_ksr[36];
    int ra_egt[36];
    int ra_am[36];
    int ra_vib[36];
    int ra_tl[36];
    int ra_ksl[36];
    int ra_ar[36];
    int ra_dr[36];
    int ra_sl[36];
    int ra_rr[36];
    int ra_wf[36];
    int ra_fnum[18];
    int ra_block[18];
    int ra_keyon[18];
    int ra_connect[18];
    int ra_fb[18];
    int ra_pan[18];
    int ra_connect_pair[18];
    int multi[2];
    int ksr[2];
    int egt[2];
    int am[2];
    int vib[2];
    int tl[2];
    int ksl[2];
    int ar[2];
    int dr[2];
    int sl[2];
    int rr[2];
    int wf[2];
    int fnum[2];
    int block[2];
    int keyon[2];
    int connect[2];
    int fb[2];
    int pan[2];
    int connect_pair[2];

    int64_t ra_dbg1[2];
    int ra_dbg2[2];
    int ra_dbg_load[2];

    int fb_l[2][2];
    int pan_l[2][2];

    int write0_sr;
    int write0_l[4];
    int write0;

    int write1_sr;
    int write1_l[4];
    int write1;

    int connect_l[2];
    int connect_pair_l[2];

    int t1_cnt[2];
    int t2_cnt[2];
    int t1_of[2];
    int t2_of[2];
    int t1_status;
    int t2_status;
    int timer_st_load_l;
    int timer_st_load;
    int t1_start;
    int t2_start;
    int t1_start_l[2];
    int t2_start_l[2];

    int reset0;
    int reset1;

    int pg_phase_o[4];
    int pg_dbg[2];
    int pg_dbg_load_l[2];
    int noise_lfsr[2];
    int pg_index[2];
    int pg_cells[36];
    int pg_out_rhy;

    int trem_load_l;
    int trem_load;
    int trem_st_load_l;
    int trem_st_load;
    int trem_carry[2];
    int trem_value[2];
    int trem_dir[2];
    int trem_step;
    int trem_out;
    int trem_of[2];

    int eg_load_l1[2];
    int eg_load_l;
    int eg_load;

    int64_t eg_timer_masked[2];
    int eg_carry[2];
    int eg_mask[2];
    int eg_subcnt[2];
    int eg_subcnt_l[2];
    int eg_sync_l[2];
    int eg_timer_low;
    int eg_shift;
    int eg_timer_dbg[2];

    int eg_timer_i;
    int eg_timer_o[4];
    int eg_state_o[4];
    int eg_level_o[4];
    int eg_index[2];
    int eg_cells[36];

    int eg_out[2];
    int eg_dbg[2];
    int eg_dbg_load_l[2];

    int hh_load;
    int tc_load;
    int hh_bit2;
    int hh_bit3;
    int hh_bit7;
    int hh_bit8;
    int tc_bit3;
    int tc_bit5;

    int op_logsin[2];
    int op_saw[2];
    int op_saw_phase[2];
    int op_shift[2];
    int op_pow[2];
    int op_mute[2];
    int op_sign[2];
    int op_fb[4][13][2];
    int op_mod[2];

    int op_value;
    int op_value_debug;

    int accm_a[2];
    int accm_b[2];
    int accm_c[2];
    int accm_d[2];
    int accm_shift_a[2];
    int accm_shift_b[2];
    int accm_shift_c[2];
    int accm_shift_d[2];
    int accm_load_ac_l;
    int accm_load_ac;
    int accm_load_bd_l;
    int accm_load_bd;
    int accm_a_of;
    int accm_a_sign;
    int accm_b_of;
    int accm_b_sign;
    int accm_c_of;
    int accm_c_sign;
    int accm_d_of;
    int accm_d_sign;

    int o_doab;
    int o_docd;
    int o_sy;
    int o_smpac;
    int o_smpbd;
    int o_irq_pull;
    int o_test;

    int data_o;
    int data_z;
} fmopl3_t;

// modification
void FMOPL3_Clock(fmopl3_t *chip);
