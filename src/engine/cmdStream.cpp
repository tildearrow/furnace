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

#define _USE_MATH_DEFINES
#include "cmdStream.h"
#include "dispatch.h"
#include "engine.h"
#include "../ta-log.h"

bool DivCSChannelState::doCall(unsigned int addr) {
  if (callStackPos>=8) {
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

size_t DivCSPlayer::getDataLen() {
  return bLen;
}

DivCSChannelState* DivCSPlayer::getChanState(int ch) {
  return &chan[ch];
}

unsigned char* DivCSPlayer::getFastDelays() {
  return fastDelays;
}

unsigned char* DivCSPlayer::getFastCmds() {
  return fastCmds;
}

void DivCSPlayer::cleanup() {
  delete b;
  b=NULL;
  bLen=0;
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
      } else if (next>=0xd0 && next<=0xdf) {
        command=fastCmds[next&15];
      } else if (next>=0xe0 && next<=0xef) { // preset delay
        chan[i].waitTicks=fastDelays[next&15];
        chan[i].lastWaitLen=chan[i].waitTicks;
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
        case 0xb8: case 0xbe: case 0xc0: case 0xc2:
        case 0xc3: case 0xc4: case 0xc5: case 0xc6:
        case 0xc7: case 0xc8: case 0xc9: case 0xca:
          command=next-0xb4;
          break;
        case 0xf7:
          command=stream.readC();
          break;
        case 0xf8: {
          unsigned int callAddr=chan[i].readPos+2+stream.readS();
          if (!chan[i].doCall(callAddr)) {
            logE("%d: (callb16) stack error!",i);
          }
          break;
        }
        case 0xf6: {
          unsigned int callAddr=chan[i].readPos+4+stream.readI();
          if (!chan[i].doCall(callAddr)) {
            logE("%d: (callb32) stack error!",i);
          }
          break;
        }
        case 0xf5: {
          unsigned int callAddr=stream.readI();
          if (!chan[i].doCall(callAddr)) {
            logE("%d: (call) stack error!",i);
          }
          break;
        }
        case 0xf4: {
          logE("%d: (callsym) not supported here!",i);
          chan[i].readPos=0;
          break;
        }
        case 0xf9:
          if (!chan[i].callStackPos) {
            logE("%d: (ret) stack error!",i);
            chan[i].readPos=0;
            break;
          }
          chan[i].readPos=chan[i].callStack[--chan[i].callStackPos];
          mustTell=false;
          break;
        case 0xfa:
          chan[i].readPos=stream.readI();
          mustTell=false;
          break;
        case 0xfb:
          logE("TODO: RATE");
          stream.readI();
          break;
        case 0xfc:
          chan[i].waitTicks=(unsigned short)stream.readS();
          chan[i].lastWaitLen=chan[i].waitTicks;
          break;
        case 0xfd:
          chan[i].waitTicks=(unsigned char)stream.readC();
          chan[i].lastWaitLen=chan[i].waitTicks;
          break;
        case 0xfe:
          chan[i].waitTicks=1;
          chan[i].lastWaitLen=chan[i].waitTicks;
          break;
        case 0xff:
          chan[i].readPos=chan[i].startPos;
          mustTell=false;
          logI("%d: stop go back to %x",i,chan[i].readPos);
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
          case DIV_CMD_HINT_VIBRATO_RANGE:
          case DIV_CMD_HINT_VIBRATO_SHAPE:
          case DIV_CMD_HINT_VOLUME:
          case DIV_CMD_HINT_ARP_TIME:
            arg0=(unsigned char)stream.readC();
            break;
          case DIV_CMD_HINT_PITCH:
            arg0=(signed char)stream.readC();
            break;
          case DIV_CMD_HINT_VIBRATO:
          case DIV_CMD_HINT_ARPEGGIO:
          case DIV_CMD_HINT_PORTA:
            arg0=(signed char)stream.readC();
            arg1=(unsigned char)stream.readC();
            break;
          case DIV_CMD_PANNING:
            arg0=(unsigned char)stream.readC();
            arg1=(unsigned char)stream.readC();
            break;
          case DIV_CMD_PRE_PORTA:
            arg0=(unsigned char)stream.readC();
            arg1=(arg0&0x40)?1:0;
            arg0=(arg0&0x80)?1:0;
            break;
          case DIV_CMD_HINT_VOL_SLIDE:
            arg0=(short)stream.readS();
            break;
          case DIV_CMD_HINT_VOL_SLIDE_TARGET:
            arg0=(short)stream.readS();
            arg1=(short)stream.readS();
            break;
          case DIV_CMD_HINT_LEGATO:
            arg0=(unsigned char)stream.readC();
            if (arg0==0xff) {
              arg0=DIV_NOTE_NULL;
            } else {
              arg0-=60;
            }
            break;
          case DIV_CMD_SAMPLE_MODE:
          case DIV_CMD_SAMPLE_FREQ:
          case DIV_CMD_SAMPLE_BANK:
          case DIV_CMD_SAMPLE_POS:
          case DIV_CMD_SAMPLE_DIR:
          case DIV_CMD_FM_HARD_RESET:
          case DIV_CMD_FM_LFO:
          case DIV_CMD_FM_LFO_WAVE:
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
            arg0=(unsigned char)stream.readC();
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
            arg0=(unsigned char)stream.readC();
            arg1=(unsigned char)stream.readC();
            break;
          case DIV_CMD_C64_FINE_DUTY:
          case DIV_CMD_C64_FINE_CUTOFF:
          case DIV_CMD_LYNX_LFSR_LOAD:
            arg0=(unsigned short)stream.readS();
            break;
          case DIV_CMD_FM_FIXFREQ:
            arg0=(unsigned short)stream.readS();
            arg1=arg0&0x7ff;
            arg0>>=12;
            break;
          case DIV_CMD_NES_SWEEP:
            arg0=(unsigned char)stream.readC();
            arg1=arg0&0x77;
            arg0=(arg0&8)?1:0;
            break;
        }

        switch (command) {
          case DIV_CMD_HINT_VOLUME:
            chan[i].volume=arg0<<8;
            sendVolume=true;
            break;
          case DIV_CMD_HINT_VOL_SLIDE:
            chan[i].volSpeed=arg0;
            chan[i].volSpeedTarget=-1;
            break;
          case DIV_CMD_HINT_VOL_SLIDE_TARGET:
            chan[i].volSpeed=arg0;
            chan[i].volSpeedTarget=arg0==0 ? -1 : arg1;
            break;
          case DIV_CMD_HINT_PITCH:
            chan[i].pitch=arg0;
            sendPitch=true;
            break;
          case DIV_CMD_HINT_VIBRATO:
            chan[i].vibratoDepth=arg0;
            chan[i].vibratoRate=arg1;
            sendPitch=true;
            break;
          case DIV_CMD_HINT_PORTA:
            chan[i].portaTarget=arg0;
            chan[i].portaSpeed=arg1;
            break;
          case DIV_CMD_HINT_LEGATO:
            chan[i].note=arg0;
            e->dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
            break;
          case DIV_CMD_HINT_ARPEGGIO:
            chan[i].arp=(((unsigned char)arg0)<<4)|(arg1&15);
            break;
          case DIV_CMD_HINT_ARP_TIME:
            arpSpeed=arg0;
            break;
          default: // dispatch it
            e->dispatchCmd(DivCommand((DivDispatchCmds)command,i,arg0,arg1));
            break;
        }
      }

      if (mustTell) chan[i].readPos=stream.tell();
    }

    if (sendVolume || chan[i].volSpeed!=0) {
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
      e->dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
    }

    if (sendPitch || chan[i].vibratoDepth!=0) {
      if (chan[i].vibratoDepth>0) {
        chan[i].vibratoPos+=chan[i].vibratoRate;
        if (chan[i].vibratoPos>=64) chan[i].vibratoPos-=64;
      }
      e->dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(vibTable[chan[i].vibratoPos&63]*chan[i].vibratoDepth)/15));
    }

    if (chan[i].portaSpeed) {
      e->dispatchCmd(DivCommand(DIV_CMD_NOTE_PORTA,i,chan[i].portaSpeed*(e->song.linearPitch==2?e->song.pitchSlideSpeed:1),chan[i].portaTarget));
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
  }

  return ticked;
}

bool DivCSPlayer::init() {
  unsigned char magic[4];
  stream.seek(0,SEEK_SET);
  stream.read(magic,4);

  if (memcmp(magic,"FCS",4)!=0) return false;

  unsigned int chans=stream.readI();

  for (unsigned int i=0; i<chans; i++) {
    if (i>=DIV_MAX_CHANS) {
      stream.readI();
      continue;
    }
    if ((int)i>=e->getTotalChannelCount()) {
      stream.readI();
      continue;
    }
    chan[i].startPos=stream.readI();
    chan[i].readPos=chan[i].startPos;
  }

  stream.read(fastDelays,16);
  stream.read(fastCmds,16);

  // initialize state
  for (int i=0; i<e->getTotalChannelCount(); i++) {
    chan[i].volMax=(e->getDispatch(e->dispatchOfChan[i])->dispatch(DivCommand(DIV_CMD_GET_VOLMAX,e->dispatchChanOfChan[i]))<<8)|0xff;
    chan[i].volume=chan[i].volMax;
  }

  for (int i=0; i<64; i++) {
    vibTable[i]=127*sin(((double)i/64.0)*(2*M_PI));
  }

  arpSpeed=1;

  return true;
}

// DivEngine

bool DivEngine::playStream(unsigned char* f, size_t length) {
  BUSY_BEGIN;
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
