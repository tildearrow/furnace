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

#ifndef _INS_EDIT_COMMON_H
#define _INS_EDIT_COMMON_H
#include "../gui.h"

#define P(x) if (x) { \
  MARK_MODIFIED; \
  e->notifyInsChange(curIns); \
  updateFMPreview=true; \
}

#define PARAMETER MARK_MODIFIED; e->notifyInsChange(curIns); updateFMPreview=true;

void addAALine(ImDrawList* dl, const ImVec2& p1, const ImVec2& p2, const ImU32 color, float thickness=1.0f);

extern const char* macroAbsoluteMode;
extern const char* macroRelativeMode;
extern const char* macroQSoundMode;
extern const char* macroDummyMode;

String macroHoverNote(int id, float val, void* u);

#endif
