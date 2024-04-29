/*
 * Copyright (C) 2023-2024 nukeykt
 * Copyright (C) 2024 tildearrow
 *
 * This file is part of AtomicSSG.
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
 *  SSG (AY-3-8910/YM2149) emulator.
 *  based on YM2608-LLE by nukeykt.
 *
 */
typedef struct {
    int clk;
    int ic; // neg
    int cs; // neg
    int wr; // neg
    int rd; // neg
    int a0;
    int data;
    int test; // set to 1
    int gpio_a;
    int gpio_b;
}
ssg_input_t;

typedef struct {
    ssg_input_t input;

    // bitfield
    // bit 0: AY-3-8910 (16-step envelope)
    // bit 1: AY-3-8914 (different register map and envelope volume)
    // bit 2: AY8930 (TODO)
    int type;

    int ic;

    int ic_latch1[2];
    int ic_latch2[2];
    int ic_latch3[2];
    int ic_check1;
    int ic_check3;
    int prescaler_latch[2];
    int ic_check2;
    int prescaler_sel[2];
    int pssel_l[15][2];
    int ic_latch_fm[2];

    int clk1;
    int clk2;

    int ssg_write0;
    int ssg_write1;
    int ssg_read1;

    int write2;
    int write3;

    int read2;
    int read3;

    int read0;

    int write0_trig0;
    int write0_trig1;
    int write0_l[3];
    int write0_en;

    int write1_trig0;
    int write1_trig1;
    int write1_l[3];
    int write1_en;

    int write2_trig0;
    int write2_trig1;
    int write2_l[3];
    int write2_en;

    int write3_trig0;
    int write3_trig1;
    int write3_l[3];
    int write3_en;

    int data_l;

    int data_bus1;
    int data_bus2; // inverted

    int read_bus;

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

    int o_gpio_a;
    int o_gpio_a_d;
    int o_gpio_b;
    int o_gpio_b_d;

    int o_analog[3];

    int o_data;
    int o_data_d;
}
ssg_t;

void SSG_SetType(ssg_t* chip, int type);
void SSG_Reset(ssg_t* chip);
void SSG_Clock(ssg_t* chip, int clk);
void SSG_Write(ssg_t* chip, unsigned char addr, unsigned char val);
