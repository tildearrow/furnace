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

#include "engine.h"
#include "../ta-log.h"
#include "../utfutils.h"
#include "song.h"
#include "platform/nds.h"

SafeWriter* w;
unsigned int offset = 0;

static void writeByte(unsigned char byte) {
  if ((offset&31) == 0) {
    w->writeText("\n  "); 
  }
  w->writeText(fmt::format("0x{:02x}, ",byte)); 
  offset++;
}

SafeWriter* DivEngine::saveNDS(unsigned int refreshrate, bool loop) {
  int NDS=-1;
  int IGNORED=0;

  // find indexes for YM and VERA. Ignore other systems.
  for (int i=0; i<song.systemLen; i++) {
    switch (song.system[i]) {
      case DIV_SYSTEM_NDS:
        if (NDS>=0) {
          IGNORED++;
          break;
        }
        NDS=i;
        logD("NDS detected as chip id %d",i);
        break;
      default:
        IGNORED++;
        logD("Ignoring chip %d systemID %d",i,(int)song.system[i]);
        break;
    }
  }
  if (NDS<0) {
    logE("NDS not found");
    return NULL;
  }
  if (IGNORED>0) {
    logW("furDS export ignoring %d system%c",IGNORED,IGNORED>1?'s':' ');
  }

  stop();
  repeatPattern=false;
  setOrder(0);
  BUSY_BEGIN_SOFT;

  logI("exporting furDS C file at %d Hz",refreshrate);

  double origRate=got.rate;
  got.rate=(double)(refreshrate);

  // determine loop point
  int loopOrder=0;
  int loopRow=0;
  int loopEnd=0;
  walkSong(loopOrder,loopRow,loopEnd);
  logI("loop point: %d %d",loopOrder,loopRow);
  warnings="";

  w=new SafeWriter;
  w->init();

  // reset the playback state
  curOrder=0;
  freelance=false;
  playing=false;
  extValuePresent=false;
  remainingLoops=-1;

  // Prepare to write song data
  playSub(false);
  //size_t tickCount=0;
  bool done=false;
  bool loopNow=false;
  int loopPos=-1;
  int wait=0;
  int oldWait=0;

  if (NDS>=0) disCont[NDS].dispatch->toggleRegisterDump(true);

  //disCont[NDS].dispatch->renderSamples(NDS);
  size_t sampleMemLength = disCont[NDS].dispatch->getSampleMemUsage(NDS);
  const void *samples = disCont[NDS].dispatch->getSampleMem(NDS);
  offset = 0;
  w->writeText("unsigned char samples[] = {"); 
  for (size_t i = 0; i < sampleMemLength; i++) {
    writeByte(*((uint8_t*)(samples+i)));
  }
  w->writeText("\n};\n");

  w->writeText(fmt::format("\nunsigned int sampleLength = {};\n",sampleMemLength));

  offset = 0;
  w->writeText("unsigned char song[] = {"); 
  while (!done) {
    if (nextTick() || !playing) {
      done=true;
      if (!loop) {
        writeByte(1);
        writeByte(wait&0xff);
        writeByte(wait>>8);
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].dispatch->getRegisterWrites().clear();
        }
        break;
      }
      if (!playing) {
        loopPos=-1;
      }
    }
    if (loopPos==-1) {
      if (loopOrder==curOrder && loopRow==curRow && loop)
        loopNow=true;
      if (loopNow) {
        // If Virtual Tempo is in use, our exact loop point
        // might be skipped due to quantization error.
        // If this happens, the tick immediately following is our loop point.
        if (ticks==1 || !(loopOrder==curOrder && loopRow==curRow)) {
          loopPos=offset;
          loopNow=false;
        }
      }
    }
    if (done) break;
    // get register dumps
    int i=0;
    if (NDS>=0) {
      // dump NDS writes
      i=NDS;
      std::vector<DivRegWrite>& writes=disCont[i].dispatch->getRegisterWrites();
      if (writes.size()>0) {
        logD("ndsOps: Writing %d messages to chip %d",writes.size(),i);
        writeByte(1);
        writeByte(wait&0xff);
        writeByte(wait>>8);
        wait=0;
      }
      wait++;
      for (DivRegWrite& write: writes) {
        if (i==NDS) {
          if (write.addr==0x100&&write.addr<0x108) {
            //writeByte(2);
            //writeByte(write.addr&0xff);
            //writeByte(write.val&0xff);
          } else {
            writeByte(0);
            writeByte(write.addr&0xff);
            writeByte(write.val&0xff);
          }
        }
      }
      writes.clear();
    }
  }
  // end of song

  // done - close out.
  got.rate=origRate;
  if (NDS>=0) disCont[NDS].dispatch->toggleRegisterDump(false);

  remainingLoops=-1;
  playing=false;
  freelance=false;
  extValuePresent=false;

  w->writeText("\n};\n");

  w->writeText(fmt::format("\nunsigned int songRate = {};\n",refreshrate));
  w->writeText(fmt::format("\nint loopPoint = {};\n",loopPos));
  w->writeText(fmt::format("\nunsigned int songLength = {};\n",offset));

  BUSY_END;
  return w;
}
