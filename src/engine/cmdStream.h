/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#ifndef _CMD_STREAM_H
#define _CMD_STREAM_H

#include "defines.h"
#include "safeReader.h"

class DivEngine;

struct DivCSChannelState {
  unsigned int readPos;
  int waitTicks;

  int note, pitch;
  int volume, volMax, volSpeed;
  int vibratoDepth, vibratoRate, vibratoPos;
  int portaTarget, portaSpeed;
  unsigned char arp, arpStage, arpTicks;

  unsigned int callStack[8];
  unsigned char callStackPos;

  struct TraceEntry {
    unsigned int addr;
    unsigned char length;
    unsigned char data[11];
  } trace[32];
  unsigned char tracePos;

  bool doCall(unsigned int addr);

  DivCSChannelState():
    readPos(0),
    waitTicks(0),
    note(-1),
    pitch(0),
    volume(0x7f00),
    volMax(0),
    volSpeed(0),
    vibratoDepth(0),
    vibratoRate(0),
    vibratoPos(0),
    portaTarget(0),
    portaSpeed(0),
    arp(0),
    arpStage(0),
    arpTicks(0),
    callStackPos(0) {}
};

class DivCSPlayer {
  DivEngine* e;
  unsigned char* b;
  SafeReader stream;
  DivCSChannelState chan[DIV_MAX_CHANS];
  unsigned char fastDelays[16];
  unsigned char fastCmds[16];
  unsigned char arpSpeed;

  short vibTable[64];
  public:
    void cleanup();
    bool tick();
    bool init();
    DivCSPlayer(DivEngine* en, unsigned char* buf, size_t len):
      e(en),
      b(buf),
      stream(buf,len) {}
};

#endif
