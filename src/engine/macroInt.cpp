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
}

void DivMacroInt::init(DivInstrument* which) {
  ins=which;
  volPos=0;
  arpPos=0;
  dutyPos=0;
  wavePos=0;
  hasVol=false;
  hasArp=false;
  hasDuty=false;
  hasWave=false;
  hadVol=false;
  hadArp=false;
  hadDuty=false;
  hadWave=false;
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

  if (ins->std.arpMacroMode) {
    arpMode=true;
  }
}
