/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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

#include "dataErrors.h"
#include "engine.h"
#include "instrument.h"
#include "../ta-log.h"
#include "../fileutils.h"

void DivInstrument::putInsData(SafeWriter* w) {
  size_t blockStartSeek, blockEndSeek;

  w->write("INST",4);
  blockStartSeek=w->tell();
  w->writeI(0);

  w->writeS(DIV_ENGINE_VERSION);

  w->writeC(type);
  w->writeC(0);

  w->writeString(name,false);

  // FM
  w->writeC(fm.alg);
  w->writeC(fm.fb);
  w->writeC(fm.fms);
  w->writeC(fm.ams);
  w->writeC(fm.ops);
  w->writeC(fm.opllPreset);
  w->writeC(0); // reserved
  w->writeC(0);

  for (int j=0; j<4; j++) {
    DivInstrumentFM::Operator& op=fm.op[j];
    w->writeC(op.am);
    w->writeC(op.ar);
    w->writeC(op.dr);
    w->writeC(op.mult);
    w->writeC(op.rr);
    w->writeC(op.sl);
    w->writeC(op.tl);
    w->writeC(op.dt2);
    w->writeC(op.rs);
    w->writeC(op.dt);
    w->writeC(op.d2r);
    w->writeC(op.ssgEnv);

    w->writeC(op.dam);
    w->writeC(op.dvb);
    w->writeC(op.egt);
    w->writeC(op.ksl);
    w->writeC(op.sus);
    w->writeC(op.vib);
    w->writeC(op.ws);
    w->writeC(op.ksr);

    // reserved
    for (int k=0; k<12; k++) {
      w->writeC(0);
    }
  }

  // GB
  w->writeC(gb.envVol);
  w->writeC(gb.envDir);
  w->writeC(gb.envLen);
  w->writeC(gb.soundLen);

  // C64
  w->writeC(c64.triOn);
  w->writeC(c64.sawOn);
  w->writeC(c64.pulseOn);
  w->writeC(c64.noiseOn);
  w->writeC(c64.a);
  w->writeC(c64.d);
  w->writeC(c64.s);
  w->writeC(c64.r);
  w->writeS(c64.duty);
  w->writeC(c64.ringMod);
  w->writeC(c64.oscSync);
  w->writeC(c64.toFilter);
  w->writeC(c64.initFilter);
  w->writeC(c64.volIsCutoff);
  w->writeC(c64.res);
  w->writeC(c64.lp);
  w->writeC(c64.bp);
  w->writeC(c64.hp);
  w->writeC(c64.ch3off);
  w->writeS(c64.cut);
  w->writeC(c64.dutyIsAbs);
  w->writeC(c64.filterIsAbs);
  
  // Amiga
  w->writeS(amiga.initSample);
  w->writeC(amiga.useWave);
  w->writeC(amiga.waveLen);
  for (int j=0; j<12; j++) { // reserved
    w->writeC(0);
  }

  // standard
  w->writeI(std.volMacro.len);
  w->writeI(std.arpMacro.len);
  w->writeI(std.dutyMacro.len);
  w->writeI(std.waveMacro.len);
  w->writeI(std.pitchMacro.len);
  w->writeI(std.ex1Macro.len);
  w->writeI(std.ex2Macro.len);
  w->writeI(std.ex3Macro.len);
  w->writeI(std.volMacro.loop);
  w->writeI(std.arpMacro.loop);
  w->writeI(std.dutyMacro.loop);
  w->writeI(std.waveMacro.loop);
  w->writeI(std.pitchMacro.loop);
  w->writeI(std.ex1Macro.loop);
  w->writeI(std.ex2Macro.loop);
  w->writeI(std.ex3Macro.loop);
  w->writeC(std.arpMacro.mode);
  w->writeC(0); // reserved
  w->writeC(0);
  w->writeC(0);
  for (int j=0; j<std.volMacro.len; j++) {
    w->writeI(std.volMacro.val[j]);
  }
  for (int j=0; j<std.arpMacro.len; j++) {
    w->writeI(std.arpMacro.val[j]);
  }
  for (int j=0; j<std.dutyMacro.len; j++) {
    w->writeI(std.dutyMacro.val[j]);
  }
  for (int j=0; j<std.waveMacro.len; j++) {
    w->writeI(std.waveMacro.val[j]);
  }
  for (int j=0; j<std.pitchMacro.len; j++) {
    w->writeI(std.pitchMacro.val[j]);
  }
  for (int j=0; j<std.ex1Macro.len; j++) {
    w->writeI(std.ex1Macro.val[j]);
  }
  for (int j=0; j<std.ex2Macro.len; j++) {
    w->writeI(std.ex2Macro.val[j]);
  }
  for (int j=0; j<std.ex3Macro.len; j++) {
    w->writeI(std.ex3Macro.val[j]);
  }

  // FM macros and open status
  w->writeI(std.algMacro.len);
  w->writeI(std.fbMacro.len);
  w->writeI(std.fmsMacro.len);
  w->writeI(std.amsMacro.len);
  w->writeI(std.algMacro.loop);
  w->writeI(std.fbMacro.loop);
  w->writeI(std.fmsMacro.loop);
  w->writeI(std.amsMacro.loop);

  w->writeC(std.volMacro.open);
  w->writeC(std.arpMacro.open);
  w->writeC(std.dutyMacro.open);
  w->writeC(std.waveMacro.open);
  w->writeC(std.pitchMacro.open);
  w->writeC(std.ex1Macro.open);
  w->writeC(std.ex2Macro.open);
  w->writeC(std.ex3Macro.open);
  w->writeC(std.algMacro.open);
  w->writeC(std.fbMacro.open);
  w->writeC(std.fmsMacro.open);
  w->writeC(std.amsMacro.open);

  for (int j=0; j<std.algMacro.len; j++) {
    w->writeI(std.algMacro.val[j]);
  }
  for (int j=0; j<std.fbMacro.len; j++) {
    w->writeI(std.fbMacro.val[j]);
  }
  for (int j=0; j<std.fmsMacro.len; j++) {
    w->writeI(std.fmsMacro.val[j]);
  }
  for (int j=0; j<std.amsMacro.len; j++) {
    w->writeI(std.amsMacro.val[j]);
  }

  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& op=std.opMacros[i];

    w->writeI(op.amMacro.len);
    w->writeI(op.arMacro.len);
    w->writeI(op.drMacro.len);
    w->writeI(op.multMacro.len);
    w->writeI(op.rrMacro.len);
    w->writeI(op.slMacro.len);
    w->writeI(op.tlMacro.len);
    w->writeI(op.dt2Macro.len);
    w->writeI(op.rsMacro.len);
    w->writeI(op.dtMacro.len);
    w->writeI(op.d2rMacro.len);
    w->writeI(op.ssgMacro.len);
    w->writeI(op.amMacro.loop);
    w->writeI(op.arMacro.loop);
    w->writeI(op.drMacro.loop);
    w->writeI(op.multMacro.loop);
    w->writeI(op.rrMacro.loop);
    w->writeI(op.slMacro.loop);
    w->writeI(op.tlMacro.loop);
    w->writeI(op.dt2Macro.loop);
    w->writeI(op.rsMacro.loop);
    w->writeI(op.dtMacro.loop);
    w->writeI(op.d2rMacro.loop);
    w->writeI(op.ssgMacro.loop);
    w->writeC(op.amMacro.open);
    w->writeC(op.arMacro.open);
    w->writeC(op.drMacro.open);
    w->writeC(op.multMacro.open);
    w->writeC(op.rrMacro.open);
    w->writeC(op.slMacro.open);
    w->writeC(op.tlMacro.open);
    w->writeC(op.dt2Macro.open);
    w->writeC(op.rsMacro.open);
    w->writeC(op.dtMacro.open);
    w->writeC(op.d2rMacro.open);
    w->writeC(op.ssgMacro.open);
  }

  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& op=std.opMacros[i];
    for (int j=0; j<op.amMacro.len; j++) {
      w->writeC(op.amMacro.val[j]&0xff);
    }
    for (int j=0; j<op.arMacro.len; j++) {
      w->writeC(op.arMacro.val[j]&0xff);
    }
    for (int j=0; j<op.drMacro.len; j++) {
      w->writeC(op.drMacro.val[j]&0xff);
    }
    for (int j=0; j<op.multMacro.len; j++) {
      w->writeC(op.multMacro.val[j]&0xff);
    }
    for (int j=0; j<op.rrMacro.len; j++) {
      w->writeC(op.rrMacro.val[j]&0xff);
    }
    for (int j=0; j<op.slMacro.len; j++) {
      w->writeC(op.slMacro.val[j]&0xff);
    }
    for (int j=0; j<op.tlMacro.len; j++) {
      w->writeC(op.tlMacro.val[j]&0xff);
    }
    for (int j=0; j<op.dt2Macro.len; j++) {
      w->writeC(op.dt2Macro.val[j]&0xff);
    }
    for (int j=0; j<op.rsMacro.len; j++) {
      w->writeC(op.rsMacro.val[j]&0xff);
    }
    for (int j=0; j<op.dtMacro.len; j++) {
      w->writeC(op.dtMacro.val[j]&0xff);
    }
    for (int j=0; j<op.d2rMacro.len; j++) {
      w->writeC(op.d2rMacro.val[j]&0xff);
    }
    for (int j=0; j<op.ssgMacro.len; j++) {
      w->writeC(op.ssgMacro.val[j]&0xff);
    }
  }

  // release points
  w->writeI(std.volMacro.rel);
  w->writeI(std.arpMacro.rel);
  w->writeI(std.dutyMacro.rel);
  w->writeI(std.waveMacro.rel);
  w->writeI(std.pitchMacro.rel);
  w->writeI(std.ex1Macro.rel);
  w->writeI(std.ex2Macro.rel);
  w->writeI(std.ex3Macro.rel);
  w->writeI(std.algMacro.rel);
  w->writeI(std.fbMacro.rel);
  w->writeI(std.fmsMacro.rel);
  w->writeI(std.amsMacro.rel);
  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& op=std.opMacros[i];

    w->writeI(op.amMacro.rel);
    w->writeI(op.arMacro.rel);
    w->writeI(op.drMacro.rel);
    w->writeI(op.multMacro.rel);
    w->writeI(op.rrMacro.rel);
    w->writeI(op.slMacro.rel);
    w->writeI(op.tlMacro.rel);
    w->writeI(op.dt2Macro.rel);
    w->writeI(op.rsMacro.rel);
    w->writeI(op.dtMacro.rel);
    w->writeI(op.d2rMacro.rel);
    w->writeI(op.ssgMacro.rel);
  }

  // extended op macros
  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& op=std.opMacros[i];

    w->writeI(op.damMacro.len);
    w->writeI(op.dvbMacro.len);
    w->writeI(op.egtMacro.len);
    w->writeI(op.kslMacro.len);
    w->writeI(op.susMacro.len);
    w->writeI(op.vibMacro.len);
    w->writeI(op.wsMacro.len);
    w->writeI(op.ksrMacro.len);
    
    w->writeI(op.damMacro.loop);
    w->writeI(op.dvbMacro.loop);
    w->writeI(op.egtMacro.loop);
    w->writeI(op.kslMacro.loop);
    w->writeI(op.susMacro.loop);
    w->writeI(op.vibMacro.loop);
    w->writeI(op.wsMacro.loop);
    w->writeI(op.ksrMacro.loop);

    w->writeI(op.damMacro.rel);
    w->writeI(op.dvbMacro.rel);
    w->writeI(op.egtMacro.rel);
    w->writeI(op.kslMacro.rel);
    w->writeI(op.susMacro.rel);
    w->writeI(op.vibMacro.rel);
    w->writeI(op.wsMacro.rel);
    w->writeI(op.ksrMacro.rel);

    w->writeC(op.damMacro.open);
    w->writeC(op.dvbMacro.open);
    w->writeC(op.egtMacro.open);
    w->writeC(op.kslMacro.open);
    w->writeC(op.susMacro.open);
    w->writeC(op.vibMacro.open);
    w->writeC(op.wsMacro.open);
    w->writeC(op.ksrMacro.open);
  }

  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& op=std.opMacros[i];
    for (int j=0; j<op.damMacro.len; j++) {
      w->writeC(op.damMacro.val[j]);
    }
    for (int j=0; j<op.dvbMacro.len; j++) {
      w->writeC(op.dvbMacro.val[j]);
    }
    for (int j=0; j<op.egtMacro.len; j++) {
      w->writeC(op.egtMacro.val[j]);
    }
    for (int j=0; j<op.kslMacro.len; j++) {
      w->writeC(op.kslMacro.val[j]);
    }
    for (int j=0; j<op.susMacro.len; j++) {
      w->writeC(op.susMacro.val[j]);
    }
    for (int j=0; j<op.vibMacro.len; j++) {
      w->writeC(op.vibMacro.val[j]);
    }
    for (int j=0; j<op.wsMacro.len; j++) {
      w->writeC(op.wsMacro.val[j]);
    }
    for (int j=0; j<op.ksrMacro.len; j++) {
      w->writeC(op.ksrMacro.val[j]);
    }
  }

  // OPL drum data
  w->writeC(fm.fixedDrums);
  w->writeC(0); // reserved
  w->writeS(fm.kickFreq);
  w->writeS(fm.snareHatFreq);
  w->writeS(fm.tomTopFreq);

  // sample map
  w->writeC(amiga.useNoteMap);
  if (amiga.useNoteMap) {
    for (int note=0; note<120; note++) {
      w->writeI(amiga.noteMap[note].freq);
    }
    for (int note=0; note<120; note++) {
      w->writeS(amiga.noteMap[note].map);
    }
  }

  // N163
  w->writeI(n163.wave);
  w->writeC(n163.wavePos);
  w->writeC(n163.waveLen);
  w->writeC(n163.waveMode);
  w->writeC(0); // reserved

  // more macros
  w->writeI(std.panLMacro.len);
  w->writeI(std.panRMacro.len);
  w->writeI(std.phaseResetMacro.len);
  w->writeI(std.ex4Macro.len);
  w->writeI(std.ex5Macro.len);
  w->writeI(std.ex6Macro.len);
  w->writeI(std.ex7Macro.len);
  w->writeI(std.ex8Macro.len);
  
  w->writeI(std.panLMacro.loop);
  w->writeI(std.panRMacro.loop);
  w->writeI(std.phaseResetMacro.loop);
  w->writeI(std.ex4Macro.loop);
  w->writeI(std.ex5Macro.loop);
  w->writeI(std.ex6Macro.loop);
  w->writeI(std.ex7Macro.loop);
  w->writeI(std.ex8Macro.loop);

  w->writeI(std.panLMacro.rel);
  w->writeI(std.panRMacro.rel);
  w->writeI(std.phaseResetMacro.rel);
  w->writeI(std.ex4Macro.rel);
  w->writeI(std.ex5Macro.rel);
  w->writeI(std.ex6Macro.rel);
  w->writeI(std.ex7Macro.rel);
  w->writeI(std.ex8Macro.rel);

  w->writeC(std.panLMacro.open);
  w->writeC(std.panRMacro.open);
  w->writeC(std.phaseResetMacro.open);
  w->writeC(std.ex4Macro.open);
  w->writeC(std.ex5Macro.open);
  w->writeC(std.ex6Macro.open);
  w->writeC(std.ex7Macro.open);
  w->writeC(std.ex8Macro.open);

  for (int j=0; j<std.panLMacro.len; j++) {
    w->writeI(std.panLMacro.val[j]);
  }
  for (int j=0; j<std.panRMacro.len; j++) {
    w->writeI(std.panRMacro.val[j]);
  }
  for (int j=0; j<std.phaseResetMacro.len; j++) {
    w->writeI(std.phaseResetMacro.val[j]);
  }
  for (int j=0; j<std.ex4Macro.len; j++) {
    w->writeI(std.ex4Macro.val[j]);
  }
  for (int j=0; j<std.ex5Macro.len; j++) {
    w->writeI(std.ex5Macro.val[j]);
  }
  for (int j=0; j<std.ex6Macro.len; j++) {
    w->writeI(std.ex6Macro.val[j]);
  }
  for (int j=0; j<std.ex7Macro.len; j++) {
    w->writeI(std.ex7Macro.val[j]);
  }
  for (int j=0; j<std.ex8Macro.len; j++) {
    w->writeI(std.ex8Macro.val[j]);
  }

  // FDS
  w->writeI(fds.modSpeed);
  w->writeI(fds.modDepth);
  w->writeC(fds.initModTableWithFirstWave);
  w->writeC(0); // reserved
  w->writeC(0);
  w->writeC(0);
  w->write(fds.modTable,32);

  // OPZ
  w->writeC(fm.fms2);
  w->writeC(fm.ams2);
  
  // wave synth
  w->writeI(ws.wave1);
  w->writeI(ws.wave2);
  w->writeC(ws.rateDivider);
  w->writeC(ws.effect);
  w->writeC(ws.enabled);
  w->writeC(ws.global);
  w->writeC(ws.speed);
  w->writeC(ws.param1);
  w->writeC(ws.param2);
  w->writeC(ws.param3);
  w->writeC(ws.param4);

  // other macro modes
  w->writeC(std.volMacro.mode);
  w->writeC(std.dutyMacro.mode);
  w->writeC(std.waveMacro.mode);
  w->writeC(std.pitchMacro.mode);
  w->writeC(std.ex1Macro.mode);
  w->writeC(std.ex2Macro.mode);
  w->writeC(std.ex3Macro.mode);
  w->writeC(std.algMacro.mode);
  w->writeC(std.fbMacro.mode);
  w->writeC(std.fmsMacro.mode);
  w->writeC(std.amsMacro.mode);
  w->writeC(std.panLMacro.mode);
  w->writeC(std.panRMacro.mode);
  w->writeC(std.phaseResetMacro.mode);
  w->writeC(std.ex4Macro.mode);
  w->writeC(std.ex5Macro.mode);
  w->writeC(std.ex6Macro.mode);
  w->writeC(std.ex7Macro.mode);
  w->writeC(std.ex8Macro.mode);

  // C64 no test
  w->writeC(c64.noTest);

  // MultiPCM
  w->writeC(multipcm.ar);
  w->writeC(multipcm.d1r);
  w->writeC(multipcm.dl);
  w->writeC(multipcm.d2r);
  w->writeC(multipcm.rr);
  w->writeC(multipcm.rc);
  w->writeC(multipcm.lfo);
  w->writeC(multipcm.vib);
  w->writeC(multipcm.am);
  for (int j=0; j<23; j++) { // reserved
    w->writeC(0);
  }

  blockEndSeek=w->tell();
  w->seek(blockStartSeek,SEEK_SET);
  w->writeI(blockEndSeek-blockStartSeek-4);
  w->seek(0,SEEK_END);
}

DivDataErrors DivInstrument::readInsData(SafeReader& reader, short version) {
  char magic[4];
  reader.read(magic,4);
  if (memcmp(magic,"INST",4)!=0) {
    logE("invalid instrument header!");
    return DIV_DATA_INVALID_HEADER;
  }
  reader.readI();

  reader.readS(); // format version. ignored.
  type=(DivInstrumentType)reader.readC();
  mode=(type==DIV_INS_FM);
  reader.readC();
  name=reader.readString();

  // FM
  fm.alg=reader.readC();
  fm.fb=reader.readC();
  fm.fms=reader.readC();
  fm.ams=reader.readC();
  fm.ops=reader.readC();
  if (version>=60) {
    fm.opllPreset=reader.readC();
  } else {
    reader.readC();
  }
  reader.readC();
  reader.readC();

  for (int j=0; j<4; j++) {
    DivInstrumentFM::Operator& op=fm.op[j];
    op.am=reader.readC();
    op.ar=reader.readC();
    op.dr=reader.readC();
    op.mult=reader.readC();
    op.rr=reader.readC();
    op.sl=reader.readC();
    op.tl=reader.readC();
    op.dt2=reader.readC();
    op.rs=reader.readC();
    op.dt=reader.readC();
    op.d2r=reader.readC();
    op.ssgEnv=reader.readC();

    op.dam=reader.readC();
    op.dvb=reader.readC();
    op.egt=reader.readC();
    op.ksl=reader.readC();
    op.sus=reader.readC();
    op.vib=reader.readC();
    op.ws=reader.readC();
    op.ksr=reader.readC();

    // reserved
    for (int k=0; k<12; k++) reader.readC();
  }

  // GB
  gb.envVol=reader.readC();
  gb.envDir=reader.readC();
  gb.envLen=reader.readC();
  gb.soundLen=reader.readC();

  // C64
  c64.triOn=reader.readC();
  c64.sawOn=reader.readC();
  c64.pulseOn=reader.readC();
  c64.noiseOn=reader.readC();
  c64.a=reader.readC();
  c64.d=reader.readC();
  c64.s=reader.readC();
  c64.r=reader.readC();
  c64.duty=reader.readS();
  c64.ringMod=reader.readC();
  c64.oscSync=reader.readC();
  c64.toFilter=reader.readC();
  c64.initFilter=reader.readC();
  c64.volIsCutoff=reader.readC();
  c64.res=reader.readC();
  c64.lp=reader.readC();
  c64.bp=reader.readC();
  c64.hp=reader.readC();
  c64.ch3off=reader.readC();
  c64.cut=reader.readS();
  c64.dutyIsAbs=reader.readC();
  c64.filterIsAbs=reader.readC();

  // Amiga
  amiga.initSample=reader.readS();
  if (version>=82) {
    amiga.useWave=reader.readC();
    amiga.waveLen=(unsigned char)reader.readC();
  } else {
    reader.readC();
    reader.readC();
  }
  // reserved
  for (int k=0; k<12; k++) reader.readC();

  // standard
  std.volMacro.len=reader.readI();
  std.arpMacro.len=reader.readI();
  std.dutyMacro.len=reader.readI();
  std.waveMacro.len=reader.readI();
  if (version>=17) {
    std.pitchMacro.len=reader.readI();
    std.ex1Macro.len=reader.readI();
    std.ex2Macro.len=reader.readI();
    std.ex3Macro.len=reader.readI();
  }
  std.volMacro.loop=reader.readI();
  std.arpMacro.loop=reader.readI();
  std.dutyMacro.loop=reader.readI();
  std.waveMacro.loop=reader.readI();
  if (version>=17) {
    std.pitchMacro.loop=reader.readI();
    std.ex1Macro.loop=reader.readI();
    std.ex2Macro.loop=reader.readI();
    std.ex3Macro.loop=reader.readI();
  }
  std.arpMacro.mode=reader.readC();
  // these 3 were macro heights before but they are not used anymore
  int oldVolHeight=reader.readC();
  int oldDutyHeight=reader.readC();
  reader.readC(); // oldWaveHeight
  reader.read(std.volMacro.val,4*std.volMacro.len);
  reader.read(std.arpMacro.val,4*std.arpMacro.len);
  reader.read(std.dutyMacro.val,4*std.dutyMacro.len);
  reader.read(std.waveMacro.val,4*std.waveMacro.len);
  if (version<31) {
    if (!std.arpMacro.mode) for (int j=0; j<std.arpMacro.len; j++) {
      std.arpMacro.val[j]-=12;
    }
  }
  if (type==DIV_INS_C64 && version<87) {
    if (c64.volIsCutoff && !c64.filterIsAbs) for (int j=0; j<std.volMacro.len; j++) {
      std.volMacro.val[j]-=18;
    }
    if (!c64.dutyIsAbs) for (int j=0; j<std.dutyMacro.len; j++) {
      std.dutyMacro.val[j]-=12;
    }
  }
  if (version>=17) {
    reader.read(std.pitchMacro.val,4*std.pitchMacro.len);
    reader.read(std.ex1Macro.val,4*std.ex1Macro.len);
    reader.read(std.ex2Macro.val,4*std.ex2Macro.len);
    reader.read(std.ex3Macro.val,4*std.ex3Macro.len);
  } else {
    if (type==DIV_INS_STD) {
      if (oldVolHeight==31) {
        type=DIV_INS_PCE;
      }
      if (oldDutyHeight==31) {
        type=DIV_INS_AY;
      }
    }
  }

  // FM macros
  if (version>=29) {
    std.algMacro.len=reader.readI();
    std.fbMacro.len=reader.readI();
    std.fmsMacro.len=reader.readI();
    std.amsMacro.len=reader.readI();
    std.algMacro.loop=reader.readI();
    std.fbMacro.loop=reader.readI();
    std.fmsMacro.loop=reader.readI();
    std.amsMacro.loop=reader.readI();
    std.volMacro.open=reader.readC();
    std.arpMacro.open=reader.readC();
    std.dutyMacro.open=reader.readC();
    std.waveMacro.open=reader.readC();
    std.pitchMacro.open=reader.readC();
    std.ex1Macro.open=reader.readC();
    std.ex2Macro.open=reader.readC();
    std.ex3Macro.open=reader.readC();
    std.algMacro.open=reader.readC();
    std.fbMacro.open=reader.readC();
    std.fmsMacro.open=reader.readC();
    std.amsMacro.open=reader.readC();

    reader.read(std.algMacro.val,4*std.algMacro.len);
    reader.read(std.fbMacro.val,4*std.fbMacro.len);
    reader.read(std.fmsMacro.val,4*std.fmsMacro.len);
    reader.read(std.amsMacro.val,4*std.amsMacro.len);

    for (int i=0; i<4; i++) {
      DivInstrumentSTD::OpMacro& op=std.opMacros[i];

      op.amMacro.len=reader.readI();
      op.arMacro.len=reader.readI();
      op.drMacro.len=reader.readI();
      op.multMacro.len=reader.readI();
      op.rrMacro.len=reader.readI();
      op.slMacro.len=reader.readI();
      op.tlMacro.len=reader.readI();
      op.dt2Macro.len=reader.readI();
      op.rsMacro.len=reader.readI();
      op.dtMacro.len=reader.readI();
      op.d2rMacro.len=reader.readI();
      op.ssgMacro.len=reader.readI();

      op.amMacro.loop=reader.readI();
      op.arMacro.loop=reader.readI();
      op.drMacro.loop=reader.readI();
      op.multMacro.loop=reader.readI();
      op.rrMacro.loop=reader.readI();
      op.slMacro.loop=reader.readI();
      op.tlMacro.loop=reader.readI();
      op.dt2Macro.loop=reader.readI();
      op.rsMacro.loop=reader.readI();
      op.dtMacro.loop=reader.readI();
      op.d2rMacro.loop=reader.readI();
      op.ssgMacro.loop=reader.readI();

      op.amMacro.open=reader.readC();
      op.arMacro.open=reader.readC();
      op.drMacro.open=reader.readC();
      op.multMacro.open=reader.readC();
      op.rrMacro.open=reader.readC();
      op.slMacro.open=reader.readC();
      op.tlMacro.open=reader.readC();
      op.dt2Macro.open=reader.readC();
      op.rsMacro.open=reader.readC();
      op.dtMacro.open=reader.readC();
      op.d2rMacro.open=reader.readC();
      op.ssgMacro.open=reader.readC();
    }

    // FM macro low 8 bits
    for (int i=0; i<4; i++) {
      DivInstrumentSTD::OpMacro& op=std.opMacros[i];
      for (int j=0; j<op.amMacro.len; j++) {
        op.amMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.arMacro.len; j++) {
        op.arMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.drMacro.len; j++) {
        op.drMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.multMacro.len; j++) {
        op.multMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.rrMacro.len; j++) {
        op.rrMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.slMacro.len; j++) {
        op.slMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.tlMacro.len; j++) {
        op.tlMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.dt2Macro.len; j++) {
        op.dt2Macro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.rsMacro.len; j++) {
        op.rsMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.dtMacro.len; j++) {
        op.dtMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.d2rMacro.len; j++) {
        op.d2rMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.ssgMacro.len; j++) {
        op.ssgMacro.val[j]=(unsigned char)reader.readC();
      }
    }
  }

  // release points
  if (version>=44) {
    std.volMacro.rel=reader.readI();
    std.arpMacro.rel=reader.readI();
    std.dutyMacro.rel=reader.readI();
    std.waveMacro.rel=reader.readI();
    std.pitchMacro.rel=reader.readI();
    std.ex1Macro.rel=reader.readI();
    std.ex2Macro.rel=reader.readI();
    std.ex3Macro.rel=reader.readI();
    std.algMacro.rel=reader.readI();
    std.fbMacro.rel=reader.readI();
    std.fmsMacro.rel=reader.readI();
    std.amsMacro.rel=reader.readI();

    for (int i=0; i<4; i++) {
      DivInstrumentSTD::OpMacro& op=std.opMacros[i];

      op.amMacro.rel=reader.readI();
      op.arMacro.rel=reader.readI();
      op.drMacro.rel=reader.readI();
      op.multMacro.rel=reader.readI();
      op.rrMacro.rel=reader.readI();
      op.slMacro.rel=reader.readI();
      op.tlMacro.rel=reader.readI();
      op.dt2Macro.rel=reader.readI();
      op.rsMacro.rel=reader.readI();
      op.dtMacro.rel=reader.readI();
      op.d2rMacro.rel=reader.readI();
      op.ssgMacro.rel=reader.readI();
    }
  }

  // extended op macros
  if (version>=61) {
    for (int i=0; i<4; i++) {
      DivInstrumentSTD::OpMacro& op=std.opMacros[i];

      op.damMacro.len=reader.readI();
      op.dvbMacro.len=reader.readI();
      op.egtMacro.len=reader.readI();
      op.kslMacro.len=reader.readI();
      op.susMacro.len=reader.readI();
      op.vibMacro.len=reader.readI();
      op.wsMacro.len=reader.readI();
      op.ksrMacro.len=reader.readI();

      op.damMacro.loop=reader.readI();
      op.dvbMacro.loop=reader.readI();
      op.egtMacro.loop=reader.readI();
      op.kslMacro.loop=reader.readI();
      op.susMacro.loop=reader.readI();
      op.vibMacro.loop=reader.readI();
      op.wsMacro.loop=reader.readI();
      op.ksrMacro.loop=reader.readI();

      op.damMacro.rel=reader.readI();
      op.dvbMacro.rel=reader.readI();
      op.egtMacro.rel=reader.readI();
      op.kslMacro.rel=reader.readI();
      op.susMacro.rel=reader.readI();
      op.vibMacro.rel=reader.readI();
      op.wsMacro.rel=reader.readI();
      op.ksrMacro.rel=reader.readI();

      op.damMacro.open=reader.readC();
      op.dvbMacro.open=reader.readC();
      op.egtMacro.open=reader.readC();
      op.kslMacro.open=reader.readC();
      op.susMacro.open=reader.readC();
      op.vibMacro.open=reader.readC();
      op.wsMacro.open=reader.readC();
      op.ksrMacro.open=reader.readC();
    }

    for (int i=0; i<4; i++) {
      DivInstrumentSTD::OpMacro& op=std.opMacros[i];
      for (int j=0; j<op.damMacro.len; j++) {
        op.damMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.dvbMacro.len; j++) {
        op.dvbMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.egtMacro.len; j++) {
        op.egtMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.kslMacro.len; j++) {
        op.kslMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.susMacro.len; j++) {
        op.susMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.vibMacro.len; j++) {
        op.vibMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.wsMacro.len; j++) {
        op.wsMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.ksrMacro.len; j++) {
        op.ksrMacro.val[j]=(unsigned char)reader.readC();
      }
    }
  }

  // OPL drum data
  if (version>=63) {
    fm.fixedDrums=reader.readC();
    reader.readC(); // reserved
    fm.kickFreq=reader.readS();
    fm.snareHatFreq=reader.readS();
    fm.tomTopFreq=reader.readS();
  }

  // clear noise macro if PCE instrument and version<63
  if (version<63 && type==DIV_INS_PCE) {
    std.dutyMacro.len=0;
    std.dutyMacro.loop=-1;
    std.dutyMacro.rel=-1;
  }

  // clear wave macro if OPLL instrument and version<70
  if (version<70 && type==DIV_INS_OPLL) {
    std.waveMacro.len=0;
    std.waveMacro.loop=-1;
    std.waveMacro.rel=-1;
  }

  // sample map
  if (version>=67) {
    amiga.useNoteMap=reader.readC();
    if (amiga.useNoteMap) {
      for (int note=0; note<120; note++) {
        amiga.noteMap[note].freq=reader.readI();
      }
      for (int note=0; note<120; note++) {
        amiga.noteMap[note].map=reader.readS();
      }
    }
  }

  // N163
  if (version>=73) {
    n163.wave=reader.readI();
    n163.wavePos=(unsigned char)reader.readC();
    n163.waveLen=(unsigned char)reader.readC();
    n163.waveMode=(unsigned char)reader.readC();
    reader.readC(); // reserved
  }

  if (version>=76) {
    // more macros
    std.panLMacro.len=reader.readI();
    std.panRMacro.len=reader.readI();
    std.phaseResetMacro.len=reader.readI();
    std.ex4Macro.len=reader.readI();
    std.ex5Macro.len=reader.readI();
    std.ex6Macro.len=reader.readI();
    std.ex7Macro.len=reader.readI();
    std.ex8Macro.len=reader.readI();

    std.panLMacro.loop=reader.readI();
    std.panRMacro.loop=reader.readI();
    std.phaseResetMacro.loop=reader.readI();
    std.ex4Macro.loop=reader.readI();
    std.ex5Macro.loop=reader.readI();
    std.ex6Macro.loop=reader.readI();
    std.ex7Macro.loop=reader.readI();
    std.ex8Macro.loop=reader.readI();

    std.panLMacro.rel=reader.readI();
    std.panRMacro.rel=reader.readI();
    std.phaseResetMacro.rel=reader.readI();
    std.ex4Macro.rel=reader.readI();
    std.ex5Macro.rel=reader.readI();
    std.ex6Macro.rel=reader.readI();
    std.ex7Macro.rel=reader.readI();
    std.ex8Macro.rel=reader.readI();

    std.panLMacro.open=reader.readC();
    std.panRMacro.open=reader.readC();
    std.phaseResetMacro.open=reader.readC();
    std.ex4Macro.open=reader.readC();
    std.ex5Macro.open=reader.readC();
    std.ex6Macro.open=reader.readC();
    std.ex7Macro.open=reader.readC();
    std.ex8Macro.open=reader.readC();

    reader.read(std.panLMacro.val,4*std.panLMacro.len);
    reader.read(std.panRMacro.val,4*std.panRMacro.len);
    reader.read(std.phaseResetMacro.val,4*std.phaseResetMacro.len);
    reader.read(std.ex4Macro.val,4*std.ex4Macro.len);
    reader.read(std.ex5Macro.val,4*std.ex5Macro.len);
    reader.read(std.ex6Macro.val,4*std.ex6Macro.len);
    reader.read(std.ex7Macro.val,4*std.ex7Macro.len);
    reader.read(std.ex8Macro.val,4*std.ex8Macro.len);

    // FDS
    fds.modSpeed=reader.readI();
    fds.modDepth=reader.readI();
    fds.initModTableWithFirstWave=reader.readC();
    reader.readC(); // reserved
    reader.readC();
    reader.readC();
    reader.read(fds.modTable,32);
  }

  // OPZ
  if (version>=77) {
    fm.fms2=reader.readC();
    fm.ams2=reader.readC();
  }

  // wave synth
  if (version>=79) {
    ws.wave1=reader.readI();
    ws.wave2=reader.readI();
    ws.rateDivider=reader.readC();
    ws.effect=reader.readC();
    ws.enabled=reader.readC();
    ws.global=reader.readC();
    ws.speed=reader.readC();
    ws.param1=reader.readC();
    ws.param2=reader.readC();
    ws.param3=reader.readC();
    ws.param4=reader.readC();
  }

  // other macro modes
  if (version>=84) {
    std.volMacro.mode=reader.readC();
    std.dutyMacro.mode=reader.readC();
    std.waveMacro.mode=reader.readC();
    std.pitchMacro.mode=reader.readC();
    std.ex1Macro.mode=reader.readC();
    std.ex2Macro.mode=reader.readC();
    std.ex3Macro.mode=reader.readC();
    std.algMacro.mode=reader.readC();
    std.fbMacro.mode=reader.readC();
    std.fmsMacro.mode=reader.readC();
    std.amsMacro.mode=reader.readC();
    std.panLMacro.mode=reader.readC();
    std.panRMacro.mode=reader.readC();
    std.phaseResetMacro.mode=reader.readC();
    std.ex4Macro.mode=reader.readC();
    std.ex5Macro.mode=reader.readC();
    std.ex6Macro.mode=reader.readC();
    std.ex7Macro.mode=reader.readC();
    std.ex8Macro.mode=reader.readC();
  }

  // C64 no test
  if (version>=89) {
    c64.noTest=reader.readC();
  }

  // MultiPCM
  if (version>=93) {
    multipcm.ar=reader.readC();
    multipcm.d1r=reader.readC();
    multipcm.dl=reader.readC();
    multipcm.d2r=reader.readC();
    multipcm.rr=reader.readC();
    multipcm.rc=reader.readC();
    multipcm.lfo=reader.readC();
    multipcm.vib=reader.readC();
    multipcm.am=reader.readC();
    // reserved
    for (int k=0; k<23; k++) reader.readC();
  }

  return DIV_DATA_SUCCESS;
}

bool DivInstrument::save(const char* path) {
  SafeWriter* w=new SafeWriter();
  w->init();

  // write magic
  w->write("-Furnace instr.-",16);

  // write version
  w->writeS(DIV_ENGINE_VERSION);

  // reserved
  w->writeS(0);

  // pointer to data
  w->writeI(32);

  // currently reserved (TODO; wavetable and sample here)
  w->writeS(0);
  w->writeS(0);
  w->writeI(0);

  putInsData(w);

  FILE* outFile=ps_fopen(path,"wb");
  if (outFile==NULL) {
    logE("could not save instrument: %s!",strerror(errno));
    w->finish();
    return false;
  }
  if (fwrite(w->getFinalBuf(),1,w->size(),outFile)!=w->size()) {
    logW("did not write entire instrument!");
  }
  fclose(outFile);
  w->finish();
  return true;
}
