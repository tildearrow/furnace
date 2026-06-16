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

JSON serializePattern(DivPattern* pat, int rows, int effectCols);
JSON serializeInstrument(DivInstrument* ins);

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
    subsong["instruments"]={};
    for (int j=0; j<song.insLen; j++) {
      subsong["instruments"].push_back(serializeInstrument(getIns(j)));
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

JSON serializeInstrument(DivInstrument* ins) {
  JSON json;
  if (!ins->name.empty())
    json["name"]=ins->name;
  json["type"]=ins->type;

  bool featureFM=false;
  bool featureMA=false;
  bool feature64=false;
  bool featureGB=false;
  bool featureSM=false;
  bool featureOx[4]={false,false,false,false};
  bool featureLD=false;
  bool featureSN=false;
  bool featureN1=false;
  bool featureFD=false;
  bool featureWS=false;
  bool featureMP=false;
  bool featureSU=false;
  bool featureES=false;
  bool featureX1=false;
  bool featureNE=false;
  bool featureEF=false;
  bool featurePN=false;
  bool featureS2=false;
  bool featureS3=false;
  switch (ins->type) {
    case DIV_INS_FM: featureFM=true; break;
    case DIV_INS_GB:
      featureGB=true;
      if (ins->ws.enabled) featureWS=true;
      break;
    case DIV_INS_C64: feature64=true; break;
    case DIV_INS_AMIGA: featureSM=true; break;
    case DIV_INS_PCE:
      featureSM=true;
      if (ins->ws.enabled) featureWS=true;
      break;
    case DIV_INS_AY: featureSM=true; break;
    case DIV_INS_AY8930: featureSM=true; break;
    case DIV_INS_VRC6: featureSM=true; break;
    case DIV_INS_OPLL:
      featureFM=true;
      if (ins->fm.fixedDrums) featureLD=true;
      break;
    case DIV_INS_OPL:
      featureFM=true;
      if (ins->fm.fixedDrums) featureLD=true;
      break;
    case DIV_INS_FDS:
      featureFD=true;
      if (ins->ws.enabled) featureWS=true;
      break;
    case DIV_INS_VBOY:
      featureFD=true;
      if (ins->ws.enabled) featureWS=true;
      break;
    case DIV_INS_N163:
      featureN1=true;
      if (ins->ws.enabled) featureWS=true;
      break;
    case DIV_INS_SCC:
      if (ins->ws.enabled) featureWS=true;
      break;
    case DIV_INS_OPZ: featureFM=true; break;
    case DIV_INS_POKEY:
    case DIV_INS_BEEPER: break;
    case DIV_INS_SWAN:
      featureSM=true;
      if (ins->ws.enabled) featureWS=true;
      break;
    case DIV_INS_MIKEY: featureSM=true; break;
    case DIV_INS_VERA: break;
    case DIV_INS_X1_010:
      featureX1=true;
      featureSM=true;
      if (ins->ws.enabled) featureWS=true;
      break;
    case DIV_INS_VRC6_SAW: break;
    case DIV_INS_ES5506:
      featureSM=true;
      featureES=true;
      break;
    case DIV_INS_MULTIPCM:
      featureSM=true;
      featureMP=true;
      break;
    case DIV_INS_SNES:
      featureSM=true;
      featureSN=true;
      if (ins->ws.enabled) featureWS=true;
      break;
    case DIV_INS_SU:
      featureSM=true;
      featureSU=true;
      break;
    case DIV_INS_NAMCO:
      if (ins->ws.enabled) featureWS=true;
      break;
    case DIV_INS_OPL_DRUMS:
      featureFM=true;
      if (ins->fm.fixedDrums) featureLD=true;
      break;
    case DIV_INS_OPM: featureFM=true; break;
    case DIV_INS_NES:
      featureSM=true;
      featureNE=true;
      break;
    case DIV_INS_MSM6258:
    case DIV_INS_MSM6295:
    case DIV_INS_ADPCMA:
    case DIV_INS_ADPCMB:
    case DIV_INS_SEGAPCM:
    case DIV_INS_QSOUND:
    case DIV_INS_YMZ280B:
    case DIV_INS_RF5C68:
    case DIV_INS_K007232:
    case DIV_INS_GA20:
    case DIV_INS_K053260:
    case DIV_INS_C140:
    case DIV_INS_C219:
    case DIV_INS_NDS:
    case DIV_INS_GBA_DMA:
    case DIV_INS_GBA_MINMOD:
    case DIV_INS_SUPERVISION:
      featureSM=true;
      break;
    case DIV_INS_SM8521:
      if (ins->ws.enabled) featureWS=true;
      break;
      featureSM=true;
      break;
      featureSM=true;
      break;
    case DIV_INS_ESFM:
      featureFM=true;
      featureEF=true;
      break;
    case DIV_INS_POWERNOISE:
    case DIV_INS_POWERNOISE_SLOPE:
      featurePN=true;
      break;
    case DIV_INS_SID2:
      feature64=true;
      featureS2=true;
      break;
    case DIV_INS_SID3:
      featureS3=true;
      featureSM=true;
      if (ins->ws.enabled) featureWS=true;
      break;
    case DIV_INS_STD:
    case DIV_INS_TIA:
    case DIV_INS_SAA1099:
    case DIV_INS_VIC:
    case DIV_INS_PET:
    case DIV_INS_MSM5232:
    case DIV_INS_T6W28:
    case DIV_INS_POKEMINI:
    case DIV_INS_PV1000:
    case DIV_INS_DAVE:
    case DIV_INS_BIFURCATOR:
    case DIV_INS_TED:
    case DIV_INS_UPD1771C:
    case DIV_INS_MAX:
    case DIV_INS_NULL:
      break;
  }
  
  if (ins->std.volMacro.len ||
      ins->std.arpMacro.len ||
      ins->std.dutyMacro.len ||
      ins->std.waveMacro.len ||
      ins->std.pitchMacro.len ||
      ins->std.ex1Macro.len ||
      ins->std.ex2Macro.len ||
      ins->std.ex3Macro.len ||
      ins->std.algMacro.len ||
      ins->std.fbMacro.len ||
      ins->std.fmsMacro.len ||
      ins->std.amsMacro.len ||
      ins->std.panLMacro.len ||
      ins->std.panRMacro.len ||
      ins->std.phaseResetMacro.len ||
      ins->std.ex4Macro.len ||
      ins->std.ex5Macro.len ||
      ins->std.ex6Macro.len ||
      ins->std.ex7Macro.len ||
      ins->std.ex8Macro.len ||
      ins->std.ex9Macro.len ||
      ins->std.ex10Macro.len) {
    featureMA=true;
  }
#define SET_VALUE(v,x) v[#x]=ins->v.x
  if (featureFM) {
    JSON fm;
    SET_VALUE(fm,alg);
    SET_VALUE(fm,fb);
    SET_VALUE(fm,fms);
    SET_VALUE(fm,ams);
    SET_VALUE(fm,fms2);
    SET_VALUE(fm,ams2);
    SET_VALUE(fm,ops);
    SET_VALUE(fm,opllPreset);
    SET_VALUE(fm,block);
    SET_VALUE(fm,fixedDrums);
    SET_VALUE(fm,kickFreq);
    SET_VALUE(fm,snareHatFreq);
    SET_VALUE(fm,tomTopFreq);
    JSON ops;
    for (int i=0; i<4; i++) {
      JSON op;
#define SET_OP_VALUE(x) op[#x]=ins->fm.op[i].x;
      SET_OP_VALUE(enable);
      SET_OP_VALUE(am);
      SET_OP_VALUE(ar);
      SET_OP_VALUE(dr);
      SET_OP_VALUE(mult);
      SET_OP_VALUE(rr);
      SET_OP_VALUE(sl);
      SET_OP_VALUE(tl);
      SET_OP_VALUE(dt2);
      SET_OP_VALUE(rs);
      SET_OP_VALUE(dt);
      SET_OP_VALUE(d2r);
      SET_OP_VALUE(ssgEnv);
      SET_OP_VALUE(dam);
      SET_OP_VALUE(dvb);
      SET_OP_VALUE(egt);
      SET_OP_VALUE(ksl);
      SET_OP_VALUE(sus);
      SET_OP_VALUE(vib);
      SET_OP_VALUE(ws);
      SET_OP_VALUE(ksr);
      SET_OP_VALUE(kvs);
#undef SET_OP_VALUE
      ops.push_back(op);
    }
    fm["operators"]=ops;
    json["fm"]=fm;
  }
  if (featureGB) {
    
  }

  return json;
}
