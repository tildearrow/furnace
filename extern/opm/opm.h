/* Nuked OPM
 * Copyright (C) 2020 Nuke.YKT
 *
 * This file is part of Nuked OPM.
 *
 * Nuked OPM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2.1
 * of the License, or (at your option) any later version.
 *
 * Nuked OPM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Nuked OPM. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Nuked OPM emulator.
 *  Thanks:
 *      siliconpr0n.org(digshadow, John McMaster):
 *          YM2151 and other FM chip decaps and die shots.
 *
 * version: 0.9.2 beta
 */
#ifndef _OPM_H_
#define _OPM_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t cycles;
    uint8_t ic;
    uint8_t ic2;
    // IO
    uint8_t write_data;
    uint8_t write_a;
    uint8_t write_a_en;
    uint8_t write_d;
    uint8_t write_d_en;
    uint8_t write_busy;
    uint8_t write_busy_cnt;
    uint8_t mode_address;
    uint8_t io_ct1;
    uint8_t io_ct2;

    // LFO
    uint8_t lfo_am_lock;
    uint8_t lfo_pm_lock;
    uint8_t lfo_counter1;
    uint8_t lfo_counter1_of1;
    uint8_t lfo_counter1_of2;
    uint16_t lfo_counter2;
    uint8_t lfo_counter2_load;
    uint8_t lfo_counter2_of;
    uint8_t lfo_counter2_of_lock;
    uint8_t lfo_counter2_of_lock2;
    uint8_t lfo_counter3_clock;
    uint16_t lfo_counter3;
    uint8_t lfo_counter3_step;
    uint8_t lfo_frq_update;
    uint8_t lfo_clock;
    uint8_t lfo_clock_lock;
    uint8_t lfo_clock_test;
    uint8_t lfo_test;
    uint32_t lfo_val;
    uint8_t lfo_val_carry;
    uint32_t lfo_out1;
    uint32_t lfo_out2;
    uint32_t lfo_out2_b;
    uint8_t lfo_mult_carry;
    uint8_t lfo_trig_sign;
    uint8_t lfo_saw_sign;
    uint8_t lfo_bit_counter;

    // Env Gen
    uint8_t eg_state[32];
    uint16_t eg_level[32];
    uint8_t eg_rate[2];
    uint8_t eg_sl[2];
    uint8_t eg_tl[3];
    uint8_t eg_zr[2];
    uint8_t eg_timershift_lock;
    uint8_t eg_timer_lock;
    uint8_t eg_inchi;
    uint8_t eg_shift;
    uint8_t eg_clock;
    uint8_t eg_clockcnt;
    uint8_t eg_clockquotinent;
    uint8_t eg_inc;
    uint8_t eg_ratemax[2];
    uint8_t eg_instantattack;
    uint8_t eg_inclinear;
    uint8_t eg_incattack;
    uint8_t eg_mute;
    uint16_t eg_outtemp[2];
    uint16_t eg_out[2];
    uint16_t eg_am;
    uint8_t eg_ams[2];
    uint8_t eg_timercarry;
    uint32_t eg_timer;
    uint32_t eg_timer2;
    uint8_t eg_timerbstop;
    uint32_t eg_serial;
    uint8_t eg_serial_bit;
    uint8_t eg_test;
    

    // Phase Gen
    uint16_t pg_fnum[32];
    uint8_t pg_kcode[32];
    uint32_t pg_inc[32];
    uint32_t pg_phase[32];
    uint8_t pg_reset[32];
    uint8_t pg_reset_latch[32];
    uint32_t pg_serial;

    // Operator
    uint16_t op_phase_in;
    uint16_t op_mod_in;
    uint16_t op_phase;
    uint16_t op_logsin[3];
    uint16_t op_atten;
    uint16_t op_exp[2];
    uint8_t op_pow[2];
    uint32_t op_sign;
    int16_t op_out[6];
    uint32_t op_connect;
    uint8_t op_counter;
    uint8_t op_fbupdate;
    uint8_t op_fbshift;
    uint8_t op_c1update;
    uint8_t op_modtable[5];
    int16_t op_m1[8][2];
    int16_t op_c1[8];
    int16_t op_mod[3];
    int16_t op_fb[2];
    uint8_t op_mixl;
    uint8_t op_mixr;
    uint16_t op_chmix[8];

    // Mixer

    int32_t mix[2];
    int32_t mix2[2];
    int32_t mix_op;
    uint32_t mix_serial[2];
    uint32_t mix_bits;
    uint32_t mix_top_bits_lock;
    uint8_t mix_sign_lock;
    uint8_t mix_sign_lock2;
    uint8_t mix_exp_lock;
    uint8_t mix_clamp_low[2];
    uint8_t mix_clamp_high[2];
    uint8_t mix_out_bit;

    // Output
    uint8_t smp_so;
    uint8_t smp_sh1;
    uint8_t smp_sh2;
    uint16_t ch_out[8];

    // Noise
    uint32_t noise_lfsr;
    uint32_t noise_timer;
    uint8_t noise_timer_of;
    uint8_t noise_update;
    uint8_t noise_temp;

    // Register set
    uint8_t mode_test[8];
    uint8_t mode_kon_operator[4];
    uint8_t mode_kon_channel;

    uint8_t reg_address;
    uint8_t reg_address_ready;
    uint8_t reg_data;
    uint8_t reg_data_ready;

    uint8_t ch_rl[8];
    uint8_t ch_fb[8];
    uint8_t ch_connect[8];
    uint8_t ch_kc[8];
    uint8_t ch_kf[8];
    uint8_t ch_pms[8];
    uint8_t ch_ams[8];

    uint8_t sl_dt1[32];
    uint8_t sl_mul[32];
    uint8_t sl_tl[32];
    uint8_t sl_ks[32];
    uint8_t sl_ar[32];
    uint8_t sl_am_e[32];
    uint8_t sl_d1r[32];
    uint8_t sl_dt2[32];
    uint8_t sl_d2r[32];
    uint8_t sl_d1l[32];
    uint8_t sl_rr[32];

    uint8_t noise_en;
    uint8_t noise_freq;


    // Timer
    uint16_t timer_a_reg;
    uint8_t timer_b_reg;
    uint8_t timer_a_temp;
    uint8_t timer_a_do_reset, timer_a_do_load;
    uint8_t timer_a_inc;
    uint16_t timer_a_val;
    uint8_t timer_a_of;
    uint8_t timer_a_load;
    uint8_t timer_a_status;

    uint8_t timer_b_sub;
    uint8_t timer_b_sub_of;
    uint8_t timer_b_inc;
    uint16_t timer_b_val;
    uint8_t timer_b_of;
    uint8_t timer_b_do_reset, timer_b_do_load;
    uint8_t timer_b_temp;
    uint8_t timer_b_status;
    uint8_t timer_irq;

    uint8_t lfo_freq_hi;
    uint8_t lfo_freq_lo;
    uint8_t lfo_pmd;
    uint8_t lfo_amd;
    uint8_t lfo_wave;

    uint8_t timer_irqa, timer_irqb;
    uint8_t timer_loada, timer_loadb;
    uint8_t timer_reseta, timer_resetb;
    uint8_t mode_csm;

    uint8_t nc_active, nc_active_lock, nc_sign, nc_sign_lock, nc_sign_lock2;
    uint8_t nc_bit;
    uint16_t nc_out;
    int16_t op_mix;

    uint8_t kon_csm, kon_csm_lock;
    uint8_t kon_do;
    uint8_t kon_chanmatch;
    uint8_t kon[32];
    uint8_t kon2[32];
    uint8_t mode_kon[32];

    // DAC
    uint8_t dac_osh1, dac_osh2;
    uint16_t dac_bits;
    int32_t dac_output[2];
} opm_t;

void OPM_Clock(opm_t *chip, int32_t *output, uint8_t *sh1, uint8_t *sh2, uint8_t *so);
void OPM_Write(opm_t *chip, uint32_t port, uint8_t data);
uint8_t OPM_Read(opm_t *chip, uint32_t port);
uint8_t OPM_ReadIRQ(opm_t *chip);
uint8_t OPM_ReadCT1(opm_t *chip);
uint8_t OPM_ReadCT2(opm_t *chip);
void OPM_SetIC(opm_t *chip, uint8_t ic);
void OPM_Reset(opm_t *chip);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
