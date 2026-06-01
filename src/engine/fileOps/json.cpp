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
#include "nlohmann/json.hpp"

using JSON = nlohmann::json;

JSON serializeInstrument(DivInstrument* ins) {
  JSON json;
  json["name"]=ins->name;
  json["type"]=ins->type;
  
  return json;
}

JSON serializePattern(DivPattern* pat, int rows, int effectCols) {
  JSON json;
  if (pat->isEmpty()) {
    json={};
    return json;
  }
  if (!pat->name.empty())
    json["name"]=pat->name;
  json["rows"]={};
  for (int i=0; i<rows; i++) {
    JSON row;
    bool isEmpty=true;
    if (pat->newData[i][DIV_PAT_NOTE]!=-1) {
      row["note"]=pat->newData[i][DIV_PAT_NOTE];
      isEmpty=false;
    }
    if (pat->newData[i][DIV_PAT_INS]!=-1) {
      row["ins"]=pat->newData[i][DIV_PAT_INS];
      isEmpty=false;
    }
    if (pat->newData[i][DIV_PAT_VOL]!=-1) {
      row["vol"]=pat->newData[i][DIV_PAT_VOL];
      isEmpty=false;
    }
    std::vector<std::pair<int,int>> effects;
    // while getting effects check for any empty cells after non-empty ones
    // example: instead of storing
    // [.... .... EA01 .... .... ....]
    // only store
    // [.... .... EA01]
    // removing those makes the export smaller
    int trailCount=0;
    for (int j=0; j<effectCols; j++) {
      std::pair<int,int> effect;
      effect={pat->newData[i][DIV_PAT_FX(j)],pat->newData[i][DIV_PAT_FXVAL(j)]};
      trailCount++;
      if (pat->newData[i][DIV_PAT_FX(j)]!=-1 || pat->newData[i][DIV_PAT_FXVAL(j)]!=-1) {
        isEmpty=false;
        trailCount=0;
      }
      effects.push_back(effect);
    }
    // remove trailing empty cells
    effects.resize(effects.size()-trailCount);
    // rewrite the vector as a json
    for (std::pair<int,int>& effPair:effects) {
      JSON effect;
      effect["code"]=effPair.first;
      effect["value"]=effPair.second;
      row["effects"].push_back(effect);
    }
    if (isEmpty)
      json["rows"].push_back({});
    else
      json["rows"].push_back(row);
    
  }

  return json;
}

SafeWriter* DivEngine::saveJSON(bool pretty) {
  saveLock.lock();

  JSON json;
  json["furVersion"]["versionString"]=DIV_VERSION;
  json["furVersion"]["versionNumber"]=DIV_ENGINE_VERSION;

  json["songInfo"]["name"]=song.name;
  json["songInfo"]["author"]=song.author;
  json["songInfo"]["album"]=song.category;
  json["songInfo"]["system"]=song.systemName;
  json["songInfo"]["tuning"]=song.tuning;
  json["songInfo"]["instrumentCount"]=song.insLen;
  json["songInfo"]["wavetableCount"]=song.waveLen;
  json["songInfo"]["sampleCount"]=song.sampleLen;

  json["chips"]={};
  for (int i=0; i<song.systemLen; i++) {
    JSON chip;
    chip["id"]=song.system[i];
    chip["name"]=getSystemName(song.system[i]);
    chip["volume"]=song.systemVol[i];
    chip["panning"]=song.systemPan[i];
    chip["front/rear"]=song.systemPanFR[i];
    JSON chipFlags;
    for (auto flag:song.systemFlags[i].configMap()) {
      chipFlags[flag.first]=flag.second;
    }
    chip["flags"]=chipFlags;
    json["chips"].push_back(chip);
  }

  json["comments"]=song.notes;
  json["subsongs"]={};
  
  for (size_t i=0; i<song.subsong.size(); i++) {
    JSON subsong;
    DivSubSong* s=song.subsong[i];
    subsong["name"]=s->name;
    subsong["tickRate"]=s->hz;
    subsong["speeds"]={};
    for (int j=0; j<s->speeds.len; j++)
      subsong["speeds"].push_back(s->speeds.val[j]);
    subsong["virtualTempo"]={s->virtualTempoN,s->virtualTempoD};
    subsong["patternLength"]=s->patLen;

    subsong["orders"]={};
    subsong["patterns"]={};
    for (int j=0; j<song.chans; j++) {
      JSON order;
      JSON patterns;
      for (int k=0; k<s->ordersLen; k++) {
        order.push_back(s->orders.ord[j][k]);
        patterns.push_back(serializePattern(s->pat[j].getPattern(j,false),s->patLen,s->pat[j].effectCols));
      }
      subsong["orders"].push_back(order);
      subsong["patterns"].push_back(patterns);
    }
    json["subsongs"].push_back(subsong);
  }

  SafeWriter* w=new SafeWriter;
  w->init();
  String dump;
  if (pretty) {
    dump=json.dump(2);
  } else {
    dump=json.dump();
  }
  w->writeString(dump,false);

  saveLock.unlock();
  return w;
}