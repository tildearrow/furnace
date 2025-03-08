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

#include "taAudio.h"
#ifdef HAVE_RTMIDI
#include "rtmidi.h"
#endif

bool TAAudio::initMidi(bool jack) {
#ifndef HAVE_RTMIDI
  return false;
#else
  midiIn=new TAMidiInRtMidi;
  midiOut=new TAMidiOutRtMidi;

  if (!midiIn->init()) {
    delete midiIn;
    midiIn=NULL;
    return false;
  }

  if (!midiOut->init()) {
    midiIn->quit();
    delete midiOut;
    delete midiIn;
    midiOut=NULL;
    midiIn=NULL;
    return false;
  }
  return true;
#endif
}

void TAAudio::quitMidi() {
  if (midiIn!=NULL) {
    midiIn->quit();
    delete midiIn;
    midiIn=NULL;
  }
  if (midiOut!=NULL) {
    midiOut->quit();
    delete midiOut;
    midiOut=NULL;
  }
}