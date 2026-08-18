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
  if (ins->fm==defaultIns.fm) return;
  lua_pushstring(s,"fm");
  lua_newtable(s);
  API_ADD_VALUE("alg",ins->fm.alg,integer)
  API_ADD_VALUE("feedback",ins->fm.fb,integer)
  API_ADD_VALUE("fms",ins->fm.fms,integer)
  API_ADD_VALUE("ams",ins->fm.ams,integer)
  API_ADD_VALUE("fms2",ins->fm.fms2,integer)
  API_ADD_VALUE("ams2",ins->fm.ams2,integer)
  API_ADD_VALUE("ops",ins->fm.ops,integer)
  API_ADD_VALUE("opllPreset",ins->fm.opllPreset,integer)
  API_ADD_VALUE("block",ins->fm.block,integer)
  API_ADD_VALUE("fixedDrums",ins->fm.fixedDrums,boolean)
  API_ADD_VALUE("kickFreq",ins->fm.kickFreq,integer)
  API_ADD_VALUE("snareHatFreq",ins->fm.snareHatFreq,integer)
  API_ADD_VALUE("tomTopFreq",ins->fm.tomTopFreq,integer)
  // operators
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
  lua_settable(s,-3);
}

// TODO!!!
void writeFeatureGB(DivInstrument* ins, lua_State* s) {
  if (ins->gb==defaultIns.gb) return;
}
void writeFeatureC64(DivInstrument* ins, lua_State* s) {
  if (ins->c64==defaultIns.c64) return;
}
void writeFeatureAmiga(DivInstrument* ins, lua_State* s) {
  if (ins->amiga==defaultIns.amiga) return;
}
void writeFeatureX1(DivInstrument* ins, lua_State* s) {
  if (ins->x1_010==defaultIns.x1_010) return;
}
void writeFeatureN163(DivInstrument* ins, lua_State* s) {
  if (ins->n163==defaultIns.n163) return;
}
void writeFeatureFDS(DivInstrument* ins, lua_State* s) {
  if (ins->fds==defaultIns.fds) return;
}
void writeFeatureMultiPCM(DivInstrument* ins, lua_State* s) {
  if (ins->multipcm==defaultIns.multipcm) return;
}
void writeFeatureWaveSynth(DivInstrument* ins, lua_State* s) {
  if (ins->ws==defaultIns.ws) return;
}
void writeFeatureSoundUnit(DivInstrument* ins, lua_State* s) {
  if (ins->su==defaultIns.su) return;
}
void writeFeatureES5506(DivInstrument* ins, lua_State* s) {
  if (ins->es5506==defaultIns.es5506) return;
}
void writeFeatureSNES(DivInstrument* ins, lua_State* s) {
  if (ins->snes==defaultIns.snes) return;
}
void writeFeatureESFM(DivInstrument* ins, lua_State* s) {
  if (ins->esfm==defaultIns.esfm) return;
}
void writeFeaturePowerNoise(DivInstrument* ins, lua_State* s) {
  if (ins->powernoise==defaultIns.powernoise) return;
}
void writeFeatureSID2(DivInstrument* ins, lua_State* s) {
  if (ins->sid2==defaultIns.sid2) return;
}
void writeFeatureSID3(DivInstrument* ins, lua_State* s) {
  if (ins->sid3==defaultIns.sid3) return;
}
void writeFeatureKlattsch(DivInstrument* ins, lua_State* s) {
  if (ins->klattsch==defaultIns.klattsch) return;
}
