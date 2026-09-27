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

#ifndef FURNACE_HORIZONTAL_PATTERN_H
#define FURNACE_HORIZONTAL_PATTERN_H

#include "../engine/defines.h"
#include <cstring>

// Pure row operations, shared by the view and regression tests. A displayed
// duration ends at the next note event; chip envelopes/effects may end it sooner.
namespace FurnaceHorizontalPattern {
inline bool pitched(int note) {
  return note>=0 && note<180;
}

inline const char* eventLabel(int note) {
  switch (note) {
    case DIV_NOTE_OFF: return "OFF";
    case DIV_NOTE_REL: return "REL";
    case DIV_MACRO_REL: return "MREL";
    case DIV_NOTE_RAW: return "RAW";
    case DIV_NOTE_NULL_PAT: return "BUG";
    default: return "???";
  }
}

// Align special events to their preceding note, or the next note for a leading
// event. Raw frequencies can retain a saved pitch. Never modify stored data.
inline int eventPitch(const short data[][DIV_MAX_COLS], int rows, int row, int fallback) {
  if (pitched(data[row][DIV_PAT_NOTE])) return data[row][DIV_PAT_NOTE];
  if (data[row][DIV_PAT_NOTE]==DIV_NOTE_RAW && pitched(data[row][DIV_PAT_NOTE_BUFFER])) return data[row][DIV_PAT_NOTE_BUFFER];
  for (int r=row-1; r>=0; r--) {
    if (pitched(data[r][DIV_PAT_NOTE])) return data[r][DIV_PAT_NOTE];
    if (data[r][DIV_PAT_NOTE]==DIV_NOTE_RAW && pitched(data[r][DIV_PAT_NOTE_BUFFER])) return data[r][DIV_PAT_NOTE_BUFFER];
  }
  for (int r=row+1; r<rows; r++) {
    if (pitched(data[r][DIV_PAT_NOTE])) return data[r][DIV_PAT_NOTE];
    if (data[r][DIV_PAT_NOTE]==DIV_NOTE_RAW && pitched(data[r][DIV_PAT_NOTE_BUFFER])) return data[r][DIV_PAT_NOTE_BUFFER];
  }
  return fallback;
}

inline int endRow(const short data[][DIV_MAX_COLS], int rows, int row) {
  for (int next=row+1; next<rows; next++) {
    if (data[next][DIV_PAT_NOTE]!=-1) return next;
  }
  return rows;
}

// mode: 1 insert, 2 move/resize, 3 erase. Reject collisions before touching data.
// Effects stay at their absolute rows. Only note/ins/volume travel with a note.
inline bool edit(short data[][DIV_MAX_COLS], int rows, int mode, int from,
                 int to, int note, int end, int instrument) {
  if (rows<1 || rows>DIV_MAX_ROWS || mode<1 || mode>3) return false;
  if (mode!=1 && (from<0 || from>=rows || !pitched(data[from][DIV_PAT_NOTE]))) return false;
  if (mode!=3 && (to<0 || to>=rows || !pitched(note) || end<=to || end>rows)) return false;
  short result[DIV_MAX_ROWS][DIV_MAX_COLS];
  memcpy(result,data,rows*sizeof(result[0]));
  int oldEnd=mode==1?rows:endRow(data,rows,from);
  int volume=-1;
  if (mode!=1) {
    instrument=data[from][DIV_PAT_INS];
    volume=data[from][DIV_PAT_VOL];
    result[from][DIV_PAT_NOTE]=-1;
    result[from][DIV_PAT_INS]=-1;
    result[from][DIV_PAT_VOL]=-1;
    // A cut is the explicit end of this note. Releases and other events are
    // independent commands and must never be silently removed.
    if (oldEnd<rows && data[oldEnd][DIV_PAT_NOTE]==DIV_NOTE_OFF) {
      result[oldEnd][DIV_PAT_NOTE]=-1;
    }
  }
  if (mode!=3) {
    for (int r=to; r<end; r++) {
      // A new attack at an existing cut is the usual way to join two notes.
      if (r==to && result[r][DIV_PAT_NOTE]==DIV_NOTE_OFF) continue;
      if (result[r][DIV_PAT_NOTE]!=-1) return false;
    }
    // Do not replace an instrument/volume-only event at the destination.
    if (to!=from || mode==1) {
      if (result[to][DIV_PAT_INS]!=-1 || result[to][DIV_PAT_VOL]!=-1) return false;
    }
    result[to][DIV_PAT_NOTE]=note;
    if (instrument>=0) result[to][DIV_PAT_INS]=instrument;
    if (volume>=0) result[to][DIV_PAT_VOL]=volume;
    // A note at the boundary already ends this span. Otherwise write a cut.
    if (end<rows && result[end][DIV_PAT_NOTE]==-1) result[end][DIV_PAT_NOTE]=DIV_NOTE_OFF;
  }
  memcpy(data,result,rows*sizeof(result[0]));
  return true;
}
}

#endif
