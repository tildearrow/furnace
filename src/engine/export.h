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

#ifndef _EXPORT_H
#define _EXPORT_H

#include "song.h"
#include <initializer_list>
#include "../pch.h"

class DivEngine;

enum DivROMExportOptions {
  DIV_ROM_ABSTRACT=0,
  DIV_ROM_AMIGA_VALIDATION,

  DIV_ROM_MAX
};

struct DivROMExportOutput {
  String name;
  SafeWriter* data;
  
  DivROMExportOutput(String n, SafeWriter* d):
    name(n),
    data(d) {}
  DivROMExportOutput():
    name(""),
    data(NULL) {}
};

struct DivROMExportProgress {
  String name;
  float amount;
};

class DivROMExport {
  std::vector<String> exportLog;
  std::mutex logLock;
  void logAppend(String what);
  public:
    virtual bool go(DivEngine* eng);
    virtual void abort();
    virtual std::vector<DivROMExportOutput> getResult();
    virtual bool hasFailed();
    virtual DivROMExportProgress getProgress();
    virtual ~DivROMExport() {}
};

struct DivROMExportDef {
  const char* name;
  const char* author;
  const char* description;
  DivSystem requisites[32];
  int requisitesLen;
  bool multiOutput;

  DivROMExportDef(const char* n, const char* a, const char* d, std::initializer_list<DivSystem> req, bool multiOut):
    name(n),
    author(a),
    description(d),
    multiOutput(multiOut) {
    requisitesLen=0;
    memset(requisites,0,32*sizeof(DivSystem));
    for (DivSystem i: req) {
      requisites[requisitesLen++]=i;
    }
  }
};

#endif
