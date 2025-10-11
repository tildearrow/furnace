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

#include "macroInt.h"
#include <chrono>
#define _USE_MATH_DEFINES
#include "dispatch.h"
#include "engine.h"
#include "workPool.h"
#include "../ta-log.h"
#include <math.h>

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

static const char* notes[12]={
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
  "HINT_VOL_SLIDE_TARGET",
  "HINT_TREMOLO",
  "HINT_PANBRELLO",
  "HINT_PAN_SLIDE",
  "HINT_PANNING",

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

  "FM_LFO2",
  "FM_LFO2_WAVE",

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

  "SURROUND_PANNING",

  "FM_AM2_DEPTH",
  "FM_PM2_DEPTH",

  "ES5506_FILTER_MODE",
  "ES5506_FILTER_K1",
  "ES5506_FILTER_K2",
  "ES5506_FILTER_K1_SLIDE",
  "ES5506_FILTER_K2_SLIDE",
  "ES5506_ENVELOPE_COUNT",
  "ES5506_ENVELOPE_LVRAMP",
  "ES5506_ENVELOPE_RVRAMP",
  "ES5506_ENVELOPE_K1RAMP",
  "ES5506_ENVELOPE_K2RAMP",
  "ES5506_PAUSE",

  "HINT_ARP_TIME",

  "SNES_GLOBAL_VOL_LEFT",
  "SNES_GLOBAL_VOL_RIGHT",

  "NES_LINEAR_LENGTH",

  "EXTERNAL",

  "C64_AD",
  "C64_SR",

  "ESFM_OP_PANNING",
  "ESFM_OUTLVL",
  "ESFM_MODIN",
  "ESFM_ENV_DELAY",

  "MACRO_RESTART",

  "POWERNOISE_COUNTER_LOAD",
  "POWERNOISE_IO_WRITE",

  "DAVE_HIGH_PASS",
  "DAVE_RING_MOD",
  "DAVE_SWAP_COUNTERS",
  "DAVE_LOW_PASS",
  "DAVE_CLOCK_DIV",

  "MINMOD_ECHO",

  "BIFURCATOR_STATE_LOAD",
  "BIFURCATOR_PARAMETER",

  "FDS_MOD_AUTO",
  
  "FM_OPMASK",

  "MULTIPCM_MIX_FM",
  "MULTIPCM_MIX_PCM",
  "MULTIPCM_LFO",
  "MULTIPCM_VIB",
  "MULTIPCM_AM",
  "MULTIPCM_AR",
  "MULTIPCM_D1R",
  "MULTIPCM_DL",
  "MULTIPCM_D2R",
  "MULTIPCM_RR",
  "MULTIPCM_RC",
  "MULTIPCM_DAMP",
  "MULTIPCM_PSEUDO_REVERB",
  "MULTIPCM_LFO_RESET",
  "MULTIPCM_LEVEL_DIRECT",

  "SID3_SPECIAL_WAVE",
  "SID3_RING_MOD_SRC",
  "SID3_HARD_SYNC_SRC",
  "SID3_PHASE_MOD_SRC",
  "SID3_WAVE_MIX",
  "SID3_LFSR_FEEDBACK_BITS",
  "SID3_1_BIT_NOISE",
  "SID3_FILTER_DISTORTION",
  "SID3_FILTER_OUTPUT_VOLUME",
  "SID3_CHANNEL_INVERSION",
  "SID3_FILTER_CONNECTION",
  "SID3_FILTER_MATRIX",
  "SID3_FILTER_ENABLE",

  "C64_PW_SLIDE",
  "C64_CUTOFF_SLIDE",

  "SID3_PHASE_RESET",
  "SID3_NOISE_PHASE_RESET",
  "SID3_ENVELOPE_RESET",
  "SID3_CUTOFF_SCALING",
  "SID3_RESONANCE_SCALING",

  "WS_GLOBAL_SPEAKER_VOLUME",

  "FM_ALG",
  "FM_FMS",
  "FM_AMS",
  "FM_FMS2",
  "FM_AMS2"
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

  if (output) if (!skipping && output->midiOut!=NULL && !isChannelMuted(c.chan)) {
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
            if (chan[c.chan].curMidiNote>=0) {
              output->midiOut->send(TAMidiMessage(0x90|(c.chan&15),chan[c.chan].curMidiNote,scaledVol));
            }
            break;
          case DIV_CMD_NOTE_OFF:
          case DIV_CMD_NOTE_OFF_ENV:
            if (chan[c.chan].curMidiNote>=0) {
              output->midiOut->send(TAMidiMessage(0x80|(c.chan&15),chan[c.chan].curMidiNote,scaledVol));
            }
            chan[c.chan].curMidiNote=-1;
            break;
          case DIV_CMD_INSTRUMENT:
            if (chan[c.chan].lastIns!=c.value && midiOutProgramChange) {
              output->midiOut->send(TAMidiMessage(0xc0|(c.chan&15),c.value&0x7f,0));
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
            if (pan<0) pan=0;
            if (pan>127) pan=127;
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

bool DivEngine::perSystemPreEffect(int ch, unsigned char effect, unsigned char effectVal) {
  DivSysDef* sysDef=sysDefs[sysOfChan[ch]];
  if (sysDef==NULL) return false;
  auto iter=sysDef->preEffectHandlers.find(effect);
  if (iter==sysDef->preEffectHandlers.end()) return false;
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

void DivEngine::processRowPre(int i) {
  int whatOrder=curOrder;
  int whatRow=curRow;
  DivPattern* pat=curPat[i].getPattern(curOrders->ord[i][whatOrder],false);
  for (int j=0; j<curPat[i].effectCols; j++) {
    short effect=pat->data[whatRow][4+(j<<1)];
    short effectVal=pat->data[whatRow][5+(j<<1)];

    if (effectVal==-1) effectVal=0;
    effectVal&=255;
    perSystemPreEffect(i,effect,effectVal);
  }
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
      effectVal&=255;

      switch (effect) {
        case 0x09: // select groove pattern/speed 1
          if (song.grooves.empty()) {
            if (effectVal>0) speeds.val[0]=effectVal;
          } else {
            if (effectVal<(short)song.grooves.size()) {
              speeds=song.grooves[effectVal];
              curSpeed=0;
            }
          }
          break;
        case 0x0f: // speed 1/speed 2
          if (speeds.len==2 && song.grooves.empty()) {
            if (effectVal>0) speeds.val[1]=effectVal;
          } else {
            if (effectVal>0) speeds.val[0]=effectVal;
          }
          break;
        case 0xfd: // virtual tempo num
          if (effectVal>0) virtualTempoN=effectVal;
          break;
        case 0xfe: // virtual tempo den
          if (effectVal>0) virtualTempoD=effectVal;
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
              chan[i].rowDelay=effectVal;
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
        dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
        chan[i].stopOnOff=false;
      }
      if (disCont[dispatchOfChan[i]].dispatch->keyOffAffectsPorta(dispatchChanOfChan[i])) {
        chan[i].portaNote=-1;
        chan[i].portaSpeed=-1;
        dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
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
        dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
        chan[i].stopOnOff=false;
      }
      if (disCont[dispatchOfChan[i]].dispatch->keyOffAffectsPorta(dispatchChanOfChan[i])) {
        chan[i].portaNote=-1;
        chan[i].portaSpeed=-1;
        dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
        /*if (i==2 && sysOfChan[i]==DIV_SYSTEM_SMS) {
          chan[i+1].portaNote=-1;
          chan[i+1].portaSpeed=-1;
        }*/
      }
      chan[i].scheduledSlideReset=true;
    }
    dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF_ENV,i));
    chan[i].releasing=true;
  } else if (pat->data[whatRow][0]==102) { // env release
    dispatchCmd(DivCommand(DIV_CMD_ENV_RELEASE,i));
    chan[i].releasing=true;
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
  int volPortaTarget=-1;
  bool noApplyVolume=false;
  for (int j=0; j<curPat[i].effectCols; j++) {
    short effect=pat->data[whatRow][4+(j<<1)];
    if (effect==0xd3 || effect==0xd4) { // vol porta
      volPortaTarget=pat->data[whatRow][3]<<8; // can be -256

      short effectVal=pat->data[whatRow][5+(j<<1)];
      if (effectVal==-1) effectVal=0;
      effectVal&=255;

      noApplyVolume=effectVal>0; // "D3.." or "D300" shouldn't stop volume from applying
      break; // technically you could have both D3 and D4... let's not care
    }
  }
  
  if (pat->data[whatRow][3]!=-1 && !noApplyVolume) {
    if (!song.oldAlwaysSetVolume || disCont[dispatchOfChan[i]].dispatch->getLegacyAlwaysSetVolume() || (MIN(chan[i].volMax,chan[i].volume)>>8)!=pat->data[whatRow][3]) {
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
  bool surroundPanChanged=false;
  bool sampleOffSet=false;

  // effects
  for (int j=0; j<curPat[i].effectCols; j++) {
    short effect=pat->data[whatRow][4+(j<<1)];
    short effectVal=pat->data[whatRow][5+(j<<1)];

    if (effectVal==-1) effectVal=0;
    effectVal&=255;

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
      case 0x83: // pan slide
        if (effectVal!=0) {
          if ((effectVal&15)!=0) {
            chan[i].panSpeed=(effectVal&15);
          } else {
            chan[i].panSpeed=-(effectVal>>4);
          }
          // panbrello and slides are incompatible
          chan[i].panDepth=0;
          chan[i].panRate=0;
          chan[i].panPos=0;
        } else {
          chan[i].panSpeed=0;
        }
        dispatchCmd(DivCommand(DIV_CMD_HINT_PAN_SLIDE,i,chan[i].panSpeed&0xff));
        break;
      case 0x84: // panbrello
        if (chan[i].panDepth==0) {
          chan[i].panPos=0;
        }
        chan[i].panDepth=effectVal&15;
        chan[i].panRate=effectVal>>4;
        if (chan[i].panDepth!=0) {
          chan[i].panSpeed=0;
        }
        dispatchCmd(DivCommand(DIV_CMD_HINT_PANBRELLO,i,effectVal));
        break;
      case 0x88: // panning rear (split 4-bit)
        chan[i].panRL=(effectVal>>4)|(effectVal&0xf0);
        chan[i].panRR=(effectVal&15)|((effectVal&15)<<4);
        surroundPanChanged=true;
        break;
      case 0x89: // panning left (split 8-bit)
        chan[i].panRL=effectVal;
        surroundPanChanged=true;
        break;
      case 0x8a: // panning right (split 8-bit)
        chan[i].panRR=effectVal;
        surroundPanChanged=true;
        break;
      case 0x01: // ramp up
        if (song.ignoreDuplicateSlides && (lastSlide==0x01 || lastSlide==0x1337)) break;
        lastSlide=0x01;
        if (effectVal==0) {
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
          chan[i].inPorta=false;
          if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        } else {
          chan[i].portaNote=song.limitSlides?0x60:255;
          chan[i].portaSpeed=effectVal;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
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
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
          chan[i].inPorta=false;
          if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        } else {
          chan[i].portaNote=song.limitSlides?disCont[dispatchOfChan[i]].dispatch->getPortaFloor(dispatchChanOfChan[i]):-60;
          chan[i].portaSpeed=effectVal;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
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
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
          chan[i].inPorta=false;
          dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        } else {
          chan[i].lastPorta=effectVal;
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
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
          chan[i].portaStop=true;
          if (chan[i].keyOn) chan[i].doNote=false;
          chan[i].stopOnOff=song.stopPortaOnNoteOff; // what?!
          chan[i].scheduledSlideReset=false;
          dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,1));
          lastSlide=0x1337; // i hate this so much
        }
        break;
      case 0x04: // vibrato
        if (effectVal) chan[i].lastVibrato=effectVal;
        chan[i].vibratoDepth=effectVal&15;
        chan[i].vibratoRate=effectVal>>4;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VIBRATO,i,(chan[i].vibratoDepth&15)|(chan[i].vibratoRate<<4)));
        dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
        break;
      case 0x05: // vol slide + vibrato
        if (effectVal==0) {
          chan[i].vibratoDepth=0;
          chan[i].vibratoRate=0;
        } else {
          chan[i].vibratoDepth=chan[i].lastVibrato&15;
          chan[i].vibratoRate=chan[i].lastVibrato>>4;
        }
        dispatchCmd(DivCommand(DIV_CMD_HINT_VIBRATO,i,(chan[i].vibratoDepth&15)|(chan[i].vibratoRate<<4)));
        dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
        // TODO: non-0x-or-x0 value should be treated as 00
        if (effectVal!=0) {
          if ((effectVal&15)!=0) {
            chan[i].volSpeed=-(effectVal&15)*64;
          } else {
            chan[i].volSpeed=(effectVal>>4)*64;
          }
          // tremolo and vol slides are incompatible
          chan[i].tremoloDepth=0;
          chan[i].tremoloRate=0;
        } else {
          chan[i].volSpeed=0;
        }
        chan[i].volSpeedTarget=-1;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE,i,chan[i].volSpeed));
        break;
      case 0x06: // vol slide + porta
        if (effectVal==0 || chan[i].lastPorta==0) {
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
          chan[i].inPorta=false;
          dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        } else {
          calledPorta=true;
          if (chan[i].note==chan[i].oldNote && !chan[i].inPorta && song.buggyPortaAfterSlide) {
            chan[i].portaNote=chan[i].note;
            chan[i].portaSpeed=-1;
          } else {
            chan[i].portaNote=chan[i].note;
            chan[i].portaSpeed=chan[i].lastPorta;
            chan[i].inPorta=true;
            chan[i].wasShorthandPorta=false;
          }
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
          chan[i].portaStop=true;
          if (chan[i].keyOn) chan[i].doNote=false;
          chan[i].stopOnOff=song.stopPortaOnNoteOff; // what?!
          chan[i].scheduledSlideReset=false;
          dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,1));
          lastSlide=0x1337; // i hate this so much
        }
        // TODO: non-0x-or-x0 value should be treated as 00
        if (effectVal!=0) {
          if ((effectVal&15)!=0) {
            chan[i].volSpeed=-(effectVal&15)*64;
          } else {
            chan[i].volSpeed=(effectVal>>4)*64;
          }
          // tremolo and vol slides are incompatible
          chan[i].tremoloDepth=0;
          chan[i].tremoloRate=0;
        } else {
          chan[i].volSpeed=0;
        }
        chan[i].volSpeedTarget=-1;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE,i,chan[i].volSpeed));
        break;
      case 0x07: // tremolo
        if (chan[i].tremoloDepth==0) {
          chan[i].tremoloPos=0;
        }
        chan[i].tremoloDepth=effectVal&15;
        chan[i].tremoloRate=effectVal>>4;
        dispatchCmd(DivCommand(DIV_CMD_HINT_TREMOLO,i,effectVal));
        if (chan[i].tremoloDepth!=0) {
          chan[i].volSpeed=0;
          chan[i].volSpeedTarget=-1;
        } else {
          dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
          dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
        }
        break;
      case 0x0a: // volume ramp
        // TODO: non-0x-or-x0 value should be treated as 00
        if (effectVal!=0) {
          if ((effectVal&15)!=0) {
            chan[i].volSpeed=-(effectVal&15)*64;
          } else {
            chan[i].volSpeed=(effectVal>>4)*64;
          }
          // tremolo and vol slides are incompatible
          chan[i].tremoloDepth=0;
          chan[i].tremoloRate=0;
        } else {
          chan[i].volSpeed=0;
        }
        chan[i].volSpeedTarget=-1;
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
        if (song.oldSampleOffset) {
          dispatchCmd(DivCommand(DIV_CMD_SAMPLE_POS,i,(((effect&0x0f)<<8)|effectVal)*256));
        } else {
          if (effect<0x93) {
            chan[i].sampleOff&=~(0xff<<((effect-0x90)<<3));
            chan[i].sampleOff|=effectVal<<((effect-0x90)<<3);
            sampleOffSet=true;
          }
        }
        break;
      case 0xc0: case 0xc1: case 0xc2: case 0xc3: // set Hz
        divider=(double)(((effect&0x3)<<8)|effectVal);
        if (divider<1) divider=1;
        cycles=got.rate/divider;
        clockDrift=0;
        subticks=0;
        break;
      case 0xdc: // delayed mute
        if (effectVal>0 && (song.delayBehavior==2 || effectVal<nextSpeed)) {
          chan[i].volCut=effectVal+1;
          chan[i].cutType=0;
        }
        break;
      case 0xd3: // volume portamento (vol porta)
        // tremolo and vol slides are incompatible
        chan[i].tremoloDepth=0;
        chan[i].tremoloRate=0;
        chan[i].volSpeed=volPortaTarget<0 ? 0 : volPortaTarget>chan[i].volume ? effectVal : -effectVal;
        chan[i].volSpeedTarget=chan[i].volSpeed==0 ? -1 : volPortaTarget;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE_TARGET,i,chan[i].volSpeed,chan[i].volSpeedTarget));
        break;
      case 0xd4: // volume portamento fast (vol porta fast)
        // tremolo and vol slides are incompatible
        chan[i].tremoloDepth=0;
        chan[i].tremoloRate=0;
        chan[i].volSpeed=volPortaTarget<0 ? 0 : volPortaTarget>chan[i].volume ? 256*effectVal : -256*effectVal;
        chan[i].volSpeedTarget=chan[i].volSpeed==0 ? -1 : volPortaTarget;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE_TARGET,i,chan[i].volSpeed,chan[i].volSpeedTarget));
        break;
      case 0xe0: // arp speed
        if (effectVal>0) {
          curSubSong->arpLen=effectVal;
          dispatchCmd(DivCommand(DIV_CMD_HINT_ARP_TIME,i,curSubSong->arpLen));
        }
        break;
      case 0xe1: // portamento up
        chan[i].portaNote=chan[i].note+(effectVal&15);
        chan[i].portaSpeed=(effectVal>>4)*4;
        dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
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
        dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
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
      case 0xe3: // vibrato shape
        chan[i].vibratoShape=effectVal;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VIBRATO_SHAPE,i,chan[i].vibratoShape));
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
      case 0xe6: // Delayed legato
        // why does this have to follow FamiTracker verbatim
        // couldn't you do better?
        if ((effectVal&15)!=0) {
          chan[i].legatoDelay=(((effectVal&0xf0)>>4)&7)+1;
          if (effectVal&128) {
            chan[i].legatoTarget=-(effectVal&15);
          } else {
            chan[i].legatoTarget=(effectVal&15);
          }
        } else {
          chan[i].legatoDelay=-1;
          chan[i].legatoTarget=0;
        }
        break;
      case 0xe7: // delayed macro release
        // "Bruh"
        if (effectVal>0 && (song.delayBehavior==2 || effectVal<nextSpeed)) {
          chan[i].cut=effectVal+1;
          chan[i].cutType=2;
        }
        break;
      case 0xe8: // delayed legato up
        // see? you COULD do better!
        if ((effectVal&15)!=0) {
          chan[i].legatoDelay=((effectVal&0xf0)>>4)+1;
          chan[i].legatoTarget=(effectVal&15);
        } else {
          chan[i].legatoDelay=-1;
          chan[i].legatoTarget=0;
        }
        break;
      case 0xe9: // delayed legato down
        if ((effectVal&15)!=0) {
          chan[i].legatoDelay=((effectVal&0xf0)>>4)+1;
          chan[i].legatoTarget=-(effectVal&15);
        } else {
          chan[i].legatoDelay=-1;
          chan[i].legatoTarget=0;
        }
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
          chan[i].cutType=0;
        }
        break;
      case 0xee: // external command
        //printf("\x1b[1;36m%d: extern command %d\x1b[m\n",i,effectVal);
        extValue=effectVal;
        extValuePresent=true;
        dispatchCmd(DivCommand(DIV_CMD_EXTERNAL,i,effectVal));
        break;
      case 0xef: // global pitch
        globalPitch+=(signed char)(effectVal-0x80);
        break;
      case 0xf0: // set Hz by tempo
        divider=(double)effectVal*2.0/5.0;
        if (divider<1) divider=1;
        cycles=got.rate/divider;
        clockDrift=0;
        subticks=0;
        break;
      case 0xf3: // fine volume ramp up
        // tremolo and vol slides are incompatible
        chan[i].tremoloDepth=0;
        chan[i].tremoloRate=0;
        chan[i].volSpeed=effectVal;
        chan[i].volSpeedTarget=-1;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE,i,chan[i].volSpeed));
        break;
      case 0xf4: // fine volume ramp down
        // tremolo and vol slides are incompatible
        chan[i].tremoloDepth=0;
        chan[i].tremoloRate=0;
        chan[i].volSpeed=-effectVal;
        chan[i].volSpeedTarget=-1;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE,i,chan[i].volSpeed));
        break;
      case 0xf5: // disable macro
        dispatchCmd(DivCommand(DIV_CMD_MACRO_OFF,i,effectVal&0xff));
        break;
      case 0xf6: // enable macro
        dispatchCmd(DivCommand(DIV_CMD_MACRO_ON,i,effectVal&0xff));
        break;
      case 0xf7: // restart macro
        dispatchCmd(DivCommand(DIV_CMD_MACRO_RESTART,i,effectVal&0xff));
        break;
      case 0xf8: // single volume ramp up
        chan[i].volSpeed=0; // add compat flag?
        chan[i].volSpeedTarget=-1;
        chan[i].volume=MIN(chan[i].volume+effectVal*256,chan[i].volMax);
        dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
        break;
      case 0xf9: // single volume ramp down
        chan[i].volSpeed=0; // add compat flag?
        chan[i].volSpeedTarget=-1;
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
          // tremolo and vol slides are incompatible
          chan[i].tremoloDepth=0;
          chan[i].tremoloRate=0;
        } else {
          chan[i].volSpeed=0;
        }
        chan[i].volSpeedTarget=-1;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE,i,chan[i].volSpeed));
        break;
      case 0xfc: // delayed note release
        if (song.delayBehavior==2 || effectVal<nextSpeed) {
          chan[i].cut=effectVal+1;
          chan[i].cutType=1;
        }
        break;
      
      case 0xff: // stop song
        shallStopSched=true;
        logV("scheduling stop");
        break;
    }
  }

  if (sampleOffSet) {
    dispatchCmd(DivCommand(DIV_CMD_SAMPLE_POS,i,chan[i].sampleOff));
  }

  if (panChanged) {
    dispatchCmd(DivCommand(DIV_CMD_PANNING,i,chan[i].panL,chan[i].panR));
    dispatchCmd(DivCommand(DIV_CMD_HINT_PANNING,i,chan[i].panL,chan[i].panR));
  }
  if (surroundPanChanged) {
    dispatchCmd(DivCommand(DIV_CMD_SURROUND_PANNING,i,2,chan[i].panRL));
    dispatchCmd(DivCommand(DIV_CMD_SURROUND_PANNING,i,3,chan[i].panRR));
  }

  if (insChanged && (chan[i].inPorta || calledPorta) && song.newInsTriggersInPorta) {
    dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,i,DIV_NOTE_NULL));
  }

  if (chan[i].doNote) {
    if (!song.continuousVibrato) {
      chan[i].vibratoPos=0;
    }
    dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
    if (chan[i].legato && (!chan[i].inPorta || song.brokenPortaLegato)) {
      dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
      dispatchCmd(DivCommand(DIV_CMD_HINT_LEGATO,i,chan[i].note));
    } else {
      if (chan[i].inPorta && chan[i].keyOn && !chan[i].shorthandPorta) {
        if (song.e1e2StopOnSameNote && chan[i].wasShorthandPorta) {
          chan[i].portaSpeed=-1;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
          if (!song.brokenShortcutSlides) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
          chan[i].wasShorthandPorta=false;
          chan[i].inPorta=false;
        } else {
          chan[i].portaNote=chan[i].note;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
        }
      } else if (!chan[i].noteOnInhibit) {
        dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,i,chan[i].note,chan[i].volume>>8));
        chan[i].releasing=false;
        if (song.resetArpPhaseOnNewNote) {
           chan[i].arpStage=-1;
        }
        chan[i].goneThroughNote=true;
        chan[i].wentThroughNote=true;
        keyHit[i]=true;
      }
    }
    chan[i].doNote=false;
    if (!chan[i].keyOn && chan[i].scheduledSlideReset) {
      chan[i].portaNote=-1;
      chan[i].portaSpeed=-1;
      dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
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
    effectVal&=255;
    if (!perSystemPostEffect(i,effect,effectVal)) {
      switch (effect) {
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
      }
    }
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

  if (!stepPlay) {
    playPosLock.lock();
    prevOrder=curOrder;
    prevRow=curRow;
    playPosLock.unlock();
  }

  for (int i=0; i<chans; i++) {
    // try to find pre effects
    processRowPre(i);
  }

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
    if (shallStopSched) {
      curRow=curSubSong->patLen-1;
    } else {
      nextOrder();
    }
    if (haltOn==DIV_HALT_PATTERN) halted=true;
  }

  // new loop detection routine
  if (!endOfSong && walked[((curOrder<<5)+(curRow>>3))&8191]&(1<<(curRow&7)) && !shallStopSched) {
    logV("loop reached");
    endOfSong=true;
    memset(walked,0,8192);
  }

  prevSpeed=nextSpeed;
  if (song.brokenSpeedSel) {
    unsigned char speed2=(speeds.len>=2)?speeds.val[1]:speeds.val[0];
    unsigned char speed1=speeds.val[0];
    
    if ((curSubSong->patLen&1) && curOrder&1) {
      ticks=((curRow&1)?speed2:speed1)*(curSubSong->timeBase+1);
      nextSpeed=(curRow&1)?speed1:speed2;
    } else {
      ticks=((curRow&1)?speed1:speed2)*(curSubSong->timeBase+1);
      nextSpeed=(curRow&1)?speed2:speed1;
    }
  } else {
    ticks=speeds.val[curSpeed]*(curSubSong->timeBase+1);
    curSpeed++;
    if (curSpeed>=speeds.len) curSpeed=0;
    nextSpeed=speeds.val[curSpeed];
  }

  /*
  if (skipping) {
    ticks=1;
  }*/

  // post row details
  for (int i=0; i<chans; i++) {
    DivPattern* pat=curPat[i].getPattern(curOrders->ord[i][curOrder],false);
    if (!(pat->data[curRow][0]==0 && pat->data[curRow][1]==0)) {
      if (pat->data[curRow][0]!=100 && pat->data[curRow][0]!=101 && pat->data[curRow][0]!=102) {
        if (!chan[i].legato) {
          bool wantPreNote=false;
          if (disCont[dispatchOfChan[i]].dispatch!=NULL) {
            wantPreNote=disCont[dispatchOfChan[i]].dispatch->getWantPreNote();
            if (wantPreNote) {
              bool doPreparePreNote=true;
              int addition=0;

              for (int j=0; j<curPat[i].effectCols; j++) {
                if (!song.preNoteNoEffect) {
                  if (pat->data[curRow][4+(j<<1)]==0x03 && pat->data[curRow][5+(j<<1)]!=0 && pat->data[curRow][5+(j<<1)]!=-1) {
                    doPreparePreNote=false;
                    break;
                  }
                  if (pat->data[curRow][4+(j<<1)]==0x06 && pat->data[curRow][5+(j<<1)]!=0 && pat->data[curRow][5+(j<<1)]!=-1) {
                    doPreparePreNote=false;
                    break;
                  }
                  if (pat->data[curRow][4+(j<<1)]==0xea) {
                    if (pat->data[curRow][5+(j<<1)]>0) {
                      doPreparePreNote=false;
                      break;
                    }
                  }
                }
                if (pat->data[curRow][4+(j<<1)]==0xed) {
                  if (pat->data[curRow][5+(j<<1)]>0) {
                    addition=pat->data[curRow][5+(j<<1)]&255;
                    break;
                  }
                }
              }
              if (doPreparePreNote) dispatchCmd(DivCommand(DIV_CMD_PRE_NOTE,i,ticks+addition));
            }
          }

          if (song.oneTickCut) {
            bool doPrepareCut=true;
            int addition=0;

            for (int j=0; j<curPat[i].effectCols; j++) {
              if (pat->data[curRow][4+(j<<1)]==0x03 && pat->data[curRow][5+(j<<1)]!=0 && pat->data[curRow][5+(j<<1)]!=-1) {
                doPrepareCut=false;
                break;
              }
              if (pat->data[curRow][4+(j<<1)]==0x06 && pat->data[curRow][5+(j<<1)]!=0 && pat->data[curRow][5+(j<<1)]!=-1) {
                doPrepareCut=false;
                break;
              }
              if (pat->data[curRow][4+(j<<1)]==0xea) {
                if (pat->data[curRow][5+(j<<1)]>0) {
                  doPrepareCut=false;
                  break;
                }
              }
              if (pat->data[curRow][4+(j<<1)]==0xed) {
                if (pat->data[curRow][5+(j<<1)]>0) {
                  addition=pat->data[curRow][5+(j<<1)]&255;
                  break;
                }
              }
            }
            if (doPrepareCut && !wantPreNote && chan[i].cut<=0) {
              chan[i].cut=ticks+addition;
              chan[i].cutType=0;
            }
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
  
  cycles=got.rate/(divider*tickMult);
  clockDrift+=fmod(got.rate,(double)(divider*tickMult));
  if (clockDrift>=(divider*tickMult)) {
    clockDrift-=(divider*tickMult);
    cycles++;
  }

  // don't let user play anything during export
  if (exporting) pendingNotes.clear();

  if (!pendingNotes.empty()) {
    bool isOn[DIV_MAX_CHANS];
    memset(isOn,0,DIV_MAX_CHANS*sizeof(bool));
    
    for (int i=pendingNotes.size()-1; i>=0; i--) {
      if (pendingNotes[i].channel<0 || pendingNotes[i].channel>=chans) continue;
      if (pendingNotes[i].on) {
        isOn[pendingNotes[i].channel]=true;
      } else {
        if (isOn[pendingNotes[i].channel]) {
          //logV("erasing off -> on sequence in %d",pendingNotes[i].channel);
          pendingNotes[i].nop=true;
        }
      }
    }
  }

  while (!pendingNotes.empty()) {
    DivNoteEvent& note=pendingNotes.front();
    if (note.nop || note.channel<0 || note.channel>=chans) {
      pendingNotes.pop_front();
      continue;
    }
    if (note.insChange) {
      dispatchCmd(DivCommand(DIV_CMD_INSTRUMENT,note.channel,note.ins,0));
      pendingNotes.pop_front();
      continue;
    }
    if (note.on) {
      if (!(midiIsDirect && midiIsDirectProgram && note.fromMIDI)) {
        dispatchCmd(DivCommand(DIV_CMD_INSTRUMENT,note.channel,note.ins,1));
      }
      if (note.volume>=0 && !disCont[dispatchOfChan[note.channel]].dispatch->isVolGlobal()) {
        float curvedVol=pow((float)note.volume/127.0f,midiVolExp);
        int mappedVol=disCont[dispatchOfChan[note.channel]].dispatch->mapVelocity(dispatchChanOfChan[note.channel],curvedVol);
        dispatchCmd(DivCommand(DIV_CMD_VOLUME,note.channel,mappedVol));
      }
      dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,note.channel,note.note));
      keyHit[note.channel]=true;
      chan[note.channel].note = note.note;
      chan[note.channel].releasing=false;
      chan[note.channel].noteOnInhibit=true;
      chan[note.channel].lastIns=note.ins;
    } else {
      DivMacroInt* macroInt=disCont[dispatchOfChan[note.channel]].dispatch->getChanMacroInt(dispatchChanOfChan[note.channel]);
      if (macroInt!=NULL) {
        if (macroInt->hasRelease && !disCont[dispatchOfChan[note.channel]].dispatch->isVolGlobal()) {
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

      // apply delayed rows before potentially advancing to a new row, which would overwrite the
      // delayed row's state before it has a chance to do anything. a typical example would be
      // a delay scheduling a note-on to be simultaneous with the next row, and the next row also
      // containing a delayed note. if we don't apply the delayed row first, 
      for (int i=0; i<chans; i++) {
        // delay effects
        if (chan[i].rowDelay>0) {
          if (--chan[i].rowDelay==0) {
            processRow(i,true);
          }
        }
      }

      if (stepPlay!=1) {
        tempoAccum+=(skipping && virtualTempoN<virtualTempoD)?virtualTempoD:virtualTempoN;
        while (tempoAccum>=virtualTempoD) {
          tempoAccum-=virtualTempoD;
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
            if (stepPlay==2) {
              stepPlay=1;
              playPosLock.lock();
              prevOrder=curOrder;
              prevRow=curRow;
              playPosLock.unlock();
            }
            nextRow();
            break;
          }
        }
        // under no circumstances shall the accumulator become this large
        if (tempoAccum>1023) tempoAccum=1023;
      }

      // process stuff
      if (!shallStop) for (int i=0; i<chans; i++) {
        // retrigger
        if (chan[i].retrigSpeed) {
          if (--chan[i].retrigTick<0) {
            chan[i].retrigTick=chan[i].retrigSpeed-1;
            dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,i,DIV_NOTE_NULL));
            keyHit[i]=true;
          }
        }

        // volume slides and tremolo
        if (!song.noSlidesOnFirstTick || !firstTick) {
          if (chan[i].volSpeed!=0) {
            chan[i].volume=(chan[i].volume&0xff)|(dispatchCmd(DivCommand(DIV_CMD_GET_VOLUME,i))<<8);
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
                dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
                dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
                dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE,i,0));
              }
            }
            if (chan[i].volume>chan[i].volMax) {
              chan[i].volume=chan[i].volMax;
              chan[i].volSpeed=0;
              chan[i].volSpeedTarget=-1;
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
              chan[i].volSpeedTarget=-1;
              dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
              dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
            } else {
              dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
            }
          } else if (chan[i].tremoloDepth>0) {
            chan[i].tremoloPos+=chan[i].tremoloRate;
            chan[i].tremoloPos&=127;
            dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,MAX(0,chan[i].volume-(tremTable[chan[i].tremoloPos]*chan[i].tremoloDepth))>>8));
          }
        }

        // panning slides
        if (chan[i].panSpeed!=0) {
          int newPanL=chan[i].panL;
          int newPanR=chan[i].panR;
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

          if (newPanL<0) newPanL=0;
          if (newPanL>0xff) newPanL=0xff;
          if (newPanR<0) newPanR=0;
          if (newPanR>0xff) newPanR=0xff;

          chan[i].panL=newPanL;
          chan[i].panR=newPanR;

          dispatchCmd(DivCommand(DIV_CMD_PANNING,i,chan[i].panL,chan[i].panR));
        } else if (chan[i].panDepth>0) {
          chan[i].panPos+=chan[i].panRate;
          chan[i].panPos&=255;

          // calculate inverted...
          switch (chan[i].panPos&0xc0) {
            case 0: // center -> right
              chan[i].panL=((chan[i].panPos&0x3f)<<2);
              chan[i].panR=0;
              break;
            case 0x40: // right -> center
              chan[i].panL=0xff-((chan[i].panPos&0x3f)<<2);
              chan[i].panR=0;
              break;
            case 0x80: // center -> left
              chan[i].panL=0;
              chan[i].panR=((chan[i].panPos&0x3f)<<2);
              break;
            case 0xc0: // left -> center
              chan[i].panL=0;
              chan[i].panR=0xff-((chan[i].panPos&0x3f)<<2);
              break;
          }

          // multiply by depth
          chan[i].panL=(chan[i].panL*chan[i].panDepth)/15;
          chan[i].panR=(chan[i].panR*chan[i].panDepth)/15;

          // then invert it to get final panning
          chan[i].panL^=0xff;
          chan[i].panR^=0xff;

          dispatchCmd(DivCommand(DIV_CMD_PANNING,i,chan[i].panL,chan[i].panR));
        }

        // vibrato
        if (chan[i].vibratoDepth>0) {
          chan[i].vibratoPos+=chan[i].vibratoRate;
          while (chan[i].vibratoPos>=64) chan[i].vibratoPos-=64;

          chan[i].vibratoPosGiant+=chan[i].vibratoRate;
          while (chan[i].vibratoPosGiant>=512) chan[i].vibratoPosGiant-=512;

          int vibratoOut=0;
          switch (chan[i].vibratoShape) {
            case 1: // sine, up only
              vibratoOut=MAX(0,vibTable[chan[i].vibratoPos]);
              break;
            case 2: // sine, down only
              vibratoOut=MIN(0,vibTable[chan[i].vibratoPos]);
              break;
            case 3: // triangle
              vibratoOut=(chan[i].vibratoPos&31);
              if (chan[i].vibratoPos&16) {
                vibratoOut=32-(chan[i].vibratoPos&31);
              }
              if (chan[i].vibratoPos&32) {
                vibratoOut=-vibratoOut;
              }
              vibratoOut<<=3;
              break;
            case 4: // ramp up
              vibratoOut=chan[i].vibratoPos<<1;
              break;
            case 5: // ramp down
              vibratoOut=-chan[i].vibratoPos<<1;
              break;
            case 6: // square
              vibratoOut=(chan[i].vibratoPos>=32)?-127:127;
              break;
            case 7: // random (TODO: use LFSR)
              vibratoOut=(rand()&255)-128;
              break;
            case 8: // square up
              vibratoOut=(chan[i].vibratoPos>=32)?0:127;
              break;
            case 9: // square down
              vibratoOut=(chan[i].vibratoPos>=32)?0:-127;
              break;
            case 10: // half sine up
              vibratoOut=vibTable[chan[i].vibratoPos>>1];
              break;
            case 11: // half sine down
              vibratoOut=vibTable[32|(chan[i].vibratoPos>>1)];
              break;
            default: // sine
              vibratoOut=vibTable[chan[i].vibratoPos];
              break;
          }
          dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibratoOut*chan[i].vibratoFine)>>4)/15)));
        }

        // delayed legato
        if (chan[i].legatoDelay>0) {
          if (--chan[i].legatoDelay<1) {
            chan[i].note+=chan[i].legatoTarget;
            dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
            dispatchCmd(DivCommand(DIV_CMD_HINT_LEGATO,i,chan[i].note));
            chan[i].legatoDelay=-1;
            chan[i].legatoTarget=0;
          }
        }

        // portamento and pitch slides
        if (!song.noSlidesOnFirstTick || !firstTick) {
          if ((chan[i].keyOn || chan[i].keyOff) && chan[i].portaSpeed>0) {
            if (dispatchCmd(DivCommand(DIV_CMD_NOTE_PORTA,i,chan[i].portaSpeed*(song.linearPitch==2?song.pitchSlideSpeed:1),chan[i].portaNote))==2 && chan[i].portaStop && song.targetResetsSlides) {
              chan[i].portaSpeed=0;
              dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
              chan[i].oldNote=chan[i].note;
              chan[i].note=chan[i].portaNote;
              chan[i].inPorta=false;
              dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
              dispatchCmd(DivCommand(DIV_CMD_HINT_LEGATO,i,chan[i].note));
            }
          }
        }

        // note cut
        if (chan[i].cut>0) {
          if (--chan[i].cut<1) {
            if (chan[i].cutType==2) {
              dispatchCmd(DivCommand(DIV_CMD_ENV_RELEASE,i));
              chan[i].releasing=true;
            } else {
              chan[i].oldNote=chan[i].note;
              //chan[i].note=-1;
              if (chan[i].inPorta && song.noteOffResetsSlides) {
                chan[i].keyOff=true;
                chan[i].keyOn=false;
                if (chan[i].stopOnOff) {
                  chan[i].portaNote=-1;
                  chan[i].portaSpeed=-1;
                  dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
                  chan[i].stopOnOff=false;
                }
                if (disCont[dispatchOfChan[i]].dispatch->keyOffAffectsPorta(dispatchChanOfChan[i])) {
                  chan[i].portaNote=-1;
                  chan[i].portaSpeed=-1;
                  dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
                }
                dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
                chan[i].scheduledSlideReset=true;
              }
              if (chan[i].cutType==1) {
                dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF_ENV,i));
              } else {
                dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF,i));
              }
              chan[i].releasing=true;
            }
          }
        }

        // volume cut/mute
        if (chan[i].volCut>0) {
          if (--chan[i].volCut<1) {
            chan[i].volume=0;
            dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
            dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
          }
        }

        // arpeggio
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

  if (subticks==tickMult && cmdStreamInt) {
    if (!cmdStreamInt->tick()) {
      // !!!
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
    // reset all chan oscs
    for (int i=0; i<chans; i++) {
      DivDispatchOscBuffer* buf=disCont[dispatchOfChan[i]].dispatch->getOscBuffer(dispatchChanOfChan[i]);
      if (buf!=NULL) {
        buf->reset();
      }
    }
    return ret;
  }

  // system tick
  for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->tick(subticks==tickMult);

  if (!freelance) {
    if (stepPlay!=1) {
      if (!noAccum) {
        double dt=divider*tickMult;
        if (skipping) {
          dt*=(double)virtualTempoN/(double)MAX(1,virtualTempoD);
        }
        totalTicksR++;
        totalTicks+=1000000/dt;
        totalTicksOff+=fmod(1000000.0,dt);
        while (totalTicksOff>=dt) {
          totalTicksOff-=dt;
          totalTicks++;
        }
      }
      if (totalTicks>=1000000) {
        totalTicks-=1000000;
        if (totalSeconds<0x7fffffff) totalSeconds++;
        cmdsPerSecond=totalCmds-lastCmds;
        lastCmds=totalCmds;
      }
    }

    if (consoleMode && !disableStatusOut && subticks<=1 && !skipping) fprintf(stderr,"\x1b[2K> %d:%.2d:%.2d.%.2d  %.2x/%.2x:%.3d/%.3d  %4dcmd/s\x1b[G",totalSeconds/3600,(totalSeconds/60)%60,totalSeconds%60,totalTicks/10000,curOrder,curSubSong->ordersLen,curRow,curSubSong->patLen,cmdsPerSecond);
  }

  if (haltOn==DIV_HALT_TICK) halted=true;

  return ret;
}

int DivEngine::getBufferPos() {
  return bufferPos;
}

void DivEngine::runMidiClock(int totalCycles) {
  if (freelance) return;
  midiClockCycles-=totalCycles;
  while (midiClockCycles<=0) {
    curMidiClock++;
    if (output) if (!skipping && output->midiOut!=NULL && midiOutClock) {
      output->midiOut->send(TAMidiMessage(TA_MIDI_CLOCK,0,0));
    }

    double hl=curSubSong->hilightA;
    if (hl<=0.0) hl=4.0;
    double timeBase=curSubSong->timeBase+1;
    double speedSum=0;
    double vD=virtualTempoD;
    for (int i=0; i<MIN(16,speeds.len); i++) {
      speedSum+=speeds.val[i];
    }
    speedSum/=MAX(1,speeds.len);
    if (timeBase<1.0) timeBase=1.0;
    if (speedSum<1.0) speedSum=1.0;
    if (vD<1) vD=1;
    double bpm=((24.0*divider)/(timeBase*hl*speedSum))*(double)virtualTempoN/vD;
    if (bpm<1.0) bpm=1.0;
    int increment=got.rate/(bpm);
    if (increment<1) increment=1;

    midiClockCycles+=increment;
    midiClockDrift+=fmod(got.rate,(double)(bpm));
    if (midiClockDrift>=(bpm)) {
      midiClockDrift-=(bpm);
      midiClockCycles++;
    }
  }
}

void DivEngine::runMidiTime(int totalCycles) {
  if (freelance) return;
  if (got.rate<1) return;
  midiTimeCycles-=totalCycles;
  while (midiTimeCycles<=0) {
    if (curMidiTimePiece==0) {
      curMidiTimeCode=curMidiTime;
    }
    if (!(curMidiTimePiece&3)) curMidiTime++;

    double frameRate=96.0;
    int timeRate=midiOutTimeRate;
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

    int hour=0;
    int minute=0;
    int second=0;
    int frame=0;
    int drop=0;
    int actualTime=curMidiTimeCode;
    
    switch (timeRate) {
      case 1: // 24
        frameRate=96.0;
        hour=(actualTime/(60*60*24))%24;
        minute=(actualTime/(60*24))%60;
        second=(actualTime/24)%60;
        frame=actualTime%24;
        break;
      case 2: // 25
        frameRate=100.0;
        hour=(actualTime/(60*60*25))%24;
        minute=(actualTime/(60*25))%60;
        second=(actualTime/25)%60;
        frame=actualTime%25;
        break;
      case 3: // 29.97 (NTSC drop)
        frameRate=120.0*(1000.0/1001.0);

        // drop
        drop=((actualTime/(30*60))-(actualTime/(30*600)))*2;
        actualTime+=drop;

        hour=(actualTime/(60*60*30))%24;
        minute=(actualTime/(60*30))%60;
        second=(actualTime/30)%60;
        frame=actualTime%30;
        break;
      case 4: // 30 (NTSC non-drop)
      default:
        frameRate=120.0;
        hour=(actualTime/(60*60*30))%24;
        minute=(actualTime/(60*30))%60;
        second=(actualTime/30)%60;
        frame=actualTime%30;
        break;
    }

    if (output) if (!skipping && output->midiOut!=NULL && midiOutTime) {
      unsigned char val=0;
      switch (curMidiTimePiece) {
        case 0:
          val=frame&15;
          break;
        case 1:
          val=frame>>4;
          break;
        case 2:
          val=second&15;
          break;
        case 3:
          val=second>>4;
          break;
        case 4:
          val=minute&15;
          break;
        case 5:
          val=minute>>4;
          break;
        case 6:
          val=hour&15;
          break;
        case 7:
          val=(hour>>4)|((timeRate-1)<<1);
          break;
      }
      val|=curMidiTimePiece<<4;
      output->midiOut->send(TAMidiMessage(TA_MIDI_MTC_FRAME,val,0));
    }
    curMidiTimePiece=(curMidiTimePiece+1)&7;

    midiTimeCycles+=got.rate/(frameRate);
    midiTimeDrift+=fmod(got.rate,(double)(frameRate));
    if (midiTimeDrift>=(frameRate)) {
      midiTimeDrift-=(frameRate);
      midiTimeCycles++;
    }
  }
}

void _runDispatch1(void* d) {
}

void _runDispatch2(void* d) {

}

void DivEngine::nextBuf(float** in, float** out, int inChans, int outChans, unsigned int size) {
  lastNBIns=inChans;
  lastNBOuts=outChans;
  lastNBSize=size;

  if (!size) {
    logW("nextBuf called with size 0!");
    return;
  }
  lastLoopPos=-1;

  if (out!=NULL) {
    for (int i=0; i<outChans; i++) {
      memset(out[i],0,size*sizeof(float));
    }
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

  if (renderPool==NULL) {
    unsigned int howManyThreads=song.systemLen;
    if (howManyThreads<2) howManyThreads=0;
    if (howManyThreads>renderPoolThreads) howManyThreads=renderPoolThreads;
    renderPool=new DivWorkPool(howManyThreads);
  }

  // process MIDI events (TODO: everything)
  if (output) if (output->midiIn) while (!output->midiIn->queue.empty()) {
    TAMidiMessage& msg=output->midiIn->queue.front();
    if (midiDebug) {
      if (msg.type==TA_MIDI_SYSEX) {
        logD("MIDI debug: %.2X SysEx",msg.type);
      } else {
        logD("MIDI debug: %.2X %.2X %.2X",msg.type,msg.data[0],msg.data[1]);
      }
    }
    int ins=-1;
    if ((ins=midiCallback(msg))!=-3) {
      int chan=msg.type&15;
      switch (msg.type&0xf0) {
        case TA_MIDI_NOTE_OFF: {
          if (midiIsDirect) {
            if (chan<0 || chan>=chans) break;
            pendingNotes.push_back(DivNoteEvent(chan,-1,-1,-1,false,false,true));
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
          if (msg.data[1]==0) {
            if (midiIsDirect) {
              if (chan<0 || chan>=chans) break;
              pendingNotes.push_back(DivNoteEvent(chan,-1,-1,-1,false,false,true));
            } else {
              autoNoteOff(msg.type&15,msg.data[0]-12,msg.data[1]);
            }
          } else {
            if (midiIsDirect) {
              if (chan<0 || chan>=chans) break;
              pendingNotes.push_back(DivNoteEvent(chan,ins,msg.data[0]-12,msg.data[1],true,false,true));
            } else {
              autoNoteOn(msg.type&15,ins,msg.data[0]-12,msg.data[1]);
            }
          }
          break;
        }
        case TA_MIDI_PROGRAM: {
          if (midiIsDirect && midiIsDirectProgram) {
            pendingNotes.push_back(DivNoteEvent(chan,msg.data[0],0,0,false,true,true));
          }
          break;
        }
      }
    } else if (midiDebug) {
      logD("callback wants ignore");
    }
    //logD("%.2x",msg.type);
    output->midiIn->queue.pop();
  }
  
  // process sample/wave preview
  if (((sPreview.sample>=0 && sPreview.sample<(int)song.sample.size()) || (sPreview.wave>=0 && sPreview.wave<(int)song.wave.size())) && !exporting) {
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
          if (--sPreview.posSub<=0) {
            sPreview.posSub=sPreview.rateMul;
            if (sPreview.dir) {
              sPreview.pos--;
            } else {
              sPreview.pos++;
            }
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
        if (--sPreview.posSub<=0) {
          sPreview.posSub=sPreview.rateMul;
          if (++sPreview.pos>=wave->len) {
            sPreview.pos=0;
          }
        }
        blip_add_delta(samp_bb,i,samp_temp-samp_prevSample);
        samp_prevSample=samp_temp;
      }
    }

    blip_end_frame(samp_bb,prevtotal);
    blip_read_samples(samp_bb,samp_bbOut+samp_bbOff,size-samp_bbOff,0);
  } else {
    memset(samp_bbOut,0,size*sizeof(short));
  }

  // process audio
  bool mustPlay=playing && !halted;
  if (mustPlay) {
    // logic starts here
    for (int i=0; i<song.systemLen; i++) {
      disCont[i].runPos=0;
    }

    if (metroTickLen<size) {
      if (metroTick!=NULL) delete[] metroTick;
      metroTick=new unsigned char[size];
      metroTickLen=size;
    }

    memset(metroTick,0,size);

    int attempts=0;
    int runLeftG=size;
    while (++attempts<(int)size) {
      // -1. set bufferPos
      bufferPos=size-runLeftG;

      // 0. check if we've halted
      if (halted) break;
      // 1. check whether we are done with all buffers
      if (runLeftG<=0) break;

      // 2. check whether we gonna tick
      if (cycles<=0) {
        // we have to tick
        if (nextTick()) {
          /*totalTicks=0;
          totalSeconds=0;*/
          lastLoopPos=size-runLeftG;
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
          unsigned int realPos=size-runLeftG;
          if (realPos>=size) realPos=size-1;
          metroTick[realPos]=pendingMetroTick;
          pendingMetroTick=0;
        }
      } else {
        // 3. run MIDI clock
        int midiTotal=MIN(cycles,runLeftG);
        runMidiClock(midiTotal);

        // 4. run MIDI timecode
        runMidiTime(midiTotal);

        // 5. tick the clock and fill buffers as needed
        if (cycles<runLeftG) {
          // run until the end of this tick
          for (int i=0; i<song.systemLen; i++) {
            disCont[i].cycles=cycles;
            disCont[i].size=size;
            renderPool->push([](void* d) {
              DivDispatchContainer* dc=(DivDispatchContainer*)d;

              int lastAvail=blip_samples_avail(dc->bb[0]);
              if (lastAvail>0) {
                if (lastAvail>=dc->cycles) {
                  dc->flush(dc->runPos,dc->cycles);
                  dc->runPos+=dc->cycles;
                  return;
                } else {
                  dc->flush(dc->runPos,lastAvail);
                  dc->runPos+=lastAvail;
                  dc->cycles-=lastAvail;
                }
              }
              
              int total=blip_clocks_needed(dc->bb[0],dc->cycles);
              if (total>(int)dc->bbInLen) {
                logD("growing dispatch %p bbIn to %d",(void*)dc,total+256);
                dc->grow(total+256);
              }
              dc->acquire(total);
              dc->fillBuf(total,dc->runPos,dc->cycles);
              dc->runPos+=dc->cycles;
            },&disCont[i]);
          }
          renderPool->wait();
          runLeftG-=cycles;
          cycles=0;
        } else {
          // run until the end of this audio buffer
          cycles-=runLeftG;
          for (int i=0; i<song.systemLen; i++) {
            disCont[i].cycles=runLeftG;
            renderPool->push([](void* d) {
              DivDispatchContainer* dc=(DivDispatchContainer*)d;

              int lastAvail=blip_samples_avail(dc->bb[0]);
              if (lastAvail>0) {
                if (lastAvail>=dc->cycles) {
                  dc->flush(dc->runPos,dc->cycles);
                  dc->runPos+=dc->cycles;
                  return;
                } else {
                  dc->flush(dc->runPos,lastAvail);
                  dc->runPos+=lastAvail;
                  dc->cycles-=lastAvail;
                }
              }

              int total=blip_clocks_needed(dc->bb[0],dc->cycles);
              if (total>(int)dc->bbInLen) {
                logD("growing dispatch %p bbIn to %d",(void*)dc,total+256);
                dc->grow(total+256);
              }
              dc->acquire(total);
              dc->fillBuf(total,dc->runPos,dc->cycles);
            },&disCont[i]);
          }
          runLeftG=0;
          renderPool->wait();
        }
      }
    }

    //logD("attempts: %d",attempts);
    if (attempts>=(int)(size+10)) {
      logE("hang detected! stopping! at %d seconds %d micro (%d>=%d)",totalSeconds,totalTicks,attempts,(int)size);
      freelance=false;
      playing=false;
      extValuePresent=false;
    }
    totalProcessed=size-runLeftG;

    for (int i=0; i<song.systemLen; i++) {
      if (size<disCont[i].lastAvail) {
        logW("%d: size<lastAvail! %d<%d",i,size,disCont[i].lastAvail);
        continue;
      }
      disCont[i].size=size;
      /*
      renderPool->push([](void* d) {
        DivDispatchContainer* dc=(DivDispatchContainer*)d;
        dc->fillBuf(dc->runtotal,dc->lastAvail,dc->size-dc->lastAvail);
      },&disCont[i]);*/
    }
    renderPool->wait();
  }

  // process metronome
  if (metroBufLen<size || metroBuf==NULL) {
    if (metroBuf!=NULL) delete[] metroBuf;
    metroBuf=new float[size];
    metroBufLen=size;
  }

  memset(metroBuf,0,metroBufLen*sizeof(float));

  if (mustPlay && metronome && !freelance) {
    for (size_t i=0; i<size; i++) {
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
        for (int j=0; j<outChans; j++) {
          metroBuf[i]=(sin(metroPos*2*M_PI))*metroAmp*metroVol;
        }
      }
      metroAmp-=0.0003f;
      if (metroAmp<0.0f) metroAmp=0.0f;
      metroPos+=metroFreq;
      while (metroPos>=1) metroPos--;
    }
  }

  // resolve patchbay
  for (unsigned int i: song.patchbay) {
    const unsigned short srcPort=i>>16;
    const unsigned short destPort=i&0xffff;

    const unsigned short srcPortSet=srcPort>>4;
    const unsigned short destPortSet=destPort>>4;
    const unsigned char srcSubPort=srcPort&15;
    const unsigned char destSubPort=destPort&15;

    // null portset
    if (destPortSet==0xfff) continue;

    // system outputs
    if (destPortSet==0x000) {
      if (destSubPort>=outChans) continue;

      // chip outputs
      if (srcPortSet<song.systemLen && playing && !halted) {
        if (srcSubPort<disCont[srcPortSet].dispatch->getOutputCount()) {
          float vol=song.systemVol[srcPortSet]*disCont[srcPortSet].dispatch->getPostAmp()*song.masterVol;

          switch (destSubPort&3) {
            case 0:
              vol*=MIN(1.0f,1.0f-song.systemPan[srcPortSet])*MIN(1.0f,1.0f+song.systemPanFR[srcPortSet]);
              break;
            case 1:
              vol*=MIN(1.0f,1.0f+song.systemPan[srcPortSet])*MIN(1.0f,1.0f+song.systemPanFR[srcPortSet]);
              break;
            case 2:
              vol*=MIN(1.0f,1.0f-song.systemPan[srcPortSet])*MIN(1.0f,1.0f-song.systemPanFR[srcPortSet]);
              break;
            case 3:
              vol*=MIN(1.0f,1.0f+song.systemPan[srcPortSet])*MIN(1.0f,1.0f-song.systemPanFR[srcPortSet]);
              break;
          }

          for (size_t j=0; j<size; j++) {
            out[destSubPort][j]+=((float)disCont[srcPortSet].bbOut[srcSubPort][j]/32768.0)*vol;
          }
        }
      } else if (srcPortSet==0xffd) {
        // sample preview
        for (size_t j=0; j<size; j++) {
          out[destSubPort][j]+=previewVol*(samp_bbOut[j]/32768.0);
        }
      } else if (srcPortSet==0xffe && playing && !halted) {
        // metronome
        for (size_t j=0; j<size; j++) {
          out[destSubPort][j]+=metroBuf[j];
        }
      }

      // nothing/invalid
    }

    // nothing/invalid
  }

  // dump to oscillator buffer
  for (unsigned int i=0; i<size; i++) {
    for (int j=0; j<outChans; j++) {
      if (oscBuf[j]==NULL) continue;
      oscBuf[j][oscWritePos]=out[j][i];
    }
    if (++oscWritePos>=32768) oscWritePos=0;
  }
  oscSize=size;

  // get per-chip peaks
  float decay=2.f*size/got.rate;
  for (int i=0; i<song.systemLen; i++) {
    DivDispatch* disp=disCont[i].dispatch;
    for (int j=0; j<disp->getOutputCount(); j++) {
      chipPeak[i][j]*=1.0-decay;
      float peak=chipPeak[i][j];
      for (unsigned int k=0; k<size; k++) {
        float out=disCont[i].bbOut[j][k]*song.systemVol[i]*disp->getPostAmp()/32768.0f; // TODO: PARSE PANNING, FRONT/REAR AND PATCHBAY
        // switch (j) {
        //   case 0:
        //     out*=MIN(1.0f,1.0f-song.systemPan[i])*MIN(1.0f,1.0f+song.systemPanFR[i]);
        //     break;
        //   case 1:
        //     out*=MIN(1.0f,1.0f+song.systemPan[i])*MIN(1.0f,1.0f+song.systemPanFR[i]);
        //     break;
        //   case 2:
        //     out*=MIN(1.0f,1.0f-song.systemPan[i])*MIN(1.0f,1.0f-song.systemPanFR[i]);
        //     break;
        //   case 3:
        //     out*=MIN(1.0f,1.0f+song.systemPan[i])*MIN(1.0f,1.0f-song.systemPanFR[i]);
        //     break;
        //   default: break;
        // }
        if (out>peak) peak=out;
      }
      chipPeak[i][j]+=(peak-chipPeak[i][j])*0.9;
    }
  }

  // force mono audio (if enabled)
  if (forceMono && outChans>1) {
    for (size_t i=0; i<size; i++) {
      float chanSum=out[0][i];
      for (int j=1; j<outChans; j++) {
        chanSum+=out[j][i];
      }
      out[0][i]=chanSum/outChans;
      for (int j=1; j<outChans; j++) {
        out[j][i]=out[0][i];
      }
    }
  }

  // clamp output (if enabled)
  if (clampSamples) {
    for (size_t i=0; i<size; i++) {
      for (int j=0; j<outChans; j++) {
        if (out[j][i]<-1.0) out[j][i]=-1.0;
        if (out[j][i]>1.0) out[j][i]=1.0;
      }
    }
  }
  isBusy.unlock();

  std::chrono::steady_clock::time_point ts_processEnd=std::chrono::steady_clock::now();

  processTime=std::chrono::duration_cast<std::chrono::nanoseconds>(ts_processEnd-ts_processBegin).count();
}
