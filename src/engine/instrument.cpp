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
  w->write("INST",4);
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
  for (int j=0; j<14; j++) { // reserved
    w->writeC(0);
  }

  // standard
  w->writeI(std.volMacroLen);
  w->writeI(std.arpMacroLen);
  w->writeI(std.dutyMacroLen);
  w->writeI(std.waveMacroLen);
  w->writeI(std.pitchMacroLen);
  w->writeI(std.ex1MacroLen);
  w->writeI(std.ex2MacroLen);
  w->writeI(std.ex3MacroLen);
  w->writeI(std.volMacroLoop);
  w->writeI(std.arpMacroLoop);
  w->writeI(std.dutyMacroLoop);
  w->writeI(std.waveMacroLoop);
  w->writeI(std.pitchMacroLoop);
  w->writeI(std.ex1MacroLoop);
  w->writeI(std.ex2MacroLoop);
  w->writeI(std.ex3MacroLoop);
  w->writeC(std.arpMacroMode);
  w->writeC(0); // reserved
  w->writeC(0);
  w->writeC(0);
  for (int j=0; j<std.volMacroLen; j++) {
    w->writeI(std.volMacro[j]);
  }
  for (int j=0; j<std.arpMacroLen; j++) {
    w->writeI(std.arpMacro[j]);
  }
  for (int j=0; j<std.dutyMacroLen; j++) {
    w->writeI(std.dutyMacro[j]);
  }
  for (int j=0; j<std.waveMacroLen; j++) {
    w->writeI(std.waveMacro[j]);
  }
  for (int j=0; j<std.pitchMacroLen; j++) {
    w->writeI(std.pitchMacro[j]);
  }
  for (int j=0; j<std.ex1MacroLen; j++) {
    w->writeI(std.ex1Macro[j]);
  }
  for (int j=0; j<std.ex2MacroLen; j++) {
    w->writeI(std.ex2Macro[j]);
  }
  for (int j=0; j<std.ex3MacroLen; j++) {
    w->writeI(std.ex3Macro[j]);
  }

  // FM macros and open status
  w->writeI(std.algMacroLen);
  w->writeI(std.fbMacroLen);
  w->writeI(std.fmsMacroLen);
  w->writeI(std.amsMacroLen);
  w->writeI(std.algMacroLoop);
  w->writeI(std.fbMacroLoop);
  w->writeI(std.fmsMacroLoop);
  w->writeI(std.amsMacroLoop);

  w->writeC(std.volMacroOpen);
  w->writeC(std.arpMacroOpen);
  w->writeC(std.dutyMacroOpen);
  w->writeC(std.waveMacroOpen);
  w->writeC(std.pitchMacroOpen);
  w->writeC(std.ex1MacroOpen);
  w->writeC(std.ex2MacroOpen);
  w->writeC(std.ex3MacroOpen);
  w->writeC(std.algMacroOpen);
  w->writeC(std.fbMacroOpen);
  w->writeC(std.fmsMacroOpen);
  w->writeC(std.amsMacroOpen);

  for (int j=0; j<std.algMacroLen; j++) {
    w->writeI(std.algMacro[j]);
  }
  for (int j=0; j<std.fbMacroLen; j++) {
    w->writeI(std.fbMacro[j]);
  }
  for (int j=0; j<std.fmsMacroLen; j++) {
    w->writeI(std.fmsMacro[j]);
  }
  for (int j=0; j<std.amsMacroLen; j++) {
    w->writeI(std.amsMacro[j]);
  }

  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& op=std.opMacros[i];

    w->writeI(op.amMacroLen);
    w->writeI(op.arMacroLen);
    w->writeI(op.drMacroLen);
    w->writeI(op.multMacroLen);
    w->writeI(op.rrMacroLen);
    w->writeI(op.slMacroLen);
    w->writeI(op.tlMacroLen);
    w->writeI(op.dt2MacroLen);
    w->writeI(op.rsMacroLen);
    w->writeI(op.dtMacroLen);
    w->writeI(op.d2rMacroLen);
    w->writeI(op.ssgMacroLen);
    w->writeI(op.amMacroLoop);
    w->writeI(op.arMacroLoop);
    w->writeI(op.drMacroLoop);
    w->writeI(op.multMacroLoop);
    w->writeI(op.rrMacroLoop);
    w->writeI(op.slMacroLoop);
    w->writeI(op.tlMacroLoop);
    w->writeI(op.dt2MacroLoop);
    w->writeI(op.rsMacroLoop);
    w->writeI(op.dtMacroLoop);
    w->writeI(op.d2rMacroLoop);
    w->writeI(op.ssgMacroLoop);
    w->writeC(op.amMacroOpen);
    w->writeC(op.arMacroOpen);
    w->writeC(op.drMacroOpen);
    w->writeC(op.multMacroOpen);
    w->writeC(op.rrMacroOpen);
    w->writeC(op.slMacroOpen);
    w->writeC(op.tlMacroOpen);
    w->writeC(op.dt2MacroOpen);
    w->writeC(op.rsMacroOpen);
    w->writeC(op.dtMacroOpen);
    w->writeC(op.d2rMacroOpen);
    w->writeC(op.ssgMacroOpen);
  }

  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& op=std.opMacros[i];
    for (int j=0; j<op.amMacroLen; j++) {
      w->writeC(op.amMacro[j]);
    }
    for (int j=0; j<op.arMacroLen; j++) {
      w->writeC(op.arMacro[j]);
    }
    for (int j=0; j<op.drMacroLen; j++) {
      w->writeC(op.drMacro[j]);
    }
    for (int j=0; j<op.multMacroLen; j++) {
      w->writeC(op.multMacro[j]);
    }
    for (int j=0; j<op.rrMacroLen; j++) {
      w->writeC(op.rrMacro[j]);
    }
    for (int j=0; j<op.slMacroLen; j++) {
      w->writeC(op.slMacro[j]);
    }
    for (int j=0; j<op.tlMacroLen; j++) {
      w->writeC(op.tlMacro[j]);
    }
    for (int j=0; j<op.dt2MacroLen; j++) {
      w->writeC(op.dt2Macro[j]);
    }
    for (int j=0; j<op.rsMacroLen; j++) {
      w->writeC(op.rsMacro[j]);
    }
    for (int j=0; j<op.dtMacroLen; j++) {
      w->writeC(op.dtMacro[j]);
    }
    for (int j=0; j<op.d2rMacroLen; j++) {
      w->writeC(op.d2rMacro[j]);
    }
    for (int j=0; j<op.ssgMacroLen; j++) {
      w->writeC(op.ssgMacro[j]);
    }
  }

  // release points
  w->writeI(std.volMacroRel);
  w->writeI(std.arpMacroRel);
  w->writeI(std.dutyMacroRel);
  w->writeI(std.waveMacroRel);
  w->writeI(std.pitchMacroRel);
  w->writeI(std.ex1MacroRel);
  w->writeI(std.ex2MacroRel);
  w->writeI(std.ex3MacroRel);
  w->writeI(std.algMacroRel);
  w->writeI(std.fbMacroRel);
  w->writeI(std.fmsMacroRel);
  w->writeI(std.amsMacroRel);
  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& op=std.opMacros[i];

    w->writeI(op.amMacroRel);
    w->writeI(op.arMacroRel);
    w->writeI(op.drMacroRel);
    w->writeI(op.multMacroRel);
    w->writeI(op.rrMacroRel);
    w->writeI(op.slMacroRel);
    w->writeI(op.tlMacroRel);
    w->writeI(op.dt2MacroRel);
    w->writeI(op.rsMacroRel);
    w->writeI(op.dtMacroRel);
    w->writeI(op.d2rMacroRel);
    w->writeI(op.ssgMacroRel);
  }

  // extended op macros
  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& op=std.opMacros[i];

    w->writeI(op.damMacroLen);
    w->writeI(op.dvbMacroLen);
    w->writeI(op.egtMacroLen);
    w->writeI(op.kslMacroLen);
    w->writeI(op.susMacroLen);
    w->writeI(op.vibMacroLen);
    w->writeI(op.wsMacroLen);
    w->writeI(op.ksrMacroLen);
    
    w->writeI(op.damMacroLoop);
    w->writeI(op.dvbMacroLoop);
    w->writeI(op.egtMacroLoop);
    w->writeI(op.kslMacroLoop);
    w->writeI(op.susMacroLoop);
    w->writeI(op.vibMacroLoop);
    w->writeI(op.wsMacroLoop);
    w->writeI(op.ksrMacroLoop);

    w->writeI(op.damMacroRel);
    w->writeI(op.dvbMacroRel);
    w->writeI(op.egtMacroRel);
    w->writeI(op.kslMacroRel);
    w->writeI(op.susMacroRel);
    w->writeI(op.vibMacroRel);
    w->writeI(op.wsMacroRel);
    w->writeI(op.ksrMacroRel);

    w->writeC(op.damMacroOpen);
    w->writeC(op.dvbMacroOpen);
    w->writeC(op.egtMacroOpen);
    w->writeC(op.kslMacroOpen);
    w->writeC(op.susMacroOpen);
    w->writeC(op.vibMacroOpen);
    w->writeC(op.wsMacroOpen);
    w->writeC(op.ksrMacroOpen);
  }

  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& op=std.opMacros[i];
    for (int j=0; j<op.damMacroLen; j++) {
      w->writeC(op.damMacro[j]);
    }
    for (int j=0; j<op.dvbMacroLen; j++) {
      w->writeC(op.dvbMacro[j]);
    }
    for (int j=0; j<op.egtMacroLen; j++) {
      w->writeC(op.egtMacro[j]);
    }
    for (int j=0; j<op.kslMacroLen; j++) {
      w->writeC(op.kslMacro[j]);
    }
    for (int j=0; j<op.susMacroLen; j++) {
      w->writeC(op.susMacro[j]);
    }
    for (int j=0; j<op.vibMacroLen; j++) {
      w->writeC(op.vibMacro[j]);
    }
    for (int j=0; j<op.wsMacroLen; j++) {
      w->writeC(op.wsMacro[j]);
    }
    for (int j=0; j<op.ksrMacroLen; j++) {
      w->writeC(op.ksrMacro[j]);
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
    w->write(amiga.noteFreq,120*sizeof(unsigned int));
    w->write(amiga.noteMap,120*sizeof(short));
  }
}

DivDataErrors DivInstrument::readInsData(SafeReader& reader, short version) {
  char magic[4];
  reader.read(magic,4);
  if (memcmp(magic,"INST",4)!=0) {
    logE("invalid instrument header!\n");
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
  // reserved
  for (int k=0; k<14; k++) reader.readC();

  // standard
  std.volMacroLen=reader.readI();
  std.arpMacroLen=reader.readI();
  std.dutyMacroLen=reader.readI();
  std.waveMacroLen=reader.readI();
  if (version>=17) {
    std.pitchMacroLen=reader.readI();
    std.ex1MacroLen=reader.readI();
    std.ex2MacroLen=reader.readI();
    std.ex3MacroLen=reader.readI();
  }
  std.volMacroLoop=reader.readI();
  std.arpMacroLoop=reader.readI();
  std.dutyMacroLoop=reader.readI();
  std.waveMacroLoop=reader.readI();
  if (version>=17) {
    std.pitchMacroLoop=reader.readI();
    std.ex1MacroLoop=reader.readI();
    std.ex2MacroLoop=reader.readI();
    std.ex3MacroLoop=reader.readI();
  }
  std.arpMacroMode=reader.readC();
  std.volMacroHeight=reader.readC();
  std.dutyMacroHeight=reader.readC();
  std.waveMacroHeight=reader.readC();
  if (std.volMacroHeight==0) std.volMacroHeight=15;
  if (std.dutyMacroHeight==0) std.dutyMacroHeight=3;
  if (std.waveMacroHeight==0) std.waveMacroHeight=63;
  reader.read(std.volMacro,4*std.volMacroLen);
  reader.read(std.arpMacro,4*std.arpMacroLen);
  reader.read(std.dutyMacro,4*std.dutyMacroLen);
  reader.read(std.waveMacro,4*std.waveMacroLen);
  if (version<31) {
    if (!std.arpMacroMode) for (int j=0; j<std.arpMacroLen; j++) {
      std.arpMacro[j]-=12;
    }
  }
  if (version>=17) {
    reader.read(std.pitchMacro,4*std.pitchMacroLen);
    reader.read(std.ex1Macro,4*std.ex1MacroLen);
    reader.read(std.ex2Macro,4*std.ex2MacroLen);
    reader.read(std.ex3Macro,4*std.ex3MacroLen);
  } else {
    if (type==DIV_INS_STD) {
      if (std.volMacroHeight==31) {
        type=DIV_INS_PCE;
      }
      if (std.dutyMacroHeight==31) {
        type=DIV_INS_AY;
      }
    }
  }

  // FM macros
  if (version>=29) {
    std.algMacroLen=reader.readI();
    std.fbMacroLen=reader.readI();
    std.fmsMacroLen=reader.readI();
    std.amsMacroLen=reader.readI();
    std.algMacroLoop=reader.readI();
    std.fbMacroLoop=reader.readI();
    std.fmsMacroLoop=reader.readI();
    std.amsMacroLoop=reader.readI();
    std.volMacroOpen=reader.readC();
    std.arpMacroOpen=reader.readC();
    std.dutyMacroOpen=reader.readC();
    std.waveMacroOpen=reader.readC();
    std.pitchMacroOpen=reader.readC();
    std.ex1MacroOpen=reader.readC();
    std.ex2MacroOpen=reader.readC();
    std.ex3MacroOpen=reader.readC();
    std.algMacroOpen=reader.readC();
    std.fbMacroOpen=reader.readC();
    std.fmsMacroOpen=reader.readC();
    std.amsMacroOpen=reader.readC();

    reader.read(std.algMacro,4*std.algMacroLen);
    reader.read(std.fbMacro,4*std.fbMacroLen);
    reader.read(std.fmsMacro,4*std.fmsMacroLen);
    reader.read(std.amsMacro,4*std.amsMacroLen);

    for (int i=0; i<4; i++) {
      DivInstrumentSTD::OpMacro& op=std.opMacros[i];

      op.amMacroLen=reader.readI();
      op.arMacroLen=reader.readI();
      op.drMacroLen=reader.readI();
      op.multMacroLen=reader.readI();
      op.rrMacroLen=reader.readI();
      op.slMacroLen=reader.readI();
      op.tlMacroLen=reader.readI();
      op.dt2MacroLen=reader.readI();
      op.rsMacroLen=reader.readI();
      op.dtMacroLen=reader.readI();
      op.d2rMacroLen=reader.readI();
      op.ssgMacroLen=reader.readI();

      op.amMacroLoop=reader.readI();
      op.arMacroLoop=reader.readI();
      op.drMacroLoop=reader.readI();
      op.multMacroLoop=reader.readI();
      op.rrMacroLoop=reader.readI();
      op.slMacroLoop=reader.readI();
      op.tlMacroLoop=reader.readI();
      op.dt2MacroLoop=reader.readI();
      op.rsMacroLoop=reader.readI();
      op.dtMacroLoop=reader.readI();
      op.d2rMacroLoop=reader.readI();
      op.ssgMacroLoop=reader.readI();

      op.amMacroOpen=reader.readC();
      op.arMacroOpen=reader.readC();
      op.drMacroOpen=reader.readC();
      op.multMacroOpen=reader.readC();
      op.rrMacroOpen=reader.readC();
      op.slMacroOpen=reader.readC();
      op.tlMacroOpen=reader.readC();
      op.dt2MacroOpen=reader.readC();
      op.rsMacroOpen=reader.readC();
      op.dtMacroOpen=reader.readC();
      op.d2rMacroOpen=reader.readC();
      op.ssgMacroOpen=reader.readC();
    }

    for (int i=0; i<4; i++) {
      DivInstrumentSTD::OpMacro& op=std.opMacros[i];
      reader.read(op.amMacro,op.amMacroLen);
      reader.read(op.arMacro,op.arMacroLen);
      reader.read(op.drMacro,op.drMacroLen);
      reader.read(op.multMacro,op.multMacroLen);
      reader.read(op.rrMacro,op.rrMacroLen);
      reader.read(op.slMacro,op.slMacroLen);
      reader.read(op.tlMacro,op.tlMacroLen);
      reader.read(op.dt2Macro,op.dt2MacroLen);
      reader.read(op.rsMacro,op.rsMacroLen);
      reader.read(op.dtMacro,op.dtMacroLen);
      reader.read(op.d2rMacro,op.d2rMacroLen);
      reader.read(op.ssgMacro,op.ssgMacroLen);
    }
  }

  // release points
  if (version>=44) {
    std.volMacroRel=reader.readI();
    std.arpMacroRel=reader.readI();
    std.dutyMacroRel=reader.readI();
    std.waveMacroRel=reader.readI();
    std.pitchMacroRel=reader.readI();
    std.ex1MacroRel=reader.readI();
    std.ex2MacroRel=reader.readI();
    std.ex3MacroRel=reader.readI();
    std.algMacroRel=reader.readI();
    std.fbMacroRel=reader.readI();
    std.fmsMacroRel=reader.readI();
    std.amsMacroRel=reader.readI();

    for (int i=0; i<4; i++) {
      DivInstrumentSTD::OpMacro& op=std.opMacros[i];

      op.amMacroRel=reader.readI();
      op.arMacroRel=reader.readI();
      op.drMacroRel=reader.readI();
      op.multMacroRel=reader.readI();
      op.rrMacroRel=reader.readI();
      op.slMacroRel=reader.readI();
      op.tlMacroRel=reader.readI();
      op.dt2MacroRel=reader.readI();
      op.rsMacroRel=reader.readI();
      op.dtMacroRel=reader.readI();
      op.d2rMacroRel=reader.readI();
      op.ssgMacroRel=reader.readI();
    }
  }

  // extended op macros
  if (version>=61) {
    for (int i=0; i<4; i++) {
      DivInstrumentSTD::OpMacro& op=std.opMacros[i];

      op.damMacroLen=reader.readI();
      op.dvbMacroLen=reader.readI();
      op.egtMacroLen=reader.readI();
      op.kslMacroLen=reader.readI();
      op.susMacroLen=reader.readI();
      op.vibMacroLen=reader.readI();
      op.wsMacroLen=reader.readI();
      op.ksrMacroLen=reader.readI();

      op.damMacroLoop=reader.readI();
      op.dvbMacroLoop=reader.readI();
      op.egtMacroLoop=reader.readI();
      op.kslMacroLoop=reader.readI();
      op.susMacroLoop=reader.readI();
      op.vibMacroLoop=reader.readI();
      op.wsMacroLoop=reader.readI();
      op.ksrMacroLoop=reader.readI();

      op.damMacroRel=reader.readI();
      op.dvbMacroRel=reader.readI();
      op.egtMacroRel=reader.readI();
      op.kslMacroRel=reader.readI();
      op.susMacroRel=reader.readI();
      op.vibMacroRel=reader.readI();
      op.wsMacroRel=reader.readI();
      op.ksrMacroRel=reader.readI();

      op.damMacroOpen=reader.readC();
      op.dvbMacroOpen=reader.readC();
      op.egtMacroOpen=reader.readC();
      op.kslMacroOpen=reader.readC();
      op.susMacroOpen=reader.readC();
      op.vibMacroOpen=reader.readC();
      op.wsMacroOpen=reader.readC();
      op.ksrMacroOpen=reader.readC();
    }

    for (int i=0; i<4; i++) {
      DivInstrumentSTD::OpMacro& op=std.opMacros[i];
      reader.read(op.damMacro,op.damMacroLen);
      reader.read(op.dvbMacro,op.dvbMacroLen);
      reader.read(op.egtMacro,op.egtMacroLen);
      reader.read(op.kslMacro,op.kslMacroLen);
      reader.read(op.susMacro,op.susMacroLen);
      reader.read(op.vibMacro,op.vibMacroLen);
      reader.read(op.wsMacro,op.wsMacroLen);
      reader.read(op.ksrMacro,op.ksrMacroLen);
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
    std.dutyMacroLen=0;
    std.dutyMacroLoop=-1;
    std.dutyMacroRel=-1;
  }

  // sample map
  if (version>=67) {
    amiga.useNoteMap=reader.readC();
    if (amiga.useNoteMap) {
      reader.read(amiga.noteFreq,120*sizeof(unsigned int));
      reader.read(amiga.noteMap,120*sizeof(short));
    }
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
    logE("could not save instrument: %s!\n",strerror(errno));
    w->finish();
    return false;
  }
  if (fwrite(w->getFinalBuf(),1,w->size(),outFile)!=w->size()) {
    logW("did not write entire instrument!\n");
  }
  fclose(outFile);
  w->finish();
  return true;
}
