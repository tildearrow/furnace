/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

#define _USE_MATH_DEFINES
#include "cmdStream.h"
#include "dispatch.h"
#include "engine.h"
#include "../ta-log.h"

bool DivCSChannelState::doCall(unsigned int addr) {
  if (callStackPos>=DIV_MAX_CSSTACK) {
    readPos=0;
    return false;
  }

  callStack[callStackPos++]=readPos;
  readPos=addr;

  return true;
}

unsigned char* DivCSPlayer::getData() {
  return b;
}

unsigned short* DivCSPlayer::getDataAccess() {
  return bAccessTS;
}

size_t DivCSPlayer::getDataLen() {
  return bLen;
}

DivCSChannelState* DivCSPlayer::getChanState(int ch) {
  return &chan[ch];
}

unsigned int DivCSPlayer::getFileChans() {
  return fileChans;
}

unsigned char* DivCSPlayer::getFastDelays() {
  return fastDelays;
}

unsigned char* DivCSPlayer::getFastIns() {
  return fastIns;
}

unsigned char* DivCSPlayer::getFastVols() {
  return fastVols;
}

unsigned char* DivCSPlayer::getFastCmds() {
  return fastCmds;
}

unsigned int DivCSPlayer::getCurTick() {
  return curTick;
}

void DivCSPlayer::cleanup() {
  delete[] b;
  b=NULL;
  bLen=0;

  if (bAccessTS) {
    delete[] bAccessTS;
    bAccessTS=NULL;
  }
}

bool DivCSPlayer::tick() {
  bool ticked=false;
  for (int i=0; i<e->getTotalChannelCount(); i++) {
    bool sendVolume=false;
    bool sendPitch=false;
    if (chan[i].readPos==0) continue;

    ticked=true;

    chan[i].waitTicks--;
    while (chan[i].waitTicks<=0) {
      if (!stream.seek(chan[i].readPos,SEEK_SET)) {
        logE("%d: access violation! $%x",i,chan[i].readPos);
        chan[i].readPos=0;
        break;
      }

      unsigned int accessTSBegin=stream.tell();

      chan[i].trace[chan[i].tracePos++]=chan[i].readPos;
      if (chan[i].tracePos>=DIV_MAX_CSTRACE) {
        chan[i].tracePos=0;
      }

      unsigned char next=stream.readC();
      unsigned char command=0;
      bool mustTell=true;

      if (next<0xb3) { // note
        e->dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,i,(int)next-60));
        chan[i].note=(int)next-60;
        chan[i].vibratoPos=0;
      } else if (next>=0xf0) { // preset delay
        chan[i].waitTicks=fastDelays[next&15];
        chan[i].lastWaitLen=chan[i].waitTicks;
        bAccessTS[fastDelaysOff+(next&15)]=curTick;
      } else switch (next) {
        case 0xb4: // note on null
          e->dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,i,DIV_NOTE_NULL));
          chan[i].vibratoPos=0;
          break;
        case 0xb5: // note off
          e->dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF,i));
          break;
        case 0xb6: // note off env
          e->dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF_ENV,i));
          break;
        case 0xb7: // env release
          e->dispatchCmd(DivCommand(DIV_CMD_ENV_RELEASE,i));
          break;
        case 0xb8:
          command=DIV_CMD_INSTRUMENT;
          break;
        case 0xc0:
          command=DIV_CMD_PRE_PORTA;
          break;
        case 0xc1: // arp time
          arpSpeed=(unsigned char)stream.readC();
          break;
        case 0xc2: { // vibrato
          unsigned char param=stream.readC();
          chan[i].vibratoDepth=param&15;
          chan[i].vibratoRate=param>>4;
          sendPitch=true;
          break;
        }
        case 0xc3: // vibrato range
          chan[i].vibratoRange=(unsigned char)stream.readC();
          break;
        case 0xc4: // vibrato shape
          chan[i].vibratoShape=(unsigned char)stream.readC();
          break;
        case 0xc5: // pitch
          chan[i].pitch=(signed char)stream.readC();
          sendPitch=true;
          break;
        case 0xc6: // arpeggio
          chan[i].arp=(unsigned char)stream.readC();
          break;
        case 0xc7: // volume
          chan[i].volume=((unsigned char)stream.readC())<<8;
          sendVolume=true;
          break;
        case 0xc8: // vol slide
          chan[i].volSpeed=(short)(bigEndian?stream.readS_BE():stream.readS());
          chan[i].volSpeedTarget=-1;
          chan[i].tremoloDepth=0;
          break;
        case 0xc9: // porta
          chan[i].portaTarget=(int)((unsigned char)stream.readC())-60;
          chan[i].portaSpeed=(unsigned char)stream.readC();
          break;
        case 0xca: { // legato
          int arg0=(unsigned char)stream.readC();
          if (arg0==0xff) {
            arg0=DIV_NOTE_NULL;
          } else {
            arg0-=60;
          }
          chan[i].note=arg0;
          e->dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
          break;
        }
        case 0xcb: { // vol slide target
          int arg0=(short)(bigEndian?stream.readS_BE():stream.readS());
          int arg1=(short)(bigEndian?stream.readS_BE():stream.readS());
          chan[i].volSpeed=arg0;
          chan[i].volSpeedTarget=arg0==0 ? -1 : arg1;
          break;
        }
        case 0xcc: { // tremolo
          unsigned char param=stream.readC();
          chan[i].tremoloDepth=param&15;
          chan[i].tremoloRate=param>>4;
          if (chan[i].tremoloDepth==0) {
            chan[i].tremoloPos=0;
          }
          chan[i].volSpeed=0;
          chan[i].volSpeedTarget=-1;
          sendVolume=true;
          break;
        }
        case 0xcd: { // panbrello
          unsigned char param=stream.readC();
          chan[i].panbrelloDepth=param&15;
          chan[i].panbrelloRate=param>>4;
          if (chan[i].panbrelloDepth==0) {
            chan[i].panbrelloPos=0;
          } else {
            chan[i].panSpeed=0;
          }
          break;
        }
        case 0xce: // pan slide
          chan[i].panSpeed=stream.readC();
          if (chan[i].panSpeed) {
            // panbrello and slides are incompatible
            chan[i].panbrelloDepth=0;
            chan[i].panbrelloRate=0;
            chan[i].panbrelloPos=0;
          }
          break;
        case 0xcf: { // panning
          chan[i].panL=(unsigned char)stream.readC();
          chan[i].panR=(unsigned char)stream.readC();
          e->dispatchCmd(DivCommand(DIV_CMD_PANNING,i,chan[i].panL,chan[i].panR));
          break;
        }
        case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5:
          e->dispatchCmd(DivCommand(DIV_CMD_INSTRUMENT,i,fastIns[next-0xe0]));
          bAccessTS[fastInsOff+(next-0xe0)]=curTick;
          break;
        case 0xe6: case 0xe7: case 0xe8: case 0xe9: case 0xea: case 0xeb:
          chan[i].volume=fastVols[next-0xe6]<<8;
          bAccessTS[fastVolsOff+(next-0xe6)]=curTick;
          sendVolume=true;
          break;
        case 0xec: case 0xed: case 0xee: case 0xef:
          command=fastCmds[next&3];
          bAccessTS[fastCmdsOff+(next&3)]=curTick;
          break;
        case 0xd0: // placeholder
          stream.readC();
          stream.readC();
          stream.readC();
          break;
        case 0xd1: // nop
          break;
        case 0xd6: // note off + wait 1
          e->dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF,i));
          chan[i].waitTicks=1;
          chan[i].lastWaitLen=chan[i].waitTicks;
          break;
        case 0xd7:
          command=stream.readC();
          break;
        case 0xd8: {
          unsigned int callAddr=bigEndian?((unsigned short)stream.readS_BE()):((unsigned short)stream.readS());
          chan[i].readPos=stream.tell();
          if (!chan[i].doCall(callAddr)) {
            logE("%d: (call) stack error!",i);
            chan[i].readPos=0;
          }
          mustTell=false;
          break;
        }
        case 0xd5: {
          unsigned int callAddr=bigEndian?stream.readI_BE():stream.readI();
          chan[i].readPos=stream.tell();
          if (!chan[i].doCall(callAddr)) {
            logE("%d: (calli) stack error!",i);
            chan[i].readPos=0;
          }
          mustTell=false;
          break;
        }
        case 0xd4: {
          logE("%d: (callsym) not supported here!",i);
          chan[i].readPos=0;
          break;
        }
        case 0xd9:
          if (!chan[i].callStackPos) {
            logE("%d: (ret) stack error!",i);
            chan[i].readPos=0;
            break;
          }
          chan[i].readPos=chan[i].callStack[--chan[i].callStackPos];
          mustTell=false;
          break;
        case 0xda:
          chan[i].readPos=bigEndian?stream.readI_BE():stream.readI();
          mustTell=false;
          break;
        case 0xdb:
          logE("TODO: RATE");
          stream.readI();
          break;
        case 0xdc:
          chan[i].waitTicks=(unsigned short)(bigEndian?stream.readS_BE():stream.readS());
          chan[i].lastWaitLen=chan[i].waitTicks;
          break;
        case 0xdd:
          chan[i].waitTicks=(unsigned char)stream.readC();
          chan[i].lastWaitLen=chan[i].waitTicks;
          break;
        case 0xde:
          chan[i].waitTicks=1;
          chan[i].lastWaitLen=chan[i].waitTicks;
          break;
        case 0xdf:
          chan[i].readPos=0;
          mustTell=false;
          logI("%d: stop",i,chan[i].readPos);
          break;
        default:
          logE("%d: illegal instruction $%.2x! $%.x",i,next,chan[i].readPos);
          chan[i].readPos=0;
          break;
      }

      if (chan[i].readPos==0) break;

      if (command) {
        int arg0=0;
        int arg1=0;
        switch (command) {
          case DIV_CMD_INSTRUMENT:
            arg0=(unsigned char)stream.readC();
            break;
          case DIV_CMD_PRE_PORTA:
            arg0=(unsigned char)stream.readC();
            arg1=(arg0&0x40)?1:0;
            arg0=(arg0&0x80)?1:0;
            break;
          // ONE BYTE COMMANDS
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
          case DIV_CMD_REMOVED_SPECIAL_WAVE:
          case DIV_CMD_REMOVED_RING_MOD_SRC:
          case DIV_CMD_REMOVED_HARD_SYNC_SRC:
          case DIV_CMD_REMOVED_PHASE_MOD_SRC:
          case DIV_CMD_REMOVED_WAVE_MIX:
          case DIV_CMD_REMOVED_1_BIT_NOISE:
          case DIV_CMD_REMOVED_CHANNEL_INVERSION:
          case DIV_CMD_REMOVED_FILTER_CONNECTION:
          case DIV_CMD_REMOVED_FILTER_MATRIX:
          case DIV_CMD_REMOVED_FILTER_ENABLE:
          case DIV_CMD_REMOVED_PHASE_RESET:
          case DIV_CMD_REMOVED_NOISE_PHASE_RESET:
          case DIV_CMD_REMOVED_ENVELOPE_RESET:
          case DIV_CMD_REMOVED_CUTOFF_SCALING:
          case DIV_CMD_REMOVED_RESONANCE_SCALING:
          case DIV_CMD_WS_GLOBAL_SPEAKER_VOLUME:
          case DIV_CMD_FM_ALG:
          case DIV_CMD_FM_FMS:
          case DIV_CMD_FM_AMS:
          case DIV_CMD_FM_FMS2:
          case DIV_CMD_FM_AMS2:
            arg0=(unsigned char)stream.readC();
            break;
          // TWO BYTE COMMANDS
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
          case DIV_CMD_REMOVED_LFSR_FEEDBACK_BITS:
          case DIV_CMD_REMOVED_FILTER_DISTORTION:
          case DIV_CMD_REMOVED_FILTER_OUTPUT_VOLUME:
          case DIV_CMD_C64_PW_SLIDE:
          case DIV_CMD_C64_CUTOFF_SLIDE:
          case DIV_CMD_N163_WAVE_POSITION:
          case DIV_CMD_N163_WAVE_LENGTH:
            arg0=(unsigned char)stream.readC();
            arg1=(unsigned char)stream.readC();
            break;
          // ONE SHORT COMMANDS
          case DIV_CMD_C64_FINE_DUTY:
          case DIV_CMD_C64_FINE_CUTOFF:
          case DIV_CMD_LYNX_LFSR_LOAD:
          case DIV_CMD_QSOUND_ECHO_DELAY:
          case DIV_CMD_ES5506_ENVELOPE_COUNT:
            arg0=(unsigned short)(bigEndian?stream.readS_BE():stream.readS());
            break;
          // TWO SHORT COMMANDS
          case DIV_CMD_ES5506_FILTER_K1:
          case DIV_CMD_ES5506_FILTER_K2:
            arg0=(unsigned short)(bigEndian?stream.readS_BE():stream.readS());
            arg1=(unsigned short)(bigEndian?stream.readS_BE():stream.readS());
            break;
          case DIV_CMD_FM_FIXFREQ:
            arg0=(unsigned short)(bigEndian?stream.readS_BE():stream.readS());
            arg1=arg0&0x7ff;
            arg0>>=12;
            break;
          case DIV_CMD_NES_SWEEP:
            arg0=(unsigned char)stream.readC();
            arg1=arg0&0x77;
            arg0=(arg0&8)?1:0;
            break;
          case DIV_CMD_SAMPLE_POS:
            arg0=(unsigned int)(bigEndian?stream.readI_BE():stream.readI());
            break;
        }

        // dispatch it
        e->dispatchCmd(DivCommand((DivDispatchCmds)command,i,arg0,arg1));
      }

      for (unsigned int j=accessTSBegin; j<stream.tell(); j++) {
        if (j<bLen) {
          bAccessTS[j]=curTick;
        }
      }

      if (mustTell) chan[i].readPos=stream.tell();
    }

    if (sendVolume || chan[i].volSpeed!=0 || chan[i].tremoloDepth!=0) {
      int preSpeedVol=chan[i].volume;
      chan[i].volume+=chan[i].volSpeed;
      if (chan[i].volSpeedTarget!=-1) {
        bool atTarget=false;
        if (chan[i].volSpeed>0) {
          atTarget=(chan[i].volume>=chan[i].volSpeedTarget);
        } else if (chan[i].volSpeed<0) {
          atTarget=(chan[i].volume<=chan[i].volSpeedTarget);
        } else {
          atTarget=true;
          chan[i].volSpeedTarget=chan[i].volume;
        }

        if (atTarget) {
          if (chan[i].volSpeed>0) {
            chan[i].volume=MAX(preSpeedVol,chan[i].volSpeedTarget);
          } else if (chan[i].volSpeed<0) {
            chan[i].volume=MIN(preSpeedVol,chan[i].volSpeedTarget);
          }
          chan[i].volSpeed=0;
          chan[i].volSpeedTarget=-1;
        }
      }

      if (chan[i].volume<0) {
        chan[i].volume=0;
      }
      if (chan[i].volume>chan[i].volMax) {
        chan[i].volume=chan[i].volMax;
      }
      if (chan[i].tremoloDepth>0) {
        chan[i].tremoloPos+=chan[i].tremoloRate;
        chan[i].tremoloPos&=127;
        e->dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,MAX(0,chan[i].volume-(tremTable[chan[i].tremoloPos]*chan[i].tremoloDepth))>>8));
      } else {
        e->dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
      }
    }

    if (sendPitch || chan[i].vibratoDepth!=0) {
      if (chan[i].vibratoDepth>0) {
        chan[i].vibratoPos+=chan[i].vibratoRate;
        if (chan[i].vibratoPos>=64) chan[i].vibratoPos-=64;
      }
      e->dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(vibTable[chan[i].vibratoPos&63]*chan[i].vibratoDepth)/15));
    }

    if (chan[i].portaSpeed) {
      e->dispatchCmd(DivCommand(DIV_CMD_NOTE_PORTA,i,chan[i].portaSpeed*(e->song.compatFlags.linearPitch?e->song.compatFlags.pitchSlideSpeed:1),chan[i].portaTarget));
    }
    if (chan[i].arp && !chan[i].portaSpeed) {
      if (chan[i].arpTicks==0) {
        switch (chan[i].arpStage) {
          case 0:
            e->dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
            break;
          case 1:
            e->dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note+(chan[i].arp>>4)));
            break;
          case 2:
            e->dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note+(chan[i].arp&15)));
            break;
        }
        chan[i].arpStage++;
        if (chan[i].arpStage>=3) chan[i].arpStage=0;
        chan[i].arpTicks=arpSpeed;
      }
      chan[i].arpTicks--;
    }

    if (chan[i].panSpeed) {
      int newPanL=chan[i].panL;
      int newPanR=chan[i].panR;
      // increase one side until it has reached max. then decrease the other.
      if (chan[i].panSpeed>0) { // right
        if (newPanR>=0xff) {
          newPanL-=chan[i].panSpeed;
        } else {
          newPanR+=chan[i].panSpeed;
        }
      } else { // left
        if (newPanL>=0xff) {
          newPanR+=chan[i].panSpeed;
        } else {
          newPanL-=chan[i].panSpeed;
        }
      }

      // clamp to boundaries
      if (newPanL<0) newPanL=0;
      if (newPanL>0xff) newPanL=0xff;
      if (newPanR<0) newPanR=0;
      if (newPanR>0xff) newPanR=0xff;

      // set new pan
      chan[i].panL=newPanL;
      chan[i].panR=newPanR;

      // send panning command
      e->dispatchCmd(DivCommand(DIV_CMD_PANNING,i,chan[i].panL,chan[i].panR));
    }

    if (chan[i].panbrelloDepth>0) {
      chan[i].panbrelloPos+=chan[i].panbrelloRate;
      chan[i].panbrelloPos&=255;

      // calculate inverted...
      // split position into four sections and calculate panning value
      switch (chan[i].panbrelloPos&0xc0) {
        case 0: // center -> right
          chan[i].panL=((chan[i].panbrelloPos&0x3f)<<2);
          chan[i].panR=0;
          break;
        case 0x40: // right -> center
          chan[i].panL=0xff-((chan[i].panbrelloPos&0x3f)<<2);
          chan[i].panR=0;
          break;
        case 0x80: // center -> left
          chan[i].panL=0;
          chan[i].panR=((chan[i].panbrelloPos&0x3f)<<2);
          break;
        case 0xc0: // left -> center
          chan[i].panL=0;
          chan[i].panR=0xff-((chan[i].panbrelloPos&0x3f)<<2);
          break;
      }

      // multiply by depth
      chan[i].panL=(chan[i].panL*chan[i].panbrelloDepth)/15;
      chan[i].panR=(chan[i].panR*chan[i].panbrelloDepth)/15;

      // then invert it to get final panning
      chan[i].panL^=0xff;
      chan[i].panR^=0xff;

      e->dispatchCmd(DivCommand(DIV_CMD_PANNING,i,chan[i].panL,chan[i].panR));
    }
  }

  // cycle over access times in order to ensure deltas are always higher than 256
  // (and prevent spurious highlights)
  for (int i=0; i<16; i++) {
    short delta=(((short)(curTick&0xffff))-(short)bAccessTS[deltaCyclePos]);
    if (delta>256) {
      bAccessTS[deltaCyclePos]=curTick-512;
    }
    if (++deltaCyclePos>=bLen) {
      deltaCyclePos=0;
    }
  }

  curTick++;

  return ticked;
}

bool DivCSPlayer::init() {
  unsigned char magic[4];
  stream.seek(0,SEEK_SET);
  stream.read(magic,4);

  if (memcmp(magic,"FCS",4)!=0) return false;

  fileChans=(unsigned short)stream.readS();
  unsigned char flags=stream.readC();
  stream.readC(); // reserved

  longPointers=flags&1;
  bigEndian=flags&2;

  if (bigEndian) fileChans=(((fileChans&0xff00)>>8)|((fileChans&0xff)<<8));

  fastDelaysOff=stream.tell();
  stream.read(fastDelays,16);
  fastInsOff=stream.tell();
  stream.read(fastIns,6);
  fastVolsOff=stream.tell();
  stream.read(fastVols,6);
  fastCmdsOff=stream.tell();
  stream.read(fastCmds,4);

  if (longPointers) {
    for (unsigned int i=0; i<fileChans; i++) {
      if (i>=DIV_MAX_CHANS) {
        stream.readI();
        continue;
      }
      if ((int)i>=e->getTotalChannelCount()) {
        stream.readI();
        continue;
      }
      if (bigEndian) {
        chan[i].startPos=stream.readI_BE();
      } else {
        chan[i].startPos=stream.readI();
      }
      chan[i].readPos=chan[i].startPos;
    }
  } else {
    for (unsigned int i=0; i<fileChans; i++) {
      if (i>=DIV_MAX_CHANS) {
        stream.readS();
        continue;
      }
      if ((int)i>=e->getTotalChannelCount()) {
        stream.readS();
        continue;
      }
      if (bigEndian) {
        chan[i].startPos=stream.readS_BE();
      } else {
        chan[i].startPos=stream.readS();
      }
      chan[i].readPos=chan[i].startPos;
    }
  }
  
  // read stack sizes
  for (unsigned int i=0; i<fileChans; i++) {
    chan[i].callStackSize=stream.readC();
  }

  // initialize state
  for (int i=0; i<e->getTotalChannelCount(); i++) {
    if (e->song.dispatchChanOfChan[i]>=0) {
      chan[i].volMax=(e->getDispatch(e->song.dispatchOfChan[i])->dispatch(DivCommand(DIV_CMD_GET_VOLMAX,e->song.dispatchChanOfChan[i]))<<8)|0xff;
    } else {
      // fallback
      chan[i].volMax=0xfff;
    }
    chan[i].volume=chan[i].volMax;
  }

  for (int i=0; i<64; i++) {
    vibTable[i]=127*sin(((double)i/64.0)*(2*M_PI));
  }
  for (int i=0; i<128; i++) {
    tremTable[i]=255*0.5*(1.0-cos(((double)i/128.0)*(2*M_PI)));
  }
  

  arpSpeed=1;
  bAccessTS=new unsigned short[bLen];
  // this value ensures all deltas are higher than 256
  memset(bAccessTS,0xc0,bLen*sizeof(unsigned short));
  curTick=0;
  deltaCyclePos=0;

  return true;
}

// DivEngine

bool DivEngine::playStream(unsigned char* f, size_t length) {
  BUSY_BEGIN;
  // kill the previous player
  if (cmdStreamInt) {
    cmdStreamInt->cleanup();
    delete cmdStreamInt;
    cmdStreamInt=NULL;
  }
  cmdStreamInt=new DivCSPlayer(this,f,length);
  if (!cmdStreamInt->init()) {
    logE("not a command stream!");
    lastError="not a command stream";
    delete[] f;
    delete cmdStreamInt;
    cmdStreamInt=NULL;
    BUSY_END;
    return false;
  }

  if (!playing) {
    reset();
    freelance=true;
    playing=true;
  }
  BUSY_END;
  return true;
}

DivCSPlayer* DivEngine::getStreamPlayer() {
  return cmdStreamInt;
}

bool DivEngine::killStream() {
  if (!cmdStreamInt) return false;
  BUSY_BEGIN;
  cmdStreamInt->cleanup();
  delete cmdStreamInt;
  cmdStreamInt=NULL;
  BUSY_END;
  return true;
}
