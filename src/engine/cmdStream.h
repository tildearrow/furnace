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

#ifndef _CMD_STREAM_H
#define _CMD_STREAM_H

#include "defines.h"
#include "safeReader.h"

#define DIV_MAX_CSTRACE 64
#define DIV_MAX_CSSTACK 128

class DivEngine;

struct DivCSChannelState {
  unsigned int startPos;
  unsigned int readPos;
  int waitTicks;
  int lastWaitLen;

  int note, pitch;
  int volume, volMax, volSpeed, volSpeedTarget;
  int vibratoDepth, vibratoRate, vibratoPos, vibratoRange, vibratoShape;
  int portaTarget, portaSpeed;
  unsigned char arp, arpStage, arpTicks;

  unsigned int callStack[DIV_MAX_CSSTACK];
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
  unsigned short* bAccessTS;
  size_t bLen;
  SafeReader stream;
  DivCSChannelState chan[DIV_MAX_CHANS];
  unsigned char fastDelays[16];
  unsigned char fastCmds[16];
  unsigned char arpSpeed;
  unsigned int fileChans;
  unsigned int curTick, fastDelaysOff, fastCmdsOff, deltaCyclePos;
  bool longPointers;
  bool bigEndian;

  short vibTable[64];
  public:
    unsigned char* getData();
    unsigned short* getDataAccess();
    size_t getDataLen();
    DivCSChannelState* getChanState(int ch);
    unsigned int getFileChans();
    unsigned char* getFastDelays();
    unsigned char* getFastCmds();
    unsigned int getCurTick();
    void cleanup();
    bool tick();
    bool init();
    DivCSPlayer(DivEngine* en, unsigned char* buf, size_t len):
      e(en),
      b(buf),
      bAccessTS(NULL),
      bLen(len),
      stream(buf,len) {}
};

struct DivCSProgress {
  int stage, count, total;
  int optStage, findTotal;
  int optCurrent, optTotal;
  int findCurrent, expandCurrent;
  int origCurrent, origCount;
  DivCSProgress():
    stage(0),
    count(0),
    total(0),
    optStage(0),
    findTotal(0),
    optCurrent(0),
    optTotal(0),
    findCurrent(0),
    expandCurrent(0),
    origCurrent(0),
    origCount(0) {}
};

struct DivCSOptions {
  bool longPointers;
  bool bigEndian;
  bool noCmdCallOpt;
  bool noDelayCondense;
  bool noSubBlock;

  DivCSOptions():
    longPointers(false),
    bigEndian(false),
    noCmdCallOpt(false),
    noDelayCondense(false),
    noSubBlock(false) {}
};

// command stream utilities
namespace DivCS {
  int getCmdLength(unsigned char ext);
  int getInsLength(unsigned char ins, unsigned char ext=0, unsigned char* speedDial=NULL);
};

#endif
