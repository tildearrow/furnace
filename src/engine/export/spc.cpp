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

#include "spc.h"
#include "../engine.h"
#include "../ta-log.h"
#include <fmt/printf.h>
#include <algorithm>
#include <map>
#include <tuple>
#include <vector>

void DivExportSNES::run() {
  DivObjectPool pool;

  int chipIndex=conf.getInt("sysToExport",-1);
  if (chipIndex<0 || chipIndex>=e->song.systemLen) {
    chipIndex=-1;
    for (int i=0; i<e->song.systemLen; i++) {
      if (e->song.system[i]==DIV_SYSTEM_SNES) {
        chipIndex=i;
        break;
      }
    }
    if (chipIndex<0) {
      logAppend("ERROR: SNES not found!");
      failed=true;
      running=false;
      return;
    }
  } else if (e->song.system[chipIndex]!=DIV_SYSTEM_SNES) {
    logAppend("ERROR: selected chip is not an SNES!");
    failed=true;
    running=false;
    return;
  }

  DivDispatch* dispatch=e->getDispatch(chipIndex);

  if (dispatch==NULL) {
    logAppend("ERROR: dispatch is NULL!");
    failed=true;
    running=false;
    return;
  }

  // export chip data
  logAppend("compiling system data...");
  if (!dispatch->compileROMData(1,pool)) {
    logAppend("ERROR: couldn't compile system data!");
    failed=true;
    running=false;
    return;
  }

  // export ins
  logAppend("compiling instruments...");
  if (!e->compileAllIns(pool,DIV_INS_SNES)) {
    logAppend("ERROR: couldn't compile instruments!");
    failed=true;
    running=false;
    return;
  }

  // export seq
  logAppend("compiling sequence data...");
  e->saveCommand(pool);

  // bake
  logAppend("baking song data...");
  SafeWriter* w=bakeObjectsASM(pool);
  if (w==NULL) {
    logAppend("ERROR: couldn't bake song data!");
    failed=true;
    running=false;
    return;
  }

  delete w;
  w=NULL;

  // destroy current pool

  // export sample
  logAppend("compiling sample data...");
  if (!dispatch->compileROMData(0,pool)) {
    logAppend("ERROR: couldn't compile system data!");
    failed=true;
    running=false;
    return;
  }

  // bake
  logAppend("baking sample data...");
  w=bakeObjectsBinary(pool);
  if (w==NULL) {
    logAppend("ERROR: couldn't bake song data!");
    failed=true;
    running=false;
    return;
  }

  // write song info and speed
  logAppend("writing song info...");

  // compile
  logAppend("building ROM...");

  logAppend("linking ROM...");

  w=new SafeWriter;
  w->init();
  output.push_back(DivROMExportOutput("export.spc",w));

  logAppend("finished!");

  running=false;
}

bool DivExportSNES::go(DivEngine* eng) {
  progress[0].name="Progress";
  progress[0].amount=0.0f;

  e=eng;
  running=true;
  failed=false;
  mustAbort=false;
  exportThread=new std::thread(&DivExportSNES::run,this);
  return true;
}

void DivExportSNES::wait() {
  if (exportThread!=NULL) {
    exportThread->join();
    delete exportThread;
  }
}

void DivExportSNES::abort() {
  mustAbort=true;
  wait();
}

bool DivExportSNES::isRunning() {
  return running;
}

bool DivExportSNES::hasFailed() {
  return failed;
}

DivROMExportProgress DivExportSNES::getProgress(int index) {
  if (index<0 || index>1) return progress[1];
  return progress[index];
}
