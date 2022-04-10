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

#include "macroInt.h"
#include "instrument.h"

void DivMacroStruct::doMacro(DivInstrumentMacro& source, bool released) {
  if (finished) finished=false;
  if (had!=has) {
    finished=true;
  }
  had=has;
  if (has) {
    val=source.val[pos++];
    if (source.rel>=0 && pos>source.rel && !released) {
      if (source.loop<source.len && source.loop>=0 && source.loop<source.rel) {
        pos=source.loop;
      } else {
        pos--;
      }
    }
    if (pos>=source.len) {
      if (source.loop<source.len && source.loop>=0 && (source.loop>=source.rel || source.rel>=source.len)) {
        pos=source.loop;
      } else {
        has=false;
      }
    }
  }
}

// CPU hell
void DivMacroInt::next() {
  if (ins==NULL) return;
  // Run macros
  if (!macroList.empty()) {
      for (std::list<DivMacroExecList>::iterator iter = macroList.begin(); iter!= macroList.end(); iter++) {
        iter->doMacro(released);
    }
  }
}

void DivMacroInt::release() {
  released=true;
}

void DivMacroInt::init(DivInstrument* which) {
  ins=which;
  macroList.clear();
  // initialize common macros
  vol.init();
  arp.init();
  duty.init();
  wave.init();
  pitch.init();
  ex1.init();
  ex2.init();
  ex3.init();
  alg.init();
  fb.init();
  fms.init();
  ams.init();
  fms2.init();
  ams2.init();
  panL.init();
  panR.init();
  phaseReset.init();
  ex4.init();
  ex5.init();
  ex6.init();
  ex7.init();
  ex8.init();

  released=false;

  // initialize FM operator macro
  op[0]=IntOp();
  op[1]=IntOp();
  op[2]=IntOp();
  op[3]=IntOp();

  // initialize wavesynth macro
  ws=IntWS();

  if (ins==NULL) return;

  // prepare common macro
  if (ins->std.volMacro.len>0) {
    macroList.push_back(DivMacroExecList(vol,ins->std.volMacro));
    vol.prepare(&ins->std.volMacro);
  }
  if (ins->std.arpMacro.len>0) {
    macroList.push_back(DivMacroExecList(arp,ins->std.arpMacro));
    arp.prepare(&ins->std.arpMacro);
  }
  if (ins->std.dutyMacro.len>0) {
    macroList.push_back(DivMacroExecList(duty,ins->std.dutyMacro));
    duty.prepare(&ins->std.dutyMacro);
  }
  if (ins->std.waveMacro.len>0) {
    macroList.push_back(DivMacroExecList(wave,ins->std.waveMacro));
    wave.prepare(&ins->std.waveMacro);
  }
  if (ins->std.pitchMacro.len>0) {
    macroList.push_back(DivMacroExecList(pitch,ins->std.pitchMacro));
    pitch.prepare(&ins->std.pitchMacro);
  }
  if (ins->std.ex1Macro.len>0) {
    macroList.push_back(DivMacroExecList(ex1,ins->std.ex1Macro));
    ex1.prepare(&ins->std.ex1Macro);
  }
  if (ins->std.ex2Macro.len>0) {
    macroList.push_back(DivMacroExecList(ex2,ins->std.ex2Macro));
    ex2.prepare(&ins->std.ex2Macro);
  }
  if (ins->std.ex3Macro.len>0) {
    macroList.push_back(DivMacroExecList(ex3,ins->std.ex3Macro));
    ex3.prepare(&ins->std.ex3Macro);
  }
  if (ins->std.algMacro.len>0) {
    macroList.push_back(DivMacroExecList(alg,ins->std.algMacro));
    alg.prepare(&ins->std.algMacro);
  }
  if (ins->std.fbMacro.len>0) {
    macroList.push_back(DivMacroExecList(fb,ins->std.fbMacro));
    fb.prepare(&ins->std.fbMacro);
  }
  if (ins->std.fmsMacro.len>0) {
    macroList.push_back(DivMacroExecList(fms,ins->std.fmsMacro));
    fms.prepare(&ins->std.fmsMacro);
  }
  if (ins->std.fms2Macro.len>0) {
    macroList.push_back(DivMacroExecList(fms2,ins->std.fms2Macro));
    fms2.prepare(&ins->std.fms2Macro);
  }
  if (ins->std.amsMacro.len>0) {
    macroList.push_back(DivMacroExecList(ams,ins->std.amsMacro));
    ams.prepare(&ins->std.amsMacro);
  }
  if (ins->std.ams2Macro.len>0) {
    macroList.push_back(DivMacroExecList(ams2,ins->std.ams2Macro));
    ams2.prepare(&ins->std.ams2Macro);
  }

  // TODO: other macros
  if (ins->std.panLMacro.len>0) {
    macroList.push_back(DivMacroExecList(panL,ins->std.panLMacro));
    panL.prepare(&ins->std.panLMacro);
  }
  if (ins->std.panRMacro.len>0) {
    macroList.push_back(DivMacroExecList(panR,ins->std.panRMacro));
    panR.prepare(&ins->std.panRMacro);
  }
  if (ins->std.phaseResetMacro.len>0) {
    macroList.push_back(DivMacroExecList(phaseReset,ins->std.phaseResetMacro));
    phaseReset.prepare(&ins->std.phaseResetMacro);
  }
  if (ins->std.ex4Macro.len>0) {
    macroList.push_back(DivMacroExecList(ex4,ins->std.ex4Macro));
    ex4.prepare(&ins->std.ex4Macro);
  }
  if (ins->std.ex5Macro.len>0) {
    macroList.push_back(DivMacroExecList(ex5,ins->std.ex5Macro));
    ex5.prepare(&ins->std.ex5Macro);
  }
  if (ins->std.ex6Macro.len>0) {
    macroList.push_back(DivMacroExecList(ex6,ins->std.ex6Macro));
    ex6.prepare(&ins->std.ex6Macro);
  }
  if (ins->std.ex7Macro.len>0) {
    macroList.push_back(DivMacroExecList(ex7,ins->std.ex7Macro));
    ex7.prepare(&ins->std.ex7Macro);
  }
  if (ins->std.ex8Macro.len>0) {
    macroList.push_back(DivMacroExecList(ex8,ins->std.ex8Macro));
    ex8.prepare(&ins->std.ex8Macro);
  }

  // prepare FM operator macros
  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& m=ins->std.opMacros[i];
    IntOp& o=op[i];
    if (m.amMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.am,m.amMacro));
      o.am.prepare(&m.amMacro);
    }
    if (m.arMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.ar,m.arMacro));
      o.ar.prepare(&m.arMacro);
    }
    if (m.drMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.dr,m.drMacro));
      o.dr.prepare(&m.drMacro);
    }
    if (m.multMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.mult,m.multMacro));
      o.mult.prepare(&m.multMacro);
    }
    if (m.rrMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.rr,m.rrMacro));
      o.rr.prepare(&m.rrMacro);
    }
    if (m.slMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.sl,m.slMacro));
      o.sl.prepare(&m.slMacro);
    }
    if (m.tlMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.tl,m.tlMacro));
      o.tl.prepare(&m.tlMacro);
    }
    if (m.dt2Macro.len>0) {
      macroList.push_back(DivMacroExecList(o.dt2,m.dt2Macro));
      o.dt2.prepare(&m.dt2Macro);
    }
    if (m.rsMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.rs,m.rsMacro));
      o.rs.prepare(&m.rsMacro);
    }
    if (m.dtMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.dt,m.dtMacro));
      o.dt.prepare(&m.dtMacro);
    }
    if (m.d2rMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.d2r,m.d2rMacro));
      o.d2r.prepare(&m.d2rMacro);
    }
    if (m.ssgMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.ssg,m.ssgMacro));
      o.ssg.prepare(&m.ssgMacro);
    }

    if (m.damMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.dam,m.damMacro));
      o.dam.prepare(&m.damMacro);
    }
    if (m.dvbMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.dvb,m.dvbMacro));
      o.dvb.prepare(&m.dvbMacro);
    }
    if (m.egtMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.egt,m.egtMacro));
      o.egt.prepare(&m.egtMacro);
    }
    if (m.kslMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.ksl,m.kslMacro));
      o.ksl.prepare(&m.kslMacro);
    }
    if (m.susMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.sus,m.susMacro));
      o.sus.prepare(&m.susMacro);
    }
    if (m.vibMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.vib,m.vibMacro));
      o.vib.prepare(&m.vibMacro);
    }
    if (m.wsMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.ws,m.wsMacro));
      o.ws.prepare(&m.wsMacro);
    }
    if (m.ksrMacro.len>0) {
      macroList.push_back(DivMacroExecList(o.ksr,m.ksrMacro));
      o.ksr.prepare(&m.ksrMacro);
    }
  }

  // prepare wavesynth macros
  if (ins->std.ws.wave1Macro.len>0) {
    macroList.push_back(DivMacroExecList(ws.wave1,ins->std.ws.wave1Macro));
    ws.wave1.prepare(&ins->std.ws.wave1Macro);
  }
  if (ins->std.ws.wave2Macro.len>0) {
    macroList.push_back(DivMacroExecList(ws.wave2,ins->std.ws.wave2Macro));
    ws.wave2.prepare(&ins->std.ws.wave2Macro);
  }
  if (ins->std.ws.rateDividerMacro.len>0) {
    macroList.push_back(DivMacroExecList(ws.rateDivider,ins->std.ws.rateDividerMacro));
    ws.rateDivider.prepare(&ins->std.ws.rateDividerMacro);
  }
  if (ins->std.ws.effectMacro.len>0) {
    macroList.push_back(DivMacroExecList(ws.effect,ins->std.ws.effectMacro));
    ws.effect.prepare(&ins->std.ws.effectMacro);
  }
  if (ins->std.ws.oneShotMacro.len>0) {
    macroList.push_back(DivMacroExecList(ws.oneShot,ins->std.ws.oneShotMacro));
    ws.oneShot.prepare(&ins->std.ws.oneShotMacro);
  }
  if (ins->std.ws.enabledMacro.len>0) {
    macroList.push_back(DivMacroExecList(ws.enabled,ins->std.ws.enabledMacro));
    ws.enabled.prepare(&ins->std.ws.enabledMacro);
  }
  if (ins->std.ws.globalMacro.len>0) {
    macroList.push_back(DivMacroExecList(ws.global,ins->std.ws.globalMacro));
    ws.global.prepare(&ins->std.ws.globalMacro);
  }
  if (ins->std.ws.speedMacro.len>0) {
    macroList.push_back(DivMacroExecList(ws.speed,ins->std.ws.speedMacro));
    ws.speed.prepare(&ins->std.ws.speedMacro);
  }
  if (ins->std.ws.param1Macro.len>0) {
    macroList.push_back(DivMacroExecList(ws.param1,ins->std.ws.param1Macro));
    ws.param1.prepare(&ins->std.ws.param1Macro);
  }
  if (ins->std.ws.param2Macro.len>0) {
    macroList.push_back(DivMacroExecList(ws.param2,ins->std.ws.param2Macro));
    ws.param2.prepare(&ins->std.ws.param2Macro);
  }
  if (ins->std.ws.param3Macro.len>0) {
    macroList.push_back(DivMacroExecList(ws.param3,ins->std.ws.param3Macro));
    ws.param3.prepare(&ins->std.ws.param3Macro);
  }
  if (ins->std.ws.param4Macro.len>0) {
    macroList.push_back(DivMacroExecList(ws.param4,ins->std.ws.param4Macro));
    ws.param4.prepare(&ins->std.ws.param4Macro);
  }
  if (!macroList.empty()) {
      for (std::list<DivMacroExecList>::iterator iter = macroList.begin(); iter!= macroList.end(); iter++) {
        iter->prepare();
    }
  }
}

void DivMacroInt::notifyInsDeletion(DivInstrument* which) {
  if (ins==which) {
    init(NULL);
  }
}
