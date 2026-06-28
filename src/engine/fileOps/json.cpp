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
JSON serializeWavetable(DivWavetable* wave);
JSON serializeSample(DivSample* sample);
JSON serializeCompatFlags(DivCompatFlags* flags);

SafeWriter* DivEngine::saveJSON(DivJSONExportOptions* options) {
  saveLock.lock();

  JSON json;

  json["songInfo"]["version"]=song.version;
  json["songInfo"]["name"]=song.name;
  json["songInfo"]["author"]=song.author;
  json["songInfo"]["album"]=song.category;
  json["songInfo"]["system"]={song.autoSystem,song.systemName};
  json["songInfo"]["tuning"]=song.tuning;
  json["songInfo"]["instrumentCount"]=song.insLen;
  json["songInfo"]["wavetableCount"]=song.waveLen;
  json["songInfo"]["sampleCount"]=song.sampleLen;
  json["songInfo"]["comments"]=song.notes;

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

  json["subsongs"]={};

  if (options->exportInstruments) {
    json["instruments"]={};
    for (int j=0; j<song.insLen; j++) {
      json["instruments"].push_back(serializeInstrument(getIns(j)));
    }
  }
  if (options->exportWaves) {
    json["wavetables"]={};
    for (int j=0; j<song.waveLen; j++) {
      json["wavetables"].push_back(serializeWavetable(getWave(j)));
    }
  }
  if (options->exportSamples) {
    json["samples"]={};
    for (int j=0; j<song.sampleLen; j++) {
      json["samples"].push_back(serializeSample(getSample(j)));
    }
  }

  json["grooves"]={};
  for (auto& g:song.grooves) {
    JSON groove;
    groove["len"]=g.len;
    groove["val"]={};
    for (int i=0; i<g.len; i++) {
      groove["val"].push_back(g.val[i]);
    }
    json["grooves"].push_back(groove);
  }

  JSON patchbay;
  patchbay["auto"]=song.patchbayAuto;
  for (unsigned int& i:song.patchbay) {
    patchbay["connections"].push_back(i);
  }
  json["patchbay"]=patchbay;

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
    subsong["orderLength"]=s->ordersLen;
    subsong["highlights"]={s->hilightA,s->hilightB};

    if (options->exportOrders) subsong["orders"]={};
    if (options->exportPatterns) subsong["patterns"]={};
    subsong["channelData"]={};
    JSON order;
    JSON patterns;
    for (int j=0; j<song.chans; j++) {
      if (options->exportOrders || options->exportPatterns) {
        for (int k=0; k<s->ordersLen; k++) {
          if (options->exportOrders) order.push_back(s->orders.ord[j][k]);
          if (options->exportPatterns) patterns.push_back(serializePattern(s->pat[j].getPattern(j,false),s->patLen,s->pat[j].effectCols));
        }
        if (options->exportOrders) subsong["orders"].push_back(order);
        if (options->exportPatterns) subsong["patterns"].push_back(patterns);
      }

      JSON chanData;
      chanData["effectColumns"]=s->pat[j].effectCols;
      chanData["show"]["pattern"]=s->chanShow[j];
      chanData["show"]["chanOsc"]=s->chanShowChanOsc[j];
      chanData["collapse"]=s->chanCollapse[j];
      chanData["name"]=s->chanName[j];
      chanData["shortName"]=s->chanShortName[j];
      unsigned int color=s->chanColor[j];
      if (color) {
        chanData["color"]["r"]=(color)&255;
        chanData["color"]["g"]=(color>>8)&255;
        chanData["color"]["b"]=(color>>16)&255;
        chanData["color"]["a"]=(color>>24)&255;
      }
      subsong["channelData"].push_back(chanData);
    }
    subsong["notes"]=s->notes;

    json["subsongs"].push_back(subsong);
  }

  json["assetDirs"]={};
  JSON dir;
  if (!song.insDir.empty()) for (DivAssetDir& i:song.insDir) {
    dir["name"]=i.name;
    dir["assets"]={};
    for (int a:i.entries) {
      dir["assets"].push_back(a);
    }
    json["assetDirs"]["ins"].push_back(dir);
  }
  if (!song.waveDir.empty()) for (DivAssetDir& i:song.waveDir) {
    dir["name"]=i.name;
    dir["assets"]={};
    for (int a:i.entries) {
      dir["assets"].push_back(a);
    }
    json["assetDirs"]["wave"].push_back(dir);
  }
  if (!song.sampleDir.empty()) for (DivAssetDir& i:song.sampleDir) {
    dir["name"]=i.name;
    dir["assets"]={};
    for (int a:i.entries) {
      dir["assets"].push_back(a);
    }
    json["assetDirs"]["sample"].push_back(dir);
  }

  json["compatFlags"]=serializeCompatFlags(&song.compatFlags);

  SafeWriter* w=new SafeWriter;
  w->init();
  switch (options->format) {
    case DivJSONExportOptions::EXPORT_BSON: {
      std::vector<uint8_t> bsonDump=JSON::to_bson(json);
      w->write(bsonDump.data(),bsonDump.size());
      break;
    }
    case DivJSONExportOptions::EXPORT_CBOR: {
      std::vector<uint8_t> cborDump=JSON::to_cbor(json);
      w->write(cborDump.data(),cborDump.size());
      break;
    }
    case DivJSONExportOptions::EXPORT_JSON:
    default: {
      String dump;
      if (options->jsonPretty) {
        dump=json.dump(2);
      } else {
        dump=json.dump();
      }
      w->writeString(dump,false);
      break;
    }
  }
  if (w->tell()==0) {
    lastError="empty file";
  }
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
    // TODO: raw note
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
  if (featureMA) {
    json["macros"]={};
    DivInstrumentMacro* curMacro;
    for (DivMacroType i=DIV_MACRO_VOL; i<=DIV_MACRO_EX10; i=(DivMacroType)((int)i+1)) {
      curMacro=ins->std.macroByType(i);
      if (curMacro->len==0) continue;
      JSON macro;
      macro["code"]=(int)curMacro->macroType;
      macro["length"]=(int)curMacro->len;
      macro["loop"]=(int)curMacro->loop;
      macro["release"]=(int)curMacro->rel;
      macro["mode"]=curMacro->mode;
      macro["open"]=(int)curMacro->open&1;
      int macroType=(curMacro->open>>1)&3;
      macro["type"]=macroType;
      macro["instantRelease"]=(bool)(curMacro->open&(1<<3));
      macro["wordSize"]=(int)(curMacro->open>>5)&3;
      macro["delay"]=(int)curMacro->delay;
      macro["speed"]=(int)curMacro->speed;
      switch (macroType) {
        case 0: { // normal
          macro["data"]={};
          for (unsigned char j=0; j<curMacro->len; j++) {
            macro["data"].push_back((int)curMacro->val[j]);
          }
          break;
        }
        case 1: { // adsr
          macro["data"]["bottom"]=curMacro->val[0];
          macro["data"]["top"]=curMacro->val[1];
          macro["data"]["attack"]=curMacro->val[2];
          macro["data"]["hold"]=curMacro->val[3];
          macro["data"]["decay"]=curMacro->val[4];
          macro["data"]["sustain"]=curMacro->val[5];
          macro["data"]["susTime"]=curMacro->val[6];
          macro["data"]["susDecay"]=curMacro->val[7];
          macro["data"]["release"]=curMacro->val[8];
          break;
        }
        case 2: { // lfo
          macro["data"]["bottom"]=curMacro->val[0];
          macro["data"]["top"]=curMacro->val[1];
          macro["data"]["speed"]=curMacro->val[11];
          macro["data"]["shape"]=curMacro->val[12];
          macro["data"]["phase"]=curMacro->val[13];
          break;
        }
        default:
          logW("json: invalid macro type!");
      }
      json["macros"].push_back(macro);
    }
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
    JSON gb;
    SET_VALUE(gb,envVol);
    SET_VALUE(gb,envDir);
    SET_VALUE(gb,envLen);
    SET_VALUE(gb,soundLen);
    SET_VALUE(gb,hwSeqLen);
    SET_VALUE(gb,softEnv);
    SET_VALUE(gb,alwaysInit);
    SET_VALUE(gb,doubleWave);
    gb["hwSeq"]={};
    for (int i=0; i<=ins->gb.hwSeqLen; i++) {
      JSON seq;
      seq["cmd"]=ins->gb.hwSeq[i].cmd;
      seq["data"]=ins->gb.hwSeq[i].data;
      gb["hwSeq"].push_back(seq);
    }
    json["gb"]=gb;
  }
  if (feature64) {
    JSON c64;
    SET_VALUE(c64,triOn);
    SET_VALUE(c64,sawOn);
    SET_VALUE(c64,pulseOn);
    SET_VALUE(c64,noiseOn);
    SET_VALUE(c64,a);
    SET_VALUE(c64,d);
    SET_VALUE(c64,s);
    SET_VALUE(c64,r);
    SET_VALUE(c64,duty);
    SET_VALUE(c64,ringMod);
    SET_VALUE(c64,oscSync);
    SET_VALUE(c64,toFilter);
    SET_VALUE(c64,initFilter);
    SET_VALUE(c64,dutyIsAbs);
    SET_VALUE(c64,filterIsAbs);
    SET_VALUE(c64,noTest);
    SET_VALUE(c64,resetDuty);
    SET_VALUE(c64,res);
    SET_VALUE(c64,cut);
    SET_VALUE(c64,hp);
    SET_VALUE(c64,lp);
    SET_VALUE(c64,bp);
    SET_VALUE(c64,ch3off);
    json["64"]=c64;
  }
  if (featureSM) {
    JSON amiga;
    SET_VALUE(amiga,initSample);
    SET_VALUE(amiga,useNoteMap);
    SET_VALUE(amiga,useSample);
    SET_VALUE(amiga,useWave);
    SET_VALUE(amiga,waveLen);
    if (ins->amiga.useNoteMap) {
      amiga["sampleMap"]={};
      for (int i=0; i<180; i++) {
        JSON map;
        map["freq"]=ins->amiga.noteMap[i].freq;
        map["map"]=ins->amiga.noteMap[i].map;
        map["dpcmFreq"]=ins->amiga.noteMap[i].dpcmFreq;
        map["dpcmDelta"]=ins->amiga.noteMap[i].dpcmDelta;
        amiga["sampleMap"].push_back(map);
      }
    }
    json["amiga"]=amiga;
  }
  if (featureX1) {
    JSON x1_010;
    SET_VALUE(x1_010,bankSlot);
    json["x1_010"]=x1_010;
  }
  if (featureN1) {
    JSON n163;
    SET_VALUE(n163,wave);
    SET_VALUE(n163,wavePos);
    SET_VALUE(n163,waveLen);
    SET_VALUE(n163,perChanPos);
    JSON wavePosCh={}, waveLenCh={};
    for (int i=0; i<8; i++) {
      wavePosCh.push_back(ins->n163.wavePosCh[i]);
      waveLenCh.push_back(ins->n163.waveLenCh[i]);
    }
    n163["wavePosCh"]=wavePosCh;
    n163["waveLenCh"]=waveLenCh;
    json["n163"]=n163;
  }
  if (featureFD) {
    JSON fds;
    SET_VALUE(fds,modSpeed);
    SET_VALUE(fds,modDepth);
    SET_VALUE(fds,initModTableWithFirstWave);
    fds["modTable"]={};
    for (int i=0; i<32; i++)
      fds["modTable"].push_back(ins->fds.modTable[i]);

    json["fds"]=fds;
  }
  if (featureMP) {
    JSON multipcm;
    SET_VALUE(multipcm,ar);
    SET_VALUE(multipcm,d1r);
    SET_VALUE(multipcm,dl);
    SET_VALUE(multipcm,d2r);
    SET_VALUE(multipcm,rr);
    SET_VALUE(multipcm,rc);
    SET_VALUE(multipcm,lfo);
    SET_VALUE(multipcm,vib);
    SET_VALUE(multipcm,am);
    SET_VALUE(multipcm,damp);
    SET_VALUE(multipcm,pseudoReverb);
    SET_VALUE(multipcm,lfoReset);
    SET_VALUE(multipcm,levelDirect);
    json["multipcm"]=multipcm;
  }
  if (featureSU) {
    JSON su;
    SET_VALUE(su,switchRoles);
    SET_VALUE(su,hwSeqLen);
    su["hwSeq"]={};
    for (int i=0; i<=ins->su.hwSeqLen; i++) {
      JSON seq;
      seq["cmd"]=ins->su.hwSeq[i].cmd;
      seq["bound"]=ins->su.hwSeq[i].bound;
      seq["val"]=ins->su.hwSeq[i].val;
      seq["speed"]=ins->su.hwSeq[i].speed;
      // seq["padding"]=ins->su.hwSeq[i].padding;
      su["hwSeq"].push_back(seq);
    }
    json["su"]=su;
  }
  if (featureES) {
    JSON es;
    es["filter"]={};
    es["envelope"]={};
    es["filter"]["mode"]=ins->es5506.filter.mode;
    es["filter"]["k1"]=ins->es5506.filter.k1;
    es["filter"]["k2"]=ins->es5506.filter.k2;
    es["envelope"]["ecount"]=ins->es5506.envelope.ecount;
    es["envelope"]["lVRamp"]=ins->es5506.envelope.lVRamp;
    es["envelope"]["rVRamp"]=ins->es5506.envelope.rVRamp;
    es["envelope"]["k1Ramp"]=ins->es5506.envelope.k1Ramp;
    es["envelope"]["k2Ramp"]=ins->es5506.envelope.k2Ramp;
    es["envelope"]["k1Slow"]=ins->es5506.envelope.k1Slow;
    es["envelope"]["k2Slow"]=ins->es5506.envelope.k2Slow;
    json["es5506"]=es;
  }
  if (featureSN) {
    JSON snes;
    SET_VALUE(snes,useEnv);
    SET_VALUE(snes,sus);
    SET_VALUE(snes,gainMode);
    SET_VALUE(snes,a);
    SET_VALUE(snes,d);
    SET_VALUE(snes,s);
    SET_VALUE(snes,r);
    SET_VALUE(snes,d2);
    json["snes"]=snes;
  }
  if (featureEF) {
    JSON esfm;
    SET_VALUE(esfm,noise);
    esfm["op"]={};
    for (int i=0; i<4; i++) {
      JSON op;
#define SET_OP_VALUE(x) op[#x]=ins->esfm.op[i].x;
      SET_OP_VALUE(delay);
      SET_OP_VALUE(outLvl);
      SET_OP_VALUE(modIn);
      SET_OP_VALUE(left);
      SET_OP_VALUE(right);
      SET_OP_VALUE(fixed);
      SET_OP_VALUE(ct);
      SET_OP_VALUE(dt);
#undef SET_OP_VALUE
      esfm["op"].push_back(op);
    }
    json["esfm"]=esfm;
  }
  if (featurePN) {
    JSON powernoise;
    SET_VALUE(powernoise,octave);
    json["powernoise"]=powernoise;
  }

  return json;
}

JSON serializeWavetable(DivWavetable* wave) {
  JSON json;
  json["len"]=wave->len;
  // json["min"]=wave->min;
  json["max"]=wave->max;
  json["data"]={};
  for (int i=0; i<wave->len; i++) {
    json["data"].push_back(wave->data[i]);
  }

  return json;
}

JSON serializeSample(DivSample* sample) {
  JSON json;
  json["name"]=sample->name;
  json["centerRate"]=sample->centerRate;
  json["loopStart"]=sample->loopStart;
  json["loopEnd"]=sample->loopEnd;
  json["depth"]=sample->depth;
  json["loop"]=sample->loop;
  json["brrEmphasis"]=sample->brrEmphasis;
  json["brrNoFilter"]=sample->brrNoFilter;
  json["dither"]=sample->dither;
  json["loopMode"]=sample->loopMode;
  json["samples"]=sample->samples;

  json["renderOn"]={};
  for (int i=0; i<DIV_MAX_SAMPLE_TYPE; i++) {
    JSON renderPerType={};
    for (int j=0; j<DIV_MAX_CHIPS; j++) {
      renderPerType.push_back(sample->renderOn[i][j]);
    }
    json["renderOn"].push_back(renderPerType);
  }

  json["data"]={};
  unsigned char* data=(unsigned char*)sample->getCurBuf();
  for (unsigned int i=0; i<sample->getCurBufLen(); i++) {
    json["data"].push_back(data[i]);
  }
  return json;
}

JSON serializeCompatFlags(DivCompatFlags* flags) {
  JSON json;
#define SET_FLAG(f) json[ #f ]=flags->f
  SET_FLAG(limitSlides);
  SET_FLAG(linearPitch);
  SET_FLAG(pitchSlideSpeed);
  SET_FLAG(loopModality);
  SET_FLAG(delayBehavior);
  SET_FLAG(jumpTreatment);
  SET_FLAG(properNoiseLayout);
  SET_FLAG(waveDutyIsVol);
  SET_FLAG(resetMacroOnPorta);
  SET_FLAG(legacyVolumeSlides);
  SET_FLAG(compatibleArpeggio);
  SET_FLAG(noteOffResetsSlides);
  SET_FLAG(targetResetsSlides);
  SET_FLAG(arpNonPorta);
  SET_FLAG(algMacroBehavior);
  SET_FLAG(brokenShortcutSlides);
  SET_FLAG(ignoreDuplicateSlides);
  SET_FLAG(stopPortaOnNoteOff);
  SET_FLAG(continuousVibrato);
  SET_FLAG(brokenDACMode);
  SET_FLAG(oneTickCut);
  SET_FLAG(newInsTriggersInPorta);
  SET_FLAG(arp0Reset);
  SET_FLAG(brokenSpeedSel);
  SET_FLAG(noSlidesOnFirstTick);
  SET_FLAG(rowResetsArpPos);
  SET_FLAG(ignoreJumpAtEnd);
  SET_FLAG(buggyPortaAfterSlide);
  SET_FLAG(gbInsAffectsEnvelope);
  SET_FLAG(sharedExtStat);
  SET_FLAG(ignoreDACModeOutsideIntendedChannel);
  SET_FLAG(e1e2AlsoTakePriority);
  SET_FLAG(newSegaPCM);
  SET_FLAG(fbPortaPause);
  SET_FLAG(snDutyReset);
  SET_FLAG(pitchMacroIsLinear);
  SET_FLAG(oldOctaveBoundary);
  SET_FLAG(noOPN2Vol);
  SET_FLAG(newVolumeScaling);
  SET_FLAG(volMacroLinger);
  SET_FLAG(brokenOutVol);
  SET_FLAG(brokenOutVol2);
  SET_FLAG(e1e2StopOnSameNote);
  SET_FLAG(brokenPortaArp);
  SET_FLAG(snNoLowPeriods);
  SET_FLAG(disableSampleMacro);
  SET_FLAG(oldArpStrategy);
  SET_FLAG(brokenPortaLegato);
  SET_FLAG(brokenFMOff);
  SET_FLAG(preNoteNoEffect);
  SET_FLAG(oldDPCM);
  SET_FLAG(resetArpPhaseOnNewNote);
  SET_FLAG(ceilVolumeScaling);
  SET_FLAG(oldAlwaysSetVolume);
  SET_FLAG(oldSampleOffset);
  SET_FLAG(oldCenterRate);
  SET_FLAG(noVolSlideReset);
#undef SET_FLAG
  return json;
}
