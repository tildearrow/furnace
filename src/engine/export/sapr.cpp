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

// thanks asiekierka!
// I have ported your code to this ROM export framework.

#include "sapr.h"
#include "../engine.h"
#include "../ta-log.h"
#include <fmt/printf.h>
#include <array>
#include <vector>

static String ticksToTime(double rate, int ticks) {
  double timing = ticks / rate;

  return fmt::sprintf("%02d:%02d.%03d",
    (int) timing / 60,
    (int) timing % 60,
    (int) (timing * 1000) % 1000
  );
}

void DivExportSAPR::run() {
  int sapScanlines=0; // TODO: property!
  int POKEY=-1;
  int IGNORED=0;

  // Locate system index.
  for (int i=0; i<e->song.systemLen; i++) {
    if (e->song.system[i] == DIV_SYSTEM_POKEY) {
      if (POKEY>=0) {
        IGNORED++;
        logAppendf("Ignoring duplicate POKEY id %d",i);
        break;
      }
      POKEY=i;
      logAppendf("POKEY detected as chip id %d",i);
      break;
    } else {
      IGNORED++;
      logAppendf("Ignoring chip id %d, system id %d",i,(int)e->song.system[i]);
      break;
    }
  }
  if (POKEY<0) {
    logAppendf("ERROR: Could not find POKEY");
    failed=true;
    running=false;
    return;
  }
  if (IGNORED>0) {
    logAppendf("WARNING: SAP export ignoring %d unsupported system%c",IGNORED,IGNORED>1?'s':' ');
  }

  bool palTiming=(e->song.systemFlags[POKEY].getInt("clockSel",0) != 0);
  int scanlinesPerFrame = (palTiming?312:262);
  size_t tickCount=0;
  std::vector<std::array<uint8_t, 9>> regs;

  if (sapScanlines <= 0) {
    sapScanlines = scanlinesPerFrame;
  }
  //double sapRate = (palTiming?49.86:59.92) * scanlinesPerFrame / sapScanlines;
  double sapRate = (palTiming?50:60) * (double)scanlinesPerFrame / (double)sapScanlines;


  e->stop();
  e->repeatPattern=false;
  e->setOrder(0);

  logAppend("playing and logging register writes...");

  e->synchronizedSoft([&]() {
    double origRate = e->got.rate;
    e->got.rate=sapRate;

    // Determine loop point.
    int loopOrder=0;
    int loopRow=0;
    int loopEnd=0;
    e->walkSong(loopOrder,loopRow,loopEnd);
    logAppendf("loop point: %d %d",loopOrder,loopRow);
    e->warnings="";

    // Reset the playback state.
    e->curOrder=0;
    e->freelance=false;
    e->playing=false;
    e->extValuePresent=false;
    e->remainingLoops=-1;

    // Prepare to write song data.
    e->playSub(false);
    bool done=false;
    e->disCont[POKEY].dispatch->toggleRegisterDump(true);
    std::array<uint8_t, 9> currRegs;

    while (!done) {
      if (e->nextTick(false,true) || !e->playing) {
        done=true;
        for (int i=0; i<e->song.systemLen; i++) {
          e->disCont[i].dispatch->getRegisterWrites().clear();
        }
        break;
      }
      // get register dumps
      std::vector<DivRegWrite>& writes=e->disCont[POKEY].dispatch->getRegisterWrites();
      if (writes.size() > 0) {
        logAppendf("saprOps: found %d messages",writes.size());
        for (DivRegWrite& write: writes)
          if ((write.addr & 0xF) < 9)
            currRegs[write.addr & 0xF] = write.val;
        writes.clear();
      }

      // write wait
      tickCount++;
      int totalWait=e->cycles;
      if (totalWait>0 && !done) {
        while (totalWait) {
          regs.push_back(currRegs);
          totalWait--;
          tickCount++;
        }
      }
    }
    // end of song

    // done - close out.
    e->got.rate=origRate;
    e->disCont[POKEY].dispatch->toggleRegisterDump(false);

    e->remainingLoops=-1;
    e->playing=false;
    e->freelance=false;
    e->extValuePresent=false;
  });

  logAppend("writing data...");
  progress[0].amount=0.95f;

  auto w = new SafeWriter;
  w->init(); 
  // Write SAP header: Author, name, timing, type.
  w->writeText(fmt::sprintf("SAP\r\nAUTHOR \"%s\"\r\nNAME \"%s\"\r\n%s\r\nTYPE R\r\n",
    e->song.author, e->song.name, palTiming ? "PAL" : "NTSC"
  ));
  if (sapScanlines != scanlinesPerFrame) {
    // Fastplay.
    w->writeText(fmt::sprintf("FASTPLAY %d\r\n", sapScanlines));
  }
  // Time.
  w->writeText(fmt::sprintf("TIME %s\r\n", ticksToTime(sapRate, tickCount)));
  w->writeText("\r\n");
  // Registers.
  for (auto atRegs : regs) {
    w->write(atRegs.data(), 9 * sizeof(uint8_t));
  }

  output.push_back(DivROMExportOutput("export.sap",w));

  progress[0].amount=1.0f;
  
  logAppend("finished!");

  running=false;
}

bool DivExportSAPR::go(DivEngine* eng) {
  progress[0].name="Progress";
  progress[0].amount=0.0f;

  e=eng;
  running=true;
  failed=false;
  mustAbort=false;
  exportThread=new std::thread(&DivExportSAPR::run,this);
  return true;
}

void DivExportSAPR::wait() {
  if (exportThread!=NULL) {
    logV("waiting for export thread...");
    exportThread->join();
    delete exportThread;
  }
}

void DivExportSAPR::abort() {
  mustAbort=true;
  wait();
}

bool DivExportSAPR::isRunning() {
  return running;
}

bool DivExportSAPR::hasFailed() {
  return failed;
}

DivROMExportProgress DivExportSAPR::getProgress(int index) {
  if (index<0 || index>1) return progress[1];
  return progress[index];
}
