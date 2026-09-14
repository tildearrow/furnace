/*
 * Copyright (C) 2025 nukeykt
 *
 * This file is part of YM2151-LLE.
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
 *  YM2151 and YM2164 emulator.
 *  Thanks:
 *      John McMaster (siliconpr0n.org):
 *          YM2151 decap
 *      gtr3qq (https://github.com/gtr3qq):
 *          YM2164 decap
 *
 */
#pragma once
#include <stdint.h>

typedef struct {
    int ym2164;
    int clk;
    int ic; // neg
    int cs; // neg
    int wr; // neg
    int rd; // neg
    int a0;
    int data;
} fmopm_input_t;


typedef struct {
    fmopm_input_t input;

    int clk1;
    int clk2;
    int oclk1;
    int oclk2;

    int ic_latch[2];
    int ic_latch2[2];
    int ic;

    int write0_trig;
    int write0_l[3];
    int write1_trig;
    int write1_l[3];

    int data1;
    int data2;

    int fsm_cnt[2];
    int fsm_out[18];
    int fsm_sh1_l[2];
    int fsm_sh2_l[2];
    int fsm_op_inc;
    int fsm_op_rst;
    int fsm_op_cnt[2];
    int fsm_acc_sync;
    int fsm_sync1[2];
    int fsm_cycle31;
    int fsm_cycle0;
    int fsm_cycle0_l;
    int fsm_cycle1;
    int fsm_cycle1_l;
    int fsm_reg_sync[2];
    int fsm_lfo_mul[2];

    int reg_write_01[2];
    int reg_write_08[2];
    int reg_write_0f[2];
    int reg_write_10[2];
    int reg_write_11[2];
    int reg_write_12[2];
    int reg_write_14[2];
    int reg_write_18[2];
    int reg_write_19[2];
    int reg_write_1b[2];
    int reg_test[2];
    int reg_timer_a[2];
    int reg_timer_b[2];
    int reg_timer_a_load[2];
    int reg_timer_b_load[2];
    int reg_timer_a_load_l;
    int reg_timer_b_load_l;
    int reg_timer_a_irq[2];
    int reg_timer_b_irq[2];
    int reg_csm_en[2];
    int reg_noise_en[2];
    int reg_noise_freq[2];
    int reg_kon_channel[2];
    int reg_kon_operator[2];
    int reg_lfo_freq[2];
    int reg_lfo_amd[2];
    int reg_lfo_pmd[2];
    int reg_lfo_freq_write;
    int reg_lfo_wave[2];
    int reg_ct[2];
    int reg_address[2];
    int reg_address_valid[2];
    int reg_data[2];
    int reg_data_valid[2];
    int reg_counter[2];
    int reg_match00;
    int reg_match20;
    int reg_match20_l[2];
    int reg_match28;
    int reg_match28_l[2];
    int reg_match30;
    int reg_match30_l[2];
    int reg_match38;
    int reg_match40;
    int reg_match60;
    int reg_match80;
    int reg_matcha0;
    int reg_matchc0;
    int reg_matche0;
    uint8_t reg_con_fb_rl[2][8];
    uint8_t reg_kc[2][8];
    uint8_t reg_kf[2][8];
    uint8_t reg_ams_pms[2][8];
    uint8_t reg_mul_dt1[2][32];
    uint8_t reg_tl[2][32];
    uint8_t reg_ar_ks[2][32];
    uint8_t reg_d1r_am[2][32];
    uint8_t reg_d2r_dt2[2][32];
    uint8_t reg_rr_d1l[2][32];
    int reg_div4[2];
    int reg_ch_sel[2];
    uint64_t reg_op_sel[2];
    int reg_ch_sync;
    uint64_t reg_ch_cell[8];
    uint64_t reg_ch_bus;
    uint64_t reg_ch_latch;
    uint64_t reg_ch_in[2];
    uint64_t reg_op_cell[32];
    uint64_t reg_op_bus;
    uint64_t reg_op_latch;
    uint64_t reg_op_in[2];
    int reg_ramp_sync;
    int reg_ramp_step;
    uint8_t reg_ramp_cnt[2][8];
    int reg_tl_latch[3];
    uint16_t reg_tl_value[2][31];
    uint16_t reg_tl_value_l;
    uint16_t reg_tl_value_sum;
    int reg_tl_add1;
    int reg_tl_add2;
    int reg_rl[3];
    uint8_t reg_fb[2][4];
    int reg_con;
    int reg_con_l;
    int reg_pms;
    uint8_t reg_dt2[2][27];

    int reg_kon_cnt[2];
    int reg_kon_match;
    int reg_kon[4][2];

    int timer_a_cnt[2];
    int timer_a_inc;
    int timer_a_of;
    int timer_a_en[2];
    int timer_a_load;
    int timer_a_reset[2];
    int timer_a_set;
    int timer_a_status[2];
    int timer_b_cnt[2];
    int timer_b_inc;
    int timer_b_of;
    int timer_b_en[2];
    int timer_b_load;
    int timer_b_reset[2];
    int timer_b_set;
    int timer_b_status[2];
    int timer_b_subcnt[2];
    int timer_b_subcnt_of;
    int timer_irq;

    int busy_cnt[2];
    int busy_cnt_en[2];

    int lfo_sync[2];
    int lfo_sync2[2];
    int lfo_cnt1[2];
    int lfo_cnt1_h[2];
    int lfo_cnt1_of_l;
    int lfo_cnt1_of_h;
    int lfo_cnt1_load_val_hi;
    int lfo_cnt1_load[4];
    int lfo_subcnt[2];
    int lfo_subcnt_of[4];
    int lfo_cnt2_inc;
    int lfo_cnt2[2];
    int lfo_cnt2_of[2];
    int lfo_test;
    int lfo_inc[2];
    int lfo_inc_lock;
    int lfo_depth;
    int lfo_bcnt[2];
    int lfo_bcnt_rst;
    int lfo_sum_c_out;
    int lfo_sum_c_in;
    int lfo_out_shifter[2];
    int lfo_shifter[2];
    int lfo_wave1; // square
    int lfo_wave2; // triangle
    int lfo_wave3; // noise
    int lfo_sel;
    int lfo_sum2_c_out[2];
    int lfo_sign_saw;
    int lfo_sign_trig;
    int lfo_cnt1_of_h_lock;
    int lfo_cnt1_of_h_latch;
    int lfo_premul[2];
    int lfo_am[2];
    int lfo_pm[2];

    int freq_pms;
    int freq_lfo_pm;
    int freq_lfo_sign[5];
    int freq_kc_lfo[5];
    int freq_kc_lfo_l_of[2];
    int freq_kc_lfo_of[4];
    int freq_kc_lfo_of2[2];
    int freq_kc_corr_sub[2];
    int freq_kc_cliplow;
    int freq_kc_cliphigh;
    int freq_dt2[3];
    int freq_kc_dt_lo;
    int freq_kc_dt_hi;
    int freq_kc_clipped_hi[2];
    int freq_kc_dt[5];
    int freq_kc_dt_c[2];
    int freq_kc_dt_of[3];
    int freq_kcode[4];
    int freq_freq_frac[3];
    int freq_basefreq[3];
    int freq_slope;
    int freq_slopetype;
    int freq_freq_lerp_a;
    int freq_freq_lerp_b;
    int freq_freq_lerp_d_e;
    int freq_freq_lerp;
    int freq_fnum[3];

    int pg_block[2];
    int pg_freq;
    int pg_dt_multi;
    int pg_dt_add;
    int pg_freqdt[4];
    int pg_multi[7];
    int pg_add[10];
    int pg_reset[5];
    int pg_phase[2][31];
    int pg_phase2[2];
    int pg_out;
    int pg_dbgsync;
    int pg_dbg[2];
    int dt_add1;
    int dt_add2;
    int dt_sum;
    int dt_blockmax[2];
    int dt_note[2];
    int dt_sign[2];

    int noise_cnt[2];
    int noise_cnt_inc;
    int noise_cnt_match[3];
    int noise_lfsr[2];
    int noise_bit[2];

    int eg_ic[2];
    int eg_timerlock_l;
    int eg_rate_sel[3];
    int eg_half;
    int eg_rate;
    int eg_zerorate[2];
    int eg_timer_carry[2];
    int eg_timer_lo;
    int eg_timer_lo_lock;
    int eg_shift_lock;
    int eg_timer[2];
    int eg_timer_masked[2];
    int eg_masking[2];
    int eg_clock[2];
    int eg_sync[2];
    int eg_sync2[2];
    int eg_subcnt[2];
    int eg_subcnt_reset;
    int eg_csm_kon[3];
    uint64_t eg_keyon[2];
    int eg_kon1;
    int eg_kon2;
    int eg_state[2][2];
    int eg_off;
    int eg_zero;
    int eg_slreach;
    int eg_linear;
    int eg_exponent;
    int eg_maxrate[2];
    int eg_ks;
    int eg_rateks[2];
    int eg_rateks_l[2];
    int eg_rate12;
    int eg_rate13;
    int eg_rate14;
    int eg_rate15;
    int eg_stephi;
    int eg_zerorate2[2];
    int eg_inc1_c1;
    int eg_inc2_c1;
    int eg_inc2_c2;
    int eg_inc3_c1;
    int eg_inc3_c2;
    int eg_inc4_c1;
    int eg_inc4_c2;
    int eg_inc1;
    int eg_inc2;
    int eg_inc3;
    int eg_inc4;
    int eg_rate_lo[2];
    int eg_shift_sum[2];
    uint16_t eg_level[2][31];
    int eg_sl[4];
    int eg_instantattack;
    int eg_mute;
    int eg_inc;
    int eg_nextlevel[2];
    int eg_ams;
    int eg_lfo;
    int eg_lfo_am;
    int eg_level_lfo[3];
    int eg_tl[5];
    int eg_level_tl[2];
    int eg_test[3];
    int eg_out;
    int eg_dbg[2];
    int eg_pgreset[2];

    int op_phase;
    int op_mod_in[2];
    int op_phase2[2];
    int op_sign[2];
    int op_phase_index[2];
    int op_logsin_base[2];
    int op_logsin_delta[2];
    int op_logsin[2];
    int op_env;
    int op_atten[4];
    int op_pow_base[2];
    int op_pow_delta[2];
    int op_pow[2];
    int op_shift[4];
    int op_pow_shift[2];
    uint16_t op_value[2][5];
    uint16_t op_op1[2][8];
    uint16_t op_op1_old[2][8];
    uint16_t op_op2[2][8];
    int op_mod1;
    int op_mod2;
    int op_modsum[3];
    int op_update_op1[2];
    int op_update_op2[2];
    int op_mix;
    int op_m1_op1[2];
    int op_m1_op2[2];
    int op_m1_prev[2];
    int op_m2_op1[2];
    int op_m2_prev[2];
    int op_test_bit;

    int accm_sync_l;
    int accm_noise_sync[2];
    int accm_clear_l[2];
    int accm_clear_r[2];
    int accm_input;
    int accm_noise_bit[4];
    int accm_noise_bit_l;
    int accm_env_active_l;
    int accm_env_active[2];
    int accm_env_rst[2];
    int accm_noise_env[2];
    int accm_noise_env_l;
    int accm_mix_l;
    int accm_mix_r;
    int accm_input_l;
    int accm_input_r;
    int accm_l[3];
    int accm_r[3];
    int accm_l_shifter[2];
    int accm_r_shifter[2];
    int accm_l_shifter_o[2];
    int accm_r_shifter_o[2];
    int accm_l_top[2];
    int accm_r_top[2];
    int accm_l_clamp_lo;
    int accm_l_clamp_hi;
    int accm_r_clamp_lo;
    int accm_r_clamp_hi;
    int accm_l_bit[3];
    int accm_r_bit[3];
    int accm_lr_sel[3];
    int accm_shifter[2];
    int accm_sync2[2];
    int accm_lrbit;
    int accm_topbits;
    int accm_load;
    int accm_sign;
    int accm_shift;
    int accm_bit[3];

    int read_dbg;
    int read_dbg_data;
    int read_bus;
    int read_bus_latch;
    int ct_test;

    int o_sy;
    int o_sh1;
    int o_sh2;
    int o_so;
    int o_data;
    int o_data_z;
    int o_ct1;
    int o_ct2;
    int o_irq_pull;

    // tildearrow: per-chan osc
    int ch_out[8];
} fmopm_t;

// tildearrow: expose this function
void FMOPM_Clock(fmopm_t* chip, int clk);