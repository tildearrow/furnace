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
// #define FMOPNA_YM2608
// #define FMOPNA_YM2610
// #define FMOPNA_YM2612

#if !defined(FMOPNA_YM2608) && !defined(FMOPNA_YM2610) && !defined(FMOPNA_YM2612)
#error no chip type defined
#endif

typedef struct {
    int clk;
    int ic; // neg
    int cs; // neg
    int wr; // neg
    int rd; // neg
    int a0;
    int a1;
    int data;
    int test; // set to 1
#ifdef FMOPNA_YM2608
    int gpio_a;
    int gpio_b;
    int dt0;
    int dm;
    int ad;
    int da;
#endif
#ifdef FMOPNA_YM2610
    int rad;
    int pad;
    int ym2610b;
#endif
}
#ifdef FMOPNA_YM2608
fmopna_input_t;
#elif defined (FMOPNA_YM2610)
fmopna_2610_input_t;
#else
fmopna_2612_input_t;
#endif

typedef struct {

#ifdef FMOPNA_YM2608
    fmopna_input_t input;
#elif defined (FMOPNA_YM2610)
    fmopna_2610_input_t input;
#else
    fmopna_2612_input_t input;
#endif

    int ic;

    int ic_latch1[2];
    int ic_latch2[2];
    int ic_latch3[2];
    int ic_check1;
    int ic_check3;
    int prescaler_latch[2];
#ifndef FMOPNA_YM2612
    int ic_check2;
#endif
#ifdef FMOPNA_YM2608
    int prescaler_sel[2];
#endif
    int pssel_l[15][2];
    int ic_latch_fm[2];

    int mclk1;
    int mclk2;

    int clk1;
    int clk2;

#ifndef FMOPNA_YM2612
    int aclk1;
    int aclk2;
    int bclk1;
    int bclk2;
    int cclk1;
    int cclk2;

    int dclk;

    int ssg_write0;
    int ssg_write1;
    int ssg_read1;

    int write2;
    int write3;

    int read2;
#endif
#ifdef FMOPNA_YM2608
    int read3;
#endif

    int read0;

    int write0_trig0;
    int write0_trig1;
    int write0_l[3];
    int write0_en;

    int write1_trig0;
    int write1_trig1;
    int write1_l[3];
    int write1_en;

#ifdef FMOPNA_YM2608
    int write2_trig0;
    int write2_trig1;
    int write2_l[3];
    int write2_en;
#endif

#ifndef FMOPNA_YM2612
    int write3_trig0;
    int write3_trig1;
    int write3_l[3];
    int write3_en;
#endif

    int data_l;

    int data_bus1;
    int data_bus2; // inverted

    int read_bus;

#ifdef FMOPNA_YM2608
    int addr_10[2];
    int addr_10h[2];
    int addr_12[2];
#endif
    int addr_21[2];
    int addr_22[2];
    int addr_24[2];
    int addr_25[2];
    int addr_26[2];
    int addr_27[2];
    int addr_28[2];
#ifdef FMOPNA_YM2608
    int addr_29[2];
    int addr_ff[2];
#endif
#ifdef FMOPNA_YM2610
    int addr_00[2];
    int addr_02[2];
    int addr_1c[2];
#endif
#ifdef FMOPNA_YM2612
    int addr_2a[2];
    int addr_2b[2];
    int addr_2c[2];
#endif

#ifdef FMOPNA_YM2608
    int reg_mask[2];
#endif
#ifndef FMOPNA_YM2612
    int reg_test_12[2];
#endif
    int reg_test_21[2];
    int reg_lfo[2];
    int reg_timer_a[2];
    int reg_timer_b[2];
    int reg_ch3[2];
    int reg_timer_a_load[2];
    int reg_timer_b_load[2];
    int reg_timer_a_enable[2];
    int reg_timer_b_enable[2];
    int reg_timer_a_reset[2];
    int reg_timer_b_reset[2];
#ifdef FMOPNA_YM2608
    int reg_sch[2];
    int reg_irq[2];
#endif
#ifdef FMOPNA_YM2610
    int reg_flags[2];
#endif
    int reg_kon_operator[2];
    int reg_kon_channel[2];
#ifdef FMOPNA_YM2612
    int reg_dac_en[2];
    int reg_dac_data[2];
    int reg_test_2c[2];
#endif
    int fm_address[2];
    int fm_data[2];
    int write_fm_address[2];
    int write_fm_data[2]; 
    int reg_cnt1[2];
    int reg_cnt2[2];
    int reg_cnt_sync;
    int reg_key_cnt1[2];
    int reg_key_cnt2[2];
#ifdef FMOPNA_YM2608
    int reg_cnt_rss[2];
    int reg_cnt_rss_of;
    int rss_18;
#endif
    int reg_kon_match;
    int reg_kon[4][2];
    int kon_comb;
    int reg_ch3_sel;
    int ch3_en;
    int ch3_csm;
    int ch3_csm_load;
    int reg_csm_l;
    int reg_sync_timer;
    int reg_sync_timer_l[2];
    int reg_sync_timer_load;

    int timer_a_cnt[2];
    int timer_a_of[2];
    int timer_a_load;
    int timer_a_reg_load;
    int timer_a_reg_load_l[2];
    int timer_a_status[2];
    int timer_b_subcnt[2];
    int timer_b_subcnt_of[2];
    int timer_b_cnt[2];
    int timer_b_of[2];
    int timer_b_load;
    int timer_b_reg_load;
    int timer_b_reg_load_l[2];
    int timer_b_status[2];

#ifndef FMOPNA_YM2612
    int irq_eos_l;
#endif
#ifdef FMOPNA_YM2608
    int irq_mask_eos;
    int irq_mask_brdy;
    int irq_mask_zero;
#endif

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
    int fm_isb4;

    int reg_a4[2];
    unsigned short reg_freq[2][6];
    int reg_ac[2];
    unsigned short reg_freq_3ch[2][6];
    unsigned char reg_connect_fb[2][6];
    unsigned char reg_b4[2][6];
    int fnum[4];
    int kcode[4];
    int ch_cnt_sync;
    int ch_cnt1[2];
    int ch_cnt2[2];
#ifdef FMOPNA_YM2608
    unsigned char reg_rss[2][6];
#endif
    unsigned char op_multi_dt[2][12][2];
    unsigned char op_tl[2][12][2];
    unsigned char op_ar_ks[2][12][2];
    unsigned char op_dr_a[2][12][2];
    unsigned char op_sr[2][12][2];
    unsigned char op_rr_sl[2][12][2];
    unsigned char op_ssg[2][12][2];

    int fsm_cnt1[2];
    int fsm_cnt2[2];
    int fsm_out[27];
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
#ifndef FMOPNA_YM2612
    int fsm_rss;
    int fsm_rss2;
#endif
    int fsm_sel0[2];
    int fsm_sel1[2];
    int fsm_sel2[2];
    int fsm_sel23[2];
    int fsm_sel_ch3[2];
#ifndef FMOPNA_YM2612
    int fsm_sel11[2];
    int fsm_sh1[2];
    int fsm_sh2[2];
#endif
#ifdef FMOPNA_YM2612
    int fsm_dac_ch6[2];
    int fsm_dac_load[2];
    int fsm_dac_out_sel[2];
#endif

    int lfo_sync[4];
    int lfo_subcnt[2];
    int lfo_subcnt_of;
    int lfo_cnt_rst;
    int lfo_cnt[2];
#ifdef FMOPNA_YM2608
    int lfo_cnt_of;
    int lfo_mode;
#endif
    int lfo_cnt_load;
    int lfo_fnum1;
    int lfo_fnum2;
    int lfo_shift;
    int lfo_sign;
    int lfo_pm;
    int lfo_fnum;
    int lfo_am;

    int pg_block;
    int pg_freq;
    int pg_dt_multi;
    int pg_dt_add;
    int pg_freqdt[2];
    int pg_multi[5];
    int pg_add[6];
    int pg_reset[4];
    int pg_phase[2][23];
    int pg_phase2[2];
    int pg_out;
    int pg_dbgsync;
#ifdef FMOPNA_YM2610
    int pg_dbgsync_l[2];
#endif
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
    int eg_clock_delay[2];
    int eg_ic[2];

    int eg_step[3];
    int eg_timer_step[2];

    int eg_timer[2];
    int eg_timer_sum[2];
    int eg_timer_carry[2];
    int eg_timer_mask[2];
    int eg_timer_masked[2];
    int eg_timer_test;
    int eg_timer_test_bit[2];
    int eg_timer_low_lock;
    int eg_shift_lock;

    int eg_rate_sel;
    int eg_rate_ar;
    int eg_rate_dr;
    int eg_rate_sr;
    int eg_rate_rr;
    int eg_rate;
    int eg_rate_nonzero[3];
    unsigned char eg_state[2][23];
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
    unsigned short eg_level[2][22];
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
    int eg_am_l[2];
    int eg_am_shift[2];
    int eg_of1;
    int eg_output_lfo;
    int eg_csm_tl;
    int eg_ch3_l[2];
    int eg_out;
    int eg_dbg_sync;
    int eg_debug[2];
    int eg_debug_inc;

    int op_phase1;
    int op_phase2;
    unsigned short op_mod[2][6];
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
    unsigned short op_op1_0[2][6];
    unsigned short op_op1_1[2][6];
    unsigned short op_op2[2][6];
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

#ifndef FMOPNA_YM2612
#ifdef FMOPNA_YM2608
    int ssg_prescaler1[2];
    int ssg_prescaler2[2];
#endif
    int ssg_div1[2];
    int ssg_div2[2];
    int ssg_div3[2];
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
    int ssg_test;
    int ssg_noise_step;
    int ssg_noise_lfsr[2];
    int ssg_noise_bit;


    int rss_cnt1[2];
    int rss_cnt1_sync;

    int rss_eclk1_l;
    int rss_eclk2_l;
    int rss_eclk1;
    int rss_eclk2;
    int rss_dclk_l;
    int rss_fclk_sel[2];
    int rss_fclk1;
    int rss_fclk2;
    int rss_cnt2[2];
    int rss_fmcnt[2];
    int rss_fmcnt_of;
    int rss_fmcnt_sync;
    int rss_params[2];
    int rss_tl_sel[2];
    int rss_tl[2];
    int rss_keydm[2];
    int rss_keymask[2];
    int rss_ic[2];
    int rss_key[2];
    int rss_stop[2];
    int rss_eos_l;
#ifdef FMOPNA_YM2608
    int rss_step;
    int rss_isend;
#endif

#ifdef FMOPNA_YM2610
    int rss_address[2];
    int rss_address_wr[2];
    int rss_data[2];
    int rss_data_wr[2];
    int rss_write08;
    int rss_write10;
    int rss_write18;
    int rss_write20;
    int rss_write28;
    unsigned char rss_reg_pan_tl[2][6];
    unsigned char rss_reg_start_l[2][6];
    unsigned char rss_reg_start_h[2][6];
    unsigned char rss_reg_stop_l[2][6];
    unsigned char rss_reg_stop_h[2][6];
    int rss_fm_match_l;
    int rss_params_start[3];
    int rss_params_start_h;
    int rss_params_stop[3];
    int rss_eos_load;
    int rss_rmpx[2];
    int rss_roe[4];
    int rss_rad_bus;
    int rss_stop_flag[2];
#endif

    int rss_accum[2];
    int rss_regs[2][17];

    int rss_delta_ix_load;
    int rss_delta_ix;
    int rss_ix_load;
    int rss_ix;
    int rss_cnt1_is1;
    int rss_nibble;
    int rss_tl_l;
    int rss_tl_shift[3];

    int rss_sample_load;
    int rss_sample;
    int rss_multi_ctrl[2];
    int rss_multi_accum[2];
    int rss_multi_accum_load;
    int rss_sample_multi;
    int rss_sample_shift_load;
    int rss_sample_shift;
    int rss_dbg_load;
    int rss_dbg_data;

    int rss_pan[3];

#ifdef FMOPNA_YM2608
    int ad_is6;
    int ad_is7;
    int ad_is8;
    int ad_isc;
    int ad_isd;
    int ad_ise;
    int ad_isf;
    int ad_reg_limit_l;
    int ad_reg_limit_h;
    int ad_reg_prescale_l;
    int ad_reg_prescale_h;
    int ad_reg_spoff;
    int ad_reg_memdata;
    int ad_reg_rec;
    int ad_reg_rom;
    int ad_reg_ramtype;
    int ad_reg_da_ad;
    int ad_reg_sample;
    int ad_sample_l[3];
    int ad_read_port_l[2];
    int ad_write_port_l[2];
    int ad_rw_l[2];
    int ad_rw_en;
    int ad_rec_start_l[2];
    int ad_limit_match[2];
    int ad_limit_match2[2];
    int ad_mem_sync[2];
    int ad_mem_sync_run;
    int ad_mem_ptr_store;
    int ad_mem_data_l1;
    int ad_mem_data_l2;
    int ad_mem_addr_bank;
    int ad_mem_data_bus;
    int ad_mem_bit_cnt[2];
    int ad_mem_shift_cnt0_l[2];
    int ad_brdy_set_l[2];
    int ad_mem_rw_en[2];
    int ad_mem_ucnt[2];
    int ad_mem_w22;
    int ad_dsp_ctrl10_l;
    int ad_dsp_alu_neg[2];
    int ad_ad_cnt1[2];
    int ad_ad_cmp_i;
    int ad_ad_shift;
    int ad_ad_w53[2];
    int ad_ad_w55;
    int ad_ad_w55_l[2];
    int ad_ad_w56;
    int ad_ad_buf;
    int ad_ad_input;
    int ad_ad_cnt2[2];
    int ad_ad_w57[3];
    int ad_ad_w58[3]; // sh2
    int ad_ad_w60;
    int ad_ad_w61[2]; // opo
    int ad_ad_w62[2];
    int ad_ad_w65_l;
    int ad_ad_w66[2];
    int ad_ad_w68;
    int ad_ad_cnt3[2];
    int ad_ad_cnt3_load;
    int ad_ad_cnt3_load_val;
    int ad_ad_cnt3_of[2];
    int ad_ad_cnt3_en[2];
    int ad_ad_quiet;
    int ad_comp_da;
    int ad_dsp_enc_bit;
    int ad_dsp_enc_bit_l[2];
    int ad_mem_we[2];
    int ad_mem_cas[2];
    int ad_mem_ras[2];
#else
    int ad_mem_pmpx[2];
    int ad_mem_poe[2];
#endif
    int ad_is0;
    int ad_is1;
    int ad_is2;
    int ad_is3;
    int ad_is4;
    int ad_is5;
    int ad_is9;
    int ad_isa;
    int ad_isb;
    int ad_reg_reset;
    int ad_reg_repeat;
    int ad_reg_start;
    int ad_reg_r;
    int ad_reg_l;
    int ad_reg_delta_l;
    int ad_reg_delta_h;
    int ad_reg_start_l;
    int ad_reg_start_h;
    int ad_reg_stop_l;
    int ad_reg_stop_h;
    int ad_reg_level;
    int ad_code_ptr[2];
    int ad_code_ctrl;
    int ad_code_ctrl_l;
    int ad_w2[2];
    int ad_w4[2];
    int ad_start_l[3];
    int ad_w2_l[2];
    int ad_code_end[2];
    int ad_addr_isend_l;
    int ad_addr_isend_l2[2];
    int ad_mem_ctrl;
    int ad_mem_ctrl_l;
    int ad_mem_code_ptr[2];
    int ad_end_sel[2];
    int ad_stop_match[2];
    int ad_stop_match2[2];
    int ad_start_sel[2];
    int ad_address_cnt[4][2];
    int ad_address_carry[2];
    int ad_mem_cond[2];
    int ad_mem_bus;
    int ad_mem_data_l3;
    int ad_mem_data_l4[2];
    int ad_mem_w7[2];
    int ad_mem_shift_cnt[2];
    int ad_mem_nibble;
    int ad_mem_w8[2];
    int ad_mem_nibble_msb;
    int ad_mem_nibble_load;
    int ad_mem_mem_en_l[2];
    int ad_code_ed_end[2];
    int ad_mem_mem_stop[2];
    int ad_mem_w10[2];
    int ad_w12[2];
    int ad_w13[2];
    int ad_mem_w15[2];
    int ad_mem_w17[2];
    int ad_mem_w20[2];
    int ad_mem_w21;
    int ad_mode6_l[2];
    int ad_dsp_bus;
    int ad_dsp_ctrl;
    int ad_dsp_delta_sel[2];
    int ad_dsp_w30_l[2];
    int ad_dsp_w31_l[2];
    int ad_dsp_w32;
    int ad_dsp_w32_l;
    int ad_dsp_w33;
    int ad_dsp_w34;
    int ad_dsp_w36[2];
    int ad_dsp_w35_l;
    int ad_dsp_cnt1[2];
    int ad_dsp_cnt1_run[2];
    int ad_dsp_mul_accm1[2];
    int ad_dsp_mul_accm1_add1;
    int ad_dsp_mul_accm1_add2;
    int ad_dsp_mul_accm1_c;
    int ad_dsp_mul_accm2;
    int ad_dsp_mul_accm2_add1;
    int ad_dsp_mul_accm2_add2;
    int ad_dsp_mul_accm2_load;
    int ad_dsp_w38[2];
    int ad_dsp_w40;
    int ad_dsp_w41[2];
    int ad_dsp_w43[2];
    int ad_dsp_w45;
    int ad_dsp_w46[2];
    int ad_output;
    int ad_dsp_cnt2[2];
    int ad_dsp_load_alu1[2];
    int ad_dsp_load_alu1_h;
    int ad_dsp_load_alu2[2];
    int ad_dsp_alu_in1;
    int ad_dsp_alu_in2;
    int ad_dsp_load_res[2];
    int ad_dsp_alu_res;
    int ad_dsp_alu_shift;
    int ad_dsp_alu_mask[2];
    int ad_dsp_carry_mode[2];
    int ad_dsp_alu_of;
    int ad_dsp_read_res[2];
    int ad_code_sync[3];
    int ad_dsp_w52[2];
    int ad_code_reg_id;
    int ad_dsp_regs[8];
    int ad_dsp_regs_o;
    int ad_set_eos;
    int ad_dsp_vol_o[2];
    int ad_dsp_sregs2[2][2];
    int ad_dsp_w69[2];
    int ad_mem_dir;

#ifdef FMOPNA_YM2608
    int ad_da_data;
    int ac_da_shift[2];
    int ac_da_w70[2];
    int ac_da_set[2];
    int opo_da[2];
    int opo_ad;
#endif

    int ac_fm_output;
    int ac_fm_output_en;
    int ac_fm_pan;
    int ac_fm_accm1[2];
    int ac_da_sync2;
    int ac_ad_output;
    int ac_da_sync;
    int ac_da_sync3[2];
    int ac_fm_accm2[2];
    int ac_rss_sum_l;
    int ac_rss_sum_r;
    int ac_rss_accm1[2];
    int ac_rss_accm2[2];
    int ac_rss_load;
    int ac_shifter[2];
    int ac_shifter_load_l;
    int ac_shifter_load_r;
    int ac_shifter_bit;
    int ac_shifter_top;
    int ac_opo;
    int sh1_l;
    int sh2_l;
    int opo_fm;

    int last_rss_sample;
#endif

    int busy_cnt[2];
    int busy_cnt_en[2];
    int status_timer_a;
    int status_timer_b;
    int eg_dbg;
#ifdef FMOPNA_YM2608
    int status_brdy;
    int status_zero;
    int brdy_flag;
    int zero_flag;
    int zero_set;
#endif
#ifdef FMOPNA_YM2610
    int flag_rss_0;
    int flag_rss_1;
    int flag_rss_2;
    int flag_rss_3;
    int flag_rss_4;
    int flag_rss_5;
    int status_rss_0;
    int status_rss_1;
    int status_rss_2;
    int status_rss_3;
    int status_rss_4;
    int status_rss_5;
#endif

#ifndef FMOPNA_YM2612
    int status_eos;
    int eos_flag;
    int eos_l[2];
    int eos_repeat;
#endif

#ifdef FMOPNA_YM2612
    int ch_op_output;
    int ch_op_add;
    int ch_accm_load;
    short ch_accm[2][6];
    short ch_buf[2][6];
    int ch_output;
    int ch_load[2];
    int ch_sel;
    int ch_dbg[2];
    int ch_ch6;
    int ch_pan;
    int o_mol;
    int o_mor;
    int o_test;
    int o_test_d;
#endif

#ifdef FMOPNA_YM2608
    int o_gpio_a;
    int o_gpio_a_d;
    int o_gpio_b;
    int o_gpio_b_d;
    int o_spoff;
    int o_a8;
    int o_romcs; // neg
    int o_mden;
    int o_we; // neg
    int o_cas; // neg
    int o_ras; // neg
    int o_dm;
    int o_dm_d;
#endif

#ifdef FMOPNA_YM2610
    int o_rmpx;
    int o_roe; // neg
    int o_rad;
    int o_rad_d;
    int o_ra8;
    int o_ra20;

    int o_pmpx;
    int o_poe; // neg
    int o_pad;
    int o_pad_d;
    int o_pa8;

    int ym2610b;
#endif

#ifndef FMOPNA_YM2612
    float o_analog;
    float o_analog_ch[3];
    int o_sh1;
    int o_sh2;
    int o_opo;
    int o_s;
#endif
    int o_irq_pull;
    int o_data;
    int o_data_d;

    int tm_w1;
}
#ifdef FMOPNA_YM2608
fmopna_t;
#elif defined (FMOPNA_YM2610)
fmopna_2610_t;
#else
fmopna_2612_t;
#endif

#ifdef FMOPNA_YM2608
void FMOPNA_Clock(fmopna_t* chip, int clk);
#elif defined (FMOPNA_YM2610)
void FMOPNA_2610_Clock(fmopna_2610_t* chip, int clk);
#else
void FMOPNA_2612_Clock(fmopna_2612_t* chip, int clk);
#endif

