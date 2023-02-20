/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

#ifndef _ZSM_H
#define _ZSM_H

//#include "engine.h"
#include "safeWriter.h"
#include "dispatch.h"
#include <stdlib.h>

#define ZSM_HEADER_SIZE 16
#define ZSM_VERSION 1
#define ZSM_YM_CMD 0x40
#define ZSM_DELAY_CMD 0x80
#define ZSM_YM_MAX_WRITES 63
#define ZSM_DELAY_MAX 127
#define ZSM_EOF ZSM_DELAY_CMD

enum YM_STATE { ym_PREV, ym_NEW, ym_STATES };
enum PSG_STATE { psg_PREV, psg_NEW, psg_STATES };

class DivZSM {
  private:
    SafeWriter* w;
    int ymState[ym_STATES][256];
    int psgState[psg_STATES][64];
    std::vector<DivRegWrite> ymwrites;
    int loopOffset;
    int numWrites;
    int ticks;
    int tickRate;
  int ymMask = 0;
  int psgMask = 0;
  public:
    DivZSM();
    ~DivZSM();
    void init(unsigned int rate = 60);
    int getoffset();
    void writeYM(unsigned char a, unsigned char v);
    void writePSG(unsigned char a, unsigned char v);
    void writePCM(unsigned char a, unsigned char v);
    void tick(int numticks = 1);
    void setLoopPoint();
    SafeWriter* finish();
  private:
    void flushWrites();
    void flushTicks();
};

#endif
