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

// dispatch.h: definition of a DivDispatch and related things.

#ifndef _DISPATCH_H
#define _DISPATCH_H

#include <stdlib.h>
#include <string.h>
#include "../pch.h"
#include "blip_buf.h"
#include "config.h"
#include "defines.h"
#include "macroInt.h"

// custom clock limits
// these values are intentional. they prevent the user from overloading the engine by using extraneous clocks.
#define MIN_CUSTOM_CLOCK 100000
#define MAX_CUSTOM_CLOCK 40000000


#define addWrite(a,v) regWrites.push_back(DivRegWrite(a,v));

// forward declarations
class DivEngine;
class DivMacroInt;
struct DivSample;

/**
 * DivDispatchCmds - the enum containing all engine commands.
 * these are sent from the engine to dispatches during playback.
 *
 * HOW TO ADD A NEW COMMAND:
 * - append it to this enum.
 *   - if you place it at the beginning or in the middle of this enum, you'll break the command stream format, so don't.
 * - in playback.cpp there is a const char* cmdName[] array, which contains the command names as strings for the commands (and other debug stuff).
 *   - if you miss it, Furnace won't compile.
 *
 * the comments are: (arg1, arg2) -> val
 * not all commands have a return value
 */
enum DivDispatchCmds {
  DIV_CMD_NOTE_ON=0, // (note)
  DIV_CMD_NOTE_OFF,
  DIV_CMD_NOTE_OFF_ENV,
  DIV_CMD_ENV_RELEASE,
  DIV_CMD_INSTRUMENT, // (ins, force)
  DIV_CMD_VOLUME, // (vol)
  // TODO: think of possibly moving this
  DIV_CMD_GET_VOLUME, // () -> vol
  // TODO: move. shouldn't be a command.
  DIV_CMD_GET_VOLMAX, // () -> volMax
  DIV_CMD_NOTE_PORTA, // (target, speed) -> 2 if target reached
  DIV_CMD_PITCH, // (pitch)
  DIV_CMD_PANNING, // (left, right)
  DIV_CMD_LEGATO, // (note)
  DIV_CMD_PRE_PORTA, // (inPorta, isPortaOrSlide)
  DIV_CMD_PRE_NOTE, // used in C64 (note)

  // these will be used in ROM export.
  // do NOT implement!
  DIV_CMD_HINT_VIBRATO, // (speed, depth)
  DIV_CMD_HINT_VIBRATO_RANGE, // (range)
  DIV_CMD_HINT_VIBRATO_SHAPE, // (shape)
  DIV_CMD_HINT_PITCH, // (pitch)
  DIV_CMD_HINT_ARPEGGIO, // (note1, note2)
  DIV_CMD_HINT_VOLUME, // (vol)
  DIV_CMD_HINT_VOL_SLIDE, // (amount, oneTick)
  DIV_CMD_HINT_PORTA, // (target, speed)
  DIV_CMD_HINT_LEGATO, // (note)
  DIV_CMD_HINT_VOL_SLIDE_TARGET, // (amount, target)
  DIV_CMD_HINT_TREMOLO, // (speed/depth as a byte)
  DIV_CMD_HINT_PANBRELLO, // (speed/depth as a byte)
  DIV_CMD_HINT_PAN_SLIDE, // (speed)
  DIV_CMD_HINT_PANNING, // (left, right)

  DIV_CMD_SAMPLE_MODE, // (enabled)
  DIV_CMD_SAMPLE_FREQ, // (frequency)
  DIV_CMD_SAMPLE_BANK, // (bank)
  DIV_CMD_SAMPLE_POS, // (pos)
  DIV_CMD_SAMPLE_DIR, // (direction)

  DIV_CMD_FM_HARD_RESET, // (enabled)
  DIV_CMD_FM_LFO, // (speed)
  DIV_CMD_FM_LFO_WAVE, // (waveform)
  DIV_CMD_FM_TL, // (op, value)
  DIV_CMD_FM_AM, // (op, value)
  DIV_CMD_FM_AR, // (op, value)
  DIV_CMD_FM_DR, // (op, value)
  DIV_CMD_FM_SL, // (op, value)
  DIV_CMD_FM_D2R, // (op, value)
  DIV_CMD_FM_RR, // (op, value)
  DIV_CMD_FM_DT, // (op, value)
  DIV_CMD_FM_DT2, // (op, value)
  DIV_CMD_FM_RS, // (op, value)
  DIV_CMD_FM_KSR, // (op, value)
  DIV_CMD_FM_VIB, // (op, value)
  DIV_CMD_FM_SUS, // (op, value)
  DIV_CMD_FM_WS, // (op, value)
  DIV_CMD_FM_SSG, // (op, value)
  DIV_CMD_FM_REV, // (op, value)
  DIV_CMD_FM_EG_SHIFT, // (op, value)
  DIV_CMD_FM_FB, // (value)
  DIV_CMD_FM_MULT, // (op, value)
  DIV_CMD_FM_FINE, // (op, value)
  DIV_CMD_FM_FIXFREQ, // (op, value)
  DIV_CMD_FM_EXTCH, // (enabled)
  DIV_CMD_FM_AM_DEPTH, // (depth)
  DIV_CMD_FM_PM_DEPTH, // (depth)

  DIV_CMD_FM_LFO2, // (speed)
  DIV_CMD_FM_LFO2_WAVE, // (waveform)

  DIV_CMD_STD_NOISE_FREQ, // (freq)
  DIV_CMD_STD_NOISE_MODE, // (mode)

  DIV_CMD_WAVE, // (waveform)
  
  DIV_CMD_GB_SWEEP_TIME, // (time)
  DIV_CMD_GB_SWEEP_DIR, // (direction)

  DIV_CMD_PCE_LFO_MODE, // (mode)
  DIV_CMD_PCE_LFO_SPEED, // (speed)

  DIV_CMD_NES_SWEEP, // (direction, value)
  DIV_CMD_NES_DMC, // (value)

  DIV_CMD_C64_CUTOFF, // (value)
  DIV_CMD_C64_RESONANCE, // (value)
  DIV_CMD_C64_FILTER_MODE, // (value)
  DIV_CMD_C64_RESET_TIME, // (value)
  DIV_CMD_C64_RESET_MASK, // (mask)
  DIV_CMD_C64_FILTER_RESET, // (value)
  DIV_CMD_C64_DUTY_RESET, // (value)
  DIV_CMD_C64_EXTENDED, // (value)
  DIV_CMD_C64_FINE_DUTY, // (value)
  DIV_CMD_C64_FINE_CUTOFF, // (value)

  DIV_CMD_AY_ENVELOPE_SET,
  DIV_CMD_AY_ENVELOPE_LOW,
  DIV_CMD_AY_ENVELOPE_HIGH,
  DIV_CMD_AY_ENVELOPE_SLIDE,
  DIV_CMD_AY_NOISE_MASK_AND, // (value)
  DIV_CMD_AY_NOISE_MASK_OR, // (value)
  DIV_CMD_AY_AUTO_ENVELOPE, // (value)
  DIV_CMD_AY_IO_WRITE, // (port, value)
  DIV_CMD_AY_AUTO_PWM,

  DIV_CMD_FDS_MOD_DEPTH,
  DIV_CMD_FDS_MOD_HIGH,
  DIV_CMD_FDS_MOD_LOW,
  DIV_CMD_FDS_MOD_POS,
  DIV_CMD_FDS_MOD_WAVE,

  DIV_CMD_SAA_ENVELOPE, // (value)

  DIV_CMD_AMIGA_FILTER, // (enabled)
  DIV_CMD_AMIGA_AM, // (enabled)
  DIV_CMD_AMIGA_PM, // (enabled)

  DIV_CMD_LYNX_LFSR_LOAD, // (value)
  
  DIV_CMD_QSOUND_ECHO_FEEDBACK,
  DIV_CMD_QSOUND_ECHO_DELAY,
  DIV_CMD_QSOUND_ECHO_LEVEL,
  DIV_CMD_QSOUND_SURROUND,

  DIV_CMD_X1_010_ENVELOPE_SHAPE,
  DIV_CMD_X1_010_ENVELOPE_ENABLE,
  DIV_CMD_X1_010_ENVELOPE_MODE,
  DIV_CMD_X1_010_ENVELOPE_PERIOD,
  DIV_CMD_X1_010_ENVELOPE_SLIDE,
  DIV_CMD_X1_010_AUTO_ENVELOPE,
  DIV_CMD_X1_010_SAMPLE_BANK_SLOT,

  DIV_CMD_WS_SWEEP_TIME, // (time)
  DIV_CMD_WS_SWEEP_AMOUNT, // (value)

  DIV_CMD_N163_WAVE_POSITION,
  DIV_CMD_N163_WAVE_LENGTH,
  DIV_CMD_N163_WAVE_UNUSED1,
  DIV_CMD_N163_WAVE_UNUSED2,
  DIV_CMD_N163_WAVE_LOADPOS,
  DIV_CMD_N163_WAVE_LOADLEN,
  DIV_CMD_N163_WAVE_UNUSED3,
  DIV_CMD_N163_CHANNEL_LIMIT,
  DIV_CMD_N163_GLOBAL_WAVE_LOAD,
  DIV_CMD_N163_GLOBAL_WAVE_LOADPOS,
  DIV_CMD_N163_UNUSED4,
  DIV_CMD_N163_UNUSED5,

  DIV_CMD_SU_SWEEP_PERIOD_LOW, // (which, val)
  DIV_CMD_SU_SWEEP_PERIOD_HIGH, // (which, val)
  DIV_CMD_SU_SWEEP_BOUND, // (which, val)
  DIV_CMD_SU_SWEEP_ENABLE, // (which, val)
  DIV_CMD_SU_SYNC_PERIOD_LOW,
  DIV_CMD_SU_SYNC_PERIOD_HIGH,

  DIV_CMD_ADPCMA_GLOBAL_VOLUME,

  DIV_CMD_SNES_ECHO,
  DIV_CMD_SNES_PITCH_MOD,
  DIV_CMD_SNES_INVERT,
  DIV_CMD_SNES_GAIN_MODE,
  DIV_CMD_SNES_GAIN,
  DIV_CMD_SNES_ECHO_ENABLE,
  DIV_CMD_SNES_ECHO_DELAY,
  DIV_CMD_SNES_ECHO_VOL_LEFT,
  DIV_CMD_SNES_ECHO_VOL_RIGHT,
  DIV_CMD_SNES_ECHO_FEEDBACK,
  DIV_CMD_SNES_ECHO_FIR,

  DIV_CMD_NES_ENV_MODE,
  DIV_CMD_NES_LENGTH,
  DIV_CMD_NES_COUNT_MODE,

  DIV_CMD_MACRO_OFF, // (which)
  DIV_CMD_MACRO_ON, // (which)

  DIV_CMD_SURROUND_PANNING, // (out, val)

  DIV_CMD_FM_AM2_DEPTH, // (depth)
  DIV_CMD_FM_PM2_DEPTH, // (depth)

  DIV_CMD_ES5506_FILTER_MODE, // (value)
  DIV_CMD_ES5506_FILTER_K1, // (value, mask)
  DIV_CMD_ES5506_FILTER_K2, // (value, mask)
  DIV_CMD_ES5506_FILTER_K1_SLIDE, // (value, negative)
  DIV_CMD_ES5506_FILTER_K2_SLIDE, // (value, negative)
  DIV_CMD_ES5506_ENVELOPE_COUNT, // (count)
  DIV_CMD_ES5506_ENVELOPE_LVRAMP, // (ramp)
  DIV_CMD_ES5506_ENVELOPE_RVRAMP, // (ramp)
  DIV_CMD_ES5506_ENVELOPE_K1RAMP, // (ramp, slowdown)
  DIV_CMD_ES5506_ENVELOPE_K2RAMP, // (ramp, slowdown)
  DIV_CMD_ES5506_PAUSE, // (value)

  DIV_CMD_HINT_ARP_TIME, // (value)

  DIV_CMD_SNES_GLOBAL_VOL_LEFT,
  DIV_CMD_SNES_GLOBAL_VOL_RIGHT,

  DIV_CMD_NES_LINEAR_LENGTH,

  DIV_CMD_EXTERNAL, // (value)

  DIV_CMD_C64_AD, // (value)
  DIV_CMD_C64_SR, // (value)

  DIV_CMD_ESFM_OP_PANNING, // (op, value)
  DIV_CMD_ESFM_OUTLVL, // (op, value)
  DIV_CMD_ESFM_MODIN, // (op, value)
  DIV_CMD_ESFM_ENV_DELAY, // (op, value)

  DIV_CMD_MACRO_RESTART, // (which)

  DIV_CMD_POWERNOISE_COUNTER_LOAD, // (which, val)
  DIV_CMD_POWERNOISE_IO_WRITE, // (port, value)
  
  DIV_CMD_DAVE_HIGH_PASS,
  DIV_CMD_DAVE_RING_MOD,
  DIV_CMD_DAVE_SWAP_COUNTERS,
  DIV_CMD_DAVE_LOW_PASS,
  DIV_CMD_DAVE_CLOCK_DIV,

  DIV_CMD_MINMOD_ECHO,

  DIV_CMD_BIFURCATOR_STATE_LOAD,
  DIV_CMD_BIFURCATOR_PARAMETER,

  DIV_CMD_FDS_MOD_AUTO,

  DIV_CMD_FM_OPMASK, // (mask)

  DIV_CMD_MULTIPCM_MIX_FM, // (value)
  DIV_CMD_MULTIPCM_MIX_PCM, // (value)
  DIV_CMD_MULTIPCM_LFO, // (value)
  DIV_CMD_MULTIPCM_VIB, // (value)
  DIV_CMD_MULTIPCM_AM, // (value)
  DIV_CMD_MULTIPCM_AR, // (value)
  DIV_CMD_MULTIPCM_D1R, // (value)
  DIV_CMD_MULTIPCM_DL, // (value)
  DIV_CMD_MULTIPCM_D2R, // (value)
  DIV_CMD_MULTIPCM_RC, // (value)
  DIV_CMD_MULTIPCM_RR, // (value)
  DIV_CMD_MULTIPCM_DAMP, // (value)
  DIV_CMD_MULTIPCM_PSEUDO_REVERB, // (value)
  DIV_CMD_MULTIPCM_LFO_RESET, // (value)
  DIV_CMD_MULTIPCM_LEVEL_DIRECT, // (value)
  
  DIV_CMD_SID3_SPECIAL_WAVE,
  DIV_CMD_SID3_RING_MOD_SRC,
  DIV_CMD_SID3_HARD_SYNC_SRC,
  DIV_CMD_SID3_PHASE_MOD_SRC,
  DIV_CMD_SID3_WAVE_MIX,
  DIV_CMD_SID3_LFSR_FEEDBACK_BITS,
  DIV_CMD_SID3_1_BIT_NOISE,
  DIV_CMD_SID3_FILTER_DISTORTION,
  DIV_CMD_SID3_FILTER_OUTPUT_VOLUME,
  DIV_CMD_SID3_CHANNEL_INVERSION,
  DIV_CMD_SID3_FILTER_CONNECTION,
  DIV_CMD_SID3_FILTER_MATRIX,
  DIV_CMD_SID3_FILTER_ENABLE,

  DIV_CMD_C64_PW_SLIDE,
  DIV_CMD_C64_CUTOFF_SLIDE,

  DIV_CMD_SID3_PHASE_RESET,
  DIV_CMD_SID3_NOISE_PHASE_RESET,
  DIV_CMD_SID3_ENVELOPE_RESET,

  DIV_CMD_SID3_CUTOFF_SCALING,
  DIV_CMD_SID3_RESONANCE_SCALING,

  DIV_CMD_WS_GLOBAL_SPEAKER_VOLUME, // (multiplier)

  DIV_CMD_FM_ALG,
  DIV_CMD_FM_FMS,
  DIV_CMD_FM_AMS,
  DIV_CMD_FM_FMS2,
  DIV_CMD_FM_AMS2,

  DIV_CMD_MAX
};


/**
 * currently we don't use this but eventually we will.
 */
struct DivPitchTable {
  int pitch[12+1];
  int pitchDiff[12+1];
  unsigned int maxFreq;
  unsigned char blockBits, shift;
  bool period, linearity;

  // get pitch
  int get(int base, int pitch1, int pitch2);

  // linear: note
  // non-linear: get(note,0,0)
  int getBase(int note);

  /**
   * calculate pitch table.
   * @param tuning the A-4 tuning to use.
   * @param clock the chip's clock.
   * @param divider the divider or frequency base.
   * @param maximum the maximum period/frequency value supported by the chip.
   * @param period whether to use periods instead of accumulator values.
   * @param linear whether pitch linearity is set to full.
   */
  void init(float tuning, double clock, double divider, int maximum, bool period, bool linear);

  DivPitchTable():
    maxFreq(0xffffffff),
    blockBits(0),
    period(false),
    linearity(true) {
    memset(pitch,0,sizeof(pitch));
    memset(pitchDiff,0,sizeof(pitchDiff));
  }
};

/**
 * the SharedChannel struct holds common channel state, such as frequency, volume, note activity and so on.
 * this is used by almost every dispatch.
 *
 * create a struct inherited from SharedChannel in your dispatch's class definition:
 *
 * struct Channel: public SharedChannel {
 *   // state...
 * };
 */
struct SharedChannel {
  // freq: the output frequency (usually).
  // - this is calculated on frequency changes (freqChanged should be checked during tick()).
  // - the function that calculates frequency is DivEngine::calcFreq(). pass in the rest of variables
  //   and you should get a frequency.
  // - certain chips require conversion of this frequency to some usable value
  //   (e.g. SAA1099 has 8-bit divider and 3-bit octave selector).
  // baseFreq: frequency of the current note, including pitch slides.
  // - linear pitch: 8.7 fixed number. integer part is note and fractional part is pitch (in 128ths).
  // - non-linear pitch: 
  // - this value is set during note changes. calculate it by using NOTE_FREQUENCY() or NOTE_PERIODIC().
  //   - remember to set CHIP_DIVIDER/CHIP_FREQBASE in your dispatch's code!
  // baseNoteOverride: set when the arp macro's value is fixed. in that case, fixedArp will be true.
  // pitch: the pitch offset. set on DIV_CMD_PITCH (calculated from current E5xx and vibrato state).
  // pitch2: pitch macro's output.
  // arpOff: the arp macro's value (relative), in semitones.
  int freq, baseFreq, baseNoteOverride, pitch, pitch2, arpOff;
  // ins: current instrument. -1 is none/default.
  // note: current note, in semitones. 0 is C-(-5) and 60 is C-0.
  // sampleNote: note in sample map.
  // sampleNoteDelta: difference between note and sampleNote, used in arp calculation, legato and pitch slides.
  int ins, note, sampleNote, sampleNoteDelta;
  // active: whether the note is currently on.
  // insChanged: whether an instrument change has occurred.
  // - if this is true, make sure to commit the new instrument during DIV_CMD_NOTE_ON and set this to false.
  // freqChanged: whether freq needs to be updated.
  // - should be set during DIV_CMD_NOTE_ON, DIV_CMD_NOTE_PORTA, DIV_CMD_LEGATO, DIV_CMD_PITCH and other
  //   commands which alter pitch or baseFreq.
  // - should also be set by the arp macro handler.
  // - check for this variable on tick().
  // keyOn: whether there's a pending note on.
  // - the checks for freqChanged and keyOn are usually fused. see pce.cpp to see what I mean.
  // - check for this variable on tick().
  // keyOff: whether there's a pending note off.
  // - the checks for freqChanged and keyOff are usually fused. see pce.cpp to see what I mean.
  // - check for this variable on tick().
  // portaPause: used by the FM chips for compatibility.
  // - please pretend this variable doesn't exist...
  // inPorta: whether we currently are in a portamento.
  // - should be set during DIV_CMD_PRE_PORTA.
  // - in non-linear pitch, this variable is used to inhibit certain pitch changes during a pitch slide.
  // rawFreq: whether the baseFreq is raw and overrides frequency calculation.
  bool active, insChanged, freqChanged, fixedArp, keyOn, keyOff, portaPause, inPorta, rawFreq;
  // vol: the current volume, set during DIV_CMD_VOLUME.
  // outVol: the *output* volume.
  // - this is the same as vol when we don't have a volume macro going on.
  // - otherwise it is the result of a calculation with vol and the volume macro's value.
  //   - calculate this value by using VOL_SCALE_LINEAR()/VOL_SCALE_LOG() in tick().
  int vol, outVol;
  // std: this is the macro interpreter.
  // - the name comes from DefleMask, where macro-able instruments have "STD" type.
  //   - don't laugh at me.
  // - initialize it during DIV_CMD_NOTE_ON. use the macroInit() helper.
  // - de-initialize it (by calling macroInit(NULL)) during DIV_CMD_NOTE_OFF.
  //   - don't do this if your chip supports hardware envelopes!
  // - call mask() during DIV_CMD_MACRO_OFF/DIV_CMD_MACRO_ON.
  // - call restart() during DIV_CMD_MACRO_RESTART.
  // - call next() during tick().
  // - make sure to bind the engine during reset()! use setEngine().
  // - also remember to call notifyInsDeletion() on notifyInsDeletion().
  //   - if you don't do this, you'll be referencing a potentially extinct instrument
  //     and prompt Furnace to collapse.
  DivMacroInt std;
  // this is a pointer to your dispatch's pitch table.
  // - this should be initialized during reset()!
  DivPitchTable* pitchTable;

  // here are some helper functions.
  /**
   * handle arpeggio. call during tick() like so:
   *
   * if (NEW_ARP_STRAT) {
   *   chan[i].handleArp();
   * } else if (chan[i].std.arp.had) {
   *   if (!chan[i].inPorta) {
   *     chan[i].baseFreq=NOTE_PERIODIC(parent->calcArp(chan[i].note,chan[i].std.arp.val));
   *   }
   *   chan[i].freqChanged=true;
   * }
   *
   * the reason why we do it like that is because there are two arp strategies.
   * one of them allows you to run a pitch slide and arp macro simultaneously. the other one does not...
   * the latter is used in non-linear pitch or when the old arp strategy compat flag is enabled.
   * @param offset disregard. I don't remember what's this for.
   */
  void handleArp(int offset=0) {
    if (std.arp.had) {
      if (std.arp.val<0) {
        if (!(std.arp.val&0x40000000)) {
          baseNoteOverride=(std.arp.val|0x40000000)+offset+60;
          fixedArp=true;
        } else {
          arpOff=std.arp.val;
          fixedArp=false;
        }
      } else {
        if (std.arp.val&0x40000000) {
          baseNoteOverride=(std.arp.val&(~0x40000000))+offset+60;
          fixedArp=true;
        } else {
          arpOff=std.arp.val;
          fixedArp=false;
        }
      }
      freqChanged=true;
    }
  }
  /**
   * initializes the macro interpreter. call during DIV_CMD_NOTE_ON or DIV_CMD_NOTE_OFF (when applicable).
   * don't call std.init() directly! we need some other variables to be set as well.
   * @param which the instrument to read. set this to NULL to reset state.
   */
  void macroInit(DivInstrument* which) {
    std.init(which);
    pitch2=0;
    arpOff=0;
    baseNoteOverride=0;
    fixedArp=false;
  }
  /**
   * calculates base frequency from the current pitch table. use this when setting baseFreq.
   * @param note the note.
   */
  int calcBaseFreq(int note) {
    if (pitchTable==NULL) return 0;
    return pitchTable->getBase(note);
  }
  /**
   * calculates final frequency from current frequency values.
   * @return the frequency.
   */
  int calcFreq() {
    if (rawFreq) return baseFreq;
    if (pitchTable==NULL) return 0;
    if (!pitchTable->linearity) {
      return pitchTable->get(baseFreq,pitch,pitch2);
    }
    if (fixedArp) {
      return pitchTable->get(baseNoteOverride<<7,pitch,pitch2);
    }
    return pitchTable->get(baseFreq+(arpOff<<7),pitch,pitch2);
  }
  /**
   * call this constructor in your Channel's constructor, which should initialize the channel's state.
   * call your Channel's constructor during reset().
   *
   * @param initVol the initial channel volume.
   */
  SharedChannel(int initVol, bool linear):
    freq(0),
    baseFreq(linear?0x1e00:0),
    baseNoteOverride(0),
    pitch(0),
    pitch2(0),
    arpOff(0),
    ins(-1),
    note(0),
    sampleNote(DIV_NOTE_NULL),
    sampleNoteDelta(0),
    active(false),
    insChanged(true),
    freqChanged(false),
    fixedArp(false),
    keyOn(false),
    keyOff(false),
    portaPause(false),
    inPorta(false),
    rawFreq(false),
    vol(initVol),
    outVol(initVol),
    std(),
    pitchTable(NULL) {} 
};

/**
 * DivPitchTableManager is a helper class that manages pitch tables for each sample.
 */
class DivPitchTableManager {
  DivEngine* e;
  DivPitchTable defaultPitchTable;
  DivPitchTable* samplePitchTable;
  size_t samplePitchTableLen;

  size_t eSongSampleSize();
  void updateSub(float tuning, double clock, double divider, int maximum, bool period, bool linear, int sample);

  public:
    /**
     * get pitch table for a sample.
     * @param sample the sample number.
     * @return a DivPitchTable for that sample, or NULL if it doesn't exist.
     */
    DivPitchTable* get(int sample);
    /**
     * update the pitch tables.
     * this function also updates references to the pitch tables in case the
     * pitch table array must be recreated.
     * @param chan an array of SharedChannel... hold on. this is not going to work well.
     * @return whether the number of pitch tables has changed.
     */
    template<class T> bool update(T* chan, size_t numChans, float tuning, double clock, double divider, int maximum, bool period, bool linear, int sample=-1) {
      if (e==NULL) return false;

      bool hasSizeChanged=false;

      // first check whether we need to resize our pitch table array
      if (samplePitchTableLen!=eSongSampleSize()) {
        if (eSongSampleSize()<1) {
          // remove all references to the pitch table
          DivPitchTable* firstEntry=samplePitchTable;
          DivPitchTable* lastEntry=&samplePitchTable[samplePitchTableLen-1];

          for (size_t i=0; i<numChans; i++) {
            if (chan[i].pitchTable>=firstEntry && chan[i].pitchTable<=lastEntry) {
              chan[i].pitchTable=NULL;
            }
          }

          // now deallocate it
          delete[] samplePitchTable;
          samplePitchTable=NULL;
        } else {
          // recreate the pitch table array
          DivPitchTable* newArray=new DivPitchTable[eSongSampleSize()];
          if (samplePitchTable) {
            memcpy(newArray,samplePitchTable,MIN(eSongSampleSize(),samplePitchTableLen)*sizeof(DivPitchTable));

            // adjust pitch table references
            DivPitchTable* firstEntry=samplePitchTable;
            DivPitchTable* lastEntry=&samplePitchTable[samplePitchTableLen-1];

            for (size_t i=0; i<numChans; i++) {
              if (chan[i].pitchTable>=firstEntry && chan[i].pitchTable<=lastEntry) {
                chan[i].pitchTable=newArray+(chan[i].pitchTable-firstEntry);
              }
            }

            delete[] samplePitchTable;
          }
          samplePitchTable=newArray;
        }
        samplePitchTableLen=eSongSampleSize();
        hasSizeChanged=true;
      }

      updateSub(tuning,clock,divider,maximum,period,linear,sample);
      return hasSizeChanged;
    }
    /**
     * delete the pitch tables.
     */
    template<class T> void destroy(T* chan, size_t numChans) {
      if (e==NULL) return;
      if (samplePitchTable) {
        DivPitchTable* firstEntry=samplePitchTable;
        DivPitchTable* lastEntry=&samplePitchTable[samplePitchTableLen-1];

        for (size_t i=0; i<numChans; i++) {
          if (chan[i].pitchTable>=firstEntry && chan[i].pitchTable<=lastEntry) {
            chan[i].pitchTable=NULL;
          }
        }

        delete[] samplePitchTable;
        samplePitchTable=NULL;
        samplePitchTableLen=0;
      }
    }
    /**
     * initialize this pitch table manager.
     */
    void init(DivEngine* eng);
    DivPitchTableManager():
      e(NULL),
      samplePitchTable(NULL),
      samplePitchTableLen(0) {}
    ~DivPitchTableManager();
};

/**
 * a DivCommand encapsulates an engine command.
 */
struct DivCommand {
  // the command type.
  DivDispatchCmds cmd;
  // chan: the destination channel.
  // - when generated in the engine, this is relative to the first channel in the song.
  // - when sent to the dispatch, the engine will remap it relative to the first channel in that dispatch.
  // dis: this is the same as chan, but does not become remapped.
  // - it always is relative to the first channel in the song.
  // - normally you shouldn't use this. it's only used during remapping.
  unsigned char chan, dis;
  // the two parameters of a command.
  int value, value2;

  // two-value constructor.
  DivCommand(DivDispatchCmds c, unsigned char ch, int val, int val2):
    cmd(c),
    chan(ch),
    dis(ch),
    value(val),
    value2(val2) {}
  // single-value constructor.
  DivCommand(DivDispatchCmds c, unsigned char ch, int val):
    cmd(c),
    chan(ch),
    dis(ch),
    value(val),
    value2(0) {}
  // no-parameter constructor.
  DivCommand(DivDispatchCmds c, unsigned char ch):
    cmd(c),
    chan(ch),
    dis(ch),
    value(0),
    value2(0) {}
};

/**
 * a delayed command, to be executed later.
 * currently unused.
 */
struct DivDelayedCommand {
  int ticks;
  DivCommand cmd;
};

/**
 * standard specification for a register write.
 * used in register dump-based exports (e.g. VGM).
 */
struct DivRegWrite {
  /**
   * the address.
   * an address of 0xffffxx00 indicates a Furnace specific command.
   * usually, instance refers to chip.
   * the following commands are available:
   * - 0xffffxx00: start sample playback
   *   - xx is the instance ID
   *   - value is the sample number
   * - 0xffffxx01: set sample rate
   *   - xx is the instance ID
   *   - value is the sample rate
   * - 0xffffxx02: stop sample playback
   *   - xx is the instance ID
   * - 0xffffxx03: set sample playback direction
   *   - x is the instance ID
   * - 0xffffxx04: switch sample bank
   *   - for use in VGM export
   * - 0xffffxx05: set sample position
   *   - xx is the instance ID
   *   - value is the sample position
   * - 0xffffffff: reset
   * - 0xfffffffe: add delay
   *   - value is the delay in cycles
   */
  unsigned int addr;
  // the value to write.
  unsigned int val;
  DivRegWrite():
    addr(0), val(0) {}
  DivRegWrite(unsigned int a, unsigned int v):
    addr(a), val(v) {}
};

/**
 * a delayed register write.
 */
struct DivDelayedWrite {
  // the write's delay.
  int time;
  // this variable is internal.
  // it is used by VGM export to make sure these writes are in order.
  // do not change.
  int order;
  // the register write.
  DivRegWrite write;
  // constructor with order.
  DivDelayedWrite(int t, int o, unsigned int a, unsigned int v):
    time(t),
    order(o),
    write(a,v) {}
  // constructor.
  DivDelayedWrite(int t, unsigned int a, unsigned int v):
    time(t),
    order(0),
    write(a,v) {}
};

/**
 * encapsulates a channel's sample position.
 * this is used by DivDispatch::getSamplePos().
 */
struct DivSamplePos {
  // sample: the sample index.
  // pos: the current position.
  // freq: the frequency in Hz.
  int sample, pos, freq;
  DivSamplePos(int s, int p, int f):
    sample(s),
    pos(p),
    freq(f) {}
  DivSamplePos():
    sample(-1),
    pos(0),
    freq(0) {}
};

// these are used to determine fractional precision of the chan osc buffer's needle.
// I was planning to give the needle more precision on 64-bit systems until
// I ran into all sorts of issues, so I decided to keep it at 16 for everyone.
constexpr size_t OSCBUF_PREC=(sizeof(size_t)>=8)?16:16;
constexpr size_t OSCBUF_MASK=(UINTMAX_C(1)<<OSCBUF_PREC)-1;

// don't use this unless you know what you're doing. stick to putSample().
#define putSampleIKnowWhatIAmDoing(_ob,_pos,_val) \
  _ob->data[_pos]=_val;

/**
 * this is a buffer for a channel's output.
 * it is used for the per-channel oscilloscope.
 * it should not be used for per-channel audio export as its output is optimized and may have a lower rate/quality.
 */
struct DivDispatchOscBuffer {
  // the input rate of this osc buffer.
  // the output buffer will be filled at a rate of 65536 samples per second no matter what.
  size_t rate;
  // used to calculate resampling rate for the output.
  size_t rateMul;
  // current position in the output buffer, as a fixed point number.
  // it is 16.16 for performance reasons.
  unsigned int needle;
  // the read position of the output buffer.
  // set by the GUI after rendering the per-chan osc.
  unsigned short readNeedle;
  // this was formerly used but now disabled as part of an optimization...
  //unsigned short lastSample;
  // follow: serves no purpose. formerly a debug option.
  // mustNotKillNeedle: set when the needle's fractional part is non-zero. moves start and end positions by one if they differ to prevent glitches.
  bool follow, mustNotKillNeedle;
  // the output data.
  // if a sample is -1, it means "hold the previous sample".
  // if you're wondering why, it's to speed up acquireDirect() by not having to fill in each sample.
  // actual -1 samples become -2 to avoid conflicts. see what I told you about optimization?
  short data[65536];

  /**
   * put a sample into the output buffer.
   * @param pos offset relative to the needle (should be the same as the position in acquire()).
   * @param val the input sample.
   */
  inline void putSample(const size_t pos, const short val) {
    unsigned short realPos=((needle+pos*rateMul)>>OSCBUF_PREC);
    if (val==-1) {
      data[realPos]=0xfffe;
      return;
    }
    //lastSample=val;
    data[realPos]=val;
  }
  // this was an inline function, but I decided to turn it into a macro for further optimization.
  // if you must know what this actually does and how to use it, go to platform/esfm.cpp.
  /*
  inline void putSampleIKnowWhatIAmDoing(const unsigned short pos, const short val) {
    //unsigned short realPos=((needle+pos*rateMul)>>OSCBUF_PREC);
    if (val==-1) {
      data[pos]=0xfffe;
      return;
    }
    //lastSample=val;
    data[pos]=val;
  }*/
  /**
   * begin processing of a new frame.
   * must be called at the beginning of acquire() before you start feeding samples.
   * @param len the frame's length (how many input samples to make room for).
   */
  inline void begin(size_t len) {
    size_t calc=(len*rateMul);
    unsigned short start=needle>>16;
    unsigned short end=(needle+calc)>>16;

    if (mustNotKillNeedle && start!=end) {
      start++;
      end++;
    }

    //logD("C %d %d %d",len,calc,rate);

    if (end<start) {
      //logE("ELS %d %d %d",end,start,calc);
      memset(&data[start],-1,(0x10000-start)*sizeof(short));
      memset(data,-1,end*sizeof(short));
      //data[needle>>16]=lastSample;
      return;
    }
    memset(&data[start],-1,(end-start)*sizeof(short));
    //data[needle>>16]=lastSample;
  }
  /**
   * finish processing of the current frame and advances the needle.
   * must be called at the end of acquire().
   * @param len the frame's length.
   */
  inline void end(size_t len) {
    size_t calc=len*rateMul;
    needle+=calc;
    mustNotKillNeedle=needle&0xffff;//(data[needle>>16]!=-1);
    //data[needle>>16]=lastSample;
  }
  /**
   * reset the buffer and state.
   */
  void reset() {
    memset(data,-1,65536*sizeof(short));
    needle=0;
    readNeedle=0;
    mustNotKillNeedle=false;
    //lastSample=0;
  }
  /**
   * set the input rate and recalculate internals.
   * @param r input rate.
   */
  void setRate(unsigned int r) {
    double rateMulD=65536.0/(double)r;
    rateMulD*=(double)(UINTMAX_C(1)<<OSCBUF_PREC);
    rate=r;
    rateMul=(size_t)rateMulD;
  }
  DivDispatchOscBuffer():
    rate(65536),
    rateMul(UINTMAX_C(1)<<OSCBUF_PREC),
    needle(0),
    readNeedle(0),
    //lastSample(0),
    follow(true),
    mustNotKillNeedle(false) {
    memset(data,-1,65536*sizeof(short));
  }
};

/**
 * a channel pair provides a hint to the GUI which informs the user which channels are paired with a channel for modulation
 * (e.g. ring/freq/phase mod, filtering and register sharing).
 */
struct DivChannelPair {
  // the label.
  const char* label;
  // the paired channels.
  // -1 means "none".
  signed char pairs[8];

  // constructor to pair up to 8 channels.
  DivChannelPair(const char* l, signed char p0, signed char p1, signed char p2, signed char p3, signed char p4, signed char p5, signed char p6, signed char p7):
    label(l),
    pairs{p0,p1,p2,p3,p4,p5,p6,p7} {}
  // constructor to pair a single channel.
  DivChannelPair(const char* l, signed char p):
    label(l),
    pairs{p,-1,-1,-1,-1,-1,-1,-1} {}
  // empty.
  DivChannelPair():
    label(NULL),
    pairs{-1,-1,-1,-1,-1,-1,-1,-1} {}
};

/**
 * mode hints provide the GUI with information about a channel's status.
 * this is presented to the user when the channel status option is enabled in the pattern view.
 */
struct DivChannelModeHints {
  // names of channel mode hints.
  const char* hint[4];
  // types of modes.
  // valid types:
  // - 0: disabled
  // - 1: volume
  // - 2: pitch
  // - 3: panning
  // - 4: chip primary
  // - 5: chip secondary
  // - 6: mixing
  // - 7: DSP
  // - 8: note
  // - 9: misc 1
  // - 10: misc 2
  // - 11: misc 3
  // - 12: attack
  // - 13: decay
  // - 14: sustain
  // - 15: release
  // - 16: dec linear
  // - 17: dec exp
  // - 18: inc linear
  // - 19: inc bent
  // - 20: direct
  // - 21: warning
  // - 22: error
  unsigned char type[4];
  // number of hints. up to 4.
  unsigned char count;

  DivChannelModeHints():
    hint{NULL,NULL,NULL,NULL},
    type{0,0,0,0},
    count(0) {}
};

/**
 * this enum describe possible types for memory entries in a memory composition struct.
 */
enum DivMemoryEntryType {
  DIV_MEMORY_FREE=0, // shouldn't be used
  DIV_MEMORY_PADDING,
  DIV_MEMORY_RESERVED,
  DIV_MEMORY_SAMPLE,
  DIV_MEMORY_SAMPLE_ALT1,
  DIV_MEMORY_SAMPLE_ALT2,
  DIV_MEMORY_SAMPLE_ALT3,
  DIV_MEMORY_WAVE_RAM,
  DIV_MEMORY_WAVE_STATIC,
  DIV_MEMORY_ECHO,
  DIV_MEMORY_N163_LOAD,
  DIV_MEMORY_N163_PLAY,
  DIV_MEMORY_BANK0,
  DIV_MEMORY_BANK1,
  DIV_MEMORY_BANK2,
  DIV_MEMORY_BANK3,
  DIV_MEMORY_BANK4,
  DIV_MEMORY_BANK5,
  DIV_MEMORY_BANK6,
  DIV_MEMORY_BANK7,
};

/**
 * a memory entry describes a region of data within a chip's memory.
 * this is contained by the memory composition struct.
 */
struct DivMemoryEntry {
  // the type of this entry.
  DivMemoryEntryType type;
  // name.
  String name;
  // related asset (usually a sample).
  // used by the GUI to let the user jump to a sample by clicking on the region.
  int asset;
  // the region within memory that this entry occupies.
  size_t begin, end;
  DivMemoryEntry(DivMemoryEntryType t, String n, int a, size_t b, size_t e):
    type(t),
    name(n),
    asset(a),
    begin(b),
    end(e) {}
  DivMemoryEntry():
    type(DIV_MEMORY_FREE),
    name(""),
    asset(-1),
    begin(0),
    end(0) {}
};

/**
 * possible waveform view formats.
 * used by the GUI to display live memory data over the memory composition graph.
 */
enum DivMemoryWaveView: unsigned char {
  DIV_MEMORY_WAVE_NONE=0,
  DIV_MEMORY_WAVE_4BIT, // Namco 163
  DIV_MEMORY_WAVE_6BIT, // Virtual Boy
  DIV_MEMORY_WAVE_8BIT_SIGNED, // SCC
};

/**
 * used to describe the contents of a chip's memory, such as samples, waveforms and other data.
 * returned by DivDispatch::getMemCompo().
 */
struct DivMemoryComposition {
  // contains memory entries.
  std::vector<DivMemoryEntry> entries;
  // name of this memory space.
  String name;
  // capacity of this memory space, in bytes.
  size_t capacity;
  // how much memory is in use.
  size_t used;
  // a pointer to memory contents. used if waveformView is not NONE.
  const unsigned char* memory;
  // this may be set to allow the GUI to display a visualization of memory data alongside the graph.
  DivMemoryWaveView waveformView;
  DivMemoryComposition():
    name(""),
    capacity(0),
    used(0),
    memory(NULL),
    waveformView(DIV_MEMORY_WAVE_NONE) {}
};

/**
 * a "dispatch" performs the following:
 * - processes engine commands
 * - runs macros (if necessary)
 * - performs register writes
 * - emulates a sound chip, synthesizes sound or otherwise provides an audible output to the engine
 * it is one of the vital components of Furnace. it is referred to as "chip" in the UI.
 * it gets its name from the fact this is where commands are dispatched to.
 * this is not called DivChip because not all dispatches are chips (despite the UI calling them chips). an example is the Generic PCM DAC.
 *
 * implementations lie in platform/ and are prefixed with DivPlatform*.
 * check out platform/pce.cpp and platform/pce.h. these are templates I use frequently when adding new chips.
 */
class DivDispatch {
  protected:
    // the parent engine attached to this dispatch.
    DivEngine* parent;
    // this contains all register writes made to a chip.
    // only populate this when dumpWrites is enabled!
    std::vector<DivRegWrite> regWrites;
    // skipRegisterWrites: set while the engine is "seeking" in the song. when set, you shouldn't write to registers.
    // dumpWrites: set when the engine wants to know what are we writing. used during register dump export (e.g. VGM).
    bool skipRegisterWrites, dumpWrites;
  public:
    /**
     * the rate the samples are provided.
     * the engine shall resample to the output rate.
     * you have to initialize this one during init() or setFlags().
     */
    int rate;
    
    /**
     * the actual chip's clock.
     * you have to initialize this one during init() or setFlags().
     */
    int chipClock;

    /**
     * fill a buffer with sound data.
     * @param buf pointers to output buffers.
     * @param len the amount of samples to fill.
     */
    virtual void acquire(short** buf, size_t len);

    /**
     * fill a buffer with sound data (direct access to blip_buf).
     * @param bb pointers to blip_buf instances.
     * @param len the amount of samples to fill.
     */
    virtual void acquireDirect(blip_buffer_t** bb, size_t len);

    /**
     * post-process a rendered sound buffer.
     * @param buf pointers to output buffers.
     * @param outIndex the output index.
     * @param len the number of samples in the buffer.
     * @param sampleRate the current audio output rate (usually 44100 or 48000).
     */
    virtual void postProcess(short* buf, int outIndex, size_t len, int sampleRate);

    /**
     * fill a write stream with data (e.g. for software-mixed PCM).
     * @param stream the write stream.
     * @param rate stream rate (e.g. 44100 for VGM).
     * @param len number of samples.
     */
    virtual void fillStream(std::vector<DivDelayedWrite>& stream, int sRate, size_t len);

    /**
     * send a command to this dispatch.
     * @param c a DivCommand.
     * @return a return value which varies depending on the command.
     */
    virtual int dispatch(DivCommand c);

    /**
     * reset the state of this dispatch.
     */
    virtual void reset();

    /**
     * ticks this dispatch.
     * @param sysTick whether the engine has ticked (if not then this may be a sub-tick used in low-latency mode).
     */
    virtual void tick(bool sysTick=true);

    /**
     * get the state of a channel.
     * @param chan the channel.
     * @return a pointer, or NULL.
     */
    virtual SharedChannel* getChanState(int chan);

    /**
     * get the DivMacroInt of a channel.
     * @param chan the channel.
     * @return a pointer, or NULL.
     */
    virtual DivMacroInt* getChanMacroInt(int chan);

    /**
     * get the stereo panning of a channel.
     * @param chan the channel.
     * @return a 16-bit number. left in top 8 bits and right in bottom 8 bits.
     */
    virtual unsigned short getPan(int chan);

    /**
     * get "paired" channels.
     * @param ch the channel to query.
     * @param ret the DivChannelPair vector of pairs.
     */
    virtual void getPaired(int ch, std::vector<DivChannelPair>& ret);

    /**
     * get channel mode hints.
     * @param chan the channel to query.
     * @return a DivChannelModeHints.
     */
    virtual DivChannelModeHints getModeHints(int chan);
    

    /**
     * get currently playing sample (and its position).
     * @param chan the channel.
     * @return a DivSamplePos. if sample is -1 then nothing is playing or the
     * channel doesn't play samples.
     */
    virtual DivSamplePos getSamplePos(int chan);

    /**
     * get an oscilloscope buffer for a channel.
     * @param chan the channel.
     * @return a pointer to a DivDispatchOscBuffer, or NULL if not supported.
     */
    virtual DivDispatchOscBuffer* getOscBuffer(int chan);
    
    /**
     * get the register pool of this dispatch.
     * @return a pointer, or NULL.
     */
    virtual unsigned char* getRegisterPool();

    /**
     * get the size of the register pool of this dispatch.
     * @return the size.
     */
    virtual int getRegisterPoolSize();

    /**
     * get the bit depth of the register pool of this dispatch.
     * If the result is 16, it should be casted to unsigned short.
     * @return the depth. Default value is 8
     */
    virtual int getRegisterPoolDepth();

    /**
     * get this dispatch's state. DO NOT IMPLEMENT YET.
     * @return a pointer to the dispatch's state. must be deallocated manually!
     */
    virtual void* getState();

    /**
     * set this dispatch's state. DO NOT IMPLEMENT YET.
     * @param state a pointer to a state pertaining to this dispatch,
     * or NULL if this dispatch does not support state saves.
     */
    virtual void setState(void* state);

    /**
     * mute a channel.
     * @param ch the channel to mute.
     * @param mute whether to mute or unmute.
     */
    virtual void muteChannel(int ch, bool mute);

    /**
     * get the number of outputs this dispatch provides.
     * @return number of outputs (usually 1 or 2 but may be more). SHALL NOT be less than one.
     */
    virtual int getOutputCount();

    /**
     * test whether sending a key off command to a channel should reset arp too.
     * @param ch the channel in question.
     * @return whether it does.
     */
    virtual bool keyOffAffectsArp(int ch);

    /**
     * test whether sending a key off command to a channel should reset slides too.
     * @param ch the channel in question.
     * @return whether it does.
     */
    virtual bool keyOffAffectsPorta(int ch);

    /**
     * test whether volume is global.
     * @return whether it is.
     */
    virtual bool isVolGlobal();

    /**
     * test whether a channel supports soft panning.
     * @return truth.
     */
    virtual bool hasSoftPan(int ch);

    /**
     * map MIDI velocity (from 0 to 127) to chip volume.
     * @param ch the chip channel. -1 means N/A.
     * @param vel input velocity, from 0.0 to 1.0.
     * @return output volume.
     */
    virtual int mapVelocity(int ch, float vel);

    /**
     * map chip volume to gain.
     * @param ch the chip channel. -1 means N/A.
     * @param vol input volume.
     * @return output gain fron 0.0 to 1.0.
     */
    virtual float getGain(int ch, int vol);

    /**
     * get the lowest note in a portamento.
     * @param ch the channel in question.
     * @return the lowest note.
     */
    virtual int getPortaFloor(int ch);

    /**
     * check whether to always set volume on volume change (even when same volume).
     * only for compatibility purposes!
     * @return truth.
     */
    virtual bool getLegacyAlwaysSetVolume();

    /**
     * get the required amplification level of this dispatch's output.
     * @return the amplification level.
     */
    virtual float getPostAmp();

    /**
     * check whether DC offset correction is required.
     * @return truth.
     */
    virtual bool getDCOffRequired();

    /**
     * check whether PRE_NOTE command is desired.
     * @return truth.
     */
    virtual bool getWantPreNote();

    /**
     * check whether acquireDirect is available.
     * @return whether it is.
     */
    virtual bool hasAcquireDirect();

    /**
     * get minimum chip clock.
     * @return clock in Hz, or 0 if custom clocks are not supported.
     */
    virtual int getClockRangeMin();

    /**
     * get maximum chip clock.
     * @return clock in Hz, or 0 if custom clocks are not supported.
     */
    virtual int getClockRangeMax();

    /**
     * set the chip flags.
     * @param flags a DivConfig containing chip flags.
     */
    virtual void setFlags(const DivConfig& flags);

    /**
     * set skip reg writes.
     */
    virtual void setSkipRegisterWrites(bool value);

    /**
     * notify instrument change.
     */
    virtual void notifyInsChange(int ins);

    /**
     * notify wavetable change.
     */
    virtual void notifyWaveChange(int wave);

    /**
     * notify sample change.
     */
    virtual void notifySampleChange(int sample);

    /**
     * notify addition of an instrument.
     */
    virtual void notifyInsAddition(int sysID);

    /**
     * notify deletion of an instrument.
     */
    virtual void notifyInsDeletion(void* ins);

    /**
     * notify that playback stopped.
     */
    virtual void notifyPlaybackStop();

    /**
     * force-retrigger instruments.
     */
    virtual void forceIns();

    /**
     * enable register dumping.
     */
    virtual void toggleRegisterDump(bool enable);

    /**
     * get register writes.
     */
    std::vector<DivRegWrite>& getRegisterWrites();

    /**
     * poke a register.
     * @param addr address.
     * @param val value.
     */
    virtual void poke(unsigned int addr, unsigned short val);

    /**
     * poke a register.
     * @param wlist a vector containing DivRegWrites.
     */
    virtual void poke(std::vector<DivRegWrite>& wlist);

    /**
     * get available registers.
     * @return an array of C strings, terminated by NULL; or NULL if none available.
     */
    virtual const char** getRegisterSheet();

    /**
     * Get sample memory buffer.
     * @param index the memory index.
     * @return a pointer to sample memory, or NULL.
     */
    virtual const void* getSampleMem(int index=0);

    /**
     * Get sample memory capacity.
     * @param index the memory index.
     * @return memory capacity in bytes, or 0 if memory doesn't exist.
     */
    virtual size_t getSampleMemCapacity(int index=0);

    /**
     * get sample memory name.
     * @param index the memory index.
     * @return a name, or NULL if it doesn't have any name in particular.
     */
    virtual const char* getSampleMemName(int index=0);

    /**
     * Get sample memory start offset.
     * @param index the memory index.
     * @return memory start offset in bytes.
     */
    virtual size_t getSampleMemOffset(int index=0);

    /**
     * Get sample memory usage.
     * @param index the memory index.
     * @return memory usage in bytes.
     */
    virtual size_t getSampleMemUsage(int index=0);

    /**
     * check whether chip has sample pointer header in sample memory.
     * @param index the memory index.
     * @return whether it did.
     */
    virtual bool hasSamplePtrHeader(int index=0);

    /**
     * check whether sample has been loaded in memory.
     * @param index index.
     * @param sample the sample in question.
     * @return whether it did.
     */
    virtual bool isSampleLoaded(int index, int sample);
    
    /**
     * get memory composition.
     * @param index the memory index.
     * @return a pointer to DivMemoryComposition, or NULL.
     */
    virtual const DivMemoryComposition* getMemCompo(int index);

    /**
     * get a "compiled" version of sample memory.
     * this may be the same as getSampleMem() or not (may include extra data such as sample offsets).
     * used in ROM export.
     * @param index the memory index.
     * @param size the memory size will be stored here.
     * @return a pointer to compiled sample memory which must be deallocated after use (delete[]), or NULL if not implemented.
     */
    virtual const void* compileSampleMem(int index, size_t& size);

    /**
     * Render samples into sample memory.
     * @param sysID the chip's index in the chip list.
     */
    virtual void renderSamples(int sysID);

    /**
     * tell this DivDispatch that the tuning, pitch linearity or rate of a sample has changed, and therefore the pitch table must be regenerated.
     * @param sample the sample index if it's a rate change. this can be used to regenerate the table of a single sample. set to -1 when the tuning/pitch linearity changes and a full recalculation must take place.
     */
    virtual void notifyPitchTable(int sample=-1);

    /**
     * initialize this DivDispatch.
     * @param parent the parent DivEngine.
     * @param channels the number of channels to acquire.
     * @param sugRate the suggested rate. this may change, so don't rely on it.
     * @param flags a DivConfig containing chip flags.
     * @return the number of channels allocated.
     */
    virtual int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);

    /**
     * quit the DivDispatch.
     */
    virtual void quit();

    virtual ~DivDispatch();
};

// custom chip clock helper define. put in setFlags, but before rate is set.
#define CHECK_CUSTOM_CLOCK \
  if (flags.getInt("customClock",0)>0) { \
    chipClock=flags.getInt("customClock",getClockRangeMin()); \
    if (chipClock>getClockRangeMax()) chipClock=getClockRangeMax(); \
    if (chipClock<getClockRangeMin()) chipClock=getClockRangeMin(); \
  }

// NOTE: these definitions may be deprecated in the future. see DivPitchTable.
// pitch calculation:
// - a DivDispatch usually contains four variables per channel:
//   - baseFreq: this changes on new notes, legato, arpeggio and slides.
//   - pitch: this changes with DIV_CMD_PITCH (E5xx/04xy).
//   - freq: this is the result of combining baseFreq and pitch using DivEngine::calcFreq().
//   - freqChanged: whether baseFreq and/or pitch have changed, and a frequency recalculation is required on the next tick.
// - the following definitions will help you calculate baseFreq.
// - to use them, define CHIP_DIVIDER and/or CHIP_FREQBASE in your code (not in the header though!).
//   the value depends on the chip.
#define NOTE_PERIODIC(x) round(parent->calcBaseFreq(chipClock,CHIP_DIVIDER,x,true))
#define NOTE_PERIODIC_NOROUND(x) parent->calcBaseFreq(chipClock,CHIP_DIVIDER,x,true)
#define NOTE_FREQUENCY(x) parent->calcBaseFreq(chipClock,CHIP_FREQBASE,x,false)

// this is a special case definition. only use it for f-num/block-based chips.
#define NOTE_FNUM_BLOCK(x,bits,blk) parent->calcBaseFreqFNumBlock(chipClock,CHIP_FREQBASE,x,bits,blk)

// this is for volume scaling calculation.
#define VOL_SCALE_LINEAR(x,y,range) ((parent->song.compatFlags.ceilVolumeScaling)?((((x)*(y))+(range-1))/(range)):(((x)*(y))/(range)))
#define VOL_SCALE_LOG(x,y,range) (CLAMP(((x)+(y))-(range),0,(range)))
#define VOL_SCALE_LINEAR_BROKEN(x,y,range) ((parent->song.compatFlags.newVolumeScaling)?(VOL_SCALE_LINEAR(x,y,range)):(VOL_SCALE_LOG(x,y,range)))
#define VOL_SCALE_LOG_BROKEN(x,y,range) ((parent->song.compatFlags.newVolumeScaling)?(VOL_SCALE_LOG(x,y,range)):(VOL_SCALE_LINEAR(x,y,range)))

// these are here for convenience.
// it is encouraged to use these, since you get an exact value this way.
// - NTSC colorburst: 3.58MHz
// - PAL colorburst: 4.43MHz
#define COLOR_NTSC (315000000.0/88.0)
#define COLOR_PAL (283.75*15625.0+25.0)

// this macro clamps a variable.
#define CLAMP_VAR(x,xMin,xMax) \
  if ((x)<(xMin)) (x)=(xMin); \
  if ((x)>(xMax)) (x)=(xMax);

// used to determine whether we can use handleArp() to handle the arpeggio macro.
// otherwise, baseFreq must be altered on each arp macro tick.
#define NEW_ARP_STRAT (parent->song.compatFlags.linearPitch && !parent->song.compatFlags.oldArpStrategy)

// this is used by DIV_CMD_LEGATO handling code in some dispatches for compatibility.
#define HACKY_LEGATO_MESS chan[c.chan].std.arp.will && !chan[c.chan].std.arp.mode && !NEW_ARP_STRAT

#endif
