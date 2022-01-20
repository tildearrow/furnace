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
