/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "engine.h"
#include "../ta-log.h"
#include <unordered_map>

int DivCS::getCmdLength(unsigned char ext) {
  switch (ext) {
    case DIV_CMD_SAMPLE_MODE:
    case DIV_CMD_SAMPLE_FREQ:
    case DIV_CMD_SAMPLE_BANK:
    case DIV_CMD_SAMPLE_DIR:
    case DIV_CMD_FM_HARD_RESET:
    case DIV_CMD_FM_LFO:
    case DIV_CMD_FM_LFO_WAVE:
    case DIV_CMD_FM_LFO2:
    case DIV_CMD_FM_LFO2_WAVE:
    case DIV_CMD_FM_FB:
    case DIV_CMD_FM_EXTCH:
    case DIV_CMD_FM_AM_DEPTH:
    case DIV_CMD_FM_PM_DEPTH:
    case DIV_CMD_STD_NOISE_FREQ:
    case DIV_CMD_STD_NOISE_MODE:
    case DIV_CMD_WAVE:
    case DIV_CMD_GB_SWEEP_TIME:
    case DIV_CMD_GB_SWEEP_DIR:
    case DIV_CMD_PCE_LFO_MODE:
    case DIV_CMD_PCE_LFO_SPEED:
    case DIV_CMD_NES_DMC:
    case DIV_CMD_C64_CUTOFF:
    case DIV_CMD_C64_RESONANCE:
    case DIV_CMD_C64_FILTER_MODE:
    case DIV_CMD_C64_RESET_TIME:
    case DIV_CMD_C64_RESET_MASK:
    case DIV_CMD_C64_FILTER_RESET:
    case DIV_CMD_C64_DUTY_RESET:
    case DIV_CMD_C64_EXTENDED:
    case DIV_CMD_AY_ENVELOPE_SET:
    case DIV_CMD_AY_ENVELOPE_LOW:
    case DIV_CMD_AY_ENVELOPE_HIGH:
    case DIV_CMD_AY_ENVELOPE_SLIDE:
    case DIV_CMD_AY_NOISE_MASK_AND:
    case DIV_CMD_AY_NOISE_MASK_OR:
    case DIV_CMD_AY_AUTO_ENVELOPE:
    case DIV_CMD_FDS_MOD_DEPTH:
    case DIV_CMD_FDS_MOD_HIGH:
    case DIV_CMD_FDS_MOD_LOW:
    case DIV_CMD_FDS_MOD_POS:
    case DIV_CMD_FDS_MOD_WAVE:
    case DIV_CMD_SAA_ENVELOPE:
    case DIV_CMD_AMIGA_FILTER:
    case DIV_CMD_AMIGA_AM:
    case DIV_CMD_AMIGA_PM:
    case DIV_CMD_MACRO_OFF:
    case DIV_CMD_MACRO_ON:
    case DIV_CMD_MACRO_RESTART:
    case DIV_CMD_HINT_ARP_TIME:
    case DIV_CMD_QSOUND_ECHO_FEEDBACK:
    case DIV_CMD_QSOUND_ECHO_LEVEL:
    case DIV_CMD_QSOUND_SURROUND:
    case DIV_CMD_X1_010_ENVELOPE_SHAPE:
    case DIV_CMD_X1_010_ENVELOPE_ENABLE:
    case DIV_CMD_X1_010_ENVELOPE_MODE:
    case DIV_CMD_X1_010_ENVELOPE_PERIOD:
    case DIV_CMD_X1_010_ENVELOPE_SLIDE:
    case DIV_CMD_X1_010_AUTO_ENVELOPE:
    case DIV_CMD_X1_010_SAMPLE_BANK_SLOT:
    case DIV_CMD_WS_SWEEP_TIME:
    case DIV_CMD_WS_SWEEP_AMOUNT:
    case DIV_CMD_N163_WAVE_POSITION:
    case DIV_CMD_N163_WAVE_LENGTH:
    case DIV_CMD_N163_WAVE_UNUSED1:
    case DIV_CMD_N163_WAVE_UNUSED2:
    case DIV_CMD_N163_WAVE_LOADPOS:
    case DIV_CMD_N163_WAVE_LOADLEN:
    case DIV_CMD_N163_WAVE_UNUSED3:
    case DIV_CMD_N163_CHANNEL_LIMIT:
    case DIV_CMD_N163_GLOBAL_WAVE_LOAD:
    case DIV_CMD_N163_GLOBAL_WAVE_LOADPOS:
    case DIV_CMD_N163_UNUSED4:
    case DIV_CMD_N163_UNUSED5:
    case DIV_CMD_SU_SYNC_PERIOD_LOW:
    case DIV_CMD_SU_SYNC_PERIOD_HIGH:
    case DIV_CMD_ADPCMA_GLOBAL_VOLUME:
    case DIV_CMD_SNES_ECHO:
    case DIV_CMD_SNES_PITCH_MOD:
    case DIV_CMD_SNES_INVERT:
    case DIV_CMD_SNES_GAIN_MODE:
    case DIV_CMD_SNES_GAIN:
    case DIV_CMD_SNES_ECHO_ENABLE:
    case DIV_CMD_SNES_ECHO_DELAY:
    case DIV_CMD_SNES_ECHO_VOL_LEFT:
    case DIV_CMD_SNES_ECHO_VOL_RIGHT:
    case DIV_CMD_SNES_ECHO_FEEDBACK:
    case DIV_CMD_NES_ENV_MODE:
    case DIV_CMD_NES_LENGTH:
    case DIV_CMD_NES_COUNT_MODE:
    case DIV_CMD_FM_AM2_DEPTH:
    case DIV_CMD_FM_PM2_DEPTH:
    case DIV_CMD_ES5506_ENVELOPE_LVRAMP:
    case DIV_CMD_ES5506_ENVELOPE_RVRAMP:
    case DIV_CMD_ES5506_PAUSE:
    case DIV_CMD_ES5506_FILTER_MODE:
    case DIV_CMD_SNES_GLOBAL_VOL_LEFT:
    case DIV_CMD_SNES_GLOBAL_VOL_RIGHT:
    case DIV_CMD_NES_LINEAR_LENGTH:
    case DIV_CMD_EXTERNAL:
    case DIV_CMD_C64_AD:
    case DIV_CMD_C64_SR:
    case DIV_CMD_DAVE_HIGH_PASS:
    case DIV_CMD_DAVE_RING_MOD:
    case DIV_CMD_DAVE_SWAP_COUNTERS:
    case DIV_CMD_DAVE_LOW_PASS:
    case DIV_CMD_DAVE_CLOCK_DIV:
    case DIV_CMD_MINMOD_ECHO:
    case DIV_CMD_FDS_MOD_AUTO:
    case DIV_CMD_FM_OPMASK:
    case DIV_CMD_MULTIPCM_MIX_FM:
    case DIV_CMD_MULTIPCM_MIX_PCM:
    case DIV_CMD_MULTIPCM_LFO:
    case DIV_CMD_MULTIPCM_VIB:
    case DIV_CMD_MULTIPCM_AM:
    case DIV_CMD_MULTIPCM_AR:
    case DIV_CMD_MULTIPCM_D1R:
    case DIV_CMD_MULTIPCM_DL:
    case DIV_CMD_MULTIPCM_D2R:
    case DIV_CMD_MULTIPCM_RC:
    case DIV_CMD_MULTIPCM_RR:
    case DIV_CMD_MULTIPCM_DAMP:
    case DIV_CMD_MULTIPCM_PSEUDO_REVERB:
    case DIV_CMD_MULTIPCM_LFO_RESET:
    case DIV_CMD_MULTIPCM_LEVEL_DIRECT:
    case DIV_CMD_SID3_SPECIAL_WAVE:
    case DIV_CMD_SID3_RING_MOD_SRC:
    case DIV_CMD_SID3_HARD_SYNC_SRC:
    case DIV_CMD_SID3_PHASE_MOD_SRC:
    case DIV_CMD_SID3_WAVE_MIX:
    case DIV_CMD_SID3_1_BIT_NOISE:
    case DIV_CMD_SID3_CHANNEL_INVERSION:
    case DIV_CMD_SID3_FILTER_CONNECTION:
    case DIV_CMD_SID3_FILTER_MATRIX:
    case DIV_CMD_SID3_FILTER_ENABLE:
    case DIV_CMD_SID3_PHASE_RESET:
    case DIV_CMD_SID3_NOISE_PHASE_RESET:
    case DIV_CMD_SID3_ENVELOPE_RESET:
    case DIV_CMD_SID3_CUTOFF_SCALING:
    case DIV_CMD_SID3_RESONANCE_SCALING:
    case DIV_CMD_WS_GLOBAL_SPEAKER_VOLUME:
      return 1;
    case DIV_CMD_FM_TL:
    case DIV_CMD_FM_AM:
    case DIV_CMD_FM_AR:
    case DIV_CMD_FM_DR:
    case DIV_CMD_FM_SL:
    case DIV_CMD_FM_D2R:
    case DIV_CMD_FM_RR:
    case DIV_CMD_FM_DT:
    case DIV_CMD_FM_DT2:
    case DIV_CMD_FM_RS:
    case DIV_CMD_FM_KSR:
    case DIV_CMD_FM_VIB:
    case DIV_CMD_FM_SUS:
    case DIV_CMD_FM_WS:
    case DIV_CMD_FM_SSG:
    case DIV_CMD_FM_REV:
    case DIV_CMD_FM_EG_SHIFT:
    case DIV_CMD_FM_MULT:
    case DIV_CMD_FM_FINE:
    case DIV_CMD_AY_IO_WRITE:
    case DIV_CMD_AY_AUTO_PWM:
    case DIV_CMD_SURROUND_PANNING:
    case DIV_CMD_SU_SWEEP_PERIOD_LOW:
    case DIV_CMD_SU_SWEEP_PERIOD_HIGH:
    case DIV_CMD_SU_SWEEP_BOUND:
    case DIV_CMD_SU_SWEEP_ENABLE:
    case DIV_CMD_SNES_ECHO_FIR:
    case DIV_CMD_ES5506_FILTER_K1_SLIDE:
    case DIV_CMD_ES5506_FILTER_K2_SLIDE:
    case DIV_CMD_ES5506_ENVELOPE_K1RAMP:
    case DIV_CMD_ES5506_ENVELOPE_K2RAMP:
    case DIV_CMD_ESFM_OP_PANNING:
    case DIV_CMD_ESFM_OUTLVL:
    case DIV_CMD_ESFM_MODIN:
    case DIV_CMD_ESFM_ENV_DELAY:
    case DIV_CMD_POWERNOISE_COUNTER_LOAD:
    case DIV_CMD_POWERNOISE_IO_WRITE:
    case DIV_CMD_BIFURCATOR_STATE_LOAD:
    case DIV_CMD_BIFURCATOR_PARAMETER:
    case DIV_CMD_SID3_LFSR_FEEDBACK_BITS:
    case DIV_CMD_SID3_FILTER_DISTORTION:
    case DIV_CMD_SID3_FILTER_OUTPUT_VOLUME:
    case DIV_CMD_C64_PW_SLIDE:
    case DIV_CMD_C64_CUTOFF_SLIDE:
      return 2;
    case DIV_CMD_C64_FINE_DUTY:
    case DIV_CMD_C64_FINE_CUTOFF:
    case DIV_CMD_LYNX_LFSR_LOAD:
    case DIV_CMD_QSOUND_ECHO_DELAY:
    case DIV_CMD_ES5506_ENVELOPE_COUNT:
      return 2;
    case DIV_CMD_ES5506_FILTER_K1:
    case DIV_CMD_ES5506_FILTER_K2:
      return 4;
    case DIV_CMD_FM_FIXFREQ:
      return 2;
    case DIV_CMD_NES_SWEEP:
      return 2;
    case DIV_CMD_SAMPLE_POS:
      return 4;
    default:
      return 0;
  }
  return 0;
}

int DivCS::getInsLength(unsigned char ins, unsigned char ext, unsigned char* speedDial) {
  switch (ins) {
    case 0xb8: // ins
    case 0xc0: // pre porta
    case 0xc3: // vib range
    case 0xc4: // vib shape
    case 0xc5: // pitch
    case 0xc6: // arpeggio
    case 0xc7: // volume
    case 0xca: // legato
    case 0xfd: // waitc
      return 2;
    case 0xcf: // pan
    case 0xc2: // vibrato
    case 0xc8: // vol slide
    case 0xc9: // porta
      return 3;
    // speed dial commands
    case 0xd0: case 0xd1: case 0xd2: case 0xd3:
    case 0xd4: case 0xd5: case 0xd6: case 0xd7:
    case 0xd8: case 0xd9: case 0xda: case 0xdb:
    case 0xdc: case 0xdd: case 0xde: case 0xdf:
      if (speedDial==NULL) return 0;
      return 1+getCmdLength(speedDial[ins&15]);
    case 0xf0: // opt
      return 4;
    case 0xf7: // cmd
      // determine length from secondary
      if (ext==0) return 0;
      return 2+getCmdLength(ext);
    case 0xf8: // call
    case 0xfc: // waits
      return 3;
    case 0xf4: // callsym
    case 0xf5: // calli
    case 0xfa: // jmp
    case 0xfb: // rate
    case 0xcb: // volporta
      return 5;
  }
  return 1;
}

void writeCommandValues(SafeWriter* w, const DivCommand& c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.value==DIV_NOTE_NULL) {
        w->writeC(0xb4);
      } else {
        w->writeC(CLAMP(c.value+60,0,0xb3));
      }
      break;
    case DIV_CMD_NOTE_OFF:
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
    case DIV_CMD_INSTRUMENT:
    case DIV_CMD_PRE_PORTA:
    case DIV_CMD_HINT_VIBRATO:
    case DIV_CMD_HINT_VIBRATO_RANGE:
    case DIV_CMD_HINT_VIBRATO_SHAPE:
    case DIV_CMD_HINT_PITCH:
    case DIV_CMD_HINT_ARPEGGIO:
    case DIV_CMD_HINT_VOLUME:
    case DIV_CMD_HINT_PORTA:
    case DIV_CMD_HINT_VOL_SLIDE:
    case DIV_CMD_HINT_VOL_SLIDE_TARGET:
    case DIV_CMD_HINT_LEGATO:
    case DIV_CMD_HINT_TREMOLO:
    case DIV_CMD_HINT_PANBRELLO:
    case DIV_CMD_HINT_PAN_SLIDE:
    case DIV_CMD_HINT_PANNING:
      w->writeC((unsigned char)c.cmd+0xb4);
      break;
    default:
      w->writeC(0xf7);
      w->writeC(c.cmd);
      break;
  }
  switch (c.cmd) {
    case DIV_CMD_HINT_LEGATO:
      if (c.value==DIV_NOTE_NULL) {
        w->writeC(0xff);
      } else {
        w->writeC(c.value+60);
      }
      break;
    case DIV_CMD_NOTE_ON:
    case DIV_CMD_NOTE_OFF:
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      break;
    case DIV_CMD_INSTRUMENT:
    case DIV_CMD_HINT_VIBRATO_RANGE:
    case DIV_CMD_HINT_VIBRATO_SHAPE:
    case DIV_CMD_HINT_PITCH:
    case DIV_CMD_HINT_VOLUME:
    case DIV_CMD_HINT_TREMOLO:
    case DIV_CMD_HINT_PANBRELLO:
    case DIV_CMD_HINT_PAN_SLIDE:
    case DIV_CMD_HINT_ARPEGGIO:
      w->writeC(c.value);
      break;
    case DIV_CMD_HINT_PANNING:
    case DIV_CMD_HINT_VIBRATO:
    case DIV_CMD_HINT_PORTA:
      w->writeC(c.value);
      w->writeC(c.value2);
      break;
    case DIV_CMD_PRE_PORTA:
      w->writeC((c.value?0x80:0)|(c.value2?0x40:0));
      break;
    case DIV_CMD_HINT_VOL_SLIDE:
      w->writeS(c.value);
      break;
    case DIV_CMD_HINT_VOL_SLIDE_TARGET:
      w->writeS(c.value);
      w->writeS(c.value2);
      break;
    case DIV_CMD_SAMPLE_MODE:
    case DIV_CMD_SAMPLE_FREQ:
    case DIV_CMD_SAMPLE_BANK:
    case DIV_CMD_SAMPLE_DIR:
    case DIV_CMD_FM_HARD_RESET:
    case DIV_CMD_FM_LFO:
    case DIV_CMD_FM_LFO_WAVE:
    case DIV_CMD_FM_LFO2:
    case DIV_CMD_FM_LFO2_WAVE:
    case DIV_CMD_FM_FB:
    case DIV_CMD_FM_EXTCH:
    case DIV_CMD_FM_AM_DEPTH:
    case DIV_CMD_FM_PM_DEPTH:
    case DIV_CMD_STD_NOISE_FREQ:
    case DIV_CMD_STD_NOISE_MODE:
    case DIV_CMD_WAVE:
    case DIV_CMD_GB_SWEEP_TIME:
    case DIV_CMD_GB_SWEEP_DIR:
    case DIV_CMD_PCE_LFO_MODE:
    case DIV_CMD_PCE_LFO_SPEED:
    case DIV_CMD_NES_DMC:
    case DIV_CMD_C64_CUTOFF:
    case DIV_CMD_C64_RESONANCE:
    case DIV_CMD_C64_FILTER_MODE:
    case DIV_CMD_C64_RESET_TIME:
    case DIV_CMD_C64_RESET_MASK:
    case DIV_CMD_C64_FILTER_RESET:
    case DIV_CMD_C64_DUTY_RESET:
    case DIV_CMD_C64_EXTENDED:
    case DIV_CMD_AY_ENVELOPE_SET:
    case DIV_CMD_AY_ENVELOPE_LOW:
    case DIV_CMD_AY_ENVELOPE_HIGH:
    case DIV_CMD_AY_ENVELOPE_SLIDE:
    case DIV_CMD_AY_NOISE_MASK_AND:
    case DIV_CMD_AY_NOISE_MASK_OR:
    case DIV_CMD_AY_AUTO_ENVELOPE:
    case DIV_CMD_FDS_MOD_DEPTH:
    case DIV_CMD_FDS_MOD_HIGH:
    case DIV_CMD_FDS_MOD_LOW:
    case DIV_CMD_FDS_MOD_POS:
    case DIV_CMD_FDS_MOD_WAVE:
    case DIV_CMD_SAA_ENVELOPE:
    case DIV_CMD_AMIGA_FILTER:
    case DIV_CMD_AMIGA_AM:
    case DIV_CMD_AMIGA_PM:
    case DIV_CMD_MACRO_OFF:
    case DIV_CMD_MACRO_ON:
    case DIV_CMD_MACRO_RESTART:
    case DIV_CMD_HINT_ARP_TIME:
    case DIV_CMD_QSOUND_ECHO_FEEDBACK:
    case DIV_CMD_QSOUND_ECHO_LEVEL:
    case DIV_CMD_QSOUND_SURROUND:
    case DIV_CMD_X1_010_ENVELOPE_SHAPE:
    case DIV_CMD_X1_010_ENVELOPE_ENABLE:
    case DIV_CMD_X1_010_ENVELOPE_MODE:
    case DIV_CMD_X1_010_ENVELOPE_PERIOD:
    case DIV_CMD_X1_010_ENVELOPE_SLIDE:
    case DIV_CMD_X1_010_AUTO_ENVELOPE:
    case DIV_CMD_X1_010_SAMPLE_BANK_SLOT:
    case DIV_CMD_WS_SWEEP_TIME:
    case DIV_CMD_WS_SWEEP_AMOUNT:
    case DIV_CMD_N163_WAVE_POSITION:
    case DIV_CMD_N163_WAVE_LENGTH:
    case DIV_CMD_N163_WAVE_UNUSED1:
    case DIV_CMD_N163_WAVE_UNUSED2:
    case DIV_CMD_N163_WAVE_LOADPOS:
    case DIV_CMD_N163_WAVE_LOADLEN:
    case DIV_CMD_N163_WAVE_UNUSED3:
    case DIV_CMD_N163_CHANNEL_LIMIT:
    case DIV_CMD_N163_GLOBAL_WAVE_LOAD:
    case DIV_CMD_N163_GLOBAL_WAVE_LOADPOS:
    case DIV_CMD_N163_UNUSED4:
    case DIV_CMD_N163_UNUSED5:
    case DIV_CMD_SU_SYNC_PERIOD_LOW:
    case DIV_CMD_SU_SYNC_PERIOD_HIGH:
    case DIV_CMD_ADPCMA_GLOBAL_VOLUME:
    case DIV_CMD_SNES_ECHO:
    case DIV_CMD_SNES_PITCH_MOD:
    case DIV_CMD_SNES_INVERT:
    case DIV_CMD_SNES_GAIN_MODE:
    case DIV_CMD_SNES_GAIN:
    case DIV_CMD_SNES_ECHO_ENABLE:
    case DIV_CMD_SNES_ECHO_DELAY:
    case DIV_CMD_SNES_ECHO_VOL_LEFT:
    case DIV_CMD_SNES_ECHO_VOL_RIGHT:
    case DIV_CMD_SNES_ECHO_FEEDBACK:
    case DIV_CMD_NES_ENV_MODE:
    case DIV_CMD_NES_LENGTH:
    case DIV_CMD_NES_COUNT_MODE:
    case DIV_CMD_FM_AM2_DEPTH:
    case DIV_CMD_FM_PM2_DEPTH:
    case DIV_CMD_ES5506_ENVELOPE_LVRAMP:
    case DIV_CMD_ES5506_ENVELOPE_RVRAMP:
    case DIV_CMD_ES5506_PAUSE:
    case DIV_CMD_ES5506_FILTER_MODE:
    case DIV_CMD_SNES_GLOBAL_VOL_LEFT:
    case DIV_CMD_SNES_GLOBAL_VOL_RIGHT:
    case DIV_CMD_NES_LINEAR_LENGTH:
    case DIV_CMD_EXTERNAL:
    case DIV_CMD_C64_AD:
    case DIV_CMD_C64_SR:
    case DIV_CMD_DAVE_HIGH_PASS:
    case DIV_CMD_DAVE_RING_MOD:
    case DIV_CMD_DAVE_SWAP_COUNTERS:
    case DIV_CMD_DAVE_LOW_PASS:
    case DIV_CMD_DAVE_CLOCK_DIV:
    case DIV_CMD_MINMOD_ECHO:
    case DIV_CMD_FDS_MOD_AUTO:
    case DIV_CMD_FM_OPMASK:
    case DIV_CMD_MULTIPCM_MIX_FM:
    case DIV_CMD_MULTIPCM_MIX_PCM:
    case DIV_CMD_MULTIPCM_LFO:
    case DIV_CMD_MULTIPCM_VIB:
    case DIV_CMD_MULTIPCM_AM:
    case DIV_CMD_MULTIPCM_AR:
    case DIV_CMD_MULTIPCM_D1R:
    case DIV_CMD_MULTIPCM_DL:
    case DIV_CMD_MULTIPCM_D2R:
    case DIV_CMD_MULTIPCM_RC:
    case DIV_CMD_MULTIPCM_RR:
    case DIV_CMD_MULTIPCM_DAMP:
    case DIV_CMD_MULTIPCM_PSEUDO_REVERB:
    case DIV_CMD_MULTIPCM_LFO_RESET:
    case DIV_CMD_MULTIPCM_LEVEL_DIRECT:
    case DIV_CMD_SID3_SPECIAL_WAVE:
    case DIV_CMD_SID3_RING_MOD_SRC:
    case DIV_CMD_SID3_HARD_SYNC_SRC:
    case DIV_CMD_SID3_PHASE_MOD_SRC:
    case DIV_CMD_SID3_WAVE_MIX:
    case DIV_CMD_SID3_1_BIT_NOISE:
    case DIV_CMD_SID3_CHANNEL_INVERSION:
    case DIV_CMD_SID3_FILTER_CONNECTION:
    case DIV_CMD_SID3_FILTER_MATRIX:
    case DIV_CMD_SID3_FILTER_ENABLE:
    case DIV_CMD_SID3_PHASE_RESET:
    case DIV_CMD_SID3_NOISE_PHASE_RESET:
    case DIV_CMD_SID3_ENVELOPE_RESET:
    case DIV_CMD_SID3_CUTOFF_SCALING:
    case DIV_CMD_SID3_RESONANCE_SCALING:
    case DIV_CMD_WS_GLOBAL_SPEAKER_VOLUME:
      w->writeC(c.value);
      break;
    case DIV_CMD_FM_TL:
    case DIV_CMD_FM_AM:
    case DIV_CMD_FM_AR:
    case DIV_CMD_FM_DR:
    case DIV_CMD_FM_SL:
    case DIV_CMD_FM_D2R:
    case DIV_CMD_FM_RR:
    case DIV_CMD_FM_DT:
    case DIV_CMD_FM_DT2:
    case DIV_CMD_FM_RS:
    case DIV_CMD_FM_KSR:
    case DIV_CMD_FM_VIB:
    case DIV_CMD_FM_SUS:
    case DIV_CMD_FM_WS:
    case DIV_CMD_FM_SSG:
    case DIV_CMD_FM_REV:
    case DIV_CMD_FM_EG_SHIFT:
    case DIV_CMD_FM_MULT:
    case DIV_CMD_FM_FINE:
    case DIV_CMD_AY_IO_WRITE:
    case DIV_CMD_AY_AUTO_PWM:
    case DIV_CMD_SURROUND_PANNING:
    case DIV_CMD_SU_SWEEP_PERIOD_LOW:
    case DIV_CMD_SU_SWEEP_PERIOD_HIGH:
    case DIV_CMD_SU_SWEEP_BOUND:
    case DIV_CMD_SU_SWEEP_ENABLE:
    case DIV_CMD_SNES_ECHO_FIR:
    case DIV_CMD_ES5506_FILTER_K1_SLIDE:
    case DIV_CMD_ES5506_FILTER_K2_SLIDE:
    case DIV_CMD_ES5506_ENVELOPE_K1RAMP:
    case DIV_CMD_ES5506_ENVELOPE_K2RAMP:
    case DIV_CMD_ESFM_OP_PANNING:
    case DIV_CMD_ESFM_OUTLVL:
    case DIV_CMD_ESFM_MODIN:
    case DIV_CMD_ESFM_ENV_DELAY:
    case DIV_CMD_POWERNOISE_COUNTER_LOAD:
    case DIV_CMD_POWERNOISE_IO_WRITE:
    case DIV_CMD_BIFURCATOR_STATE_LOAD:
    case DIV_CMD_BIFURCATOR_PARAMETER:
    case DIV_CMD_SID3_LFSR_FEEDBACK_BITS:
    case DIV_CMD_SID3_FILTER_DISTORTION:
    case DIV_CMD_SID3_FILTER_OUTPUT_VOLUME:
    case DIV_CMD_C64_PW_SLIDE:
    case DIV_CMD_C64_CUTOFF_SLIDE:
      w->writeC(c.value);
      w->writeC(c.value2);
      break;
    case DIV_CMD_C64_FINE_DUTY:
    case DIV_CMD_C64_FINE_CUTOFF:
    case DIV_CMD_LYNX_LFSR_LOAD:
    case DIV_CMD_QSOUND_ECHO_DELAY:
    case DIV_CMD_ES5506_ENVELOPE_COUNT:
      w->writeS(c.value);
      break;
    case DIV_CMD_ES5506_FILTER_K1:
    case DIV_CMD_ES5506_FILTER_K2:
      w->writeS(c.value);
      w->writeS(c.value2);
      break;
    case DIV_CMD_FM_FIXFREQ:
      w->writeS((c.value<<12)|(c.value2&0x7ff));
      break;
    case DIV_CMD_NES_SWEEP:
      w->writeC((c.value?8:0)|(c.value2&0x77));
      break;
    case DIV_CMD_SAMPLE_POS:
      w->writeI(c.value);
      break;
    default:
      logW("unimplemented command %s!",cmdName[c.cmd]);
      break;
  }
  // padding (TODO: optimize)
  while (w->tell()&7) w->writeC(0);
}

#define _EXT(b,x,l) (((size_t)((x)+1)<(size_t)(l))?(b[(x)+1]):0)

using namespace DivCS;

int estimateBlockSize(unsigned char* buf, size_t len, unsigned char* speedDial) {
  int ret=0;
  for (size_t i=0; i<len; i+=8) {
    ret+=getInsLength(buf[i],buf[i+1],speedDial);
  }
  return ret;
}


void reloc(unsigned char* buf, size_t len, unsigned int sourceAddr, unsigned int destAddr, unsigned char* speedDial) {
  unsigned int delta=destAddr-sourceAddr;
  for (size_t i=0; i<len;) {
    int insLen=getInsLength(buf[i],_EXT(buf,i,len),speedDial);
    if (insLen<1) {
      logE("INS %x NOT IMPLEMENTED...",buf[i]);
      break;
    }
    switch (buf[i]) {
      case 0xf5: // calli
      case 0xfa: { // jmp
        unsigned int addr=buf[i+1]|(buf[i+2]<<8)|(buf[i+3]<<16)|(buf[i+4]<<24);
        addr+=delta;
        buf[i+1]=addr&0xff;
        buf[i+2]=(addr>>8)&0xff;
        buf[i+3]=(addr>>16)&0xff;
        buf[i+4]=(addr>>24)&0xff;
        break;
      }
      case 0xf8: { // call
        unsigned short addr=buf[i+1]|(buf[i+2]<<8);
        addr+=delta;
        buf[i+1]=addr&0xff;
        buf[i+2]=(addr>>8)&0xff;
        break;
      }
    }
    i+=insLen;
  }
}

SafeWriter* stripNops(SafeWriter* s) {
  std::unordered_map<unsigned int,unsigned int> addrTable;
  SafeWriter* oldStream=s;
  unsigned char* buf=oldStream->getFinalBuf();
  s=new SafeWriter;
  s->init();

  // prepare address map
  size_t addr=0;
  for (size_t i=0; i<oldStream->size(); i+=8) {
    addrTable[i]=addr;
    if (buf[i]!=0xf1) addr+=8;
  }

  // translate addresses
  for (size_t i=0; i<oldStream->size(); i+=8) {
    switch (buf[i]) {
      case 0xf5: // calli
      case 0xfa: { // jmp
        unsigned int addr=buf[i+1]|(buf[i+2]<<8)|(buf[i+3]<<16)|(buf[i+4]<<24);
        try {
          addr=addrTable[addr];
          buf[i+1]=addr&0xff;
          buf[i+2]=(addr>>8)&0xff;
          buf[i+3]=(addr>>16)&0xff;
          buf[i+4]=(addr>>24)&0xff;
        } catch (std::out_of_range& e) {
          logW("address %x is not mappable!",addr);
        }
        break;
      }
      case 0xf8: { // call
        unsigned int addr=buf[i+1]|(buf[i+2]<<8);
        try {
          addr=addrTable[addr];
          buf[i+1]=addr&0xff;
          buf[i+2]=(addr>>8)&0xff;
        } catch (std::out_of_range& e) {
          logW("address %x is not mappable!",addr);
        }
        break;
      }
    }
    if (buf[i]!=0xf1) {
      s->write(&buf[i],8);
    }
  }
  
  oldStream->finish();
  delete oldStream;
  return s;
}

struct BlockMatch {
  size_t orig, block;
  unsigned int len;
  bool done;
  BlockMatch(size_t o, size_t b, unsigned int l):
    orig(o), block(b), len(l), done(false) {}
  BlockMatch():
    orig(0), block(0), len(0), done(false) {}
};

#define OVERLAPS(a1,a2,b1,b2) ((b1)<(a2) && (b2)>(a1))

#define MIN_MATCH_SIZE 16

// TODO:
// - see if we can optimize even more
SafeWriter* findSubBlocks(SafeWriter* stream, std::vector<SafeWriter*>& subBlocks, unsigned char* speedDial) {
  unsigned char* buf=stream->getFinalBuf();
  size_t matchSize=MIN_MATCH_SIZE;
  std::vector<BlockMatch> matches;

  // repeat until we run out of matches
  while (true) {
    matchSize=MIN_MATCH_SIZE;
    matches.clear();

    // fast match algorithm
    // search for small matches, and then find bigger ones
    for (size_t i=0; i<stream->size(); i+=8) {
      for (size_t j=i+matchSize; j<stream->size(); j+=8) {
        if (memcmp(&buf[i],&buf[j],matchSize)==0) {
          // store this match for later
          matches.push_back(BlockMatch(i,j,matchSize));
        }
      }
    }

    logD("%d candidates",(int)matches.size());

    // quit if there isn't anything
    if (matches.empty()) return stream;

    // search for bigger matches
    bool wantMore=true;
    do {
      wantMore=false;
      matchSize+=8;
      for (size_t i=0; i<matches.size(); i++) {
        BlockMatch& b=matches[i];

        // don't do anything if this match is done
        if (b.done) continue;

        // stop if this match is near the edge
        if ((b.orig+matchSize)>stream->size() || (b.block+matchSize)>stream->size()) {
          b.done=true;
          continue;
        }

        // check
        if (memcmp(&buf[b.orig],&buf[b.block],matchSize)==0) {
          // this match may be bigger
          b.len=matchSize;
          wantMore=true;
        } else {
          // this is the max size
          b.done=true;
        }
      }
    } while (wantMore);

    // first stage done
    // set done to false unless:
    // - this match overlaps with itself
    // - this block only consists of calls
    size_t nonOverlapCount=0;
    for (BlockMatch& i: matches) {
      i.done=false;
      if (OVERLAPS(i.orig,i.orig+i.len,i.block,i.block+i.len)) {
        // self-overlapping
        i.done=true;
      } else {
        bool onlyCalls=true;
        for (size_t j=i.orig; j<i.orig+i.len; j+=8) {
          if (buf[j]!=0xf4) {
            onlyCalls=false;
            break;
          }
        }
        if (onlyCalls) {
          i.done=true;
        } else {
          nonOverlapCount++;
        }
      }
    }

    logD("%d good candidates",(int)nonOverlapCount);

    // quit if there isn't anything
    if (!nonOverlapCount) return stream;

    // work on largest matches
    // progress to smaller ones until we run out of them
    logD("largest match: %d",(int)matchSize);

    std::vector<BlockMatch> workMatches;
    bool newBlocks=false;

    // try with a smaller size
    matchSize=0;
    for (BlockMatch& i: matches) {
      if (i.done) continue;
      if (i.len>matchSize) matchSize=i.len;
    }

    workMatches.clear();
    // find matches with matching size
    for (BlockMatch& i: matches) {
      if (i.len==matchSize) {
        // mark it as done and push it
        workMatches.push_back(i);
        i.done=true;
      }
    }

    // check which sub-blocks are viable to make
    size_t lastOrig=SIZE_MAX;
    size_t lastOrigOff=0;
    int gains=0;
    int blockSize=0;
    for (size_t i=0; i<=workMatches.size(); i++) {
      BlockMatch b(SIZE_MAX,SIZE_MAX,0);
      if (i<workMatches.size()) b=workMatches[i];
      // unlikely
      if (b.done) continue;

      if (b.orig!=lastOrig) {
        if (lastOrig!=SIZE_MAX) {
          // commit previous block and start new one
          logV("%x gains: %d",(int)lastOrig,gains);
          if (gains<=0) {
            // don't make a sub-block for these matches since we only have loss
            logV("(LOSSES!)");
            for (size_t j=lastOrigOff; j<i; j++) {
              workMatches[j].done=true;
            }
          }
        }
        lastOrig=b.orig;
        lastOrigOff=i;
        if (lastOrig!=SIZE_MAX) {
          blockSize=estimateBlockSize(&buf[b.orig],b.len,speedDial);
        } else {
          blockSize=0;
        }
        gains=-4;
      }
      gains+=(blockSize-3);
    }

    // make sub-blocks
    lastOrig=SIZE_MAX;
    size_t subBlockID=subBlocks.size();
    for (BlockMatch& i: workMatches) {
      // skip invalid matches (yes, this can happen)
      if (i.done) continue;

      // create new sub-block if necessary
      if (i.orig!=lastOrig) {
        subBlockID=subBlocks.size();
        newBlocks=true;
        logV("new sub-block %d",(int)subBlockID);

        // isolate this sub-block
        SafeWriter* newBlock=new SafeWriter;
        newBlock->init();
        newBlock->write(&buf[i.orig],i.len);
        newBlock->writeC(0xf9); // ret
        // padding
        newBlock->writeC(0);
        newBlock->writeC(0);
        newBlock->writeC(0);
        newBlock->writeC(0);
        newBlock->writeC(0);
        newBlock->writeC(0);
        newBlock->writeC(0);
        subBlocks.push_back(newBlock);
        lastOrig=i.orig;

        // insert call on the original block
        buf[i.orig]=0xf4;
        buf[i.orig+1]=subBlockID&0xff;
        buf[i.orig+2]=(subBlockID>>8)&0xff;
        buf[i.orig+3]=(subBlockID>>16)&0xff;
        buf[i.orig+4]=(subBlockID>>24)&0xff;
        buf[i.orig+5]=0;
        buf[i.orig+6]=0;
        buf[i.orig+7]=0;

        // replace the rest with nop
        for (size_t j=i.orig+8; j<i.orig+i.len; j++) {
          buf[j]=0xf1;
        }
      }

      // set match to the last sub-block
      buf[i.block]=0xf4;
      buf[i.block+1]=subBlockID&0xff;
      buf[i.block+2]=(subBlockID>>8)&0xff;
      buf[i.block+3]=(subBlockID>>16)&0xff;
      buf[i.block+4]=(subBlockID>>24)&0xff;
      buf[i.block+5]=0;
      buf[i.block+6]=0;
      buf[i.block+7]=0;

      // replace the rest with nop
      for (size_t j=i.block+8; j<i.block+i.len; j++) {
        buf[j]=0xf1;
      }

      // invalidate overlapping work matches
      for (BlockMatch& j: workMatches) {
        if (j.orig!=i.orig) {
          j.done=true;
        }
        if (OVERLAPS(i.block,i.block+i.len,j.block,j.block+j.len)) {
          j.done=true;
        }
      }

      // invalidate overlapping matches
      for (BlockMatch& j: matches) {
        if (OVERLAPS(i.orig,i.orig+i.len,j.orig,j.orig+j.len) ||
            OVERLAPS(i.orig,i.orig+i.len,j.block,j.block+j.len) ||
            OVERLAPS(i.block,i.block+i.len,j.orig,j.orig+j.len) ||
            OVERLAPS(i.block,i.block+i.len,j.block,j.block+j.len)) {
          j.done=true;
        }
      }
    }

    logV("done!");

    // get out if we haven't made any blocks
    if (!newBlocks) break;

    // remove nop's
    stream=stripNops(stream);
    buf=stream->getFinalBuf();

    logV("doing it again...");
  }

  return stream;
}

SafeWriter* packStream(SafeWriter* s, unsigned char* speedDial) {
  std::unordered_map<unsigned int,unsigned int> addrTable;
  SafeWriter* oldStream=s;
  unsigned char* buf=oldStream->getFinalBuf();
  s=new SafeWriter;
  s->init();

  // prepare address map
  size_t addr=0;
  for (size_t i=0; i<oldStream->size(); i+=8) {
    addrTable[i]=addr;
    addr+=getInsLength(buf[i],_EXT(buf,i,oldStream->size()),speedDial);
  }

  // translate addresses and write stream
  for (size_t i=0; i<oldStream->size(); i+=8) {
    int insLen=getInsLength(buf[i],_EXT(buf,i,oldStream->size()),speedDial);
    if (insLen<1) {
      logE("INS %x NOT IMPLEMENTED...",buf[i]);
      break;
    }
    switch (buf[i]) {
      case 0xf5: // calli
      case 0xfa: { // jmp
        unsigned int addr=buf[i+1]|(buf[i+2]<<8)|(buf[i+3]<<16)|(buf[i+4]<<24);
        try {
          addr=addrTable[addr];
          buf[i+1]=addr&0xff;
          buf[i+2]=(addr>>8)&0xff;
          buf[i+3]=(addr>>16)&0xff;
          buf[i+4]=(addr>>24)&0xff;
        } catch (std::out_of_range& e) {
          logW("address %x is not mappable!",addr);
        }
        break;
      }
      case 0xf8: { // call
        unsigned int addr=buf[i+1]|(buf[i+2]<<8);
        try {
          addr=addrTable[addr];
          buf[i+1]=addr&0xff;
          buf[i+2]=(addr>>8)&0xff;
        } catch (std::out_of_range& e) {
          logW("address %x is not mappable!",addr);
        }
        break;
      }
    }
    s->write(&buf[i],insLen);
  }
  
  oldStream->finish();
  delete oldStream;
  return s;
}

SafeWriter* DivEngine::saveCommand(DivCSProgress* progress, unsigned int disablePasses) {
  stop();
  repeatPattern=false;
  shallStop=false;
  setOrder(0);
  BUSY_BEGIN_SOFT;
  // determine loop point
  int loopOrder=0;
  int loopRow=0;
  int loopEnd=0;
  walkSong(loopOrder,loopRow,loopEnd);
  logI("loop point: %d %d",loopOrder,loopRow);

  int cmdPopularity[256];
  int delayPopularity[256];

  int sortedCmdPopularity[16];
  int sortedDelayPopularity[16];
  unsigned char sortedCmd[16];
  unsigned char sortedDelay[16];
  
  SafeWriter* chanStream[DIV_MAX_CHANS];
  unsigned int chanStreamOff[DIV_MAX_CHANS];
  std::vector<size_t> tickPos[DIV_MAX_CHANS];
  int loopTick=-1;

  memset(cmdPopularity,0,256*sizeof(int));
  memset(delayPopularity,0,256*sizeof(int));
  memset(chanStream,0,DIV_MAX_CHANS*sizeof(void*));
  memset(chanStreamOff,0,DIV_MAX_CHANS*sizeof(unsigned int));
  memset(sortedCmdPopularity,0,16*sizeof(int));
  memset(sortedDelayPopularity,0,16*sizeof(int));
  memset(sortedCmd,0,16);
  memset(sortedDelay,0,16);

  SafeWriter* w=new SafeWriter;
  w->init();

  // write header
  w->write("FCS",4);
  w->writeI(chans);
  // offsets
  for (int i=0; i<chans; i++) {
    chanStream[i]=new SafeWriter;
    chanStream[i]->init();
    w->writeI(0);
  }
  // preset delays and speed dial
  for (int i=0; i<32; i++) {
    w->writeC(0);
  }

  // play the song ourselves
  bool done=false;
  playSub(false);
  
  int tick=0;
  bool oldCmdStreamEnabled=cmdStreamEnabled;
  cmdStreamEnabled=true;
  double curDivider=divider;

  // PASS 0: play the song and log channel command streams
  while (!done) {
    for (int i=0; i<chans; i++) {
      tickPos[i].push_back(chanStream[i]->tell());
    }
    if (loopTick==-1) {
      if (loopOrder==curOrder && loopRow==curRow) {
        if ((ticks-((tempoAccum+virtualTempoN)/virtualTempoD))<=0) {
          logI("loop is on tick %d",tick);
          loopTick=tick;
          // marker
          for (int i=0; i<chans; i++) {
            chanStream[i]->writeC(0xf0);
            chanStream[i]->writeC(0x00);
            chanStream[i]->writeC(0x00);
            chanStream[i]->writeC(0x00);
            // padding
            chanStream[i]->writeC(0x00);
            chanStream[i]->writeC(0x00);
            chanStream[i]->writeC(0x00);
            chanStream[i]->writeC(0x00);
          }
        }
      }
    }
    if (nextTick(false,true) || !playing) {
      done=true;
      break;
    }
    // get command stream
    if (curDivider!=divider) {
      curDivider=divider;
      chanStream[0]->writeC(0xfb);
      chanStream[0]->writeI((int)(curDivider*65536));
      // padding
      chanStream[0]->writeC(0x00);
      chanStream[0]->writeC(0x00);
      chanStream[0]->writeC(0x00);
    }
    for (DivCommand& i: cmdStream) {
      switch (i.cmd) {
        // strip away hinted/useless commands
        case DIV_CMD_GET_VOLUME:
        case DIV_CMD_VOLUME:
        case DIV_CMD_PANNING:
        case DIV_CMD_NOTE_PORTA:
        case DIV_CMD_LEGATO:
        case DIV_CMD_PITCH:
        case DIV_CMD_PRE_NOTE:
          break;
        default:
          cmdPopularity[i.cmd]++;
          writeCommandValues(chanStream[i.chan],i);
          break;
      }
    }
    cmdStream.clear();
    for (int i=0; i<chans; i++) {
      chanStream[i]->writeC(0xfe);
      // padding
      chanStream[i]->writeC(0x00);
      chanStream[i]->writeC(0x00);
      chanStream[i]->writeC(0x00);
      chanStream[i]->writeC(0x00);
      chanStream[i]->writeC(0x00);
      chanStream[i]->writeC(0x00);
      chanStream[i]->writeC(0x00);
    }
    tick++;
  }
  if (!playing || loopTick<0) {
    for (int i=0; i<chans; i++) {
      chanStream[i]->writeC(0xff);
      // padding
      chanStream[i]->writeC(0x00);
      chanStream[i]->writeC(0x00);
      chanStream[i]->writeC(0x00);
      chanStream[i]->writeC(0x00);
      chanStream[i]->writeC(0x00);
      chanStream[i]->writeC(0x00);
      chanStream[i]->writeC(0x00);
    }
  } else {
    for (int i=0; i<chans; i++) {
      if ((int)tickPos[i].size()>loopTick) {
        chanStream[i]->writeC(0xfa);
        chanStream[i]->writeI(tickPos[i][loopTick]);
        logD("chan %d loop addr: %x",i,tickPos[i][loopTick]);
        // padding
        chanStream[i]->writeC(0x00);
        chanStream[i]->writeC(0x00);
        chanStream[i]->writeC(0x00);
      } else {
        logW("chan %d unable to find loop addr!",i);
        chanStream[i]->writeC(0xff);
        // padding
        chanStream[i]->writeC(0x00);
        chanStream[i]->writeC(0x00);
        chanStream[i]->writeC(0x00);
        chanStream[i]->writeC(0x00);
        chanStream[i]->writeC(0x00);
        chanStream[i]->writeC(0x00);
        chanStream[i]->writeC(0x00);
      }
    }
  }
  logV("%d",tick);
  cmdStreamEnabled=oldCmdStreamEnabled;

  remainingLoops=-1;
  playing=false;
  freelance=false;
  extValuePresent=false;
  BUSY_END;

  // PASS 1: optimize command calls
  if (!(disablePasses&1)) {
    // calculate command usage
    int sortCand=-1;
    int sortPos=0;
    while (sortPos<16) {
      sortCand=-1;
      for (int i=DIV_CMD_SAMPLE_MODE; i<256; i++) {
        if (cmdPopularity[i]) {
          if (sortCand==-1) {
            sortCand=i;
          } else if (cmdPopularity[sortCand]<cmdPopularity[i]) {
            sortCand=i;
          }
        }
      }
      if (sortCand==-1) break;

      sortedCmdPopularity[sortPos]=cmdPopularity[sortCand];
      sortedCmd[sortPos]=sortCand;
      cmdPopularity[sortCand]=0;
      sortPos++;
    }

    // set speed dial commands
    for (int h=0; h<chans; h++) {
      unsigned char* buf=chanStream[h]->getFinalBuf();
      for (size_t i=0; i<chanStream[h]->size(); i+=8) {
        if (buf[i]==0xf7) {
          // find whether this command is in speed dial
          for (int j=0; j<16; j++) {
            if (buf[i+1]==sortedCmd[j]) {
              buf[i]=0xd0+j;
              // move everything to the left
              for (int k=i+2; k<(int)i+8; k++) {
                buf[k-1]=buf[k];
              }
              break;
            }
          }
        }
      }
    }
  }

  // PASS 2: condense delays
  if (!(disablePasses&2)) {
    // calculate delay usage
    for (int h=0; h<chans; h++) {
      unsigned char* buf=chanStream[h]->getFinalBuf();
      int delayCount=0;
      for (size_t i=0; i<chanStream[h]->size(); i+=8) {
        if (buf[i]==0xfe) {
          delayCount++;
        } else {
          if (delayCount>1 && delayCount<=255) {
            delayPopularity[delayCount]++;
          }
          delayCount=0;
        }
      }
    }

    // preset delays
    int sortCand=-1;
    int sortPos=0;
    while (sortPos<16) {
      sortCand=-1;
      for (int i=0; i<256; i++) {
        if (delayPopularity[i]) {
          if (sortCand==-1) {
            sortCand=i;
          } else if (delayPopularity[sortCand]<delayPopularity[i]) {
            sortCand=i;
          }
        }
      }
      if (sortCand==-1) break;

      sortedDelayPopularity[sortPos]=delayPopularity[sortCand];
      sortedDelay[sortPos]=sortCand;
      delayPopularity[sortCand]=0;
      sortPos++;
    }


    // condense delays
    for (int h=0; h<chans; h++) {
      unsigned char* buf=chanStream[h]->getFinalBuf();
      int delayPos=-1;
      int delayCount=0;
      int delayLast=0;
      for (size_t i=0; i<chanStream[h]->size(); i+=8) {
        if (buf[i]==0xfe) {
          if (delayPos==-1) delayPos=i;
          delayCount++;
          delayLast=i;
        } else {
          // finish the last delay if any
          if (delayPos!=-1) {
            if (delayCount>1) {
              if (delayLast<delayPos) {
                logE("delayLast<delayPos! %d<%d",delayLast,delayPos);
              } else {
                // write condensed delay and fill the rest with nop
                if (delayCount>255) {
                  buf[delayPos++]=0xfc;
                  buf[delayPos++]=delayCount&0xff;
                  buf[delayPos++]=(delayCount>>8)&0xff;
                } else {
                  bool foundShort=false;
                  for (int j=0; j<16; j++) {
                    if (sortedDelay[j]==delayCount) {
                      buf[delayPos++]=0xe0+j;
                      foundShort=true;
                      break;
                    }
                  }
                  if (!foundShort) {
                    buf[delayPos++]=0xfd;
                    buf[delayPos++]=delayCount;
                  }
                }
                // padding
                while (delayPos&7) buf[delayPos++]=0;
                // fill with nop
                for (int j=delayPos; j<=delayLast; j++) {
                  buf[j]=0xf1;
                }
              }
            }
            delayPos=-1;
            delayCount=0;
          }
        }
      }
    }
  }

  // PASS 3: remove nop's
  // this includes modifying call addresses to compensate
  for (int h=0; h<chans; h++) {
    chanStream[h]=stripNops(chanStream[h]);
  }

  // PASS 4: find sub-blocks and isolate them
  if (!(disablePasses&4)) {
    for (int h=0; h<chans; h++) {
      std::vector<SafeWriter*> subBlocks;
      size_t beforeSize=chanStream[h]->size();
      
      // 6 is the minimum size that can be reliably optimized
      logI("finding sub-blocks in chan %d",h);
      chanStream[h]=findSubBlocks(chanStream[h],subBlocks,sortedCmd);
      // find sub-blocks within sub-blocks
      size_t subBlocksLast=0;
      size_t subBlocksLen=subBlocks.size();
      logI("finding sub-blocks within sub-blocks",h);
      while (subBlocksLast!=subBlocksLen) {
        logI("got %d blocks... starting from %d",(int)subBlocksLen,(int)subBlocksLast);
        for (size_t i=subBlocksLast; i<subBlocksLen; i++) {
          SafeWriter* newBlock=findSubBlocks(subBlocks[i],subBlocks,sortedCmd);
          subBlocks[i]=newBlock;
        }
        subBlocksLast=subBlocksLen;
        subBlocksLen=subBlocks.size();
      }

      // insert sub-blocks and resolve symbols
      logI("%d sub-blocks total",(int)subBlocks.size());
      std::vector<size_t> blockOff;
      chanStream[h]->seek(0,SEEK_END);
      for (size_t i=0; i<subBlocks.size(); i++) {
        SafeWriter* block=subBlocks[i];

        // check whether this block is duplicate
        int dupOf=-1;
        for (size_t j=0; j<i; j++) {
          if (subBlocks[j]->size()==subBlocks[i]->size()) {
            if (memcmp(subBlocks[j]->getFinalBuf(),subBlocks[i]->getFinalBuf(),subBlocks[j]->size())==0) {
              logW("we have one");
              dupOf=j;
              break;
            }
          }
        }

        if (dupOf>=0) {
          // push address of original block (discard duplicate)
          blockOff.push_back(blockOff[dupOf]);
        } else {
          // write sub-block
          blockOff.push_back(chanStream[h]->tell());
          logV("block size: %d",(int)block->size());
          assert(!(block->size()&7));
          chanStream[h]->write(block->getFinalBuf(),block->size());
        }
      }

      for (SafeWriter* block: subBlocks) {
        block->finish();
        delete block;
      }
      subBlocks.clear();

      // resolve symbols
      unsigned char* buf=chanStream[h]->getFinalBuf();
      for (size_t j=0; j<chanStream[h]->size(); j+=8) {
        if (buf[j]==0xf4) { // callsym
          unsigned int addr=buf[j+1]|(buf[j+2]<<8)|(buf[j+3]<<16)|(buf[j+4]<<24);
          if (addr<blockOff.size()) {
            // turn it into call
            addr=blockOff[addr];
            buf[j]=0xf8;
            buf[j+1]=addr&0xff;
            buf[j+2]=(addr>>8)&0xff;
            //buf[j+3]=(addr>>16)&0xff;
            //buf[j+4]=(addr>>24)&0xff;
          } else {
            logE("requested symbol %d is out of bounds!",addr);
          }
        }
      }

      size_t afterSize=chanStream[h]->size();
      logI("(before: %d - after: %d)",(int)beforeSize,(int)afterSize);
      assert(!(chanStream[h]->size()&7));
    }
  }

  // PASS 5: remove nop's (again)
  for (int h=0; h<chans; h++) {
    chanStream[h]=stripNops(chanStream[h]);
  }

  // PASS 6: pack streams
  for (int h=0; h<chans; h++) {
    chanStream[h]=packStream(chanStream[h],sortedCmd);
  }

  // write results
  for (int i=0; i<chans; i++) {
    chanStreamOff[i]=w->tell();
    logI("- %d: off %x size %ld",i,chanStreamOff[i],chanStream[i]->size());
    reloc(chanStream[i]->getFinalBuf(),chanStream[i]->size(),0,w->tell(),sortedCmd);
    w->write(chanStream[i]->getFinalBuf(),chanStream[i]->size());
    chanStream[i]->finish();
    delete chanStream[i];
  }

  w->seek(8,SEEK_SET);
  for (int i=0; i<chans; i++) {
    w->writeI(chanStreamOff[i]);
  }

  logD("delay popularity:");
  for (int i=0; i<16; i++) {
    w->writeC(sortedDelay[i]);
    if (sortedDelayPopularity[i]) logD("- %d: %d",sortedDelay[i],sortedDelayPopularity[i]);
  }

  logD("command popularity:");
  for (int i=0; i<16; i++) {
    w->writeC(sortedCmd[i]);
    if (sortedCmdPopularity[i]) logD("- %s ($%.2x): %d",cmdName[sortedCmd[i]],sortedCmd[i],sortedCmdPopularity[i]);
  }

  return w;
}
