#include "macroInt.h"

void DivMacroInt::next() {
  if (ins==NULL) return;

  if (finishedVol) finishedVol=false;
  if (hadVol!=hasVol) {
    finishedVol=true;
  }
  hadVol=hasVol;
  if (hasVol) {
    vol=ins->std.volMacro[volPos++];
    if (volPos>=ins->std.volMacroLen) {
      if (ins->std.volMacroLoop<ins->std.volMacroLen && ins->std.volMacroLoop>=0) {
        volPos=ins->std.volMacroLoop;
      } else {
        hasVol=false;
      }
    }
  }

  if (finishedArp) finishedArp=false;
  if (hadArp!=hasArp) {
    finishedArp=true;
  }
  hadArp=hasArp;
  if (hasArp) {
    arp=ins->std.arpMacro[arpPos++];
    if (arpPos>=ins->std.arpMacroLen) {
      if (ins->std.arpMacroLoop>=0 && ins->std.arpMacroLoop<ins->std.arpMacroLen) {
        arpPos=ins->std.arpMacroLoop;
      } else {
        hasArp=false;
      }
    }
  }

  if (finishedDuty) finishedDuty=false;
  if (hadDuty!=hasDuty) {
    finishedDuty=true;
  }
  hadDuty=hasDuty;
  if (hasDuty) {
    duty=ins->std.dutyMacro[dutyPos++];
    if (dutyPos>=ins->std.dutyMacroLen && ins->std.dutyMacroLoop<ins->std.dutyMacroLen) {
      if (ins->std.dutyMacroLoop>=0) {
        dutyPos=ins->std.dutyMacroLoop;
      } else {
        hasDuty=false;
      }
    }
  }

  if (finishedWave) finishedWave=false;
  if (hadWave!=hasWave) {
    finishedWave=true;
  }
  hadWave=hasWave;
  if (hasWave) {
    wave=ins->std.waveMacro[wavePos++];
    if (wavePos>=ins->std.waveMacroLen && ins->std.waveMacroLoop<ins->std.waveMacroLen) {
      if (ins->std.waveMacroLoop>=0) {
        wavePos=ins->std.waveMacroLoop;
      } else {
        hasWave=false;
      }
    }
  }

  if (finishedPitch) finishedPitch=false;
  if (hadPitch!=hasPitch) {
    finishedPitch=true;
  }
  hadPitch=hasPitch;
  if (hasPitch) {
    pitch=ins->std.pitchMacro[pitchPos++];
    if (pitchPos>=ins->std.pitchMacroLen && ins->std.pitchMacroLoop<ins->std.pitchMacroLen) {
      if (ins->std.pitchMacroLoop>=0) {
        pitchPos=ins->std.pitchMacroLoop;
      } else {
        hasPitch=false;
      }
    }
  }

  if (finishedEx1) finishedEx1=false;
  if (hadEx1!=hasEx1) {
    finishedEx1=true;
  }
  hadEx1=hasEx1;
  if (hasEx1) {
    ex1=ins->std.ex1Macro[ex1Pos++];
    if (ex1Pos>=ins->std.ex1MacroLen && ins->std.ex1MacroLoop<ins->std.ex1MacroLen) {
      if (ins->std.ex1MacroLoop>=0) {
        ex1Pos=ins->std.ex1MacroLoop;
      } else {
        hasEx1=false;
      }
    }
  }

  if (finishedEx2) finishedEx2=false;
  if (hadEx2!=hasEx2) {
    finishedEx2=true;
  }
  hadEx2=hasEx2;
  if (hasEx2) {
    ex2=ins->std.ex2Macro[ex2Pos++];
    if (ex2Pos>=ins->std.ex2MacroLen && ins->std.ex2MacroLoop<ins->std.ex2MacroLen) {
      if (ins->std.ex2MacroLoop>=0) {
        ex2Pos=ins->std.ex2MacroLoop;
      } else {
        hasEx2=false;
      }
    }
  }

  if (finishedEx3) finishedEx3=false;
  if (hadEx3!=hasEx3) {
    finishedEx3=true;
  }
  hadEx3=hasEx3;
  if (hasEx3) {
    ex3=ins->std.ex3Macro[ex3Pos++];
    if (ex3Pos>=ins->std.ex3MacroLen && ins->std.ex3MacroLoop<ins->std.ex3MacroLen) {
      if (ins->std.ex3MacroLoop>=0) {
        ex3Pos=ins->std.ex3MacroLoop;
      } else {
        hasEx3=false;
      }
    }
  }
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
  hasVol=false;
  hasArp=false;
  hasDuty=false;
  hasWave=false;
  hasPitch=false;
  hasEx1=false;
  hasEx2=false;
  hasEx3=false;
  hadVol=false;
  hadArp=false;
  hadDuty=false;
  hadWave=false;
  hadPitch=false;
  hadEx1=false;
  hadEx2=false;
  hadEx3=false;
  willVol=false;
  willArp=false;
  willDuty=false;
  willWave=false;
  willPitch=false;
  willEx1=false;
  willEx2=false;
  willEx3=false;
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

  if (ins->std.arpMacroMode) {
    arpMode=true;
  }
}
