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

#include "export.h"

static const char* describeType(unsigned char type) {
  switch (type) {
    case DIV_OBJECT_OTHER:
      return "Obj";
    case DIV_OBJECT_INS:
      return "Ins";
    case DIV_OBJECT_MACRO:
      return "Macro";
    case DIV_OBJECT_SAMPLE_MAP:
      return "SampleMap";
    case DIV_OBJECT_WAVE_SYNTH:
      return "WaveSynth";
    case DIV_OBJECT_INS_LIST_LOW:
      return "InsListLow";
    case DIV_OBJECT_INS_LIST_HIGH:
      return "InsListHigh";
    case DIV_OBJECT_CHIP_DATA:
      return "ChipData";
    case DIV_OBJECT_PITCH_TABLE:
      return "Pitch";
    case DIV_OBJECT_PITCH_TABLE_LIST_LOW:
      return "PitchListLow";
    case DIV_OBJECT_PITCH_TABLE_LIST_HIGH:
      return "PitchListHigh";
  }
  return "Obj";
}

SafeWriter* bakeObjectsASM(DivObjectPool& pool) {
  std::vector<String> labels;
  int labelCount[DIV_OBJECT_MAX];

  // make sure the pool has something
  if (pool.empty()) return NULL;

  // generate symbol names
  // TODO: use object name if present
  memset(labelCount,0,sizeof(int)*DIV_OBJECT_MAX);
  for (DivObject& i: pool) {
    unsigned char countIndex=i.type;
    if (countIndex>=DIV_OBJECT_MAX) countIndex=DIV_OBJECT_OTHER;

    labels.push_back(fmt::sprintf("song%s%d",describeType(countIndex),labelCount[countIndex]));
    labelCount[countIndex]++;
  }

  // prepare our writer
  SafeWriter* w=new SafeWriter;
  w->init();

  w->writeText("; baked with Furnace.\n\n");

  for (size_t _i=0; _i<pool.size(); _i++) {
    // write label
    w->writeText(labels[_i]);
    w->writeText(":");

    // write object data
    DivObject& i=pool[_i];
    unsigned char columnBytes=0;
    unsigned int nextReloc=0;
    for (size_t j=0; j<i.len; j++) {
      if (nextReloc<i.reloc.size()) {
        DivRelocInfo& reloc=i.reloc[nextReloc];
        if (reloc.offset==j) {
          // write label reference
          String label="error";
          if (reloc.objectIndex<pool.size()) {
            label=labels[reloc.objectIndex];
          }

          columnBytes=0;
          w->writeText("\n");
          switch (reloc.type) {
            case DIV_RELOC_PTR_U8:
              w->writeText(fmt::sprintf("  .db %s",label));
              break;
            case DIV_RELOC_PTR_U16:
              w->writeText(fmt::sprintf("  .dw %s",label));
              j++;
              break;
            case DIV_RELOC_PTR_U32:
              w->writeText(fmt::sprintf("  .dd %s",label));
              j+=3;
              break;
            case DIV_RELOC_PTR_U64:
              // we don't support this
              w->writeText(fmt::sprintf("  .dd %s, 0",label));
              j+=7;
              break;
            case DIV_RELOC_PTR_U16LSB:
              w->writeText(fmt::sprintf("  .db <%s",label));
              break;
            case DIV_RELOC_PTR_U16MSB:
              w->writeText(fmt::sprintf("  .db >%s",label));
              break;
            case DIV_RELOC_PTR_U16BE:
              w->writeText(fmt::sprintf("  .dw %s",label));
              j++;
              break;
            case DIV_RELOC_PTR_U32BE:
              w->writeText(fmt::sprintf("  .dd %s",label));
              j+=3;
              break;
            case DIV_RELOC_PTR_U64BE:
              // we don't support this
              w->writeText(fmt::sprintf("  .dd %s, 0",label));
              j+=7;
              break;
          }
          nextReloc++;
          continue;
        }
      }

      if (!columnBytes) w->writeText("\n  .db");
      w->writeText(fmt::sprintf(" $%.2x,",i.data[j]));

      if (++columnBytes>=16) {
        columnBytes=0;
      }
    }
    w->writeText("\n\n");
  }

  return w;
}

SafeWriter* bakeObjectsBinary(DivObjectPool& pool, unsigned int addr) {
  return NULL;
}
