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

#ifndef _OBJECT_H
#define _OBJECT_H

enum DivRelocPointerType {
  DIV_RELOC_PTR_U8=0,
  DIV_RELOC_PTR_U16,
  DIV_RELOC_PTR_U32,
  DIV_RELOC_PTR_U64,
  // 6502 - lower/upper bytes in separate locations
  DIV_RELOC_PTR_U16LSB,
  DIV_RELOC_PTR_U16MSB,
  // big-endian
  DIV_RELOC_PTR_U16BE,
  DIV_RELOC_PTR_U32BE,
  DIV_RELOC_PTR_U64BE,
};

/**
 * an entry in a DivObject's relocation table.
 */
struct DivRelocInfo {
  // offset in data where the pointer is located.
  unsigned int offset;
  // the index in an object pool to the target object.
  unsigned int objectIndex;
  // the offset within the object.
  int objectOffset;
  // type of the pointer.
  DivRelocPointerType type;

  DivRelocInfo(unsigned int off, unsigned int objIndex, DivRelocPointerType t, int objOff=0):
    offset(off),
    objectIndex(objIndex),
    objectOffset(objOff),
    type(t) {}
};

/**
 * enum of DivObject types.
 */
enum DivObjectType {
  DIV_OBJECT_OTHER=0,
  DIV_OBJECT_INS,
  DIV_OBJECT_MACRO,
  DIV_OBJECT_SAMPLE_MAP,
  DIV_OBJECT_WAVE_SYNTH,
  DIV_OBJECT_INS_LIST_LOW,
  DIV_OBJECT_INS_LIST_HIGH,
  DIV_OBJECT_CHIP_DATA,
  DIV_OBJECT_PITCH_TABLE,
  DIV_OBJECT_PITCH_TABLE_LIST_LOW,
  DIV_OBJECT_PITCH_TABLE_LIST_HIGH,
  DIV_OBJECT_OCTAVE_SHIFT_LIST,

  DIV_OBJECT_MAX
};

/**
 * an object contains binary data and a relocation table.
 * used in ROM export.
 *
 * data must be allocated and deleted manually!
 */
struct DivObject {
  const char* nameHint;
  unsigned char* data;
  size_t len;
  unsigned char type;
  std::vector<DivRelocInfo> reloc;
  DivObject(unsigned char* d, size_t l, unsigned char t):
    nameHint(NULL),
    data(d),
    len(l),
    type(t) {}
  DivObject(unsigned char* d, size_t l, unsigned char t, const char* n):
    nameHint(n),
    data(d),
    len(l),
    type(t) {}
  DivObject():
    nameHint(NULL),
    data(NULL),
    len(0),
    type(0) {}
};

typedef std::vector<DivObject> DivObjectPool;

#endif
