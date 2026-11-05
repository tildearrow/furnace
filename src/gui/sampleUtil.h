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

#define SAMPLE_OP_BEGIN \
  unsigned int start=0; \
  unsigned int end=sample->samples; \
  if (sampleSelStart<0) sampleSelStart=0; \
  if (sampleSelStart>(int)sample->samples) sampleSelStart=sample->samples; \
  if (sampleSelEnd<0) sampleSelEnd=0; \
  if (sampleSelEnd>(int)sample->samples) sampleSelEnd=sample->samples; \
  if (sampleSelStart!=-1 && sampleSelEnd!=-1 && sampleSelStart!=sampleSelEnd) { \
    start=sampleSelStart; \
    end=sampleSelEnd; \
    if (start>end) { \
      start^=end; \
      end^=start; \
      start^=end; \
    } \
  }
