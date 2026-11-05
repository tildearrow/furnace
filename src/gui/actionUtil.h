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

#define DETERMINE_FIRST \
  int firstChannel=0; \
  for (int i=0; i<e->getTotalChannelCount(); i++) { \
    if (e->curSubSong->chanShow[i]) { \
      firstChannel=i; \
      break; \
    } \
  }

#define DETERMINE_LAST \
  int lastChannel=0; \
  for (int i=e->getTotalChannelCount()-1; i>=0; i--) { \
    if (e->curSubSong->chanShow[i]) { \
      lastChannel=i+1; \
      break; \
    } \
  }

#define DETERMINE_FIRST_LAST \
  DETERMINE_FIRST \
  DETERMINE_LAST

