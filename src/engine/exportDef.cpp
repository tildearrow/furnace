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

#include "engine.h"

DivROMExportDef* DivEngine::romExportDefs[DIV_ROM_MAX];

const DivROMExportDef* DivEngine::getROMExportDef(DivROMExportOptions opt) {
  return romExportDefs[opt];
}

bool DivEngine::isROMExportViable(DivROMExportOptions opt)
{
  const DivROMExportDef* newDef=getROMExportDef(opt);
  if (newDef==NULL) {
    return false;
  }

  unsigned char sysReqCount[DIV_SYSTEM_MAX];
  memset(sysReqCount,0,DIV_SYSTEM_MAX);
  for (int i=0; i<song.systemLen; i++) {
    sysReqCount[song.system[i]]++;
  }

  unsigned char defReqCount[DIV_SYSTEM_MAX];
  memset(defReqCount,0,DIV_SYSTEM_MAX);
  for (DivSystem j: newDef->requisites) {
    defReqCount[j]++;
  }

  switch (newDef->requisitePolicy) {
    case DIV_REQPOL_EXACT:
      for (int j=0; j<DIV_SYSTEM_MAX; j++) {
        if (defReqCount[j]!=sysReqCount[j]) {
          return false;
        }
      }
      break;
    case DIV_REQPOL_ANY:
      for (int j=0; j<DIV_SYSTEM_MAX; j++) {
        if (defReqCount[j]>sysReqCount[j]) {
          return false;
        }
      }
      break;
    case DIV_REQPOL_LAX:
      for (DivSystem j: newDef->requisites) {
        if (defReqCount[j]<=sysReqCount[j]) {
          return true;
        }
      }
      return false;
  }
  return true;
}

void DivEngine::registerROMExports() {
  logD("registering ROM exports...");

  romExportDefs[DIV_ROM_AMIGA_VALIDATION]=new DivROMExportDef(
    "Amiga Validation", "tildearrow",
    "a test export for ensuring Amiga emulation is accurate. do not use!",
    NULL, NULL,
    {DIV_SYSTEM_AMIGA},
    true, DIV_REQPOL_EXACT
  );

  romExportDefs[DIV_ROM_ZSM]=new DivROMExportDef(
    "Commander X16 ZSM", "ZeroByteOrg and MooingLemur",
    "Commander X16 Zsound Music File.\n"
    "for use with Melodius, Calliope and/or ZSMKit:\n"
    "- https://github.com/mooinglemur/zsmkit (development)\n"
    "- https://github.com/mooinglemur/melodius (player)\n"
    "- https://github.com/ZeroByteOrg/calliope (player)\n",
    "ZSM file", ".zsm",
    {
      DIV_SYSTEM_YM2151, DIV_SYSTEM_VERA
    },
    false, DIV_REQPOL_LAX
  );

  romExportDefs[DIV_ROM_TIUNA]=new DivROMExportDef(
    "Atari 2600 (TIunA)", "Natt Akuma",
    "advanced driver with software tuning support.\n"
    "see https://github.com/AYCEdemo/twin-tiuna for code.",
    "assembly files", ".asm",
    {
      DIV_SYSTEM_TIA
    },
    false, DIV_REQPOL_ANY
  );

  romExportDefs[DIV_ROM_SAP_R]=new DivROMExportDef(
    "Atari 8-bit SAP-R", "asiekierka",
    "SAP type R export for POKEY songs.\n"
    "register dump-based, unlike normal SAP.\n"
    "for playback, you may use:\n"
    "- Altirra\n"
    "- lzss-sap (https://github.com/dmsc/lzss-sap/)",
    "SAP files", ".sap",
    {
      DIV_SYSTEM_POKEY
    },
    false, DIV_REQPOL_EXACT
  );

  romExportDefs[DIV_ROM_IPOD]=new DivROMExportDef(
    "iPod .tone alarm", "AArt1256",
    "this is very cursed...",
    "alarm tone files", ".tone",
    {
      DIV_SYSTEM_PCSPKR
    },
    false, DIV_REQPOL_ANY
  );
}
