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
  w->writeC(4); // operator count; always 4
  w->writeC(0); // reserved
  w->writeC(0);
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
  reader.readC();
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
