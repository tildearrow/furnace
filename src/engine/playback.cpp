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

#include "macroInt.h"
#include <chrono>
#define _USE_MATH_DEFINES
#include "dispatch.h"
#include "engine.h"
#include "../ta-log.h"
#include <math.h>

constexpr int MASTER_CLOCK_PREC=(sizeof(void*)==8)?8:0;

void DivEngine::nextOrder() {
  curRow=0;
  if (repeatPattern) return;
  if (++curOrder>=curSubSong->ordersLen) {
    logV("end of orders reached");
    endOfSong=true;
    memset(walked,0,8192);
    curOrder=0;
  }
}

const char* notes[12]={
  "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
};

// update this when adding new commands.
const char* cmdName[]={
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

  "HINT_VIBRATO",
  "HINT_VIBRATO_RANGE",
  "HINT_VIBRATO_SHAPE",
  "HINT_PITCH",
  "HINT_ARPEGGIO",
  "HINT_VOLUME",
  "HINT_VOL_SLIDE",
  "HINT_PORTA",
  "HINT_LEGATO",

  "SAMPLE_MODE",
  "SAMPLE_FREQ",
  "SAMPLE_BANK",
  "SAMPLE_POS",
  "SAMPLE_DIR",

  "FM_HARD_RESET",
  "FM_LFO",
  "FM_LFO_WAVE",
  "FM_TL",
  "FM_AM",
  "FM_AR",
  "FM_DR",
  "FM_SL",
  "FM_D2R",
  "FM_RR",
  "FM_DT",
  "FM_DT2",
  "FM_RS",
  "FM_KSR",
  "FM_VIB",
  "FM_SUS",
  "FM_WS",
  "FM_SSG",
  "FM_REV",
  "FM_EG_SHIFT",
  "FM_FB",
  "FM_MULT",
  "FM_FINE",
  "FM_FIXFREQ",
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
  "NES_DMC",

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
  "AY_IO_WRITE",
  "AY_AUTO_PWM",

  "FDS_MOD_DEPTH",
  "FDS_MOD_HIGH",
  "FDS_MOD_LOW",
  "FDS_MOD_POS",
  "FDS_MOD_WAVE",

  "SAA_ENVELOPE",

  "AMIGA_FILTER",
  "AMIGA_AM",
  "AMIGA_PM",
  
  "LYNX_LFSR_LOAD",

  "QSOUND_ECHO_FEEDBACK",
  "QSOUND_ECHO_DELAY",
  "QSOUND_ECHO_LEVEL",
  "QSOUND_SURROUND",

  "X1_010_ENVELOPE_SHAPE",
  "X1_010_ENVELOPE_ENABLE",
  "X1_010_ENVELOPE_MODE",
  "X1_010_ENVELOPE_PERIOD",
  "X1_010_ENVELOPE_SLIDE",
  "X1_010_AUTO_ENVELOPE",
  "X1_010_SAMPLE_BANK_SLOT",

  "WS_SWEEP_TIME",
  "WS_SWEEP_AMOUNT",

  "N163_WAVE_POSITION",
  "N163_WAVE_LENGTH",
  "N163_WAVE_MODE",
  "N163_WAVE_LOAD",
  "N163_WAVE_LOADPOS",
  "N163_WAVE_LOADLEN",
  "N163_WAVE_LOADMODE",
  "N163_CHANNEL_LIMIT",
  "N163_GLOBAL_WAVE_LOAD",
  "N163_GLOBAL_WAVE_LOADPOS",
  "N163_GLOBAL_WAVE_LOADLEN",
  "N163_GLOBAL_WAVE_LOADMODE",

  "SU_SWEEP_PERIOD_LOW",
  "SU_SWEEP_PERIOD_HIGH",
  "SU_SWEEP_BOUND",
  "SU_SWEEP_ENABLE",
  "SU_SYNC_PERIOD_LOW",
  "SU_SYNC_PERIOD_HIGH",

  "ADPCMA_GLOBAL_VOLUME",

  "SNES_ECHO",
  "SNES_PITCH_MOD",
  "SNES_INVERT",
  "SNES_GAIN_MODE",
  "SNES_GAIN",
  "SNES_ECHO_ENABLE",
  "SNES_ECHO_DELAY",
  "SNES_ECHO_VOL_LEFT",
  "SNES_ECHO_VOL_RIGHT",
  "SNES_ECHO_FEEDBACK",
  "SNES_ECHO_FIR",

  "NES_ENV_MODE",
  "NES_LENGTH",
  "NES_COUNT_MODE",

  "MACRO_OFF",
  "MACRO_ON",

  "ALWAYS_SET_VOLUME"
};

static_assert((sizeof(cmdName)/sizeof(void*))==DIV_CMD_MAX,"update cmdName!");

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
    if (!skipping) {
      switch (c.cmd) {
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
          printf("%8d | %d: %s(%d, %d)\n",totalTicksR,c.chan,cmdName[c.cmd],c.value,c.value2);
      }
    }
  }
  totalCmds++;
  if (cmdStreamEnabled && cmdStream.size()<2000) {
    cmdStream.push_back(c);
  }

  if (output) if (!skipping && output->midiOut!=NULL) {
    if (output->midiOut->isDeviceOpen()) {
      if (midiOutMode==DIV_MIDI_MODE_NOTE) {
        int scaledVol=(chan[c.chan].volume*127)/MAX(1,chan[c.chan].volMax);
        if (scaledVol<0) scaledVol=0;
        if (scaledVol>127) scaledVol=127;
        switch (c.cmd) {
          case DIV_CMD_NOTE_ON:
          case DIV_CMD_LEGATO:
            if (chan[c.chan].curMidiNote>=0) {
              output->midiOut->send(TAMidiMessage(0x80|(c.chan&15),chan[c.chan].curMidiNote,scaledVol));
            }
            if (c.value!=DIV_NOTE_NULL) {
              chan[c.chan].curMidiNote=c.value+12;
              if (chan[c.chan].curMidiNote<0) chan[c.chan].curMidiNote=0;
              if (chan[c.chan].curMidiNote>127) chan[c.chan].curMidiNote=127;
            }
            output->midiOut->send(TAMidiMessage(0x90|(c.chan&15),chan[c.chan].curMidiNote,scaledVol));
            break;
          case DIV_CMD_NOTE_OFF:
          case DIV_CMD_NOTE_OFF_ENV:
            if (chan[c.chan].curMidiNote>=0) {
              output->midiOut->send(TAMidiMessage(0x80|(c.chan&15),chan[c.chan].curMidiNote,scaledVol));
            }
            chan[c.chan].curMidiNote=-1;
            break;
          case DIV_CMD_INSTRUMENT:
            if (chan[c.chan].lastIns!=c.value) {
              output->midiOut->send(TAMidiMessage(0xc0|(c.chan&15),c.value,0));
            }
            break;
          case DIV_CMD_VOLUME:
            if (chan[c.chan].curMidiNote>=0 && chan[c.chan].midiAftertouch) {
              chan[c.chan].midiAftertouch=false;
              output->midiOut->send(TAMidiMessage(0xa0|(c.chan&15),chan[c.chan].curMidiNote,scaledVol));
            }
            break;
          case DIV_CMD_PITCH: {
            int pitchBend=8192+(c.value<<5);
            if (pitchBend<0) pitchBend=0;
            if (pitchBend>16383) pitchBend=16383;
            if (pitchBend!=chan[c.chan].midiPitch) {
              chan[c.chan].midiPitch=pitchBend;
              output->midiOut->send(TAMidiMessage(0xe0|(c.chan&15),pitchBend&0x7f,pitchBend>>7));
            }
            break;
          }
          case DIV_CMD_PANNING: {
            int pan=convertPanSplitToLinearLR(c.value,c.value2,127);
            output->midiOut->send(TAMidiMessage(0xb0|(c.chan&15),0x0a,pan));
            break;
          }
          case DIV_CMD_HINT_PORTA: {
            if (c.value2>0) {
              if (c.value<=0 || c.value>=255) break;
              //output->midiOut->send(TAMidiMessage(0x80|(c.chan&15),chan[c.chan].curMidiNote,scaledVol));
              int target=c.value+12;
              if (target<0) target=0;
              if (target>127) target=127;
              
              if (chan[c.chan].curMidiNote>=0) {
                output->midiOut->send(TAMidiMessage(0xb0|(c.chan&15),0x54,chan[c.chan].curMidiNote));
              }
              output->midiOut->send(TAMidiMessage(0xb0|(c.chan&15),0x05,1/*MIN(0x7f,c.value2/4)*/));
              output->midiOut->send(TAMidiMessage(0xb0|(c.chan&15),0x41,0x7f));
              
              output->midiOut->send(TAMidiMessage(0x90|(c.chan&15),target,scaledVol));
            } else {
              output->midiOut->send(TAMidiMessage(0xb0|(c.chan&15),0x41,0));
            }
            break;
          }
          default:
            break;
        }
      }
    }
  }

  c.chan=dispatchChanOfChan[c.dis];

  return disCont[dispatchOfChan[c.dis]].dispatch->dispatch(c);
}

bool DivEngine::perSystemEffect(int ch, unsigned char effect, unsigned char effectVal) {
  DivSysDef* sysDef=sysDefs[sysOfChan[ch]];
  if (sysDef==NULL) return false;
  auto iter=sysDef->effectHandlers.find(effect);
  if (iter==sysDef->effectHandlers.end()) return false;
  EffectHandler handler=iter->second;
  int val=0;
  int val2=0;
  try {
    val=handler.val?handler.val(effect,effectVal):effectVal;
    val2=handler.val2?handler.val2(effect,effectVal):0;
  } catch (DivDoNotHandleEffect& e) {
    return false;
  }
  // wouldn't this cause problems if it were to return 0?
  return dispatchCmd(DivCommand(handler.dispatchCmd,ch,val,val2));
}

bool DivEngine::perSystemPostEffect(int ch, unsigned char effect, unsigned char effectVal) {
  DivSysDef* sysDef=sysDefs[sysOfChan[ch]];
  if (sysDef==NULL) return false;
  auto iter=sysDef->postEffectHandlers.find(effect);
  if (iter==sysDef->postEffectHandlers.end()) return false;
  EffectHandler handler=iter->second;
  int val=0;
  int val2=0;
  try {
    val=handler.val?handler.val(effect,effectVal):effectVal;
    val2=handler.val2?handler.val2(effect,effectVal):0;
  } catch (DivDoNotHandleEffect& e) {
    return true;
  }
  // wouldn't this cause problems if it were to return 0?
  return dispatchCmd(DivCommand(handler.dispatchCmd,ch,val,val2));
}

void DivEngine::processRow(int i, bool afterDelay) {
  int whatOrder=afterDelay?chan[i].delayOrder:curOrder;
  int whatRow=afterDelay?chan[i].delayRow:curRow;
  DivPattern* pat=curPat[i].getPattern(curOrders->ord[i][whatOrder],false);
  // pre effects
  if (!afterDelay) {
    bool returnAfterPre=false;
    for (int j=0; j<curPat[i].effectCols; j++) {
      short effect=pat->data[whatRow][4+(j<<1)];
      short effectVal=pat->data[whatRow][5+(j<<1)];

      if (effectVal==-1) effectVal=0;

      switch (effect) {
        case 0x09: // speed 1
          if (effectVal>0) speed1=effectVal;
          break;
        case 0x0f: // speed 2
          if (effectVal>0) speed2=effectVal;
          break;
        case 0x0b: // change order
          if (changeOrd==-1 || song.jumpTreatment==0) {
            changeOrd=effectVal;
            if (song.jumpTreatment==1 || song.jumpTreatment==2) {
              changePos=0;
            }
          }
          break;
        case 0x0d: // next order
          if (song.jumpTreatment==2) {
            if ((curOrder<(curSubSong->ordersLen-1) || !song.ignoreJumpAtEnd)) {
              changeOrd=-2;
              changePos=effectVal;
            }
          } else if (song.jumpTreatment==1) {
            if (changeOrd<0 && (curOrder<(curSubSong->ordersLen-1) || !song.ignoreJumpAtEnd)) {
              changeOrd=-2;
              changePos=effectVal;
            }
          } else {
            if (curOrder<(curSubSong->ordersLen-1) || !song.ignoreJumpAtEnd) {
              if (changeOrd<0) {
                changeOrd=-2;
              }
              changePos=effectVal;
            }
          }
          break;
        case 0xed: // delay
          if (effectVal!=0) {
            bool comparison=(song.delayBehavior==1)?(effectVal<=nextSpeed):(effectVal<(nextSpeed*(curSubSong->timeBase+1)));
            if (song.delayBehavior==2) comparison=true;
            if (comparison) {
              chan[i].rowDelay=effectVal+1;
              chan[i].delayOrder=whatOrder;
              chan[i].delayRow=whatRow;
              if (effectVal==nextSpeed) {
                //if (sysOfChan[i]!=DIV_SYSTEM_YM2610 && sysOfChan[i]!=DIV_SYSTEM_YM2610_EXT) chan[i].delayLocked=true;
              } else {
                chan[i].delayLocked=false;
              }
              returnAfterPre=true;
            } else {
              logV("higher than nextSpeed! %d>%d",effectVal,nextSpeed);
              chan[i].delayLocked=false;
            }
          }
          break;
      }
    }
    if (returnAfterPre) return;
  } else {
    //logV("honoring delay at position %d",whatRow);
  }

  if (chan[i].delayLocked) return;

  // instrument
  bool insChanged=false;
  if (pat->data[whatRow][2]!=-1) {
    if (chan[i].lastIns!=pat->data[whatRow][2]) {
      dispatchCmd(DivCommand(DIV_CMD_INSTRUMENT,i,pat->data[whatRow][2]));
      chan[i].lastIns=pat->data[whatRow][2];
      insChanged=true;
      if (song.legacyVolumeSlides && chan[i].volume==chan[i].volMax+1) {
        logV("forcing volume");
        chan[i].volume=chan[i].volMax;
        dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
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
        dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
        chan[i].stopOnOff=false;
      }
      if (disCont[dispatchOfChan[i]].dispatch->keyOffAffectsPorta(dispatchChanOfChan[i])) {
        chan[i].portaNote=-1;
        chan[i].portaSpeed=-1;
        dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
        /*if (i==2 && sysOfChan[i]==DIV_SYSTEM_SMS) {
          chan[i+1].portaNote=-1;
          chan[i+1].portaSpeed=-1;
        }*/
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
        dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
        chan[i].stopOnOff=false;
      }
      if (disCont[dispatchOfChan[i]].dispatch->keyOffAffectsPorta(dispatchChanOfChan[i])) {
        chan[i].portaNote=-1;
        chan[i].portaSpeed=-1;
        dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
        /*if (i==2 && sysOfChan[i]==DIV_SYSTEM_SMS) {
          chan[i+1].portaNote=-1;
          chan[i+1].portaSpeed=-1;
        }*/
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
        dispatchCmd(DivCommand(DIV_CMD_HINT_ARPEGGIO,i,chan[i].arp));
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
      if (pat->data[whatRow][0]==0 && pat->data[whatRow][1]==0) {
        chan[i].midiAftertouch=true;
      }
      chan[i].volume=pat->data[whatRow][3]<<8;
      dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
      dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
    }
  }

  chan[i].retrigSpeed=0;

  short lastSlide=-1;
  bool calledPorta=false;
  bool panChanged=false;

  // effects
  for (int j=0; j<curPat[i].effectCols; j++) {
    short effect=pat->data[whatRow][4+(j<<1)];
    short effectVal=pat->data[whatRow][5+(j<<1)];

    if (effectVal==-1) effectVal=0;

    // per-system effect
    if (!perSystemEffect(i,effect,effectVal)) switch (effect) {
      case 0x08: // panning (split 4-bit)
        chan[i].panL=(effectVal>>4)|(effectVal&0xf0);
        chan[i].panR=(effectVal&15)|((effectVal&15)<<4);
        panChanged=true;
        break;
      case 0x80: { // panning (linear)
        unsigned short pan=convertPanLinearToSplit(effectVal,8,255);
        chan[i].panL=pan>>8;
        chan[i].panR=pan&0xff;
        panChanged=true;
        break;
      }
      case 0x81: // panning left (split 8-bit)
        chan[i].panL=effectVal;
        panChanged=true;
        break;
      case 0x82: // panning right (split 8-bit)
        chan[i].panR=effectVal;
        panChanged=true;
        break;
      case 0x01: // ramp up
        if (song.ignoreDuplicateSlides && (lastSlide==0x01 || lastSlide==0x1337)) break;
        lastSlide=0x01;
        if (effectVal==0) {
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
          chan[i].inPorta=false;
          if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        } else {
          chan[i].portaNote=song.limitSlides?0x60:255;
          chan[i].portaSpeed=effectVal;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
          chan[i].portaStop=true;
          chan[i].nowYouCanStop=false;
          chan[i].stopOnOff=false;
          chan[i].scheduledSlideReset=false;
          chan[i].wasShorthandPorta=false;
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
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
          chan[i].inPorta=false;
          if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        } else {
          chan[i].portaNote=song.limitSlides?disCont[dispatchOfChan[i]].dispatch->getPortaFloor(dispatchChanOfChan[i]):-60;
          chan[i].portaSpeed=effectVal;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
          chan[i].portaStop=true;
          chan[i].nowYouCanStop=false;
          chan[i].stopOnOff=false;
          chan[i].scheduledSlideReset=false;
          chan[i].wasShorthandPorta=false;
          chan[i].inPorta=false;
          if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,0));
        }
        break;
      case 0x03: // portamento
        if (effectVal==0) {
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
          chan[i].inPorta=false;
          dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        } else {
          calledPorta=true;
          if (chan[i].note==chan[i].oldNote && !chan[i].inPorta && song.buggyPortaAfterSlide) {
            chan[i].portaNote=chan[i].note;
            chan[i].portaSpeed=-1;
          } else {
            chan[i].portaNote=chan[i].note;
            chan[i].portaSpeed=effectVal;
            chan[i].inPorta=true;
            chan[i].wasShorthandPorta=false;
          }
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
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
        dispatchCmd(DivCommand(DIV_CMD_HINT_VIBRATO,i,chan[i].vibratoDepth,chan[i].vibratoRate));
        dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
        break;
      case 0x07: // tremolo
        // TODO
        // this effect is really weird. i thought it would alter the tremolo depth but turns out it's completely different
        // this is how it works:
        // - 07xy enables tremolo
        // - when enabled, a "low" boundary is calculated based on the current volume
        // - then a volume slide down starts to the low boundary, and then when this is reached a volume slide up begins
        // - this process repeats until 0700 or 0Axy are found
        // - note that a volume value does not stop tremolo - instead it glitches this whole thing up
        break;
      case 0x0a: // volume ramp
        // TODO: non-0x-or-x0 value should be treated as 00
        if (effectVal!=0) {
          if ((effectVal&15)!=0) {
            chan[i].volSpeed=-(effectVal&15)*64;
          } else {
            chan[i].volSpeed=(effectVal>>4)*64;
          }
        } else {
          chan[i].volSpeed=0;
        }
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE,i,chan[i].volSpeed));
        break;
      case 0x00: // arpeggio
        chan[i].arp=effectVal;
        if (chan[i].arp==0 && song.arp0Reset) {
          chan[i].resetArp=true;
        }
        dispatchCmd(DivCommand(DIV_CMD_HINT_ARPEGGIO,i,chan[i].arp));
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
        if (divider<1) divider=1;
        cycles=got.rate*pow(2,MASTER_CLOCK_PREC)/divider;
        clockDrift=0;
        subticks=0;
        break;
      case 0xe0: // arp speed
        if (effectVal>0) {
          curSubSong->arpLen=effectVal;
        }
        break;
      case 0xe1: // portamento up
        chan[i].portaNote=chan[i].note+(effectVal&15);
        chan[i].portaSpeed=(effectVal>>4)*4;
        dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
        chan[i].portaStop=true;
        chan[i].nowYouCanStop=false;
        chan[i].stopOnOff=song.stopPortaOnNoteOff; // what?!
        chan[i].scheduledSlideReset=false;
        if ((effectVal&15)!=0) {
          chan[i].inPorta=true;
          chan[i].shorthandPorta=true;
          chan[i].wasShorthandPorta=true;
          if (!song.brokenShortcutSlides) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,0));
          if (song.e1e2AlsoTakePriority) lastSlide=0x1337; // ...
        } else {
          chan[i].inPorta=false;
          if (!song.brokenShortcutSlides) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        }
        break;
      case 0xe2: // portamento down
        chan[i].portaNote=chan[i].note-(effectVal&15);
        chan[i].portaSpeed=(effectVal>>4)*4;
        dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
        chan[i].portaStop=true;
        chan[i].nowYouCanStop=false;
        chan[i].stopOnOff=song.stopPortaOnNoteOff; // what?!
        chan[i].scheduledSlideReset=false;
        if ((effectVal&15)!=0) {
          chan[i].inPorta=true;
          chan[i].shorthandPorta=true;
          chan[i].wasShorthandPorta=true;
          if (!song.brokenShortcutSlides) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,0));
          if (song.e1e2AlsoTakePriority) lastSlide=0x1337; // ...
        } else {
          chan[i].inPorta=false;
          if (!song.brokenShortcutSlides) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        }
        break;
      case 0xe3: // vibrato direction
        chan[i].vibratoDir=effectVal;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VIBRATO_SHAPE,i,chan[i].vibratoDir));
        break;
      case 0xe4: // vibrato fine
        chan[i].vibratoFine=effectVal;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VIBRATO_RANGE,i,chan[i].vibratoFine));
        break;
      case 0xe5: // pitch
        chan[i].pitch=effectVal-0x80;
        if (sysOfChan[i]==DIV_SYSTEM_YM2151) { // YM2151 pitch oddity
          chan[i].pitch*=2;
          if (chan[i].pitch<-128) chan[i].pitch=-128;
          if (chan[i].pitch>127) chan[i].pitch=127;
        }
        //chan[i].pitch+=globalPitch;
        dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
        dispatchCmd(DivCommand(DIV_CMD_HINT_PITCH,i,chan[i].pitch));
        break;
      case 0xea: // legato mode
        chan[i].legato=effectVal;
        break;
      case 0xeb: // sample bank
        dispatchCmd(DivCommand(DIV_CMD_SAMPLE_BANK,i,effectVal));
        break;
      case 0xec: // delayed note cut
        if (effectVal>0 && (song.delayBehavior==2 || effectVal<nextSpeed)) {
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
        if (divider<1) divider=1;
        cycles=got.rate*pow(2,MASTER_CLOCK_PREC)/divider;
        clockDrift=0;
        subticks=0;
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
        dispatchCmd(DivCommand(DIV_CMD_NOTE_PORTA,i,chan[i].portaSpeed*(song.linearPitch==2?song.pitchSlideSpeed:1),chan[i].portaNote));
        chan[i].portaNote=-1;
        chan[i].portaSpeed=-1;
        chan[i].inPorta=false;
        if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        break;
      case 0xf3: // fine volume ramp up
        chan[i].volSpeed=effectVal;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE,i,chan[i].volSpeed));
        break;
      case 0xf4: // fine volume ramp down
        chan[i].volSpeed=-effectVal;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE,i,chan[i].volSpeed));
        break;
      case 0xf5: // disable macro
        dispatchCmd(DivCommand(DIV_CMD_MACRO_OFF,i,effectVal&0xff));
        break;
      case 0xf6: // enable macro
        dispatchCmd(DivCommand(DIV_CMD_MACRO_ON,i,effectVal&0xff));
        break;
      case 0xf8: // single volume ramp up
        chan[i].volume=MIN(chan[i].volume+effectVal*256,chan[i].volMax);
        dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
        break;
      case 0xf9: // single volume ramp down
        chan[i].volume=MAX(chan[i].volume-effectVal*256,0);
        dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
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
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE,i,chan[i].volSpeed));
        break;
      
      case 0xff: // stop song
        shallStopSched=true;
        logV("scheduling stop");
        break;
    }
  }

  if (panChanged) {
    dispatchCmd(DivCommand(DIV_CMD_PANNING,i,chan[i].panL,chan[i].panR));
  }

  if (insChanged && (chan[i].inPorta || calledPorta) && song.newInsTriggersInPorta) {
    dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,i,DIV_NOTE_NULL));
  }

  if (chan[i].doNote) {
    if (!song.continuousVibrato) {
      chan[i].vibratoPos=0;
    }
    dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
    if (chan[i].legato) {
      dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
      dispatchCmd(DivCommand(DIV_CMD_HINT_LEGATO,i,chan[i].note));
    } else {
      if (chan[i].inPorta && chan[i].keyOn && !chan[i].shorthandPorta) {
        if (song.e1e2StopOnSameNote && chan[i].wasShorthandPorta) {
          chan[i].portaSpeed=-1;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
          if (!song.brokenShortcutSlides) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
          chan[i].wasShorthandPorta=false;
          chan[i].inPorta=false;
        } else {
          chan[i].portaNote=chan[i].note;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
        }
      } else if (!chan[i].noteOnInhibit) {
        dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,i,chan[i].note,chan[i].volume>>8));
        keyHit[i]=true;
      }
    }
    chan[i].doNote=false;
    if (!chan[i].keyOn && chan[i].scheduledSlideReset) {
      chan[i].portaNote=-1;
      chan[i].portaSpeed=-1;
      dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
      chan[i].scheduledSlideReset=false;
      chan[i].inPorta=false;
    }
    if (!chan[i].keyOn && chan[i].volume>chan[i].volMax) {
      chan[i].volume=chan[i].volMax;
      dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
      dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
    }
    chan[i].keyOn=true;
    chan[i].keyOff=false;
  }
  chan[i].nowYouCanStop=true;
  chan[i].shorthandPorta=false;
  chan[i].noteOnInhibit=false;

  // post effects
  for (int j=0; j<curPat[i].effectCols; j++) {
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
  if (view==DIV_STATUS_PATTERN && !skipping) {
    strcpy(pb1,"");
    strcpy(pb3,"");
    for (int i=0; i<chans; i++) {
      snprintf(pb,4095," %.2x",curOrders->ord[i][curOrder]);
      strcat(pb1,pb);
      
      DivPattern* pat=curPat[i].getPattern(curOrders->ord[i][curOrder],false);
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
      for (int j=0; j<curPat[i].effectCols; j++) {
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

  if (curSubSong->hilightA>0) {
    if ((curRow%curSubSong->hilightA)==0) {
      pendingMetroTick=1;
      elapsedBeats++;
    }
  }
  if (curSubSong->hilightB>0) {
    if ((curRow%curSubSong->hilightB)==0) {
      pendingMetroTick=2;
      elapsedBars++;
      elapsedBeats=0;
    }
  }

  prevOrder=curOrder;
  prevRow=curRow;

  for (int i=0; i<chans; i++) {
    if (song.delayBehavior!=2) {
      chan[i].rowDelay=0;
    }
    processRow(i,false);
  }

  walked[((curOrder<<5)+(curRow>>3))&8191]|=1<<(curRow&7);

  if (changeOrd!=-1) {
    if (repeatPattern) {
      curRow=0;
      changeOrd=-1;
    } else {
      curRow=changePos;
      changePos=0;
      if (changeOrd==-2) changeOrd=curOrder+1;
      // old loop detection routine
      //if (changeOrd<=curOrder) endOfSong=true;
      curOrder=changeOrd;
      if (curOrder>=curSubSong->ordersLen) {
        curOrder=0;
        endOfSong=true;
        memset(walked,0,8192);
      }
      changeOrd=-1;
    }
    if (haltOn==DIV_HALT_PATTERN) halted=true;
  } else if (playing) if (++curRow>=curSubSong->patLen) {
    nextOrder();
    if (haltOn==DIV_HALT_PATTERN) halted=true;
  }

  // new loop detection routine
  if (!endOfSong && walked[((curOrder<<5)+(curRow>>3))&8191]&(1<<(curRow&7))) {
    logV("loop reached");
    endOfSong=true;
    memset(walked,0,8192);
  }

  if (song.brokenSpeedSel) {
    if ((curSubSong->patLen&1) && curOrder&1) {
      ticks=((curRow&1)?speed2:speed1)*(curSubSong->timeBase+1);
      nextSpeed=(curRow&1)?speed1:speed2;
    } else {
      ticks=((curRow&1)?speed1:speed2)*(curSubSong->timeBase+1);
      nextSpeed=(curRow&1)?speed2:speed1;
    }
  } else {
    if (speedAB) {
      ticks=speed2*(curSubSong->timeBase+1);
      nextSpeed=speed1;
    } else {
      ticks=speed1*(curSubSong->timeBase+1);
      nextSpeed=speed2;
    }
    speedAB=!speedAB;
  }

  // post row details
  for (int i=0; i<chans; i++) {
    DivPattern* pat=curPat[i].getPattern(curOrders->ord[i][curOrder],false);
    if (!(pat->data[curRow][0]==0 && pat->data[curRow][1]==0)) {
      if (pat->data[curRow][0]!=100 && pat->data[curRow][0]!=101 && pat->data[curRow][0]!=102) {
        if (!chan[i].legato) {
          bool wantPreNote=false;
          if (disCont[dispatchOfChan[i]].dispatch!=NULL) {
            wantPreNote=disCont[dispatchOfChan[i]].dispatch->getWantPreNote();
            if (wantPreNote) dispatchCmd(DivCommand(DIV_CMD_PRE_NOTE,i,ticks));
          }

          if (song.oneTickCut) {
            bool doPrepareCut=true;

            for (int j=0; j<curPat[i].effectCols; j++) {
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
            if (doPrepareCut && !wantPreNote && chan[i].cut<=0) chan[i].cut=ticks;
          }
        }
      }
    }
  }

  if (haltOn==DIV_HALT_ROW) halted=true;
  firstTick=true;
}

bool DivEngine::nextTick(bool noAccum, bool inhibitLowLat) {
  bool ret=false;
  if (divider<1) divider=1;

  if (lowLatency && !skipping && !inhibitLowLat) {
    tickMult=1000/divider;
    if (tickMult<1) tickMult=1;
  } else {
    tickMult=1;
  }
  
  cycles=got.rate*pow(2,MASTER_CLOCK_PREC)/(divider*tickMult);
  clockDrift+=fmod(got.rate*pow(2,MASTER_CLOCK_PREC),(double)(divider*tickMult));
  if (clockDrift>=(divider*tickMult)) {
    clockDrift-=(divider*tickMult);
    cycles++;
  }

  if (!pendingNotes.empty()) {
    bool isOn[DIV_MAX_CHANS];
    memset(isOn,0,DIV_MAX_CHANS*sizeof(bool));
    
    for (int i=pendingNotes.size()-1; i>=0; i--) {
      if (pendingNotes[i].channel<0 || pendingNotes[i].channel>=chans) continue;
      if (pendingNotes[i].on) {
        isOn[pendingNotes[i].channel]=true;
      } else {
        if (isOn[pendingNotes[i].channel]) {
          logV("erasing off -> on sequence in %d",pendingNotes[i].channel);
          pendingNotes.erase(pendingNotes.begin()+i);
        }
      }
    }
  }

  while (!pendingNotes.empty()) {
    DivNoteEvent& note=pendingNotes.front();
    if (note.channel<0 || note.channel>=chans) {
      pendingNotes.pop_front();
      continue;
    }
    if (note.on) {
      dispatchCmd(DivCommand(DIV_CMD_INSTRUMENT,note.channel,note.ins,1));
      dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,note.channel,note.note));
      keyHit[note.channel]=true;
      chan[note.channel].noteOnInhibit=true;
    } else {
      DivMacroInt* macroInt=disCont[dispatchOfChan[note.channel]].dispatch->getChanMacroInt(dispatchChanOfChan[note.channel]);
      if (macroInt!=NULL) {
        if (macroInt->hasRelease) {
          dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF_ENV,note.channel));
        } else {
          dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF,note.channel));
        }
      } else {
        dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF,note.channel));
      }
    }
    pendingNotes.pop_front();
  }

  if (!freelance) {
    if (--subticks<=0) {
      subticks=tickMult;

      // MIDI clock
      if (output) if (!skipping && output->midiOut!=NULL && midiOutClock) {
        output->midiOut->send(TAMidiMessage(TA_MIDI_CLOCK,0,0));
      }

      if (stepPlay!=1) {
        tempoAccum+=curSubSong->virtualTempoN;
        while (tempoAccum>=curSubSong->virtualTempoD) {
          tempoAccum-=curSubSong->virtualTempoD;
          if (--ticks<=0) {
            ret=endOfSong;
            if (shallStopSched) {
              logV("acknowledging scheduled stop");
              shallStop=true;
              break;
            } else if (endOfSong) {
              if (song.loopModality!=2) {
                playSub(true);
              }
            }
            endOfSong=false;
            if (stepPlay==2) stepPlay=1;
            nextRow();
            break;
          }
        }
        // under no circumstances shall the accumulator become this large
        if (tempoAccum>1023) tempoAccum=1023;
      }
      // process stuff
      if (!shallStop) for (int i=0; i<chans; i++) {
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
        if (!song.noSlidesOnFirstTick || !firstTick) {
          if (chan[i].volSpeed!=0) {
            chan[i].volume=(chan[i].volume&0xff)|(dispatchCmd(DivCommand(DIV_CMD_GET_VOLUME,i))<<8);
            chan[i].volume+=chan[i].volSpeed;
            if (chan[i].volume>chan[i].volMax) {
              chan[i].volume=chan[i].volMax;
              chan[i].volSpeed=0;
              dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
              dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
              dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE,i,0));
            } else if (chan[i].volume<0) {
              chan[i].volSpeed=0;
              dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE,i,0));
              if (song.legacyVolumeSlides) {
                chan[i].volume=chan[i].volMax+1;
              } else {
                chan[i].volume=0;
              }
              dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
              dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
            } else {
              dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
            }
          }
        }
        if (chan[i].vibratoDepth>0) {
          chan[i].vibratoPos+=chan[i].vibratoRate;
          if (chan[i].vibratoPos>=64) chan[i].vibratoPos-=64;

          chan[i].vibratoPosGiant+=chan[i].vibratoRate;
          if (chan[i].vibratoPos>=512) chan[i].vibratoPos-=512;

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
        if (!song.noSlidesOnFirstTick || !firstTick) {
          if ((chan[i].keyOn || chan[i].keyOff) && chan[i].portaSpeed>0) {
            if (dispatchCmd(DivCommand(DIV_CMD_NOTE_PORTA,i,chan[i].portaSpeed*(song.linearPitch==2?song.pitchSlideSpeed:1),chan[i].portaNote))==2 && chan[i].portaStop && song.targetResetsSlides) {
              chan[i].portaSpeed=0;
              dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
              chan[i].oldNote=chan[i].note;
              chan[i].note=chan[i].portaNote;
              chan[i].inPorta=false;
              dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
              dispatchCmd(DivCommand(DIV_CMD_HINT_LEGATO,i,chan[i].note));
            }
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
                dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
                chan[i].stopOnOff=false;
              }
              if (disCont[dispatchOfChan[i]].dispatch->keyOffAffectsPorta(dispatchChanOfChan[i])) {
                chan[i].portaNote=-1;
                chan[i].portaSpeed=-1;
                dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,chan[i].portaNote,chan[i].portaSpeed));
                /*if (i==2 && sysOfChan[i]==DIV_SYSTEM_SMS) {
                  chan[i+1].portaNote=-1;
                  chan[i+1].portaSpeed=-1;
                }*/
              }
              dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
              chan[i].scheduledSlideReset=true;
            }
            dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF,i));
          }
        }
        if (chan[i].resetArp) {
          dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
          dispatchCmd(DivCommand(DIV_CMD_HINT_LEGATO,i,chan[i].note));
          chan[i].resetArp=false;
        }
        if (song.rowResetsArpPos && firstTick) {
          chan[i].arpStage=-1;
        }
        if (chan[i].arp!=0 && !chan[i].arpYield && chan[i].portaSpeed<1) {
          if (--chan[i].arpTicks<1) {
            chan[i].arpTicks=curSubSong->arpLen;
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
  } else {
    // still tick the subtick counter
    if (--subticks<=0) {
      subticks=tickMult;
    }
  }

  firstTick=false;

  if (shallStop) {
    freelance=false;
    playing=false;
    extValuePresent=false;
    stepPlay=0;
    remainingLoops=-1;
    sPreview.sample=-1;
    sPreview.wave=-1;
    sPreview.pos=0;
    sPreview.dir=false;
    ret=true;
    shallStop=false;
    shallStopSched=false;
    return ret;
  }

  // system tick
  for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->tick(subticks==tickMult);

  if (!freelance) {
    if (stepPlay!=1) {
      if (!noAccum) {
        totalTicksR++;
        totalTicks+=1000000/(divider*tickMult);
      }
      if (totalTicks>=1000000) {
        totalTicks-=1000000;
        totalSeconds++;
        cmdsPerSecond=totalCmds-lastCmds;
        lastCmds=totalCmds;
      }
    }

    if (consoleMode && subticks<=1 && !skipping) fprintf(stderr,"\x1b[2K> %d:%.2d:%.2d.%.2d  %.2x/%.2x:%.3d/%.3d  %4dcmd/s\x1b[G",totalSeconds/3600,(totalSeconds/60)%60,totalSeconds%60,totalTicks/10000,curOrder,curSubSong->ordersLen,curRow,curSubSong->patLen,cmdsPerSecond);
  }

  if (haltOn==DIV_HALT_TICK) halted=true;

  return ret;
}

void DivEngine::nextBuf(float** in, float** out, int inChans, int outChans, unsigned int size) {
  lastLoopPos=-1;

  if (out!=NULL) {
    memset(out[0],0,size*sizeof(float));
    memset(out[1],0,size*sizeof(float));
  }

  if (softLocked) {
    if (!isBusy.try_lock()) {
      logV("audio is soft-locked (%d)",softLockCount++);
      return;
    }
  } else {
    isBusy.lock();
  }
  got.bufsize=size;

  std::chrono::steady_clock::time_point ts_processBegin=std::chrono::steady_clock::now();

  // process MIDI events (TODO: everything)
  if (output) if (output->midiIn) while (!output->midiIn->queue.empty()) {
    TAMidiMessage& msg=output->midiIn->queue.front();
    int ins=-1;
    if ((ins=midiCallback(msg))!=-2) {
      int chan=msg.type&15;
      switch (msg.type&0xf0) {
        case TA_MIDI_NOTE_OFF: {
          if (chan<0 || chan>=chans) break;
          if (midiIsDirect) {
            pendingNotes.push_back(DivNoteEvent(chan,-1,-1,-1,false));
          } else {
            autoNoteOff(msg.type&15,msg.data[0]-12,msg.data[1]);
          }
          if (!playing) {
            reset();
            freelance=true;
            playing=true;
          }
          break;
        }
        case TA_MIDI_NOTE_ON: {
          if (chan<0 || chan>=chans) break;
          if (msg.data[1]==0) {
            if (midiIsDirect) {
              pendingNotes.push_back(DivNoteEvent(chan,-1,-1,-1,false));
            } else {
              autoNoteOff(msg.type&15,msg.data[0]-12,msg.data[1]);
            }
          } else {
            if (midiIsDirect) {
              pendingNotes.push_back(DivNoteEvent(chan,ins,msg.data[0]-12,msg.data[1],true));
            } else {
              autoNoteOn(msg.type&15,ins,msg.data[0]-12,msg.data[1]);
            }
          }
          break;
        }
        case TA_MIDI_PROGRAM: {
          // TODO: change instrument event thingy
          break;
        }
      }
    }
    logD("%.2x",msg.type);
    output->midiIn->queue.pop();
  }
  
  // process audio
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
        if (sPreview.pos>=(int)s->samples || (sPreview.pEnd>=0 && sPreview.pos>=sPreview.pEnd)) {
          samp_temp=0;
        } else {
          samp_temp=s->data16[sPreview.pos];
          if (sPreview.dir) {
            sPreview.pos--;
          }
          else {
            sPreview.pos++;
          }
        }
        blip_add_delta(samp_bb,i,samp_temp-samp_prevSample);
        samp_prevSample=samp_temp;

        if (sPreview.dir) { // backward
          if (sPreview.pos<s->loopStart || (sPreview.pBegin>=0 && sPreview.pos<sPreview.pBegin)) {
            if (s->isLoopable() && sPreview.pos<s->loopEnd) {
              switch (s->loopMode) {
                case DivSampleLoopMode::DIV_SAMPLE_LOOP_FORWARD:
                  sPreview.pos=s->loopStart;
                  sPreview.dir=false;
                  break;
                case DivSampleLoopMode::DIV_SAMPLE_LOOP_BACKWARD:
                  sPreview.pos=s->loopEnd-1;
                  sPreview.dir=true;
                  break;
                case DivSampleLoopMode::DIV_SAMPLE_LOOP_PINGPONG:
                  sPreview.pos=s->loopStart;
                  sPreview.dir=false;
                  break;
                default:
                  break;
              }
            }
          }
        } else { // forward
          if (sPreview.pos>=s->loopEnd || (sPreview.pEnd>=0 && sPreview.pos>=sPreview.pEnd)) {
            if (s->isLoopable() && sPreview.pos>=s->loopStart) {
              switch (s->loopMode) {
                case DivSampleLoopMode::DIV_SAMPLE_LOOP_FORWARD:
                  sPreview.pos=s->loopStart;
                  sPreview.dir=false;
                  break;
                case DivSampleLoopMode::DIV_SAMPLE_LOOP_BACKWARD:
                  sPreview.pos=s->loopEnd-1;
                  sPreview.dir=true;
                  break;
                case DivSampleLoopMode::DIV_SAMPLE_LOOP_PINGPONG:
                  sPreview.pos=s->loopEnd-1;
                  sPreview.dir=true;
                  break;
                default:
                  break;
              }
            }
          }
        }
      }
      if (sPreview.dir) { // backward
        if (sPreview.pos<=s->loopStart || (sPreview.pBegin>=0 && sPreview.pos<=sPreview.pBegin)) {
          if (s->isLoopable() && sPreview.pos>=s->loopStart) {
            switch (s->loopMode) {
              case DivSampleLoopMode::DIV_SAMPLE_LOOP_FORWARD:
                sPreview.pos=s->loopStart;
                sPreview.dir=false;
                break;
              case DivSampleLoopMode::DIV_SAMPLE_LOOP_BACKWARD:
                sPreview.pos=s->loopEnd-1;
                sPreview.dir=true;
                break;
              case DivSampleLoopMode::DIV_SAMPLE_LOOP_PINGPONG:
                sPreview.pos=s->loopStart;
                sPreview.dir=false;
                break;
              default:
                break;
            }
          } else if (sPreview.pos<0) {
            sPreview.sample=-1;
          }
        }
      } else { // forward
        if (sPreview.pos>=s->loopEnd || (sPreview.pEnd>=0 && sPreview.pos>=sPreview.pEnd)) {
          if (s->isLoopable() && sPreview.pos>=s->loopStart) {
            switch (s->loopMode) {
              case DivSampleLoopMode::DIV_SAMPLE_LOOP_FORWARD:
                sPreview.pos=s->loopStart;
                sPreview.dir=false;
                break;
              case DivSampleLoopMode::DIV_SAMPLE_LOOP_BACKWARD:
                sPreview.pos=s->loopEnd-1;
                sPreview.dir=true;
                break;
              case DivSampleLoopMode::DIV_SAMPLE_LOOP_PINGPONG:
                sPreview.pos=s->loopEnd-1;
                sPreview.dir=true;
                break;
              default:
                break;
            }
          } else if (sPreview.pos>=(int)s->samples) {
            sPreview.sample=-1;
          }
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
        if (++sPreview.pos>=wave->len) {
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
      for (unsigned int i=0; i<size; i++) {
        oscBuf[0][oscWritePos]=out[0][i];
        oscBuf[1][oscWritePos]=out[1][i];
        if (++oscWritePos>=32768) oscWritePos=0;
      }
      oscSize=size;
    }
    isBusy.unlock();
    return;
  }

  // logic starts here
  for (int i=0; i<song.systemLen; i++) {
    // TODO: we may have a problem here
    disCont[i].lastAvail=blip_samples_avail(disCont[i].bb[0]);
    if (disCont[i].lastAvail>0) {
      disCont[i].flush(disCont[i].lastAvail);
    }
    if (size<disCont[i].lastAvail) {
      disCont[i].runtotal=0;
    } else {
      disCont[i].runtotal=blip_clocks_needed(disCont[i].bb[0],size-disCont[i].lastAvail);
    }
    if (disCont[i].runtotal>disCont[i].bbInLen) {
      logV("growing dispatch %d bbIn to %d",i,disCont[i].runtotal+256);
      delete[] disCont[i].bbIn[0];
      delete[] disCont[i].bbIn[1];
      disCont[i].bbIn[0]=new short[disCont[i].runtotal+256];
      disCont[i].bbIn[1]=new short[disCont[i].runtotal+256];
      disCont[i].bbInLen=disCont[i].runtotal+256;
    }
    disCont[i].runLeft=disCont[i].runtotal;
    disCont[i].runPos=0;
  }

  if (metroTickLen<size) {
    if (metroTick!=NULL) delete[] metroTick;
    metroTick=new unsigned char[size];
    metroTickLen=size;
  }

  memset(metroTick,0,size);

  int attempts=0;
  int runLeftG=size<<MASTER_CLOCK_PREC;
  while (++attempts<(int)size) {
    // 0. check if we've halted
    if (halted) break;
    // 1. check whether we are done with all buffers
    if (runLeftG<=0) break;

    // 2. check whether we gonna tick
    if (cycles<=0) {
      // we have to tick
      if (nextTick()) {
        lastLoopPos=size-(runLeftG>>MASTER_CLOCK_PREC);
        logD("last loop pos: %d for a size of %d and runLeftG of %d",lastLoopPos,size,runLeftG);
        totalLoops++;
        if (remainingLoops>0) {
          remainingLoops--;
          if (!remainingLoops) {
            logI("end of song!");
            remainingLoops=-1;
            playing=false;
            freelance=false;
            extValuePresent=false;
            break;
          }
        }
      }
      if (pendingMetroTick) {
        unsigned int realPos=size-(runLeftG>>MASTER_CLOCK_PREC);
        if (realPos>=size) realPos=size-1;
        metroTick[realPos]=pendingMetroTick;
        pendingMetroTick=0;
      }
    } else {
      // 3. tick the clock and fill buffers as needed
      if (cycles<runLeftG) {
        for (int i=0; i<song.systemLen; i++) {
          int total=(cycles*disCont[i].runtotal)/(size<<MASTER_CLOCK_PREC);
          disCont[i].acquire(disCont[i].runPos,total);
          disCont[i].runLeft-=total;
          disCont[i].runPos+=total;
        }
        runLeftG-=cycles;
        cycles=0;
      } else {
        cycles-=runLeftG;
        runLeftG=0;
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].acquire(disCont[i].runPos,disCont[i].runLeft);
          disCont[i].runLeft=0;
        }
      }
    }
  }

  if (out==NULL || halted) {
    isBusy.unlock();
    return;
  }

  //logD("attempts: %d",attempts);
  if (attempts>=(int)size) {
    logE("hang detected! stopping! at %d seconds %d micro",totalSeconds,totalTicks);
    freelance=false;
    playing=false;
    extValuePresent=false;
  }
  totalProcessed=size-(runLeftG>>MASTER_CLOCK_PREC);

  for (int i=0; i<song.systemLen; i++) {
    if (size<disCont[i].lastAvail) {
      logW("%d: size<lastAvail! %d<%d",i,size,disCont[i].lastAvail);
      continue;
    }
    disCont[i].fillBuf(disCont[i].runtotal,disCont[i].lastAvail,size-disCont[i].lastAvail);
  }

  for (int i=0; i<song.systemLen; i++) {
    float volL=((float)song.systemVol[i]/64.0f)*((float)MIN(127,127-(int)song.systemPan[i])/127.0f)*song.masterVol;
    float volR=((float)song.systemVol[i]/64.0f)*((float)MIN(127,127+(int)song.systemPan[i])/127.0f)*song.masterVol;
    volL*=disCont[i].dispatch->getPostAmp();
    volR*=disCont[i].dispatch->getPostAmp();
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
      out[0][i]+=(sin(metroPos*2*M_PI))*metroAmp*metroVol;
      out[1][i]+=(sin(metroPos*2*M_PI))*metroAmp*metroVol;
    }
    metroAmp-=0.0003f;
    if (metroAmp<0.0f) metroAmp=0.0f;
    metroPos+=metroFreq;
    while (metroPos>=1) metroPos--;
  }

  for (unsigned int i=0; i<size; i++) {
    oscBuf[0][oscWritePos]=out[0][i];
    oscBuf[1][oscWritePos]=out[1][i];
    if (++oscWritePos>=32768) oscWritePos=0;
  }
  oscSize=size;

  if (forceMono) {
    for (size_t i=0; i<size; i++) {
      out[0][i]=(out[0][i]+out[1][i])*0.5;
      out[1][i]=out[0][i];
    }
  }
  if (clampSamples) {
    for (size_t i=0; i<size; i++) {
      if (out[0][i]<-1.0) out[0][i]=-1.0;
      if (out[0][i]>1.0) out[0][i]=1.0;
      if (out[1][i]<-1.0) out[1][i]=-1.0;
      if (out[1][i]>1.0) out[1][i]=1.0;
    }
  }
  isBusy.unlock();

  std::chrono::steady_clock::time_point ts_processEnd=std::chrono::steady_clock::now();

  processTime=std::chrono::duration_cast<std::chrono::nanoseconds>(ts_processEnd-ts_processBegin).count();
}
