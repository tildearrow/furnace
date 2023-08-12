/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

#include "dispatch.h"
#include "song.h"
#define _USE_MATH_DEFINES
#include "engine.h"
#include "instrument.h"
#include "safeReader.h"
#include "../ta-log.h"
#include "../fileutils.h"
#ifdef HAVE_SDL2
#include "../audio/sdlAudio.h"
#endif
#include <stdexcept>
#ifdef HAVE_JACK
#include "../audio/jack.h"
#endif
#include <math.h>
#include <float.h>
#ifdef HAVE_SNDFILE
#include "sfWrapper.h"
#endif
#include <fmt/printf.h>

void process(void* u, float** in, float** out, int inChans, int outChans, unsigned int size) {
  ((DivEngine*)u)->nextBuf(in,out,inChans,outChans,size);
}

const char* DivEngine::getEffectDesc(unsigned char effect, int chan, bool notNull) {
  switch (effect) {
    case 0x00:
      return "00xy: Arpeggio";
    case 0x01:
      return "01xx: Pitch slide up";
    case 0x02:
      return "02xx: Pitch slide down";
    case 0x03:
      return "03xx: Portamento";
    case 0x04:
      return "04xy: Vibrato (x: speed; y: depth)";
    case 0x05:
      return "05xy: Volume slide + vibrato (compatibility only!)";
    case 0x06:
      return "06xy: Volume slide + portamento (compatibility only!)";
    case 0x07:
      return "07xy: Tremolo (x: speed; y: depth)";
    case 0x08:
      return "08xy: Set panning (x: left; y: right)";
    case 0x09:
      return "09xx: Set groove pattern (speed 1 if no grooves exist)";
    case 0x0a:
      return "0Axy: Volume slide (0y: down; x0: up)";
    case 0x0b:
      return "0Bxx: Jump to pattern";
    case 0x0c:
      return "0Cxx: Retrigger";
    case 0x0d:
      return "0Dxx: Jump to next pattern";
    case 0x0f:
      return "0Fxx: Set speed (speed 2 if no grooves exist)";
    case 0x80:
      return "80xx: Set panning (00: left; 80: center; FF: right)";
    case 0x81:
      return "81xx: Set panning (left channel)";
    case 0x82:
      return "82xx: Set panning (right channel)";
    case 0x88:
      return "88xx: Set panning (rear channels; x: left; y: right)";
      break;
    case 0x89:
      return "89xx: Set panning (rear left channel)";
      break;
    case 0x8a:
      return "8Axx: Set panning (rear right channel)";
      break;
    case 0xc0: case 0xc1: case 0xc2: case 0xc3:
      return "Cxxx: Set tick rate (hz)";
    case 0xe0:
      return "E0xx: Set arp speed";
    case 0xe1:
      return "E1xy: Note slide up (x: speed; y: semitones)";
    case 0xe2:
      return "E2xy: Note slide down (x: speed; y: semitones)";
    case 0xe3:
      return "E3xx: Set vibrato shape (0: up/down; 1: up only; 2: down only)";
    case 0xe4:
      return "E4xx: Set vibrato range";
    case 0xe5:
      return "E5xx: Set pitch (80: center)";
    case 0xea:
      return "EAxx: Legato";
    case 0xeb:
      return "EBxx: Set LEGACY sample mode bank";
    case 0xec:
      return "ECxx: Note cut";
    case 0xed:
      return "EDxx: Note delay";
    case 0xee:
      return "EExx: Send external command";
    case 0xf0:
      return "F0xx: Set tick rate (bpm)";
    case 0xf1:
      return "F1xx: Single tick note slide up";
    case 0xf2:
      return "F2xx: Single tick note slide down";
    case 0xf3:
      return "F3xx: Fine volume slide up";
    case 0xf4:
      return "F4xx: Fine volume slide down";
    case 0xf5:
      return "F5xx: Disable macro (see manual)";
    case 0xf6:
      return "F6xx: Enable macro (see manual)";
    case 0xf8:
      return "F8xx: Single tick volume slide up";
    case 0xf9:
      return "F9xx: Single tick volume slide down";
    case 0xfa:
      return "FAxx: Fast volume slide (0y: down; x0: up)";
    case 0xff:
      return "FFxx: Stop song";
    default:
      if ((effect&0xf0)==0x90) {
        return "9xxx: Set sample offset*256";
      } else if (chan>=0 && chan<chans) {
        DivSysDef* sysDef=sysDefs[sysOfChan[chan]];
        auto iter=sysDef->effectHandlers.find(effect);
        if (iter!=sysDef->effectHandlers.end()) {
          return iter->second.description;
        }
        iter=sysDef->postEffectHandlers.find(effect);
        if (iter!=sysDef->postEffectHandlers.end()) {
          return iter->second.description;
        }
      }
      break;
  }
  return notNull?"Invalid effect":NULL;
}

void DivEngine::walkSong(int& loopOrder, int& loopRow, int& loopEnd) {
  loopOrder=0;
  loopRow=0;
  loopEnd=-1;
  int nextOrder=-1;
  int nextRow=0;
  int effectVal=0;
  int lastSuspectedLoopEnd=-1;
  DivPattern* pat[DIV_MAX_CHANS];
  unsigned char wsWalked[8192];
  memset(wsWalked,0,8192);
  for (int i=0; i<curSubSong->ordersLen; i++) {
    for (int j=0; j<chans; j++) {
      pat[j]=curPat[j].getPattern(curOrders->ord[j][i],false);
    }
    if (i>lastSuspectedLoopEnd) {
      lastSuspectedLoopEnd=i;
    }
    for (int j=nextRow; j<curSubSong->patLen; j++) {
      nextRow=0;
      bool changingOrder=false;
      bool jumpingOrder=false;
      if (wsWalked[((i<<5)+(j>>3))&8191]&(1<<(j&7))) {
        loopOrder=i;
        loopRow=j;
        loopEnd=lastSuspectedLoopEnd;
        return;
      }
      for (int k=0; k<chans; k++) {
        for (int l=0; l<curPat[k].effectCols; l++) {
          effectVal=pat[k]->data[j][5+(l<<1)];
          if (effectVal<0) effectVal=0;
          if (pat[k]->data[j][4+(l<<1)]==0x0d) {
            if (song.jumpTreatment==2) {
              if ((i<curSubSong->ordersLen-1 || !song.ignoreJumpAtEnd)) {
                nextOrder=i+1;
                nextRow=effectVal;
                jumpingOrder=true;
              }
            } else if (song.jumpTreatment==1) {
              if (nextOrder==-1 && (i<curSubSong->ordersLen-1 || !song.ignoreJumpAtEnd)) {
                nextOrder=i+1;
                nextRow=effectVal;
                jumpingOrder=true;
              }
            } else {
              if ((i<curSubSong->ordersLen-1 || !song.ignoreJumpAtEnd)) {
                if (!changingOrder) {
                  nextOrder=i+1;
                }
                jumpingOrder=true;
                nextRow=effectVal;
              }
            }
          } else if (pat[k]->data[j][4+(l<<1)]==0x0b) {
            if (nextOrder==-1 || song.jumpTreatment==0) {
              nextOrder=effectVal;
              if (song.jumpTreatment==1 || song.jumpTreatment==2 || !jumpingOrder) {
                nextRow=0;
              }
              changingOrder=true;
            }
          }
        }
      }

      wsWalked[((i<<5)+(j>>3))&8191]|=1<<(j&7);
      
      if (nextOrder!=-1) {
        i=nextOrder-1;
        nextOrder=-1;
        break;
      }
    }
  }
}

#define EXPORT_BUFSIZE 2048

double DivEngine::benchmarkPlayback() {
  float* outBuf[2];
  outBuf[0]=new float[EXPORT_BUFSIZE];
  outBuf[1]=new float[EXPORT_BUFSIZE];

  curOrder=0;
  prevOrder=0;
  remainingLoops=1;
  playSub(false);

  std::chrono::high_resolution_clock::time_point timeStart=std::chrono::high_resolution_clock::now();

  // benchmark
  while (playing) {
    nextBuf(NULL,outBuf,0,2,EXPORT_BUFSIZE);
  }

  std::chrono::high_resolution_clock::time_point timeEnd=std::chrono::high_resolution_clock::now();

  delete[] outBuf[0];
  delete[] outBuf[1];

  double t=(double)(std::chrono::duration_cast<std::chrono::microseconds>(timeEnd-timeStart).count())/1000000.0;
  printf("[RESULT] %fs\n",t);
  return t;
}

double DivEngine::benchmarkSeek() {
  double t[20];
  curOrder=curSubSong->ordersLen-1;
  prevOrder=curSubSong->ordersLen-1;

  // benchmark
  for (int i=0; i<20; i++) {
    std::chrono::high_resolution_clock::time_point timeStart=std::chrono::high_resolution_clock::now();
    playSub(false);     
    std::chrono::high_resolution_clock::time_point timeEnd=std::chrono::high_resolution_clock::now();
    t[i]=(double)(std::chrono::duration_cast<std::chrono::microseconds>(timeEnd-timeStart).count())/1000000.0;
    printf("[#%d] %fs\n",i+1,t[i]);
  }

  double tMin=DBL_MAX;
  double tMax=0.0;
  double tAvg=0.0;
  for (int i=0; i<20; i++) {
    if (t[i]<tMin) tMin=t[i];
    if (t[i]>tMax) tMax=t[i];
    tAvg+=t[i];
  }
  tAvg/=20.0;

  printf("[RESULT] min %fs max %fs average %fs\n",tMin,tMax,tAvg);
  return tAvg;
}

#define WRITE_TICK(x) \
  if (binary) { \
    if (!wroteTick[x]) { \
      wroteTick[x]=true; \
      if (tick-lastTick[x]>255) { \
        chanStream[x]->writeC(0xfc); \
        chanStream[x]->writeS(tick-lastTick[x]); \
      } else if (tick-lastTick[x]>1) { \
        delayPopularity[tick-lastTick[x]]++; \
        chanStream[x]->writeC(0xfd); \
        chanStream[x]->writeC(tick-lastTick[x]); \
      } else if (tick-lastTick[x]>0) { \
        chanStream[x]->writeC(0xfe); \
      } \
      lastTick[x]=tick; \
    } \
  } else { \
    if (!wroteTickGlobal) { \
      wroteTickGlobal=true; \
      w->writeText(fmt::sprintf(">> TICK %d\n",tick)); \
    } \
  }

void writePackedCommandValues(SafeWriter* w, const DivCommand& c) {
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
    case DIV_CMD_PANNING:
    case DIV_CMD_PRE_PORTA:
    case DIV_CMD_HINT_VIBRATO:
    case DIV_CMD_HINT_VIBRATO_RANGE:
    case DIV_CMD_HINT_VIBRATO_SHAPE:
    case DIV_CMD_HINT_PITCH:
    case DIV_CMD_HINT_ARPEGGIO:
    case DIV_CMD_HINT_VOLUME:
    case DIV_CMD_HINT_PORTA:
    case DIV_CMD_HINT_VOL_SLIDE:
    case DIV_CMD_HINT_LEGATO:
      w->writeC((unsigned char)c.cmd+0xb4);
      break;
    default:
      w->writeC(0xf0); // unoptimized extended command
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
      w->writeC(c.value);
      break;
    case DIV_CMD_PANNING:
    case DIV_CMD_HINT_VIBRATO:
    case DIV_CMD_HINT_ARPEGGIO:
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
    case DIV_CMD_HINT_ARP_TIME:
      w->writeC(1); // length
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
      w->writeC(2); // length
      w->writeC(c.value);
      w->writeC(c.value2);
      break;
    case DIV_CMD_C64_FINE_DUTY:
    case DIV_CMD_C64_FINE_CUTOFF:
    case DIV_CMD_LYNX_LFSR_LOAD:
      w->writeC(2); // length
      w->writeS(c.value);
      break;
    case DIV_CMD_FM_FIXFREQ:
      w->writeC(2); // length
      w->writeS((c.value<<12)|(c.value2&0x7ff));
      break;
    case DIV_CMD_NES_SWEEP:
      w->writeC(1); // length
      w->writeC((c.value?8:0)|(c.value2&0x77));
      break;
    default:
      logW("unimplemented command %s!",cmdName[c.cmd]);
      w->writeC(0); // length
      break;
  }
}

SafeWriter* DivEngine::saveCommand(bool binary) {
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
  bool wroteTick[DIV_MAX_CHANS];

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
  if (binary) {
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
  } else {
    w->writeText("# Furnace Command Stream\n\n");

    w->writeText("[Information]\n");
    w->writeText(fmt::sprintf("name: %s\n",song.name));
    w->writeText(fmt::sprintf("author: %s\n",song.author));
    w->writeText(fmt::sprintf("category: %s\n",song.category));
    w->writeText(fmt::sprintf("system: %s\n",song.systemName));

    w->writeText("\n");

    w->writeText("[SubSongInformation]\n");
    w->writeText(fmt::sprintf("name: %s\n",curSubSong->name));
    w->writeText(fmt::sprintf("tickRate: %f\n",curSubSong->hz));

    w->writeText("\n");

    w->writeText("[SysDefinition]\n");
    // TODO

    w->writeText("\n");
  }

  // play the song ourselves
  bool done=false;
  playSub(false);
  
  if (!binary) {
    w->writeText("[Stream]\n");
  }
  int tick=0;
  bool oldCmdStreamEnabled=cmdStreamEnabled;
  cmdStreamEnabled=true;
  double curDivider=divider;
  int lastTick[DIV_MAX_CHANS];

  memset(lastTick,0,DIV_MAX_CHANS*sizeof(int));
  while (!done) {
    if (nextTick(false,true) || !playing) {
      done=true;
    }
    // get command stream
    bool wroteTickGlobal=false;
    memset(wroteTick,0,DIV_MAX_CHANS*sizeof(bool));
    if (curDivider!=divider) {
      curDivider=divider;
      WRITE_TICK(0);
      if (binary) {
        chanStream[0]->writeC(0xfb);
        chanStream[0]->writeI((int)(curDivider*65536));
      } else {
        w->writeText(fmt::sprintf(">> SET_RATE %f\n",curDivider));
      }
    }
    for (DivCommand& i: cmdStream) {
      switch (i.cmd) {
        // strip away hinted/useless commands
        case DIV_ALWAYS_SET_VOLUME:
          break;
        case DIV_CMD_GET_VOLUME:
          break;
        case DIV_CMD_VOLUME:
          break;
        case DIV_CMD_NOTE_PORTA:
          break;
        case DIV_CMD_LEGATO:
          break;
        case DIV_CMD_PITCH:
          break;
        case DIV_CMD_PRE_NOTE:
          break;
        default:
          WRITE_TICK(i.chan);
          if (binary) {
            cmdPopularity[i.cmd]++;
            writePackedCommandValues(chanStream[i.chan],i);
          } else {
            w->writeText(fmt::sprintf("  %d: %s %d %d\n",i.chan,cmdName[i.cmd],i.value,i.value2));
          }
          break;
      }
    }
    cmdStream.clear();
    tick++;
  }
  cmdStreamEnabled=oldCmdStreamEnabled;

  if (binary) {
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

    sortCand=-1;
    sortPos=0;
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

    for (int i=0; i<chans; i++) {
      chanStream[i]->writeC(0xff);
      // optimize stream
      SafeWriter* oldStream=chanStream[i];
      SafeReader* reader=oldStream->toReader();
      chanStream[i]=new SafeWriter;
      chanStream[i]->init();

      while (1) {
        try {
          unsigned char next=reader->readC();
          switch (next) {
            case 0xb8: // instrument
            case 0xc0: // pre porta
            case 0xc3: // vibrato range
            case 0xc4: // vibrato shape
            case 0xc5: // pitch
            case 0xc7: // volume
            case 0xca: // legato
              chanStream[i]->writeC(next);
              next=reader->readC();
              chanStream[i]->writeC(next);
              break;
            case 0xbe: // panning
            case 0xc2: // vibrato
            case 0xc6: // arpeggio
            case 0xc8: // vol slide
            case 0xc9: // porta
              chanStream[i]->writeC(next);
              next=reader->readC();
              chanStream[i]->writeC(next);
              next=reader->readC();
              chanStream[i]->writeC(next);
              break;
            case 0xf0: { // full command (pre)
              unsigned char cmd=reader->readC();
              bool foundShort=false;
              for (int j=0; j<16; j++) {
                if (sortedCmd[j]==cmd) {
                  chanStream[i]->writeC(0xd0+j);
                  foundShort=true;
                  break;
                }
              }
              if (!foundShort) {
                chanStream[i]->writeC(0xf7); // full command
                chanStream[i]->writeC(cmd);
              }

              unsigned char cmdLen=reader->readC();
              logD("cmdLen: %d",cmdLen);
              for (unsigned char j=0; j<cmdLen; j++) {
                next=reader->readC();
                chanStream[i]->writeC(next);
              }
              break;
            }
            case 0xfb: // tick rate
              chanStream[i]->writeC(next);
              next=reader->readC();
              chanStream[i]->writeC(next);
              next=reader->readC();
              chanStream[i]->writeC(next);
              next=reader->readC();
              chanStream[i]->writeC(next);
              next=reader->readC();
              chanStream[i]->writeC(next);
              break;
            case 0xfc: { // 16-bit wait
              unsigned short delay=reader->readS();
              bool foundShort=false;
              for (int j=0; j<16; j++) {
                if (sortedDelay[j]==delay) {
                  chanStream[i]->writeC(0xe0+j);
                  foundShort=true;
                  break;
                }
              }
              if (!foundShort) {
                chanStream[i]->writeC(next);
                chanStream[i]->writeS(delay);
              }
              break;
            }
            case 0xfd: { // 8-bit wait
              unsigned char delay=reader->readC();
              bool foundShort=false;
              for (int j=0; j<16; j++) {
                if (sortedDelay[j]==delay) {
                  chanStream[i]->writeC(0xe0+j);
                  foundShort=true;
                  break;
                }
              }
              if (!foundShort) {
                chanStream[i]->writeC(next);
                chanStream[i]->writeC(delay);
              }
              break;
            }
            default:
              chanStream[i]->writeC(next);
              break;
          }
        } catch (EndOfFileException& e) {
          break;
        }
      }

      oldStream->finish();
      delete oldStream;
    }

    for (int i=0; i<chans; i++) {
      chanStreamOff[i]=w->tell();
      logI("- %d: off %x size %ld",i,chanStreamOff[i],chanStream[i]->size());
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
      if (sortedCmdPopularity[i]) logD("- %s: %d",cmdName[sortedCmd[i]],sortedCmdPopularity[i]);
    }
  } else {
    if (!playing) {
      w->writeText(">> END\n");
    } else {
      w->writeText(">> LOOP 0\n");
    }
  }

  remainingLoops=-1;
  playing=false;
  freelance=false;
  extValuePresent=false;
  BUSY_END;

  return w;
}

void _runExportThread(DivEngine* caller) {
  caller->runExportThread();
}

bool DivEngine::isExporting() {
  return exporting;
}

#ifdef HAVE_SNDFILE
void DivEngine::runExportThread() {
  size_t fadeOutSamples=got.rate*exportFadeOut;
  size_t curFadeOutSample=0;
  bool isFadingOut=false;
  switch (exportMode) {
    case DIV_EXPORT_MODE_ONE: {
      SNDFILE* sf;
      SF_INFO si;
      SFWrapper sfWrap;
      si.samplerate=got.rate;
      si.channels=2;
      si.format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;

      sf=sfWrap.doOpen(exportPath.c_str(),SFM_WRITE,&si);
      if (sf==NULL) {
        logE("could not open file for writing! (%s)",sf_strerror(NULL));
        exporting=false;
        return;
      }

      float* outBuf[3];
      outBuf[0]=new float[EXPORT_BUFSIZE];
      outBuf[1]=new float[EXPORT_BUFSIZE];
      outBuf[2]=new float[EXPORT_BUFSIZE*2];

      // take control of audio output
      deinitAudioBackend();
      playSub(false);

      logI("rendering to file...");

      while (playing) {
        size_t total=0;
        nextBuf(NULL,outBuf,0,2,EXPORT_BUFSIZE);
        if (totalProcessed>EXPORT_BUFSIZE) {
          logE("error: total processed is bigger than export bufsize! %d>%d",totalProcessed,EXPORT_BUFSIZE);
          totalProcessed=EXPORT_BUFSIZE;
        }
        for (int i=0; i<(int)totalProcessed; i++) {
          total++;
          if (isFadingOut) {
            double mul=(1.0-((double)curFadeOutSample/(double)fadeOutSamples));
            outBuf[2][i<<1]=MAX(-1.0f,MIN(1.0f,outBuf[0][i]))*mul;
            outBuf[2][1+(i<<1)]=MAX(-1.0f,MIN(1.0f,outBuf[1][i]))*mul;
            if (++curFadeOutSample>=fadeOutSamples) {
              playing=false;
              break;
            }
          } else {
            outBuf[2][i<<1]=MAX(-1.0f,MIN(1.0f,outBuf[0][i]));
            outBuf[2][1+(i<<1)]=MAX(-1.0f,MIN(1.0f,outBuf[1][i]));
            if (lastLoopPos>-1 && i>=lastLoopPos && totalLoops>=exportLoopCount) {
              logD("start fading out...");
              isFadingOut=true;
            }
          }
        }
        
        if (sf_writef_float(sf,outBuf[2],total)!=(int)total) {
          logE("error: failed to write entire buffer!");
          break;
        }
      }

      delete[] outBuf[0];
      delete[] outBuf[1];
      delete[] outBuf[2];

      if (sfWrap.doClose()!=0) {
        logE("could not close audio file!");
      }
      exporting=false;

      if (initAudioBackend()) {
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].setRates(got.rate);
          disCont[i].setQuality(lowQuality);
        }
        if (!output->setRun(true)) {
          logE("error while activating audio!");
        }
      }
      logI("done!");
      break;
    }
    case DIV_EXPORT_MODE_MANY_SYS: {
      SNDFILE* sf[DIV_MAX_CHIPS];
      SF_INFO si[DIV_MAX_CHIPS];
      String fname[DIV_MAX_CHIPS];
      SFWrapper sfWrap[DIV_MAX_CHIPS];
      for (int i=0; i<song.systemLen; i++) {
        sf[i]=NULL;
        si[i].samplerate=got.rate;
        si[i].channels=disCont[i].dispatch->getOutputCount();
        si[i].format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;
      }

      for (int i=0; i<song.systemLen; i++) {
        fname[i]=fmt::sprintf("%s_s%02d.wav",exportPath,i+1);
        logI("- %s",fname[i].c_str());
        sf[i]=sfWrap[i].doOpen(fname[i].c_str(),SFM_WRITE,&si[i]);
        if (sf[i]==NULL) {
          logE("could not open file for writing! (%s)",sf_strerror(NULL));
          for (int j=0; j<i; j++) {
            sfWrap[i].doClose();
          }
          return;
        }
      }

      float* outBuf[DIV_MAX_OUTPUTS];
      memset(outBuf,0,sizeof(void*)*DIV_MAX_OUTPUTS);
      outBuf[0]=new float[EXPORT_BUFSIZE];
      outBuf[1]=new float[EXPORT_BUFSIZE];
      short* sysBuf[DIV_MAX_CHIPS];
      for (int i=0; i<song.systemLen; i++) {
        sysBuf[i]=new short[EXPORT_BUFSIZE*disCont[i].dispatch->getOutputCount()];
      }

      // take control of audio output
      deinitAudioBackend();
      playSub(false);

      logI("rendering to files...");

      while (playing) {
        size_t total=0;
        nextBuf(NULL,outBuf,0,2,EXPORT_BUFSIZE);
        if (totalProcessed>EXPORT_BUFSIZE) {
          logE("error: total processed is bigger than export bufsize! %d>%d",totalProcessed,EXPORT_BUFSIZE);
          totalProcessed=EXPORT_BUFSIZE;
        }
        for (int j=0; j<(int)totalProcessed; j++) {
          total++;
          if (isFadingOut) {
            double mul=(1.0-((double)curFadeOutSample/(double)fadeOutSamples));
            for (int i=0; i<song.systemLen; i++) {
              for (int k=0; k<si[i].channels; k++) {
                if (disCont[i].bbOut[k]==NULL) {
                  sysBuf[i][k+(j*si[i].channels)]=0;
                } else {
                  sysBuf[i][k+(j*si[i].channels)]=(double)disCont[i].bbOut[k][j]*mul;
                }
              }
            }
            if (++curFadeOutSample>=fadeOutSamples) {
              playing=false;
              break;
            }
          } else {
            for (int i=0; i<song.systemLen; i++) {
              for (int k=0; k<si[i].channels; k++) {
                if (disCont[i].bbOut[k]==NULL) {
                  sysBuf[i][k+(j*si[i].channels)]=0;
                } else {
                  sysBuf[i][k+(j*si[i].channels)]=disCont[i].bbOut[k][j];
                }
              }
            }
            if (lastLoopPos>-1 && j>=lastLoopPos && totalLoops>=exportLoopCount) {
              logD("start fading out...");
              isFadingOut=true;
            }
          }
        }
        for (int i=0; i<song.systemLen; i++) {
          if (sf_writef_short(sf[i],sysBuf[i],total)!=(int)total) {
            logE("error: failed to write entire buffer! (%d)",i);
            break;
          }
        }
      }

      delete[] outBuf[0];
      delete[] outBuf[1];

      for (int i=0; i<song.systemLen; i++) {
        delete[] sysBuf[i];
        if (sfWrap[i].doClose()!=0) {
          logE("could not close audio file!");
        }
      }
      exporting=false;

      if (initAudioBackend()) {
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].setRates(got.rate);
          disCont[i].setQuality(lowQuality);
        }
        if (!output->setRun(true)) {
          logE("error while activating audio!");
        }
      }
      logI("done!");
      break;
    }
    case DIV_EXPORT_MODE_MANY_CHAN: {
      // take control of audio output
      deinitAudioBackend();

      float* outBuf[3];
      outBuf[0]=new float[EXPORT_BUFSIZE];
      outBuf[1]=new float[EXPORT_BUFSIZE];
      outBuf[2]=new float[EXPORT_BUFSIZE*2];
      int loopCount=remainingLoops;

      logI("rendering to files...");
      
      for (int i=0; i<chans; i++) {
        SNDFILE* sf;
        SF_INFO si;
        SFWrapper sfWrap;
        String fname=fmt::sprintf("%s_c%02d.wav",exportPath,i+1);
        logI("- %s",fname.c_str());
        si.samplerate=got.rate;
        si.channels=2;
        si.format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;

        sf=sfWrap.doOpen(fname.c_str(),SFM_WRITE,&si);
        if (sf==NULL) {
          logE("could not open file for writing! (%s)",sf_strerror(NULL));
          break;
        }

        for (int j=0; j<chans; j++) {
          bool mute=(j!=i);
          isMuted[j]=mute;
        }
        if (getChannelType(i)==5) {
          for (int j=i; j<chans; j++) {
            if (getChannelType(j)!=5) break;
            isMuted[j]=false;
          }
        }
        for (int j=0; j<chans; j++) {
          if (disCont[dispatchOfChan[j]].dispatch!=NULL) {
            disCont[dispatchOfChan[j]].dispatch->muteChannel(dispatchChanOfChan[j],isMuted[j]);
          }
        }
        
        curOrder=0;
        prevOrder=0;
        curFadeOutSample=0;
        lastLoopPos=-1;
        totalLoops=0;
        isFadingOut=false;
        if (exportFadeOut<=0.01) {
          remainingLoops=loopCount;
        } else {
          remainingLoops=-1;
        }
        playSub(false);

        while (playing) {
          size_t total=0;
          nextBuf(NULL,outBuf,0,2,EXPORT_BUFSIZE);
          if (totalProcessed>EXPORT_BUFSIZE) {
            logE("error: total processed is bigger than export bufsize! %d>%d",totalProcessed,EXPORT_BUFSIZE);
            totalProcessed=EXPORT_BUFSIZE;
          }
          for (int j=0; j<(int)totalProcessed; j++) {
            total++;
            if (isFadingOut) {
              double mul=(1.0-((double)curFadeOutSample/(double)fadeOutSamples));
              outBuf[2][j<<1]=MAX(-1.0f,MIN(1.0f,outBuf[0][j]))*mul;
              outBuf[2][1+(j<<1)]=MAX(-1.0f,MIN(1.0f,outBuf[1][j]))*mul;
              if (++curFadeOutSample>=fadeOutSamples) {
                playing=false;
                break;
              }
            } else {
              outBuf[2][j<<1]=MAX(-1.0f,MIN(1.0f,outBuf[0][j]));
              outBuf[2][1+(j<<1)]=MAX(-1.0f,MIN(1.0f,outBuf[1][j]));
              if (lastLoopPos>-1 && j>=lastLoopPos && totalLoops>=exportLoopCount) {
                logD("start fading out...");
                isFadingOut=true;
              }
            }
          }
          if (sf_writef_float(sf,outBuf[2],total)!=(int)total) {
            logE("error: failed to write entire buffer!");
            break;
          }
        }

        if (sfWrap.doClose()!=0) {
          logE("could not close audio file!");
        }

        if (getChannelType(i)==5) {
          i++;
          while (true) {
            if (i>=chans) break;
            if (getChannelType(i)!=5) break;
            i++;
          }
          i--;
        }

        if (stopExport) break;
      }
      exporting=false;

      delete[] outBuf[0];
      delete[] outBuf[1];
      delete[] outBuf[2];

      for (int i=0; i<chans; i++) {
        isMuted[i]=false;
        if (disCont[dispatchOfChan[i]].dispatch!=NULL) {
          disCont[dispatchOfChan[i]].dispatch->muteChannel(dispatchChanOfChan[i],false);
        }
      }

      if (initAudioBackend()) {
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].setRates(got.rate);
          disCont[i].setQuality(lowQuality);
        }
        if (!output->setRun(true)) {
          logE("error while activating audio!");
        }
      }
      logI("done!");
      break;
    }
  }
  stopExport=false;
}
#else
void DivEngine::runExportThread() {
}
#endif

bool DivEngine::saveAudio(const char* path, int loops, DivAudioExportModes mode, double fadeOutTime) {
#ifndef HAVE_SNDFILE
  logE("Furnace was not compiled with libsndfile. cannot export!");
  return false;
#else
  exportPath=path;
  exportMode=mode;
  exportFadeOut=fadeOutTime;
  if (exportMode!=DIV_EXPORT_MODE_ONE) {
    // remove extension
    String lowerCase=exportPath;
    for (char& i: lowerCase) {
      if (i>='A' && i<='Z') i+='a'-'A';
    }
    size_t extPos=lowerCase.rfind(".wav");
    if (extPos!=String::npos) {
      exportPath=exportPath.substr(0,extPos);
    }
  }
  exporting=true;
  stopExport=false;
  stop();
  repeatPattern=false;
  setOrder(0);
  if (exportFadeOut<=0.01) {
    remainingLoops=loops;
  } else {
    remainingLoops=-1;
  }
  exportLoopCount=loops;
  exportThread=new std::thread(_runExportThread,this);
  return true;
#endif
}

void DivEngine::waitAudioFile() {
  if (exportThread!=NULL) {
    exportThread->join();
  }
}

bool DivEngine::haltAudioFile() {
  stopExport=true;
  stop();
  return true;
}

void DivEngine::notifyInsChange(int ins) {
  BUSY_BEGIN;
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].dispatch->notifyInsChange(ins);
  }
  BUSY_END;
}

void DivEngine::notifyWaveChange(int wave) {
  BUSY_BEGIN;
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].dispatch->notifyWaveChange(wave);
  }
  BUSY_END;
}

int DivEngine::loadSampleROM(String path, ssize_t expectedSize, unsigned char*& ret) {
  ret = NULL;
  if (path.empty()) {
    return 0;
  }
  logI("loading ROM %s...",path);
  FILE* f=ps_fopen(path.c_str(),"rb");
  if (f==NULL) {
    logE("error: %s",strerror(errno));
    lastError=strerror(errno);
    return -1;
  }
  if (fseek(f,0,SEEK_END)<0) {
    logE("size error: %s",strerror(errno));
    lastError=fmt::sprintf("on seek: %s",strerror(errno));
    fclose(f);
    return -1;
  }
  ssize_t len=ftell(f);
  if (len==(SIZE_MAX>>1)) {
    logE("could not get file length: %s",strerror(errno));
    lastError=fmt::sprintf("on pre tell: %s",strerror(errno));
    fclose(f);
    return -1;
  }
  if (len<1) {
    if (len==0) {
      logE("that file is empty!");
      lastError="file is empty";
    } else {
      logE("tell error: %s",strerror(errno));
      lastError=fmt::sprintf("on tell: %s",strerror(errno));
    }
    fclose(f);
    return -1;
  }
  if (len!=expectedSize) {
    logE("ROM size mismatch, expected: %d bytes, was: %d bytes", expectedSize, len);
    lastError=fmt::sprintf("ROM size mismatch, expected: %d bytes, was: %d", expectedSize, len);
    return -1;
  }
  if (fseek(f,0,SEEK_SET)<0) {
    logE("size error: %s",strerror(errno));
    lastError=fmt::sprintf("on get size: %s",strerror(errno));
    fclose(f);
    return -1;
  }
  unsigned char* file=new unsigned char[len];
  if (fread(file,1,(size_t)len,f)!=(size_t)len) {
    logE("read error: %s",strerror(errno));
    lastError=fmt::sprintf("on read: %s",strerror(errno));
    fclose(f);
    delete[] file;
    return -1;
  }
  fclose(f);
  ret = file;
  return 0;
}

unsigned int DivEngine::getSampleFormatMask() {
  unsigned int formatMask=1U<<16; // 16-bit is always on
  for (int i=0; i<song.systemLen; i++) {
    const DivSysDef* s=getSystemDef(song.system[i]);
    if (s==NULL) continue;
    formatMask|=s->sampleFormatMask;
  }
  return formatMask;
}

int DivEngine::loadSampleROMs() {
  if (yrw801ROM!=NULL) {
    delete[] yrw801ROM;
    yrw801ROM=NULL;
  }
  if (tg100ROM!=NULL) {
    delete[] tg100ROM;
    tg100ROM=NULL;
  }
  if (mu5ROM!=NULL) {
    delete[] mu5ROM;
    mu5ROM=NULL;
  }
  int error = 0;
  error += loadSampleROM(getConfString("yrw801Path",""), 0x200000, yrw801ROM);
  error += loadSampleROM(getConfString("tg100Path",""), 0x200000, tg100ROM);
  error += loadSampleROM(getConfString("mu5Path",""), 0x200000, mu5ROM);
  return error;
}

void DivEngine::renderSamplesP() {
  BUSY_BEGIN;
  renderSamples();
  BUSY_END;
}

void DivEngine::renderSamples() {
  sPreview.sample=-1;
  sPreview.pos=0;
  sPreview.dir=false;

  logD("rendering samples...");

  // step 0: make sample format mask
  unsigned int formatMask=1U<<16; // 16-bit is always on
  for (int i=0; i<song.systemLen; i++) {
    const DivSysDef* s=getSystemDef(song.system[i]);
    if (s==NULL) continue;
    formatMask|=s->sampleFormatMask;
  }

  // step 1: render samples
  for (int i=0; i<song.sampleLen; i++) {
    song.sample[i]->render(formatMask);
  }

  // step 2: render samples to dispatch
  for (int i=0; i<song.systemLen; i++) {
    if (disCont[i].dispatch!=NULL) {
      disCont[i].dispatch->renderSamples(i);
    }
  }
}

String DivEngine::decodeSysDesc(String desc) {
  DivConfig newDesc;
  bool hasVal=false;
  bool negative=false;
  int val=0;
  int curStage=0;
  int sysID=0;
  float sysVol=0;
  float sysPan=0;
  int sysFlags=0;
  int curSys=0;
  desc+=' '; // ha
  for (char i: desc) {
    switch (i) {
      case ' ':
        if (hasVal) {
          if (negative) val=-val;
          switch (curStage) {
            case 0:
              sysID=val;
              curStage++;
              break;
            case 1:
              sysVol=(float)val/64.0f;
              curStage++;
              break;
            case 2:
              sysPan=(float)val/127.0f;
              curStage++;
              break;
            case 3:
              sysFlags=val;

              if (sysID!=0) {
                if (sysVol<-1.0f) sysVol=-1.0f;
                if (sysVol>1.0f) sysVol=1.0f;
                if (sysPan<-1.0f) sysPan=-1.0f;
                if (sysPan>1.0f) sysPan=1.0f;
                newDesc.set(fmt::sprintf("id%d",curSys),sysID);
                newDesc.set(fmt::sprintf("vol%d",curSys),sysVol);
                newDesc.set(fmt::sprintf("pan%d",curSys),sysPan);
                newDesc.set(fmt::sprintf("fr%d",curSys),0.0f);
                DivConfig newFlagsC;
                newFlagsC.clear();
                convertOldFlags((unsigned int)sysFlags,newFlagsC,systemFromFileFur(sysID));
                newDesc.set(fmt::sprintf("flags%d",curSys),newFlagsC.toBase64());
                curSys++;
              }

              curStage=0;
              break;
          }
          hasVal=false;
          negative=false;
          val=0;
        }
        break;
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        val=(val*10)+(i-'0');
        hasVal=true;
        break;
      case '-':
        if (!hasVal) negative=true;
        break;
    }
  }

  return newDesc.toBase64();
}

void DivEngine::initSongWithDesc(const char* description, bool inBase64, bool oldVol) {
  int chanCount=0;
  DivConfig c;
  if (inBase64) {
    c.loadFromBase64(description);
  } else {
    c.loadFromMemory(description);
  }
  int index=0;
  for (; index<DIV_MAX_CHIPS; index++) {
    song.system[index]=systemFromFileFur(c.getInt(fmt::sprintf("id%d",index),0));
    if (song.system[index]==DIV_SYSTEM_NULL) {
      break;
    }
    chanCount+=getChannelCount(song.system[index]);
    if (chanCount>=DIV_MAX_CHANS) {
      song.system[index]=DIV_SYSTEM_NULL;
      break;
    }
    song.systemVol[index]=c.getFloat(fmt::sprintf("vol%d",index),1.0f);
    song.systemPan[index]=c.getFloat(fmt::sprintf("pan%d",index),0.0f);
    song.systemPanFR[index]=c.getFloat(fmt::sprintf("fr%d",index),0.0f);
    song.systemFlags[index].clear();

    if (oldVol) {
      song.systemVol[index]/=64.0f;
      song.systemPan[index]/=127.0f;
    }

    String flags=c.getString(fmt::sprintf("flags%d",index),"");
    song.systemFlags[index].loadFromBase64(flags.c_str());
  }
  song.systemLen=index;
  
  // extra attributes
  song.subsong[0]->hz=c.getDouble("tickRate",60.0);
}

void DivEngine::createNew(const char* description, String sysName, bool inBase64) {
  quitDispatch();
  BUSY_BEGIN;
  saveLock.lock();
  song.unload();
  song=DivSong();
  changeSong(0);
  if (description!=NULL) {
    initSongWithDesc(description,inBase64);
  }
  if (sysName=="") {
    song.systemName=getSongSystemLegacyName(song,!getConfInt("noMultiSystem",0));
  } else {
    song.systemName=sysName;
  }
  recalcChans();
  saveLock.unlock();
  BUSY_END;
  initDispatch();
  BUSY_BEGIN;
  renderSamples();
  reset();
  BUSY_END;
}

void DivEngine::createNewFromDefaults() {
  quitDispatch();
  BUSY_BEGIN;
  saveLock.lock();
  song.unload();
  song=DivSong();
  changeSong(0);

  String preset=getConfString("initialSys2","");
  bool oldVol=getConfInt("configVersion",DIV_ENGINE_VERSION)<135;
  if (preset.empty()) {
    // try loading old preset
    logD("trying to load old preset");
    preset=decodeSysDesc(getConfString("initialSys",""));
    oldVol=false;
  }
  logD("preset size %ld",preset.size());
  if (preset.size()>0 && (preset.size()&3)==0) {
    initSongWithDesc(preset.c_str(),true,oldVol);
  }
  String sysName=getConfString("initialSysName","");
  if (sysName=="") {
    song.systemName=getSongSystemLegacyName(song,!getConfInt("noMultiSystem",0));
  } else {
    song.systemName=sysName;
  }

  recalcChans();
  saveLock.unlock();
  BUSY_END;
  initDispatch();
  BUSY_BEGIN;
  renderSamples();
  reset();
  BUSY_END;
}

void DivEngine::swapChannels(int src, int dest) {
  logV("swapping channel %d with %d",src,dest);
  if (src==dest) {
    logV("not swapping channels because it's the same channel!",src,dest);
    return;
  }

  for (int i=0; i<DIV_MAX_PATTERNS; i++) {
    curOrders->ord[dest][i]^=curOrders->ord[src][i];
    curOrders->ord[src][i]^=curOrders->ord[dest][i];
    curOrders->ord[dest][i]^=curOrders->ord[src][i];

    DivPattern* prev=curPat[src].data[i];
    curPat[src].data[i]=curPat[dest].data[i];
    curPat[dest].data[i]=prev;
  }

  curPat[src].effectCols^=curPat[dest].effectCols;
  curPat[dest].effectCols^=curPat[src].effectCols;
  curPat[src].effectCols^=curPat[dest].effectCols;

  String prevChanName=curSubSong->chanName[src];
  String prevChanShortName=curSubSong->chanShortName[src];
  bool prevChanShow=curSubSong->chanShow[src];
  unsigned char prevChanCollapse=curSubSong->chanCollapse[src];

  curSubSong->chanName[src]=curSubSong->chanName[dest];
  curSubSong->chanShortName[src]=curSubSong->chanShortName[dest];
  curSubSong->chanShow[src]=curSubSong->chanShow[dest];
  curSubSong->chanCollapse[src]=curSubSong->chanCollapse[dest];
  curSubSong->chanName[dest]=prevChanName;
  curSubSong->chanShortName[dest]=prevChanShortName;
  curSubSong->chanShow[dest]=prevChanShow;
  curSubSong->chanCollapse[dest]=prevChanCollapse;
}

void DivEngine::stompChannel(int ch) {
  logV("stomping channel %d",ch);
  for (int i=0; i<DIV_MAX_PATTERNS; i++) {
    curOrders->ord[ch][i]=0;
  }
  curPat[ch].wipePatterns();
  curPat[ch].effectCols=1;
  curSubSong->chanName[ch]="";
  curSubSong->chanShortName[ch]="";
  curSubSong->chanShow[ch]=true;
  curSubSong->chanCollapse[ch]=false;
}

void DivEngine::changeSong(size_t songIndex) {
  if (songIndex>=song.subsong.size()) return;
  curSubSong=song.subsong[songIndex];
  curPat=song.subsong[songIndex]->pat;
  curOrders=&song.subsong[songIndex]->orders;
  curSubSongIndex=songIndex;
  curOrder=0;
  curRow=0;
  prevOrder=0;
  prevRow=0;
}

void DivEngine::moveAsset(std::vector<DivAssetDir>& dir, int before, int after) {
  if (before<0 || after<0) return;
  for (DivAssetDir& i: dir) {
    for (size_t j=0; j<i.entries.size(); j++) {
      // erase matching entry
      if (i.entries[j]==before) {
        i.entries[j]=after;
      } else if (i.entries[j]==after) {
        i.entries[j]=before;
      }
    }
  }
}

void DivEngine::removeAsset(std::vector<DivAssetDir>& dir, int entry) {
  if (entry<0) return;
  for (DivAssetDir& i: dir) {
    for (size_t j=0; j<i.entries.size(); j++) {
      // erase matching entry
      if (i.entries[j]==entry) {
        i.entries.erase(i.entries.begin()+j);
        j--;
      } else if (i.entries[j]>entry) {
        i.entries[j]--;
      }
    }
  }
}

void DivEngine::checkAssetDir(std::vector<DivAssetDir>& dir, size_t entries) {
  bool* inAssetDir=new bool[entries];
  memset(inAssetDir,0,entries*sizeof(bool));

  for (DivAssetDir& i: dir) {
    for (size_t j=0; j<i.entries.size(); j++) {
      // erase invalid entry
      if (i.entries[j]<0 || i.entries[j]>=(int)entries) {
        i.entries.erase(i.entries.begin()+j);
        j--;
        continue;
      }

      // erase duplicate entry
      if (inAssetDir[i.entries[j]]) {
        i.entries.erase(i.entries.begin()+j);
        j--;
        continue;
      }
      
      // mark entry as present
      inAssetDir[i.entries[j]]=true;
    }
  }

  // get unsorted directory
  DivAssetDir* unsortedDir=NULL;
  for (DivAssetDir& i: dir) {
    if (i.name.empty()) {
      unsortedDir=&i;
      break;
    }
  }

  // add missing items to unsorted directory
  for (size_t i=0; i<entries; i++) {
    if (!inAssetDir[i]) {
      // create unsorted directory if it doesn't exist
      if (unsortedDir==NULL) {
        dir.push_back(DivAssetDir(""));
        unsortedDir=&(*dir.rbegin());
      }
      unsortedDir->entries.push_back(i);
    }
  }

  delete[] inAssetDir;
}

void DivEngine::swapChannelsP(int src, int dest) {
  if (src<0 || src>=chans) return;
  if (dest<0 || dest>=chans) return;
  BUSY_BEGIN;
  saveLock.lock();
  swapChannels(src,dest);
  saveLock.unlock();
  BUSY_END;
}

void DivEngine::changeSongP(size_t index) {
  if (index>=song.subsong.size()) return;
  if (index==curSubSongIndex) return;
  stop();
  BUSY_BEGIN;
  saveLock.lock();
  changeSong(index);
  saveLock.unlock();
  BUSY_END;
}

int DivEngine::addSubSong() {
  if (song.subsong.size()>=127) return -1;
  BUSY_BEGIN;
  saveLock.lock();
  song.subsong.push_back(new DivSubSong);
  saveLock.unlock();
  BUSY_END;
  return song.subsong.size()-1;
}

int DivEngine::duplicateSubSong(int index) {
  if (song.subsong.size()>=127) return -1;
  BUSY_BEGIN;
  saveLock.lock();
  DivSubSong* theCopy=new DivSubSong;
  DivSubSong* theOrig=song.subsong[index];

  theCopy->name=theOrig->name;
  theCopy->notes=theOrig->notes;
  theCopy->hilightA=theOrig->hilightA;
  theCopy->hilightB=theOrig->hilightB;
  theCopy->timeBase=theOrig->timeBase;
  theCopy->arpLen=theOrig->arpLen;
  theCopy->speeds=theOrig->speeds;
  theCopy->virtualTempoN=theOrig->virtualTempoN;
  theCopy->virtualTempoD=theOrig->virtualTempoD;
  theCopy->hz=theOrig->hz;
  theCopy->patLen=theOrig->patLen;
  theCopy->ordersLen=theOrig->ordersLen;
  theCopy->orders=theOrig->orders;
  
  memcpy(theCopy->chanShow,theOrig->chanShow,DIV_MAX_CHANS*sizeof(bool));
  memcpy(theCopy->chanCollapse,theOrig->chanCollapse,DIV_MAX_CHANS);

  for (int i=0; i<DIV_MAX_CHANS; i++) {
    theCopy->chanName[i]=theOrig->chanName[i];
    theCopy->chanShortName[i]=theOrig->chanShortName[i];

    theCopy->pat[i].effectCols=theOrig->pat[i].effectCols;

    for (int j=0; j<DIV_MAX_PATTERNS; j++) {
      if (theOrig->pat[i].data[j]==NULL) continue;
      DivPattern* origPat=theOrig->pat[i].getPattern(j,false);
      DivPattern* copyPat=theCopy->pat[i].getPattern(j,true);
      origPat->copyOn(copyPat);
    }
  }

  song.subsong.push_back(theCopy);
  
  saveLock.unlock();
  BUSY_END;
  return song.subsong.size()-1;
}

bool DivEngine::removeSubSong(int index) {
  if (song.subsong.size()<=1) return false;
  stop();
  BUSY_BEGIN;
  saveLock.lock();
  song.subsong[index]->clearData();
  delete song.subsong[index];
  song.subsong.erase(song.subsong.begin()+index);
  changeSong(0);
  saveLock.unlock();
  BUSY_END;
  return true;
}

void DivEngine::moveSubSongUp(size_t index) {
  if (index<1 || index>=song.subsong.size()) return;
  BUSY_BEGIN;
  saveLock.lock();

  if (index==curSubSongIndex) {
    curSubSongIndex--;
  } else if (index-1==curSubSongIndex) {
    curSubSongIndex++;
  }

  DivSubSong* prev=song.subsong[index-1];
  song.subsong[index-1]=song.subsong[index];
  song.subsong[index]=prev;

  saveLock.unlock();
  BUSY_END;
}

void DivEngine::moveSubSongDown(size_t index) {
  if (index>=song.subsong.size()-1) return;
  BUSY_BEGIN;
  saveLock.lock();

  if (index==curSubSongIndex) {
    curSubSongIndex++;
  } else if (index+1==curSubSongIndex) {
    curSubSongIndex--;
  }

  DivSubSong* prev=song.subsong[index+1];
  song.subsong[index+1]=song.subsong[index];
  song.subsong[index]=prev;

  saveLock.unlock();
  BUSY_END;
}

void DivEngine::clearSubSongs() {
  BUSY_BEGIN;
  saveLock.lock();
  song.clearSongData();
  changeSong(0);
  curOrder=0;
  prevOrder=0;
  saveLock.unlock();
  BUSY_END;
}

void DivEngine::changeSystem(int index, DivSystem which, bool preserveOrder) {
  int chanCount=chans;
  quitDispatch();
  BUSY_BEGIN;
  saveLock.lock();

  if (!preserveOrder) {
    int firstChan=0;
    int chanMovement=getChannelCount(which)-getChannelCount(song.system[index]);
    while (dispatchOfChan[firstChan]!=index) firstChan++;
    int lastChan=firstChan+getChannelCount(song.system[index]);
    if (chanMovement!=0) {
      if (chanMovement>0) {
        // add channels
        for (int i=chanCount+chanMovement-1; i>=lastChan+chanMovement; i--) {
          swapChannels(i,i-chanMovement);
        }
        for (int i=lastChan; i<lastChan+chanMovement; i++) {
          stompChannel(i);
        }
      } else {
        // remove channels
        for (int i=lastChan+chanMovement; i<lastChan; i++) {
          stompChannel(i);
        }
        for (int i=lastChan+chanMovement; i<chanCount+chanMovement; i++) {
          swapChannels(i,i-chanMovement);
        }
      }
    }
  }

  song.system[index]=which;
  song.systemFlags[index].clear();
  recalcChans();
  saveLock.unlock();
  BUSY_END;
  initDispatch();
  BUSY_BEGIN;
  renderSamples();
  reset();
  BUSY_END;
}

bool DivEngine::addSystem(DivSystem which) {
  if (song.systemLen>DIV_MAX_CHIPS) {
    lastError=fmt::sprintf("max number of systems is %d",DIV_MAX_CHIPS);
    return false;
  }
  if (chans+getChannelCount(which)>DIV_MAX_CHANS) {
    lastError=fmt::sprintf("max number of total channels is %d",DIV_MAX_CHANS);
    return false;
  }
  quitDispatch();
  BUSY_BEGIN;
  saveLock.lock();
  song.system[song.systemLen]=which;
  song.systemVol[song.systemLen]=1.0;
  song.systemPan[song.systemLen]=0;
  song.systemPanFR[song.systemLen]=0;
  song.systemFlags[song.systemLen++].clear();
  recalcChans();
  saveLock.unlock();
  BUSY_END;
  initDispatch();
  BUSY_BEGIN;
  saveLock.lock();
  if (song.patchbayAuto) {
    autoPatchbay();
  } else {
    int i=song.systemLen-1;
    if (disCont[i].dispatch!=NULL) {
      unsigned int outs=disCont[i].dispatch->getOutputCount();
      if (outs>16) outs=16;
      if (outs<2) {
        for (unsigned int j=0; j<DIV_MAX_OUTPUTS; j++) {
          song.patchbay.push_back((i<<20)|j);
        }
      } else {
        for (unsigned int j=0; j<outs; j++) {

          song.patchbay.push_back((i<<20)|(j<<16)|j);
        }
      }
    }
  }
  saveLock.unlock();
  renderSamples();
  reset();
  BUSY_END;
  return true;
}

// TODO: maybe issue with subsongs?
bool DivEngine::removeSystem(int index, bool preserveOrder) {
  if (song.systemLen<=1) {
    lastError="cannot remove the last one";
    return false;
  }
  if (index<0 || index>=song.systemLen) {
    lastError="invalid index";
    return false;
  }
  int chanCount=chans;
  quitDispatch();
  BUSY_BEGIN;
  saveLock.lock();

  if (!preserveOrder) {
    int firstChan=0;
    while (dispatchOfChan[firstChan]!=index) firstChan++;
    for (int i=0; i<getChannelCount(song.system[index]); i++) {
      stompChannel(i+firstChan);
    }
    for (int i=firstChan+getChannelCount(song.system[index]); i<chanCount; i++) {
      swapChannels(i,i-getChannelCount(song.system[index]));
    }
  }

  // patchbay
  for (size_t i=0; i<song.patchbay.size(); i++) {
    if (((song.patchbay[i]>>20)&0xfff)==(unsigned int)index) {
      song.patchbay.erase(song.patchbay.begin()+i);
      i--;
    }
  }

  song.system[index]=DIV_SYSTEM_NULL;
  song.systemLen--;
  for (int i=index; i<song.systemLen; i++) {
    song.system[i]=song.system[i+1];
    song.systemVol[i]=song.systemVol[i+1];
    song.systemPan[i]=song.systemPan[i+1];
    song.systemPanFR[i]=song.systemPanFR[i+1];
    song.systemFlags[i]=song.systemFlags[i+1];
  }
  recalcChans();
  saveLock.unlock();
  BUSY_END;
  initDispatch();
  BUSY_BEGIN;
  renderSamples();
  reset();
  BUSY_END;
  return true;
}

bool DivEngine::swapSystem(int src, int dest, bool preserveOrder) {
  if (src==dest) {
    lastError="source and destination are equal";
    return false;
  }
  if (src<0 || src>=song.systemLen) {
    lastError="invalid source index";
    return false;
  }
  if (dest<0 || dest>=song.systemLen) {
    lastError="invalid destination index";
    return false;
  }
  //int chanCount=chans;
  quitDispatch();
  BUSY_BEGIN;
  saveLock.lock();

  if (!preserveOrder) {
    // move channels
    unsigned char unswappedChannels[DIV_MAX_CHANS];
    unsigned char swappedChannels[DIV_MAX_CHANS];
    std::vector<std::vector<int>> swapList;
    std::vector<int> chanList;

    int tchans=0;

    for (int i=0; i<song.systemLen; i++) {
      tchans+=getChannelCount(song.system[i]);
    }

    memset(unswappedChannels,0,DIV_MAX_CHANS);
    memset(swappedChannels,0,DIV_MAX_CHANS);
    
    for (int i=0; i<tchans; i++) {
      unswappedChannels[i]=i;
    }

    // prepare swap list
    int index=0;
    for (int i=0; i<song.systemLen; i++) {
      chanList.clear();
      for (int j=0; j<getChannelCount(song.system[i]); j++) {
        chanList.push_back(index);
        index++;
      }
      swapList.push_back(chanList);
    }
    swapList[src].swap(swapList[dest]);

    // unfold it
    index=0;
    for (std::vector<int>& i: swapList) {
      for (int& j: i) {
        swappedChannels[index++]=j;
      }
    }

    // swap channels
    logV("swap list:");
    for (int i=0; i<tchans; i++) {
      logV("- %d -> %d",unswappedChannels[i],swappedChannels[i]);
    }

    for (size_t i=0; i<song.subsong.size(); i++) {
      DivOrders prevOrders=song.subsong[i]->orders;
      DivPattern* prevPat[DIV_MAX_CHANS][DIV_MAX_PATTERNS];
      unsigned char prevEffectCols[DIV_MAX_CHANS];
      String prevChanName[DIV_MAX_CHANS];
      String prevChanShortName[DIV_MAX_CHANS];
      bool prevChanShow[DIV_MAX_CHANS];
      unsigned char prevChanCollapse[DIV_MAX_CHANS];

      for (int j=0; j<tchans; j++) {
        for (int k=0; k<DIV_MAX_PATTERNS; k++) {
          prevPat[j][k]=song.subsong[i]->pat[j].data[k];
        }
        prevEffectCols[j]=song.subsong[i]->pat[j].effectCols;

        prevChanName[j]=song.subsong[i]->chanName[j];
        prevChanShortName[j]=song.subsong[i]->chanShortName[j];
        prevChanShow[j]=song.subsong[i]->chanShow[j];
        prevChanCollapse[j]=song.subsong[i]->chanCollapse[j];
      }

      for (int j=0; j<tchans; j++) {
        for (int k=0; k<DIV_MAX_PATTERNS; k++) {
          song.subsong[i]->orders.ord[j][k]=prevOrders.ord[swappedChannels[j]][k];
          song.subsong[i]->pat[j].data[k]=prevPat[swappedChannels[j]][k];
        }

        song.subsong[i]->pat[j].effectCols=prevEffectCols[swappedChannels[j]];
        song.subsong[i]->chanName[j]=prevChanName[swappedChannels[j]];
        song.subsong[i]->chanShortName[j]=prevChanShortName[swappedChannels[j]];
        song.subsong[i]->chanShow[j]=prevChanShow[swappedChannels[j]];
        song.subsong[i]->chanCollapse[j]=prevChanCollapse[swappedChannels[j]];
      }
    }
  }

  DivSystem srcSystem=song.system[src];
  float srcVol=song.systemVol[src];
  float srcPan=song.systemPan[src];
  float srcPanFR=song.systemPanFR[src];

  song.system[src]=song.system[dest];
  song.system[dest]=srcSystem;

  song.systemVol[src]=song.systemVol[dest];
  song.systemVol[dest]=srcVol;

  song.systemPan[src]=song.systemPan[dest];
  song.systemPan[dest]=srcPan;

  song.systemPanFR[src]=song.systemPanFR[dest];
  song.systemPanFR[dest]=srcPanFR;

  // I am kinda scared to use std::swap
  DivConfig oldFlags=song.systemFlags[src];
  song.systemFlags[src]=song.systemFlags[dest];
  song.systemFlags[dest]=oldFlags;

  // patchbay
  for (unsigned int& i: song.patchbay) {
    if (((i>>20)&0xfff)==(unsigned int)src) {
      i=(i&(~0xfff00000))|((unsigned int)dest<<20);
    } else if (((i>>20)&0xfff)==(unsigned int)dest) {
      i=(i&(~0xfff00000))|((unsigned int)src<<20);
    }
  }

  recalcChans();
  saveLock.unlock();
  BUSY_END;
  initDispatch();
  BUSY_BEGIN;
  renderSamples();
  reset();
  BUSY_END;
  return true;
}

void DivEngine::poke(int sys, unsigned int addr, unsigned short val) {
  if (sys<0 || sys>=song.systemLen) return;
  BUSY_BEGIN;
  disCont[sys].dispatch->poke(addr,val);
  BUSY_END;
}

void DivEngine::poke(int sys, std::vector<DivRegWrite>& wlist) {
  if (sys<0 || sys>=song.systemLen) return;
  BUSY_BEGIN;
  disCont[sys].dispatch->poke(wlist);
  BUSY_END;
}

String DivEngine::getLastError() {
  return lastError;
}

String DivEngine::getWarnings() {
  return warnings;
}

String DivEngine::getPlaybackDebugInfo() {
  return fmt::sprintf(
    "curOrder: %d\n"
    "prevOrder: %d\n"
    "curRow: %d\n"
    "prevRow: %d\n"
    "ticks: %d\n"
    "subticks: %d\n"
    "totalLoops: %d\n"
    "lastLoopPos: %d\n"
    "nextSpeed: %d\n"
    "divider: %f\n"
    "cycles: %d\n"
    "clockDrift: %f\n"
    "midiClockCycles: %d\n"
    "midiClockDrift: %f\n"
    "midiTimeCycles: %d\n"
    "midiTimeDrift: %f\n"
    "changeOrd: %d\n"
    "changePos: %d\n"
    "totalSeconds: %d\n"
    "totalTicks: %d\n"
    "totalTicksR: %d\n"
    "curMidiClock: %d\n"
    "curMidiTime: %d\n"
    "totalCmds: %d\n"
    "lastCmds: %d\n"
    "cmdsPerSecond: %d\n"
    "globalPitch: %d\n"
    "extValue: %d\n"
    "tempoAccum: %d\n"
    "totalProcessed: %d\n"
    "bufferPos: %d\n",
    curOrder,prevOrder,curRow,prevRow,ticks,subticks,totalLoops,lastLoopPos,nextSpeed,divider,cycles,clockDrift,
    midiClockCycles,midiClockDrift,midiTimeCycles,midiTimeDrift,changeOrd,changePos,totalSeconds,totalTicks,
    totalTicksR,curMidiClock,curMidiTime,totalCmds,lastCmds,cmdsPerSecond,globalPitch,
    (int)extValue,(int)tempoAccum,(int)totalProcessed,(int)bufferPos
  );
}

DivInstrument* DivEngine::getIns(int index, DivInstrumentType fallbackType) {
  if (index==-2 && tempIns!=NULL) {
    return tempIns;
  }
  if (index<0 || index>=song.insLen) {
    switch (fallbackType) {
      case DIV_INS_OPLL:
        return &song.nullInsOPLL;
        break;
      case DIV_INS_OPL:
        return &song.nullInsOPL;
        break;
      case DIV_INS_OPL_DRUMS:
        return &song.nullInsOPLDrums;
        break;
      default:
        break;
    }
    return &song.nullIns;
  }
  return song.ins[index];
}

DivWavetable* DivEngine::getWave(int index) {
  if (index<0 || index>=song.waveLen) {
    if (song.waveLen>0) {
      return song.wave[0];
    } else {
      return &song.nullWave;
    }
  }
  return song.wave[index];
}

DivSample* DivEngine::getSample(int index) {
  if (index<0 || index>=song.sampleLen) return &song.nullSample;
  return song.sample[index];
}

DivDispatch* DivEngine::getDispatch(int index) {
  if (index<0 || index>=song.systemLen) return NULL;
  return disCont[index].dispatch;
}

void DivEngine::setLoops(int loops) {
  remainingLoops=loops;
}

DivChannelState* DivEngine::getChanState(int ch) {
  if (ch<0 || ch>=chans) return NULL;
  return &chan[ch];
}

void* DivEngine::getDispatchChanState(int ch) {
  if (ch<0 || ch>=chans) return NULL;
  return disCont[dispatchOfChan[ch]].dispatch->getChanState(dispatchChanOfChan[ch]);
}

unsigned char* DivEngine::getRegisterPool(int sys, int& size, int& depth) {
  if (sys<0 || sys>=song.systemLen) return NULL;
  if (disCont[sys].dispatch==NULL) return NULL;
  size=disCont[sys].dispatch->getRegisterPoolSize();
  depth=disCont[sys].dispatch->getRegisterPoolDepth();
  return disCont[sys].dispatch->getRegisterPool();
}

DivMacroInt* DivEngine::getMacroInt(int chan) {
  if (chan<0 || chan>=chans) return NULL;
  return disCont[dispatchOfChan[chan]].dispatch->getChanMacroInt(dispatchChanOfChan[chan]);
}

DivSamplePos DivEngine::getSamplePos(int chan) {
  if (chan<0 || chan>=chans) return DivSamplePos();
  return disCont[dispatchOfChan[chan]].dispatch->getSamplePos(dispatchChanOfChan[chan]);
}

DivDispatchOscBuffer* DivEngine::getOscBuffer(int chan) {
  if (chan<0 || chan>=chans) return NULL;
  return disCont[dispatchOfChan[chan]].dispatch->getOscBuffer(dispatchChanOfChan[chan]);
}

void DivEngine::enableCommandStream(bool enable) {
  cmdStreamEnabled=enable;
}

void DivEngine::getCommandStream(std::vector<DivCommand>& where) {
  BUSY_BEGIN;
  where.clear();
  for (DivCommand& i: cmdStream) {
    where.push_back(i);
  }
  cmdStream.clear();
  BUSY_END;
}

void DivEngine::playSub(bool preserveDrift, int goalRow) {
  logV("playSub() called");
  std::chrono::high_resolution_clock::time_point timeStart=std::chrono::high_resolution_clock::now();
  for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->setSkipRegisterWrites(false);
  reset();
  if (preserveDrift && curOrder==0) {
    logV("preserveDrift && curOrder is true");
    return;
  }
  bool oldRepeatPattern=repeatPattern;
  repeatPattern=false;
  int goal=curOrder;
  curOrder=0;
  curRow=0;
  prevOrder=0;
  prevRow=0;
  stepPlay=0;
  if (curSubSong!=NULL) curSubSong->arpLen=1;
  int prevDrift, prevMidiClockDrift, prevMidiTimeDrift;
  prevDrift=clockDrift;
  prevMidiClockDrift=midiClockDrift;
  prevMidiTimeDrift=midiTimeDrift;
  clockDrift=0;
  cycles=0;
  midiClockCycles=0;
  midiClockDrift=0;
  midiTimeCycles=0;
  midiTimeDrift=0;
  if (!preserveDrift) {
    ticks=1;
    tempoAccum=0;
    totalTicks=0;
    totalSeconds=0;
    totalTicksR=0;
    curMidiClock=0;
    curMidiTime=0;
    curMidiTimeCode=0;
    curMidiTimePiece=0;
    totalLoops=0;
    lastLoopPos=-1;
  }
  endOfSong=false;
  // whaaaaa?
  curSpeed=0;
  playing=true;
  skipping=true;
  memset(walked,0,8192);
  for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->setSkipRegisterWrites(true);
  logV("goal: %d goalRow: %d",goal,goalRow);
  while (playing && curOrder<goal) {
    if (nextTick(preserveDrift)) {
      skipping=false;
      return;
    }
    if (!preserveDrift) {
      runMidiClock(cycles);
      runMidiTime(cycles);
    }
  }
  int oldOrder=curOrder;
  while (playing && (curRow<goalRow || ticks>1)) {
    if (nextTick(preserveDrift)) {
      skipping=false;
      return;
    }
    if (!preserveDrift) {
      runMidiClock(cycles);
      runMidiTime(cycles);
    }
    if (oldOrder!=curOrder) break;
    if (ticks-((tempoAccum+curSubSong->virtualTempoN)/MAX(1,curSubSong->virtualTempoD))<1 && curRow>=goalRow) break;
  }
  for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->setSkipRegisterWrites(false);
  if (goal>0 || goalRow>0) {
    for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->forceIns();
  }
  for (int i=0; i<chans; i++) {
    chan[i].cut=-1;
  }
  repeatPattern=oldRepeatPattern;
  if (preserveDrift) {
    clockDrift=prevDrift;
    midiClockDrift=prevMidiClockDrift;
    midiTimeDrift=prevMidiTimeDrift;
  } else {
    clockDrift=0;
    cycles=0;
    midiClockCycles=0;
    midiClockDrift=0;
    midiTimeCycles=0;
    midiTimeDrift=0;
    if (curMidiTime>0) {
      curMidiTime--;
    }
    if (curMidiClock>0) {
      curMidiClock--;
    }
    curMidiTimePiece=0;
  }
  if (!preserveDrift) {
    ticks=1;
    subticks=1;
    prevOrder=curOrder;
    prevRow=curRow;
    tempoAccum=0;
  }
  skipping=false;
  cmdStream.clear();
  std::chrono::high_resolution_clock::time_point timeEnd=std::chrono::high_resolution_clock::now();
  logV("playSub() took %dµs",std::chrono::duration_cast<std::chrono::microseconds>(timeEnd-timeStart).count());
}

/*
int DivEngine::calcBaseFreq(double clock, double divider, int note, bool period) {
  double base=(period?(song.tuning*0.0625):song.tuning)*pow(2.0,(float)(note+3)/12.0);
  return period?
         round((clock/base)/divider):
         base*(divider/clock);
}*/

double DivEngine::calcBaseFreq(double clock, double divider, int note, bool period) {
  if (song.linearPitch==2) { // full linear
    return (note<<7);
  }
  double base=(period?(song.tuning*0.0625):song.tuning)*pow(2.0,(float)(note+3)/12.0);
  return period?
         (clock/base)/divider:
         base*(divider/clock);
}

#define CONVERT_FNUM_BLOCK(bf,bits,note) \
  double tuning=song.tuning; \
  if (tuning<400.0) tuning=400.0; \
  if (tuning>500.0) tuning=500.0; \
  int boundaryBottom=tuning*pow(2.0,0.25)*(divider/clock); \
  int boundaryTop=2.0*tuning*pow(2.0,0.25)*(divider/clock); \
  while (boundaryTop>((1<<bits)-1)) { \
    boundaryTop>>=1; \
    boundaryBottom>>=1; \
  } \
  int block=(note)/12; \
  if (block<0) block=0; \
  if (block>7) block=7; \
  bf>>=block; \
  if (bf<0) bf=0; \
  /* octave boundaries */ \
  while (bf>0 && bf<boundaryBottom && block>0) { \
    bf<<=1; \
    block--; \
  } \
  if (bf>boundaryTop) { \
    while (block<7 && bf>boundaryTop) { \
      bf>>=1; \
      block++; \
    } \
    if (bf>((1<<bits)-1)) { \
      bf=(1<<bits)-1; \
    } \
  } \
  /* logV("f-num: %d block: %d",bf,block); */ \
  return bf|(block<<bits);

int DivEngine::calcBaseFreqFNumBlock(double clock, double divider, int note, int bits) {
  if (song.linearPitch==2) { // full linear
    return (note<<7);
  }
  int bf=calcBaseFreq(clock,divider,note,false);
  CONVERT_FNUM_BLOCK(bf,bits,note)
}

int DivEngine::calcFreq(int base, int pitch, int arp, bool arpFixed, bool period, int octave, int pitch2, double clock, double divider, int blockBits) {
  if (song.linearPitch==2) {
    // do frequency calculation here
    int nbase=base+pitch+pitch2;
    if (!song.oldArpStrategy) {
      if (arpFixed) {
        nbase=(arp<<7)+pitch+pitch2;
      } else {
        nbase+=arp<<7;
      }
    }
    double fbase=(period?(song.tuning*0.0625):song.tuning)*pow(2.0,(float)(nbase+384)/(128.0*12.0));
    int bf=period?
           round((clock/fbase)/divider):
           round(fbase*(divider/clock));
    if (blockBits>0) {
      CONVERT_FNUM_BLOCK(bf,blockBits,nbase>>7)
    } else {
      return bf;
    }
  }
  if (song.linearPitch==1) {
    // global pitch multiplier
    int whatTheFuck=(1024+(globalPitch<<6)-(globalPitch<0?globalPitch-6:0));
    if (whatTheFuck<1) whatTheFuck=1; // avoids division by zero but please kill me
    if (song.pitchMacroIsLinear) {
      pitch+=pitch2;
    }
    pitch+=2048;
    if (pitch<0) pitch=0;
    if (pitch>4095) pitch=4095;
    int ret=period?
              ((base*(reversePitchTable[pitch]))/whatTheFuck):
              (((base*(pitchTable[pitch]))>>10)*whatTheFuck)/1024;
    if (!song.pitchMacroIsLinear) {
      ret+=period?(-pitch2):pitch2;
    }
    return ret;
  }
  return period?
           base-pitch-pitch2:
           base+((pitch*octave)>>1)+pitch2;
}

int DivEngine::calcArp(int note, int arp, int offset) {
  if (arp<0) {
    if (!(arp&0x40000000)) return (arp|0x40000000)+offset;
  } else {
    if (arp&0x40000000) return (arp&(~0x40000000))+offset;
  }
  return note+arp;
}

int DivEngine::convertPanSplitToLinear(unsigned int val, unsigned char bits, int range) {
  int panL=val>>bits;
  int panR=val&((1<<bits)-1);
  int diff=panR-panL;
  float pan=0.5f;
  if (diff!=0) {
    pan=(1.0f+((float)diff/(float)MAX(panL,panR)))*0.5f;
  }
  return pan*range;
}

int DivEngine::convertPanSplitToLinearLR(unsigned char left, unsigned char right, int range) {
  return convertPanSplitToLinear((left<<8)|right,8,range);
}

unsigned int DivEngine::convertPanLinearToSplit(int val, unsigned char bits, int range) {
  if (val<0) val=0;
  if (val>range) val=range;
  int maxV=(1<<bits)-1;
  int panL=(((range-val)*maxV*2))/range;
  int panR=((val)*maxV*2)/range;
  if (panL>maxV) panL=maxV;
  if (panR>maxV) panR=maxV;
  return (panL<<bits)|panR;
}

void DivEngine::play() {
  BUSY_BEGIN_SOFT;
  curOrder=prevOrder;
  sPreview.sample=-1;
  sPreview.wave=-1;
  sPreview.pos=0;
  sPreview.dir=false;
  shallStop=false;
  if (stepPlay==0) {
    freelance=false;
    playSub(false);
  } else {
    stepPlay=0;
  }
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    keyHit[i]=false;
  }
  curMidiTimePiece=0;
  if (output) if (!skipping && output->midiOut!=NULL) {
    if (midiOutClock) {
      output->midiOut->send(TAMidiMessage(TA_MIDI_POSITION,(curMidiClock>>7)&0x7f,curMidiClock&0x7f));
    }
    if (midiOutTime) {
      TAMidiMessage msg;
      msg.type=TA_MIDI_SYSEX;
      msg.sysExData.reset(new unsigned char[10],std::default_delete<unsigned char[]>());
      msg.sysExLen=10;
      unsigned char* msgData=msg.sysExData.get();
      int actualTime=curMidiTime;
      int timeRate=midiOutTimeRate;
      int drop=0;
      if (timeRate<1 || timeRate>4) {
        if (curSubSong->hz>=47.98 && curSubSong->hz<=48.02) {
          timeRate=1;
        } else if (curSubSong->hz>=49.98 && curSubSong->hz<=50.02) {
          timeRate=2;
        } else if (curSubSong->hz>=59.9 && curSubSong->hz<=60.11) {
          timeRate=4;
        } else {
          timeRate=4;
        }
      }

      switch (timeRate) {
        case 1: // 24
          msgData[5]=(actualTime/(60*60*24))%24;
          msgData[6]=(actualTime/(60*24))%60;
          msgData[7]=(actualTime/24)%60;
          msgData[8]=actualTime%24;
          break;
        case 2: // 25
          msgData[5]=(actualTime/(60*60*25))%24;
          msgData[6]=(actualTime/(60*25))%60;
          msgData[7]=(actualTime/25)%60;
          msgData[8]=actualTime%25;
          break;
        case 3: // 29.97 (NTSC drop)
          // drop
          drop=((actualTime/(30*60))-(actualTime/(30*600)))*2;
          actualTime+=drop;

          msgData[5]=(actualTime/(60*60*30))%24;
          msgData[6]=(actualTime/(60*30))%60;
          msgData[7]=(actualTime/30)%60;
          msgData[8]=actualTime%30;
          break;
        case 4: // 30 (NTSC non-drop)
        default:
          msgData[5]=(actualTime/(60*60*30))%24;
          msgData[6]=(actualTime/(60*30))%60;
          msgData[7]=(actualTime/30)%60;
          msgData[8]=actualTime%30;
          break;
      }

      msgData[5]|=(timeRate-1)<<5;

      msgData[0]=0xf0;
      msgData[1]=0x7f;
      msgData[2]=0x7f;
      msgData[3]=0x01;
      msgData[4]=0x01;
      msgData[9]=0xf7;
      output->midiOut->send(msg);
    }
    output->midiOut->send(TAMidiMessage(TA_MIDI_MACHINE_PLAY,0,0));
  }
  BUSY_END;
}

void DivEngine::playToRow(int row) {
  BUSY_BEGIN_SOFT;
  sPreview.sample=-1;
  sPreview.wave=-1;
  sPreview.pos=0;
  sPreview.dir=false;
  freelance=false;
  playSub(false,row);
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    keyHit[i]=false;
  }
  BUSY_END;
}

void DivEngine::stepOne(int row) {
  if (!isPlaying()) {
    BUSY_BEGIN_SOFT;
    freelance=false;
    playSub(false,row);
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      keyHit[i]=false;
    }
  } else {
    BUSY_BEGIN;
  }
  stepPlay=2;
  ticks=1;
  prevOrder=curOrder;
  prevRow=curRow;
  BUSY_END;
}

void DivEngine::stop() {
  BUSY_BEGIN;
  freelance=false;
  if (!playing) {
    //Send midi panic
    if (output) if (output->midiOut!=NULL) {
      output->midiOut->send(TAMidiMessage(TA_MIDI_CONTROL,0x7B,0));
      logV("Midi panic sent");
    }
  }
  playing=false;
  extValuePresent=false;
  endOfSong=false; // what?
  stepPlay=0;
  curOrder=prevOrder;
  curRow=prevRow;
  remainingLoops=-1;
  sPreview.sample=-1;
  sPreview.wave=-1;
  sPreview.pos=0;
  sPreview.dir=false;
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].dispatch->notifyPlaybackStop();
  }
  if (output) if (output->midiOut!=NULL) {
    output->midiOut->send(TAMidiMessage(TA_MIDI_MACHINE_STOP,0,0));
    for (int i=0; i<chans; i++) {
      if (chan[i].curMidiNote>=0) {
        output->midiOut->send(TAMidiMessage(0x80|(i&15),chan[i].curMidiNote,0));
      }
    }
  }

  // reset all chan oscs
  for (int i=0; i<chans; i++) {
    DivDispatchOscBuffer* buf=disCont[dispatchOfChan[i]].dispatch->getOscBuffer(dispatchChanOfChan[i]);
    if (buf!=NULL) {
      memset(buf->data,0,65536*sizeof(short));
      buf->needle=0;
      buf->readNeedle=0;
    }
  }
  BUSY_END;
}

void DivEngine::halt() {
  BUSY_BEGIN;
  halted=true;
  BUSY_END;
}

void DivEngine::resume() {
  BUSY_BEGIN;
  halted=false;
  haltOn=DIV_HALT_NONE;
  BUSY_END;
}

void DivEngine::haltWhen(DivHaltPositions when) {
  BUSY_BEGIN;
  halted=false;
  haltOn=when;
  BUSY_END;
}

bool DivEngine::isHalted() {
  return halted;
}

const char** DivEngine::getRegisterSheet(int sys) {
  if (sys<0 || sys>=song.systemLen) return NULL;
  return disCont[sys].dispatch->getRegisterSheet();
}

void DivEngine::recalcChans() {
  bool isInsTypePossible[DIV_INS_MAX];
  chans=0;
  int chanIndex=0;
  memset(isInsTypePossible,0,DIV_INS_MAX*sizeof(bool));
  for (int i=0; i<song.systemLen; i++) {
    int chanCount=getChannelCount(song.system[i]);
    chans+=chanCount;
    for (int j=0; j<chanCount; j++) {
      sysOfChan[chanIndex]=song.system[i];
      dispatchOfChan[chanIndex]=i;
      dispatchChanOfChan[chanIndex]=j;
      chanIndex++;

      if (sysDefs[song.system[i]]!=NULL) {
        if (sysDefs[song.system[i]]->chanInsType[j][0]!=DIV_INS_NULL) {
          isInsTypePossible[sysDefs[song.system[i]]->chanInsType[j][0]]=true;
        }

        if (sysDefs[song.system[i]]->chanInsType[j][1]!=DIV_INS_NULL) {
          isInsTypePossible[sysDefs[song.system[i]]->chanInsType[j][1]]=true;
        }
      }
    }
  }

  possibleInsTypes.clear();
  for (int i=0; i<DIV_INS_MAX; i++) {
    if (isInsTypePossible[i]) possibleInsTypes.push_back((DivInstrumentType)i);
  }

  checkAssetDir(song.insDir,song.ins.size());
  checkAssetDir(song.waveDir,song.wave.size());
  checkAssetDir(song.sampleDir,song.sample.size());

  hasLoadedSomething=true;
}

void DivEngine::reset() {
  if (output) if (output->midiOut!=NULL) {
    output->midiOut->send(TAMidiMessage(TA_MIDI_MACHINE_STOP,0,0));
    for (int i=0; i<chans; i++) {
      if (chan[i].curMidiNote>=0) {
        output->midiOut->send(TAMidiMessage(0x80|(i&15),chan[i].curMidiNote,0));
      }
    }
  }
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    chan[i]=DivChannelState();
    if (i<chans) chan[i].volMax=(disCont[dispatchOfChan[i]].dispatch->dispatch(DivCommand(DIV_CMD_GET_VOLMAX,dispatchChanOfChan[i]))<<8)|0xff;
    chan[i].volume=chan[i].volMax;
    if (song.linearPitch==0) chan[i].vibratoFine=4;
  }
  extValue=0;
  extValuePresent=0;
  speeds=curSubSong->speeds;
  firstTick=false;
  shallStop=false;
  shallStopSched=false;
  pendingMetroTick=0;
  elapsedBars=0;
  elapsedBeats=0;
  nextSpeed=speeds.val[0];
  divider=curSubSong->hz;
  globalPitch=0;
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].dispatch->reset();
    disCont[i].clear();
  }
}

void DivEngine::syncReset() {
  BUSY_BEGIN;
  reset();
  BUSY_END;
}

const int sampleRates[6]={
  4000, 8000, 11025, 16000, 22050, 32000
};

int DivEngine::fileToDivRate(int frate) {
  if (frate<0) frate=0;
  if (frate>5) frate=5;
  return sampleRates[frate];
}

int DivEngine::divToFileRate(int drate) {
  if (drate>26000) {
    return 5;
  } else if (drate>18000) {
    return 4;
  } else if (drate>14000) {
    return 3;
  } else if (drate>9500) {
    return 2;
  } else if (drate>6000) {
    return 1;
  } else {
    return 0;
  }
  return 4;
}

void DivEngine::testFunction() {
  logI("it works!");
}

int DivEngine::getEffectiveSampleRate(int rate) {
  if (rate<1) return 0;
  switch (song.system[0]) {
    case DIV_SYSTEM_YMU759:
      return 8000;
    case DIV_SYSTEM_YM2612: case DIV_SYSTEM_YM2612_EXT:
      return 1278409/(1280000/rate);
    case DIV_SYSTEM_PCE:
      return 1789773/(1789773/rate);
    case DIV_SYSTEM_SEGAPCM: case DIV_SYSTEM_SEGAPCM_COMPAT:
      return (31250*MIN(255,(rate*255/31250)))/255;
    case DIV_SYSTEM_QSOUND:
      return (24038*MIN(65535,(rate*4096/24038)))/4096;
    case DIV_SYSTEM_YM2610: case DIV_SYSTEM_YM2610_EXT: case DIV_SYSTEM_YM2610_FULL: case DIV_SYSTEM_YM2610_FULL_EXT: case DIV_SYSTEM_YM2610B: case DIV_SYSTEM_YM2610B_EXT:
      return 18518;
    case DIV_SYSTEM_VERA:
      return (48828*MIN(128,(rate*128/48828)))/128;
    case DIV_SYSTEM_X1_010:
      return (31250*MIN(255,(rate*16/31250)))/16; // TODO: support variable clock case
    case DIV_SYSTEM_ES5506:
      return (31250*MIN(131071,(rate*2048/31250)))/2048; // TODO: support variable clock, channel limit case
    default:
      break;
  }
  return rate;
}

void DivEngine::previewSample(int sample, int note, int pStart, int pEnd) {
  BUSY_BEGIN;
  previewSampleNoLock(sample,note,pStart,pEnd);
  BUSY_END;
}

void DivEngine::stopSamplePreview() {
  BUSY_BEGIN;
  stopSamplePreviewNoLock();
  BUSY_END;
}

void DivEngine::previewWave(int wave, int note) {
  BUSY_BEGIN;
  previewWaveNoLock(wave,note);
  BUSY_END;
}

void DivEngine::stopWavePreview() {
  BUSY_BEGIN;
  stopWavePreviewNoLock();
  BUSY_END;
}

void DivEngine::previewSampleNoLock(int sample, int note, int pStart, int pEnd) {
  sPreview.pBegin=pStart;
  sPreview.pEnd=pEnd;
  sPreview.dir=false;
  if (sample<0 || sample>=(int)song.sample.size()) {
    sPreview.sample=-1;
    sPreview.pos=0;
    sPreview.dir=false;
    return;
  }
  blip_clear(samp_bb);
  double rate=song.sample[sample]->centerRate;
  if (note>=0) {
    rate=(pow(2.0,(double)(note)/12.0)*((double)song.sample[sample]->centerRate)*0.0625);
    if (rate<=0) rate=song.sample[sample]->centerRate;
  }
  if (rate<100) rate=100;
  double rateOrig=rate;
  sPreview.rateMul=1;
  while (sPreview.rateMul<0x40000000 && rate<got.rate) {
    sPreview.rateMul<<=1;
    rate*=2.0;
  }
  blip_set_rates(samp_bb,rate,got.rate);
  samp_prevSample=0;
  sPreview.rate=rateOrig;
  sPreview.pos=(sPreview.pBegin>=0)?sPreview.pBegin:0;
  sPreview.posSub=0;
  sPreview.sample=sample;
  sPreview.wave=-1;
  sPreview.dir=false;
}

void DivEngine::stopSamplePreviewNoLock() {
  sPreview.sample=-1;
  sPreview.pos=0;
  sPreview.dir=false;
}

void DivEngine::previewWaveNoLock(int wave, int note) {
  if (wave<0 || wave>=(int)song.wave.size()) {
    sPreview.wave=-1;
    sPreview.pos=0;
    sPreview.dir=false;
    return;
  }
  if (song.wave[wave]->len<=0) {
    return;
  }
  blip_clear(samp_bb);
  double rate=song.wave[wave]->len*((song.tuning*0.0625)*pow(2.0,(double)(note+3)/12.0));
  if (rate<100) rate=100;
  double rateOrig=rate;
  sPreview.rateMul=1;
  while (sPreview.rateMul<0x40000000 && rate<got.rate) {
    sPreview.rateMul<<=1;
    rate*=2.0;
  }
  blip_set_rates(samp_bb,rate,got.rate);
  samp_prevSample=0;
  sPreview.rate=rateOrig;
  sPreview.pos=0;
  sPreview.posSub=0;
  sPreview.sample=-1;
  sPreview.wave=wave;
  sPreview.dir=false;
}

void DivEngine::stopWavePreviewNoLock() {
  sPreview.wave=-1;
  sPreview.pos=0;
  sPreview.dir=false;
}

bool DivEngine::isPreviewingSample() {
  return (sPreview.sample>=0 && sPreview.sample<(int)song.sample.size());
}

int DivEngine::getSamplePreviewPos() {
  return sPreview.pos;
}

double DivEngine::getSamplePreviewRate() {
  return sPreview.rate;
}

String DivEngine::getConfigPath() {
  return configPath;
}

int DivEngine::getMaxVolumeChan(int ch) {
  return chan[ch].volMax>>8;
}

unsigned char DivEngine::getOrder() {
  return prevOrder;
}

int DivEngine::getRow() {
  return prevRow;
}

int DivEngine::getElapsedBars() {
  return elapsedBars;
}

int DivEngine::getElapsedBeats() {
  return elapsedBeats;
}

size_t DivEngine::getCurrentSubSong() {
  return curSubSongIndex;
}

const DivGroovePattern& DivEngine::getSpeeds() {
  return speeds;
}

float DivEngine::getHz() {
  return curSubSong->hz;
}

float DivEngine::getCurHz() {
  return divider;
}

int DivEngine::getTotalSeconds() {
  return totalSeconds;
}

int DivEngine::getTotalTicks() {
  return totalTicks;
}

bool DivEngine::getRepeatPattern() {
  return repeatPattern;
}

void DivEngine::setRepeatPattern(bool value) {
  BUSY_BEGIN;
  repeatPattern=value;
  BUSY_END;
}

bool DivEngine::hasExtValue() {
  return extValuePresent;
}

unsigned char DivEngine::getExtValue() {
  return extValue;
}

bool DivEngine::isPlaying() {
  return (playing && !freelance);
}

bool DivEngine::isRunning() {
  return playing;
}

bool DivEngine::isStepping() {
  return !(stepPlay==0);
}

bool DivEngine::isChannelMuted(int chan) {
  return isMuted[chan];
}

void DivEngine::toggleMute(int chan) {
  muteChannel(chan,!isMuted[chan]);
}

void DivEngine::toggleSolo(int chan) {
  bool solo=false;
  for (int i=0; i<chans; i++) {
    if (i==chan) {
      solo=true;
      continue;
    } else {
      if (!isMuted[i]) {
        solo=false;
        break;
      }
    }
  }
  BUSY_BEGIN;
  if (!solo) {
    for (int i=0; i<chans; i++) {
      isMuted[i]=(i!=chan);
      if (disCont[dispatchOfChan[i]].dispatch!=NULL) {
        disCont[dispatchOfChan[i]].dispatch->muteChannel(dispatchChanOfChan[i],isMuted[i]);
      }
    }
  } else {
    for (int i=0; i<chans; i++) {
      isMuted[i]=false;
      if (disCont[dispatchOfChan[i]].dispatch!=NULL) {
        disCont[dispatchOfChan[i]].dispatch->muteChannel(dispatchChanOfChan[i],isMuted[i]);
      }
    }
  }
  BUSY_END;
}

void DivEngine::muteChannel(int chan, bool mute) {
  BUSY_BEGIN;
  isMuted[chan]=mute;
  if (disCont[dispatchOfChan[chan]].dispatch!=NULL) {
    disCont[dispatchOfChan[chan]].dispatch->muteChannel(dispatchChanOfChan[chan],isMuted[chan]);
  }
  BUSY_END;
}

void DivEngine::unmuteAll() {
  BUSY_BEGIN;
  for (int i=0; i<chans; i++) {
    isMuted[i]=false;
    if (disCont[dispatchOfChan[i]].dispatch!=NULL) {
      disCont[dispatchOfChan[i]].dispatch->muteChannel(dispatchChanOfChan[i],isMuted[i]);
    }
  }
  BUSY_END;
}

int DivEngine::addInstrument(int refChan, DivInstrumentType fallbackType) {
  if (song.ins.size()>=256) return -1;
  BUSY_BEGIN;
  DivInstrument* ins=new DivInstrument;
  int insCount=(int)song.ins.size();
  DivInstrumentType prefType;
  if (refChan<0) {
    prefType=fallbackType;
  } else {
    prefType=getPreferInsType(refChan);
  }
  switch (prefType) {
    case DIV_INS_OPLL:
      *ins=song.nullInsOPLL;
      break;
    case DIV_INS_OPL:
      *ins=song.nullInsOPL;
      break;
    case DIV_INS_OPL_DRUMS:
      *ins=song.nullInsOPLDrums;
      break;
    default:
      break;
  }
  if (refChan>=0) {
    if (sysOfChan[refChan]==DIV_SYSTEM_QSOUND) {
      *ins=song.nullInsQSound;
    }
  }
  ins->name=fmt::sprintf("Instrument %d",insCount);
  if (prefType!=DIV_INS_NULL) {
    ins->type=prefType;
  }
  saveLock.lock();
  song.ins.push_back(ins);
  song.insLen=insCount+1;
  checkAssetDir(song.insDir,song.ins.size());
  saveLock.unlock();
  BUSY_END;
  return insCount;
}

int DivEngine::addInstrumentPtr(DivInstrument* which) {
  if (song.ins.size()>=256) {
    delete which;
    return -1;
  }
  BUSY_BEGIN;
  saveLock.lock();
  song.ins.push_back(which);
  song.insLen=song.ins.size();
  checkAssetDir(song.insDir,song.ins.size());
  checkAssetDir(song.waveDir,song.wave.size());
  checkAssetDir(song.sampleDir,song.sample.size());
  saveLock.unlock();
  BUSY_END;
  return song.insLen;
}

void DivEngine::loadTempIns(DivInstrument* which) {
  BUSY_BEGIN;
  if (tempIns==NULL) {
    tempIns=new DivInstrument;
  }
  *tempIns=*which;
  BUSY_END;
}

void DivEngine::delInstrument(int index) {
  BUSY_BEGIN;
  saveLock.lock();
  if (index>=0 && index<(int)song.ins.size()) {
    for (int i=0; i<song.systemLen; i++) {
      disCont[i].dispatch->notifyInsDeletion(song.ins[index]);
    }
    delete song.ins[index];
    song.ins.erase(song.ins.begin()+index);
    song.insLen=song.ins.size();
    for (int i=0; i<chans; i++) {
      for (size_t j=0; j<song.subsong.size(); j++) {
        for (int k=0; k<DIV_MAX_PATTERNS; k++) {
          if (song.subsong[j]->pat[i].data[k]==NULL) continue;
          for (int l=0; l<song.subsong[j]->patLen; l++) {
            if (song.subsong[j]->pat[i].data[k]->data[l][2]>index) {
              song.subsong[j]->pat[i].data[k]->data[l][2]--;
            }
          }
        }
      }
    }
    removeAsset(song.insDir,index);
    checkAssetDir(song.insDir,song.ins.size());
  }
  saveLock.unlock();
  BUSY_END;
}

int DivEngine::addWave() {
  if (song.wave.size()>=256) {
    lastError="too many wavetables!";
    return -1;
  }
  BUSY_BEGIN;
  saveLock.lock();
  DivWavetable* wave=new DivWavetable;
  int waveCount=(int)song.wave.size();
  song.wave.push_back(wave);
  song.waveLen=waveCount+1;
  checkAssetDir(song.waveDir,song.wave.size());
  saveLock.unlock();
  BUSY_END;
  return waveCount;
}

int DivEngine::addWavePtr(DivWavetable* which) {
  if (song.wave.size()>=256) {
    lastError="too many wavetables!";
    delete which;
    return -1;
  }
  BUSY_BEGIN;
  saveLock.lock();
  int waveCount=(int)song.wave.size();
  song.wave.push_back(which);
  song.waveLen=waveCount+1;
  checkAssetDir(song.waveDir,song.wave.size());
  saveLock.unlock();
  BUSY_END;
  return song.waveLen;
}

DivWavetable* DivEngine::waveFromFile(const char* path, bool addRaw) {
  FILE* f=ps_fopen(path,"rb");
  if (f==NULL) {
    lastError=fmt::sprintf("%s",strerror(errno));
    return NULL;
  }
  unsigned char* buf;
  ssize_t len;
  if (fseek(f,0,SEEK_END)!=0) {
    fclose(f);
    lastError=fmt::sprintf("could not seek to end: %s",strerror(errno));
    return NULL;
  }
  len=ftell(f);
  if (len<0) {
    fclose(f);
    lastError=fmt::sprintf("could not determine file size: %s",strerror(errno));
    return NULL;
  }
  if (len==(SIZE_MAX>>1)) {
    fclose(f);
    lastError="file size is invalid!";
    return NULL;
  }
  if (len==0) {
    fclose(f);
    lastError="file is empty";
    return NULL;
  }
  if (fseek(f,0,SEEK_SET)!=0) {
    fclose(f);
    lastError=fmt::sprintf("could not seek to beginning: %s",strerror(errno));
    return NULL;
  }
  buf=new unsigned char[len];
  if (fread(buf,1,len,f)!=(size_t)len) {
    logW("did not read entire wavetable file buffer!");
    delete[] buf;
    lastError=fmt::sprintf("could not read entire file: %s",strerror(errno));
    return NULL;
  }
  fclose(f);

  SafeReader reader=SafeReader(buf,len);

  unsigned char magic[16];
  bool isFurnaceTable=false;
  try {
    reader.read(magic,16);
    if (memcmp("-Furnace waveta-",magic,16)==0) {
      isFurnaceTable=true;
    }
  } catch (EndOfFileException& e) {
    reader.seek(0,SEEK_SET);
  }

  DivWavetable* wave=new DivWavetable;
  try {
    if (isFurnaceTable) {
      reader.seek(16,SEEK_SET);
      short version=reader.readS();
      reader.readS(); // reserved
      reader.seek(20,SEEK_SET);
      if (wave->readWaveData(reader,version)!=DIV_DATA_SUCCESS) {
        lastError="invalid wavetable header/data!";
        delete wave;
        delete[] buf;
        return NULL;
      }
    } else {
      try {
        // read as .dmw
        reader.seek(0,SEEK_SET);
        int len=reader.readI();
        logD("wave length %d",len);
        if (len<=0 || len>256) {
          throw EndOfFileException(&reader,reader.size());
        }
        wave->len=len;
        wave->max=(unsigned char)reader.readC();
        if (wave->max==255) { // new wavetable format
          unsigned char waveVersion=reader.readC();
          logI("reading modern .dmw...");
          logD("wave version %d",waveVersion);
          wave->max=(unsigned char)reader.readC();
          for (int i=0; i<len; i++) {
            wave->data[i]=reader.readI();
          }
        } else if (reader.size()==(size_t)(len+5)) {
          // read as .dmw
          logI("reading .dmw...");
          if (len>256) len=256;
          for (int i=0; i<len; i++) {
            wave->data[i]=(unsigned char)reader.readC();
          }
        } else {
          // read as binary
          if (addRaw) {
            logI("reading binary...");
            len=reader.size();
            if (len>256) len=256;
            reader.seek(0,SEEK_SET);
            for (int i=0; i<len; i++) {
              wave->data[i]=(unsigned char)reader.readC();
              if (wave->max<wave->data[i]) wave->max=wave->data[i];
            }
            wave->len=len;
          } else {
            delete wave;
            delete[] buf;
            return NULL;
          }
        }
      } catch (EndOfFileException& e) {
        // read as binary
        if (addRaw) {
          len=reader.size();
          logI("reading binary for being too small...");
          if (len>256) len=256;
          reader.seek(0,SEEK_SET);
          for (int i=0; i<len; i++) {
            wave->data[i]=(unsigned char)reader.readC();
            if (wave->max<wave->data[i]) wave->max=wave->data[i];
          }
          wave->len=len;
        } else {
          delete wave;
          delete[] buf;
          return NULL;
        }
      }
    }
  } catch (EndOfFileException& e) {
    delete wave;
    delete[] buf;
    lastError="premature end of file";
    return NULL;
  }
  
  return wave;
}

void DivEngine::delWave(int index) {
  BUSY_BEGIN;
  saveLock.lock();
  if (index>=0 && index<(int)song.wave.size()) {
    delete song.wave[index];
    song.wave.erase(song.wave.begin()+index);
    song.waveLen=song.wave.size();
    removeAsset(song.waveDir,index);
    checkAssetDir(song.waveDir,song.wave.size());
  }
  saveLock.unlock();
  BUSY_END;
}

int DivEngine::addSample() {
  if (song.sample.size()>=256) {
    lastError="too many samples!";
    return -1;
  }
  BUSY_BEGIN;
  saveLock.lock();
  DivSample* sample=new DivSample;
  int sampleCount=(int)song.sample.size();
  sample->name=fmt::sprintf("Sample %d",sampleCount);
  song.sample.push_back(sample);
  song.sampleLen=sampleCount+1;
  sPreview.sample=-1;
  sPreview.pos=0;
  sPreview.dir=false;
  checkAssetDir(song.sampleDir,song.sample.size());
  saveLock.unlock();
  renderSamples();
  BUSY_END;
  return sampleCount;
}

int DivEngine::addSamplePtr(DivSample* which) {
  if (song.sample.size()>=256) {
    lastError="too many samples!";
    delete which;
    return -1;
  }
  int sampleCount=(int)song.sample.size();
  BUSY_BEGIN;
  saveLock.lock();
  song.sample.push_back(which);
  song.sampleLen=sampleCount+1;
  checkAssetDir(song.sampleDir,song.sample.size());
  saveLock.unlock();
  renderSamples();
  BUSY_END;
  return sampleCount;
}

DivSample* DivEngine::sampleFromFile(const char* path) {
  if (song.sample.size()>=256) {
    lastError="too many samples!";
    return NULL;
  }
  BUSY_BEGIN;
  warnings="";

  const char* pathRedux=strrchr(path,DIR_SEPARATOR);
  if (pathRedux==NULL) {
    pathRedux=path;
  } else {
    pathRedux++;
  }
  String stripPath;
  const char* pathReduxEnd=strrchr(pathRedux,'.');
  if (pathReduxEnd==NULL) {
    stripPath=pathRedux;
  } else {
    for (const char* i=pathRedux; i!=pathReduxEnd && (*i); i++) {
      stripPath+=*i;
    }
  }

  const char* ext=strrchr(path,'.');
  if (ext!=NULL) {
    String extS;
    for (; *ext; ext++) {
      char i=*ext;
      if (i>='A' && i<='Z') {
        i+='a'-'A';
      }
      extS+=i;
    }
    if (extS==".dmc" || extS==".brr") { // read as .dmc or .brr
      size_t len=0;
      DivSample* sample=new DivSample;
      sample->name=stripPath;

      FILE* f=ps_fopen(path,"rb");
      if (f==NULL) {
        BUSY_END;
        lastError=fmt::sprintf("could not open file! (%s)",strerror(errno));
        delete sample;
        return NULL;
      }

      if (fseek(f,0,SEEK_END)<0) {
        fclose(f);
        BUSY_END;
        lastError=fmt::sprintf("could not get file length! (%s)",strerror(errno));
        delete sample;
        return NULL;
      }

      len=ftell(f);

      if (len==0) {
        fclose(f);
        BUSY_END;
        lastError="file is empty!";
        delete sample;
        return NULL;
      }

      if (len==(SIZE_MAX>>1)) {
        fclose(f);
        BUSY_END;
        lastError="file is invalid!";
        delete sample;
        return NULL;
      }

      if (fseek(f,0,SEEK_SET)<0) {
        fclose(f);
        BUSY_END;
        lastError=fmt::sprintf("could not seek to beginning of file! (%s)",strerror(errno));
        delete sample;
        return NULL;
      }

      if (extS==".dmc") {
        sample->rate=33144;
        sample->centerRate=33144;
        sample->depth=DIV_SAMPLE_DEPTH_1BIT_DPCM;
        sample->init(len*8);
      } else if (extS==".brr") {
        sample->rate=32000;
        sample->centerRate=32000;
        sample->depth=DIV_SAMPLE_DEPTH_BRR;
        sample->init(16*(len/9));
      } else {
        fclose(f);
        BUSY_END;
        lastError="wait... is that right? no I don't think so...";
        delete sample;
        return NULL;
      }

      unsigned char* dataBuf=sample->dataDPCM;
      if (extS==".brr") {
        dataBuf=sample->dataBRR;
        if ((len%9)==2) {
          // read loop position
          unsigned short loopPos=0;
          logD("BRR file has loop position");
          if (fread(&loopPos,1,2,f)!=2) {
            logW("could not read loop position! %s",strerror(errno));
          } else {
#ifdef TA_BIG_ENDIAN
            loopPos=(loopPos>>8)|(loopPos<<8);
#endif
            sample->loopStart=16*(loopPos/9);
            sample->loopEnd=sample->samples;
            sample->loop=true;
            sample->loopMode=DIV_SAMPLE_LOOP_FORWARD;
          }
          len-=2;
          if (len==0) {
            fclose(f);
            BUSY_END;
            lastError="BRR sample is empty!";
            delete sample;
            return NULL;
          }
        } else if ((len%9)!=0) {
          fclose(f);
          BUSY_END;
          lastError="possibly corrupt BRR sample!";
          delete sample;
          return NULL;
        }
      }

      if (fread(dataBuf,1,len,f)==0) {
        fclose(f);
        BUSY_END;
        lastError=fmt::sprintf("could not read file! (%s)",strerror(errno));
        delete sample;
        return NULL;
      }
      BUSY_END;
      return sample;
    }
  }

#ifndef HAVE_SNDFILE
  lastError="Furnace was not compiled with libsndfile!";
  return NULL;
#else
  SF_INFO si;
  SFWrapper sfWrap;
  memset(&si,0,sizeof(SF_INFO));
  SNDFILE* f=sfWrap.doOpen(path,SFM_READ,&si);
  if (f==NULL) {
    BUSY_END;
    int err=sf_error(NULL);
    if (err==SF_ERR_SYSTEM) {
      lastError=fmt::sprintf("could not open file! (%s %s)",sf_error_number(err),strerror(errno));
    } else {
      lastError=fmt::sprintf("could not open file! (%s)\nif this is raw sample data, you may import it by right-clicking the Load Sample icon and selecting \"import raw\".",sf_error_number(err));
    }
    return NULL;
  }
  if (si.frames>16777215) {
    lastError="this sample is too big! max sample size is 16777215.";
    sfWrap.doClose();
    BUSY_END;
    return NULL;
  }
  void* buf=NULL;
  sf_count_t sampleLen=sizeof(short);
  if ((si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_PCM_U8) {
    logD("sample is 8-bit unsigned");
    buf=new unsigned char[si.channels*si.frames];
    sampleLen=sizeof(unsigned char);
  } else if ((si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_FLOAT)  {
    logD("sample is 32-bit float");
    buf=new float[si.channels*si.frames];
    sampleLen=sizeof(float);
  } else {
    logD("sample is 16-bit signed");
    buf=new short[si.channels*si.frames];
    sampleLen=sizeof(short);
  }
  if ((si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_PCM_U8 || (si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_FLOAT) {
    if (sf_read_raw(f,buf,si.frames*si.channels*sampleLen)!=(si.frames*si.channels*sampleLen)) {
      logW("sample read size mismatch!");
    }
  } else {
    if (sf_read_short(f,(short*)buf,si.frames*si.channels)!=(si.frames*si.channels)) {
      logW("sample read size mismatch!");
    }
  }
  DivSample* sample=new DivSample;
  int sampleCount=(int)song.sample.size();
  sample->name=stripPath;

  int index=0;
  if ((si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_PCM_U8) {
    sample->depth=DIV_SAMPLE_DEPTH_8BIT;
  } else {
    sample->depth=DIV_SAMPLE_DEPTH_16BIT;
  }
  sample->init(si.frames);
  if ((si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_PCM_U8) {
    for (int i=0; i<si.frames*si.channels; i+=si.channels) {
      int averaged=0;
      for (int j=0; j<si.channels; j++) {
        averaged+=((int)((unsigned char*)buf)[i+j])-128;
      }
      averaged/=si.channels;
      sample->data8[index++]=averaged;
    }
    delete[] (unsigned char*)buf;
  } else if ((si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_FLOAT)  {
    for (int i=0; i<si.frames*si.channels; i+=si.channels) {
      float averaged=0.0f;
      for (int j=0; j<si.channels; j++) {
        averaged+=((float*)buf)[i+j];
      }
      averaged/=si.channels;
      averaged*=32767.0;
      if (averaged<-32768.0) averaged=-32768.0;
      if (averaged>32767.0) averaged=32767.0;
      sample->data16[index++]=averaged;
    }
    delete[] (float*)buf;
  } else {
    for (int i=0; i<si.frames*si.channels; i+=si.channels) {
      int averaged=0;
      for (int j=0; j<si.channels; j++) {
        averaged+=((short*)buf)[i+j];
      }
      averaged/=si.channels;
      sample->data16[index++]=averaged;
    }
    delete[] (short*)buf;
  }

  sample->rate=si.samplerate;
  if (sample->rate<4000) sample->rate=4000;
  if (sample->rate>96000) sample->rate=96000;
  sample->centerRate=si.samplerate;

  SF_INSTRUMENT inst;
  if (sf_command(f, SFC_GET_INSTRUMENT, &inst, sizeof(inst)) == SF_TRUE)
  {
    // There's no documentation on libsndfile detune range, but the code
    // implies -50..50. Yet when loading a file you can get a >50 value.
    if(inst.detune > 50)
      inst.detune = inst.detune - 100;
    short pitch = ((0x3c-inst.basenote)*100) + inst.detune;
    sample->centerRate=si.samplerate*pow(2.0,pitch/(12.0 * 100.0));
    if(inst.loop_count && inst.loops[0].mode >= SF_LOOP_FORWARD)
    {
      sample->loop=true;
      sample->loopMode=(DivSampleLoopMode)(inst.loops[0].mode-SF_LOOP_FORWARD);
      sample->loopStart=inst.loops[0].start;
      sample->loopEnd=inst.loops[0].end;
      if(inst.loops[0].end < (unsigned int)sampleCount)
        sampleCount=inst.loops[0].end;
    }
    else
      sample->loop=false;
  }

  if (sample->centerRate<4000) sample->centerRate=4000;
  if (sample->centerRate>64000) sample->centerRate=64000;
  sfWrap.doClose();
  BUSY_END;
  return sample;
#endif
}

DivSample* DivEngine::sampleFromFileRaw(const char* path, DivSampleDepth depth, int channels, bool bigEndian, bool unsign, bool swapNibbles) {
  if (song.sample.size()>=256) {
    lastError="too many samples!";
    return NULL;
  }
  if (channels<1) {
    lastError="invalid channel count";
    return NULL;
  }
  if (depth!=DIV_SAMPLE_DEPTH_8BIT && depth!=DIV_SAMPLE_DEPTH_16BIT) {
    if (channels!=1) {
      lastError="channel count has to be 1 for non-8/16-bit format";
      return NULL;
    }
  }
  BUSY_BEGIN;
  warnings="";

  const char* pathRedux=strrchr(path,DIR_SEPARATOR);
  if (pathRedux==NULL) {
    pathRedux=path;
  } else {
    pathRedux++;
  }
  String stripPath;
  const char* pathReduxEnd=strrchr(pathRedux,'.');
  if (pathReduxEnd==NULL) {
    stripPath=pathRedux;
  } else {
    for (const char* i=pathRedux; i!=pathReduxEnd && (*i); i++) {
      stripPath+=*i;
    }
  }

  size_t len=0;
  size_t lenDivided=0;
  DivSample* sample=new DivSample;
  sample->name=stripPath;

  FILE* f=ps_fopen(path,"rb");
  if (f==NULL) {
    BUSY_END;
    lastError=fmt::sprintf("could not open file! (%s)",strerror(errno));
    delete sample;
    return NULL;
  }

  if (fseek(f,0,SEEK_END)<0) {
    fclose(f);
    BUSY_END;
    lastError=fmt::sprintf("could not get file length! (%s)",strerror(errno));
    delete sample;
    return NULL;
  }

  len=ftell(f);

  if (len==0) {
    fclose(f);
    BUSY_END;
    lastError="file is empty!";
    delete sample;
    return NULL;
  }

  if (len==(SIZE_MAX>>1)) {
    fclose(f);
    BUSY_END;
    lastError="file is invalid!";
    delete sample;
    return NULL;
  }

  if (fseek(f,0,SEEK_SET)<0) {
    fclose(f);
    BUSY_END;
    lastError=fmt::sprintf("could not seek to beginning of file! (%s)",strerror(errno));
    delete sample;
    return NULL;
  }

  lenDivided=len/channels;

  unsigned int samples=lenDivided;
  switch (depth) {
    case DIV_SAMPLE_DEPTH_1BIT:
    case DIV_SAMPLE_DEPTH_1BIT_DPCM:
      samples=lenDivided*8;
      break;
    case DIV_SAMPLE_DEPTH_YMZ_ADPCM:
    case DIV_SAMPLE_DEPTH_QSOUND_ADPCM:
    case DIV_SAMPLE_DEPTH_ADPCM_A:
    case DIV_SAMPLE_DEPTH_ADPCM_B:
    case DIV_SAMPLE_DEPTH_VOX:
      samples=lenDivided*2;
      break;
    case DIV_SAMPLE_DEPTH_8BIT:
      samples=lenDivided;
      break;
    case DIV_SAMPLE_DEPTH_BRR:
      samples=16*((lenDivided+8)/9);
      break;
    case DIV_SAMPLE_DEPTH_16BIT:
      samples=(lenDivided+1)/2;
      break;
    default:
      break;
  }

  if (samples>16777215) {
    fclose(f);
    BUSY_END;
    lastError="this sample is too big! max sample size is 16777215.";
    delete sample;
    return NULL;
  }

  sample->rate=32000;
  sample->centerRate=32000;
  sample->depth=depth;
  sample->init(samples);

  unsigned char* buf=new unsigned char[len];
  if (fread(buf,1,len,f)==0) {
    fclose(f);
    BUSY_END;
    lastError=fmt::sprintf("could not read file! (%s)",strerror(errno));
    delete[] buf;
    delete sample;
    return NULL;
  }

  fclose(f);

  // import sample
  size_t pos=0;
  if (depth==DIV_SAMPLE_DEPTH_16BIT) {
    for (unsigned int i=0; i<samples; i++) {
      int accum=0;
      for (int j=0; j<channels; j++) {
        if (pos+1>=len) break;
        if (bigEndian) {
          accum+=(short)(((short)((buf[pos]<<8)|buf[pos+1]))^(unsign?0x8000:0));
        } else {
          accum+=(short)(((short)(buf[pos]|(buf[pos+1]<<8)))^(unsign?0x8000:0));
        }
        pos+=2;
      }
      accum/=channels;
      sample->data16[i]=accum;
    }
  } else if (depth==DIV_SAMPLE_DEPTH_8BIT) {
    for (unsigned int i=0; i<samples; i++) {
      int accum=0;
      for (int j=0; j<channels; j++) {
        if (pos>=len) break;
        accum+=(signed char)(buf[pos++]^(unsign?0x80:0));
      }
      accum/=channels;
      sample->data8[i]=accum;
    }
  } else {
    memcpy(sample->getCurBuf(),buf,len);
  }
  delete[] buf;

  // swap nibbles if needed
  if (swapNibbles) {
    unsigned char* b=(unsigned char*)sample->getCurBuf();
    for (unsigned int i=0; i<sample->getCurBufLen(); i++) {
      b[i]=(b[i]<<4)|(b[i]>>4);
    }
  }

  BUSY_END;
  return sample;
}

void DivEngine::delSample(int index) {
  BUSY_BEGIN;
  sPreview.sample=-1;
  sPreview.pos=0;
  sPreview.dir=false;
  saveLock.lock();
  if (index>=0 && index<(int)song.sample.size()) {
    delete song.sample[index];
    song.sample.erase(song.sample.begin()+index);
    song.sampleLen=song.sample.size();
    removeAsset(song.sampleDir,index);
    checkAssetDir(song.sampleDir,song.sample.size());

    // compensate
    for (DivInstrument* i: song.ins) {
      if (i->amiga.initSample==index) {
        i->amiga.initSample=-1;
      } else if (i->amiga.initSample>index) {
        i->amiga.initSample--;
      }
      for (int j=0; j<120; j++) {
        if (i->amiga.noteMap[j].map==index) {
          i->amiga.noteMap[j].map=-1;
        } else if (i->amiga.noteMap[j].map>index) {
          i->amiga.noteMap[j].map--;
        }
      }
    }

    renderSamples();
  }
  saveLock.unlock();
  BUSY_END;
}

void DivEngine::addOrder(int pos, bool duplicate, bool where) {
  unsigned char order[DIV_MAX_CHANS];
  if (curSubSong->ordersLen>=(DIV_MAX_PATTERNS-1)) return;
  memset(order,0,DIV_MAX_CHANS);
  BUSY_BEGIN_SOFT;
  if (duplicate) {
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      order[i]=curOrders->ord[i][pos];
    }
  } else {
    bool used[DIV_MAX_PATTERNS];
    for (int i=0; i<chans; i++) {
      memset(used,0,sizeof(bool)*DIV_MAX_PATTERNS);
      for (int j=0; j<curSubSong->ordersLen; j++) {
        used[curOrders->ord[i][j]]=true;
      }
      order[i]=(DIV_MAX_PATTERNS-1);
      for (int j=0; j<DIV_MAX_PATTERNS; j++) {
        if (!used[j]) {
          order[i]=j;
          break;
        }
      }
    }
  }
  if (where) { // at the end
    saveLock.lock();
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      curOrders->ord[i][curSubSong->ordersLen]=order[i];
    }
    curSubSong->ordersLen++;
    saveLock.unlock();
  } else { // after current order
    saveLock.lock();
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      for (int j=curSubSong->ordersLen; j>pos; j--) {
        curOrders->ord[i][j]=curOrders->ord[i][j-1];
      }
      curOrders->ord[i][pos+1]=order[i];
    }
    curSubSong->ordersLen++;
    saveLock.unlock();
    curOrder=pos+1;
    prevOrder=curOrder;
    if (playing && !freelance) {
      playSub(false);
    }
  }
  BUSY_END;
}

void DivEngine::deepCloneOrder(int pos, bool where) {
  unsigned char order[DIV_MAX_CHANS];
  if (curSubSong->ordersLen>=(DIV_MAX_PATTERNS-1)) return;
  warnings="";
  BUSY_BEGIN_SOFT;
  for (int i=0; i<chans; i++) {
    bool didNotFind=true;
    logD("channel %d",i);
    order[i]=curOrders->ord[i][pos];
    // find free slot
    for (int j=0; j<DIV_MAX_PATTERNS; j++) {
      logD("finding free slot in %d...",j);
      if (curPat[i].data[j]==NULL) {
        int origOrd=order[i];
        order[i]=j;
        DivPattern* oldPat=curPat[i].getPattern(origOrd,false);
        DivPattern* pat=curPat[i].getPattern(j,true);
        memcpy(pat->data,oldPat->data,DIV_MAX_ROWS*DIV_MAX_COLS*sizeof(short));
        logD("found at %d",j);
        didNotFind=false;
        break;
      }
    }
    if (didNotFind) {
      addWarning(fmt::sprintf("no free patterns in channel %d!",i));
    }
  }
  if (where) { // at the end
    saveLock.lock();
    for (int i=0; i<chans; i++) {
      curOrders->ord[i][curSubSong->ordersLen]=order[i];
    }
    curSubSong->ordersLen++;
    saveLock.unlock();
  } else { // after current order
    saveLock.lock();
    for (int i=0; i<chans; i++) {
      for (int j=curSubSong->ordersLen; j>pos; j--) {
        curOrders->ord[i][j]=curOrders->ord[i][j-1];
      }
      curOrders->ord[i][pos+1]=order[i];
    }
    curSubSong->ordersLen++;
    saveLock.unlock();
    if (pos<=curOrder) curOrder++;
    if (playing && !freelance) {
      playSub(false);
    }
  }
  BUSY_END;
}

void DivEngine::deleteOrder(int pos) {
  if (curSubSong->ordersLen<=1) return;
  BUSY_BEGIN_SOFT;
  saveLock.lock();
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    for (int j=pos; j<curSubSong->ordersLen; j++) {
      curOrders->ord[i][j]=curOrders->ord[i][j+1];
    }
  }
  curSubSong->ordersLen--;
  saveLock.unlock();
  if (curOrder>pos) curOrder--;
  if (curOrder>=curSubSong->ordersLen) curOrder=curSubSong->ordersLen-1;
  if (playing && !freelance) {
    playSub(false);
  }
  BUSY_END;
}

void DivEngine::moveOrderUp(int& pos) {
  BUSY_BEGIN_SOFT;
  if (pos<1) {
    BUSY_END;
    return;
  }
  saveLock.lock();
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    curOrders->ord[i][pos]^=curOrders->ord[i][pos-1];
    curOrders->ord[i][pos-1]^=curOrders->ord[i][pos];
    curOrders->ord[i][pos]^=curOrders->ord[i][pos-1];
  }
  saveLock.unlock();
  if (curOrder==pos) {
    curOrder--;
  }
  pos--;
  if (playing && !freelance) {
    playSub(false);
  }
  BUSY_END;
}

void DivEngine::moveOrderDown(int& pos) {
  BUSY_BEGIN_SOFT;
  if (pos>=curSubSong->ordersLen-1) {
    BUSY_END;
    return;
  }
  saveLock.lock();
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    curOrders->ord[i][pos]^=curOrders->ord[i][pos+1];
    curOrders->ord[i][pos+1]^=curOrders->ord[i][pos];
    curOrders->ord[i][pos]^=curOrders->ord[i][pos+1];
  }
  saveLock.unlock();
  if (curOrder==pos) {
    curOrder++;
  }
  pos++;
  if (playing && !freelance) {
    playSub(false);
  }
  BUSY_END;
}

void DivEngine::exchangeIns(int one, int two) {
  for (int i=0; i<chans; i++) {
    for (size_t j=0; j<song.subsong.size(); j++) {
      for (int k=0; k<DIV_MAX_PATTERNS; k++) {
        if (song.subsong[j]->pat[i].data[k]==NULL) continue;
        for (int l=0; l<song.subsong[j]->patLen; l++) {
          if (song.subsong[j]->pat[i].data[k]->data[l][2]==one) {
            song.subsong[j]->pat[i].data[k]->data[l][2]=two;
          } else if (song.subsong[j]->pat[i].data[k]->data[l][2]==two) {
            song.subsong[j]->pat[i].data[k]->data[l][2]=one;
          }
        }
      }
    }
  }
}

void DivEngine::exchangeWave(int one, int two) {
  // TODO
}

void DivEngine::exchangeSample(int one, int two) {
  for (DivInstrument* i: song.ins) {
    if (i->amiga.initSample==one) {
      i->amiga.initSample=two;
    } else if (i->amiga.initSample==two) {
      i->amiga.initSample=one;
    }
    for (int j=0; j<120; j++) {
      if (i->amiga.noteMap[j].map==one) {
        i->amiga.noteMap[j].map=two;
      } else if (i->amiga.noteMap[j].map==two) {
        i->amiga.noteMap[j].map=one;
      }
    }
  }
}

bool DivEngine::moveInsUp(int which) {
  if (which<1 || which>=(int)song.ins.size()) return false;
  BUSY_BEGIN;
  DivInstrument* prev=song.ins[which];
  saveLock.lock();
  song.ins[which]=song.ins[which-1];
  song.ins[which-1]=prev;
  moveAsset(song.insDir,which,which-1);
  exchangeIns(which,which-1);
  saveLock.unlock();
  BUSY_END;
  return true;
}

bool DivEngine::moveWaveUp(int which) {
  if (which<1 || which>=(int)song.wave.size()) return false;
  BUSY_BEGIN;
  DivWavetable* prev=song.wave[which];
  saveLock.lock();
  song.wave[which]=song.wave[which-1];
  song.wave[which-1]=prev;
  moveAsset(song.waveDir,which,which-1);
  exchangeWave(which,which-1);
  saveLock.unlock();
  BUSY_END;
  return true;
}

bool DivEngine::moveSampleUp(int which) {
  if (which<1 || which>=(int)song.sample.size()) return false;
  BUSY_BEGIN;
  sPreview.sample=-1;
  sPreview.pos=0;
  sPreview.dir=false;
  DivSample* prev=song.sample[which];
  saveLock.lock();
  song.sample[which]=song.sample[which-1];
  song.sample[which-1]=prev;
  moveAsset(song.sampleDir,which,which-1);
  exchangeSample(which,which-1);
  saveLock.unlock();
  renderSamples();
  BUSY_END;
  return true;
}

bool DivEngine::moveInsDown(int which) {
  if (which<0 || which>=((int)song.ins.size())-1) return false;
  BUSY_BEGIN;
  DivInstrument* prev=song.ins[which];
  saveLock.lock();
  song.ins[which]=song.ins[which+1];
  song.ins[which+1]=prev;
  exchangeIns(which,which+1);
  moveAsset(song.insDir,which,which+1);
  saveLock.unlock();
  BUSY_END;
  return true;
}

bool DivEngine::moveWaveDown(int which) {
  if (which<0 || which>=((int)song.wave.size())-1) return false;
  BUSY_BEGIN;
  DivWavetable* prev=song.wave[which];
  saveLock.lock();
  song.wave[which]=song.wave[which+1];
  song.wave[which+1]=prev;
  exchangeWave(which,which+1);
  moveAsset(song.waveDir,which,which+1);
  saveLock.unlock();
  BUSY_END;
  return true;
}

bool DivEngine::moveSampleDown(int which) {
  if (which<0 || which>=((int)song.sample.size())-1) return false;
  BUSY_BEGIN;
  sPreview.sample=-1;
  sPreview.pos=0;
  sPreview.dir=false;
  DivSample* prev=song.sample[which];
  saveLock.lock();
  song.sample[which]=song.sample[which+1];
  song.sample[which+1]=prev;
  exchangeSample(which,which+1);
  moveAsset(song.sampleDir,which,which+1);
  saveLock.unlock();
  renderSamples();
  BUSY_END;
  return true;
}

void DivEngine::autoPatchbay() {
  song.patchbay.clear();
  for (unsigned int i=0; i<song.systemLen; i++) {
    if (disCont[i].dispatch==NULL) continue;

    unsigned int outs=disCont[i].dispatch->getOutputCount();
    if (outs>16) outs=16;
    if (outs<2) {
      for (unsigned int j=0; j<DIV_MAX_OUTPUTS; j++) {
        song.patchbay.push_back((i<<20)|j);
      }
    } else {
      for (unsigned int j=0; j<outs; j++) {

        song.patchbay.push_back((i<<20)|(j<<16)|j);
      }
    }
  }

  // wave/sample preview
  for (unsigned int j=0; j<DIV_MAX_OUTPUTS; j++) {
    song.patchbay.push_back(0xffd00000|j);
  }

  // metronome
  for (unsigned int j=0; j<DIV_MAX_OUTPUTS; j++) {
    song.patchbay.push_back(0xffe00000|j);
  }
}

void DivEngine::autoPatchbayP() {
  BUSY_BEGIN;
  saveLock.lock();
  autoPatchbay();
  saveLock.unlock();
  BUSY_END;
}

void DivEngine::recalcPatchbay() {

}

bool DivEngine::patchConnect(unsigned int src, unsigned int dest) {
  unsigned int armed=(src<<16)|(dest&0xffff);
  for (unsigned int i: song.patchbay) {
    if (i==armed) return false;
  }
  BUSY_BEGIN;
  saveLock.lock();
  song.patchbay.push_back(armed);
  song.patchbayAuto=false;
  saveLock.unlock();
  BUSY_END;
  return true;
}

bool DivEngine::patchDisconnect(unsigned int src, unsigned int dest) {
  unsigned int armed=(src<<16)|(dest&0xffff);
  for (auto i=song.patchbay.begin(); i!=song.patchbay.end(); i++) {
    if (*i==armed) {
      BUSY_BEGIN;
      saveLock.lock();
      song.patchbay.erase(i);
      song.patchbayAuto=false;
      saveLock.unlock();
      BUSY_END;
      return true;
    }
  }
  return false;
}

void DivEngine::patchDisconnectAll(unsigned int portSet) {
  BUSY_BEGIN;
  saveLock.lock();

  if (portSet&0x1000) {
    portSet&=0xfff;

    for (size_t i=0; i<song.patchbay.size(); i++) {
      if ((song.patchbay[i]&0xfff0)==(portSet<<4)) {
        song.patchbay.erase(song.patchbay.begin()+i);
        i--;
      }
    }
  } else {
    portSet&=0xfff;

    for (size_t i=0; i<song.patchbay.size(); i++) {
      if ((song.patchbay[i]&0xfff00000)==(portSet<<20)) {
        song.patchbay.erase(song.patchbay.begin()+i);
        i--;
      }
    }
  }

  saveLock.unlock();
  BUSY_END;
}

void DivEngine::noteOn(int chan, int ins, int note, int vol) {
  if (chan<0 || chan>=chans) return;
  BUSY_BEGIN;
  pendingNotes.push_back(DivNoteEvent(chan,ins,note,vol,true));
  if (!playing) {
    reset();
    freelance=true;
    playing=true;
  }
  BUSY_END;
}

void DivEngine::noteOff(int chan) {
  if (chan<0 || chan>=chans) return;
  BUSY_BEGIN;
  pendingNotes.push_back(DivNoteEvent(chan,-1,-1,-1,false));
  if (!playing) {
    reset();
    freelance=true;
    playing=true;
  }
  BUSY_END;
}

void DivEngine::autoNoteOn(int ch, int ins, int note, int vol) {
  bool isViable[DIV_MAX_CHANS];
  bool canPlayAnyway=false;
  bool notInViableChannel=false;
  if (midiBaseChan<0) midiBaseChan=0;
  if (midiBaseChan>=chans) midiBaseChan=chans-1;
  int finalChan=midiBaseChan;
  int finalChanType=getChannelType(finalChan);

  if (!playing) {
    reset();
    freelance=true;
    playing=true;
  }

  // 1. check which channels are viable for this instrument
  DivInstrument* insInst=getIns(ins);
  if (getPreferInsType(finalChan)!=insInst->type && getPreferInsSecondType(finalChan)!=insInst->type && getPreferInsType(finalChan)!=DIV_INS_NULL) notInViableChannel=true;
  for (int i=0; i<chans; i++) {
    if (ins==-1 || ins>=song.insLen || getPreferInsType(i)==insInst->type || (getPreferInsType(i)==DIV_INS_NULL && finalChanType==DIV_CH_NOISE) || getPreferInsSecondType(i)==insInst->type) {
      if (insInst->type==DIV_INS_OPL) {
        if (insInst->fm.ops==2 || getChannelType(i)==DIV_CH_OP) {
          isViable[i]=true;
          canPlayAnyway=true;
        } else {
          isViable[i]=false;
        }
      } else {
        isViable[i]=true;
        canPlayAnyway=true;
      }
    } else {
      isViable[i]=false;
    }
  }

  if (!canPlayAnyway) return;

  // 2. find a free channel
  do {
    if ((!midiPoly) || (isViable[finalChan] && chan[finalChan].midiNote==-1 && (insInst->type==DIV_INS_OPL || getChannelType(finalChan)==finalChanType || notInViableChannel))) {
      chan[finalChan].midiNote=note;
      chan[finalChan].midiAge=midiAgeCounter++;
      pendingNotes.push_back(DivNoteEvent(finalChan,ins,note,vol,true));
      return;
    }
    if (++finalChan>=chans) {
      finalChan=0;
    }
  } while (finalChan!=midiBaseChan);

  // 3. find the oldest channel
  int candidate=finalChan;
  do {
    if (isViable[finalChan] && (insInst->type==DIV_INS_OPL || getChannelType(finalChan)==finalChanType || notInViableChannel) && chan[finalChan].midiAge<chan[candidate].midiAge) {
      candidate=finalChan;
    }
    if (++finalChan>=chans) {
      finalChan=0;
    }
  } while (finalChan!=midiBaseChan);

  chan[candidate].midiNote=note;
  chan[candidate].midiAge=midiAgeCounter++;
  pendingNotes.push_back(DivNoteEvent(candidate,ins,note,vol,true));
}

void DivEngine::autoNoteOff(int ch, int note, int vol) {
  if (!playing) {
    return;
  }
  //if (ch<0 || ch>=chans) return;
  for (int i=0; i<chans; i++) {
    if (chan[i].midiNote==note) {
      pendingNotes.push_back(DivNoteEvent(i,-1,-1,-1,false));
      chan[i].midiNote=-1;
    }
  }
}

void DivEngine::autoNoteOffAll() {
  if (!playing) {
    return;
  }
  for (int i=0; i<chans; i++) {
    if (chan[i].midiNote!=-1) {
      pendingNotes.push_back(DivNoteEvent(i,-1,-1,-1,false));
      chan[i].midiNote=-1;
    }
  }
}

void DivEngine::setAutoNotePoly(bool poly) {
  midiPoly=poly;
}

void DivEngine::setOrder(unsigned char order) {
  BUSY_BEGIN_SOFT;
  curOrder=order;
  if (order>=curSubSong->ordersLen) curOrder=0;
  prevOrder=curOrder;
  if (playing && !freelance) {
    playSub(false);
  }
  BUSY_END;
}

void DivEngine::updateSysFlags(int system, bool restart) {
  BUSY_BEGIN_SOFT;
  disCont[system].dispatch->setFlags(song.systemFlags[system]);
  disCont[system].setRates(got.rate);

  // patchbay
  if (song.patchbayAuto) {
    saveLock.lock();
    autoPatchbay();
    saveLock.unlock();
  }

  if (restart && isPlaying()) {
    playSub(false);
  }
  BUSY_END;
}

void DivEngine::setSongRate(float hz) {
  BUSY_BEGIN;
  saveLock.lock();
  curSubSong->hz=hz;
  divider=curSubSong->hz;
  saveLock.unlock();
  BUSY_END;
}

void DivEngine::setAudio(DivAudioEngines which) {
  audioEngine=which;
}

void DivEngine::setView(DivStatusView which) {
  view=which;
}

bool DivEngine::getMetronome() {
  return metronome;
}

void DivEngine::setMetronome(bool enable) {
  metronome=enable;
  metroAmp=0;
}

void DivEngine::setMetronomeVol(float vol) {
  metroVol=vol;
}

void DivEngine::setConsoleMode(bool enable) {
  consoleMode=enable;
}

bool DivEngine::switchMaster(bool full) {
  logI("switching output...");
  deinitAudioBackend(true);
  if (full) {
    quitDispatch();
    initDispatch();
  }
  if (initAudioBackend()) {
    for (int i=0; i<song.systemLen; i++) {
      disCont[i].setRates(got.rate);
      disCont[i].setQuality(lowQuality);
    }
    if (!output->setRun(true)) {
      logE("error while activating audio!");
      return false;
    }
  } else {
    return false;
  }
  renderSamples();
  return true;
}

void DivEngine::setMidiBaseChan(int chan) {
  if (chan<0 || chan>=chans) chan=0;
  midiBaseChan=chan;
}

void DivEngine::setMidiDirect(bool value) {
  midiIsDirect=value;
}

void DivEngine::setMidiCallback(std::function<int(const TAMidiMessage&)> what) {
  midiCallback=what;
}

bool DivEngine::sendMidiMessage(TAMidiMessage& msg) {
  if (output==NULL) {
    logW("output is NULL!");
    return false;
  }
  if (output->midiOut==NULL) {
    logW("MIDI output is NULL!");
    return false;
  }
  BUSY_BEGIN;
  logD("sending MIDI message...");
  bool ret=(output->midiOut->send(msg));
  BUSY_END;
  return ret;
}

void DivEngine::synchronized(const std::function<void()>& what) {
  BUSY_BEGIN;
  what();
  BUSY_END;
}

void DivEngine::lockSave(const std::function<void()>& what) {
  saveLock.lock();
  what();
  saveLock.unlock();
}

void DivEngine::lockEngine(const std::function<void()>& what) {
  BUSY_BEGIN;
  saveLock.lock();
  what();
  saveLock.unlock();
  BUSY_END;
}

TAAudioDesc& DivEngine::getAudioDescWant() {
  return want;
}

TAAudioDesc& DivEngine::getAudioDescGot() {
  return got;
}

std::vector<String>& DivEngine::getAudioDevices() {
  return audioDevs;
}

std::vector<String>& DivEngine::getMidiIns() {
  return midiIns;
}

std::vector<String>& DivEngine::getMidiOuts() {
  return midiOuts;
}

void DivEngine::rescanAudioDevices() {
  audioDevs.clear();
  if (output!=NULL) {
    audioDevs=output->listAudioDevices();
    if (output->midiIn!=NULL) {
      midiIns=output->midiIn->listDevices();
    }
    if (output->midiOut!=NULL) {
      midiOuts=output->midiOut->listDevices();
    }
  }
}

void DivEngine::initDispatch() {
  BUSY_BEGIN;
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].init(song.system[i],this,getChannelCount(song.system[i]),got.rate,song.systemFlags[i]);
    disCont[i].setRates(got.rate);
    disCont[i].setQuality(lowQuality);
  }
  if (song.patchbayAuto) {
    saveLock.lock();
    autoPatchbay();
    saveLock.unlock();
  }
  recalcChans();
  BUSY_END;
}

void DivEngine::quitDispatch() {
  BUSY_BEGIN;
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].quit();
  }
  cycles=0;
  clockDrift=0;
  midiClockCycles=0;
  midiClockDrift=0;
  midiTimeCycles=0;
  midiTimeDrift=0;
  chans=0;
  playing=false;
  curSpeed=0;
  endOfSong=false;
  ticks=0;
  tempoAccum=0;
  curRow=0;
  curOrder=0;
  prevRow=0;
  prevOrder=0;
  nextSpeed=3;
  changeOrd=-1;
  changePos=0;
  totalTicks=0;
  totalSeconds=0;
  totalTicksR=0;
  curMidiClock=0;
  curMidiTime=0;
  curMidiTimeCode=0;
  curMidiTimePiece=0;
  totalCmds=0;
  lastCmds=0;
  cmdsPerSecond=0;
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    isMuted[i]=0;
  }
  BUSY_END;
}

bool DivEngine::initAudioBackend() {
  // load values
  logI("initializing audio.");
  if (audioEngine==DIV_AUDIO_NULL) {
    if (getConfString("audioEngine","SDL")=="JACK") {
      audioEngine=DIV_AUDIO_JACK;
    } else {
      audioEngine=DIV_AUDIO_SDL;
    }
  }

#ifdef HAVE_SDL2
  if (audioEngine==DIV_AUDIO_SDL) {
    String audioDriver=getConfString("sdlAudioDriver","");
    if (!audioDriver.empty()) {
      SDL_SetHint("SDL_HINT_AUDIODRIVER",audioDriver.c_str());
    }
  }
#endif

  lowQuality=getConfInt("audioQuality",0);
  forceMono=getConfInt("forceMono",0);
  clampSamples=getConfInt("clampSamples",0);
  lowLatency=getConfInt("lowLatency",0);
  metroVol=(float)(getConfInt("metroVol",100))/100.0f;
  midiOutClock=getConfInt("midiOutClock",0);
  midiOutTime=getConfInt("midiOutTime",0);
  midiOutTimeRate=getConfInt("midiOutTimeRate",0);
  midiOutProgramChange=getConfInt("midiOutProgramChange",0);
  midiOutMode=getConfInt("midiOutMode",DIV_MIDI_MODE_NOTE);
  if (metroVol<0.0f) metroVol=0.0f;
  if (metroVol>2.0f) metroVol=2.0f;

  if (lowLatency) logI("using low latency mode.");

  switch (audioEngine) {
    case DIV_AUDIO_JACK:
#ifndef HAVE_JACK
      logE("Furnace was not compiled with JACK support!");
      setConf("audioEngine","SDL");
      saveConf();
#ifdef HAVE_SDL2
      output=new TAAudioSDL;
#else
      logE("Furnace was not compiled with SDL support either!");
      output=new TAAudio;
#endif
#else
      output=new TAAudioJACK;
#endif
      break;
    case DIV_AUDIO_SDL:
#ifdef HAVE_SDL2
      output=new TAAudioSDL;
#else
      logE("Furnace was not compiled with SDL support!");
      output=new TAAudio;
#endif
      break;
    case DIV_AUDIO_DUMMY:
      output=new TAAudio;
      break;
    default:
      logE("invalid audio engine!");
      return false;
  }

  audioDevs=output->listAudioDevices();

  want.deviceName=getConfString("audioDevice","");
  want.bufsize=getConfInt("audioBufSize",1024);
  want.rate=getConfInt("audioRate",44100);
  want.fragments=2;
  want.inChans=0;
  want.outChans=getConfInt("audioChans",2);
  want.outFormat=TA_AUDIO_FORMAT_F32;
  want.name="Furnace";

  if (want.outChans<1) want.outChans=1;
  if (want.outChans>16) want.outChans=16;

  output->setCallback(process,this);

  if (!output->init(want,got)) {
    logE("error while initializing audio!");
    delete output;
    output=NULL;
    audioEngine=DIV_AUDIO_NULL;
    return false;
  }

  for (int i=0; i<got.outChans; i++) {
    if (oscBuf[i]==NULL) {
      oscBuf[i]=new float[32768];
    }
    memset(oscBuf[i],0,32768*sizeof(float));
  }

  if (output->initMidi(false)) {
    midiIns=output->midiIn->listDevices();
    midiOuts=output->midiOut->listDevices();
  } else {
    logW("error while initializing MIDI!");
  }
  if (output->midiIn) {
    String inName=getConfString("midiInDevice","");
    if (!inName.empty()) {
      // try opening device
      logI("opening MIDI input.");
      if (!output->midiIn->openDevice(inName)) {
        logW("could not open MIDI input device!");
      }
    } else {
      logV("no MIDI input device selected.");
    }
  }
  if (output->midiOut) {
    String outName=getConfString("midiOutDevice","");
    if (!outName.empty()) {
      // try opening device
      logI("opening MIDI output.");
      if (!output->midiOut->openDevice(outName)) {
        logW("could not open MIDI output device!");
      }
    } else {
      logV("no MIDI output device selected.");
    }
  }

  return true;
}

bool DivEngine::deinitAudioBackend(bool dueToSwitchMaster) {
  if (output!=NULL) {
    logI("closing audio output.");
    output->quit();
    if (output->midiIn) {
      if (output->midiIn->isDeviceOpen()) {
        logI("closing MIDI input.");
        output->midiIn->closeDevice();
      }
    }
    if (output->midiOut) {
      if (output->midiOut->isDeviceOpen()) {
        logI("closing MIDI output.");
        output->midiOut->closeDevice();
      }
    }
    output->quitMidi();
    delete output;
    output=NULL;
    if (dueToSwitchMaster) {
      audioEngine=DIV_AUDIO_NULL;
    }
  }
  return true;
}

void DivEngine::preInit() {
  // register systems
  if (!systemsRegistered) registerSystems();

  // init config
  initConfDir();
  logD("config path: %s",configPath.c_str());

  String logPath=configPath+DIR_SEPARATOR_STR+"furnace.log";
  startLogFile(logPath.c_str());

  logI("Furnace version " DIV_VERSION ".");
  
  loadConf();
}

bool DivEngine::init() {
  loadSampleROMs();

  // set default system preset
  if (!hasLoadedSomething) {
    logD("setting default preset");
    String preset=getConfString("initialSys2","");
    bool oldVol=getConfInt("configVersion",DIV_ENGINE_VERSION)<135;
    if (preset.empty()) {
      // try loading old preset
      logD("trying to load old preset");
      preset=decodeSysDesc(getConfString("initialSys",""));
      oldVol=false;
    }
    logD("preset size %ld",preset.size());
    if (preset.size()>0 && (preset.size()&3)==0) {
      initSongWithDesc(preset.c_str(),true,oldVol);
    }
    String sysName=getConfString("initialSysName","");
    if (sysName=="") {
      song.systemName=getSongSystemLegacyName(song,!getConfInt("noMultiSystem",0));
    } else {
      song.systemName=sysName;
    }
    hasLoadedSomething=true;
  }

  // init the rest of engine
  bool haveAudio=false;
  if (!initAudioBackend()) {
    logE("no audio output available!");
  } else {
    haveAudio=true;
  }

  samp_bb=blip_new(32768);
  if (samp_bb==NULL) {
    logE("not enough memory!");
    return false;
  }

  samp_bbOut=new short[32768];

  samp_bbIn=new short[32768];
  samp_bbInLen=32768;

  metroBuf=new float[8192];
  metroBufLen=8192;
  
  blip_set_rates(samp_bb,44100,got.rate);

  for (int i=0; i<64; i++) {
    vibTable[i]=127*sin(((double)i/64.0)*(2*M_PI));
  }
  for (int i=0; i<128; i++) {
    tremTable[i]=255*0.5*(1.0-cos(((double)i/128.0)*(2*M_PI)));
  }
  for (int i=0; i<4096; i++) {
    reversePitchTable[i]=round(1024.0*pow(2.0,(2048.0-(double)i)/(12.0*128.0)));
    pitchTable[i]=round(1024.0*pow(2.0,((double)i-2048.0)/(12.0*128.0)));
  }

  for (int i=0; i<DIV_MAX_CHANS; i++) {
    isMuted[i]=0;
    keyHit[i]=false;
  }

  initDispatch();
  renderSamples();
  reset();
  active=true;

  if (!haveAudio) {
    return false;
  } else {
    if (output==NULL) {
      logE("output is NULL!");
      return false;
    }
    if (!output->setRun(true)) {
      logE("error while activating!");
      return false;
    }
  }
  return true;
}

bool DivEngine::quit() {
  deinitAudioBackend();
  quitDispatch();
  logI("saving config.");
  saveConf();
  active=false;
  for (int i=0; i<DIV_MAX_OUTPUTS; i++) {
    if (oscBuf[i]!=NULL) delete[] oscBuf[i];
  }
  if (metroBuf!=NULL) {
    delete[] metroBuf;
    metroBuf=NULL;
    metroBufLen=0;
  }
  if (yrw801ROM!=NULL) delete[] yrw801ROM;
  if (tg100ROM!=NULL) delete[] tg100ROM;
  if (mu5ROM!=NULL) delete[] mu5ROM;
  song.unload();
  return true;
}
