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
#include <stack>
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
    case 0xc1: // arp time
    case 0xc3: // vib range
    case 0xc4: // vib shape
    case 0xc5: // pitch
    case 0xc6: // arpeggio
    case 0xc7: // volume
    case 0xca: // legato
    case 0xcc: // tremolo
    case 0xcd: // panbrello
    case 0xce: // pan slide
    case 0xdd: // waitc
    case 0xc2: // vibrato
      return 2;
    case 0xcf: // pan
    case 0xc8: // vol slide
    case 0xc9: // porta
      return 3;
    // speed dial commands
    case 0xe0: case 0xe1: case 0xe2: case 0xe3:
    case 0xe4: case 0xe5: case 0xe6: case 0xe7:
    case 0xe8: case 0xe9: case 0xea: case 0xeb:
    case 0xec: case 0xed: case 0xee: case 0xef:
      if (speedDial==NULL) return 0;
      return 1+getCmdLength(speedDial[ins&15]);
    case 0xd0: // opt
      return 4;
    case 0xd7: // cmd
      // determine length from secondary
      if (ext==0) return 0;
      return 2+getCmdLength(ext);
    case 0xd8: // call
    case 0xdc: // waits
      return 3;
    case 0xd4: // callsym
    case 0xd5: // calli
    case 0xda: // jmp
    case 0xdb: // rate
    case 0xcb: // volporta
      return 5;
  }
  return 1;
}

void writeCommandValues(SafeWriter* w, const DivCommand& c, bool bigEndian) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.value==DIV_NOTE_NULL) {
        w->writeC(0xb4);
      } else {
        w->writeC(CLAMP(c.value+60,0,0xb3));
      }
      break;
    case DIV_CMD_NOTE_OFF:
      w->writeC(0xb5);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      w->writeC(0xb6);
      break;
    case DIV_CMD_ENV_RELEASE:
      w->writeC(0xb7);
      break;
    case DIV_CMD_INSTRUMENT:
      w->writeC(0xb8);
      break;
    case DIV_CMD_PRE_PORTA:
      w->writeC(0xc0);
      break;
    case DIV_CMD_HINT_ARP_TIME:
      w->writeC(0xc1);
      break;
    case DIV_CMD_HINT_VIBRATO:
      w->writeC(0xc2);
      break;
    case DIV_CMD_HINT_VIBRATO_RANGE:
      w->writeC(0xc3);
      break;
    case DIV_CMD_HINT_VIBRATO_SHAPE:
      w->writeC(0xc4);
      break;
    case DIV_CMD_HINT_PITCH:
      w->writeC(0xc5);
      break;
    case DIV_CMD_HINT_ARPEGGIO:
      w->writeC(0xc6);
      break;
    case DIV_CMD_HINT_VOLUME:
      w->writeC(0xc7);
      break;
    case DIV_CMD_HINT_VOL_SLIDE:
      w->writeC(0xc8);
      break;
    case DIV_CMD_HINT_PORTA:
      w->writeC(0xc9);
      break;
    case DIV_CMD_HINT_LEGATO:
      w->writeC(0xca);
      break;
    case DIV_CMD_HINT_VOL_SLIDE_TARGET:
      w->writeC(0xcb);
      break;
    case DIV_CMD_HINT_TREMOLO:
      w->writeC(0xcc);
      break;
    case DIV_CMD_HINT_PANBRELLO:
      w->writeC(0xcd);
      break;
    case DIV_CMD_HINT_PAN_SLIDE:
      w->writeC(0xce);
      break;
    case DIV_CMD_HINT_PANNING:
      w->writeC(0xcf);
      break;
    default:
      w->writeC(0xd7);
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
    case DIV_CMD_HINT_ARP_TIME:
    case DIV_CMD_HINT_VIBRATO:
      w->writeC(c.value);
      break;
    case DIV_CMD_HINT_PANNING:
      w->writeC(c.value);
      w->writeC(c.value2);
      break;
    case DIV_CMD_HINT_PORTA: {
      unsigned char val=CLAMP(c.value+60,0,255);
      w->writeC(val);
      w->writeC(c.value2);
      break;
    }
    case DIV_CMD_PRE_PORTA:
      w->writeC((c.value?0x80:0)|(c.value2?0x40:0));
      break;
    case DIV_CMD_HINT_VOL_SLIDE:
      if (bigEndian) {
        w->writeS_BE(c.value);
      } else {
        w->writeS(c.value);
      }
      break;
    case DIV_CMD_HINT_VOL_SLIDE_TARGET:
      if (bigEndian) {
        w->writeS_BE(c.value);
        w->writeS_BE(c.value2);
      } else {
        w->writeS(c.value);
        w->writeS(c.value2);
      }
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
      if (bigEndian) {
        w->writeS_BE(c.value);
      } else {
        w->writeS(c.value);
      }
      break;
    case DIV_CMD_ES5506_FILTER_K1:
    case DIV_CMD_ES5506_FILTER_K2:
      if (bigEndian) {
        w->writeS_BE(c.value);
        w->writeS_BE(c.value2);
      } else {
        w->writeS(c.value);
        w->writeS(c.value2);
      }
      break;
    case DIV_CMD_FM_FIXFREQ:
      if (bigEndian) {
        w->writeS_BE((c.value<<12)|(c.value2&0x7ff));
      } else {
        w->writeS((c.value<<12)|(c.value2&0x7ff));
      }
      break;
    case DIV_CMD_NES_SWEEP:
      w->writeC((c.value?8:0)|(c.value2&0x77));
      break;
    case DIV_CMD_SAMPLE_POS:
      if (bigEndian) {
        w->writeI_BE(c.value);
      } else {
        w->writeI(c.value);
      }
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

void reloc8(unsigned char* buf, size_t len, unsigned int sourceAddr, unsigned int destAddr) {
  unsigned int delta=destAddr-sourceAddr;
  for (size_t i=0; i<len; i+=8) {
    switch (buf[i]) {
      case 0xd5: // calli
      case 0xda: { // jmp
        unsigned int addr=buf[i+1]|(buf[i+2]<<8)|(buf[i+3]<<16)|(buf[i+4]<<24);
        addr+=delta;
        buf[i+1]=addr&0xff;
        buf[i+2]=(addr>>8)&0xff;
        buf[i+3]=(addr>>16)&0xff;
        buf[i+4]=(addr>>24)&0xff;
        break;
      }
      case 0xd8: { // call
        unsigned int addr=buf[i+1]|(buf[i+2]<<8);
        addr+=delta;
        if (addr>0xffff) {
          buf[i]=0xd5;
          buf[i+1]=addr&0xff;
          buf[i+2]=(addr>>8)&0xff;
          buf[i+3]=(addr>>16)&0xff;
          buf[i+4]=(addr>>24)&0xff;
        } else {
          buf[i+1]=addr&0xff;
          buf[i+2]=(addr>>8)&0xff;
        }
        break;
      }
    }
  }
}

void reloc(unsigned char* buf, size_t len, unsigned int sourceAddr, unsigned int destAddr, unsigned char* speedDial, bool bigEndian) {
  unsigned int delta=destAddr-sourceAddr;
  for (size_t i=0; i<len;) {
    int insLen=getInsLength(buf[i],_EXT(buf,i,len),speedDial);
    if (insLen<1) {
      logE("INS %x NOT IMPLEMENTED...",buf[i]);
      break;
    }
    switch (buf[i]) {
      case 0xd5: // calli
      case 0xda: { // jmp
        unsigned int addr=buf[i+1]|(buf[i+2]<<8)|(buf[i+3]<<16)|(buf[i+4]<<24);
        addr+=delta;
        if (bigEndian) {
          buf[i+1]=(addr>>24)&0xff;
          buf[i+2]=(addr>>16)&0xff;
          buf[i+3]=(addr>>8)&0xff;
          buf[i+4]=addr&0xff;
        } else {
          buf[i+1]=addr&0xff;
          buf[i+2]=(addr>>8)&0xff;
          buf[i+3]=(addr>>16)&0xff;
          buf[i+4]=(addr>>24)&0xff;
        }
        break;
      }
      case 0xd8: { // call
        unsigned short addr=buf[i+1]|(buf[i+2]<<8);
        addr+=delta;
        if (bigEndian) {
          buf[i+1]=(addr>>8)&0xff;
          buf[i+2]=addr&0xff;
        } else {
          buf[i+1]=addr&0xff;
          buf[i+2]=(addr>>8)&0xff;
        }
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
    if (buf[i]!=0xd1) addr+=8;
  }

  // translate addresses
  for (size_t i=0; i<oldStream->size(); i+=8) {
    switch (buf[i]) {
      case 0xd5: // calli
      case 0xda: { // jmp
        unsigned int addr=buf[i+1]|(buf[i+2]<<8)|(buf[i+3]<<16)|(buf[i+4]<<24);
        assert(!(addr&7));
        if (addr>=oldStream->size()) {
          logE("OUT OF BOUNDS!");
          abort();
        }
        if (buf[addr]==0xd1) {
          logE("POINTS TO NOP");
          abort();
        }
        try {
          addr=addrTable[addr];
          buf[i+1]=addr&0xff;
          buf[i+2]=(addr>>8)&0xff;
          buf[i+3]=(addr>>16)&0xff;
          buf[i+4]=(addr>>24)&0xff;
        } catch (std::out_of_range& e) {
          logW("address %x is not mappable!",addr);
          abort();
        }
        break;
      }
      case 0xd8: { // call
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
    if (buf[i]!=0xd1) {
      s->write(&buf[i],8);
    }
  }
  
  oldStream->finish();
  delete oldStream;
  return s;
}

SafeWriter* stripNopsPacked(SafeWriter* s, unsigned char* speedDial, unsigned int* chanStreamOff) {
  std::unordered_map<unsigned int,unsigned int> addrTable;
  SafeWriter* oldStream=s;
  unsigned char* buf=oldStream->getFinalBuf();
  s=new SafeWriter;
  s->init();

  // prepare address map
  size_t addr=0;
  for (size_t i=0; i<oldStream->size();) {
    int insLen=getInsLength(buf[i],_EXT(buf,i,oldStream->size()),speedDial);
    if (insLen<1) {
      logE("INS %x NOT IMPLEMENTED...",buf[i]);
      break;
    }
    addrTable[i]=addr;
    if (buf[i]!=0xd1 && buf[i]!=0xd0) addr+=insLen;
    i+=insLen;
  }

  // translate addresses
  for (size_t i=0; i<oldStream->size();) {
    int insLen=getInsLength(buf[i],_EXT(buf,i,oldStream->size()),speedDial);
    if (insLen<1) {
      logE("INS %x NOT IMPLEMENTED...",buf[i]);
      break;
    }
    switch (buf[i]) {
      case 0xd0: // ext (for channel offsets)
        if (buf[i+3]==0) {
          int ch=buf[i+1];
          if (ch>=0 && ch<DIV_MAX_CHANS) {
            chanStreamOff[ch]=addrTable[i];
          }
        }
        break;
      case 0xd5: // calli
      case 0xda: { // jmp
        unsigned int addr=buf[i+1]|(buf[i+2]<<8)|(buf[i+3]<<8)|(buf[i+4]<<24);
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
      case 0xd8: { // call
        unsigned int addr=buf[i+1]|(buf[i+2]<<8);
        try {
          addr=addrTable[addr];
          buf[i+1]=addr&0xff;
          buf[i+2]=(addr>>8)&0xff;
          if (addr>0xffff) { // this may never happen but it's here just in case
            logW("address %x is out of range for 16-bit call!",addr);
          }
        } catch (std::out_of_range& e) {
          logW("address %x is not mappable!",addr);
        }
        break;
      }
    }
    if (buf[i]!=0xd1 && buf[i]!=0xd0) {
      s->write(&buf[i],insLen);
    }
    i+=insLen;
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

struct MatchBenefit {
  size_t index;
  int benefit;
  unsigned int len;
  MatchBenefit(size_t i, int b, unsigned int l):
    index(i), benefit(b), len(l) {}
  MatchBenefit():
    index(0), benefit(0), len(0) {}
};

#define OVERLAPS(a1,a2,b1,b2) ((b1)<(a2) && (b2)>(a1))

#define MIN_MATCH_SIZE 32

SafeWriter* findSubBlocks(SafeWriter* stream, std::vector<SafeWriter*>& subBlocks, unsigned char* speedDial, DivCSProgress* progress) {
  unsigned char* buf=stream->getFinalBuf();
  size_t matchSize=MIN_MATCH_SIZE;
  std::vector<BlockMatch> matches;
  std::vector<BlockMatch> workMatches;
  std::vector<size_t> origs;
  MatchBenefit bestBenefit;

  matches.clear();

  if (progress!=NULL) {
    progress->findTotal=stream->size();
    progress->optStage=0;
  }

  // fast match algorithm
  // search for small matches, and then find bigger ones
  logD("finding possible matches");
  for (size_t i=0; i<stream->size(); i+=8) {
    if (!(i&2047)) {
      if (progress!=NULL) progress->findCurrent=i;
    }
    bool storedOrig=false;
    for (size_t j=i+matchSize; j<stream->size(); j+=8) {
      if (memcmp(&buf[i],&buf[j],matchSize)==0) {
        if (!storedOrig) {
          // store index to the first match somewhere else for the sake of speed
          origs.push_back(matches.size());
          storedOrig=true;
        }
        // store this match for later
        matches.push_back(BlockMatch(i,j,matchSize));
      }
    }
  }

  logD("%d candidates",(int)matches.size());
  logD("%d origs",(int)origs.size());

  if (progress!=NULL) {
    if ((int)matches.size()>progress->optTotal) progress->optTotal=matches.size();
    progress->optCurrent=matches.size();
    progress->origCount=origs.size();
    progress->findCurrent=stream->size();
    progress->optStage=1;
  }

  // quit if there isn't anything
  if (matches.empty()) return stream;

  // search for bigger matches
  for (size_t i=0; i<matches.size(); i++) {
    if ((i&8191)==0) {
      logV("match %d of %d",i,(int)matches.size());
    }
    if ((i&1023)==0) {
      if (progress!=NULL) progress->expandCurrent=i;
    }
    BlockMatch& b=matches[i];

    size_t finalLen=b.len;
    size_t origPos=b.orig+b.len;
    size_t blockPos=b.block+b.len;
    while (true) {
      // origPos is guaranteed to be before blockPos
      if (blockPos>=stream->size()) {
        break;
      }

      if (buf[origPos]!=buf[blockPos]) {
        break;
      }
      origPos++;
      blockPos++;
      finalLen++;
    }

    finalLen&=~7;
    b.len=finalLen;
  }

  if (progress!=NULL) {
    progress->expandCurrent=matches.size();
    progress->optStage=2;
  }

  // new code MAN... WHY...
  // basically the workflow should be:
  // - test every block position
  //   - test every length from MIN_MATCH_SIZE to largest length
  //   - check for overlap, bad matches and all of that
  //     - for bad matches, fortunately we can use length for a speed-up... but first make it right
  //   - add weighted benefit to a list (DEBUG..... remove once it's stable)
  // - pick largest benefit from list
  // - make sub-blocks!!!
  logD("testing %d match groups for benefit",(int)origs.size());
  size_t origIndex=0;
  for (size_t i=0; i<origs.size(); i++) {
    size_t begin=origs[i];
    size_t end=(i+1<origs.size())?origs[i+1]:matches.size();
    size_t minSize=MIN_MATCH_SIZE;
    std::vector<BlockMatch> testLenMatches;

    if (progress!=NULL) progress->origCurrent=origIndex;

    origIndex++;

    if (!(i&255)) logV("orig %d of %d",(int)i,(int)origs.size());

    // test all lengths
    for (size_t len=minSize; true; len+=8) {
      testLenMatches.clear();
      // filter matches
      for (size_t _k=begin; _k<end; _k++) {
        BlockMatch& k=matches[_k];
        // match length shall be greater than or equal to current length
        if (len>k.len) continue;

        // check for bad matches, which include:
        // - match overlapping with itself
        // - block only consisting of calls
        // - block containing a ret, jmp or stop

        // 1. self-overlapping
        if (OVERLAPS(k.orig,k.orig+len,k.block,k.block+len)) continue;

        // 2. only calls and jmp/ret/stop
        bool metCriteria=true;
        for (size_t l=k.orig; l<k.orig+len; l+=8) {
          if (buf[l]==0xd4 || buf[l]==0xd5) {
            metCriteria=false;
            break;
          }
        }
        if (!metCriteria) continue;

        // 3. jmp/ret/stop
        for (size_t l=k.orig; l<k.orig+len; l+=8) {
          if (buf[l]==0xd9 || buf[l]==0xda || buf[l]==0xdf) {
            metCriteria=false;
            break;
          }
        }
        if (!metCriteria) continue;

        // all criteria met
        testLenMatches.push_back(k);
      }

      // get out if no further matches (trying with bigger sizes is guaranteed to fail)
      if (testLenMatches.empty()) {
        break;
      }

      // check for overlapping matches
      size_t overlapPos=testLenMatches[0].orig;
      size_t validCount=0;
      for (BlockMatch& k: testLenMatches) {
        //logV("test %d with %d",(int)overlapPos,(int)k.block);
        if (OVERLAPS(overlapPos,overlapPos+len,k.block,k.block+len)) {
          k.done=true;
          //logW("overlap");
        } else {
          validCount++;
        }
        overlapPos=k.block;
      }


      // calculate (weighted) benefit
      const int blockSize=estimateBlockSize(&buf[testLenMatches[0].orig],len,speedDial);
      const int gains=((blockSize-3)*validCount)-4;
      int finalBenefit=gains*2+len*3;
      if (gains<1) finalBenefit=-1;

      // check whether this set of matches has greater benefit
      if (finalBenefit>bestBenefit.benefit) {
        //logD("- %x (%d): %d = %d",(int)i,(int)len,(int)testLenMatches.size(),finalBenefit);
        bestBenefit=MatchBenefit(begin,finalBenefit,len);
        // copy matches so we don't have to select them later
        workMatches=testLenMatches;
      }
    }
  }

  // quit if there isn't benefit
  if (bestBenefit.benefit<1) return stream;

  // quit if there's nothing to work on
  if (workMatches.empty()) return stream;

  // pick best benefit
  logI("BEST BENEFIT: %d in %x with size %u",bestBenefit.benefit,(int)bestBenefit.index,bestBenefit.len);

  // work on matches with this benefit
  size_t bestOrig=matches[bestBenefit.index].orig;
  logI("match count %d",(int)workMatches.size());

  if (progress!=NULL) {
    progress->optStage=3;
    progress->origCurrent=origs.size();
  }

  // make sub-block
  size_t subBlockID=subBlocks.size();
  logV("new sub-block %d",(int)subBlockID);

  assert(!(bestOrig&7));

  // isolate this sub-block
  SafeWriter* newBlock=new SafeWriter;
  newBlock->init();
  newBlock->write(&buf[bestOrig],bestBenefit.len);
  newBlock->writeC(0xd9); // ret
  // padding
  newBlock->writeC(0);
  newBlock->writeC(0);
  newBlock->writeC(0);
  newBlock->writeC(0);
  newBlock->writeC(0);
  newBlock->writeC(0);
  newBlock->writeC(0);
  subBlocks.push_back(newBlock);

  // insert call on the original block
  buf[bestOrig]=0xd4;
  buf[bestOrig+1]=subBlockID&0xff;
  buf[bestOrig+2]=(subBlockID>>8)&0xff;
  buf[bestOrig+3]=(subBlockID>>16)&0xff;
  buf[bestOrig+4]=(subBlockID>>24)&0xff;
  buf[bestOrig+5]=0;
  buf[bestOrig+6]=0;
  buf[bestOrig+7]=0;

  // replace the rest with nop
  for (size_t j=bestOrig+8; j<bestOrig+bestBenefit.len; j++) {
    buf[j]=0xd1;
  }

  // set matches to this sub-block
  for (BlockMatch& i: workMatches) {
    // skip invalid matches
    if (i.done) continue;

    assert(!(i.block&7));

    // set match to this sub-block
    buf[i.block]=0xd4;
    buf[i.block+1]=subBlockID&0xff;
    buf[i.block+2]=(subBlockID>>8)&0xff;
    buf[i.block+3]=(subBlockID>>16)&0xff;
    buf[i.block+4]=(subBlockID>>24)&0xff;
    buf[i.block+5]=0;
    buf[i.block+6]=0;
    buf[i.block+7]=0;

    // replace the rest with nop
    for (size_t j=i.block+8; j<i.block+bestBenefit.len; j++) {
      buf[j]=0xd1;
    }
  }

  logV("done!");

  // remove nop's
  stream=stripNops(stream);
  buf=stream->getFinalBuf();

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
      case 0xd5: { // calli
        unsigned int addr=buf[i+1]|(buf[i+2]<<8)|(buf[i+3]<<16)|(buf[i+4]<<24);
        try {
          addr=addrTable[addr];
          // check whether we have sufficient room to turn this into a 16-bit call
          if (addr<0xff00) {
            buf[i]=0xd8;
            buf[i+1]=addr&0xff;
            buf[i+2]=(addr>>8)&0xff;
            buf[i+3]=0xd1;
            buf[i+4]=0xd1;
          } else {
            buf[i]=0xd5;
            buf[i+1]=addr&0xff;
            buf[i+2]=(addr>>8)&0xff;
            buf[i+3]=(addr>>16)&0xff;
            buf[i+4]=(addr>>24)&0xff;
          }
        } catch (std::out_of_range& e) {
          logW("address %x is not mappable!",addr);
        }
        break;
      }
      case 0xda: { // jmp
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
      case 0xd8: { // call
        logW("16-bit call should NEVER be generated. aborting!");
        abort();
        break;
      }
    }
    s->write(&buf[i],insLen);
  }
  
  oldStream->finish();
  delete oldStream;
  return s;
}

SafeWriter* DivEngine::saveCommand(DivCSProgress* progress, DivCSOptions options) {
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
  
  SafeWriter* globalStream;
  SafeWriter* chanStream[DIV_MAX_CHANS];
  unsigned int chanStreamOff[DIV_MAX_CHANS];
  unsigned int chanStackSize[DIV_MAX_CHANS];
  std::vector<size_t> tickPos[DIV_MAX_CHANS];
  int loopTick=-1;

  memset(cmdPopularity,0,256*sizeof(int));
  memset(delayPopularity,0,256*sizeof(int));
  memset(chanStream,0,DIV_MAX_CHANS*sizeof(void*));
  memset(chanStreamOff,0,DIV_MAX_CHANS*sizeof(unsigned int));
  memset(chanStackSize,0,DIV_MAX_CHANS*sizeof(unsigned int));
  memset(sortedCmdPopularity,0,16*sizeof(int));
  memset(sortedDelayPopularity,0,16*sizeof(int));
  memset(sortedCmd,0,16);
  memset(sortedDelay,0,16);

  SafeWriter* w=new SafeWriter;
  w->init();

  globalStream=new SafeWriter;
  globalStream->init();

  // write header
  w->write("FCS",4);
  w->writeS(chans);
  // flags
  w->writeC((options.longPointers?1:0)|(options.bigEndian?2:0));
  // reserved
  w->writeC(0);
  // preset delays and speed dial
  for (int i=0; i<32; i++) {
    w->writeC(0);
  }
  // offsets
  for (int i=0; i<chans; i++) {
    chanStream[i]=new SafeWriter;
    chanStream[i]->init();
    if (options.longPointers) {
      w->writeI(0);
    } else {
      w->writeS(0);
    }
  }
  // max stack sizes
  for (int i=0; i<chans; i++) {
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
  // song beginning marker
  for (int i=0; i<chans; i++) {
    chanStream[i]->writeC(0xd0);
    chanStream[i]->writeC(i);
    chanStream[i]->writeC(0x00);
    chanStream[i]->writeC(0x00);
    // padding
    chanStream[i]->writeC(0x00);
    chanStream[i]->writeC(0x00);
    chanStream[i]->writeC(0x00);
    chanStream[i]->writeC(0x00);
  }
  while (!done) {
    for (int i=0; i<chans; i++) {
      tickPos[i].push_back(chanStream[i]->tell());
    }
    if (loopTick==-1) {
      if (loopOrder==curOrder && loopRow==curRow) {
        if ((ticks-((tempoAccum+virtualTempoN)/virtualTempoD))<=0) {
          logI("loop is on tick %d",tick);
          loopTick=tick;
          // loop marker
          for (int i=0; i<chans; i++) {
            chanStream[i]->writeC(0xd0);
            chanStream[i]->writeC(i);
            chanStream[i]->writeC(0x00);
            chanStream[i]->writeC(0x01);
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
      chanStream[0]->writeC(0xdb);
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
          writeCommandValues(chanStream[i.chan],i,options.bigEndian);
          break;
      }
    }
    cmdStream.clear();
    for (int i=0; i<chans; i++) {
      chanStream[i]->writeC(0xde);
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
      chanStream[i]->writeC(0xdf);
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
        chanStream[i]->writeC(0xda);
        chanStream[i]->writeI(tickPos[i][loopTick]);
        logD("chan %d loop addr: %x",i,tickPos[i][loopTick]);
        // padding
        chanStream[i]->writeC(0x00);
        chanStream[i]->writeC(0x00);
        chanStream[i]->writeC(0x00);
      } else {
        logW("chan %d unable to find loop addr!",i);
        chanStream[i]->writeC(0xdf);
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
  if (!options.noCmdCallOpt) {
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
        if (buf[i]==0xd7) {
          // find whether this command is in speed dial
          for (int j=0; j<16; j++) {
            if (buf[i+1]==sortedCmd[j]) {
              buf[i]=0xe0+j;
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
  if (!options.noDelayCondense) {
    // calculate delay usage
    for (int h=0; h<chans; h++) {
      unsigned char* buf=chanStream[h]->getFinalBuf();
      int delayCount=0;
      for (size_t i=0; i<chanStream[h]->size(); i+=8) {
        if (buf[i]==0xde) {
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
        if (buf[i]==0xde) {
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
                  buf[delayPos++]=0xdc;
                  buf[delayPos++]=delayCount&0xff;
                  buf[delayPos++]=(delayCount>>8)&0xff;
                } else {
                  bool foundShort=false;
                  for (int j=0; j<16; j++) {
                    if (sortedDelay[j]==delayCount) {
                      buf[delayPos++]=0xf0+j;
                      foundShort=true;
                      break;
                    }
                  }
                  if (!foundShort) {
                    buf[delayPos++]=0xdd;
                    buf[delayPos++]=delayCount;
                  }
                }
                // padding
                while (delayPos&7) buf[delayPos++]=0;
                // fill with nop
                for (int j=delayPos; j<=delayLast; j++) {
                  buf[j]=0xd1;
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

  // PASS 3: note off + one-tick wait
  // optimize one-tick gaps sometimes used in songs
  for (int h=0; h<chans; h++) {
    unsigned char* buf=chanStream[h]->getFinalBuf();
    if (chanStream[h]->size()<8) continue;
    for (size_t i=0; i<chanStream[h]->size()-8; i+=8) {
      // find note off
      if (buf[i]==0xb5) {
        // check for contiguous wait 1
        if (buf[i+8]==0xde) {
          // turn it into 0xf6 (note off + wait 1) and change the next one to nop
          buf[i]=0xd6;
          buf[i+8]=0xd1;

          // skip the next instruction
          i+=8;
        }
      }
    }
  }

  // PASS 4: remove nop's
  // this includes modifying call addresses to compensate
  for (int h=0; h<chans; h++) {
    chanStream[h]=stripNops(chanStream[h]);
  }

  // PASS 5: put all channels together
  for (int i=0; i<chans; i++) {
    chanStreamOff[i]=globalStream->tell();
    logI("- %d: off %x size %ld",i,chanStreamOff[i],chanStream[i]->size());
    reloc8(chanStream[i]->getFinalBuf(),chanStream[i]->size(),0,globalStream->tell());
    globalStream->write(chanStream[i]->getFinalBuf(),chanStream[i]->size());
    chanStream[i]->finish();
    delete chanStream[i];
  }

  // PASS 6: find sub-blocks and isolate them
  if (!options.noSubBlock) {
    std::vector<SafeWriter*> subBlocks;
    size_t beforeSize=globalStream->size();
    
    // 6 is the minimum size that can be reliably optimized
    logI("finding sub-blocks");

    bool haveBlocks=false;
    subBlocks.clear();
    // repeat until no more sub-blocks are produced
    do {
      logD("iteration...");
      globalStream=findSubBlocks(globalStream,subBlocks,sortedCmd,progress);

      haveBlocks=!subBlocks.empty();
      // insert sub-blocks and resolve symbols
      logI("%d sub-blocks total",(int)subBlocks.size());
      std::vector<size_t> blockOff;
      blockOff.clear();
      globalStream->seek(0,SEEK_END);
      for (size_t i=0; i<subBlocks.size(); i++) {
        SafeWriter* block=subBlocks[i];

        // write sub-block
        blockOff.push_back(globalStream->tell());
        logV("block size: %d",(int)block->size());
        assert(!(block->size()&7));
        globalStream->write(block->getFinalBuf(),block->size());
      }

      for (SafeWriter* block: subBlocks) {
        block->finish();
        delete block;
      }
      subBlocks.clear();

      // resolve symbols
      unsigned char* buf=globalStream->getFinalBuf();
      for (size_t j=0; j<globalStream->size(); j+=8) {
        if (buf[j]==0xd4) { // callsym
          unsigned int addr=buf[j+1]|(buf[j+2]<<8)|(buf[j+3]<<16)|(buf[j+4]<<24);
          if (addr<blockOff.size()) {
            // turn it into call
            addr=blockOff[addr];
            buf[j]=0xd5;
            buf[j+1]=addr&0xff;
            buf[j+2]=(addr>>8)&0xff;
            buf[j+3]=(addr>>16)&0xff;
            buf[j+4]=(addr>>24)&0xff;
          } else {
            logE("requested symbol %d is out of bounds!",addr);
            abort();
          }
        }
      }
    } while (haveBlocks);

    size_t afterSize=globalStream->size();
    logI("(before: %d - after: %d)",(int)beforeSize,(int)afterSize);
    assert(!(globalStream->size()&7));
  }

  // PASS 7: pack stream
  globalStream=packStream(globalStream,sortedCmd);

  // PASS 8: remove nop's which may be produced by 32-bit call conversion
  // also find new offsets
  globalStream=stripNopsPacked(globalStream,sortedCmd,chanStreamOff);

  for (int h=0; h<chans; h++) {
    chanStreamOff[h]+=w->tell();
  }

  // write results (convert addresses to big-endian if necessary)
  reloc(globalStream->getFinalBuf(),globalStream->size(),0,w->tell(),sortedCmd,options.bigEndian);
  w->write(globalStream->getFinalBuf(),globalStream->size());

  // calculate max stack sizes
  for (int h=0; h<chans; h++) {
    std::stack<unsigned int> callStack;
    unsigned int maxStackSize=0;
    unsigned char* buf=w->getFinalBuf();
    bool done=false;
    for (size_t i=chanStreamOff[h]; i<w->size();) {
      int insLen=getInsLength(buf[i],_EXT(buf,i,w->size()),sortedCmd);
      if (insLen<1) {
        logE("%d: INS %x NOT IMPLEMENTED...",h,buf[i]);
        break;
      }
      switch (buf[i]) {
        case 0xd5: { // calli
          unsigned int addr=buf[i+1]|(buf[i+2]<<8)|(buf[i+3]<<16)|(buf[i+4]<<24);
          callStack.push(i+insLen);
          if (callStack.size()>maxStackSize) maxStackSize=callStack.size();
          i=addr;
          insLen=0;
          break;
        }
        case 0xd8: { // call
          unsigned short addr=buf[i+1]|(buf[i+2]<<8);
          callStack.push(i+insLen);
          if (callStack.size()>maxStackSize) maxStackSize=callStack.size();
          i=addr;
          insLen=0;
          break;
        }
        case 0xd9: { // ret
          if (callStack.empty()) {
            logE("%d: trying to ret with empty stack!",h);
            done=true;
            break;
          }
          i=callStack.top();
          insLen=0;
          callStack.pop();
          break;
        }
        case 0xda: // jmp
        case 0xdf: // stop
          done=true;
          break;
      }
      if (maxStackSize>255) {
        logE("%d: stack overflow!",h);
        break;
      }
      if (done) break;
      i+=insLen;
    }

    chanStackSize[h]=maxStackSize;
  }

  globalStream->finish();
  delete globalStream;

  w->seek(40,SEEK_SET);
  for (int i=0; i<chans; i++) {
    if (options.longPointers) {
      if (options.bigEndian) {
        w->writeI_BE(chanStreamOff[i]);
      } else {
        w->writeI(chanStreamOff[i]);
      }
    } else {
      if (options.bigEndian) {
        w->writeS_BE(chanStreamOff[i]);
      } else {
        w->writeS(chanStreamOff[i]);
      }
    }
  }

  logD("maximum stack sizes:");
  unsigned int cumulativeStackSize=0;
  for (int i=0; i<chans; i++) {
    w->writeC(chanStackSize[i]);
    logD("- %d: %d",i,chanStackSize[i]);
    cumulativeStackSize+=chanStackSize[i];
  }
  logD("(total stack size: %d)",cumulativeStackSize);

  logD("delay popularity:");
  w->seek(8,SEEK_SET);
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
