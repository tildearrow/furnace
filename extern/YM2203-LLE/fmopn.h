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
#pragma once

typedef struct {
	int clk;
    int ic; // neg
    int cs; // neg
    int wr; // neg
    int rd; // neg
    int a0;
    int data;
    int gpio_a;
    int gpio_b;
} fmopn_input_t;

typedef struct {
    fmopn_input_t input;

    int mclk1;
    int mclk2;
    int clk1;
    int clk2;
    int aclk1;
    int aclk2;

    int o_clk1;
    int o_clk2;
    int o_aclk1;
    int o_aclk2;


    int data_l;
    int data_bus1;
    int data_bus2;

    int write0_trig;
    int write0_l[6];
    int write1_trig;
    int write1_l[6];
    int writep_trig;
    int writep_l[3];

    int ps1[2];
    int ps1_l0;
    int ps1_l1;
    int ps2[2];

    int ic_latch[2];

    int prescaler_sel[2];

    int write_fm_address[2];
    int write_fm_data[2];
    
    int fm_address[2];
    int fm_data[2];

    int reg_cnt1[2];
    int reg_cnt2[2];

    int reg_key_cnt1[2];
    int reg_key_cnt2[2];

    int reg_test_21[2];
    int reg_timer_a[2];
    int reg_timer_b[2];
    int reg_ch3[2];
    int reg_timer_a_load[2];
    int reg_timer_b_load[2];
    int reg_timer_a_enable[2];
    int reg_timer_b_enable[2];
    int reg_timer_a_reset[2];
    int reg_timer_b_reset[2];
    int reg_kon_operator[2];
    int reg_kon_channel[2];
    int reg_kon_match;
    int reg_ch3_sel;

    int timer_a_status[2];
    int timer_b_status[2];
    int timer_a_cnt[2];
    int timer_a_of[2];
    int timer_a_load;
    int timer_a_reg_load;
    int reg_sync_timer_load;
    int reg_sync_timer_l[2];
    int reg_sync_timer;
    int timer_a_reg_load_l[2];
    int timer_b_subcnt[2];
    int timer_b_subcnt_of[2];
    int timer_b_cnt[2];
    int timer_b_reg_load;
    int timer_b_load;
    int timer_b_of[2];
    int timer_b_reg_load_l[2];

    int ch3_en;
    int ch3_csm;
    int ch3_csm_load;

    int addr_21[2];
    int addr_24[2];
    int addr_25[2];
    int addr_26[2];
    int addr_27[2];
    int addr_28[2];

    int reg_kon[4][2];

    int fm_is30;
    int fm_is40;
    int fm_is50;
    int fm_is60;
    int fm_is70;
    int fm_is80;
    int fm_is90;
    int fm_isa0;
    int fm_isa4;
    int fm_isa8;
    int fm_isac;
    int fm_isb0;

    int reg_cnt_sync;

    int busy_cnt[2];
    int busy_cnt_en[2];

    int fsm_cnt1[2];
    int fsm_cnt2[2];
    int fsm_out[14];
    int fsm_sh;
    int fsm_sh_l[2];
    int fsm_sel_11[2];
    int fsm_acc_sync[2];
    int fsm_ch3_sync[2];

    int fsm_op1_sel_l;
    int fsm_op2_sel_l;
    int fsm_op3_sel_l;
    int fsm_op4_sel_l;
    int fsm_connect;
    int alg_mod_op1_0_l;
    int alg_mod_op1_1_l;
    int alg_mod_op2_l;
    int alg_mod_prev_0_l;
    int alg_mod_prev_1_l;
    int alg_output_l;
    int alg_do_fb[2];
    int alg_load_fb;

    unsigned short reg_freq[2][3];
    unsigned short reg_freq_3ch[2][3];
    unsigned char reg_connect_fb[2][3];
    unsigned char op_multi_dt[2][12];
    unsigned char op_tl[2][12];
    unsigned char op_ar_ks[2][12];
    unsigned char op_dr_a[2][12];
    unsigned char op_sr[2][12];
    unsigned char op_rr_sl[2][12];
    unsigned char op_ssg[2][12];
    int reg_a4[2];
    int reg_ac[2];

    int ch3_sel[2];

    int fnum[5];
    int kcode[4];

    int pg_block;
    int pg_freq;
    int pg_dt_multi;
    int pg_dt_add;
    int pg_freqdt[2];
    int pg_multi[5];
    int pg_add[6];
    int pg_reset[4];
    int pg_phase[2][11];
    int pg_phase2[2];
    int pg_out;
    int pg_dbgsync;
    int pg_dbg[2];
    int dt_add1;
    int dt_add2;
    int dt_enable[2];
    int dt_sum;
    int dt_blockmax[2];
    int dt_note[2];
    int dt_sign[2];

    int eg_sync;
    int eg_prescaler[2];
    int eg_prescaler_clock_l[2];
    int eg_ic[2];

    int eg_step[3];
    int eg_timer_step[2];

    int eg_timer[2];
    int eg_timer_sum[2];
    int eg_timer_carry[2];
    int eg_timer_mask[2];
    int eg_timer_masked[2];
    int eg_timer_low_lock;
    int eg_shift_lock;

    int eg_rate_sel;
    int eg_rate_ar;
    int eg_rate_dr;
    int eg_rate_sr;
    int eg_rate_rr;
    int eg_rate;
    int eg_rate_nonzero[3];
    unsigned char eg_state[2][11];
    int eg_ks;
    int eg_ksv;
    int eg_rate2;
    int eg_ratenz;
    int eg_rate12;
    int eg_rate13;
    int eg_rate14;
    int eg_rate15;
    int eg_rate_low;
    int eg_rate_slow;
    int eg_rate_sum;
    int eg_maxrate[2];
    int eg_inc2;
    int eg_incsh0[2];
    int eg_incsh1[2];
    int eg_incsh2[2];
    int eg_incsh3[2];
    int eg_output;
    unsigned short eg_level[2][10];
    int eg_ssg_inv;
    int eg_ssg_enable[2];
    int eg_ssg_sign[2];
    int eg_ssg_holdup[2];
    int eg_ssg_pgreset[2];
    int eg_ssg_dir[2];
    int eg_ssg_egrepeat[2];
    int eg_key[2];
    int eg_kon_latch[2];
    int eg_kon_event;
    int eg_pg_reset[2];
    int eg_sl[2][2];
    int eg_tl[3][2];
    int eg_level_ssg[2];
    int eg_slreach;
    int eg_level_l[2];
    int eg_zeroreach;
    int eg_off;
    int eg_nextstate;
    int eg_state_l;
    int eg_mute;
    int eg_exp;
    int eg_linear;
    int eg_inc_total;
    int eg_nextlevel[2];
    int eg_istantattack;
    int eg_kon_csm[2];
    int eg_output2;
    int eg_csm_tl;
    int eg_ch3_l[2];
    int eg_out;
    int eg_dbg_sync;
    int eg_debug[2];

    int op_phase1;
    int op_phase2;
    int op_phase_index;
    int op_sign[2];
    int op_logsin_base;
    int op_logsin_delta;
    int op_eglevel;
    int op_att;
    int op_shift[2];
    int op_pow_base;
    int op_pow_delta;
    int op_pow;
    int op_output[4];
    unsigned short op_op1_0[2][3];
    unsigned short op_op1_1[2][3];
    unsigned short op_op2[2][3];
    int op_loadfb;
    int op_loadop2;
    int op_mod_op1_0;
    int op_mod_op1_1;
    int op_mod_op2;
    int op_mod_prev_0;
    int op_mod_prev_1;
    int op_mod1;
    int op_mod2;
    int op_mod_sum;
    int op_do_fb;
    int op_fb;

    int ssg_prescaler1[2];
    int ssg_prescaler2[2];
    int ssg_div1[2];
    int ssg_div2[2];
    int ssg_clk;
    int ssg_clk1;
    int ssg_clk2;
    int ssg_ssg_addr;
    int ssg_address;
    int ssg_freq_a[2];
    int ssg_freq_b[2];
    int ssg_freq_c[2];
    int ssg_noise;
    int ssg_mode;
    int ssg_level_a;
    int ssg_level_b;
    int ssg_level_c;
    int ssg_env[2];
    int ssg_envmode;
    int ssg_envadd;
    int ssg_envcnt[2];
    int ssg_dir[2];
    int ssg_hold[2];
    int ssg_t2[2];
    int ssg_egtrig;
    int ssg_egtrig_s;
    int ssg_egtrig_rst;
    int ssg_eg_sel_l;
    int ssg_sel[2];
    int ssg_sel_freq;
    int ssg_freq_cnt[8];
    int ssg_cnt_of[2];
    int ssg_sign[2];
    int ssg_sign_toggle;
    int ssg_cnt_of_l;
    int ssg_freq_cnt2[8];
    int ssg_sel_freq_l;
    int ssg_cnt2_add;
    int ssg_sel_eg_l[2];
    int ssg_ch_of;
    int ssg_cnt_reload;
    int ssg_eg_of[2];
    int ssg_fr_rst_l;
    int ssg_noise_add;
    int ssg_noise_cnt[2];
    int ssg_noise_of_low;
    int ssg_noise_of;
    int ssg_noise_step;
    int ssg_noise_lfsr[2];
    int ssg_noise_bit;


    int ac_fm_output;
    int ac_accum[2];
    int ac_accum_l;
    int ac_shifter1[2];
    int ac_sync[2];
    int ac_sync2[2];
    int ac_clip_h;
    int ac_clip_l;
    int ac_shifter2[2];
    int ac_hi_bits;
    int ac_exp;
    int ac_sign;
    int ac_out[2];
    int ac_load2_l;

    int read_test;
    int read_op;

    int read_data;

    int o_sh;
    int o_opo;
    int o_sy;
    float o_analog_a;
    float o_analog_b;
    float o_analog_c;
    int o_gpio_a_d;
    int o_gpio_a;
    int o_gpio_b_d;
    int o_gpio_b;
    int o_data;
    int o_irq_pull;


    short dbg_out;
} fmopn_t;

void FMOPN_Clock(fmopn_t* chip, int clk);
