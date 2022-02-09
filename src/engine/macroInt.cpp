#include "macroInt.h"
#include "instrument.h"

#define doMacro(finished,had,has,val,pos,source,sourceLen,sourceLoop,sourceRel) \
  if (finished) finished=false; \
  if (had!=has) { \
    finished=true; \
  } \
  had=has; \
  if (has) { \
    val=source[pos++]; \
    if (sourceRel>=0 && pos>sourceRel && !released) { \
      if (sourceLoop<sourceLen && sourceLoop>=0 && sourceLoop<sourceRel) { \
        pos=sourceLoop; \
      } else { \
        pos--; \
      } \
    } \
    if (pos>=sourceLen) { \
      if (sourceLoop<sourceLen && sourceLoop>=0 && (sourceLoop>=sourceRel || sourceRel>=sourceLen)) { \
        pos=sourceLoop; \
      } else { \
        has=false; \
      } \
    } \
  }

void DivMacroInt::next() {
  if (ins==NULL) return;

  doMacro(finishedVol,hadVol,hasVol,vol,volPos,ins->std.volMacro,ins->std.volMacroLen,ins->std.volMacroLoop,ins->std.volMacroRel);
  doMacro(finishedArp,hadArp,hasArp,arp,arpPos,ins->std.arpMacro,ins->std.arpMacroLen,ins->std.arpMacroLoop,ins->std.arpMacroRel);
  doMacro(finishedDuty,hadDuty,hasDuty,duty,dutyPos,ins->std.dutyMacro,ins->std.dutyMacroLen,ins->std.dutyMacroLoop,ins->std.dutyMacroRel);
  doMacro(finishedWave,hadWave,hasWave,wave,wavePos,ins->std.waveMacro,ins->std.waveMacroLen,ins->std.waveMacroLoop,ins->std.waveMacroRel);

  doMacro(finishedPitch,hadPitch,hasPitch,pitch,pitchPos,ins->std.pitchMacro,ins->std.pitchMacroLen,ins->std.pitchMacroLoop,ins->std.pitchMacroRel);
  doMacro(finishedEx1,hadEx1,hasEx1,ex1,ex1Pos,ins->std.ex1Macro,ins->std.ex1MacroLen,ins->std.ex1MacroLoop,ins->std.ex1MacroRel);
  doMacro(finishedEx2,hadEx2,hasEx2,ex2,ex2Pos,ins->std.ex2Macro,ins->std.ex2MacroLen,ins->std.ex2MacroLoop,ins->std.ex2MacroRel);
  doMacro(finishedEx3,hadEx3,hasEx3,ex3,ex3Pos,ins->std.ex3Macro,ins->std.ex3MacroLen,ins->std.ex3MacroLoop,ins->std.ex3MacroRel);

  doMacro(finishedAlg,hadAlg,hasAlg,alg,algPos,ins->std.algMacro,ins->std.algMacroLen,ins->std.algMacroLoop,ins->std.algMacroRel);
  doMacro(finishedFb,hadFb,hasFb,fb,fbPos,ins->std.fbMacro,ins->std.fbMacroLen,ins->std.fbMacroLoop,ins->std.fbMacroRel);
  doMacro(finishedFms,hadFms,hasFms,fms,fmsPos,ins->std.fmsMacro,ins->std.fmsMacroLen,ins->std.fmsMacroLoop,ins->std.fmsMacroRel);
  doMacro(finishedAms,hadAms,hasAms,ams,amsPos,ins->std.amsMacro,ins->std.amsMacroLen,ins->std.amsMacroLoop,ins->std.amsMacroRel);

  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& m=ins->std.opMacros[i];
    IntOp& o=op[i];
    doMacro(o.finishedAm,o.hadAm,o.hasAm,o.am,o.amPos,m.amMacro,m.amMacroLen,m.amMacroLoop,m.amMacroRel);
    doMacro(o.finishedAr,o.hadAr,o.hasAr,o.ar,o.arPos,m.arMacro,m.arMacroLen,m.arMacroLoop,m.arMacroRel);
    doMacro(o.finishedDr,o.hadDr,o.hasDr,o.dr,o.drPos,m.drMacro,m.drMacroLen,m.drMacroLoop,m.drMacroRel);
    doMacro(o.finishedMult,o.hadMult,o.hasMult,o.mult,o.multPos,m.multMacro,m.multMacroLen,m.multMacroLoop,m.multMacroRel);

    doMacro(o.finishedRr,o.hadRr,o.hasRr,o.rr,o.rrPos,m.rrMacro,m.rrMacroLen,m.rrMacroLoop,m.rrMacroRel);
    doMacro(o.finishedSl,o.hadSl,o.hasSl,o.sl,o.slPos,m.slMacro,m.slMacroLen,m.slMacroLoop,m.slMacroRel);
    doMacro(o.finishedTl,o.hadTl,o.hasTl,o.tl,o.tlPos,m.tlMacro,m.tlMacroLen,m.tlMacroLoop,m.tlMacroRel);
    doMacro(o.finishedDt2,o.hadDt2,o.hasDt2,o.dt2,o.dt2Pos,m.dt2Macro,m.dt2MacroLen,m.dt2MacroLoop,m.dt2MacroRel);

    doMacro(o.finishedRs,o.hadRs,o.hasRs,o.rs,o.rsPos,m.rsMacro,m.rsMacroLen,m.rsMacroLoop,m.rsMacroRel);
    doMacro(o.finishedDt,o.hadDt,o.hasDt,o.dt,o.dtPos,m.dtMacro,m.dtMacroLen,m.dtMacroLoop,m.dtMacroRel);
    doMacro(o.finishedD2r,o.hadD2r,o.hasD2r,o.d2r,o.d2rPos,m.d2rMacro,m.d2rMacroLen,m.d2rMacroLoop,m.d2rMacroRel);
    doMacro(o.finishedSsg,o.hadSsg,o.hasSsg,o.ssg,o.ssgPos,m.ssgMacro,m.ssgMacroLen,m.ssgMacroLoop,m.ssgMacroRel);
  }
}

void DivMacroInt::release() {
  released=true;
}

void DivMacroInt::init(DivInstrument* which) {
  ins=which;
  volPos=0;
  arpPos=0;
  dutyPos=0;
  wavePos=0;
  pitchPos=0;
  ex1Pos=0;
  ex2Pos=0;
  ex3Pos=0;
  algPos=0;
  fbPos=0;
  fmsPos=0;
  amsPos=0;

  released=false;

  hasVol=false;
  hasArp=false;
  hasDuty=false;
  hasWave=false;
  hasPitch=false;
  hasEx1=false;
  hasEx2=false;
  hasEx3=false;
  hasAlg=false;
  hasFb=false;
  hasFms=false;
  hasAms=false;

  hadVol=false;
  hadArp=false;
  hadDuty=false;
  hadWave=false;
  hadPitch=false;
  hadEx1=false;
  hadEx2=false;
  hadEx3=false;
  hadAlg=false;
  hadFb=false;
  hadFms=false;
  hadAms=false;

  willVol=false;
  willArp=false;
  willDuty=false;
  willWave=false;
  willPitch=false;
  willEx1=false;
  willEx2=false;
  willEx3=false;
  willAlg=false;
  willFb=false;
  willFms=false;
  willAms=false;

  op[0]=IntOp();
  op[1]=IntOp();
  op[2]=IntOp();
  op[3]=IntOp();

  arpMode=false;

  if (ins==NULL) return;

  if (ins->std.volMacroLen>0) {
    hadVol=true;
    hasVol=true;
    willVol=true;
  }
  if (ins->std.arpMacroLen>0) {
    hadArp=true;
    hasArp=true;
    willArp=true;
  }
  if (ins->std.dutyMacroLen>0) {
    hadDuty=true;
    hasDuty=true;
    willDuty=true;
  }
  if (ins->std.waveMacroLen>0) {
    hadWave=true;
    hasWave=true;
    willWave=true;
  }
  if (ins->std.pitchMacroLen>0) {
    hadPitch=true;
    hasPitch=true;
    willPitch=true;
  }
  if (ins->std.ex1MacroLen>0) {
    hadEx1=true;
    hasEx1=true;
    willEx1=true;
  }
  if (ins->std.ex2MacroLen>0) {
    hadEx2=true;
    hasEx2=true;
    willEx2=true;
  }
  if (ins->std.ex3MacroLen>0) {
    hadEx3=true;
    hasEx3=true;
    willEx3=true;
  }
  if (ins->std.algMacroLen>0) {
    hadAlg=true;
    hasAlg=true;
    willAlg=true;
  }
  if (ins->std.fbMacroLen>0) {
    hadFb=true;
    hasFb=true;
    willFb=true;
  }
  if (ins->std.fmsMacroLen>0) {
    hadFms=true;
    hasFms=true;
    willFms=true;
  }
  if (ins->std.amsMacroLen>0) {
    hadAms=true;
    hasAms=true;
    willAms=true;
  }

  if (ins->std.arpMacroMode) {
    arpMode=true;
  }

  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& m=ins->std.opMacros[i];
    IntOp& o=op[i];

    if (m.amMacroLen>0) {
      o.hadAm=true;
      o.hasAm=true;
      o.willAm=true;
    }
    if (m.arMacroLen>0) {
      o.hadAr=true;
      o.hasAr=true;
      o.willAr=true;
    }
    if (m.drMacroLen>0) {
      o.hadDr=true;
      o.hasDr=true;
      o.willDr=true;
    }
    if (m.multMacroLen>0) {
      o.hadMult=true;
      o.hasMult=true;
      o.willMult=true;
    }
    if (m.rrMacroLen>0) {
      o.hadRr=true;
      o.hasRr=true;
      o.willRr=true;
    }
    if (m.slMacroLen>0) {
      o.hadSl=true;
      o.hasSl=true;
      o.willSl=true;
    }
    if (m.tlMacroLen>0) {
      o.hadTl=true;
      o.hasTl=true;
      o.willTl=true;
    }
    if (m.dt2MacroLen>0) {
      o.hadDt2=true;
      o.hasDt2=true;
      o.willDt2=true;
    }
    if (m.rsMacroLen>0) {
      o.hadRs=true;
      o.hasRs=true;
      o.willRs=true;
    }
    if (m.dtMacroLen>0) {
      o.hadDt=true;
      o.hasDt=true;
      o.willDt=true;
    }
    if (m.d2rMacroLen>0) {
      o.hadD2r=true;
      o.hasD2r=true;
      o.willD2r=true;
    }
    if (m.ssgMacroLen>0) {
      o.hadSsg=true;
      o.hasSsg=true;
      o.willSsg=true;
    }
  }
}

void DivMacroInt::notifyInsDeletion(DivInstrument* which) {
  if (ins==which) {
    init(NULL);
  }
}