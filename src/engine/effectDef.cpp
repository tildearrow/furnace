/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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
#include "../ta-log.h"

DivEffectDef* DivEngine::effectDefs[DIV_EFFECT_MAX];

const DivEffectDef* DivEngine::getEffectDef(DivEffectType fx) {
  return effectDefs[fx];
}

void DivEngine::registerEffects() {
  logD("registering effects...");

  memset(effectDefs,0,DIV_EFFECT_MAX*sizeof(void*));

  effectDefs[DIV_EFFECT_DUMMY]=new DivEffectDef(
    "Dummy Effect",
    "a pass-through effect which does nothing."
  );
  
  effectDefs[DIV_EFFECT_EXTERNAL]=new DivEffectDef(
    "External",
    "allows you to load a plug-in. not implemented yet."
  );
  
  effectDefs[DIV_EFFECT_VOLUME]=new DivEffectDef(
    "Volume",
    "a volume effect."
  );
  
  effectDefs[DIV_EFFECT_FILTER]=new DivEffectDef(
    "Filter",
    "simulates a variety of analog filters."
  );

  effectsRegistered=true;
}
