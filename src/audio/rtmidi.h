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

#include "../../extern/rtmidi/RtMidi.h"
#include "taAudio.h"

class TAMidiInRtMidi: public TAMidiIn {
  RtMidiIn* port;
  bool isOpen;
  public:
    bool gather();
    bool isDeviceOpen();
    bool openDevice(String name);
    bool closeDevice();
    std::vector<String> listDevices();
    bool quit();
    bool init();
    TAMidiInRtMidi():
      port(NULL),
      isOpen(false) {}
};

class TAMidiOutRtMidi: public TAMidiOut {
  RtMidiOut* port;
  bool isOpen, isWorking;
  public:
    bool send(const TAMidiMessage& what);
    bool isDeviceOpen();
    bool openDevice(String name);
    bool closeDevice();
    std::vector<String> listDevices();
    bool quit();
    bool init();
    TAMidiOutRtMidi():
      port(NULL),
      isOpen(false),
      isWorking(false) {}
};