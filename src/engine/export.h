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

#ifndef _EXPORT_H
#define _EXPORT_H

#include "song.h"
#include <initializer_list>
#include "../pch.h"

class DivEngine;

enum DivROMExportOptions {
  DIV_ROM_ABSTRACT=0,
  DIV_ROM_AMIGA_VALIDATION,
  DIV_ROM_ZSM,
  DIV_ROM_TIUNA,
  DIV_ROM_SAP_R,
  DIV_ROM_IPOD,
  DIV_ROM_GRUB,

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
  protected:
    DivConfig conf;
    std::vector<DivROMExportOutput> output;
    void logAppend(String what);
  public:
    std::vector<String> exportLog;
    std::mutex logLock;

    void setConf(DivConfig& c);
    virtual bool go(DivEngine* eng);
    virtual void abort();
    virtual void wait();
    std::vector<DivROMExportOutput>& getResult();
    virtual bool hasFailed();
    virtual bool isRunning();
    virtual DivROMExportProgress getProgress(int index=0);
    virtual ~DivROMExport() {}
};

#define logAppendf(...) logAppend(fmt::sprintf(__VA_ARGS__))

enum DivROMExportReqPolicy {
  // exactly these chips.
  DIV_REQPOL_EXACT=0,
  // specified chips must be present but any amount of them is acceptable.
  DIV_REQPOL_ANY,
  // at least one of the specified chips.
  DIV_REQPOL_LAX
};

struct DivROMExportDef {
  const char* name;
  const char* author;
  const char* description;
  const char* fileType;
  const char* fileExt;
  std::vector<DivSystem> requisites;
  bool multiOutput;
  DivROMExportReqPolicy requisitePolicy;

  DivROMExportDef(const char* n, const char* a, const char* d, const char* ft, const char* fe, std::initializer_list<DivSystem> req, bool multiOut, DivROMExportReqPolicy reqPolicy):
    name(n),
    author(a),
    description(d),
    fileType(ft),
    fileExt(fe),
    multiOutput(multiOut),
    requisitePolicy(reqPolicy) {
    requisites=req;
  }
};

#endif
