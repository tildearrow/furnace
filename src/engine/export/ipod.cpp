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

// thanks asiekierka! (I used your SAP-R export code as a base for this)

#include "ipod.h"
#include "../engine.h"
#include "../ta-log.h"
#include <fmt/printf.h>
#include <array>
#include <vector>

void DivExportiPod::run() {
  int BEEPER=-1;
  int IGNORED=0;

  // Locate system index.
  for (int i=0; i<e->song.systemLen; i++) {
    if (e->song.system[i] == DIV_SYSTEM_PCSPKR) {
      if (BEEPER>=0) {
        IGNORED++;
        logAppendf("Ignoring duplicate Beeper id %d",i);
        break;
      }
      BEEPER=i;
      logAppendf("PC Speaker detected as chip id %d",i);
      break;
    } else {
      IGNORED++;
      logAppendf("Ignoring chip id %d, system id %d",i,(int)e->song.system[i]);
      break;
    }
  }
  if (BEEPER<0) {
    logAppendf("ERROR: Could not find PC Speaker/Beeper");
    failed=true;
    running=false;
    return;
  }
  if (IGNORED>0) {
    logAppendf("WARNING: iPod .tone export ignoring %d unsupported system%c",IGNORED,IGNORED>1?'s':' ');
  }

  size_t tickCount=0;

  double rate = 1000.0;

  e->stop();
  e->repeatPattern=false;
  e->setOrder(0);

  logAppend("playing and logging register writes...");

  int oldFreq = 0;
  int freq = 0;

  e->synchronizedSoft([&]() {
    double origRate = e->got.rate;
    e->got.rate=rate;

    // Determine loop point.
    int loopOrder=0;
    int loopRow=0;
    int loopEnd=0;
    e->walkSong(loopOrder,loopRow,loopEnd);
    logAppendf("loop point: %d %d",loopOrder,loopRow);
    e->warnings="";

    auto w = new SafeWriter;
    w->init(); 

    w->writeText(fmt::sprintf("%s\n", e->song.name));

    // Reset the playback state.
    e->curOrder=0;
    e->freelance=false;
    e->playing=false;
    e->extValuePresent=false;
    e->remainingLoops=-1;

    e->disCont[BEEPER].dispatch->toggleRegisterDump(true);

    // Prepare to write song data.
    e->playSub(false);
    bool done=false;

    logAppend("writing data...");
    progress[0].amount=0.15f;

    int wait_ms = 0;

    while (!done) {
      if (e->nextTick(false,true) || !e->playing) {
        done=true;
      }

      // get register dumps
      uint8_t* regPool = e->disCont[BEEPER].dispatch->getRegisterPool();
      int chipClock = e->disCont[BEEPER].dispatch->chipClock;
      freq = (int)(regPool[0]|(regPool[1]<<8));
      if (freq > 0) freq = chipClock/freq;

      // write wait
      tickCount++;
      int totalWait=e->cycles;
      if (totalWait>0 && !done) {
        while (totalWait) {
          wait_ms++;
          if (freq != oldFreq) {
            w->writeText(fmt::sprintf("%d %d\n", oldFreq, wait_ms));
            oldFreq = freq;
            wait_ms = 0;
          }
          totalWait--;
          tickCount++;
        }
      }
    }
    // end of song

    // done - close out.
    e->got.rate=origRate;
    e->disCont[BEEPER].dispatch->getRegisterWrites().clear();
    e->disCont[BEEPER].dispatch->toggleRegisterDump(false);

    e->remainingLoops=-1;
    e->playing=false;
    e->freelance=false;
    e->extValuePresent=false;

    output.push_back(DivROMExportOutput("export.tone",w));
  });


  progress[0].amount=1.0f;
  
  logAppend("finished!");

  running=false;
}

bool DivExportiPod::go(DivEngine* eng) {
  progress[0].name="Progress";
  progress[0].amount=0.0f;

  e=eng;
  running=true;
  failed=false;
  mustAbort=false;
  exportThread=new std::thread(&DivExportiPod::run,this);
  return true;
}

void DivExportiPod::wait() {
  if (exportThread!=NULL) {
    logV("waiting for export thread...");
    exportThread->join();
    delete exportThread;
  }
}

void DivExportiPod::abort() {
  mustAbort=true;
  wait();
}

bool DivExportiPod::isRunning() {
  return running;
}

bool DivExportiPod::hasFailed() {
  return failed;
}

DivROMExportProgress DivExportiPod::getProgress(int index) {
  if (index<0 || index>1) return progress[1];
  return progress[index];
}
