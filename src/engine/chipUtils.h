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

// chipUtils.h: definition of SharedChannel.
// I should merge this with dispatch.h.

#ifndef _CHIP_UTILS_H
#define _CHIP_UTILS_H

#include "defines.h"
#include "macroInt.h"

// custom clock limits
// these values are intentional. they prevent the user from overloading the engine by using extraneous clocks.
#define MIN_CUSTOM_CLOCK 100000
#define MAX_CUSTOM_CLOCK 40000000

/**
 * the SharedChannel struct holds common channel state, such as frequency, volume, note activity and so on.
 * this is used by almost every dispatch.
 *
 * create a struct inherited from SharedChannel in your dispatch's class definition:
 *
 * struct Channel: public SharedChannel<int> {
 *   // state...
 * };
 */
template<typename T> struct SharedChannel {
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
  // note: current note, in semitones. 0 is C-0.
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
  bool active, insChanged, freqChanged, fixedArp, keyOn, keyOff, portaPause, inPorta;
  // vol: the current volume, set during DIV_CMD_VOLUME.
  // outVol: the *output* volume.
  // - this is the same as vol when we don't have a volume macro going on.
  // - otherwise it is the result of a calculation with vol and the volume macro's value.
  //   - calculate this value by using VOL_SCALE_LINEAR()/VOL_SCALE_LOG() in tick().
  // the type of these two is usually int, but some chips use signed char.
  T vol, outVol;
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
          baseNoteOverride=(std.arp.val|0x40000000)+offset;
          fixedArp=true;
        } else {
          arpOff=std.arp.val;
          fixedArp=false;
        }
      } else {
        if (std.arp.val&0x40000000) {
          baseNoteOverride=(std.arp.val&(~0x40000000))+offset;
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
   * call this constructor in your Channel's constructor, which should initialize the channel's state.
   * call your Channel's constructor during reset().
   *
   * @param initVol the initial channel volume.
   */
  SharedChannel(T initVol):
    freq(0),
    baseFreq(0),
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
    vol(initVol),
    outVol(initVol),
    std() {} 
};

#endif
