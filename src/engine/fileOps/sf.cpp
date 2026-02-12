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

#include "fileOpsCommon.h"
#include <fmt/printf.h>
#define TSF_IMPLEMENTATION
#include "tsf.h"

class DivEngine;

void DivEngine::preloadSoundfont(String path, std::vector<SoundfontPreset>& ret) {
  tsf* soundfont=tsf_load_filename(path.c_str());
  if (!soundfont) return;
  ret.clear();
  for (int i=0; i<soundfont->presetNum; i++) {
    ret.push_back({String(tsf_get_presetname(soundfont,i)),(unsigned int)i,false});
  }
  tsf_close(soundfont);
}

void DivEngine::loadSoundfont(String path, std::vector<unsigned int> which, bool genSampleMap) {
  tsf* soundfont=tsf_load_filename(path.c_str());
  if (!soundfont) return;

  for (int i:which) {
    DivInstrument* map=NULL;
    if (genSampleMap) {
      map=new DivInstrument;
      map->type=DIV_INS_AMIGA;
      map->amiga.useNoteMap=true;
    }
    for (int j=0; j<soundfont->presets[i].regionNum; j++) {
      tsf_region* curRegion=&soundfont->presets[i].regions[j];
      DivSample* s=new DivSample;
      s->centerRate=curRegion->sample_rate;
      s->depth=DIV_SAMPLE_DEPTH_16BIT;
      s->name=fmt::sprintf("%s_%d",tsf_get_presetname(soundfont,i),j);
      s->init(curRegion->end-curRegion->offset);
      switch (curRegion->loop_mode) {
        case TSF_LOOPMODE_NONE: break;
        case TSF_LOOPMODE_SUSTAIN:
        case TSF_LOOPMODE_CONTINUOUS:
          s->loop=true;
          s->loopMode=DIV_SAMPLE_LOOP_FORWARD;
          s->loopStart=curRegion->loop_start-curRegion->offset;
          s->loopEnd=curRegion->loop_end-curRegion->offset;
          break;
      }
      for (unsigned int k=curRegion->offset, m=0; k<curRegion->end; k++, m++) {
        s->data16[m]=CLAMP(int(soundfont->fontSamples[k]*32768.0f),-32768,32767);
      }
      song.sample.push_back(s);

      if (genSampleMap) {
        map->name=tsf_get_presetname(soundfont,i);
        map->amiga.noteMap[curRegion->transpose+60]={
          curRegion->pitch_keycenter,
          short(song.sample.size()-1),
          0,0
        };
      }
    }
    if (genSampleMap) {
      addInstrumentPtr(map);
    }
  }

  tsf_close(soundfont);
}
