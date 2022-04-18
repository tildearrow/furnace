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

#include "engine.h"
#include "../ta-log.h"
#include "../utfutils.h"
#include "song.h"
#include "zsm.h"

constexpr int MASTER_CLOCK_PREC=(sizeof(void*)==8)?8:0;

SafeWriter* DivEngine::saveZSM(unsigned int zsmrate, bool loop) {

  int VERA = -1;
  int YM = -1;
  int IGNORED = 0;
  
  //loop = false;
  // find indexes for YM and VERA. Ignore other systems.
  for (int i=0; i<song.systemLen; i++) {
	switch (song.system[i]) {
	  case DIV_SYSTEM_VERA:
	    if (VERA >= 0) { IGNORED++;break; }
	    VERA = i;
	    logD("VERA detected as chip id %d",i);
	    break;
	  case DIV_SYSTEM_YM2151:
	    if (YM >= 0) { IGNORED++;break; }
	    YM = i;
	    logD("YM detected as chip id %d",i);
	    break;
	  default:
	    IGNORED++;
	    logD("Ignoring chip %d systemID %d",i,song.system[i]);
	}
  }
  if (VERA < 0 && YM < 0) {
	logE("No supported systems for ZSM");
	return NULL;
  }
  if (IGNORED > 0)
    logW("ZSM export ignoring %d unsupported systems",IGNORED);
  
  stop();
  repeatPattern=false;
  setOrder(0);
  BUSY_BEGIN_SOFT;

  double origRate=got.rate;
  got.rate=zsmrate & 0xffff;
  
  // determine loop point
  int loopOrder=0;
  int loopRow=0;
  int loopEnd=0;
  walkSong(loopOrder,loopRow,loopEnd);
  logI("loop point: %d %d",loopOrder,loopRow);
  warnings="";

  ZSM zsm;
  logI("ZSM survived the constructor.");
  zsm.init(zsmrate);
  curOrder=0;
  freelance=false;
  playing=false;
  extValuePresent=false;
  remainingLoops=-1;

  // write song data
  playSub(false);
  size_t tickCount=0;
  bool writeLoop=false;
  bool done=false;
  int loopPos=-1;
  int loopTick=-1;
  int writeCount=0;

  if (VERA >= 0) disCont[VERA].dispatch->toggleRegisterDump(true);
  if (YM >= 0) disCont[YM].dispatch->toggleRegisterDump(true);
 
  while (!done) {
    if (loopPos==-1) {
      if (loopOrder==curOrder && loopRow==curRow && ticks==1) {
        writeLoop=true;
      }
    }
    if (nextTick() || !playing) {
      done=true;
      if (!loop) {
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].dispatch->getRegisterWrites().clear();
        }
        break;
      }  
      if (!playing) {
        writeLoop=false;
        loopPos=-1;
      }
    }
    // get register dumps
    for (int j=0; j<2; j++) {
	  int i=0;
	  if (j==0) {
	    if (YM < 0)
	      continue;
	    else
	      i=YM;
	  }
	  if (j==1) {
	    if (VERA < 0)
	      continue;
	    else {
		  i=VERA;
	    }
	  }
      std::vector<DivRegWrite>& writes=disCont[i].dispatch->getRegisterWrites();
      if (writes.size() > 0) logD("zsmOps: Writing %d messages to chip %d",writes.size(), i);
      for (DivRegWrite& write: writes) {
        //performVGMWrite(w,song.system[i],regval,streamIDs[i],loopTimer,loopFreq,loopSample,isSecond[i]);
        logD("zsmOps: r=%02x v=%02x to chip %d",write.addr&0xff, write.val, i);
        /*
        w->writeC(0x54);
        w->writeC(write.addr&0xff);
        w->writeC(write.val);
        */
        if (i==0) zsm.writeYM(write.addr&0xff, write.val);
        if (i==1) zsm.writePSG(write.addr&0xff, write.val);
        writeCount++;
      }
      writes.clear();
    }

    // write wait
    int totalWait=cycles>>MASTER_CLOCK_PREC;
    if (totalWait>0) {
	  /*
      if (totalWait==735) {
        w->writeC(0x62);
      } else if (totalWait==882) {
        w->writeC(0x63);
      } else {
        w->writeC(0x61);
        w->writeS(totalWait);
      }
      */
//      logD("zsmOps: DELAY %d",totalWait);
      zsm.tick(totalWait);
      tickCount+=totalWait;
    }
    if (writeLoop) {
      writeLoop=false;
      //loopPos=w->tell();
      zsm.setLoopPoint();
      loopTick=tickCount;
    }
  }
  // end of song
  // done - close out.
  
  got.rate = origRate;
  disCont[VERA].dispatch->toggleRegisterDump(false);
  disCont[YM].dispatch->toggleRegisterDump(false);
  
  remainingLoops=-1;
  playing=false;
  freelance=false;
  extValuePresent=false;

  logI("%d register writes total.",writeCount);

  BUSY_END;
  return zsm.finish();
 
}
