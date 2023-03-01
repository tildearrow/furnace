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

#include "zsm.h"
#include "../ta-log.h"
#include "../utfutils.h"
#include "song.h"

DivZSM::DivZSM() {
  w = NULL;
  init();
}

DivZSM::~DivZSM() {
}

void DivZSM::init(unsigned int rate) {
  if (w != NULL) delete w;
  w = new SafeWriter;
  w->init();
  // write default ZSM data header
  w->write("zm",2); // magic header
  w->writeC(ZSM_VERSION);
  // no loop offset
  w->writeS(0);
  w->writeC(0);
  // no PCM
  w->writeS(0x00);
  w->writeC(0x00);
  // FM channel mask
  w->writeC(0x00);
  // PSG channel mask
  w->writeS(0x00);
  w->writeS((unsigned short)rate);
  // 2 reserved bytes (set to zero)
  w->writeS(0x00);
  tickRate = rate;
  loopOffset=-1;
  numWrites=0;
  memset(&ymState,-1,sizeof(ymState));
  memset(&psgState,-1,sizeof(psgState));
  ticks=0;
}

int DivZSM::getoffset() {
  return w->tell();
}

void DivZSM::writeYM(unsigned char a, unsigned char v) {
  int lastMask = ymMask;
  if (a==0x19 && v>=0x80) a=0x1a; // AMD/PSD use same reg addr. store PMD as 0x1a
  if (a==0x08 && (v&0xf8)) ymMask |= (1 << (v & 0x07)); // mark chan as in-use if keyDN
  if (a!=0x08) ymState[ym_NEW][a] = v; // cache the newly-written value
  bool writeit=false; // used to suppress spurious writes to unused channels
  if (a < 0x20) {
    if (a == 0x08) {
      // write keyUPDN messages if channel is active.
      writeit = (ymMask & (1 << (v & 0x07))) > 0;
    }
    else {
      // do not suppress global registers
      writeit = true;
    }
  } else {
    writeit = (ymMask & (1 << (a & 0x07))) > 0; // a&0x07 = chan ID for regs >=0x20
  }
  if (lastMask != ymMask) {
    // if the ymMask just changed, then the channel has become active.
    // This can only happen on a KeyDN event, so voice = v & 0x07
    // insert a keyUP just to be safe.
    ymwrites.push_back(DivRegWrite(0x08,v&0x07));
    numWrites++;
    // flush the ym_NEW cached states for this channel into the ZSM....
    for ( int i=0x20 + (v&0x07); i <= 0xff ; i+=8) {
      if (ymState[ym_NEW][i] != ymState[ym_PREV][i]) {
        ymwrites.push_back(DivRegWrite(i,ymState[ym_NEW][i]));
        numWrites++;
        // ...and update the shadow
        ymState[ym_PREV][i] = ymState[ym_NEW][i];
      }
    }
  }
  // Handle the current write if channel is active
  if (writeit && ((ymState[ym_NEW][a] != ymState[ym_PREV][a])||a==0x08) ) {
    // update YM shadow if not the KeyUPDN register.
    if (a!=0x008) ymState[ym_PREV][a] = ymState[ym_NEW][a];
    // if reg = PMD, then change back to real register 0x19
    if (a==0x1a) a=0x19;
    ymwrites.push_back(DivRegWrite(a,v));
    numWrites++;
  }
}

void DivZSM::writePSG(unsigned char a, unsigned char v) {
  // TODO: suppress writes to PSG voice that is not audible (volume=0)
  if (a  >= 64) {
    logD ("ZSM: ignoring VERA PSG write a=%02x v=%02x",a,v);
    return;
  }
  if(psgState[psg_PREV][a] == v) {
    if (psgState[psg_NEW][a] != v)
      // NEW value is being reset to the same as PREV value
      // so it is no longer a new write.
      numWrites--;
  } else {
    if (psgState[psg_PREV][a] == psgState[psg_NEW][a])
      // if this write changes the NEW cached value to something other
      // than the PREV value, then this is a new write.
      numWrites++;
  }
  psgState[psg_NEW][a] = v;
  // mark channel as used in the psgMask if volume is set > 0.
  if ((a % 4 == 2) && (v & 0x3f)) psgMask |= (1 << (a>>2));
}

void DivZSM::writePCM(unsigned char a, unsigned char v) {
  // ZSM standard for PCM playback has not been established yet.
}

void DivZSM::tick(int numticks) {
  flushWrites();
  ticks += numticks;
}

void DivZSM::setLoopPoint() {
  tick(0); // flush any ticks+writes
  flushTicks(); // flush ticks incase no writes were pending
  logI("ZSM: loop at file offset %d bytes",w->tell());
  loopOffset=w->tell();
  // update the ZSM header's loop offset value
  w->seek(0x03,SEEK_SET);
  w->writeS((short)(loopOffset&0xffff));
  w->writeC((unsigned char)((loopOffset>>16)&0xff));
  w->seek(loopOffset,SEEK_SET);
  // reset the PSG shadow and write cache
  memset(&psgState,-1,sizeof(psgState));
  // reset the YM shadow....
  memset(&ymState[ym_PREV],-1,sizeof(ymState[ym_PREV]));
  // ... and cache (except for unused channels)
  memset(&ymState[ym_NEW],-1,0x20);
  for (int chan=0; chan<8 ; chan++) {
    // do not clear state for as-yet-unused channels
    if (!(ymMask & (1<<chan))) continue;
    // clear the state for channels in use so they match the unknown state
    // of the YM shadow.
    for (int i=0x20+chan; i<=0xff; i+= 8) ymState[ym_NEW][i] = -1;
  }
}

SafeWriter* DivZSM::finish() {
  tick(0); // flush any pending writes / ticks
  flushTicks(); // flush ticks in case there were no writes pending
  w->writeC(ZSM_EOF);
  // update channel use masks.
  w->seek(0x09,SEEK_SET);
  w->writeC((unsigned char)(ymMask & 0xff));
  w->writeS((short)(psgMask & 0xffff));
  // todo: put PCM offset/data writes here once defined in ZSM standard.
  return w;
}

void DivZSM::flushWrites() {
  logD("ZSM: flushWrites.... numwrites=%d ticks=%d ymwrites=%d",numWrites,ticks,ymwrites.size());
  if (numWrites==0) return;
  flushTicks(); // only flush ticks if there are writes pending.
  for (unsigned char i=0;i<64;i++) {
    if (psgState[psg_NEW][i] == psgState[psg_PREV][i]) continue;
    psgState[psg_PREV][i]=psgState[psg_NEW][i];
    w->writeC(i);
    w->writeC(psgState[psg_NEW][i]);
  }
  int n=0; // n = completed YM writes. used to determine when to write the CMD byte...
  for (DivRegWrite& write: ymwrites) {
    if (n%ZSM_YM_MAX_WRITES == 0) {
      if(ymwrites.size()-n > ZSM_YM_MAX_WRITES) {
        w->writeC((unsigned char)(ZSM_YM_CMD+ZSM_YM_MAX_WRITES));
        logD("ZSM: YM-write: %d (%02x) [max]",ZSM_YM_MAX_WRITES,ZSM_YM_MAX_WRITES+ZSM_YM_CMD);
      } else {
        w->writeC((unsigned char)(ZSM_YM_CMD+ymwrites.size()-n));
        logD("ZSM: YM-write: %d (%02x)",ymwrites.size()-n,ZSM_YM_CMD+ymwrites.size()-n);
      }
    }
    n++;
    w->writeC(write.addr);
    w->writeC(write.val);
  }
  ymwrites.clear();
  numWrites=0;
}

void DivZSM::flushTicks() {
  while (ticks > ZSM_DELAY_MAX) {
    logD("ZSM: write delay %d (max)",ZSM_DELAY_MAX);
    w->writeC((unsigned char)(ZSM_DELAY_CMD+ZSM_DELAY_MAX));
    ticks -= ZSM_DELAY_MAX;
  }
  if (ticks>0) {
    logD("ZSM: write delay %d",ticks);
    w->writeC(ZSM_DELAY_CMD+ticks);
  }
  ticks=0;
}
