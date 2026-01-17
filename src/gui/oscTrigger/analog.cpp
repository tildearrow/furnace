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

// analog.cpp from unscope - https://github.com/Eknous-P/unscope
// modified for Furnace

#include "analog.h"

#define CHECK_TRIGGERED foundTrigger=triggerLow&&triggerHigh

TriggerAnalog::TriggerAnalog(float* cb):
  chanBuf(cb),
  triggerIndex(0),
  triggered(false) {}

bool TriggerAnalog::trigger(unsigned short windowSize, unsigned short readPos, float level, bool edge) {
  if (!chanBuf) return false;
  triggered = false;
  // locate trigger
  bool triggerHigh = false, triggerLow = false, foundTrigger = false;
  triggerIndex = (32768 - windowSize)&0x7fff;
  while (1) {
    triggerIndex--;
    float cur = chanBuf[(triggerIndex + windowSize/2 + readPos)&0x7fff];
    if (cur < level) {
      triggerLow = true;
      CHECK_TRIGGERED;
      if (foundTrigger && !edge) break;
    }
    if (cur > level) {
      triggerHigh = true;
      CHECK_TRIGGERED;
      if (foundTrigger && edge) break;
    }
    if (triggerIndex < 32768 - 2 * windowSize) return false; // out of window
  }
  triggerIndex = (triggerIndex + readPos)&0x7fff;
  triggered = true;
  return true;
}

bool TriggerAnalog::getTriggered() {
  return triggered;
}

unsigned short TriggerAnalog::getTriggerIndex() {
  return triggerIndex;
}

TriggerAnalog::~TriggerAnalog() {
}
