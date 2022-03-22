/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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

#include "blip_buf.h"
#include "song.h"
#include "wavetable.h"
#define _USE_MATH_DEFINES
#include "dispatch.h"
#include "engine.h"
#include "../ta-log.h"
#include <math.h>
#include <sndfile.h>

constexpr int MASTER_CLOCK_PREC=(sizeof(void*)==8)?8:0;

void DivEngine::nextOrder() {
  curRow=0;
  if (repeatPattern) return;
  if (++curOrder>=song.ordersLen) {
    endOfSong=true;
    curOrder=0;
  }
}

const char* notes[12]={
  "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
};

// update this when adding new commands.
const char* cmdName[DIV_CMD_MAX]={
  "NOTE_ON",
  "NOTE_OFF",
  "NOTE_OFF_ENV",
  "ENV_RELEASE",
  "INSTRUMENT",
  "VOLUME",
  "GET_VOLUME",
  "GET_VOLMAX",
  "NOTE_PORTA",
  "PITCH",
  "PANNING",
  "LEGATO",
  "PRE_PORTA",
  "PRE_NOTE",

  "SAMPLE_MODE",
  "SAMPLE_FREQ",
  "SAMPLE_BANK",
  "SAMPLE_POS",

  "FM_LFO",
  "FM_LFO_WAVE",
  "FM_TL",
  "FM_AR",
  "FM_FB",
  "FM_MULT",
  "FM_EXTCH",
  "FM_AM_DEPTH",
  "FM_PM_DEPTH",

  "GENESIS_LFO",
  
  "ARCADE_LFO",

  "STD_NOISE_FREQ",
  "STD_NOISE_MODE",

  "WAVE",
  
  "GB_SWEEP_TIME",
  "GB_SWEEP_DIR",

  "PCE_LFO_MODE",
  "PCE_LFO_SPEED",

  "NES_SWEEP",

  "C64_CUTOFF",
  "C64_RESONANCE",
  "C64_FILTER_MODE",
  "C64_RESET_TIME",
  "C64_RESET_MASK",
  "C64_FILTER_RESET",
  "C64_DUTY_RESET",
  "C64_EXTENDED",
  "C64_FINE_DUTY",
  "C64_FINE_CUTOFF",

  "AY_ENVELOPE_SET",
  "AY_ENVELOPE_LOW",
  "AY_ENVELOPE_HIGH",
  "AY_ENVELOPE_SLIDE",
  "AY_NOISE_MASK_AND",
  "AY_NOISE_MASK_OR",
  "AY_AUTO_ENVELOPE",

  "SAA_ENVELOPE",
  
  "LYNX_LFSR_LOAD",

  "QSOUND_ECHO_FEEDBACK",
  "QSOUND_ECHO_DELAY",
  "QSOUND_ECHO_LEVEL",

  "X1_010_ENVELOPE_SHAPE",
  "X1_010_ENVELOPE_ENABLE",
  "X1_010_ENVELOPE_MODE",
  "X1_010_ENVELOPE_PERIOD",
  "X1_010_ENVELOPE_SLIDE",
  "X1_010_AUTO_ENVELOPE",

  "WS_SWEEP_TIME",
  "WS_SWEEP_AMOUNT",

  "N163_WAVE_POSITION",
  "N163_WAVE_LENGTH",
  "N163_WAVE_MODE",
  "N163_WAVE_LOAD",
  "N163_WAVE_LOADPOS",
  "N163_WAVE_LOADLEN",
  "N163_CHANNEL_LIMIT",
  "N163_GLOBAL_WAVE_LOAD",
  "N163_GLOBAL_WAVE_LOADPOS",
  "N163_GLOBAL_WAVE_LOADLEN",
  "N163_GLOBAL_WAVE_LOADMODE",

  "ALWAYS_SET_VOLUME"
};

const char* formatNote(unsigned char note, unsigned char octave) {
  static char ret[4];
  if (note==100) {
    return "OFF";
  } else if (note==101) {
    return "===";
  } else if (note==102) {
    return "REL";
  } else if (octave==0 && note==0) {
    return "---";
  }
  snprintf(ret,4,"%s%d",notes[note%12],octave+note/12);
  return ret;
}

int DivEngine::dispatchCmd(DivCommand c) {
  if (view==DIV_STATUS_COMMANDS) {
    printf("%8d | %d: %s(%d, %d)\n",totalTicksR,c.chan,cmdName[c.cmd],c.value,c.value2);
  }
  totalCmds++;
  if (cmdStreamEnabled && cmdStream.size()<2000) {
    cmdStream.push_back(c);
  }
  c.chan=dispatchChanOfChan[c.dis];
  return disCont[dispatchOfChan[c.dis]].dispatch->dispatch(c);
}

bool DivEngine::perSystemEffect(int ch, unsigned char effect, unsigned char effectVal) {
  switch (sysOfChan[ch]) {
    case DIV_SYSTEM_YM2612:
    case DIV_SYSTEM_YM2612_EXT:
      switch (effect) {
        case 0x17: // DAC enable
          dispatchCmd(DivCommand(DIV_CMD_SAMPLE_MODE,ch,(effectVal>0)));
          break;
        case 0x20: // SN noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_SMS:
      switch (effect) {
        case 0x20: // SN noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_GB:
      switch (effect) {
        case 0x10: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        case 0x11: case 0x12: // duty or noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        case 0x13: // sweep params
          dispatchCmd(DivCommand(DIV_CMD_GB_SWEEP_TIME,ch,effectVal));
          break;
        case 0x14: // sweep direction
          dispatchCmd(DivCommand(DIV_CMD_GB_SWEEP_DIR,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_PCE:
      switch (effect) {
        case 0x10: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        case 0x11: // noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        case 0x12: // LFO mode
          dispatchCmd(DivCommand(DIV_CMD_PCE_LFO_MODE,ch,effectVal));
          break;
        case 0x13: // LFO speed
          dispatchCmd(DivCommand(DIV_CMD_PCE_LFO_SPEED,ch,effectVal));
          break;
        case 0x17: // PCM enable
          dispatchCmd(DivCommand(DIV_CMD_SAMPLE_MODE,ch,(effectVal>0)));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_NES:
      switch (effect) {
        case 0x12: // duty or noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        case 0x13: // sweep up
          dispatchCmd(DivCommand(DIV_CMD_NES_SWEEP,ch,0,effectVal));
          break;
        case 0x14: // sweep down
          dispatchCmd(DivCommand(DIV_CMD_NES_SWEEP,ch,1,effectVal));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_OPLL_DRUMS:
    case DIV_SYSTEM_OPL_DRUMS:
    case DIV_SYSTEM_OPL2_DRUMS:
    case DIV_SYSTEM_OPL3_DRUMS:
      switch (effect) {
        case 0x18: // drum mode toggle
          dispatchCmd(DivCommand(DIV_CMD_FM_EXTCH,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_N163:
      switch (effect) {
        case 0x10: // select instrument waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        case 0x11: // select instrument waveform position in RAM
          dispatchCmd(DivCommand(DIV_CMD_N163_WAVE_POSITION,ch,effectVal));
          break;
        case 0x12: // select instrument waveform length in RAM
          dispatchCmd(DivCommand(DIV_CMD_N163_WAVE_LENGTH,ch,effectVal));
          break;
        case 0x13: // change instrument waveform update mode
          dispatchCmd(DivCommand(DIV_CMD_N163_WAVE_MODE,ch,effectVal));
          break;
        case 0x14: // select waveform for load to RAM
          dispatchCmd(DivCommand(DIV_CMD_N163_WAVE_LOAD,ch,effectVal));
          break;
        case 0x15: // select waveform position for load to RAM
          dispatchCmd(DivCommand(DIV_CMD_N163_WAVE_LOADPOS,ch,effectVal));
          break;
        case 0x16: // select waveform length for load to RAM
          dispatchCmd(DivCommand(DIV_CMD_N163_WAVE_LOADLEN,ch,effectVal));
          break;
        case 0x17: // change waveform load mode
          dispatchCmd(DivCommand(DIV_CMD_N163_WAVE_LOADMODE,ch,effectVal));
          break;
        case 0x18: // change channel limits
          dispatchCmd(DivCommand(DIV_CMD_N163_CHANNEL_LIMIT,ch,effectVal));
          break;
        case 0x20: // (global) select waveform for load to RAM
          dispatchCmd(DivCommand(DIV_CMD_N163_GLOBAL_WAVE_LOAD,ch,effectVal));
          break;
        case 0x21: // (global) select waveform position for load to RAM
          dispatchCmd(DivCommand(DIV_CMD_N163_GLOBAL_WAVE_LOADPOS,ch,effectVal));
          break;
        case 0x22: // (global) select waveform length for load to RAM
          dispatchCmd(DivCommand(DIV_CMD_N163_GLOBAL_WAVE_LOADLEN,ch,effectVal));
          break;
        case 0x23: // (global) change waveform load mode
          dispatchCmd(DivCommand(DIV_CMD_N163_GLOBAL_WAVE_LOADMODE,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_QSOUND:
      switch (effect) {
        case 0x10: // echo feedback
          dispatchCmd(DivCommand(DIV_CMD_QSOUND_ECHO_FEEDBACK,ch,effectVal));
          break;
        case 0x11: // echo level
          dispatchCmd(DivCommand(DIV_CMD_QSOUND_ECHO_LEVEL,ch,effectVal));
          break;
        default:
          if ((effect&0xf0)==0x30) {
            dispatchCmd(DivCommand(DIV_CMD_QSOUND_ECHO_DELAY,ch,((effect & 0x0f) << 8) | effectVal));
          } else {
            return false;
          }
          break;
      }
      break;
    case DIV_SYSTEM_X1_010:
      switch (effect) {
        case 0x10: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        case 0x11: // select envelope shape
          dispatchCmd(DivCommand(DIV_CMD_X1_010_ENVELOPE_SHAPE,ch,effectVal));
          break;
        case 0x17: // PCM enable
          dispatchCmd(DivCommand(DIV_CMD_SAMPLE_MODE,ch,(effectVal>0)));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_SWAN:
      switch (effect) {
        case 0x10: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        case 0x11: // noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        case 0x12: // sweep period
          dispatchCmd(DivCommand(DIV_CMD_WS_SWEEP_TIME,ch,effectVal));
          break;
        case 0x13: // sweep amount
          dispatchCmd(DivCommand(DIV_CMD_WS_SWEEP_AMOUNT,ch,effectVal));
          break;
        case 0x17: // PCM enable
          dispatchCmd(DivCommand(DIV_CMD_SAMPLE_MODE,ch,(effectVal>0)));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_VERA:
      switch (effect) {
        case 0x20: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        case 0x22: // duty
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_BUBSYS_WSG:
    case DIV_SYSTEM_PET:
      switch (effect) {
        case 0x10: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    default:
      return false;
  }
  return true;
}

#define IS_YM2610 (sysOfChan[ch]==DIV_SYSTEM_YM2610 || sysOfChan[ch]==DIV_SYSTEM_YM2610_EXT || sysOfChan[ch]==DIV_SYSTEM_YM2610_FULL || sysOfChan[ch]==DIV_SYSTEM_YM2610_FULL_EXT || sysOfChan[ch]==DIV_SYSTEM_YM2610B || sysOfChan[ch]==DIV_SYSTEM_YM2610B_EXT)

bool DivEngine::perSystemPostEffect(int ch, unsigned char effect, unsigned char effectVal) {
  switch (sysOfChan[ch]) {
    case DIV_SYSTEM_YM2612:
    case DIV_SYSTEM_YM2612_EXT:
    case DIV_SYSTEM_YM2151:
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_YM2610_FULL:
    case DIV_SYSTEM_YM2610_FULL_EXT:
    case DIV_SYSTEM_YM2610B:
    case DIV_SYSTEM_YM2610B_EXT:
      switch (effect) {
        case 0x10: // LFO or noise mode
          if (sysOfChan[ch]==DIV_SYSTEM_YM2151) {
            dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_FREQ,ch,effectVal));
          } else {
            dispatchCmd(DivCommand(DIV_CMD_FM_LFO,ch,effectVal));
          }
          break;
        case 0x11: // FB
          dispatchCmd(DivCommand(DIV_CMD_FM_FB,ch,effectVal&7));
          break;
        case 0x12: // TL op1
          dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,0,effectVal&0x7f));
          break;
        case 0x13: // TL op2
          dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,1,effectVal&0x7f));
          break;
        case 0x14: // TL op3
          dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,2,effectVal&0x7f));
          break;
        case 0x15: // TL op4
          dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,3,effectVal&0x7f));
          break;
        case 0x16: // MULT
          if ((effectVal>>4)>0 && (effectVal>>4)<5) {
            dispatchCmd(DivCommand(DIV_CMD_FM_MULT,ch,(effectVal>>4)-1,effectVal&15));
          }
          break;
        case 0x17: // arcade LFO
          if (sysOfChan[ch]==DIV_SYSTEM_YM2151) {
            dispatchCmd(DivCommand(DIV_CMD_FM_LFO,ch,effectVal));
          }
          break;
        case 0x18: // EXT or LFO waveform
          if (sysOfChan[ch]==DIV_SYSTEM_YM2151) {
            dispatchCmd(DivCommand(DIV_CMD_FM_LFO_WAVE,ch,effectVal));
          } else {
            dispatchCmd(DivCommand(DIV_CMD_FM_EXTCH,ch,effectVal));
          }
          break;
        case 0x19: // AR global
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,-1,effectVal&31));
          break;
        case 0x1a: // AR op1
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,0,effectVal&31));
          break;
        case 0x1b: // AR op2
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,1,effectVal&31));
          break;
        case 0x1c: // AR op3
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,2,effectVal&31));
          break;
        case 0x1d: // AR op4
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,3,effectVal&31));
          break;
        case 0x1e: // UNOFFICIAL: Arcade AM depth
          dispatchCmd(DivCommand(DIV_CMD_FM_AM_DEPTH,ch,effectVal&127));
          break;
        case 0x1f: // UNOFFICIAL: Arcade PM depth
          dispatchCmd(DivCommand(DIV_CMD_FM_PM_DEPTH,ch,effectVal&127));
          break;
        case 0x20: // Neo Geo PSG mode
          if (IS_YM2610) {
            dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          }
          break;
        case 0x21: // Neo Geo PSG noise freq
          if (IS_YM2610) {
            dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_FREQ,ch,effectVal));
          }
          break;
        case 0x22: // UNOFFICIAL: Neo Geo PSG envelope enable
          if (IS_YM2610) {
            dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_SET,ch,effectVal));
          }
          break;
        case 0x23: // UNOFFICIAL: Neo Geo PSG envelope period low
          if (IS_YM2610) {
            dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_LOW,ch,effectVal));
          }
          break;
        case 0x24: // UNOFFICIAL: Neo Geo PSG envelope period high
          if (IS_YM2610) {
            dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_HIGH,ch,effectVal));
          }
          break;
        case 0x25: // UNOFFICIAL: Neo Geo PSG envelope slide up
          if (IS_YM2610) {
            dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_SLIDE,ch,-effectVal));
          }
          break;
        case 0x26: // UNOFFICIAL: Neo Geo PSG envelope slide down
          if (IS_YM2610) {
            dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_SLIDE,ch,effectVal));
          }
          break;
        case 0x29: // auto-envelope
          if (IS_YM2610) {
            dispatchCmd(DivCommand(DIV_CMD_AY_AUTO_ENVELOPE,ch,effectVal));
          }
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_OPLL:
    case DIV_SYSTEM_OPLL_DRUMS:
    case DIV_SYSTEM_VRC7:
      switch (effect) {
        case 0x11: // FB
          dispatchCmd(DivCommand(DIV_CMD_FM_FB,ch,effectVal&7));
          break;
        case 0x12: // TL op1
          dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,0,effectVal&0x3f));
          break;
        case 0x13: // TL op2
          dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,1,effectVal&0x0f));
          break;
        case 0x16: // MULT
          if ((effectVal>>4)>0 && (effectVal>>4)<3) {
            dispatchCmd(DivCommand(DIV_CMD_FM_MULT,ch,(effectVal>>4)-1,effectVal&15));
          }
          break;
        case 0x19: // AR global
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,-1,effectVal&31));
          break;
        case 0x1a: // AR op1
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,0,effectVal&31));
          break;
        case 0x1b: // AR op2
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,1,effectVal&31));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_OPL:
    case DIV_SYSTEM_OPL2:
    case DIV_SYSTEM_OPL3:
    case DIV_SYSTEM_OPL_DRUMS:
    case DIV_SYSTEM_OPL2_DRUMS:
    case DIV_SYSTEM_OPL3_DRUMS:
      switch (effect) {
        case 0x10: // DAM
          dispatchCmd(DivCommand(DIV_CMD_FM_LFO,ch,effectVal&1));
          break;
        case 0x11: // FB
          dispatchCmd(DivCommand(DIV_CMD_FM_FB,ch,effectVal&7));
          break;
        case 0x12: // TL op1
          dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,0,effectVal&0x3f));
          break;
        case 0x13: // TL op2
          dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,1,effectVal&0x3f));
          break;
        case 0x14: // TL op3
          dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,2,effectVal&0x3f));
          break;
        case 0x15: // TL op4
          dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,3,effectVal&0x3f));
          break;
        case 0x16: // MULT
          if ((effectVal>>4)>0 && (effectVal>>4)<5) {
            dispatchCmd(DivCommand(DIV_CMD_FM_MULT,ch,(effectVal>>4)-1,effectVal&15));
          }
          break;
        case 0x17: // DVB
          dispatchCmd(DivCommand(DIV_CMD_FM_LFO,ch,2+(effectVal&1)));
          break;
        case 0x19: // AR global
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,-1,effectVal&15));
          break;
        case 0x1a: // AR op1
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,0,effectVal&15));
          break;
        case 0x1b: // AR op2
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,1,effectVal&15));
          break;
        case 0x1c: // AR op3
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,2,effectVal&15));
          break;
        case 0x1d: // AR op4
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,3,effectVal&15));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580:
      switch (effect) {
        case 0x10: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        case 0x11: // cutoff
          dispatchCmd(DivCommand(DIV_CMD_C64_CUTOFF,ch,effectVal));
          break;
        case 0x12: // duty
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        case 0x13: // resonance
          dispatchCmd(DivCommand(DIV_CMD_C64_RESONANCE,ch,effectVal));
          break;
        case 0x14: // filter mode
          dispatchCmd(DivCommand(DIV_CMD_C64_FILTER_MODE,ch,effectVal));
          break;
        case 0x15: // reset time
          dispatchCmd(DivCommand(DIV_CMD_C64_RESET_TIME,ch,effectVal));
          break;
        case 0x1a: // reset mask
          dispatchCmd(DivCommand(DIV_CMD_C64_RESET_MASK,ch,effectVal));
          break;
        case 0x1b: // cutoff reset
          dispatchCmd(DivCommand(DIV_CMD_C64_FILTER_RESET,ch,effectVal));
          break;
        case 0x1c: // duty reset
          dispatchCmd(DivCommand(DIV_CMD_C64_DUTY_RESET,ch,effectVal));
          break;
        case 0x1e: // extended
          dispatchCmd(DivCommand(DIV_CMD_C64_EXTENDED,ch,effectVal));
          break;
        case 0x30: case 0x31: case 0x32: case 0x33:
        case 0x34: case 0x35: case 0x36: case 0x37:
        case 0x38: case 0x39: case 0x3a: case 0x3b:
        case 0x3c: case 0x3d: case 0x3e: case 0x3f: // fine duty
          dispatchCmd(DivCommand(DIV_CMD_C64_FINE_DUTY,ch,((effect&0x0f)<<8)|effectVal));
          break;
        case 0x40: case 0x41: case 0x42: case 0x43:
        case 0x44: case 0x45: case 0x46: case 0x47: // fine cutoff
          dispatchCmd(DivCommand(DIV_CMD_C64_FINE_CUTOFF,ch,((effect&0x07)<<8)|effectVal));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_AY8910:
    case DIV_SYSTEM_AY8930:
      switch (effect) {
        case 0x12: // duty on 8930
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,0x10+(effectVal&15)));
          break;
        case 0x20: // mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal&15));
          break;
        case 0x21: // noise freq
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_FREQ,ch,effectVal));
          break;
        case 0x22: // envelope enable
          dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_SET,ch,effectVal));
          break;
        case 0x23: // envelope period low
          dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_LOW,ch,effectVal));
          break;
        case 0x24: // envelope period high
          dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_HIGH,ch,effectVal));
          break;
        case 0x25: // envelope slide up
          dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_SLIDE,ch,-effectVal));
          break;
        case 0x26: // envelope slide down
          dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_SLIDE,ch,effectVal));
          break;
        case 0x27: // noise and mask
          dispatchCmd(DivCommand(DIV_CMD_AY_NOISE_MASK_AND,ch,effectVal));
          break;
        case 0x28: // noise or mask
          dispatchCmd(DivCommand(DIV_CMD_AY_NOISE_MASK_OR,ch,effectVal));
          break;
        case 0x29: // auto-envelope
          dispatchCmd(DivCommand(DIV_CMD_AY_AUTO_ENVELOPE,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_SAA1099:
      switch (effect) {
        case 0x10: // select channel mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        case 0x11: // set noise freq
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_FREQ,ch,effectVal));
          break;
        case 0x12: // setup envelope
          dispatchCmd(DivCommand(DIV_CMD_SAA_ENVELOPE,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_TIA:
      switch (effect) {
        case 0x10: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_SEGAPCM:
    case DIV_SYSTEM_SEGAPCM_COMPAT:
      switch (effect) {
        case 0x20: // PCM frequency
          dispatchCmd(DivCommand(DIV_CMD_SAMPLE_FREQ,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_LYNX:
      if (effect>=0x30 && effect<0x40) {
        int value = ((int)(effect&0x0f)<<8)|effectVal;
        dispatchCmd(DivCommand(DIV_CMD_LYNX_LFSR_LOAD,ch,value));
        break;
      }
      return false;
      break;
    case DIV_SYSTEM_X1_010:
      switch (effect) {
        case 0x20: // PCM frequency
          dispatchCmd(DivCommand(DIV_CMD_SAMPLE_FREQ,ch,effectVal));
          break;
        case 0x22: // envelope mode
          dispatchCmd(DivCommand(DIV_CMD_X1_010_ENVELOPE_MODE,ch,effectVal));
          break;
        case 0x23: // envelope period
          dispatchCmd(DivCommand(DIV_CMD_X1_010_ENVELOPE_PERIOD,ch,effectVal));
          break;
        case 0x25: // envelope slide up
          dispatchCmd(DivCommand(DIV_CMD_X1_010_ENVELOPE_SLIDE,ch,effectVal));
          break;
        case 0x26: // envelope slide down
          dispatchCmd(DivCommand(DIV_CMD_X1_010_ENVELOPE_SLIDE,ch,-effectVal));
          break;
        case 0x29: // auto-envelope
          dispatchCmd(DivCommand(DIV_CMD_X1_010_AUTO_ENVELOPE,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    default:
      return false;
  }
  return true;
}

void DivEngine::processRow(int i, bool afterDelay) {
  int whatOrder=afterDelay?chan[i].delayOrder:curOrder;
  int whatRow=afterDelay?chan[i].delayRow:curRow;
  DivPattern* pat=song.pat[i].getPattern(song.orders.ord[i][whatOrder],false);
  // pre effects
  if (!afterDelay) for (int j=0; j<song.pat[i].effectRows; j++) {
    short effect=pat->data[whatRow][4+(j<<1)];
    short effectVal=pat->data[whatRow][5+(j<<1)];

    if (effectVal==-1) effectVal=0;
    if (effect==0xed && effectVal!=0) {
      if (effectVal<=nextSpeed) {
        chan[i].rowDelay=effectVal+1;
        chan[i].delayOrder=whatOrder;
        chan[i].delayRow=whatRow;
        if (effectVal==nextSpeed) {
          //if (sysOfChan[i]!=DIV_SYSTEM_YM2610 && sysOfChan[i]!=DIV_SYSTEM_YM2610_EXT) chan[i].delayLocked=true;
        } else {
          chan[i].delayLocked=false;
        }
        return;
      } else {
        chan[i].delayLocked=false;
      }
    }
  }

  if (chan[i].delayLocked) return;

  // instrument
  if (pat->data[whatRow][2]!=-1) {
    dispatchCmd(DivCommand(DIV_CMD_INSTRUMENT,i,pat->data[whatRow][2]));
    if (chan[i].lastIns!=pat->data[whatRow][2]) {
      chan[i].lastIns=pat->data[whatRow][2];
      if (chan[i].inPorta && song.newInsTriggersInPorta) {
        dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,i,DIV_NOTE_NULL));
      }
    }
  }
  // note
  if (pat->data[whatRow][0]==100) { // note off
    //chan[i].note=-1;
    chan[i].keyOn=false;
    chan[i].keyOff=true;
    if (chan[i].inPorta && song.noteOffResetsSlides) {
      if (chan[i].stopOnOff) {
        chan[i].portaNote=-1;
        chan[i].portaSpeed=-1;
        chan[i].stopOnOff=false;
      }
      if (disCont[dispatchOfChan[i]].dispatch->keyOffAffectsPorta(dispatchChanOfChan[i])) {
        chan[i].portaNote=-1;
        chan[i].portaSpeed=-1;
        if (i==2 && sysOfChan[i]==DIV_SYSTEM_SMS) {
          chan[i+1].portaNote=-1;
          chan[i+1].portaSpeed=-1;
        }
      }
      chan[i].scheduledSlideReset=true;
    }
    dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF,i));
  } else if (pat->data[whatRow][0]==101) { // note off + env release
    //chan[i].note=-1;
    chan[i].keyOn=false;
    chan[i].keyOff=true;
    if (chan[i].inPorta && song.noteOffResetsSlides) {
      if (chan[i].stopOnOff) {
        chan[i].portaNote=-1;
        chan[i].portaSpeed=-1;
        chan[i].stopOnOff=false;
      }
      if (disCont[dispatchOfChan[i]].dispatch->keyOffAffectsPorta(dispatchChanOfChan[i])) {
        chan[i].portaNote=-1;
        chan[i].portaSpeed=-1;
        if (i==2 && sysOfChan[i]==DIV_SYSTEM_SMS) {
          chan[i+1].portaNote=-1;
          chan[i+1].portaSpeed=-1;
        }
      }
      chan[i].scheduledSlideReset=true;
    }
    dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF_ENV,i));
  } else if (pat->data[whatRow][0]==102) { // env release
    dispatchCmd(DivCommand(DIV_CMD_ENV_RELEASE,i));
  } else if (!(pat->data[whatRow][0]==0 && pat->data[whatRow][1]==0)) {
    chan[i].oldNote=chan[i].note;
    chan[i].note=pat->data[whatRow][0]+((signed char)pat->data[whatRow][1])*12;
    if (!chan[i].keyOn) {
      if (disCont[dispatchOfChan[i]].dispatch->keyOffAffectsArp(dispatchChanOfChan[i])) {
        chan[i].arp=0;
      }
    }
    chan[i].doNote=true;
    if (chan[i].arp!=0 && song.compatibleArpeggio) {
      chan[i].arpYield=true;
    }
  }

  // volume
  if (pat->data[whatRow][3]!=-1) {
    if (dispatchCmd(DivCommand(DIV_ALWAYS_SET_VOLUME,i)) || (MIN(chan[i].volMax,chan[i].volume)>>8)!=pat->data[whatRow][3]) {
      chan[i].volume=pat->data[whatRow][3]<<8;
      dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
    }
  }

  chan[i].retrigSpeed=0;

  short lastSlide=-1;

  // effects
  for (int j=0; j<song.pat[i].effectRows; j++) {
    short effect=pat->data[whatRow][4+(j<<1)];
    short effectVal=pat->data[whatRow][5+(j<<1)];

    if (effectVal==-1) effectVal=0;

    // per-system effect
    if (!perSystemEffect(i,effect,effectVal)) switch (effect) {
      case 0x09: // speed 1
        if (effectVal>0) speed1=effectVal;
        break;
      case 0x0f: // speed 2
        if (effectVal>0) speed2=effectVal;
        break;
      case 0x0b: // change order
        if (changeOrd==-1) {
          changeOrd=effectVal;
          changePos=0;
        }
        break;
      case 0x0d: // next order
        if (changeOrd<0 && curOrder<(song.ordersLen-1)) {
          changeOrd=-2;
          changePos=effectVal;
        }
        break;
      case 0x08: // panning
        dispatchCmd(DivCommand(DIV_CMD_PANNING,i,effectVal));
        break;
      case 0x01: // ramp up
        if (song.ignoreDuplicateSlides && (lastSlide==0x01 || lastSlide==0x1337)) break;
        lastSlide=0x01;
        if (effectVal==0) {
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
          chan[i].inPorta=false;
          if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        } else {
          chan[i].portaNote=song.limitSlides?0x60:255;
          chan[i].portaSpeed=effectVal;
          chan[i].portaStop=true;
          chan[i].nowYouCanStop=false;
          chan[i].stopOnOff=false;
          chan[i].scheduledSlideReset=false;
          chan[i].inPorta=false;
          if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,0));
        }
        break;
      case 0x02: // ramp down
        if (song.ignoreDuplicateSlides && (lastSlide==0x02 || lastSlide==0x1337)) break;
        lastSlide=0x02;
        if (effectVal==0) {
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
          chan[i].inPorta=false;
          if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        } else {
          chan[i].portaNote=song.limitSlides?disCont[dispatchOfChan[i]].dispatch->getPortaFloor(dispatchChanOfChan[i]):-60;
          chan[i].portaSpeed=effectVal;
          chan[i].portaStop=true;
          chan[i].nowYouCanStop=false;
          chan[i].stopOnOff=false;
          chan[i].scheduledSlideReset=false;
          chan[i].inPorta=false;
          if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,0));
        }
        break;
      case 0x03: // portamento
        if (effectVal==0) {
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
          chan[i].inPorta=false;
          dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        } else {
          if (chan[i].note==chan[i].oldNote && !chan[i].inPorta) {
            chan[i].portaNote=chan[i].note;
            chan[i].portaSpeed=-1;
          } else {
            chan[i].portaNote=chan[i].note;
            chan[i].portaSpeed=effectVal;
            chan[i].inPorta=true;
          }
          chan[i].portaStop=true;
          if (chan[i].keyOn) chan[i].doNote=false;
          chan[i].stopOnOff=song.stopPortaOnNoteOff; // what?!
          chan[i].scheduledSlideReset=false;
          dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,1));
          lastSlide=0x1337; // i hate this so much
        }
        break;
      case 0x04: // vibrato
        chan[i].vibratoDepth=effectVal&15;
        chan[i].vibratoRate=effectVal>>4;
        dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
        break;
      case 0x07: // tremolo
        // TODO
        break;
      case 0x0a: // volume ramp
        if (effectVal!=0) {
          if ((effectVal&15)!=0) {
            chan[i].volSpeed=-(effectVal&15)*64;
          } else {
            chan[i].volSpeed=(effectVal>>4)*64;
          }
        } else {
          chan[i].volSpeed=0;
        }
        break;
      case 0x00: // arpeggio
        chan[i].arp=effectVal;
        if (chan[i].arp==0 && song.arp0Reset) {
          chan[i].resetArp=true;
        }
        break;
      case 0x0c: // retrigger
        if (effectVal!=0) {
          chan[i].retrigSpeed=effectVal;
          chan[i].retrigTick=0;
        }
        break;
      case 0x90: case 0x91: case 0x92: case 0x93:
      case 0x94: case 0x95: case 0x96: case 0x97: 
      case 0x98: case 0x99: case 0x9a: case 0x9b:
      case 0x9c: case 0x9d: case 0x9e: case 0x9f: // set samp. pos
        dispatchCmd(DivCommand(DIV_CMD_SAMPLE_POS,i,(((effect&0x0f)<<8)|effectVal)*256));
        break;
      case 0xc0: case 0xc1: case 0xc2: case 0xc3: // set Hz
        divider=(double)(((effect&0x3)<<8)|effectVal);
        if (divider<10) divider=10;
        cycles=got.rate*pow(2,MASTER_CLOCK_PREC)/divider;
        clockDrift=0;
        break;
      case 0xe0: // arp speed
        if (effectVal>0) {
          song.arpLen=effectVal;
        }
        break;
      case 0xe1: // portamento up
        chan[i].portaNote=chan[i].note+(effectVal&15);
        chan[i].portaSpeed=(effectVal>>4)*4;
        chan[i].portaStop=true;
        chan[i].nowYouCanStop=false;
        chan[i].stopOnOff=song.stopPortaOnNoteOff; // what?!
        chan[i].scheduledSlideReset=false;
        if ((effectVal&15)!=0) {
          chan[i].inPorta=true;
          chan[i].shorthandPorta=true;
          if (!song.brokenShortcutSlides) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,0));
        } else {
          chan[i].inPorta=false;
          if (!song.brokenShortcutSlides) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        }
        break;
      case 0xe2: // portamento down
        chan[i].portaNote=chan[i].note-(effectVal&15);
        chan[i].portaSpeed=(effectVal>>4)*4;
        chan[i].portaStop=true;
        chan[i].nowYouCanStop=false;
        chan[i].stopOnOff=song.stopPortaOnNoteOff; // what?!
        chan[i].scheduledSlideReset=false;
        if ((effectVal&15)!=0) {
          chan[i].inPorta=true;
          chan[i].shorthandPorta=true;
          if (!song.brokenShortcutSlides) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,0));
        } else {
          chan[i].inPorta=false;
          if (!song.brokenShortcutSlides) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        }
        break;
      case 0xe3: // vibrato direction
        chan[i].vibratoDir=effectVal;
        break;
      case 0xe4: // vibrato fine
        chan[i].vibratoFine=effectVal;
        break;
      case 0xe5: // pitch
        chan[i].pitch=effectVal-0x80;
        if (sysOfChan[i]==DIV_SYSTEM_YM2151) { // YM2151 pitch oddity
          chan[i].pitch*=2;
          if (chan[i].pitch<-128) chan[i].pitch=-128;
          if (chan[i].pitch>127) chan[i].pitch=127;
        }
        chan[i].pitch+=globalPitch;
        dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
        break;
      case 0xea: // legato mode
        chan[i].legato=effectVal;
        break;
      case 0xeb: // sample bank
        dispatchCmd(DivCommand(DIV_CMD_SAMPLE_BANK,i,effectVal));
        break;
      case 0xec: // delayed note cut
        if (effectVal>0 && effectVal<nextSpeed) {
          chan[i].cut=effectVal+1;
        }
        break;
      case 0xee: // external command
        //printf("\x1b[1;36m%d: extern command %d\x1b[m\n",i,effectVal);
        extValue=effectVal;
        extValuePresent=true;
        break;
      case 0xef: // global pitch
        globalPitch+=(signed char)(effectVal-0x80);
        break;
      case 0xf0: // set Hz by tempo
        divider=(double)effectVal*2.0/5.0;
        if (divider<10) divider=10;
        cycles=got.rate*pow(2,MASTER_CLOCK_PREC)/divider;
        clockDrift=0;
        break;
      case 0xf1: // single pitch ramp up
      case 0xf2: // single pitch ramp down
        if (effect==0xf1) {
          chan[i].portaNote=song.limitSlides?0x60:255;
        } else {
          chan[i].portaNote=song.limitSlides?disCont[dispatchOfChan[i]].dispatch->getPortaFloor(dispatchChanOfChan[i]):-60;
        }
        chan[i].portaSpeed=effectVal;
        chan[i].portaStop=true;
        chan[i].nowYouCanStop=false;
        chan[i].stopOnOff=false;
        chan[i].scheduledSlideReset=false;
        chan[i].inPorta=false;
        if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,0));
        dispatchCmd(DivCommand(DIV_CMD_NOTE_PORTA,i,chan[i].portaSpeed,chan[i].portaNote));
        chan[i].portaNote=-1;
        chan[i].portaSpeed=-1;
        chan[i].inPorta=false;
        if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        break;
      case 0xf8: // single volume ramp up
        chan[i].volume=MIN(chan[i].volume+effectVal*256,chan[i].volMax);
        dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
        break;
      case 0xf9: // single volume ramp down
        chan[i].volume=MAX(chan[i].volume-effectVal*256,0);
        dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
        break;
      case 0xfa: // fast volume ramp
        if (effectVal!=0) {
          if ((effectVal&15)!=0) {
            chan[i].volSpeed=-(effectVal&15)*256;
          } else {
            chan[i].volSpeed=(effectVal>>4)*256;
          }
        } else {
          chan[i].volSpeed=0;
        }
        break;
      
      case 0xff: // stop song
        freelance=false;
        playing=false;
        extValuePresent=false;
        stepPlay=0;
        remainingLoops=-1;
        sPreview.sample=-1;
        sPreview.wave=-1;
        sPreview.pos=0;
        break;
    }
  }

  if (chan[i].doNote) {
    if (!song.continuousVibrato) {
      chan[i].vibratoPos=0;
    }
    dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
    if (chan[i].legato) {
      dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
    } else {
      if (chan[i].inPorta && chan[i].keyOn && !chan[i].shorthandPorta) {
        chan[i].portaNote=chan[i].note;
      } else if (!chan[i].noteOnInhibit) {
        dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,i,chan[i].note,chan[i].volume>>8));
        keyHit[i]=true;
      }
    }
    chan[i].doNote=false;
    if (!chan[i].keyOn && chan[i].scheduledSlideReset) {
      chan[i].portaNote=-1;
      chan[i].portaSpeed=-1;
      chan[i].scheduledSlideReset=false;
      chan[i].inPorta=false;
    }
    if (!chan[i].keyOn && chan[i].volume>chan[i].volMax) {
      chan[i].volume=chan[i].volMax;
      dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
    }
    chan[i].keyOn=true;
    chan[i].keyOff=false;
  }
  chan[i].nowYouCanStop=true;
  chan[i].shorthandPorta=false;
  chan[i].noteOnInhibit=false;

  // post effects
  for (int j=0; j<song.pat[i].effectRows; j++) {
    short effect=pat->data[whatRow][4+(j<<1)];
    short effectVal=pat->data[whatRow][5+(j<<1)];

    if (effectVal==-1) effectVal=0;
    perSystemPostEffect(i,effect,effectVal);
  }
}

void DivEngine::nextRow() {
  static char pb[4096];
  static char pb1[4096];
  static char pb2[4096];
  static char pb3[4096];
  if (view==DIV_STATUS_PATTERN) {
    strcpy(pb1,"");
    strcpy(pb3,"");
    for (int i=0; i<chans; i++) {
      snprintf(pb,4095," %.2x",song.orders.ord[i][curOrder]);
      strcat(pb1,pb);
      
      DivPattern* pat=song.pat[i].getPattern(song.orders.ord[i][curOrder],false);
      snprintf(pb2,4095,"\x1b[37m %s",
              formatNote(pat->data[curRow][0],pat->data[curRow][1]));
      strcat(pb3,pb2);
      if (pat->data[curRow][3]==-1) {
        strcat(pb3,"\x1b[m--");
      } else {
        snprintf(pb2,4095,"\x1b[1;32m%.2x",pat->data[curRow][3]);
        strcat(pb3,pb2);
      }
      if (pat->data[curRow][2]==-1) {
        strcat(pb3,"\x1b[m--");
      } else {
        snprintf(pb2,4095,"\x1b[0;36m%.2x",pat->data[curRow][2]);
        strcat(pb3,pb2);
      }
      for (int j=0; j<song.pat[i].effectRows; j++) {
        if (pat->data[curRow][4+(j<<1)]==-1) {
          strcat(pb3,"\x1b[m--");
        } else {
          snprintf(pb2,4095,"\x1b[1;31m%.2x",pat->data[curRow][4+(j<<1)]);
          strcat(pb3,pb2);
        }
        if (pat->data[curRow][5+(j<<1)]==-1) {
          strcat(pb3,"\x1b[m--");
        } else {
          snprintf(pb2,4095,"\x1b[1;37m%.2x",pat->data[curRow][5+(j<<1)]);
          strcat(pb3,pb2);
        }
      }
    }
    printf("| %.2x:%s | \x1b[1;33m%3d%s\x1b[m\n",curOrder,pb1,curRow,pb3);
  }

  for (int i=0; i<chans; i++) {
    chan[i].rowDelay=0;
    processRow(i,false);
  }

  if (changeOrd!=-1) {
    if (repeatPattern) {
      curRow=0;
      changeOrd=-1;
    } else {
      curRow=changePos;
      if (changeOrd==-2) changeOrd=curOrder+1;
      if (changeOrd<=curOrder) endOfSong=true;
      curOrder=changeOrd;
      if (curOrder>=song.ordersLen) {
        curOrder=0;
        endOfSong=true;
      }
      changeOrd=-1;
    }
    if (haltOn==DIV_HALT_PATTERN) halted=true;
  } else if (playing) if (++curRow>=song.patLen) {
    nextOrder();
    if (haltOn==DIV_HALT_PATTERN) halted=true;
  }

  if (speedAB) {
    ticks=speed2*(song.timeBase+1);
    nextSpeed=speed1;
  } else {
    ticks=speed1*(song.timeBase+1);
    nextSpeed=speed2;
  }
  speedAB=!speedAB;

  // post row details
  for (int i=0; i<chans; i++) {
    DivPattern* pat=song.pat[i].getPattern(song.orders.ord[i][curOrder],false);
    if (!(pat->data[curRow][0]==0 && pat->data[curRow][1]==0)) {
      if (pat->data[curRow][0]!=100 && pat->data[curRow][0]!=101 && pat->data[curRow][0]!=102) {
        if (!chan[i].legato) {
          dispatchCmd(DivCommand(DIV_CMD_PRE_NOTE,i,ticks));

          if (song.oneTickCut) {
            bool doPrepareCut=true;

            for (int j=0; j<song.pat[i].effectRows; j++) {
              if (pat->data[curRow][4+(j<<1)]==0x03) {
                doPrepareCut=false;
                break;
              }
              if (pat->data[curRow][4+(j<<1)]==0xea) {
                if (pat->data[curRow][5+(j<<1)]>0) {
                  doPrepareCut=false;
                  break;
                }
              }
            }
            if (doPrepareCut) chan[i].cut=ticks;
          }
        }
      }
    }
  }

  if (haltOn==DIV_HALT_ROW) halted=true;
}

bool DivEngine::nextTick(bool noAccum) {
  bool ret=false;
  if (divider<10) divider=10;
  
  cycles=got.rate*pow(2,MASTER_CLOCK_PREC)/divider;
  clockDrift+=fmod(got.rate*pow(2,MASTER_CLOCK_PREC),(double)divider);
  if (clockDrift>=divider) {
    clockDrift-=divider;
    cycles++;
  }

  while (!pendingNotes.empty()) {
    DivNoteEvent& note=pendingNotes.front();
    if (note.on) {
      dispatchCmd(DivCommand(DIV_CMD_INSTRUMENT,note.channel,note.ins,1));
      dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,note.channel,note.note));
      keyHit[note.channel]=true;
      chan[note.channel].noteOnInhibit=true;
    } else {
      dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF,note.channel));
    }
    pendingNotes.pop();
  }

  if (!freelance) {
    if (stepPlay!=1) if (--ticks<=0) {
      ret=endOfSong;
      if (endOfSong) {
        if (song.loopModality!=2) {
          playSub(true);
        }
      }
      endOfSong=false;
      if (stepPlay==2) stepPlay=1;
      nextRow();
    }
    // process stuff
    for (int i=0; i<chans; i++) {
      if (chan[i].rowDelay>0) {
        if (--chan[i].rowDelay==0) {
          processRow(i,true);
        }
      }
      if (chan[i].retrigSpeed) {
        if (--chan[i].retrigTick<0) {
          chan[i].retrigTick=chan[i].retrigSpeed-1;
          dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,i,DIV_NOTE_NULL));
          keyHit[i]=true;
        }
      }
      if (chan[i].volSpeed!=0) {
        chan[i].volume=(chan[i].volume&0xff)|(dispatchCmd(DivCommand(DIV_CMD_GET_VOLUME,i))<<8);
        chan[i].volume+=chan[i].volSpeed;
        if (chan[i].volume>chan[i].volMax) {
          chan[i].volume=chan[i].volMax;
          chan[i].volSpeed=0;
          dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
        } else if (chan[i].volume<0) {
          chan[i].volSpeed=0;
          if (song.legacyVolumeSlides) {
            chan[i].volume=chan[i].volMax+1;
          } else {
            chan[i].volume=0;
          }
          dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
        } else {
          dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
        }
      }
      if (chan[i].vibratoDepth>0) {
        chan[i].vibratoPos+=chan[i].vibratoRate;
        if (chan[i].vibratoPos>=64) chan[i].vibratoPos-=64;
        switch (chan[i].vibratoDir) {
          case 1: // up
            dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(MAX(0,(chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
            break;
          case 2: // down
            dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(MIN(0,(chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
            break;
          default: // both
            dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
            break;
        }
      }
      if ((chan[i].keyOn || chan[i].keyOff) && chan[i].portaSpeed>0) {
        if (dispatchCmd(DivCommand(DIV_CMD_NOTE_PORTA,i,chan[i].portaSpeed,chan[i].portaNote))==2 && chan[i].portaStop && song.targetResetsSlides) {
          chan[i].portaSpeed=0;
          chan[i].oldNote=chan[i].note;
          chan[i].note=chan[i].portaNote;
          chan[i].inPorta=false;
          dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
        }
      }
      if (chan[i].cut>0) {
        if (--chan[i].cut<1) {
          chan[i].oldNote=chan[i].note;
          //chan[i].note=-1;
          if (chan[i].inPorta && song.noteOffResetsSlides) {
            chan[i].keyOff=true;
            chan[i].keyOn=false;
            if (chan[i].stopOnOff) {
              chan[i].portaNote=-1;
              chan[i].portaSpeed=-1;
              chan[i].stopOnOff=false;
            }
            if (disCont[dispatchOfChan[i]].dispatch->keyOffAffectsPorta(dispatchChanOfChan[i])) {
              chan[i].portaNote=-1;
              chan[i].portaSpeed=-1;
              if (i==2 && sysOfChan[i]==DIV_SYSTEM_SMS) {
                chan[i+1].portaNote=-1;
                chan[i+1].portaSpeed=-1;
              }
            }
            dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
            chan[i].scheduledSlideReset=true;
          }
          dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF,i));
        }
      }
      if (chan[i].resetArp) {
        dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
        chan[i].resetArp=false;
      }
      if (chan[i].arp!=0 && !chan[i].arpYield && chan[i].portaSpeed<1) {
        if (--chan[i].arpTicks<1) {
          chan[i].arpTicks=song.arpLen;
          chan[i].arpStage++;
          if (chan[i].arpStage>2) chan[i].arpStage=0;
          switch (chan[i].arpStage) {
            case 0:
              dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
              break;
            case 1:
              dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note+(chan[i].arp>>4)));
              break;
            case 2:
              dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note+(chan[i].arp&15)));
              break;
          }
        }
      } else {
        chan[i].arpYield=false;
      }
    }
  }

  // system tick
  for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->tick();

  if (!freelance) {
    if (stepPlay!=1) {
      if (!noAccum) {
        totalTicksR++;
        totalTicks+=1000000/divider;
      }
      if (totalTicks>=1000000) {
        totalTicks-=1000000;
        totalSeconds++;
        cmdsPerSecond=totalCmds-lastCmds;
        lastCmds=totalCmds;
      }
    }

    if (consoleMode) fprintf(stderr,"\x1b[2K> %d:%.2d:%.2d.%.2d  %.2x/%.2x:%.3d/%.3d  %4dcmd/s\x1b[G",totalSeconds/3600,(totalSeconds/60)%60,totalSeconds%60,totalTicks/10000,curOrder,song.ordersLen,curRow,song.patLen,cmdsPerSecond);
  }

  if (haltOn==DIV_HALT_TICK) halted=true;

  return ret;
}

void DivEngine::nextBuf(float** in, float** out, int inChans, int outChans, unsigned int size) {
  if (out!=NULL) {
    memset(out[0],0,size*sizeof(float));
    memset(out[1],0,size*sizeof(float));
  }

  isBusy.lock();
  got.bufsize=size;
  
  if (out!=NULL && ((sPreview.sample>=0 && sPreview.sample<(int)song.sample.size()) || (sPreview.wave>=0 && sPreview.wave<(int)song.wave.size()))) {
    unsigned int samp_bbOff=0;
    unsigned int prevAvail=blip_samples_avail(samp_bb);
    if (prevAvail>size) prevAvail=size;
    if (prevAvail>0) {
      blip_read_samples(samp_bb,samp_bbOut,prevAvail,0);
      samp_bbOff=prevAvail;
    }
    size_t prevtotal=blip_clocks_needed(samp_bb,size-prevAvail);

    if (sPreview.sample>=0 && sPreview.sample<(int)song.sample.size()) {
      DivSample* s=song.sample[sPreview.sample];

      for (size_t i=0; i<prevtotal; i++) {
        if (sPreview.pos>=s->samples) {
          samp_temp=0;
        } else {
          samp_temp=s->data16[sPreview.pos++];
        }
        blip_add_delta(samp_bb,i,samp_temp-samp_prevSample);
        samp_prevSample=samp_temp;

        if (sPreview.pos>=s->samples) {
          if (s->loopStart>=0 && s->loopStart<(int)s->samples) {
            sPreview.pos=s->loopStart;
          }
        }
      }

      if (sPreview.pos>=s->samples) {
        if (s->loopStart>=0 && s->loopStart<(int)s->samples) {
          sPreview.pos=s->loopStart;
        } else {
          sPreview.sample=-1;
        }
      }
    } else if (sPreview.wave>=0 && sPreview.wave<(int)song.wave.size()) {
      DivWavetable* wave=song.wave[sPreview.wave];
      for (size_t i=0; i<prevtotal; i++) {
        if (wave->max<=0) {
          samp_temp=0;
        } else {
          samp_temp=((MIN(wave->data[sPreview.pos],wave->max)<<14)/wave->max)-8192;
        }
        if (++sPreview.pos>=(unsigned int)wave->len) {
          sPreview.pos=0;
        }
        blip_add_delta(samp_bb,i,samp_temp-samp_prevSample);
        samp_prevSample=samp_temp;
      }
    }

    blip_end_frame(samp_bb,prevtotal);
    blip_read_samples(samp_bb,samp_bbOut+samp_bbOff,size-samp_bbOff,0);
    for (size_t i=0; i<size; i++) {
      out[0][i]+=(float)samp_bbOut[i]/32768.0;
      out[1][i]+=(float)samp_bbOut[i]/32768.0;
    }
  }

  if (!playing) {
    if (out!=NULL) {
      memcpy(oscBuf[0],out[0],size*sizeof(float));
      memcpy(oscBuf[1],out[1],size*sizeof(float));
      oscSize=size;
    }
    isBusy.unlock();
    return;
  }

  // logic starts here
  size_t runtotal[32];
  size_t runLeft[32];
  size_t runPos[32];
  size_t lastAvail[32];
  for (int i=0; i<song.systemLen; i++) {
    lastAvail[i]=blip_samples_avail(disCont[i].bb[0]);
    if (lastAvail[i]>0) {
      disCont[i].flush(lastAvail[i]);
    }
    runtotal[i]=blip_clocks_needed(disCont[i].bb[0],size-lastAvail[i]);
    if (runtotal[i]>disCont[i].bbInLen) {
      delete disCont[i].bbIn[0];
      delete disCont[i].bbIn[1];
      disCont[i].bbIn[0]=new short[runtotal[i]+256];
      disCont[i].bbIn[1]=new short[runtotal[i]+256];
      disCont[i].bbInLen=runtotal[i]+256;
    }
    runLeft[i]=runtotal[i];
    runPos[i]=0;
  }

  if (metroTickLen<size) {
    if (metroTick!=NULL) delete[] metroTick;
    metroTick=new unsigned char[size];
    metroTickLen=size;
  }

  memset(metroTick,0,size);

  int attempts=0;
  int runLeftG=size<<MASTER_CLOCK_PREC;
  while (++attempts<100) {
    // 0. check if we've halted
    if (halted) break;
    // 1. check whether we are done with all buffers
    if (runLeftG<=0) break;

    // 2. check whether we gonna tick
    if (cycles<=0) {
      // we have to tick
      if (!freelance && stepPlay!=-1) {
        unsigned int realPos=size-(runLeftG>>MASTER_CLOCK_PREC);
        if (realPos>=size) realPos=size-1;
        if (song.hilightA>0) {
          if ((curRow%song.hilightA)==0 && ticks==1) metroTick[realPos]=1;
        }
        if (song.hilightB>0) {
          if ((curRow%song.hilightB)==0 && ticks==1) metroTick[realPos]=2;
        }
      }
      if (nextTick()) {
        if (remainingLoops>0) {
          remainingLoops--;
          if (!remainingLoops) {
            logI("end of song!\n");
            remainingLoops=-1;
            playing=false;
            freelance=false;
            extValuePresent=false;
            break;
          }
        }
      }
    } else {
      // 3. tick the clock and fill buffers as needed
      if (cycles<runLeftG) {
        for (int i=0; i<song.systemLen; i++) {
          int total=(cycles*runtotal[i])/(size<<MASTER_CLOCK_PREC);
          disCont[i].acquire(runPos[i],total);
          runLeft[i]-=total;
          runPos[i]+=total;
        }
        runLeftG-=cycles;
        cycles=0;
      } else {
        cycles-=runLeftG;
        runLeftG=0;
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].acquire(runPos[i],runLeft[i]);
          runLeft[i]=0;
        }
      }
    }
  }

  if (out==NULL || halted) {
    isBusy.unlock();
    return;
  }

  //logD("attempts: %d\n",attempts);
  if (attempts>=100) {
    logE("hang detected! stopping! at %d seconds %d micro\n",totalSeconds,totalTicks);
    freelance=false;
    playing=false;
    extValuePresent=false;
  }
  totalProcessed=size-(runLeftG>>MASTER_CLOCK_PREC);

  for (int i=0; i<song.systemLen; i++) {
    disCont[i].fillBuf(runtotal[i],lastAvail[i],size-lastAvail[i]);
  }

  for (int i=0; i<song.systemLen; i++) {
    float volL=((float)song.systemVol[i]/64.0f)*((float)MIN(127,127-(int)song.systemPan[i])/127.0f)*song.masterVol;
    float volR=((float)song.systemVol[i]/64.0f)*((float)MIN(127,127+(int)song.systemPan[i])/127.0f)*song.masterVol;
    if (disCont[i].dispatch->isStereo()) {
      for (size_t j=0; j<size; j++) {
        out[0][j]+=((float)disCont[i].bbOut[0][j]/32768.0)*volL;
        out[1][j]+=((float)disCont[i].bbOut[1][j]/32768.0)*volR;
      }
    } else {
      for (size_t j=0; j<size; j++) {
        out[0][j]+=((float)disCont[i].bbOut[0][j]/32768.0)*volL;
        out[1][j]+=((float)disCont[i].bbOut[0][j]/32768.0)*volR;
      }
    }
  }

  if (metronome) for (size_t i=0; i<size; i++) {
    if (metroTick[i]) {
      if (metroTick[i]==2) {
        metroFreq=1400/got.rate;
      } else {
        metroFreq=1050/got.rate;
      }
      metroPos=0;
      metroAmp=0.7f;
    }
    if (metroAmp>0.0f) {
      out[0][i]+=(sin(metroPos*2*M_PI))*metroAmp;
      out[1][i]+=(sin(metroPos*2*M_PI))*metroAmp;
    }
    metroAmp-=0.0003f;
    if (metroAmp<0.0f) metroAmp=0.0f;
    metroPos+=metroFreq;
    while (metroPos>=1) metroPos--;
  }

  memcpy(oscBuf[0],out[0],size*sizeof(float));
  memcpy(oscBuf[1],out[1],size*sizeof(float));
  oscSize=size;

  if (forceMono) {
    for (size_t i=0; i<size; i++) {
      out[0][i]=(out[0][i]+out[1][i])*0.5;
      out[1][i]=out[0][i];
    }
  }
  isBusy.unlock();
}
