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

#include "../export.h"
#include "../../ta-log.h"

bool DivROMExport::go(DivEngine* eng) {
  logW("what's this? the null ROM export?");
  return false;
}

void DivROMExport::abort() {
}

std::vector<DivROMExportOutput>& DivROMExport::getResult() {
  return output;
}

bool DivROMExport::hasFailed() {
  return true;
}

DivROMExportProgress DivROMExport::getProgress(int index) {
  DivROMExportProgress ret;
  ret.name="";
  ret.amount=0.0f;
  return ret;
}

void DivROMExport::logAppend(String what) {
  logLock.lock();
  exportLog.push_back(what);
  logLock.unlock();
  logD("export: %s",what);
}

void DivROMExport::wait() {
}

bool DivROMExport::isRunning() {
  return false;
}

void DivROMExport::setConf(DivConfig& c) {
  conf=c;
}
