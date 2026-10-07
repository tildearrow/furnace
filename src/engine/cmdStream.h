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

// cmdStream.h: reference command stream player.

#ifndef _CMD_STREAM_H
#define _CMD_STREAM_H

#include "defines.h"
#include "safeReader.h"

// size of the trace log (per channel)
#define DIV_MAX_CSTRACE 64
// maximum stack size
#define DIV_MAX_CSSTACK 128

class DivEngine;

/**
 * state for a channel in the command stream player.
 * this sort of resembles DivChannelState but is minimal.
 */
struct DivCSChannelState {
  // initial program counter address.
  unsigned int startPos;
  // the current "program counter" (position within the command stream) address.
  unsigned int readPos;
  // number of ticks to wait before processing. set by the wait command.
  int waitTicks;
  // length of the last wait command.
  int lastWaitLen;

  // I don't think I have to explain these.
  int note, pitch;
  int volume, volMax, volSpeed, volSpeedTarget;
  int vibratoDepth, vibratoRate, vibratoPos, vibratoRange, vibratoShape;
  int tremoloDepth, tremoloRate, tremoloPos;
  int panbrelloDepth, panbrelloRate, panbrelloPos;
  int portaTarget, portaSpeed;
  unsigned char arp, arpStage, arpTicks;
  unsigned char panL, panR;
  signed char panSpeed;

  // this is the call stack.
  // it contains addresses of instructions before a call.
  unsigned int callStack[DIV_MAX_CSSTACK];
  unsigned char callStackPos, callStackSize;

  // a ring buffer for trace.
  unsigned int trace[DIV_MAX_CSTRACE];
  unsigned char tracePos;

  /*
   * process a call instruction.
   * internal - do not call directly!
   * @param addr call address.
   * @return whether it was successful.
   */
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
    vibratoRange(15),
    vibratoShape(0),
    tremoloDepth(0),
    tremoloRate(0),
    tremoloPos(0),
    panbrelloDepth(0),
    panbrelloRate(0),
    panbrelloPos(0),
    portaTarget(0),
    portaSpeed(0),
    arp(0),
    arpStage(0),
    arpTicks(0),
    panL(255),
    panR(255),
    panSpeed(0),
    callStackPos(0),
    callStackSize(0),
    tracePos(0) {
    for (int i=0; i<DIV_MAX_CSTRACE; i++) {
      trace[i]=0;
    }
  }
};

/**
 * the reference command stream player.
 */
class DivCSPlayer {
  // the DivEngine associated with this player.
  DivEngine* e;
  // command stream data.
  unsigned char* b;
  // an array containing last access times (useful for a visualizer).
  unsigned short* bAccessTS;
  // size of data.
  size_t bLen;
  // this SafeReader wraps b and bLen.
  SafeReader stream;
  // channel state.
  DivCSChannelState chan[DIV_MAX_CHANS];
  // preset delays.
  unsigned char fastDelays[16];
  // speed dial instruments, volumes and commands.
  unsigned char fastIns[6];
  unsigned char fastVols[6];
  unsigned char fastCmds[4];
  // arp speed.
  unsigned char arpSpeed;
  // number of channels in the stream.
  unsigned int fileChans;
  // curTick: tick counter.
  // fastDelaysOff: offset in stream to preset delays.
  // fastInsOff: offset in stream to speed dial instruments.
  // fastVolsOff: offset in stream to speed dial volumes.
  // fastCmdsOff: offset in stream to speed dial commands.
  // deltaCyclePos: this is used to periodically refresh bAccessTS in order to prevent spurious triggers.
  unsigned int curTick, fastDelaysOff, fastInsOff, fastVolsOff, fastCmdsOff, deltaCyclePos;
  // whether the stream uses long (32-bit) pointers.
  bool longPointers;
  // whether the stream is big-endian.
  bool bigEndian;

  // vibrato table (taken from engine.h).
  short vibTable[64];
  // tremolo table (taken frmo engine.h).
  short tremTable[128];
  public:
    /**
     * get a pointer to the stream.
     * @return pointer to stream.
     */
    unsigned char* getData();
    /*
     * get a pointer to an array which contains stream access times.
     * @return pointer to stream access times.
     */
    unsigned short* getDataAccess();
    /*
     * get the stream's size.
     * @return stream size.
     */
    size_t getDataLen();
    /*
     * get channel state.
     * @param ch the channel.
     * @return a DivCSChannelState.
     */
    DivCSChannelState* getChanState(int ch);
    /*
     * get the number of channels in the stream.
     * @return number of channels.
     */
    unsigned int getFileChans();
    /*
     * get a pointer to preset delays.
     * @return guess.
     */
    unsigned char* getFastDelays();
    /*
     * get a pointer to speed dial instruments.
     * @return guess.
     */
    unsigned char* getFastIns();
    /*
     * get a pointer to speed dial volumes.
     * @return guess.
     */
    unsigned char* getFastVols();
    /*
     * get a pointer to speed dial commands.
     * @return guess.
     */
    unsigned char* getFastCmds();
    /*
     * get the tick counter's value.
     * @return ...
     */
    unsigned int getCurTick();
    /*
     * kill the current stream.
     * this will also delete it from memory. beware!
     */
    void cleanup();
    /*
     * do a tick.
     * @return whether a tick actually happened.
     */
    bool tick();
    /*
     * initialize this command stream player.
     * @return whether successful.
     */
    bool init();
    /*
     * initialize a DivCSPlayer by passing an engine, a pointer to the stream and its length.
     * @param en a DivEngine.
     * @param buf pointer to command stream. note that this DivCSPlayer will own it, so don't use it after calling cleanup()!
     * @param len the command stream's size.
     */
    DivCSPlayer(DivEngine* en, unsigned char* buf, size_t len):
      e(en),
      b(buf),
      bAccessTS(NULL),
      bLen(len),
      stream(buf,len) {}
};

/**
 * this struct defines a progress indicator for command stream export.
 */
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

/**
 * options for command stream export.
 */
struct DivCSOptions {
  // use 32-bit pointers (instead of 16-bit ones)
  bool longPointers;
  // use big-endian mode.
  bool bigEndian;
  // disable command call optimization (speed dial)
  bool noCmdCallOpt;
  // disable delay condensation (always use one-tick delays)
  bool noDelayCondense;
  // disable sub-block search
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
  /**
   * get the length of a command.
   * @param ext the command. see DivCommand enum.
   * @return length in bytes.
   */
  int getCmdLength(unsigned char ext);
  /**
   * get the length of an instruction.
   * @param ins the instruction.
   * @param ext command, if the instruction is "full command".
   * @param speedDial pointer to speed dial commands.
   * @return length in bytes.
   */
  int getInsLength(unsigned char ins, unsigned char ext=0, unsigned char* speedDial=NULL);
};

#endif
