/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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

#include "macroInt.h"
#include "instrument.h"
#include "engine.h"
#include "../ta-log.h"

#define ADSR_LOW source.val[0]
#define ADSR_HIGH source.val[1]
#define ADSR_AR source.val[2]
#define ADSR_HT source.val[3]
#define ADSR_DR source.val[4]
#define ADSR_SL source.val[5]
#define ADSR_ST source.val[6]
#define ADSR_SR source.val[7]
#define ADSR_RR source.val[8]

#define LFO_SPEED source.val[11]
#define LFO_WAVE source.val[12]
#define LFO_PHASE source.val[13]
#define LFO_LOOP source.val[14]
#define LFO_GLOBAL source.val[15]

void DivMacroStruct::prepare(DivInstrumentMacro& source, DivEngine* e) {
  has=had=actualHad=will=true;
  mode=source.mode;
  type=(source.open>>1)&3;
  activeRelease=source.open&8;
  linger=(source.macroType==DIV_MACRO_VOL && e->song.volMacroLinger);
  lfoPos=LFO_PHASE;
}

void DivMacroStruct::doMacro(DivInstrumentMacro& source, bool released, bool tick) {
  if (!tick) {
    had=false;
    return;
  }
  if (masked) {
    had=false;
    has=false;
    return;
  }
  if (released && type==1 && lastPos<3) delay=0;
  if (released && type==0 && pos<source.rel && source.rel<source.len && activeRelease) {
    delay=0;
    pos=source.rel;
  }
  if (delay>0) {
    delay--;
    if (!linger) had=false;
    return;
  }
  if (began && source.delay>0) {
    delay=source.delay;
  } else {
    delay=source.speed-1;
  }
  if (began) {
    began=false;
  }
  if (finished) {
    finished=false;
  }
  if (actualHad!=has) {
    finished=true;
  }
  actualHad=has;
  had=actualHad;

  if (has) {
    if (type==0) { // sequence
      lastPos=pos;
      val=source.val[pos++];
      if (pos>source.rel && !released) {
        if (source.loop<source.len && source.loop<source.rel) {
          pos=source.loop;
        } else {
          pos--;
        }
      }
      if (pos>=source.len) {
        if (source.loop<source.len && (source.loop>=source.rel || source.rel>=source.len)) {
          pos=source.loop;
        } else if (linger) {
          pos--;
        } else {
          has=false;
        }
      }
    }
    if (type==1) { // ADSR
      if (released && lastPos<3) lastPos=3;
      switch (lastPos) {
        case 0: // attack
          pos+=ADSR_AR;
          if (pos>255) {
            pos=255;
            lastPos=1;
            delay=ADSR_HT;
          }
          break;
        case 1: // decay
          pos-=ADSR_DR;
          if (pos<=ADSR_SL) {
            pos=ADSR_SL;
            lastPos=2;
            delay=ADSR_ST;
          }
          break;
        case 2: // sustain
          pos-=ADSR_SR;
          if (pos<0) {
            pos=0;
            lastPos=4;
          }
          break;
        case 3: // release
          pos-=ADSR_RR;
          if (pos<0) {
            pos=0;
            lastPos=4;
          }
          break;
        case 4: // end
          pos=0;
          if (!linger) has=false;
          break;
      }
      if (ADSR_HIGH>ADSR_LOW) {
        val=ADSR_LOW+((pos+(ADSR_HIGH-ADSR_LOW)*pos)>>8);
      } else {
        val=ADSR_HIGH+(((255-pos)+(ADSR_LOW-ADSR_HIGH)*(255-pos))>>8);
      }
    }
    if (type==2) { // LFO
      lfoPos+=LFO_SPEED;
      lfoPos&=1023;

      int lfoOut=0;
      switch (LFO_WAVE&3) {
        case 0: // triangle
          lfoOut=((lfoPos&512)?(1023-lfoPos):(lfoPos))>>1;
          break;
        case 1: // saw
          lfoOut=lfoPos>>2;
          break;
        case 2: // pulse
          lfoOut=(lfoPos&512)?255:0;
          break;
      }
      if (ADSR_HIGH>ADSR_LOW) {
        val=ADSR_LOW+((lfoOut+(ADSR_HIGH-ADSR_LOW)*lfoOut)>>8);
      } else {
        val=ADSR_HIGH+(((255-lfoOut)+(ADSR_LOW-ADSR_HIGH)*(255-lfoOut))>>8);
      }
    }
  }
}

void DivMacroInt::next() {
  if (ins==NULL) return;
  // run macros
  // TODO: potentially get rid of list to avoid allocations
  subTick--;
  for (size_t i=0; i<macroListLen; i++) {
    if (macroList[i]!=NULL && macroSource[i]!=NULL) {
      macroList[i]->doMacro(*macroSource[i],released,subTick==0);
    }
  }
  if (subTick<=0) {
    if (e==NULL) {
      subTick=1;
    } else {
      subTick=e->tickMult;
    }
  }
}

#define CONSIDER(x,y) \
  case y: \
    x.masked=enabled; \
    break;

#define CONSIDER_OP(oi,o) \
  CONSIDER(op[oi].am,0+o) \
  CONSIDER(op[oi].ar,1+o) \
  CONSIDER(op[oi].dr,2+o) \
  CONSIDER(op[oi].mult,3+o) \
  CONSIDER(op[oi].rr,4+o) \
  CONSIDER(op[oi].sl,5+o) \
  CONSIDER(op[oi].tl,6+o) \
  CONSIDER(op[oi].dt2,7+o) \
  CONSIDER(op[oi].rs,8+o) \
  CONSIDER(op[oi].dt,9+o) \
  CONSIDER(op[oi].d2r,10+o) \
  CONSIDER(op[oi].ssg,11+o) \
  CONSIDER(op[oi].dam,12+o) \
  CONSIDER(op[oi].dvb,13+o) \
  CONSIDER(op[oi].egt,14+o) \
  CONSIDER(op[oi].ksl,15+o) \
  CONSIDER(op[oi].sus,16+o) \
  CONSIDER(op[oi].vib,17+o) \
  CONSIDER(op[oi].ws,18+o) \
  CONSIDER(op[oi].ksr,19+o)

void DivMacroInt::mask(unsigned char id, bool enabled) {
  switch (id) {
    CONSIDER(vol,0)
    CONSIDER(arp,1)
    CONSIDER(duty,2)
    CONSIDER(wave,3)
    CONSIDER(pitch,4)
    CONSIDER(ex1,5)
    CONSIDER(ex2,6)
    CONSIDER(ex3,7)
    CONSIDER(alg,8)
    CONSIDER(fb,9)
    CONSIDER(fms,10)
    CONSIDER(ams,11)
    CONSIDER(panL,12)
    CONSIDER(panR,13)
    CONSIDER(phaseReset,14)
    CONSIDER(ex4,15)
    CONSIDER(ex5,16)
    CONSIDER(ex6,17)
    CONSIDER(ex7,18)
    CONSIDER(ex8,19)
    CONSIDER(ex9,20)
    CONSIDER(ex10,21)

    CONSIDER_OP(0,0x20)
    CONSIDER_OP(2,0x40)
    CONSIDER_OP(1,0x60)
    CONSIDER_OP(3,0x80)
  }
}

#undef CONSIDER_OP
#undef CONSIDER

#define CONSIDER(x,y,z) \
  case z: \
    macroState=&x; \
    macro=&ins->std.y; \
    break;

#define CONSIDER_OP(oi,o) \
  CONSIDER(op[oi].am,opMacros[oi].amMacro,0+o) \
  CONSIDER(op[oi].ar,opMacros[oi].arMacro,1+o) \
  CONSIDER(op[oi].dr,opMacros[oi].drMacro,2+o) \
  CONSIDER(op[oi].mult,opMacros[oi].multMacro,3+o) \
  CONSIDER(op[oi].rr,opMacros[oi].rrMacro,4+o) \
  CONSIDER(op[oi].sl,opMacros[oi].slMacro,5+o) \
  CONSIDER(op[oi].tl,opMacros[oi].tlMacro,6+o) \
  CONSIDER(op[oi].dt2,opMacros[oi].dt2Macro,7+o) \
  CONSIDER(op[oi].rs,opMacros[oi].rsMacro,8+o) \
  CONSIDER(op[oi].dt,opMacros[oi].dtMacro,9+o) \
  CONSIDER(op[oi].d2r,opMacros[oi].d2rMacro,10+o) \
  CONSIDER(op[oi].ssg,opMacros[oi].ssgMacro,11+o) \
  CONSIDER(op[oi].dam,opMacros[oi].damMacro,12+o) \
  CONSIDER(op[oi].dvb,opMacros[oi].dvbMacro,13+o) \
  CONSIDER(op[oi].egt,opMacros[oi].egtMacro,14+o) \
  CONSIDER(op[oi].ksl,opMacros[oi].kslMacro,15+o) \
  CONSIDER(op[oi].sus,opMacros[oi].susMacro,16+o) \
  CONSIDER(op[oi].vib,opMacros[oi].vibMacro,17+o) \
  CONSIDER(op[oi].ws,opMacros[oi].wsMacro,18+o) \
  CONSIDER(op[oi].ksr,opMacros[oi].ksrMacro,19+o)

void DivMacroInt::restart(unsigned char id) {
  DivMacroStruct* macroState=NULL;
  DivInstrumentMacro* macro=NULL;

  if (e==NULL) return;
  if (ins==NULL) return;
  
  switch (id) {
    CONSIDER(vol,volMacro,0)
    CONSIDER(arp,arpMacro,1)
    CONSIDER(duty,dutyMacro,2)
    CONSIDER(wave,waveMacro,3)
    CONSIDER(pitch,pitchMacro,4)
    CONSIDER(ex1,ex1Macro,5)
    CONSIDER(ex2,ex2Macro,6)
    CONSIDER(ex3,ex3Macro,7)
    CONSIDER(alg,algMacro,8)
    CONSIDER(fb,fbMacro,9)
    CONSIDER(fms,fmsMacro,10)
    CONSIDER(ams,amsMacro,11)
    CONSIDER(panL,panLMacro,12)
    CONSIDER(panR,panRMacro,13)
    CONSIDER(phaseReset,phaseResetMacro,14)
    CONSIDER(ex4,ex4Macro,15)
    CONSIDER(ex5,ex5Macro,16)
    CONSIDER(ex6,ex6Macro,17)
    CONSIDER(ex7,ex7Macro,18)
    CONSIDER(ex8,ex8Macro,19)
    CONSIDER(ex9,ex9Macro,20)
    CONSIDER(ex10,ex10Macro,21)

    CONSIDER_OP(0,0x20)
    CONSIDER_OP(2,0x40)
    CONSIDER_OP(1,0x60)
    CONSIDER_OP(3,0x80)
  }

  if (macroState==NULL || macro==NULL) return;

  if (macro->len<=0) return;
  if (macroState->masked) return;

  macroState->init();
  macroState->prepare(*macro,e);
}

#undef CONSIDER_OP
#undef CONSIDER

void DivMacroInt::release() {
  released=true;
}

void DivMacroInt::setEngine(DivEngine* eng) {
  e=eng;
}

#define ADD_MACRO(m,s) \
  if (!m.masked) { \
    macroList[macroListLen]=&m; \
    macroSource[macroListLen++]=&s; \
  }

void DivMacroInt::init(DivInstrument* which) {
  ins=which;
  // initialize
  for (size_t i=0; i<macroListLen; i++) {
    if (macroList[i]!=NULL) macroList[i]->init();
  }
  macroListLen=0;
  subTick=1;

  hasRelease=false;
  released=false;

  if (ins==NULL) return;

  // prepare common macro
  if (ins->std.volMacro.len>0) {
    ADD_MACRO(vol,ins->std.volMacro);
  }
  if (ins->std.arpMacro.len>0) {
    ADD_MACRO(arp,ins->std.arpMacro);
  }
  if (ins->std.dutyMacro.len>0) {
    ADD_MACRO(duty,ins->std.dutyMacro);
  }
  if (ins->std.waveMacro.len>0) {
    ADD_MACRO(wave,ins->std.waveMacro);
  }
  if (ins->std.pitchMacro.len>0) {
    ADD_MACRO(pitch,ins->std.pitchMacro);
  }
  if (ins->std.ex1Macro.len>0) {
    ADD_MACRO(ex1,ins->std.ex1Macro);
  }
  if (ins->std.ex2Macro.len>0) {
    ADD_MACRO(ex2,ins->std.ex2Macro);
  }
  if (ins->std.ex3Macro.len>0) {
    ADD_MACRO(ex3,ins->std.ex3Macro);
  }
  if (ins->std.algMacro.len>0) {
    ADD_MACRO(alg,ins->std.algMacro);
  }
  if (ins->std.fbMacro.len>0) {
    ADD_MACRO(fb,ins->std.fbMacro);
  }
  if (ins->std.fmsMacro.len>0) {
    ADD_MACRO(fms,ins->std.fmsMacro);
  }
  if (ins->std.amsMacro.len>0) {
    ADD_MACRO(ams,ins->std.amsMacro);
  }

  if (ins->std.panLMacro.len>0) {
    ADD_MACRO(panL,ins->std.panLMacro);
  }
  if (ins->std.panRMacro.len>0) {
    ADD_MACRO(panR,ins->std.panRMacro);
  }
  if (ins->std.phaseResetMacro.len>0) {
    ADD_MACRO(phaseReset,ins->std.phaseResetMacro);
  }
  if (ins->std.ex4Macro.len>0) {
    ADD_MACRO(ex4,ins->std.ex4Macro);
  }
  if (ins->std.ex5Macro.len>0) {
    ADD_MACRO(ex5,ins->std.ex5Macro);
  }
  if (ins->std.ex6Macro.len>0) {
    ADD_MACRO(ex6,ins->std.ex6Macro);
  }
  if (ins->std.ex7Macro.len>0) {
    ADD_MACRO(ex7,ins->std.ex7Macro);
  }
  if (ins->std.ex8Macro.len>0) {
    ADD_MACRO(ex8,ins->std.ex8Macro);
  }
  if (ins->std.ex9Macro.len>0) {
    ADD_MACRO(ex9,ins->std.ex9Macro);
  }
  if (ins->std.ex10Macro.len>0) {
    ADD_MACRO(ex10,ins->std.ex10Macro);
  }

  // prepare FM operator macros
  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& m=ins->std.opMacros[i];
    IntOp& o=op[i];
    if (m.amMacro.len>0) {
      ADD_MACRO(o.am,m.amMacro);
    }
    if (m.arMacro.len>0) {
      ADD_MACRO(o.ar,m.arMacro);
    }
    if (m.drMacro.len>0) {
      ADD_MACRO(o.dr,m.drMacro);
    }
    if (m.multMacro.len>0) {
      ADD_MACRO(o.mult,m.multMacro);
    }
    if (m.rrMacro.len>0) {
      ADD_MACRO(o.rr,m.rrMacro);
    }
    if (m.slMacro.len>0) {
      ADD_MACRO(o.sl,m.slMacro);
    }
    if (m.tlMacro.len>0) {
      ADD_MACRO(o.tl,m.tlMacro);
    }
    if (m.dt2Macro.len>0) {
      ADD_MACRO(o.dt2,m.dt2Macro);
    }
    if (m.rsMacro.len>0) {
      ADD_MACRO(o.rs,m.rsMacro);
    }
    if (m.dtMacro.len>0) {
      ADD_MACRO(o.dt,m.dtMacro);
    }
    if (m.d2rMacro.len>0) {
      ADD_MACRO(o.d2r,m.d2rMacro);
    }
    if (m.ssgMacro.len>0) {
      ADD_MACRO(o.ssg,m.ssgMacro);
    }

    if (m.damMacro.len>0) {
      ADD_MACRO(o.dam,m.damMacro);
    }
    if (m.dvbMacro.len>0) {
      ADD_MACRO(o.dvb,m.dvbMacro);
    }
    if (m.egtMacro.len>0) {
      ADD_MACRO(o.egt,m.egtMacro);
    }
    if (m.kslMacro.len>0) {
      ADD_MACRO(o.ksl,m.kslMacro);
    }
    if (m.susMacro.len>0) {
      ADD_MACRO(o.sus,m.susMacro);
    }
    if (m.vibMacro.len>0) {
      ADD_MACRO(o.vib,m.vibMacro);
    }
    if (m.wsMacro.len>0) {
      ADD_MACRO(o.ws,m.wsMacro);
    }
    if (m.ksrMacro.len>0) {
      ADD_MACRO(o.ksr,m.ksrMacro);
    }
  }

  for (size_t i=0; i<macroListLen; i++) {
    if (macroSource[i]!=NULL) {
      macroList[i]->prepare(*macroSource[i],e);
      // check ADSR mode
      if ((macroSource[i]->open&6)==2) {
        if (macroSource[i]->val[8]>0) {
          hasRelease=true;
        }
      } else if (macroSource[i]->rel<macroSource[i]->len) {
        hasRelease=true;
      }
    }
  }
}

void DivMacroInt::notifyInsDeletion(DivInstrument* which) {
  if (ins==which) {
    init(NULL);
  }
}

#define CONSIDER(x,y) case (y&0x1f): return &x; break;

DivMacroStruct* DivMacroInt::structByType(unsigned char type) {
  if (type>=0x20) {
    unsigned char o=((type>>5)-1)&3;
    switch (type&0x1f) {
      CONSIDER(op[o].am,DIV_MACRO_OP_AM)
      CONSIDER(op[o].ar,DIV_MACRO_OP_AR)
      CONSIDER(op[o].dr,DIV_MACRO_OP_DR)
      CONSIDER(op[o].mult,DIV_MACRO_OP_MULT)
      CONSIDER(op[o].rr,DIV_MACRO_OP_RR)
      CONSIDER(op[o].sl,DIV_MACRO_OP_SL)
      CONSIDER(op[o].tl,DIV_MACRO_OP_TL)
      CONSIDER(op[o].dt2,DIV_MACRO_OP_DT2)
      CONSIDER(op[o].rs,DIV_MACRO_OP_RS)
      CONSIDER(op[o].dt,DIV_MACRO_OP_DT)
      CONSIDER(op[o].d2r,DIV_MACRO_OP_D2R)
      CONSIDER(op[o].ssg,DIV_MACRO_OP_SSG)
      CONSIDER(op[o].dam,DIV_MACRO_OP_DAM)
      CONSIDER(op[o].dvb,DIV_MACRO_OP_DVB)
      CONSIDER(op[o].egt,DIV_MACRO_OP_EGT)
      CONSIDER(op[o].ksl,DIV_MACRO_OP_KSL)
      CONSIDER(op[o].sus,DIV_MACRO_OP_SUS)
      CONSIDER(op[o].vib,DIV_MACRO_OP_VIB)
      CONSIDER(op[o].ws,DIV_MACRO_OP_WS)
      CONSIDER(op[o].ksr,DIV_MACRO_OP_KSR)
    }

    return NULL;
  }

  switch (type) {
    CONSIDER(vol,DIV_MACRO_VOL)
    CONSIDER(arp,DIV_MACRO_ARP)
    CONSIDER(duty,DIV_MACRO_DUTY)
    CONSIDER(wave,DIV_MACRO_WAVE)
    CONSIDER(pitch,DIV_MACRO_PITCH)
    CONSIDER(ex1,DIV_MACRO_EX1)
    CONSIDER(ex2,DIV_MACRO_EX2)
    CONSIDER(ex3,DIV_MACRO_EX3)
    CONSIDER(alg,DIV_MACRO_ALG)
    CONSIDER(fb,DIV_MACRO_FB)
    CONSIDER(fms,DIV_MACRO_FMS)
    CONSIDER(ams,DIV_MACRO_AMS)
    CONSIDER(panL,DIV_MACRO_PAN_LEFT)
    CONSIDER(panR,DIV_MACRO_PAN_RIGHT)
    CONSIDER(phaseReset,DIV_MACRO_PHASE_RESET)
    CONSIDER(ex4,DIV_MACRO_EX4)
    CONSIDER(ex5,DIV_MACRO_EX5)
    CONSIDER(ex6,DIV_MACRO_EX6)
    CONSIDER(ex7,DIV_MACRO_EX7)
    CONSIDER(ex8,DIV_MACRO_EX8)
    CONSIDER(ex9,DIV_MACRO_EX9)
    CONSIDER(ex10,DIV_MACRO_EX10)
  }

  return NULL;
}

#undef CONSIDER
