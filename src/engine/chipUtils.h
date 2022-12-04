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

#ifndef _CHIP_UTILS_H
#define _CHIP_UTILS_H

// custom clock limits
#define MIN_CUSTOM_CLOCK 100000
#define MAX_CUSTOM_CLOCK 40000000

// common shared channel struct
struct SharedChannel {
  int ins;
  int note;
  bool active, insChanged, keyOn, keyOff;
  SharedChannel():
    ins(-1),
    note(0),
    active(false),
    insChanged(true),
    keyOn(false),
    keyOff(false) {} 
};

// common shared channel struct with frequency
struct SharedChannelFreq: public SharedChannel {
  int freq, baseFreq, pitch, pitch2;
  bool freqChanged, inPorta, portaPause;
  SharedChannelFreq():
    SharedChannel(),
    freq(0),
    baseFreq(0),
    pitch(0),
    pitch2(0),
    freqChanged(false),
    inPorta(false),
    portaPause(false) {}
};

// common shared channel volume struct
template<typename T>
struct SharedChannelVolume {
  bool volumeChanged;
  T vol, outVol, resVol;
  SharedChannelVolume(T initVol):
    vol(initVol),
    outVol(initVol),
    resVol(initVol) {}
};

#endif
