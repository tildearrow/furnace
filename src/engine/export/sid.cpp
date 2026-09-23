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

#include "sid.h"
#include "../engine.h"
#include "../ta-log.h"
#include "../../fileutils.h"
#include "../../executils.h"
#include <fmt/printf.h>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include "../../utfutils.h"
#else
#include <dirent.h>
#include <unistd.h>
#endif

void DivExportC64::run() {
  SafeWriter* w=new SafeWriter;
  w->init(); 

  // play the song and dump registers
  // then we'll compress each channel appropriately
  // freauency won't be stored as such and we'll have a tiny vibrato/porta engine instead to save space


  output.push_back(DivROMExportOutput("export.sid",w));

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
