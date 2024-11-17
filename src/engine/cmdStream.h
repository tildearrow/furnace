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

#define DIV_MAX_CSTRACE 64

class DivEngine;

struct DivCSChannelState {
  unsigned int startPos;
  unsigned int readPos;
  int waitTicks;
  int lastWaitLen;

  int note, pitch;
  int volume, volMax, volSpeed, volSpeedTarget;
  int vibratoDepth, vibratoRate, vibratoPos;
  int portaTarget, portaSpeed;
  unsigned char arp, arpStage, arpTicks, arpLen;

  unsigned int callStack[8];
  unsigned char callStackPos;

  unsigned int trace[DIV_MAX_CSTRACE];
  unsigned char tracePos;

  bool doCall(unsigned int addr);

  DivCSChannelState():
    readPos(0),
    waitTicks(0),
    lastWaitLen(0),
    note(-1),
    pitch(0),
    volume(0x7f00),
    volMax(0),
    volSpeed(0),
    volSpeedTarget(-1),
    vibratoDepth(0),
    vibratoRate(0),
    vibratoPos(0),
    portaTarget(0),
    portaSpeed(0),
    arp(0),
    arpStage(0),
    arpTicks(0),
    arpLen(1),
    callStackPos(0),
    tracePos(0) {
    for (int i=0; i<DIV_MAX_CSTRACE; i++) {
      trace[i]=0;
    }
  }
};

class DivCSPlayer {
  DivEngine* e;
  unsigned char* b;
  size_t bLen;
  SafeReader stream;
  DivCSChannelState chan[DIV_MAX_CHANS];
  unsigned char fastDelays[16];
  unsigned char fastCmds[16];
  unsigned char arpSpeed;

  short vibTable[64];
  public:
    unsigned char* getData();
    size_t getDataLen();
    DivCSChannelState* getChanState(int ch);
    unsigned char* getFastDelays();
    unsigned char* getFastCmds();
    void cleanup();
    bool tick();
    bool init();
    DivCSPlayer(DivEngine* en, unsigned char* buf, size_t len):
      e(en),
      b(buf),
      bLen(len),
      stream(buf,len) {}
};

#endif
