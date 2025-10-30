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

// this file contains most of the playback code.
// it is a mess due to the amount of compatibility flags that have been
// added over time, so I have tried my best to comment it.
// these compatibility flags are preceded by a "COMPAT FLAG" comment upon use.

#include "macroInt.h"
#include <chrono>
#define _USE_MATH_DEFINES
#include "dispatch.h"
#include "engine.h"
#include "workPool.h"
#include "../ta-log.h"
#include <math.h>

// go to next order
void DivEngine::nextOrder() {
  curRow=0;
  if (repeatPattern) return;
  if (++curOrder>=curSubSong->ordersLen) {
    logV("end of orders reached");
    endOfSong=true;
    // the walked array is used for loop detection
    // since we've reached the end, we are guaranteed to loop here, so
    // just reset it.
    memset(walked,0,8192);
    curOrder=0;
  }
}

// used for the pattern visualizer in console mode.
static const char* notes[12]={
  "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
};

// update this when adding new commands in dispatch.h.
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

// fail build if you forgot to update the array
static_assert((sizeof(cmdName)/sizeof(void*))==DIV_CMD_MAX,"update cmdName!");

// formats a note
// used for the pattern visualizer in console mode, justifying the use
// of a static array.
const char* formatNote(short note) {
  static char ret[16];
  if (note==DIV_NOTE_OFF) {
    return "OFF";
  } else if (note==DIV_NOTE_REL) {
    return "===";
  } else if (note==DIV_MACRO_REL) {
    return "REL";
  } else if (note<0) {
    return "---";
  }
  snprintf(ret,16,"%s%d",notes[note%12],(note-60)/12);
  return ret;
}

// send a command to a dispatch.
int DivEngine::dispatchCmd(DivCommand c) {
  // used for the commands visualizer in console mode
  if (view==DIV_STATUS_COMMANDS) {
    // don't print if we are "skipping" (seeking to a position, usually after channel reset on loop)
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
          // print command
          printf("%8d | %d: %s(%d, %d)\n",totalTicksR,c.chan,cmdName[c.cmd],c.value,c.value2);
      }
    }
  }
  totalCmds++;
  // up to 2000 commands can be queued in the command queue (used by the GUI for pattern visualizer)
  if (cmdStreamEnabled && cmdStream.size()<2000) {
    cmdStream.push_back(c);
  }

  // MIDI output code
  // we turn this command into MIDI messages if the output mode is "melodic"
  // if the channel is outside the range 0-15, it will be wrapped back
  if (output) if (!skipping && output->midiOut!=NULL && !isChannelMuted(c.chan)) {
    if (output->midiOut->isDeviceOpen()) {
      if (midiOutMode==DIV_MIDI_MODE_NOTE) {
        // scale volume to MIDI velocity range
        int scaledVol=(chan[c.chan].volume*127)/MAX(1,chan[c.chan].volMax);
        if (scaledVol<0) scaledVol=0;
        if (scaledVol>127) scaledVol=127;

        // process the command
        switch (c.cmd) {
          case DIV_CMD_NOTE_ON:
          case DIV_CMD_LEGATO:
            // turn the previous note off (if we have one)
            if (chan[c.chan].curMidiNote>=0) {
              output->midiOut->send(TAMidiMessage(0x80|(c.chan&15),chan[c.chan].curMidiNote,scaledVol));
            }
            // set current MIDI note
            if (c.value!=DIV_NOTE_NULL) {
              chan[c.chan].curMidiNote=c.value+12;
              if (chan[c.chan].curMidiNote<0) chan[c.chan].curMidiNote=0;
              if (chan[c.chan].curMidiNote>127) chan[c.chan].curMidiNote=127;
            }
            // send note on (if we have one)
            if (chan[c.chan].curMidiNote>=0) {
              output->midiOut->send(TAMidiMessage(0x90|(c.chan&15),chan[c.chan].curMidiNote,scaledVol));
            }
            break;
          case DIV_CMD_NOTE_OFF:
          case DIV_CMD_NOTE_OFF_ENV:
            // turn the current note off (if we have one)
            // we don't do this for macro release...
            if (chan[c.chan].curMidiNote>=0) {
              output->midiOut->send(TAMidiMessage(0x80|(c.chan&15),chan[c.chan].curMidiNote,scaledVol));
            }
            chan[c.chan].curMidiNote=-1;
            break;
          case DIV_CMD_INSTRUMENT:
            // instrument changes mapped to program change
            // only first 128 instruments
            if (chan[c.chan].lastIns!=c.value && midiOutProgramChange) {
              output->midiOut->send(TAMidiMessage(0xc0|(c.chan&15),c.value&0x7f,0));
            }
            break;
          case DIV_CMD_VOLUME:
            // volume changes are sent as MIDI aftertouch, as long as there isn't a note
            // (processRow will set midiAftertouch to true on every row without note)
            if (chan[c.chan].curMidiNote>=0 && chan[c.chan].midiAftertouch) {
              chan[c.chan].midiAftertouch=false;
              output->midiOut->send(TAMidiMessage(0xa0|(c.chan&15),chan[c.chan].curMidiNote,scaledVol));
            }
            break;
          case DIV_CMD_PITCH: {
            // map pitch changes to pitch bend (including vibrato)
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
            // this is mapped to General MIDI panning CC
            int pan=convertPanSplitToLinearLR(c.value,c.value2,127);
            if (pan<0) pan=0;
            if (pan>127) pan=127;
            output->midiOut->send(TAMidiMessage(0xb0|(c.chan&15),0x0a,pan));
            break;
          }
          case DIV_CMD_HINT_PORTA: {
            // portamento handling is complicated
            // in General MIDI, portamento consists of sending a CC for duration and another for target note
            // this differs from Furnace, where the parameter is speed rather than duration
            // it is also impossible to perform an indefinite slide down/up other than
            // by using pitch bend, but the default range is limited and we already
            // use it for pitch changes/vibrato

            // only send portamento if it is enabling
            if (c.value2>0) {
              // and only if we have a target note
              if (c.value<=0 || c.value>=255) break;
              //output->midiOut->send(TAMidiMessage(0x80|(c.chan&15),chan[c.chan].curMidiNote,scaledVol));
              int target=c.value+12;
              if (target<0) target=0;
              if (target>127) target=127;
              
              // set the source note?
              if (chan[c.chan].curMidiNote>=0) {
                output->midiOut->send(TAMidiMessage(0xb0|(c.chan&15),0x54,chan[c.chan].curMidiNote));
              }
              // set the duration
              // no effort whatsoever is done to predict how long will the slide last
              output->midiOut->send(TAMidiMessage(0xb0|(c.chan&15),0x05,1/*MIN(0x7f,c.value2/4)*/));
              // turn portamento on
              output->midiOut->send(TAMidiMessage(0xb0|(c.chan&15),0x41,0x7f));
              // send a note on (why?)
              output->midiOut->send(TAMidiMessage(0x90|(c.chan&15),target,scaledVol));
            } else {
              // disable portamento otherwise
              output->midiOut->send(TAMidiMessage(0xb0|(c.chan&15),0x41,0));
            }
            break;
          }
          // other commands are simply ignored
          default:
            break;
        }
      }
    }
  }

  // map the channel to channel of chip
  // c.dis is a copy of c.chan because we'll use it in the next call
  c.chan=dispatchChanOfChan[c.dis];

  // dispatch command to chip dispatch
  return disCont[dispatchOfChan[c.dis]].dispatch->dispatch(c);
}

// this function handles per-chip normal effects
bool DivEngine::perSystemEffect(int ch, unsigned char effect, unsigned char effectVal) {
  // don't process invalid chips
  DivSysDef* sysDef=sysDefs[sysOfChan[ch]];
  if (sysDef==NULL) return false;
  // find the effect handler
  auto iter=sysDef->effectHandlers.find(effect);
  if (iter==sysDef->effectHandlers.end()) return false;
  EffectHandler handler=iter->second;
  int val=0;
  int val2=0;
  // map values using the handler's function
  try {
    val=handler.val?handler.val(effect,effectVal):effectVal;
    val2=handler.val2?handler.val2(effect,effectVal):0;
  } catch (DivDoNotHandleEffect& e) {
    return false;
  }
  // dispatch command
  // wouldn't this cause problems if it were to return 0?
  return dispatchCmd(DivCommand(handler.dispatchCmd,ch,val,val2));
}

// this handles per-chip post effects...
bool DivEngine::perSystemPostEffect(int ch, unsigned char effect, unsigned char effectVal) {
  // don't process invalid chips
  DivSysDef* sysDef=sysDefs[sysOfChan[ch]];
  if (sysDef==NULL) return false;
  // find the effect handler
  auto iter=sysDef->postEffectHandlers.find(effect);
  if (iter==sysDef->postEffectHandlers.end()) return false;
  EffectHandler handler=iter->second;
  int val=0;
  int val2=0;
  // map values using the handler's function
  try {
    val=handler.val?handler.val(effect,effectVal):effectVal;
    val2=handler.val2?handler.val2(effect,effectVal):0;
  } catch (DivDoNotHandleEffect& e) {
    return true;
  }
  // dispatch command
  // wouldn't this cause problems if it were to return 0?
  return dispatchCmd(DivCommand(handler.dispatchCmd,ch,val,val2));
}

// ...and this handles chip pre-effects
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

// this is called by nextRow() before it calls processRow()
// `i` is the channel
void DivEngine::processRowPre(int i) {
  int whatOrder=curOrder;
  int whatRow=curRow;
  DivPattern* pat=curPat[i].getPattern(curOrders->ord[i][whatOrder],false);
  // check all effects
  for (int j=0; j<curPat[i].effectCols; j++) {
    short effect=pat->newData[whatRow][DIV_PAT_FX(j)];
    short effectVal=pat->newData[whatRow][DIV_PAT_FXVAL(j)];

    // empty effect value is the same as zero
    if (effectVal==-1) effectVal=0;
    effectVal&=255;

    // per-chip pre-effects (that's it for now!)
    // the other pre-effects are handled in processRow()
    perSystemPreEffect(i,effect,effectVal);
  }
}

// this is called by nextRow() or nextTick() (in the case of delay). it processes the next row for a channel.
// `i` is the channel and `afterDelay` determines whether this happens after EDxx or not.
// the processing order is:
// 1. pre-effects (delay and song control)
// 2. instrument
// 3. note reading (note off is done immediately)
// 4. volume
// 5. effects
// 6. note on
// 7. post-effects
void DivEngine::processRow(int i, bool afterDelay) {
  // if this is after delay, use the order/row where delay occurred
  int whatOrder=afterDelay?chan[i].delayOrder:curOrder;
  int whatRow=afterDelay?chan[i].delayRow:curRow;
  DivPattern* pat=curPat[i].getPattern(curOrders->ord[i][whatOrder],false);
  // pre effects
  // these include song control ones such as speed, tempo or jumps which shall not be delayed
  // it also includes EDxx (delay) itself so we can handle it
  if (!afterDelay) {
    // set to true if we found an EDxx effect
    bool returnAfterPre=false;
    // check all effects
    for (int j=0; j<curPat[i].effectCols; j++) {
      short effect=pat->newData[whatRow][DIV_PAT_FX(j)];
      short effectVal=pat->newData[whatRow][DIV_PAT_FXVAL(j)];

      // empty effect value is the same as zero
      if (effectVal==-1) effectVal=0;
      effectVal&=255;

      switch (effect) {
        case 0x09: // select groove pattern/speed 1
          if (song.grooves.empty()) {
            // special case: sets speed 1 if the song lacks groove patterns
            if (effectVal>0) speeds.val[0]=effectVal;
          } else {
            // sets the groove pattern and resets current speed index
            if (effectVal<(short)song.grooves.size()) {
              speeds=song.grooves[effectVal];
              curSpeed=0;
            }
          }
          break;
        case 0x0f: // speed 1/speed 2
          // if the value is 0 then ignore it
          if (speeds.len==2 && song.grooves.empty()) {
            // if there are two speeds and no groove patterns, set the second speed
            if (effectVal>0) speeds.val[1]=effectVal;
          } else {
            // otherwise set the first speed
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
          // this actually schedules an order change
          // we perform this change at the end of nextRow()

          // COMPAT FLAG: simultaneous jump treatment
          // - 0: normal (another 0Bxx effect will override the previous one)
          // - 1: old Furnace (only the first 0Bxx effect in a row takes effect)
          // - 2: DefleMask (same as 1)
          // in the case of normal, the jump row (changePos) is not reset to 0
          // this means that you can do 0Dxx 0Byy and it'll work, taking you to row xx of order yy
          if (changeOrd==-1 || song.jumpTreatment==0) {
            changeOrd=effectVal;
            if (song.jumpTreatment==1 || song.jumpTreatment==2) {
              changePos=0;
            }
          }
          break;
        case 0x0d: // next order
          // COMPAT FLAG: ignore 0Dxx on the last order (ignoreJumpAtEnd)
          // if there is a 0Dxx effect on the very last order, it is ignored

          // COMPAT FLAG: simultaneous jump treatment
          if (song.jumpTreatment==2) {
            // - 2: DefleMask (jump to next order unless it is the last one and ignoreJumpAtEnd is on)
            if ((curOrder<(curSubSong->ordersLen-1) || !song.ignoreJumpAtEnd)) {
              // changeOrd -2 means increase order by 1
              // it overrides a previous 0Bxx effect
              changeOrd=-2;
              changePos=effectVal;
            }
          } else if (song.jumpTreatment==1) {
            // - 1: old Furnace (same as 2 but ignored if 0Bxx is present)
            if (changeOrd<0 && (curOrder<(curSubSong->ordersLen-1) || !song.ignoreJumpAtEnd)) {
              changeOrd=-2;
              changePos=effectVal;
            }
          } else {
            // - 0: normal
            if (curOrder<(curSubSong->ordersLen-1) || !song.ignoreJumpAtEnd) {
              // set the target order if not set, allowing you to use 0B and 0D regardless of position
              if (changeOrd<0) {
                changeOrd=-2;
              }
              changePos=effectVal;
            }
          }
          break;
        case 0xed: // delay
          if (effectVal!=0) {
            // COMPAT FLAG: cut/delay effect policy (delayBehavior)
            // - 0: strict
            //   - delays equal or greater to the speed * timeBase are ignored
            // - 1: strict old
            //   - delays equal or greater to the speed are ignored
            // - 2: lax (default)
            //   - no delay is ever ignored unless overridden by another
            bool comparison=(song.delayBehavior==1)?(effectVal<=nextSpeed):(effectVal<(nextSpeed*(curSubSong->timeBase+1)));
            if (song.delayBehavior==2) comparison=true;
            if (comparison) {
              // set the delay row, order and timer
              chan[i].rowDelay=effectVal;
              chan[i].delayOrder=whatOrder;
              chan[i].delayRow=whatRow;

              // this here was a compatibility hack for DefleMask...
              // if the delay time happens to be equal to the speed, it'll
              // result in "delay lock" which halts all row processing
              // until another good EDxx effect is found
              // for some reason this didn't occur on Neo Geo...
              // this hack is disabled due to its dirtiness and the fact I
              // don't feel like being compatible with a buggy tracker any further
              if (effectVal==nextSpeed) {
                //if (sysOfChan[i]!=DIV_SYSTEM_YM2610 && sysOfChan[i]!=DIV_SYSTEM_YM2610_EXT) chan[i].delayLocked=true;
              } else {
                chan[i].delayLocked=false;
              }

              // once we're done with pre-effects, get out and don't process any further
              returnAfterPre=true;
            } else {
              logV("higher than nextSpeed! %d>%d",effectVal,nextSpeed);
              chan[i].delayLocked=false;
            }
          }
          break;
      }
    }
    // stop processing if EDxx was found
    if (returnAfterPre) return;
  } else {
    //logV("honoring delay at position %d",whatRow);
  }

  // stop processing if delay lock is on (won't happen, ever)
  if (chan[i].delayLocked) return;

  // now we start reading...
  // instrument
  bool insChanged=false;
  if (pat->newData[whatRow][DIV_PAT_INS]!=-1) {
    // only send an instrument change if it differs from the current ins
    if (chan[i].lastIns!=pat->newData[whatRow][DIV_PAT_INS]) {
      dispatchCmd(DivCommand(DIV_CMD_INSTRUMENT,i,pat->newData[whatRow][DIV_PAT_INS]));
      chan[i].lastIns=pat->newData[whatRow][DIV_PAT_INS];
      insChanged=true;

      // COMPAT FLAG: legacy volume slides
      // - sets volume to max once a vol slide down has finished (thus setting volume to volMax+1)
      if (song.legacyVolumeSlides && chan[i].volume==chan[i].volMax+1) {
        logV("forcing volume");
        chan[i].volume=chan[i].volMax;
        dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
      }
    }
  }

  // note reading
  // note offs are sent immediately
  if (pat->newData[whatRow][DIV_PAT_NOTE]==DIV_NOTE_OFF) { // note off
    chan[i].keyOn=false;
    chan[i].keyOff=true;

    // COMPAT FLAG: reset slides on note off (inverted in the GUI)
    // - a portamento/pitch slide will be halted upon encountering note off
    // - this will not occur if the stopPortaOnNoteOff flag is on and this is a portamento
    if (chan[i].inPorta && song.noteOffResetsSlides) {
      // stopOnOff will be false if stopPortaOnNoteOff flag is off
      if (chan[i].stopOnOff) {
        chan[i].portaNote=-1;
        chan[i].portaSpeed=-1;
        dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
        chan[i].stopOnOff=false;
      }
      // depending on the system, portamento may still be disabled
      if (disCont[dispatchOfChan[i]].dispatch->keyOffAffectsPorta(dispatchChanOfChan[i])) {
        chan[i].portaNote=-1;
        chan[i].portaSpeed=-1;
        dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
        // this here is a now-disabled hack which makes the noise channel also stop when square 3 is
        /*if (i==2 && sysOfChan[i]==DIV_SYSTEM_SMS) {
          chan[i+1].portaNote=-1;
          chan[i+1].portaSpeed=-1;
        }*/
      }
      // another compatibility hack which schedules a second reset later just in case
      chan[i].scheduledSlideReset=true;
    }

    // send note off
    dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF,i));
  } else if (pat->newData[whatRow][DIV_PAT_NOTE]==DIV_NOTE_REL) { // note off + env release
    //chan[i].note=-1;
    chan[i].keyOn=false;
    chan[i].keyOff=true;
    // same thing here regarding reset slide behavior
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

    // send note release
    dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF_ENV,i));
    chan[i].releasing=true;
  } else if (pat->newData[whatRow][DIV_PAT_NOTE]==DIV_MACRO_REL) { // env release
    // send macro release
    dispatchCmd(DivCommand(DIV_CMD_ENV_RELEASE,i));
    chan[i].releasing=true;
  } else if (pat->newData[whatRow][DIV_PAT_NOTE]!=-1) {
    // prepare/schedule a new note
    chan[i].oldNote=chan[i].note;
    chan[i].note=pat->newData[whatRow][DIV_PAT_NOTE]-60;
    // I have no idea why is this check here since keyOn is guaranteed to be false at this point
    // ...unless there's a way to trigger keyOn twice
    if (!chan[i].keyOn) {
      // the behavior of arpeggio reset upon note off varies per system
      if (disCont[dispatchOfChan[i]].dispatch->keyOffAffectsArp(dispatchChanOfChan[i])) {
        chan[i].arp=0;
        dispatchCmd(DivCommand(DIV_CMD_HINT_ARPEGGIO,i,chan[i].arp));
      }
    }
    chan[i].doNote=true;
    // COMPAT FLAG: compatible arpeggio
    // - once a new note plays, arp will not be applied for this tick
    if (chan[i].arp!=0 && song.compatibleArpeggio) {
      chan[i].arpYield=true;
    }
  }

  // volume
  int volPortaTarget=-1;
  bool noApplyVolume=false;
  // here we read all effects and check for a volume slide with target/volume "portamento"/"scivolando" (a term I invented as an equivalent)
  for (int j=0; j<curPat[i].effectCols; j++) {
    short effect=pat->newData[whatRow][DIV_PAT_FX(j)];
    if (effect==0xd3 || effect==0xd4) { // vol porta
      volPortaTarget=pat->newData[whatRow][DIV_PAT_VOL]<<8; // can be -256

      // empty effect value is treated as 0
      short effectVal=pat->newData[whatRow][DIV_PAT_FXVAL(j)];
      if (effectVal==-1) effectVal=0;
      effectVal&=255;

      noApplyVolume=effectVal>0; // "D3.." or "D300" shouldn't stop volume from applying
      break; // technically you could have both D3 and D4... let's not care
    }
  }
  
  // don't apply volume if a scivolando is set
  if (pat->newData[whatRow][DIV_PAT_VOL]!=-1 && !noApplyVolume) {
    // COMPAT FLAG: legacy ALWAYS_SET_VOLUME behavior (oldAlwaysSetVolume)
    // - prior to its addition, volume changes wouldn't be effective depending on the system if the volume is the same as the current one
    // - afterwards, volume change is made regardless in order to set the bottom byte of volume ("subvolume")
    if (!song.oldAlwaysSetVolume || disCont[dispatchOfChan[i]].dispatch->getLegacyAlwaysSetVolume() || (MIN(chan[i].volMax,chan[i].volume)>>8)!=pat->newData[whatRow][DIV_PAT_VOL]) {
      // here we let dispatchCmd() know we can do MIDI aftertouch if there isn't a note
      if (pat->newData[whatRow][DIV_PAT_NOTE]==-1) {
        chan[i].midiAftertouch=true;
      }
      // set the volume (bottom byte is set to 0)
      chan[i].volume=pat->newData[whatRow][DIV_PAT_VOL]<<8;
      dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
      dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
    }
  }

  // reset retrigger status
  // this is the only effect that takes place only in the row it is placed, in a ProTracker-like fashion (why?)
  chan[i].retrigSpeed=0;

  // stuff necessary for effect processing
  short lastSlide=-1;
  bool calledPorta=false;
  bool panChanged=false;
  bool surroundPanChanged=false;
  bool sampleOffSet=false;

  // effects
  for (int j=0; j<curPat[i].effectCols; j++) {
    short effect=pat->newData[whatRow][DIV_PAT_FX(j)];
    short effectVal=pat->newData[whatRow][DIV_PAT_FXVAL(j)];

    // an empty effect value is treated as zero
    if (effectVal==-1) effectVal=0;
    effectVal&=255;

    // per-system effect
    // if there isn't one, go through normal effects
    if (!perSystemEffect(i,effect,effectVal)) switch (effect) {
      /// PANNING
      case 0x08: // panning (split 4-bit)
        chan[i].panL=(effectVal>>4)|(effectVal&0xf0);
        chan[i].panR=(effectVal&15)|((effectVal&15)<<4);
        // panning command isn't sent until later
        panChanged=true;
        break;
      case 0x80: { // panning (linear)
        // convert to splir
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
          // set the pan slide speed
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
        // send hint (for command stream export)
        dispatchCmd(DivCommand(DIV_CMD_HINT_PAN_SLIDE,i,chan[i].panSpeed&0xff));
        break;
      case 0x84: // panbrello
        if (chan[i].panDepth==0) {
          chan[i].panPos=0;
        }
        chan[i].panDepth=effectVal&15;
        chan[i].panRate=effectVal>>4;
        if (chan[i].panDepth!=0) {
          // panbrello and slides are incompatible
          chan[i].panSpeed=0;
        }
        // send hint (for command stream export)
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
      /// PITCH and more
      // internally, slides and portamento share the same variables
      case 0x01: // pitch slide up
        // COMPAT FLAG: ignore duplicate slides
        // - only the first 01xx effect is considered
        // - 02xx still works
        // - a previous portamento (03xx) will prevent this slide from occurring
        // - E1xy/E2xy also will if *another* flag is set
        if (song.ignoreDuplicateSlides && (lastSlide==0x01 || lastSlide==0x1337)) break;
        lastSlide=0x01;
        if (effectVal==0) {
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
          chan[i].inPorta=false;
          // COMPAT FLAG: arpeggio inhibits non-porta slides
          // - the PRE_PORTA command is used to let the dispatch know we're entering a pitch slide
          // - this prompts dispatch to stop processing arp macros during a slide
          // - this only happens if pitch linearity is set to None
          // - if we don't let dispatch know, the slide will never occur as arp takes over
          if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        } else {
          // COMPAT FLAG: limit slide range
          // - this confines pitch slides from dispatch->getPortaFloor to C-8 (I think)
          // - yep, the lowest portamento note depends on the system...
          chan[i].portaNote=song.limitSlides?0x60:255;
          chan[i].portaSpeed=effectVal;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
          // most of these are used for compat flag handling
          chan[i].portaStop=true;
          chan[i].stopOnOff=false;
          chan[i].scheduledSlideReset=false;
          chan[i].wasShorthandPorta=false;
          chan[i].inPorta=false;
          // COMPAT FLAG: arpeggio inhibits non-porta slides
          // - the PRE_PORTA command is used to let the dispatch know we're entering a pitch slide
          // - this prompts dispatch to stop processing arp macros during a slide
          // - this only happens if pitch linearity is set to None
          // - if we don't let dispatch know, the slide will never occur as arp takes over
          if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,0));
        }
        break;
      case 0x02: // pitch slide down
        // COMPAT FLAG: ignore duplicate slides
        // - only the first 02xx effect is considered
        // - 01xx still works
        // - a previous portamento (03xx) will prevent this slide from occurring
        // - E1xy/E2xy also will if *another* flag is set
        if (song.ignoreDuplicateSlides && (lastSlide==0x02 || lastSlide==0x1337)) break;
        lastSlide=0x02;
        if (effectVal==0) {
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
          chan[i].inPorta=false;
          // COMPAT FLAG: arpeggio inhibits non-porta slides
          if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        } else {
          // COMPAT FLAG: limit slide range
          // - this confines pitch slides from dispatch->getPortaFloor to C-8 (I think)
          // - yep, the lowest portamento note depends on the system...
          chan[i].portaNote=song.limitSlides?disCont[dispatchOfChan[i]].dispatch->getPortaFloor(dispatchChanOfChan[i]):-60;
          chan[i].portaSpeed=effectVal;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
          chan[i].portaStop=true;
          chan[i].stopOnOff=false;
          chan[i].scheduledSlideReset=false;
          chan[i].wasShorthandPorta=false;
          chan[i].inPorta=false;
          // COMPAT FLAG: arpeggio inhibits non-porta slides
          if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,0));
        }
        break;
      case 0x03: // portamento
        // exception: the arpNonPorta flag is not checked here.
        // a portamento shall override arp macros on non-linear pitch.
        if (effectVal==0) {
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
          chan[i].inPorta=false;
          dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        } else {
          // lastPorta is used for the 06xy effect
          chan[i].lastPorta=effectVal;
          // this here is for a compatibility flag...
          calledPorta=true;
          // COMPAT FLAG: buggy portamento after sliding
          // - you might want to slide up or down and then 03xx to return to the original note
          // - if a porta to the same note is attempted after slide, for some reason it does not occur
          if (chan[i].note==chan[i].oldNote && !chan[i].inPorta && song.buggyPortaAfterSlide) {
            chan[i].portaNote=chan[i].note;
            chan[i].portaSpeed=-1;
          } else {
            // compat flags get on my way
            chan[i].portaNote=chan[i].note;
            chan[i].portaSpeed=effectVal;
            chan[i].inPorta=true;
            // ...but this one is for ANOTHER compat flag. yuck!
            chan[i].wasShorthandPorta=false;
          }
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
          // TODO; portaStop is guaranteed to be true anyway. what's the point of this?
          chan[i].portaStop=true;
          // this is why we didn't send noye on before.
          // there may be a portamento which of course prevents a note on
          if (chan[i].keyOn) chan[i].doNote=false;
          // COMPAT FLAG: stop portamento on note off
          // - if a portamento is called and then a note off occurs, stop portamento before the next note
          // - ...unless noteOffResetsSlides is disabled
          chan[i].stopOnOff=song.stopPortaOnNoteOff; // what?!
          chan[i].scheduledSlideReset=false;
          dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,1));
          // this is used to inhibit any other slide commands if the respective compat flag is enabled
          lastSlide=0x1337; // i hate this so much
        }
        break;
      // vibratos and pitch changes are mixed in.
      case 0x04: // vibrato
        // remember the last vibrato for 05xy
        if (effectVal) chan[i].lastVibrato=effectVal;
        chan[i].vibratoDepth=effectVal&15;
        chan[i].vibratoRate=effectVal>>4;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VIBRATO,i,(chan[i].vibratoDepth&15)|(chan[i].vibratoRate<<4)));
        // update pitch now
        dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
        break;
      /// VOLUME-RELATED
      case 0x05: // vol slide + vibrato
        // this effect is weird. it shouldn't be here considering we have more
        // than one effect column, but I guess it had to be done
        if (effectVal==0) {
          chan[i].vibratoDepth=0;
          chan[i].vibratoRate=0;
        } else {
          chan[i].vibratoDepth=chan[i].lastVibrato&15;
          chan[i].vibratoRate=chan[i].lastVibrato>>4;
        }
        dispatchCmd(DivCommand(DIV_CMD_HINT_VIBRATO,i,(chan[i].vibratoDepth&15)|(chan[i].vibratoRate<<4)));
        // update pitch now
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
        // same thing here. this is another effect that doesn't need to exist.
        if (effectVal==0 || chan[i].lastPorta==0) {
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
          chan[i].inPorta=false;
          dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        } else {
          // this here is for a compatibility flag...
          calledPorta=true;
          // COMPAT FLAG: buggy portamento after sliding
          // yes, this also affects 06xy.
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
          // this is the same as 03xx.
          chan[i].portaStop=true;
          if (chan[i].keyOn) chan[i].doNote=false;
          chan[i].stopOnOff=song.stopPortaOnNoteOff; // what?!
          chan[i].scheduledSlideReset=false;
          dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,1));
          lastSlide=0x1337; // i hate this so much
        }
        // now handle volume slide
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
        // the implementation of tremolo in Furnace is more like that of
        // vibrato. it oscillates according to a waveform.
        // this differs from Defle where it is a consecutive vol slide
        // up/down and exhibits a numbee of bugs.
        if (chan[i].tremoloDepth==0) {
          chan[i].tremoloPos=0;
        }
        chan[i].tremoloDepth=effectVal&15;
        chan[i].tremoloRate=effectVal>>4;
        dispatchCmd(DivCommand(DIV_CMD_HINT_TREMOLO,i,effectVal));
        // unfortunately, we cannot run both tremolo and vol slide at once.
        if (chan[i].tremoloDepth!=0) {
          chan[i].volSpeed=0;
          chan[i].volSpeedTarget=-1;
        } else {
          // restore the volume if tremolo is disabled
          dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
          dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
        }
        break;
      case 0x0a: // volume slide
        // the speed multipler is 64, which means 4 ticks between volume changes with a value of 1
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
        // reset the volume target
        chan[i].volSpeedTarget=-1;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE,i,chan[i].volSpeed));
        break;
      /// NOTE
      case 0x00: // arpeggio
        chan[i].arp=effectVal;
        // COMPAT FLAG: reset note to base on arp stop (inverted in the GUI)
        // - a 0000 effect resets arpeggio position
        if (chan[i].arp==0 && song.arp0Reset) {
          chan[i].resetArp=true;
        }
        dispatchCmd(DivCommand(DIV_CMD_HINT_ARPEGGIO,i,chan[i].arp));
        break;
      case 0x0c: // retrigger
        // this is the only non-continuous effect. it takes place exclusively
        // within one row like most PC/Amiga trackers.
        // consecutive 0Cxx effects will reset on each row...
        if (effectVal!=0) {
          chan[i].retrigSpeed=effectVal;
          chan[i].retrigTick=0;
        }
        break;
      /// MISC
      case 0x90: case 0x91: case 0x92: case 0x93:
      case 0x94: case 0x95: case 0x96: case 0x97: 
      case 0x98: case 0x99: case 0x9a: case 0x9b:
      case 0x9c: case 0x9d: case 0x9e: case 0x9f: // set samp. pos
        // COMPAT FLAG: old sample offset effect
        // - before 0.6.3 the sample offset effect was 9xxx, where `xxx` is multiplied by 256
        // - the effect was then changed to 90xx/91xx/92xx, allowing you to set the low, mid and high bytes of the offset respectively
        if (song.oldSampleOffset) {
          // send sample position now
          dispatchCmd(DivCommand(DIV_CMD_SAMPLE_POS,i,(((effect&0x0f)<<8)|effectVal)*256));
        } else {
          // change one byte and schedule sample position
          if (effect<0x93) {
            chan[i].sampleOff&=~(0xff<<((effect-0x90)<<3));
            chan[i].sampleOff|=effectVal<<((effect-0x90)<<3);
            sampleOffSet=true;
          }
        }
        break;
      case 0xc0: case 0xc1: case 0xc2: case 0xc3: // set Hz
        // Cxxx, where `xxx` is between 1 and 1023
        // divider is the tick rate in Hz
        // cycles is the number of samples between ticks
        // clockDrift is used for accuracy and subticks for low-latency mode
        // (where we run faster thsn the tick rate to allow sub-tick note events from live playback)
        divider=(double)(((effect&0x3)<<8)|effectVal);
        if (divider<1) divider=1;
        cycles=got.rate/divider;
        clockDrift=0;
        subticks=0;
        break;
      case 0xdc: // delayed mute
        // used on XM import, where ECx actually mutes the note
        // COMPAT FLAG: cut/delay effect policy (delayBehavior)
        // - 0: strict
        //   - ignore cut if equal or greater than speed
        // - 1: strict old
        //   - ignore cut if equal or greater than speed
        // - 2: lax (default)
        //   - no cut is ever ignored unless overridden by another
        if (effectVal>0 && (song.delayBehavior==2 || effectVal<nextSpeed)) {
          // the cut timer is ticked after nextRow(), so we set it one tick higher.
          chan[i].volCut=effectVal+1;
          chan[i].cutType=0;
        }
        break;
      case 0xd3: // volume portamento (vol porta)/scivolando
        // tremolo and vol slides are incompatible
        chan[i].tremoloDepth=0;
        chan[i].tremoloRate=0;
        // check whether we will slide up or down
        // the speed is 1, which means that 256 ticks will elapse between volume changes with a value of 1.
        chan[i].volSpeed=volPortaTarget<0 ? 0 : volPortaTarget>chan[i].volume ? effectVal : -effectVal;
        chan[i].volSpeedTarget=chan[i].volSpeed==0 ? -1 : volPortaTarget;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE_TARGET,i,chan[i].volSpeed,chan[i].volSpeedTarget));
        break;
      case 0xd4: // volume portamento fast (vol porta fast)
        // this is the same as D3xx, but 256 times faster.
        // tremolo and vol slides are incompatible
        chan[i].tremoloDepth=0;
        chan[i].tremoloRate=0;
        chan[i].volSpeed=volPortaTarget<0 ? 0 : volPortaTarget>chan[i].volume ? 256*effectVal : -256*effectVal;
        chan[i].volSpeedTarget=chan[i].volSpeed==0 ? -1 : volPortaTarget;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE_TARGET,i,chan[i].volSpeed,chan[i].volSpeedTarget));
        break;
      case 0xe0: // arp speed
        // the arp speed is global. I have no idea why.
        if (effectVal>0) {
          curSubSong->arpLen=effectVal;
          dispatchCmd(DivCommand(DIV_CMD_HINT_ARP_TIME,i,curSubSong->arpLen));
        }
        break;
      case 0xe1: // portamento up
        // this is a shortcut for 03xx and a higher note.
        // it has the benefit of being able to be used in conjunction with a note.
        chan[i].portaNote=chan[i].note+(effectVal&15);
        chan[i].portaSpeed=(effectVal>>4)*4;
        dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
        // these are for compatibility stuff
        chan[i].portaStop=true;
        // COMPAT FLAG: stop portamento on note off
        chan[i].stopOnOff=song.stopPortaOnNoteOff; // what?!
        chan[i].scheduledSlideReset=false;
        // only enter portamento if the speed is set
        if ((effectVal&15)!=0) {
          chan[i].inPorta=true;
          // these are for compatibility flaga.
          chan[i].shorthandPorta=true;
          chan[i].wasShorthandPorta=true;
          // COMPAT FLAG: broken shortcut slides
          // - oddly enough, shortcut slides are not communicated to the dispatch
          // - this was fixed in 0.5.7
          if (!song.brokenShortcutSlides) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,0));
          // COMPAT FLAG: E1xy/E2xy also take priority over slides
          // - another Defle hack. it places shortcut slides above pitch slides.
          if (song.e1e2AlsoTakePriority) lastSlide=0x1337; // ...
        } else {
          chan[i].inPorta=false;
          // COMPAT FLAG: broken shortcut slides
          if (!song.brokenShortcutSlides) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        }
        break;
      case 0xe2: // portamento down
        // this is the same as E1xy but in the opposite direction.
        chan[i].portaNote=chan[i].note-(effectVal&15);
        chan[i].portaSpeed=(effectVal>>4)*4;
        dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
        chan[i].portaStop=true;
        // COMPAT FLAG: stop portamento on note off
        chan[i].stopOnOff=song.stopPortaOnNoteOff; // what?!
        chan[i].scheduledSlideReset=false;
        if ((effectVal&15)!=0) {
          chan[i].inPorta=true;
          // these are for compatibility flaga.
          chan[i].shorthandPorta=true;
          chan[i].wasShorthandPorta=true;
          // COMPAT FLAG: broken shortcut slides
          if (!song.brokenShortcutSlides) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,0));
          // COMPAT FLAG: E1xy/E2xy also take priority over slides
          if (song.e1e2AlsoTakePriority) lastSlide=0x1337; // ...
        } else {
          chan[i].inPorta=false;
          // COMPAT FLAG: broken shortcut slides
          if (!song.brokenShortcutSlides) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
        }
        break;
      case 0xe3: // vibrato shape
        chan[i].vibratoShape=effectVal;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VIBRATO_SHAPE,i,chan[i].vibratoShape));
        break;
      case 0xe4: // vibrato fine
        // this sets the multiplier for vibrato depth
        chan[i].vibratoFine=effectVal;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VIBRATO_RANGE,i,chan[i].vibratoFine));
        break;
      case 0xe5: // pitch
        chan[i].pitch=effectVal-0x80;
        // send pitch now
        dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
        dispatchCmd(DivCommand(DIV_CMD_HINT_PITCH,i,chan[i].pitch));
        break;
      case 0xe6: // delayed legato
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
        // COMPAT FLAG: cut/delay effect policy (delayBehavior)
        // - 0: strict
        //   - ignore cut if equal or greater than speed
        // - 1: strict old
        //   - ignore cut if equal or greater than speed
        // - 2: lax (default)
        //   - no cut is ever ignored unless overridden by another
        // "Bruh"
        if (effectVal>0 && (song.delayBehavior==2 || effectVal<nextSpeed)) {
          // the cut timer is ticked after nextRow(), so we set it one tick higher.
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
        // again, that's why we didn't just send note on back then.
        // this effect inhibits note on.
        chan[i].legato=effectVal;
        break;
      case 0xeb: // sample bank
        // this is a legacy effect for compatibility.
        // in legacy sample mode (17xx), 12 samples are mapped to an octave.
        // this effect allows you to use another group of 12 samples.
        dispatchCmd(DivCommand(DIV_CMD_SAMPLE_BANK,i,effectVal));
        break;
      case 0xec: // delayed note cut
        // COMPAT FLAG: cut/delay effect policy (delayBehavior)
        // - 0: strict
        //   - ignore cut if equal or greater than speed
        // - 1: strict old
        //   - ignore cut if equal or greater than speed
        // - 2: lax (default)
        //   - no cut is ever ignored unless overridden by another
        if (effectVal>0 && (song.delayBehavior==2 || effectVal<nextSpeed)) {
          // the cut timer is ticked after nextRow(), so we set it one tick higher.
          chan[i].cut=effectVal+1;
          chan[i].cutType=0;
        }
        break;
      case 0xee: // external command
        // this does nothing in Furnace but is useful for export.
        //printf("\x1b[1;36m%d: extern command %d\x1b[m\n",i,effectVal);
        extValue=effectVal;
        extValuePresent=true;
        dispatchCmd(DivCommand(DIV_CMD_EXTERNAL,i,effectVal));
        break;
      case 0xf0: // set Hz by tempo
        // the resulting tick rate is effectVal*2/5
        // 125 BPM = 50Hz; 150 BPM = 60Hz...
        divider=(double)effectVal*2.0/5.0;
        if (divider<1) divider=1;
        cycles=got.rate/divider;
        clockDrift=0;
        subticks=0;
        break;
      case 0xf3: // fine volume slide up
        // tremolo and vol slides are incompatible
        chan[i].tremoloDepth=0;
        chan[i].tremoloRate=0;
        // this is 64 times slower than 0Axy.
        chan[i].volSpeed=effectVal;
        chan[i].volSpeedTarget=-1;
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOL_SLIDE,i,chan[i].volSpeed));
        break;
      case 0xf4: // fine volume slide down
        // tremolo and vol slides are incompatible
        chan[i].tremoloDepth=0;
        chan[i].tremoloRate=0;
        // this is 64 times slower than 0Axy.
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
      case 0xf8: // single volume slide up
        // this will stop volume slides
        chan[i].volSpeed=0; // add compat flag?
        chan[i].volSpeedTarget=-1;
        chan[i].volume=MIN(chan[i].volume+effectVal*256,chan[i].volMax);
        dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
        break;
      case 0xf9: // single volume slide down
        // this will stop volume slides
        chan[i].volSpeed=0; // add compat flag?
        chan[i].volSpeedTarget=-1;
        chan[i].volume=MAX(chan[i].volume-effectVal*256,0);
        dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
        dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
        break;
      case 0xfa: // fast volume slide
        // this is four times the speed of 0Axy.
        // effectively the value is the number of volume steps to change on each tick.
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
        // COMPAT FLAG: cut/delay effect policy (delayBehavior)
        // - 0: strict
        //   - ignore cut if equal or greater than speed
        // - 1: strict old
        //   - ignore cut if equal or greater than speed
        // - 2: lax (default)
        //   - no cut is ever ignored unless overridden by another
        if (song.delayBehavior==2 || effectVal<nextSpeed) {
          // the cut timer is ticked after nextRow(), so we set it one tick higher.
          chan[i].cut=effectVal+1;
          chan[i].cutType=1;
        }
        break;

      case 0xff: // stop song
        // this is handled in nextTick()
        shallStopSched=true;
        logV("scheduling stop");
        break;
    }
  }

  // commit pending effects
  // sample offset (9xxx)
  if (sampleOffSet) {
    dispatchCmd(DivCommand(DIV_CMD_SAMPLE_POS,i,chan[i].sampleOff));
  }

  // panning effects
  if (panChanged) {
    dispatchCmd(DivCommand(DIV_CMD_PANNING,i,chan[i].panL,chan[i].panR));
    dispatchCmd(DivCommand(DIV_CMD_HINT_PANNING,i,chan[i].panL,chan[i].panR));
  }
  if (surroundPanChanged) {
    dispatchCmd(DivCommand(DIV_CMD_SURROUND_PANNING,i,2,chan[i].panRL));
    dispatchCmd(DivCommand(DIV_CMD_SURROUND_PANNING,i,3,chan[i].panRR));
  }

  // COMPAT FLAG: instrument changes triggee on portamento (inverted in the GUI)
  // - before 0.6pre1 it was not possible to change instrument during portamento
  // - now it is. this sends a "null" note to allow such change
  if (insChanged && (chan[i].inPorta || calledPorta) && song.newInsTriggersInPorta) {
    dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,i,DIV_NOTE_NULL));
  }

  // commit note on
  if (chan[i].doNote) {
    // COMPAT FLAG: continuous vibrato
    // - when enabled, the vibrato position is not reset on each note
    if (!song.continuousVibrato) {
      chan[i].vibratoPos=0;
    }
    // send pitch now (why? didn't we do that already?)
    dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
    // handle legato
    // COMPAT FLAG: broken portamento during legato
    // - portamento would not occur if legato is on
    // - this was fixed in 0.6pre4
    if (chan[i].legato && (!chan[i].inPorta || song.brokenPortaLegato)) {
      dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
      dispatchCmd(DivCommand(DIV_CMD_HINT_LEGATO,i,chan[i].note));
    } else {
      // this is where we actually send a note on command to the dispatch.
      // this does not occur if portamento is in progress and it is not a shortcut slide (E1xy/E2xy)
      if (chan[i].inPorta && chan[i].keyOn && !chan[i].shorthandPorta) {
        // COMPAT FLAG: E1xy/E2xy stop on same note
        // - if there was a shortcut slide, stop it
        if (song.e1e2StopOnSameNote && chan[i].wasShorthandPorta) {
          chan[i].portaSpeed=-1;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
          // COMPAT FLAG: broken shortcut slides
          // - oddly enough, shortcut slides are not communicated to the dispatch
          // - this was fixed in 0.5.7
          if (!song.brokenShortcutSlides) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
          chan[i].wasShorthandPorta=false;
          chan[i].inPorta=false;
        } else {
          // otherwise we change the portamento target
          chan[i].portaNote=chan[i].note;
          dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
        }
      } else if (!chan[i].noteOnInhibit) {
        // noteOnInhibit is set during live playback to prevent an extra note from playing
        // we finally send the note on command
        dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,i,chan[i].note,chan[i].volume>>8));
        chan[i].releasing=false;
        // COMPAT FLAG: reset arp position on new note
        // - this does exactly what it says
        if (song.resetArpPhaseOnNewNote) {
           chan[i].arpStage=-1;
        }
        // these are used by VGM/ROM export to determine the duration of loop trail.
        // goneThroughNote and wentThroughNote arw set once a note plays on a channel.
        // wentThroughNote is then reset on loop, and the loop trail begins.
        // once all channels which had gone through a note get a note on, the loop trail duration is determined.
        chan[i].goneThroughNote=true;
        chan[i].wentThroughNote=true;
        // this may be used by the GUI for visualizers.
        keyHit[i]=true;
      }
    }
    // now that we did note, clear this flag
    chan[i].doNote=false;
    // keyOn is false after a keyOff so we can do this
    // reset slide if scheduled and not keying on
    // I don't understand
    if (!chan[i].keyOn && chan[i].scheduledSlideReset) {
      chan[i].portaNote=-1;
      chan[i].portaSpeed=-1;
      dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
      chan[i].scheduledSlideReset=false;
      chan[i].inPorta=false;
    }
    // cap the volume if it is too high and not key on
    if (!chan[i].keyOn && chan[i].volume>chan[i].volMax) {
      chan[i].volume=chan[i].volMax;
      dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
      dispatchCmd(DivCommand(DIV_CMD_HINT_VOLUME,i,chan[i].volume>>8));
    }
    // now set key on
    chan[i].keyOn=true;
    chan[i].keyOff=false;
  }
  // reset these
  chan[i].shorthandPorta=false;
  chan[i].noteOnInhibit=false;

  // post effects
  for (int j=0; j<curPat[i].effectCols; j++) {
    short effect=pat->newData[whatRow][DIV_PAT_FX(j)];
    short effectVal=pat->newData[whatRow][DIV_PAT_FXVAL(j)];

    // an empty effect value is treated as zero
    if (effectVal==-1) effectVal=0;
    effectVal&=255;

    // per-system post-effects
    // if there isn't one, try with normal effects
    if (!perSystemPostEffect(i,effect,effectVal)) {
      switch (effect) {
        // these are done later to let note on happen first
        case 0xf1: // single pitch slide up
        case 0xf2: // single pitch slide down
          if (effect==0xf1) {
            // COMPAT FLAG: limit slide range
            chan[i].portaNote=song.limitSlides?0x60:255;
          } else {
            // COMPAT FLAG: limit slide range
            chan[i].portaNote=song.limitSlides?disCont[dispatchOfChan[i]].dispatch->getPortaFloor(dispatchChanOfChan[i]):-60;
          }
          chan[i].portaSpeed=effectVal;
          chan[i].portaStop=true;
          chan[i].stopOnOff=false;
          chan[i].scheduledSlideReset=false;
          chan[i].inPorta=false;
          // COMPAT FLAG: arpeggio inhibits non-porta slides
          if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true,0));
          dispatchCmd(DivCommand(DIV_CMD_NOTE_PORTA,i,chan[i].portaSpeed*(song.linearPitch?song.pitchSlideSpeed:1),chan[i].portaNote));
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
          chan[i].inPorta=false;
          // COMPAT FLAG: arpeggio inhibits non-porta slides
          if (!song.arpNonPorta) dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
          break;
      }
    }
  }
}

// this is called by nextTick().
// this reads the next row:
// 1. update pattern console visualizer
// 2. update the metronome
// 3. call processRowPre() on all channels
// 4. call processRow() on all channels
// 5. mark row as "walked" on
// 6. advance to next row/commit pattern jumps
// 7. detect song loop
// 8. perform speed alternation
// 9. schedule cuts and pre-notes if necessary
void DivEngine::nextRow() {
  // update pattern visualizer in console mode
  // buffers for printing the next row
  static char pb[4096];
  static char pb1[4096];
  static char pb2[4096];
  static char pb3[4096];
  if (view==DIV_STATUS_PATTERN && !skipping) {
    strcpy(pb1,"");
    strcpy(pb3,"");
    for (int i=0; i<chans; i++) {
      // orders
      snprintf(pb,4095," %.2x",curOrders->ord[i][curOrder]);
      strcat(pb1,pb);

      // pattern data
      DivPattern* pat=curPat[i].getPattern(curOrders->ord[i][curOrder],false);
      snprintf(pb2,4095,"\x1b[37m %s",
              formatNote(pat->newData[curRow][DIV_PAT_NOTE]));
      strcat(pb3,pb2);
      if (pat->newData[curRow][DIV_PAT_VOL]==-1) {
        strcat(pb3,"\x1b[m--");
      } else {
        snprintf(pb2,4095,"\x1b[1;32m%.2x",pat->newData[curRow][DIV_PAT_VOL]);
        strcat(pb3,pb2);
      }
      if (pat->newData[curRow][DIV_PAT_INS]==-1) {
        strcat(pb3,"\x1b[m--");
      } else {
        snprintf(pb2,4095,"\x1b[0;36m%.2x",pat->newData[curRow][DIV_PAT_INS]);
        strcat(pb3,pb2);
      }
      for (int j=0; j<curPat[i].effectCols; j++) {
        if (pat->newData[curRow][DIV_PAT_FX(j)]==-1) {
          strcat(pb3,"\x1b[m--");
        } else {
          snprintf(pb2,4095,"\x1b[1;31m%.2x",pat->newData[curRow][DIV_PAT_FX(j)]);
          strcat(pb3,pb2);
        }
        if (pat->newData[curRow][DIV_PAT_FXVAL(j)]==-1) {
          strcat(pb3,"\x1b[m--");
        } else {
          snprintf(pb2,4095,"\x1b[1;37m%.2x",pat->newData[curRow][DIV_PAT_FXVAL(j)]);
          strcat(pb3,pb2);
        }
      }
    }
    // print orders and pattern row
    printf("| %.2x:%s | \x1b[1;33m%3d%s\x1b[m\n",curOrder,pb1,curRow,pb3);
  }

  // update and tick metronome if necessary
  // elapsedBeats/Bars is used by the GUI for the clock
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

  // set the previous order as we'll be in the next one once done
  if (!stepPlay) {
    playPosLock.lock();
    prevOrder=curOrder;
    prevRow=curRow;
    playPosLock.unlock();
  }

  // process row pre on all channels
  for (int i=0; i<chans; i++) {
    // try to find pre effects
    processRowPre(i);
  }

  // process row on all channels
  for (int i=0; i<chans; i++) {
    // COMPAT FLAG: cut/delay effect policy (delayBehavior)
    // - if not lax, reset the row delay timer so it never happens
    if (song.delayBehavior!=2) {
      chan[i].rowDelay=0;
    }
    processRow(i,false);
  }

  // mark this row as "walked" over
  // this is used to determine loop position
  walked[((curOrder<<5)+(curRow>>3))&8191]|=1<<(curRow&7);

  // commit a pending jump if there is one
  // otherwise, advance row position
  if (changeOrd!=-1) {
    // disregard if repeat pattern is on
    if (repeatPattern) {
      curRow=0;
      changeOrd=-1;
    } else {
      // jump to order and reset position
      curRow=changePos;
      changePos=0;
      // jump to next order if it is -2
      if (changeOrd==-2) changeOrd=curOrder+1;
      // old loop detection routine, now commented
      //if (changeOrd<=curOrder) endOfSong=true;
      curOrder=changeOrd;

      // if we're out of bounds, return to the beginning
      // if this happens we're guaranteed to loop
      if (curOrder>=curSubSong->ordersLen) {
        curOrder=0;
        endOfSong=true;
        memset(walked,0,8192);
      }
      changeOrd=-1;
    }
    // halt engine if requested (debug menu)
    if (haltOn==DIV_HALT_PATTERN) halted=true;
  } else if (playing) if (++curRow>=curSubSong->patLen) {
    // if we are here it means we reached the end of this pattern, so
    // advance to next order unless the song is about to stop
    if (shallStopSched) {
      curRow=curSubSong->patLen-1;
    } else {
      nextOrder();
    }
    // halt engine if requested (debug menu)
    if (haltOn==DIV_HALT_PATTERN) halted=true;
  }

  // new loop detection routine
  // if we're stepping on a row we've already walked over, we found loop
  // if the song is going to stop though, don't do anything
  if (!endOfSong && walked[((curOrder<<5)+(curRow>>3))&8191]&(1<<(curRow&7)) && !shallStopSched) {
    logV("loop reached");
    endOfSong=true;
    memset(walked,0,8192);
  }

  // perform speed alternation
  prevSpeed=nextSpeed;
  // COMPAT FLAG: broken speed alternation
  // - DefleMask uses a mandatory two-speed system
  // - if the pattern length is odd, the speed to use is determined correctly...
  // - ...unless the order count is also odd! in that case the first row of order 0 will always use speed 1, even if the song looped and we should be using speed 2
  if (song.brokenSpeedSel) {
    unsigned char speed2=(speeds.len>=2)?speeds.val[1]:speeds.val[0];
    unsigned char speed1=speeds.val[0];
    
    // if the pattern length is odd and the current order is odd, use speed 2 for even rows and speed 1 for odd ones
    if ((curSubSong->patLen&1) && curOrder&1) {
      ticks=((curRow&1)?speed2:speed1)*(curSubSong->timeBase+1);
      nextSpeed=(curRow&1)?speed1:speed2;
    } else {
      ticks=((curRow&1)?speed1:speed2)*(curSubSong->timeBase+1);
      nextSpeed=(curRow&1)?speed2:speed1;
    }
  } else {
    // normal speed alternation
    // set the number of ticks and cycle to the next speed
    ticks=speeds.val[curSpeed]*(curSubSong->timeBase+1);
    curSpeed++;
    if (curSpeed>=speeds.len) curSpeed=0;
    // cache the next speed for future operations
    nextSpeed=speeds.val[curSpeed];
  }

  /*
  if (skipping) {
    ticks=1;
  }*/

  // post row details
  // schedule pre-notes and delays (for C64 and/or a compat flag)
  for (int i=0; i<chans; i++) {
    DivPattern* pat=curPat[i].getPattern(curOrders->ord[i][curOrder],false);
    if (pat->newData[curRow][DIV_PAT_NOTE]!=-1) {
      // if there is a note
      if (pat->newData[curRow][DIV_PAT_NOTE]!=DIV_NOTE_OFF && pat->newData[curRow][DIV_PAT_NOTE]!=DIV_NOTE_REL && pat->newData[curRow][DIV_PAT_NOTE]!=DIV_MACRO_REL) {
        // if legato isn't on
        if (!chan[i].legato) {
          // check whether we should fire a pre-note event
          bool wantPreNote=false;
          if (disCont[dispatchOfChan[i]].dispatch!=NULL) {
            wantPreNote=disCont[dispatchOfChan[i]].dispatch->getWantPreNote();
            if (wantPreNote) {
              bool doPreparePreNote=true;
              int addition=0;

              // check whether there is a portamento, legato or delay effect
              // in the former two we shouldn't send pre-note
              // the latter one will delay our pre-note command
              for (int j=0; j<curPat[i].effectCols; j++) {
                // COMPAT FLAG: pre-note does not take effect into consideration
                // - a bug which does not cancel pre-note before a portamento or during legato
                // - fixed in 0.6pre9
                if (!song.preNoteNoEffect) {
                  // handle portamento
                  if (pat->newData[curRow][DIV_PAT_FX(j)]==0x03 && pat->newData[curRow][DIV_PAT_FXVAL(j)]!=0 && pat->newData[curRow][DIV_PAT_FXVAL(j)]!=-1) {
                    doPreparePreNote=false;
                    break;
                  }
                  // handle vol slide + portamento
                  if (pat->newData[curRow][DIV_PAT_FX(j)]==0x06 && pat->newData[curRow][DIV_PAT_FXVAL(j)]!=0 && pat->newData[curRow][DIV_PAT_FXVAL(j)]!=-1) {
                    doPreparePreNote=false;
                    break;
                  }
                  // handle legato
                  if (pat->newData[curRow][DIV_PAT_FX(j)]==0xea) {
                    if (pat->newData[curRow][DIV_PAT_FXVAL(j)]>0) {
                      doPreparePreNote=false;
                      break;
                    }
                  }
                }
                // delay pre-note if there is a delay effect
                if (pat->newData[curRow][DIV_PAT_FX(j)]==0xed) {
                  if (pat->newData[curRow][DIV_PAT_FXVAL(j)]>0) {
                    addition=pat->newData[curRow][DIV_PAT_FXVAL(j)]&255;
                    break;
                  }
                }
              }
              // send pre-note command
              if (doPreparePreNote) dispatchCmd(DivCommand(DIV_CMD_PRE_NOTE,i,ticks+addition));
            }
          }

          // COMPAT FLAG: auto-insert one tick gap between notes
          // - simulates behavior of certain Amiga/C64 sound drivers where a one-tick cut occurred before another note
          if (song.oneTickCut) {
            bool doPrepareCut=true;
            int addition=0;

            for (int j=0; j<curPat[i].effectCols; j++) {
              // handle portamento
              if (pat->newData[curRow][DIV_PAT_FX(j)]==0x03 && pat->newData[curRow][DIV_PAT_FXVAL(j)]!=0 && pat->newData[curRow][DIV_PAT_FXVAL(j)]!=-1) {
                doPrepareCut=false;
                break;
              }
              // handle vol slide + portamento
              if (pat->newData[curRow][DIV_PAT_FX(j)]==0x06 && pat->newData[curRow][DIV_PAT_FXVAL(j)]!=0 && pat->newData[curRow][DIV_PAT_FXVAL(j)]!=-1) {
                doPrepareCut=false;
                break;
              }
              // handle legato
              if (pat->newData[curRow][DIV_PAT_FX(j)]==0xea) {
                if (pat->newData[curRow][DIV_PAT_FXVAL(j)]>0) {
                  doPrepareCut=false;
                  break;
                }
              }
              // delay cut if there is a delay effect
              if (pat->newData[curRow][DIV_PAT_FX(j)]==0xed) {
                if (pat->newData[curRow][DIV_PAT_FXVAL(j)]>0) {
                  addition=pat->newData[curRow][DIV_PAT_FXVAL(j)]&255;
                  break;
                }
              }
            }
            // prepare a cut if a cut hasn't been scheduled already
            // and the dispatch does not want pre-note events
            if (doPrepareCut && !wantPreNote && chan[i].cut<=0) {
              chan[i].cut=ticks+addition;
              chan[i].cutType=0;
            }
          }
        }
      }
    }
  }

  // halt engine if requested (debug menu)
  if (haltOn==DIV_HALT_ROW) halted=true;

  // set firstTick to indicate this is the first tick (used for a compat flag)
  firstTick=true;
}

// advances one tick.
// it is called by nextBuf(), playSub() nd the export functions.
// noAccum will prevent the playback time from increasing.
// if inhibitLowLat is on, low-latency mode is not taken into account. this is used by the export functions.
// returns whether the song has ended.
bool DivEngine::nextTick(bool noAccum, bool inhibitLowLat) {
  bool ret=false;
  // prevent a division by zero
  if (divider<1) divider=1;

  // low-latency mode only
  // set the tick multiplier so that when multiplied by the divider the product is close to 1000
  if (lowLatency && !skipping && !inhibitLowLat) {
    tickMult=1000/divider;
    if (tickMult<1) tickMult=1;
  } else {
    tickMult=1;
  }
  
  // set the number of samples between ticks (or sub-ticks in low-latency mode)
  cycles=got.rate/(divider*tickMult);
  clockDrift+=fmod(got.rate,(double)(divider*tickMult));
  if (clockDrift>=(divider*tickMult)) {
    // correct clock since cycles is an integer
    clockDrift-=(divider*tickMult);
    cycles++;
  }

  // don't let user play anything during export
  if (exporting) pendingNotes.clear();

  // process pending notes (live playback)
  if (!pendingNotes.empty()) {
    bool isOn[DIV_MAX_CHANS];
    memset(isOn,0,DIV_MAX_CHANS*sizeof(bool));

    // this is a check that nullifies any note off event that right after a note on
    // it prevents a situation where some notes do not play
    for (int i=pendingNotes.size()-1; i>=0; i--) {
      if (pendingNotes[i].channel<0 || pendingNotes[i].channel>=chans) continue;
      if (pendingNotes[i].on) {
        isOn[pendingNotes[i].channel]=true;
      } else {
        // this is a note off - check whether the channel is going up.
        // if so, cancel this event.
        if (isOn[pendingNotes[i].channel]) {
          //logV("erasing off -> on sequence in %d",pendingNotes[i].channel);
          pendingNotes[i].nop=true;
        }
      }
    }
  }

  // process pending notes, for real this time
  while (!pendingNotes.empty()) {
    // fetch event
    DivNoteEvent& note=pendingNotes.front();
    // don't if channel is out of bounds or event is canceled
    if (note.nop || note.channel<0 || note.channel>=chans) {
      pendingNotes.pop_front();
      continue;
    }
    // process an instrument change event
    if (note.insChange) {
      dispatchCmd(DivCommand(DIV_CMD_INSTRUMENT,note.channel,note.ins,0));
      pendingNotes.pop_front();
      continue;
    }
    // otherwise process a note event
    if (note.on) {
      // note on
      // set the instrument except on MIDI direct mode
      if (!(midiIsDirect && midiIsDirectProgram && note.fromMIDI)) {
        dispatchCmd(DivCommand(DIV_CMD_INSTRUMENT,note.channel,note.ins,1));
      }
      // set volume as long as there's one associated with the event
      // and the chip has per-channel volume
      if (note.volume>=0 && !disCont[dispatchOfChan[note.channel]].dispatch->isVolGlobal()) {
        // map velocity to curve and then to equivalent chip volume
        float curvedVol=pow((float)note.volume/127.0f,midiVolExp);
        int mappedVol=disCont[dispatchOfChan[note.channel]].dispatch->mapVelocity(dispatchChanOfChan[note.channel],curvedVol);
        // fire command
        dispatchCmd(DivCommand(DIV_CMD_VOLUME,note.channel,mappedVol));
      }
      // send note on command and set channel state
      dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,note.channel,note.note));
      keyHit[note.channel]=true;
      chan[note.channel].note=note.note;
      chan[note.channel].releasing=false;
      // this prevents a duplicate note from being played while editing the pattern
      chan[note.channel].noteOnInhibit=true;
      chan[note.channel].lastIns=note.ins;
    } else {
      // note off
      DivMacroInt* macroInt=disCont[dispatchOfChan[note.channel]].dispatch->getChanMacroInt(dispatchChanOfChan[note.channel]);
      if (macroInt!=NULL) {
        // if the current instrument has a release point in any macros and
        // volume is per-channel, send a note release instead of a note off
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

  // tick the engine state if we are not in freelance mode (engine active but not running).
  // this includes ticking the sub-tick counter, processing delayed rows,
  // effects and of course, playing the next row.
  if (!freelance) {
    // decrease sub-tick counter (low-latency mode)
    // run a tick once it reached zero
    if (--subticks<=0) {
      subticks=tickMult;

      // apply delayed rows before potentially advancing to a new row, which would overwrite the
      // delayed row's state before it has a chance to do anything. a typical example would be
      // a delay scheduling a note-on to be simultaneous with the next row, and the next row also
      // containing a delayed note. if we don't apply the delayed row first, the world explodes.
      for (int i=0; i<chans; i++) {
        // delay effects
        if (chan[i].rowDelay>0) {
          if (--chan[i].rowDelay==0) {
            // we call processRow() here for the delayed row
            processRow(i,true);
          }
        }
      }

      // advance tempo accumulator (for virtual tempo) unless we are step playing and waiting for the next step (stepPlay==2)
      // then advance tick counter and then call nextRow()
      if (stepPlay!=1) {
        // fast-forward the accumulator if we are "skipping" (seeking to a position)
        // otherwise increase accumulator by virtual tempo numerator
        tempoAccum+=(skipping && virtualTempoN<virtualTempoD)?virtualTempoD:virtualTempoN;
        // while accumulator is higher than virtual tempo denominator
        while (tempoAccum>=virtualTempoD) {
          // wrap the accumulator back
          tempoAccum-=virtualTempoD;
          // tick the tick counter
          if (--ticks<=0) {
            ret=endOfSong;
            // get out if the song is going to stop (we'll stop at the end of this function)
            if (shallStopSched) {
              logV("acknowledging scheduled stop");
              shallStop=true;
              break;
            } else if (endOfSong) {
              // COMPAT FLAG: loop modality
              // - 0: reset channels. call playSub() to seek back to the loop position
              // - 1: soft-reset channels. same as 0 for now
              // - 2: don't reset
              if (song.loopModality!=2) {
                playSub(true);
              }
            }
            endOfSong=false;
            // check whether we were told to step to the next row
            // if so, go back to waiting state (stepPlay==1) and update position
            if (stepPlay==2) {
              stepPlay=1;
              playPosLock.lock();
              prevOrder=curOrder;
              prevRow=curRow;
              playPosLock.unlock();
            }
            // ...and now process the next row!
            nextRow();
            break;
          }
        }
        // under no circumstances shall the accumulator become this large
        if (tempoAccum>1023) tempoAccum=1023;
      }

      // process stuff such as effects
      if (!shallStop) for (int i=0; i<chans; i++) {
        // retrigger
        if (chan[i].retrigSpeed) {
          if (--chan[i].retrigTick<0) {
            chan[i].retrigTick=chan[i].retrigSpeed-1;
            // retrigger is a null note, which allows it to be combined with a pitch slide
            dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,i,DIV_NOTE_NULL));
            keyHit[i]=true;
          }
        }

        // volume slides and tremolo
        // COMPAT FLAG: don't slide on the first tick of a row
        // - Amiga/PC tracker behavior where slides and vibrato do not take course during the first tick of a row
        if (!song.noSlidesOnFirstTick || !firstTick) {
          // volume slides
          if (chan[i].volSpeed!=0) {
            // the call to GET_VOLUME is part of a compatibility process
            // where the stored volume in the dispatch may be different
            // from our volume (see legacy volume slides)
            chan[i].volume=(chan[i].volume&0xff)|(dispatchCmd(DivCommand(DIV_CMD_GET_VOLUME,i))<<8);
            int preSpeedVol=chan[i].volume;
            chan[i].volume+=chan[i].volSpeed;
            // handle scivolando
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
                // once we are there, stop the slide
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
            // stop sliding if we reach maximum/minimum volume
            // there isn't a compat flag for this yet... sorry...
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
              // COMPAT FLAG: legacy volume slides
              // - sets volume to max once a vol slide down has finished (thus setting volume to volMax+1)
              // - there is more to this, such as the first step of volume macro resulting in unpredictable behavior, but I don't feel like implementing THAT...
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
            // tremolo (increase position in look-up table and send a volume change)
            chan[i].tremoloPos+=chan[i].tremoloRate;
            chan[i].tremoloPos&=127;
            dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,MAX(0,chan[i].volume-(tremTable[chan[i].tremoloPos]*chan[i].tremoloDepth))>>8));
          }
        }

        // panning slides
        if (chan[i].panSpeed!=0) {
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
          dispatchCmd(DivCommand(DIV_CMD_PANNING,i,chan[i].panL,chan[i].panR));
        } else if (chan[i].panDepth>0) {
          // panbrello, similar to vibrato and tremolo
          chan[i].panPos+=chan[i].panRate;
          chan[i].panPos&=255;

          // calculate inverted...
          // split position into four sections and calculate panning value
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
          // clamp vibrato position
          while (chan[i].vibratoPos>=64) chan[i].vibratoPos-=64;

          // this is for the GUI's pattern visualizer
          chan[i].vibratoPosGiant+=chan[i].vibratoRate;
          while (chan[i].vibratoPosGiant>=512) chan[i].vibratoPosGiant-=512;

          // look-up table
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
          // vibrato and pitch are merged into one
          dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibratoOut*chan[i].vibratoFine)>>4)/15)));
        }

        // delayed legato
        if (chan[i].legatoDelay>0) {
          if (--chan[i].legatoDelay<1) {
            // change note and send legato
            chan[i].note+=chan[i].legatoTarget;
            dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
            dispatchCmd(DivCommand(DIV_CMD_HINT_LEGATO,i,chan[i].note));
            chan[i].legatoDelay=-1;
            chan[i].legatoTarget=0;
          }
        }

        // portamento and pitch slides
        // COMPAT FLAG: don't slide on the first tick of a row
        // - Amiga/PC tracker behavior where slides and vibrato do not take course during the first tick of a row
        if (!song.noSlidesOnFirstTick || !firstTick) {
          // portamento only runs if the channel has been used and the porta speed is higher than 0
          if ((chan[i].keyOn || chan[i].keyOff) && chan[i].portaSpeed>0) {
            // send a portamento update command to the dispatch.
            // it returns whether the portamento is complete and has reached the target note.
            // COMPAT FLAG: pitch linearity
            // - 0: none (pitch control and slides non-linear)
            // - 1: full (pitch slides linear... we multiply the portamento speed by a user-defined multiplier)
            // COMPAT FLAG: reset pitch slide/portamento upon reaching target (inverted in the GUI)
            // - when disabled, portamento remains active after it has finished
            if (dispatchCmd(DivCommand(DIV_CMD_NOTE_PORTA,i,chan[i].portaSpeed*(song.linearPitch?song.pitchSlideSpeed:1),chan[i].portaNote))==2 && chan[i].portaStop && song.targetResetsSlides) {
              // if we are here, it means we reached the target and shall stop
              chan[i].portaSpeed=0;
              dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
              chan[i].oldNote=chan[i].note;
              chan[i].note=chan[i].portaNote;
              chan[i].inPorta=false;
              // send legato just in case
              dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
              dispatchCmd(DivCommand(DIV_CMD_HINT_LEGATO,i,chan[i].note));
            }
          }
        }

        // note cut
        if (chan[i].cut>0) {
          if (--chan[i].cut<1) {
            if (chan[i].cutType==2) { // macro release
              dispatchCmd(DivCommand(DIV_CMD_ENV_RELEASE,i));
              chan[i].releasing=true;
            } else { // note off or release
              chan[i].oldNote=chan[i].note;
              //chan[i].note=-1;
              // COMPAT FLAG: reset slides on note off (inverted in the GUI)
              // - a portamento/pitch slide will be halted upon encountering note off
              // - this will not occur if the stopPortaOnNoteOff flag is on and this is a portamento
              if (chan[i].inPorta && song.noteOffResetsSlides) {
                chan[i].keyOff=true;
                chan[i].keyOn=false;
                // stopOnOff will be false if stopPortaOnNoteOff flag is off
                if (chan[i].stopOnOff) {
                  chan[i].portaNote=-1;
                  chan[i].portaSpeed=-1;
                  dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
                  chan[i].stopOnOff=false;
                }
                // depending on the system, portamento may still be disabled
                if (disCont[dispatchOfChan[i]].dispatch->keyOffAffectsPorta(dispatchChanOfChan[i])) {
                  chan[i].portaNote=-1;
                  chan[i].portaSpeed=-1;
                  dispatchCmd(DivCommand(DIV_CMD_HINT_PORTA,i,CLAMP(chan[i].portaNote,-128,127),MAX(chan[i].portaSpeed,0)));
                }
                dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false,0));
                // another compatibility hack which schedules a second reset later just in case
                chan[i].scheduledSlideReset=true;
              }
              if (chan[i].cutType==1) { // note release
                dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF_ENV,i));
              } else { // note off
                dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF,i));
              }
              // I am not sure why is this here and not inside the previous statement
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
          // if we must reset arp, sent a legato with the current note
          dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
          dispatchCmd(DivCommand(DIV_CMD_HINT_LEGATO,i,chan[i].note));
          chan[i].resetArp=false;
        }
        // COMPAT FLAG: reset arp position on row change
        // - simulates Amiga/PC tracker behavior where the next row resets arp pos
        if (song.rowResetsArpPos && firstTick) {
          chan[i].arpStage=-1;
        }
        // arpeggio (actually)
        // don't run it if arp yield is enabled (which will be if a compat flag is on)
        if (chan[i].arp!=0 && !chan[i].arpYield && chan[i].portaSpeed<1) {
          if (--chan[i].arpTicks<1) {
            chan[i].arpTicks=curSubSong->arpLen;
            // there are three arp stages, corresponding to note, note+x and note+y in the 00xy effect
            chan[i].arpStage++;
            if (chan[i].arpStage>2) chan[i].arpStage=0;
            // arp is sent as legato
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
          // acknowledge arp yield
          chan[i].arpYield=false;
        }
      }
    }
  } else {
    // we are in freelance mode
    // still tick the subtick counter
    if (--subticks<=0) {
      subticks=tickMult;
    }
  }

  // tick the command stream player if one is attached
  if (subticks==tickMult && cmdStreamInt) {
    if (!cmdStreamInt->tick()) {
      // !!!
    }
  }

  // this was set by nextRow()
  firstTick=false;

  // acknowledge request to stop playback
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

  // tick all chip dispatches (the argument determines whether it is a system tick or a sub-tick)
  for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->tick(subticks==tickMult);

  // update playback time
  if (!freelance) {
    if (stepPlay!=1) {
      if (!noAccum) {
        double dt=divider*tickMult;
        totalTicksR++;
        // despite the name, totalTicks is in microseconds...
        totalTicks+=1000000/dt;
        totalTicksOff+=fmod(1000000.0,dt);
        while (totalTicksOff>=dt) {
          totalTicksOff-=dt;
          totalTicks++;
        }
      }
      if (totalTicks>=1000000) {
        totalTicks-=1000000;
        // who's gonna play a song for 68 years?
        if (totalSeconds<0x7fffffff) totalSeconds++;
        cmdsPerSecond=totalCmds-lastCmds;
        lastCmds=totalCmds;
      }
    }

    // print status in console mode
    if (consoleMode && !disableStatusOut && subticks<=1 && !skipping) fprintf(stderr,"\x1b[2K> %d:%.2d:%.2d.%.2d  %.2x/%.2x:%.3d/%.3d  %4dcmd/s\x1b[G",totalSeconds/3600,(totalSeconds/60)%60,totalSeconds%60,totalTicks/10000,curOrder,curSubSong->ordersLen,curRow,curSubSong->patLen,cmdsPerSecond);
  }

  
  // halt engine if requested (debug menu)
  if (haltOn==DIV_HALT_TICK) halted=true;

  return ret;
}

// returns the buffer position. used by audio export.
int DivEngine::getBufferPos() {
  return bufferPos;
}

// runs MIDI clock.
void DivEngine::runMidiClock(int totalCycles) {
  // not in freelance mode
  if (freelance) return;
  midiClockCycles-=totalCycles;
  // run by the amount of cycles
  while (midiClockCycles<=0) {
    // send MIDI clock event
    curMidiClock++;
    if (output) if (!skipping && output->midiOut!=NULL && midiOutClock) {
      output->midiOut->send(TAMidiMessage(TA_MIDI_CLOCK,0,0));
    }

    // calculate tempo using highlight, timeBase, tick rate, speeds and virtual tempo
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
    // avoid a division by zer
    if (bpm<1.0) bpm=1.0;
    int increment=got.rate/(bpm);
    // increment should be at least 1
    if (increment<1) increment=1;

    // drift is for precision
    midiClockCycles+=increment;
    midiClockDrift+=fmod(got.rate,(double)(bpm));
    if (midiClockDrift>=(bpm)) {
      midiClockDrift-=(bpm);
      midiClockCycles++;
    }
  }
}

// runs MIDI timecode.
void DivEngine::runMidiTime(int totalCycles) {
  // not in freelance mode
  if (freelance) return;
  // not if the rate is too low
  if (got.rate<1) return;
  // run by the amount of cycles
  midiTimeCycles-=totalCycles;
  while (midiTimeCycles<=0) {
    if (curMidiTimePiece==0) {
      curMidiTimeCode=curMidiTime;
    }
    if (!(curMidiTimePiece&3)) curMidiTime++;

    double frameRate=96.0;
    int timeRate=midiOutTimeRate;
    // determine the rate depending on tick rate if set to automatic
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

    // calculate the current time
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

    // output timecode
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

// these two functions are either leftovers or something or they are there for test purposes.
// I don't remember very well.
void _runDispatch1(void* d) {
}

void _runDispatch2(void* d) {

}

// this fills the audio buffer and runs tbe engine.
// called by the audio backend and during audio export.
void DivEngine::nextBuf(float** in, float** out, int inChans, int outChans, unsigned int size) {
  // debug information
  lastNBIns=inChans;
  lastNBOuts=outChans;
  lastNBSize=size;

  // don't fill a buffer if the size is 0
  if (!size) {
    logW("nextBuf called with size 0!");
    return;
  }
  lastLoopPos=-1;

  // clear the output
  if (out!=NULL) {
    for (int i=0; i<outChans; i++) {
      memset(out[i],0,size*sizeof(float));
    }
  }

  // check the mutex.
  // soft-locking happens when synchronizedSoft is called.
  if (softLocked) {
    // in this case we just return
    if (!isBusy.try_lock()) {
      logV("audio is soft-locked (%d)",softLockCount++);
      return;
    }
  } else {
    isBusy.lock();
  }
  got.bufsize=size;

  // this is used to calculate audio load
  std::chrono::steady_clock::time_point ts_processBegin=std::chrono::steady_clock::now();

  // set up the render thread pool
  if (renderPool==NULL) {
    unsigned int howManyThreads=song.systemLen;
    if (howManyThreads<2) howManyThreads=0;
    if (howManyThreads>renderPoolThreads) howManyThreads=renderPoolThreads;
    renderPool=new DivWorkPool(howManyThreads);
  }

  // process MIDI input events
  if (output) if (output->midiIn) while (!output->midiIn->queue.empty()) {
    TAMidiMessage& msg=output->midiIn->queue.front();
    // print MIDI events if MIDI debug is enabled
    if (midiDebug) {
      if (msg.type==TA_MIDI_SYSEX) {
        logD("MIDI debug: %.2X SysEx",msg.type);
      } else {
        logD("MIDI debug: %.2X %.2X %.2X",msg.type,msg.data[0],msg.data[1]);
      }
    }
    // call the MIDI callback, which may process this event further.
    // the function should return an instrument index, which will be used
    // for all forthcoming notes.
    // special values:
    // - -1: don't change
    // - -2: "preview" instrument
    // - -3: cancel event (do not add to pending notes)
    int ins=-1;
    if ((ins=midiCallback(msg))!=-3) {
      // process event if not canceled
      int chan=msg.type&15;
      switch (msg.type&0xf0) {
        case TA_MIDI_NOTE_OFF: {
          if (midiIsDirect) {
            // in direct mode, map the event directly to the channel
            if (chan<0 || chan>=chans) break;
            pendingNotes.push_back(DivNoteEvent(chan,-1,-1,-1,false,false,true));
          } else {
            // find a suitable channel and add this event to the queue
            autoNoteOff(msg.type&15,msg.data[0]-12,msg.data[1]);
          }
          // start the engine if necessary
          if (!playing) {
            reset();
            freelance=true;
            playing=true;
          }
          break;
        }
        case TA_MIDI_NOTE_ON: {
          // trigger note off if the velocity is 0
          if (msg.data[1]==0) {
            if (midiIsDirect) {
              // in direct mode, map the event directly to the channel
              if (chan<0 || chan>=chans) break;
              pendingNotes.push_back(DivNoteEvent(chan,-1,-1,-1,false,false,true));
            } else {
              // find a suitable channel and add this event to the queue
              autoNoteOff(msg.type&15,msg.data[0]-12,msg.data[1]);
            }
          } else {
            if (midiIsDirect) {
              // in direct mode, map the event directly to the channel
              if (chan<0 || chan>=chans) break;
              pendingNotes.push_back(DivNoteEvent(chan,ins,msg.data[0]-12,msg.data[1],true,false,true));
            } else {
              // find a suitable channel and add this event to the queue
              autoNoteOn(msg.type&15,ins,msg.data[0]-12,msg.data[1]);
            }
          }
          break;
        }
        case TA_MIDI_PROGRAM: {
          // program changes in direct mode are handled here
          // the GUI should cancel this event and change the current instrument
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
  
  // process sample/wave preview (not during audio export)
  if (((sPreview.sample>=0 && sPreview.sample<(int)song.sample.size()) || (sPreview.wave>=0 && sPreview.wave<(int)song.wave.size())) && !exporting) {
    // we use blip_buf to pitch the sample
    unsigned int samp_bbOff=0;
    // if there are samples, flush them (this can happen when the playback
    // rate is less than the output rate)
    unsigned int prevAvail=blip_samples_avail(samp_bb);
    if (prevAvail>size) prevAvail=size;
    if (prevAvail>0) {
      blip_read_samples(samp_bb,samp_bbOut,prevAvail,0);
      samp_bbOff=prevAvail;
    }
    // prepare to fill the buffer
    size_t prevtotal=blip_clocks_needed(samp_bb,size-prevAvail);

    // play the sample
    if (sPreview.sample>=0 && sPreview.sample<(int)song.sample.size()) {
      DivSample* s=song.sample[sPreview.sample];

      for (size_t i=0; i<prevtotal; i++) {
        if (sPreview.pos>=(int)s->samples || (sPreview.pEnd>=0 && sPreview.pos>=sPreview.pEnd)) {
          // zero if out of bounds
          samp_temp=0;
        } else {
          // fetch sample
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
        // insert sample
        blip_add_delta(samp_bb,i,samp_temp-samp_prevSample);
        samp_prevSample=samp_temp;

        // check playback direction and move needle
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

  // process audio (run the engine)
  bool mustPlay=playing && !halted;
  if (mustPlay) {
    // logic starts here
    // first reset the run position of all dispatches
    for (int i=0; i<song.systemLen; i++) {
      disCont[i].runPos=0;
    }

    // resize the metronome tick buffer if necessary
    if (metroTickLen<size) {
      if (metroTick!=NULL) delete[] metroTick;
      metroTick=new unsigned char[size];
      metroTickLen=size;
    }

    // reset the metronome tick buffer
    memset(metroTick,0,size);

    // this variable counts how many loops we had to go through in order to fill audio buffer
    // it prevents hangs under extraordinary bug situations
    int attempts=0;
    int runLeftG=size;
    // run until the buffer is full or we believe the engine stalled
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
          // used by audio export to determine how many samples to write (otherwise it'll add silence at the end)
          lastLoopPos=size-runLeftG;
          logD("last loop pos: %d for a size of %d and runLeftG of %d",lastLoopPos,size,runLeftG);
          totalLoops++;
          // stop playing once we hit a specific number of loops (set during audio export)
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
        // check whether we gotta insert a metronome tick
        if (pendingMetroTick) {
          unsigned int realPos=size-runLeftG;
          if (realPos>=size) realPos=size-1;
          metroTick[realPos]=pendingMetroTick;
          pendingMetroTick=0;
        }
      } else {
        // we don't have to tick yet. run chip dispatches.
        // 3. run MIDI clock
        int midiTotal=MIN(cycles,runLeftG);
        runMidiClock(midiTotal);

        // 4. run MIDI timecode
        runMidiTime(midiTotal);

        // 5. tick the clock and fill buffers as needed
        // check which is nearest: a tick or end of audio buffer
        if (cycles<runLeftG) {
          // a tick will happen before the buffer ends
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
              
              // if the buffer is too small, resize it
              int total=blip_clocks_needed(dc->bb[0],dc->cycles);
              if (total>(int)dc->bbInLen) {
                logD("growing dispatch %p bbIn to %d",(void*)dc,total+256);
                dc->grow(total+256);
              }
              dc->acquire(total);
              dc->fillBuf(total,dc->runPos,dc->cycles);
              // advance run position
              dc->runPos+=dc->cycles;
            },&disCont[i]);
          }
          renderPool->wait();
          runLeftG-=cycles;
          cycles=0;
        } else {
          // the buffer will end before a tick happens
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
          // at this point runLeftG will be zero and we can break out of the loop
          runLeftG=0;
          renderPool->wait();
        }
      }
    }

    // complain and stop playback if we believe the engine has stalled
    //logD("attempts: %d",attempts);
    if (attempts>=(int)(size+10)) {
      logE("hang detected! stopping! at %d seconds %d micro (%d>=%d)",totalSeconds,totalTicks,attempts,(int)size);
      freelance=false;
      playing=false;
      extValuePresent=false;
    }
    // this is also used by audio export to cut out unnecessary silence after a stop song effect (FFxx)
    totalProcessed=size-runLeftG;

    // complain if a dispatch's audio buffer must be flushed and our audio buffer is too small for it
    // this may happen when a chip's output rate is lower than the sample rate
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

  // process file player
  // resize file player audio buffer if necessary
  if (filePlayerBufLen<size) {
    for (int i=0; i<DIV_MAX_OUTPUTS; i++) {
      if (filePlayerBuf[i]!=NULL) delete[] filePlayerBuf[i];
      filePlayerBuf[i]=new float[size];
    }
    filePlayerBufLen=size;
  }
  if (curFilePlayer!=NULL && !exporting) {
    curFilePlayer->mix(filePlayerBuf,outChans,size);
  } else {
    for (int i=0; i<DIV_MAX_OUTPUTS; i++) {
      memset(filePlayerBuf[i],0,size*sizeof(float));
    }
  }

  // process metronome
  // resize the metronome's audio buffer if necessary
  if (metroBufLen<size || metroBuf==NULL) {
    if (metroBuf!=NULL) delete[] metroBuf;
    metroBuf=new float[size];
    metroBufLen=size;
  }

  memset(metroBuf,0,metroBufLen*sizeof(float));

  // insert metronome ticks
  // a 1400Hz tick is used for bars (highlight 2) and a 1050Hz one for beats (highlight 1)
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
      // mix in the tick
      if (metroAmp>0.0f) {
        for (int j=0; j<outChans; j++) {
          metroBuf[i]=(sin(metroPos*2*M_PI))*metroAmp*metroVol;
        }
      }
      // decay
      metroAmp-=0.0003f;
      if (metroAmp<0.0f) metroAmp=0.0f;
      metroPos+=metroFreq;
      while (metroPos>=1) metroPos--;
    }
  }

  // calculate volume of reference file player (so we can attenuate the rest according to the mix slider)
  // -1 to 0: player volume goes from 0% to 100%
  // 0 to +1: tracker volume goes from 100% to 0%
  float refPlayerVol=1.0f;
  if (curFilePlayer!=NULL) {
    // only if the player window is open
    if (curFilePlayer->getActive()) {
      refPlayerVol=1.0f-curFilePlayer->getVolume();
      if (refPlayerVol>1.0f) refPlayerVol=1.0f;
    }
  }

  // now mix everything (resolve patchbay)
  for (unsigned int i: song.patchbay) {
    // there are 4096 portsets. each portset may have up to 16 outputs (subports).
    const unsigned short srcPort=i>>16;
    const unsigned short destPort=i&0xffff;

    const unsigned short srcPortSet=srcPort>>4;
    const unsigned short destPortSet=destPort>>4;
    const unsigned char srcSubPort=srcPort&15;
    const unsigned char destSubPort=destPort&15;

    // null portset (disconnected)
    if (destPortSet==0xfff) continue;

    // system outputs (the audio buffer)
    if (destPortSet==0x000) {
      if (destSubPort>=outChans) continue;

      // chip outputs
      if (srcPortSet<song.systemLen && playing && !halted) {
        if (srcSubPort<disCont[srcPortSet].dispatch->getOutputCount()) {
          float vol=song.systemVol[srcPortSet]*disCont[srcPortSet].dispatch->getPostAmp()*song.masterVol*refPlayerVol;

          // apply volume and panning
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
      } else if (srcPortSet==0xffc) {
        // file player
        for (size_t j=0; j<size; j++) {
          out[destSubPort][j]+=filePlayerBuf[srcSubPort][j];
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

  // dump to oscillator buffer (a ring buffer)
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
    if (disp==NULL) continue;
    for (int j=0; j<disp->getOutputCount(); j++) {
      if (disCont[i].bbOut[j]==NULL) continue;
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
        if (out[j][i]<-0.9999) out[j][i]=-0.9999;
        if (out[j][i]>0.9999) out[j][i]=0.9999;
      }
    }
  }
  isBusy.unlock();

  std::chrono::steady_clock::time_point ts_processEnd=std::chrono::steady_clock::now();

  // this is shown in the GUI as audio load
  processTime=std::chrono::duration_cast<std::chrono::nanoseconds>(ts_processEnd-ts_processBegin).count();
}
