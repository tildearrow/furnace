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

#include "scripting.h"

// instruments are so complicated the API helper functions are kept separate

const DivInstrument defaultIns;

void writeMacro(DivInstrumentMacro* macro, const char* key, const char* subkey, int value) {
  #define CHECK_PARAM(_k,_p,_mn,_mx) if (strcmp(key,_k)==0) {macro->_p=CLAMP(value,_mn,_mx);}
  CHECK_PARAM("delay",delay,0,255)
  else CHECK_PARAM("speed",speed,0,255)
  else CHECK_PARAM("loop",loop,0,255)
  else CHECK_PARAM("release",rel,0,255)
  else CHECK_PARAM("mode",mode,0,255)
  else if (strcmp(key,"open")==0) {
    macro->open=(macro->open&(~1))|CLAMP(value,0,1);
  } else if (strcmp(key,"instantRelease")==0) {
    macro->open=(macro->open&(~(1<<3)))|(CLAMP(value,0,1)<<3);
  }
  #undef CHECK_PARAM
  #define CHECK_PARAM(_k,_v) if (strcmp(subkey,_k)==0) {macro->val[_v]=value;}
  if (subkey) {
    if (strcmp(key,"envelope")==0) {
      CHECK_PARAM("bottom",0)
      else CHECK_PARAM("top",1)
      else CHECK_PARAM("attack",2)
      else CHECK_PARAM("hold",3)
      else CHECK_PARAM("decay",4)
      else CHECK_PARAM("sustain",5)
      else CHECK_PARAM("susTime",6)
      else CHECK_PARAM("susDecay",7)
      else CHECK_PARAM("release",8)
      macro->open=(macro->open&(~6))|2;
    } else if (strcmp(key,"lfo")==0) {
      CHECK_PARAM("bottom",0)
      else CHECK_PARAM("top",1)
      else CHECK_PARAM("speed",11)
      else CHECK_PARAM("waveform",12)
      else CHECK_PARAM("phase",13)
      macro->open=(macro->open&(~6))|4;
    }
  } else {
    macro->open=(macro->open&(~6));
    macro->len=0;
  }
}

void writeFeatureFM(DivInstrument* ins, lua_State* s) {
  lua_pushstring(s,"fm");
  lua_newtable(s);
  API_ADD_VALUE("alg",ins->fm.alg,integer)
  API_ADD_VALUE("feedback",ins->fm.fb,integer)
  API_ADD_VALUE("fms",ins->fm.fms,integer)
  API_ADD_VALUE("ams",ins->fm.ams,integer)
  API_ADD_VALUE("fmsLFO",ins->fm.fmsLFO,boolean)
  API_ADD_VALUE("amsLFO",ins->fm.amsLFO,boolean)
  API_ADD_VALUE("tremLFO",ins->fm.tremLFO,boolean)
  API_ADD_VALUE("opllPreset",ins->fm.opllPreset,integer)
  API_ADD_VALUE("block",ins->fm.block,integer)
  API_ADD_VALUE("fixedDrums",ins->fm.fixedDrums,boolean)
  API_ADD_VALUE("kickFreq",ins->fm.kickFreq,integer)
  API_ADD_VALUE("snareHatFreq",ins->fm.snareHatFreq,integer)
  API_ADD_VALUE("tomTopFreq",ins->fm.tomTopFreq,integer)
  // operators
  {
    lua_pushstring(s,"op");
    lua_newtable(s);
    for (int i=0; i<4; i++) {
      DivInstrumentFM::Operator* op=&ins->fm.op[i];
      lua_pushinteger(s,i+1);
      lua_newtable(s);
      API_ADD_VALUE("enable",op->enable,boolean)
      API_ADD_VALUE("am",op->am,integer)
      API_ADD_VALUE("ar",op->ar,integer)
      API_ADD_VALUE("dr",op->dr,integer)
      API_ADD_VALUE("mult",op->mult,integer)
      API_ADD_VALUE("rr",op->rr,integer)
      API_ADD_VALUE("sl",op->sl,integer)
      API_ADD_VALUE("tl",op->tl,integer)
      API_ADD_VALUE("dt2",op->dt2,integer)
      API_ADD_VALUE("rs",op->rs,integer)
      API_ADD_VALUE("dt",op->dt,integer)
      API_ADD_VALUE("dt2",op->dt2,integer)
      API_ADD_VALUE("d2r",op->d2r,integer)
      API_ADD_VALUE("ssgEnv",op->ssgEnv,integer)
      API_ADD_VALUE("dam",op->dam,integer)
      API_ADD_VALUE("dvb",op->dvb,integer)
      API_ADD_VALUE("egt",op->egt,integer)
      API_ADD_VALUE("ksl",op->ksl,integer)
      API_ADD_VALUE("sus",op->sus,integer)
      API_ADD_VALUE("vib",op->vib,integer)
      API_ADD_VALUE("ws",op->ws,integer)
      API_ADD_VALUE("ksr",op->ksr,integer)
      API_ADD_VALUE("kvs",op->kvs,integer)
      lua_settable(s,-3);
    }
    lua_settable(s,-3);
  }
  lua_settable(s,-3);
}

void writeFeatureGB(DivInstrument* ins, lua_State* s) {
  lua_pushstring(s,"gb");
  lua_newtable(s);
  API_ADD_VALUE("envVol",ins->gb.envVol,integer)
  API_ADD_VALUE("envDir",ins->gb.envDir,integer)
  API_ADD_VALUE("soundLen",ins->gb.soundLen,integer)
  API_ADD_VALUE("softEnv",ins->gb.softEnv,boolean)
  API_ADD_VALUE("alwaysInit",ins->gb.alwaysInit,boolean)
  API_ADD_VALUE("doubleWave",ins->gb.softEnv,boolean)
  // hw sequences
  if (ins->gb.hwSeqLen) {
    lua_pushstring(s,"hwSeq");
    lua_newtable(s);
    for (int i=0; i<ins->gb.hwSeqLen; i++) {
      lua_pushinteger(s,i+1);
      lua_newtable(s);
      API_ADD_VALUE("cmd",ins->gb.hwSeq[i].cmd,integer);
      API_ADD_VALUE("data",ins->gb.hwSeq[i].data,integer);
      lua_settable(s,-3);
    }
    lua_settable(s,-3);
  }
  lua_settable(s,-3);
}

void writeFeatureC64(DivInstrument* ins, lua_State* s) {
  lua_pushstring(s,"c64");
  lua_newtable(s);
  API_ADD_VALUE("toFilter",ins->c64.toFilter,boolean)
  API_ADD_VALUE("initFilter",ins->c64.initFilter,boolean)
  API_ADD_VALUE("dutyIsAbs",ins->c64.dutyIsAbs,boolean)
  API_ADD_VALUE("filterIsAbs",ins->c64.filterIsAbs,boolean)
  API_ADD_VALUE("noTest",ins->c64.noTest,boolean)
  API_ADD_VALUE("resetDuty",ins->c64.resetDuty,boolean)
  API_ADD_VALUE("ringMod",ins->c64.ringMod,integer)
  API_ADD_VALUE("oscSync",ins->c64.oscSync,integer)
  {
    lua_pushstring(s,"envelope");
    lua_newtable(s);
    API_ADD_VALUE("a",ins->c64.a,integer)
    API_ADD_VALUE("d",ins->c64.d,integer)
    API_ADD_VALUE("s",ins->c64.s,integer)
    API_ADD_VALUE("r",ins->c64.r,integer)
    lua_settable(s,-3);
  }
  {
    lua_pushstring(s,"osc");
    lua_newtable(s);
    API_ADD_VALUE("triOn",ins->c64.triOn,boolean)
    API_ADD_VALUE("sawOn",ins->c64.sawOn,boolean)
    API_ADD_VALUE("pulseOn",ins->c64.pulseOn,boolean)
    API_ADD_VALUE("noiseOn",ins->c64.noiseOn,boolean)
    API_ADD_VALUE("duty",ins->c64.duty,integer)
    lua_settable(s,-3);
  }
  {
    lua_pushstring(s,"filter");
    lua_newtable(s);
    API_ADD_VALUE("res",ins->c64.res,integer)
    API_ADD_VALUE("cut",ins->c64.cut,integer)
    API_ADD_VALUE("hp",ins->c64.hp,boolean)
    API_ADD_VALUE("lp",ins->c64.lp,boolean)
    API_ADD_VALUE("bp",ins->c64.bp,boolean)
    API_ADD_VALUE("ch3off",ins->c64.ch3off,boolean)
    lua_settable(s,-3);
  }
  lua_settable(s,-3);
}

void writeFeatureAmiga(DivInstrument* ins, lua_State* s) {
  lua_pushstring(s,"amiga");
  lua_newtable(s);
  API_ADD_VALUE("initSample",ins->amiga.initSample,integer)
  API_ADD_VALUE("useNoteMap",ins->amiga.useNoteMap,boolean)
  API_ADD_VALUE("useSample",ins->amiga.useSample,boolean)
  API_ADD_VALUE("useWave",ins->amiga.useWave,boolean)
  API_ADD_VALUE("waveLen",ins->amiga.waveLen,integer)
  // sample map
  // precheck for empty map
  bool mapHasAny=false;
  for (int i=0; i<180; i++) {
    DivInstrumentAmiga::SampleMap* map=&ins->amiga.noteMap[i];
    if (map->map!=-1) {
      mapHasAny=true;
      break;
    }
  }
  if (mapHasAny) {
    lua_pushstring(s,"noteMap");
    lua_newtable(s);
    for (int i=0; i<180; i++) {
      DivInstrumentAmiga::SampleMap* map=&ins->amiga.noteMap[i];
      lua_pushinteger(s,i+1);
      lua_newtable(s);
      API_ADD_VALUE("freq",map->freq,integer);
      API_ADD_VALUE("map",map->map,integer);
      API_ADD_VALUE("dpcmFreq",map->dpcmFreq,integer);
      API_ADD_VALUE("dpcmDelta",map->dpcmDelta,integer);
      lua_settable(s,-3);
    }
    lua_settable(s,-3);
  }
  lua_settable(s,-3);
}

void writeFeatureX1(DivInstrument* ins, lua_State* s) {
  lua_pushstring(s,"x1");
  lua_newtable(s);
  API_ADD_VALUE("bankSlot",ins->x1_010.bankSlot,integer)
  lua_settable(s,-3);
}

void writeFeatureN163(DivInstrument* ins, lua_State* s) {
  lua_pushstring(s,"n163");
  lua_newtable(s);
  API_ADD_VALUE("wave",ins->n163.wave,integer)
  API_ADD_VALUE("wavePos",ins->n163.wavePos,integer)
  API_ADD_VALUE("waveLen",ins->n163.waveLen,integer)
  API_ADD_VALUE("waveMode",ins->n163.waveMode,integer)
  API_ADD_VALUE("perChanPos",ins->n163.perChanPos,boolean)
  {
    lua_pushstring(s,"wavePerChan");
    lua_newtable(s);
    for (int i=0; i<8; i++) {
      lua_pushinteger(s,i+1);
      lua_newtable(s);
      API_ADD_VALUE("pos",ins->n163.wavePosCh[i],integer)
      API_ADD_VALUE("len",ins->n163.waveLenCh[i],integer)
      lua_settable(s,-3);
    }
    lua_settable(s,-3);
  }
  lua_settable(s,-3);
}

void writeFeatureFDS(DivInstrument* ins, lua_State* s) {
  lua_pushstring(s,"fds");
  lua_newtable(s);
  API_ADD_VALUE("modSpeed",ins->fds.modSpeed,integer)
  API_ADD_VALUE("modDepth",ins->fds.modDepth,integer)
  API_ADD_VALUE("initModTableWithFirstWave",ins->fds.initModTableWithFirstWave,boolean)
  {
    lua_pushstring(s,"modTable");
    lua_newtable(s);
    for (int i=0; i<32; i++) {
      lua_pushinteger(s,i+1);
      lua_pushinteger(s,ins->fds.modTable[i]);
      lua_settable(s,-3);
    }
    lua_settable(s,-3);
  }
  lua_settable(s,-3);
}

void writeFeatureMultiPCM(DivInstrument* ins, lua_State* s) {
  lua_pushstring(s,"multiPCM");
  lua_newtable(s);
  {
    lua_pushstring(s,"envelope");
    lua_newtable(s);
    API_ADD_VALUE("ar",ins->multipcm.ar,integer)
    API_ADD_VALUE("d1r",ins->multipcm.d1r,integer)
    API_ADD_VALUE("d2r",ins->multipcm.d2r,boolean)
    API_ADD_VALUE("dl",ins->multipcm.dl,integer)
    API_ADD_VALUE("rr",ins->multipcm.rr,integer)
    API_ADD_VALUE("rc",ins->multipcm.rc,integer)
    lua_settable(s,-3);
  }
  API_ADD_VALUE("lfo",ins->multipcm.lfo,integer);
  API_ADD_VALUE("vib",ins->multipcm.vib,integer);
  API_ADD_VALUE("am",ins->multipcm.am,integer);
  API_ADD_VALUE("damp",ins->multipcm.damp,boolean);
  API_ADD_VALUE("pseudoReverb",ins->multipcm.pseudoReverb,boolean);
  API_ADD_VALUE("lfoReset",ins->multipcm.lfoReset,boolean);
  API_ADD_VALUE("levelDirect",ins->multipcm.levelDirect,boolean);
  lua_settable(s,-3);
}

void writeFeatureWaveSynth(DivInstrument* ins, lua_State* s) {
  lua_pushstring(s,"waveSynth");
  lua_newtable(s);
  API_ADD_VALUE("wave1",ins->ws.wave1,integer);
  API_ADD_VALUE("wave2",ins->ws.wave2,integer);
  API_ADD_VALUE("rateDivider",ins->ws.rateDivider,integer);
  API_ADD_VALUE("effect",ins->ws.effect,integer);
  API_ADD_VALUE("oneShot",ins->ws.oneShot,boolean);
  API_ADD_VALUE("enabled",ins->ws.enabled,boolean);
  API_ADD_VALUE("global",ins->ws.global,boolean);
  API_ADD_VALUE("speed",ins->ws.speed,integer);
  API_ADD_VALUE("param1",ins->ws.param1,integer);
  API_ADD_VALUE("param2",ins->ws.param2,integer);
  API_ADD_VALUE("param3",ins->ws.param3,integer);
  API_ADD_VALUE("param4",ins->ws.param4,integer);
  lua_settable(s,-3);
}

void writeFeatureSoundUnit(DivInstrument* ins, lua_State* s) {
  lua_pushstring(s,"soundUnit");
  lua_newtable(s);
  API_ADD_VALUE("switchRoles",ins->su.switchRoles,boolean);
  {
    lua_pushstring(s,"hwSeq");
    lua_newtable(s);
    for (int i=0; i<ins->su.hwSeqLen; i++) {
      lua_pushinteger(s,i+1);
      lua_newtable(s);
      API_ADD_VALUE("cmd",ins->su.hwSeq[i].cmd,integer);
      API_ADD_VALUE("bound",ins->su.hwSeq[i].bound,integer);
      API_ADD_VALUE("val",ins->su.hwSeq[i].val,integer);
      API_ADD_VALUE("speed",ins->su.hwSeq[i].speed,integer);
      lua_settable(s,-3);
    }
    lua_settable(s,-3);
  }
  lua_settable(s,-3);
}

void writeFeatureES5506(DivInstrument* ins, lua_State* s) {
  lua_pushstring(s,"es5506");
  lua_newtable(s);
  {
    lua_pushstring(s,"filter");
    lua_newtable(s);
    API_ADD_VALUE("mode",ins->es5506.filter.mode,integer)
    API_ADD_VALUE("k1",ins->es5506.filter.k1,integer)
    API_ADD_VALUE("k2",ins->es5506.filter.k2,integer)
    lua_settable(s,-3);
  }
  {
    lua_pushstring(s,"envelope");
    lua_newtable(s);
    API_ADD_VALUE("ecount",ins->es5506.envelope.ecount,integer)
    API_ADD_VALUE("lVRamp",ins->es5506.envelope.lVRamp,integer)
    API_ADD_VALUE("rVRamp",ins->es5506.envelope.rVRamp,integer)
    API_ADD_VALUE("k1Ramp",ins->es5506.envelope.k1Ramp,integer)
    API_ADD_VALUE("k2Ramp",ins->es5506.envelope.k2Ramp,integer)
    API_ADD_VALUE("k1Slow",ins->es5506.envelope.k1Slow,boolean)
    API_ADD_VALUE("k2Slow",ins->es5506.envelope.k2Slow,boolean)
    lua_settable(s,-3);
  }
  lua_settable(s,-3);
}

void writeFeatureSNES(DivInstrument* ins, lua_State* s) {
  lua_pushstring(s,"snes");
  lua_newtable(s);
  API_ADD_VALUE("useEnv",ins->snes.useEnv,boolean);
  API_ADD_VALUE("sus",ins->snes.sus,integer);
  API_ADD_VALUE("gain",ins->snes.gain,integer);
  API_ADD_VALUE("gainMode",ins->snes.gainMode,integer);
  {
    lua_pushstring(s,"envelope");
    lua_newtable(s);
    API_ADD_VALUE("a",ins->snes.a,integer)
    API_ADD_VALUE("d",ins->snes.d,integer)
    API_ADD_VALUE("s",ins->snes.s,integer)
    API_ADD_VALUE("r",ins->snes.r,integer)
    API_ADD_VALUE("d2",ins->snes.d2,integer)
    lua_settable(s,-3);
  }
  lua_settable(s,-3);
}

void writeFeatureESFM(DivInstrument* ins, lua_State* s) {
  lua_pushstring(s,"esfm");
  lua_newtable(s);
  API_ADD_VALUE("noise",ins->esfm.noise,integer)
  // operators
  {
    lua_pushstring(s,"op");
    lua_newtable(s);
    for (int i=0; i<4; i++) {
      DivInstrumentESFM::Operator* op=&ins->esfm.op[i];
      lua_pushinteger(s,i+1);
      lua_newtable(s);
      API_ADD_VALUE("delay",op->delay,integer)
      API_ADD_VALUE("outLvl",op->outLvl,integer)
      API_ADD_VALUE("modIn",op->modIn,integer)
      API_ADD_VALUE("left",op->left,integer)
      API_ADD_VALUE("right",op->right,integer)
      API_ADD_VALUE("fixed",op->fixed,integer)
      API_ADD_VALUE("ct",op->ct,integer)
      API_ADD_VALUE("dt",op->dt,integer)
      lua_settable(s,-3);
    }
    lua_settable(s,-3);
  }
  lua_settable(s,-3);
}

void writeFeaturePowerNoise(DivInstrument* ins, lua_State* s) {
  lua_pushstring(s,"powerNoise");
  lua_newtable(s);
  API_ADD_VALUE("octave",ins->powernoise.octave,integer)
  lua_settable(s,-3);
}

void writeFeatureSID2(DivInstrument* ins, lua_State* s) {
  lua_pushstring(s,"sid2");
  lua_newtable(s);
  API_ADD_VALUE("volume",ins->sid2.volume,integer)
  API_ADD_VALUE("mixMode",ins->sid2.mixMode,integer)
  API_ADD_VALUE("noiseMode",ins->sid2.noiseMode,integer)
  lua_settable(s,-3);
}

void writeFeatureSID3(DivInstrument* ins, lua_State* s) {
  lua_pushstring(s,"sid3");
  lua_newtable(s);
  API_ADD_VALUE("mixMode",ins->sid3.mixMode,integer)
  API_ADD_VALUE("duty",ins->sid3.duty,integer)
  API_ADD_VALUE("oneBitNoise",ins->sid3.oneBitNoise,boolean)
  API_ADD_VALUE("separateNoisePitch",ins->sid3.separateNoisePitch,boolean)
  API_ADD_VALUE("doWavetable",ins->sid3.doWavetable,boolean)
  API_ADD_VALUE("resetDuty",ins->sid3.resetDuty,boolean)
  API_ADD_VALUE("dutyIsAbs",ins->sid3.dutyIsAbs,boolean)
  API_ADD_VALUE("phaseInv",ins->sid3.phaseInv,integer)
  API_ADD_VALUE("feedback",ins->sid3.feedback,integer)
  {
    lua_pushstring(s,"waveform");
    lua_newtable(s);
    API_ADD_VALUE("noise",ins->sid3.noiseOn,boolean)
    API_ADD_VALUE("pulse",ins->sid3.pulseOn,boolean)
    API_ADD_VALUE("saw",ins->sid3.sawOn,boolean)
    API_ADD_VALUE("tri",ins->sid3.triOn,boolean)
    if (ins->sid3.specialWaveOn) {
      API_ADD_VALUE("special",ins->sid3.special_wave,integer)
    }
    lua_settable(s,-3);
  }
  {
    lua_pushstring(s,"phaseMod");
    lua_newtable(s);
    API_ADD_VALUE("enable",ins->sid3.phase_mod,boolean)
    API_ADD_VALUE("source",ins->sid3.phase_mod_source,integer)
    lua_settable(s,-3);
  }
  {
    lua_pushstring(s,"ringMod");
    lua_newtable(s);
    API_ADD_VALUE("enable",ins->sid3.ringMod,boolean)
    API_ADD_VALUE("source",ins->sid3.ring_mod_source,integer)
    lua_settable(s,-3);
  }
  {
    lua_pushstring(s,"oscSync");
    lua_newtable(s);
    API_ADD_VALUE("enable",ins->sid3.oscSync,boolean)
    API_ADD_VALUE("source",ins->sid3.sync_source,integer)
    lua_settable(s,-3);
  }
  {
    lua_pushstring(s,"envelope");
    lua_newtable(s);
    API_ADD_VALUE("a",ins->sid3.a,integer)
    API_ADD_VALUE("d",ins->sid3.d,integer)
    API_ADD_VALUE("s",ins->sid3.s,integer)
    API_ADD_VALUE("sr",ins->sid3.sr,integer)
    API_ADD_VALUE("t",ins->sid3.r,integer)
    lua_settable(s,-3);
  }
  // operators
  {
    lua_pushstring(s,"filter");
    lua_newtable(s);
    for (int i=0; i<4; i++) {
      DivInstrumentSID3::Filter* filt=&ins->sid3.filt[i];
      lua_pushinteger(s,i+1);
      lua_newtable(s);
      API_ADD_VALUE("enable",filt->enabled,boolean)
      API_ADD_VALUE("init",filt->init,boolean)
      API_ADD_VALUE("absoluteCutoff",filt->absoluteCutoff,boolean)
      API_ADD_VALUE("bindCutoffOnNote",filt->bindCutoffOnNote,boolean)
      API_ADD_VALUE("bindCutoffToNote",filt->bindCutoffToNote,boolean)
      API_ADD_VALUE("bindCutoffToNoteDir",filt->bindCutoffToNoteDir,boolean)
      API_ADD_VALUE("bindResonanceOnNote",filt->bindResonanceOnNote,boolean)
      API_ADD_VALUE("bindResonanceToNote",filt->bindResonanceToNote,boolean)
      API_ADD_VALUE("bindCutoffToNoteDir",filt->bindCutoffToNoteDir,boolean)

      API_ADD_VALUE("cutoff",filt->cutoff,integer)
      API_ADD_VALUE("resonance",filt->resonance,integer)
      API_ADD_VALUE("outputVolume",filt->output_volume,integer)
      API_ADD_VALUE("distortionLevel",filt->distortion_level,integer)
      API_ADD_VALUE("mode",filt->mode,integer)
      API_ADD_VALUE("filterMatrix",filt->filter_matrix,integer)
      API_ADD_VALUE("bindCutoffToNoteStrength",filt->bindCutoffToNoteStrength,integer)
      API_ADD_VALUE("bindCutoffToNoteCenter",filt->bindCutoffToNoteCenter,integer)
      API_ADD_VALUE("bindResonanceToNoteStrength",filt->bindResonanceToNoteStrength,integer)
      API_ADD_VALUE("bindResonanceToNoteCenter",filt->bindResonanceToNoteCenter,integer)
      lua_settable(s,-3);
    }
    lua_settable(s,-3);
  }
  lua_settable(s,-3);
}

void writeFeatureKlattsch(DivInstrument* ins, lua_State* s) {
  lua_pushstring(s,"klattsch");
  lua_newtable(s);
  API_ADD_VALUE("transition",ins->klattsch.transition,integer)
  API_ADD_VALUE("voicing",ins->klattsch.voicing,integer)
  API_ADD_VALUE("aspiration",ins->klattsch.aspiration,integer)
  API_ADD_VALUE("tilt",ins->klattsch.tilt,integer)
  API_ADD_VALUE("effort",ins->klattsch.effort,integer)
  API_ADD_VALUE("vibrato",ins->klattsch.vibrato,integer)
  API_ADD_VALUE("tremolo",ins->klattsch.tremolo,integer)
  API_ADD_VALUE("gain",ins->klattsch.gain,integer)
  API_ADD_VALUE("bandwidth",ins->klattsch.bandwidth,integer)
  API_ADD_VALUE("formantShift",ins->klattsch.formantShift,integer)
  lua_settable(s,-3);
}

#define CHECK_VALUE(_k,_p,_mn,_mx) if (strcmp(key,_k)==0) {ins->_p=/*CLAMP(value,_mn,_mx)*/value;}
#define CHECK_SUBVALUE(_k,_p,_mn,_mx) if (strcmp(subkey,_k)==0) {ins->_p=/*CLAMP(value,_mn,_mx)*/value;}

void readFeatureFM(DivInstrument *ins, lua_State *s, int t) {
  lua_pushnil(s);
  while (lua_next(s,t)) {
    if (!lua_isstring(s,-2)) continue;
    const char* key=lua_tostring(s,-2);
    int value;
    switch (lua_type(s,-1)) {
      case LUA_TBOOLEAN:
        value=lua_toboolean(s,-1);
        break;
      case LUA_TNUMBER:
        value=lua_tointeger(s,-1);
        break;
      case LUA_TTABLE: { // op table
        if (strcmp(key,"op")!=0) break;
        int ops=0;
        lua_pushnil(s);
        while (lua_next(s,-2)) {
          if (lua_isinteger(s,-2)) {
            if (lua_istable(s,-1)) {
              int op=lua_tointeger(s,-2);
              if (op>4 || op<1) {
                lua_pop(s,1);
                continue;
              }
              ops++;
              lua_pushnil(s);
              while (lua_next(s,-2)) {
                if (lua_isstring(s,-2)) {
                  const char* subkey=lua_tostring(s,-2);
                  switch (lua_type(s,-1)) {
                    case LUA_TBOOLEAN:
                      value=lua_toboolean(s,-1);
                      break;
                    case LUA_TNUMBER:
                      value=lua_tointeger(s,-1);
                      break;
                    default: 
                      lua_pop(s,1);
                      continue;
                  }
                  #define CHECK_OP_VALUE(_p,_mn,_mx) if (strcmp(subkey,#_p)==0) {ins->fm.op[op-1]._p=/*CLAMP(value,_mn,_mx)*/value;}
                  CHECK_OP_VALUE(enable,0,1)
                  CHECK_OP_VALUE(am,0,1)
                  CHECK_OP_VALUE(ar,0,1)
                  CHECK_OP_VALUE(dr,0,1)
                  CHECK_OP_VALUE(mult,0,15)
                  CHECK_OP_VALUE(rr,0,1)
                  CHECK_OP_VALUE(sl,0,1)
                  CHECK_OP_VALUE(tl,0,127)
                  CHECK_OP_VALUE(dt2,0,1)
                  CHECK_OP_VALUE(rs,0,1)
                  CHECK_OP_VALUE(dt,0,1)
                  CHECK_OP_VALUE(dt2,0,1)
                  CHECK_OP_VALUE(d2r,0,1)
                  CHECK_OP_VALUE(ssgEnv,0,1)
                  CHECK_OP_VALUE(dam,0,1)
                  CHECK_OP_VALUE(dvb,0,1)
                  CHECK_OP_VALUE(egt,0,1)
                  CHECK_OP_VALUE(ksl,0,1)
                  CHECK_OP_VALUE(sus,0,1)
                  CHECK_OP_VALUE(vib,0,1)
                  CHECK_OP_VALUE(ws,0,1)
                  CHECK_OP_VALUE(ksr,0,1)
                  CHECK_OP_VALUE(kvs,0,1)
                  #undef CHECK_OP_VALUE
                }
                lua_pop(s,1);
              }
            }
          }
          lua_pop(s,1);
          ins->fm.ops=CLAMP(ops,1,4);
        }
        lua_pop(s,1);
        continue;
      }
    }
    CHECK_VALUE("alg",fm.alg,0,7)
    CHECK_VALUE("feedback",fm.fb,0,7)
    CHECK_VALUE("fms",fm.fms,0,7)
    CHECK_VALUE("ams",fm.ams,0,7)
    CHECK_VALUE("fmsLFO",fm.fmsLFO,0,1)
    CHECK_VALUE("amsLFO",fm.amsLFO,0,1)
    CHECK_VALUE("tremLFO",fm.tremLFO,0,1)
    CHECK_VALUE("opllPreset",fm.opllPreset,0,15)
    CHECK_VALUE("block",fm.block,0,7)
    CHECK_VALUE("fixedDrums",fm.fixedDrums,0,1)
    CHECK_VALUE("kickFreq",fm.kickFreq,0,7)
    CHECK_VALUE("snareHatFreq",fm.snareHatFreq,0,7)
    CHECK_VALUE("tomTopFreq",fm.tomTopFreq,0,7)
    lua_pop(s,1);
  }
}

// TODO!!!
void readFeatureGB(DivInstrument* ins, lua_State* s, int t) {
  lua_pushnil(s);
  while (lua_next(s,t)) {
    if (!lua_isstring(s,-2)) continue;
    const char* key=lua_tostring(s,-2);
    int value;
    switch (lua_type(s,-1)) {
      case LUA_TBOOLEAN:
        value=lua_toboolean(s,-1);
        break;
      case LUA_TNUMBER:
        value=lua_tointeger(s,-1);
        break;
      case LUA_TTABLE: { // hwSeq table
        if (strcmp(key,"hwSeq")!=0) break;
        int seqLen=0;
        lua_pushnil(s);
        while (lua_next(s,-2)) {
          if (lua_isinteger(s,-2)) {
            if (lua_istable(s,-1)) {
              int seq=lua_tointeger(s,-2);
              if (seq>255 || seq<1) {
                lua_pop(s,1);
                continue;
              }
              seqLen++;
              lua_pushnil(s);
              while (lua_next(s,-2)) {
                if (lua_isstring(s,-2)) {
                  const char* subkey=lua_tostring(s,-2);
                  switch (lua_type(s,-1)) {
                    case LUA_TBOOLEAN:
                      value=lua_toboolean(s,-1);
                      break;
                    case LUA_TNUMBER:
                      value=lua_tointeger(s,-1);
                      break;
                    default: 
                      lua_pop(s,1);
                      continue;
                  }
                  if (strcmp(subkey,"cmd")==0) {
                    ins->gb.hwSeq[seq-1].cmd=value;
                  }
                  if (strcmp(subkey,"data")==0) {
                    ins->gb.hwSeq[seq-1].data=value;
                  }
                }
                lua_pop(s,1);
              }
            }
          }
          lua_pop(s,1);
          ins->gb.hwSeqLen=CLAMP(seqLen,0,255);
        }
        lua_pop(s,1);
        continue;
      }
    }
    CHECK_VALUE("envVol",gb.envVol,0,7)
    CHECK_VALUE("envDir",gb.envDir,0,7)
    CHECK_VALUE("soundLen",gb.soundLen,0,7)
    CHECK_VALUE("softEnv",gb.softEnv,0,1)
    CHECK_VALUE("alwaysInit",gb.alwaysInit,0,1)
    CHECK_VALUE("doubleWave",gb.doubleWave,0,1)
    lua_pop(s,1);
  }
}

void readFeatureC64(DivInstrument* ins, lua_State* s, int t) {
  lua_pushnil(s);
  while (lua_next(s,t)) {
    if (!lua_isstring(s,-2)) continue;
    const char* key=lua_tostring(s,-2);
    int value;
    switch (lua_type(s,-1)) {
      case LUA_TBOOLEAN:
        value=lua_toboolean(s,-1);
        break;
      case LUA_TNUMBER:
        value=lua_tointeger(s,-1);
        break;
      case LUA_TTABLE: {
        int whichTable=-1;
        if (strcmp(key,"envelope")==0) {
          whichTable=0;
        } else if (strcmp(key,"osc")==0) {
          whichTable=1;
        } else if (strcmp(key,"filter")==0) {
          whichTable=2;
        } else break;
        lua_pushnil(s);
        while (lua_next(s,-2)) {
          if (!lua_isstring(s,-2)) continue;
          const char* subkey=lua_tostring(s,-2);
          switch (lua_type(s,-1)) {
            case LUA_TBOOLEAN:
              value=lua_toboolean(s,-1);
              break;
            case LUA_TNUMBER:
              value=lua_tointeger(s,-1);
              break;
          }
          switch (whichTable) {
            case 0: {
              CHECK_SUBVALUE("a",c64.a,0,15)
              CHECK_SUBVALUE("d",c64.d,0,15)
              CHECK_SUBVALUE("s",c64.s,0,15)
              CHECK_SUBVALUE("r",c64.r,0,15)
              break;
            }
            case 1: {
              CHECK_SUBVALUE("triOn",c64.triOn,0,1)
              CHECK_SUBVALUE("sawOn",c64.sawOn,0,1)
              CHECK_SUBVALUE("pulseOn",c64.pulseOn,0,1)
              CHECK_SUBVALUE("noiseOn",c64.noiseOn,0,1)
              CHECK_SUBVALUE("duty",c64.noiseOn,0,1)
              break;
            }
            case 2: {
              CHECK_SUBVALUE("res",c64.res,0,15)
              CHECK_SUBVALUE("cut",c64.cut,0,15)
              CHECK_SUBVALUE("hp",c64.hp,0,1)
              CHECK_SUBVALUE("lp",c64.lp,0,1)
              CHECK_SUBVALUE("bp",c64.bp,0,1)
              CHECK_SUBVALUE("ch3off",c64.ch3off,0,1)
              break;
            }
            default: break;
          }
          lua_pop(s,1);
        }
        lua_pop(s,1);
        continue;
      }
    }
    CHECK_VALUE("toFilter",c64.toFilter,0,1)
    CHECK_VALUE("initFilter",c64.initFilter,0,1)
    CHECK_VALUE("dutyIsAbs",c64.dutyIsAbs,0,1)
    CHECK_VALUE("filterIsAbs",c64.filterIsAbs,0,1)
    CHECK_VALUE("noTest",c64.noTest,0,1)
    CHECK_VALUE("resetDuty",c64.resetDuty,0,1)
    CHECK_VALUE("ringMod",c64.ringMod,0,1)
    CHECK_VALUE("oscSync",c64.oscSync,0,1)
    lua_pop(s,1);
  }
}

void readFeatureAmiga(DivInstrument* ins, lua_State* s, int t) {
  lua_pushnil(s);
  while (lua_next(s,t)) {
    if (!lua_isstring(s,-2)) continue;
    const char* key=lua_tostring(s,-2);
    int value;
    switch (lua_type(s,-1)) {
      case LUA_TBOOLEAN:
        value=lua_toboolean(s,-1);
        break;
      case LUA_TNUMBER:
        value=lua_tointeger(s,-1);
        break;
      case LUA_TTABLE: { // noteMap table
        if (strcmp(key,"noteMap")!=0) break;
        lua_pushnil(s);
        while (lua_next(s,-2)) {
          if (lua_isinteger(s,-2)) {
            if (lua_istable(s,-1)) {
              int map=lua_tointeger(s,-2);
              if (map>180 || map<1) {
                lua_pop(s,1);
                continue;
              }
              lua_pushnil(s);
              while (lua_next(s,-2)) {
                if (lua_isstring(s,-2)) {
                  const char* subkey=lua_tostring(s,-2);
                  switch (lua_type(s,-1)) {
                    case LUA_TBOOLEAN:
                      value=lua_toboolean(s,-1);
                      break;
                    case LUA_TNUMBER:
                      value=lua_tointeger(s,-1);
                      break;
                    default: 
                      lua_pop(s,1);
                      continue;
                  }
                  if (strcmp(subkey,"freq")==0) {
                    ins->amiga.noteMap[map-1].freq=value;
                  }
                  if (strcmp(subkey,"map")==0) {
                    ins->amiga.noteMap[map-1].map=value;
                  }
                  if (strcmp(subkey,"dpcmFreq")==0) {
                    ins->amiga.noteMap[map-1].dpcmFreq=value;
                  }
                  if (strcmp(subkey,"dpcmDelta")==0) {
                    ins->amiga.noteMap[map-1].dpcmDelta=value;
                  }
                }
                lua_pop(s,1);
              }
            }
          }
          lua_pop(s,1);
        }
        lua_pop(s,1);
        continue;
      }
    }
    CHECK_VALUE("initSample",amiga.initSample,0,65535)
    CHECK_VALUE("useNoteMap",amiga.useNoteMap,0,1)
    CHECK_VALUE("useSample",amiga.useSample,0,1)
    CHECK_VALUE("useWave",amiga.useWave,0,1)
    CHECK_VALUE("waveLen",amiga.waveLen,0,255)
    lua_pop(s,1);
  }
}

void readFeatureX1(DivInstrument* ins, lua_State* s, int t) {
  lua_pushnil(s);
  while (lua_next(s,t)) {
    if (!lua_isstring(s,-2)) continue;
    const char* key=lua_tostring(s,-2);
    int value;
    switch (lua_type(s,-1)) {
      case LUA_TBOOLEAN:
        value=lua_toboolean(s,-1);
        break;
      case LUA_TNUMBER:
        value=lua_tointeger(s,-1);
        break;
      default: break;
    }
    CHECK_VALUE("bankSlot",x1_010.bankSlot,0,65535)
    lua_pop(s,1);
  }
}

void readFeatureN163(DivInstrument* ins, lua_State* s, int t) {
  lua_pushnil(s);
  while (lua_next(s,t)) {
    if (!lua_isstring(s,-2)) continue;
    const char* key=lua_tostring(s,-2);
    int value;
    switch (lua_type(s,-1)) {
      case LUA_TBOOLEAN:
        value=lua_toboolean(s,-1);
        break;
      case LUA_TNUMBER:
        value=lua_tointeger(s,-1);
        break;
      case LUA_TTABLE: {
        if (strcmp(key,"wavePerChan")!=0) break;
        lua_pushnil(s);
        while (lua_next(s,-2)) {
          if (lua_isinteger(s,-2)) {
            if (lua_istable(s,-1)) {
              int chan=lua_tointeger(s,-2);
              if (chan>8 || chan<1) {
                lua_pop(s,1);
                continue;
              }
              lua_pushnil(s);
              while (lua_next(s,-2)) {
                if (lua_isstring(s,-2)) {
                  const char* subkey=lua_tostring(s,-2);
                  switch (lua_type(s,-1)) {
                    case LUA_TBOOLEAN:
                      value=lua_toboolean(s,-1);
                      break;
                    case LUA_TNUMBER:
                      value=lua_tointeger(s,-1);
                      break;
                    default: 
                      lua_pop(s,1);
                      continue;
                  }
                  if (strcmp(subkey,"pos")==0) {
                    ins->n163.wavePosCh[chan-1]=value;
                  }
                  if (strcmp(subkey,"map")==0) {
                    ins->n163.waveLenCh[chan-1]=value;
                  }
                }
                lua_pop(s,1);
              }
            }
          }
          lua_pop(s,1);
        }
        lua_pop(s,1);
        continue;
      }
    }
    CHECK_VALUE("wave",n163.wave,0,65535)
    CHECK_VALUE("wavePos",n163.wavePos,0,1)
    CHECK_VALUE("waveLen",n163.waveLen,0,1)
    CHECK_VALUE("waveMode",n163.waveMode,0,1)
    CHECK_VALUE("perChanPos",n163.perChanPos,0,1)
    lua_pop(s,1);
  }
}

void readFeatureFDS(DivInstrument* ins, lua_State* s, int t) {
  lua_pushnil(s);
  while (lua_next(s,t)) {
    if (!lua_isstring(s,-2)) continue;
    const char* key=lua_tostring(s,-2);
    int value;
    switch (lua_type(s,-1)) {
      case LUA_TBOOLEAN:
        value=lua_toboolean(s,-1);
        break;
      case LUA_TNUMBER:
        value=lua_tointeger(s,-1);
        break;
      case LUA_TTABLE: { // noteMap table
        if (strcmp(key,"modTable")!=0) break;
        lua_pushnil(s);
        while (lua_next(s,-2)) {
          if (lua_isinteger(s,-2) && lua_isinteger(s,-1)) {
            int i=lua_tointeger(s,-2);
            if (i>32 || i<0) {
              lua_pop(s,1);
              continue;
            }
            value=lua_tointeger(s,-1);
            ins->fds.modTable[i]=value;
          }
          lua_pop(s,1);
        }
        lua_pop(s,1);
        continue;
      }
    }
    CHECK_VALUE("modSpeed",fds.modSpeed,0,65535)
    CHECK_VALUE("modDepth",fds.modDepth,0,1)
    CHECK_VALUE("initModTableWithFirstWave",fds.initModTableWithFirstWave,0,1)
    lua_pop(s,1);
  }
}

void readFeatureMultiPCM(DivInstrument* ins, lua_State* s, int t) {
  lua_pushnil(s);
  while (lua_next(s,t)) {
    if (!lua_isstring(s,-2)) continue;
    const char* key=lua_tostring(s,-2);
    int value;
    switch (lua_type(s,-1)) {
      case LUA_TBOOLEAN:
        value=lua_toboolean(s,-1);
        break;
      case LUA_TNUMBER:
        value=lua_tointeger(s,-1);
        break;
      case LUA_TTABLE: {
        if (strcmp(key,"envelope")!=0) {
          break;
        }
        lua_pushnil(s);
        while (lua_next(s,-2)) {
          if (!lua_isstring(s,-2)) continue;
          const char* subkey=lua_tostring(s,-2);
          switch (lua_type(s,-1)) {
            case LUA_TBOOLEAN:
              value=lua_toboolean(s,-1);
              break;
            case LUA_TNUMBER:
              value=lua_tointeger(s,-1);
              break;
          }
          CHECK_SUBVALUE("ar",multipcm.ar,0,0)
          CHECK_SUBVALUE("d1r",multipcm.d1r,0,0)
          CHECK_SUBVALUE("d2r",multipcm.d2r,0,0)
          CHECK_SUBVALUE("dl",multipcm.dl,0,0)
          CHECK_SUBVALUE("rr",multipcm.rr,0,0)
          CHECK_SUBVALUE("rc",multipcm.rc,0,0)
          lua_pop(s,1);
        }
        lua_pop(s,1);
        continue;
      }
    }
    CHECK_VALUE("lfo",multipcm.lfo,0,1)
    CHECK_VALUE("vib",multipcm.vib,0,1)
    CHECK_VALUE("am",multipcm.am,0,1)
    CHECK_VALUE("damp",multipcm.damp,0,1)
    CHECK_VALUE("pseudoReverb",multipcm.pseudoReverb,0,1)
    CHECK_VALUE("lfoReset",multipcm.lfoReset,0,1)
    CHECK_VALUE("levelDirect",multipcm.levelDirect,0,1)
    lua_pop(s,1);
  }
}

void readFeatureWaveSynth(DivInstrument* ins, lua_State* s, int t) {
  lua_pushnil(s);
  while (lua_next(s,t)) {
    if (!lua_isstring(s,-2)) continue;
    const char* key=lua_tostring(s,-2);
    int value;
    switch (lua_type(s,-1)) {
      case LUA_TBOOLEAN:
        value=lua_toboolean(s,-1);
        break;
      case LUA_TNUMBER:
        value=lua_tointeger(s,-1);
        break;
      default: {
        lua_pop(s,1);
        continue;
      }
    }
    CHECK_VALUE("wave1",ws.wave1,0,1)
    CHECK_VALUE("wave2",ws.wave2,0,1)
    CHECK_VALUE("rateDivider",ws.rateDivider,0,1)
    CHECK_VALUE("effect",ws.effect,0,255)
    CHECK_VALUE("oneShot",ws.oneShot,0,1)
    CHECK_VALUE("enabled",ws.enabled,0,1)
    CHECK_VALUE("global",ws.global,0,1)
    CHECK_VALUE("speed",ws.speed,0,1)
    CHECK_VALUE("param1",ws.param1,0,255)
    CHECK_VALUE("param2",ws.param2,0,255)
    CHECK_VALUE("param3",ws.param3,0,255)
    CHECK_VALUE("param4",ws.param4,0,255)
    lua_pop(s,1);
  }
}

void readFeatureSoundUnit(DivInstrument* ins, lua_State* s, int t) {
  
}

void readFeatureES5506(DivInstrument* ins, lua_State* s, int t) {
  
}

void readFeatureSNES(DivInstrument* ins, lua_State* s, int t) {
  
}

void readFeatureESFM(DivInstrument* ins, lua_State* s, int t) {
  
}

void readFeaturePowerNoise(DivInstrument* ins, lua_State* s, int t) {
  
}

void readFeatureSID2(DivInstrument* ins, lua_State* s, int t) {
  
}

void readFeatureSID3(DivInstrument* ins, lua_State* s, int t) {
  
}

void readFeatureKlattsch(DivInstrument* ins, lua_State* s, int t) {
  
}

