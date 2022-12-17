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
#include "engine.h"

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
  linger=(source.name=="vol" && e->song.volMacroLinger);
  lfoPos=LFO_PHASE;
}

void DivMacroStruct::doMacro(DivInstrumentMacro& source, bool released, bool tick) {
  if (!tick) {
    had=false;
    return;
  }
  if (masked) {
    had=false;
    return;
  }
  if (delay>0) {
    delay--;
    had=false;
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
      val=ADSR_LOW+((pos+(ADSR_HIGH-ADSR_LOW)*pos)>>8);
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
      val=ADSR_LOW+((lfoOut+(ADSR_HIGH-ADSR_LOW)*lfoOut)>>8);
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

    CONSIDER_OP(0,0x20)
    CONSIDER_OP(2,0x40)
    CONSIDER_OP(1,0x60)
    CONSIDER_OP(3,0x80)
  }
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
  macroList[macroListLen]=&m; \
  macroSource[macroListLen++]=&s;

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
      if ((macroSource[i]->open&6)==4) {
        hasRelease=false;
      } else if ((macroSource[i]->open&6)==2) {
        hasRelease=true;
      } else {
        hasRelease=(macroSource[i]->rel<macroSource[i]->len);
      }
    } else {
      hasRelease=false;
    }
  }
}

void DivMacroInt::notifyInsDeletion(DivInstrument* which) {
  if (ins==which) {
    init(NULL);
  }
}

// randomly-generated
constexpr unsigned int hashTable[256]={
  0x0718657, 0xe904eb33, 0x14b2da2b, 0x0ef67ca9,
  0x0f0559a, 0x4142065a, 0x4d9ab4ba, 0x3cdd601a,
  0x6635aca, 0x2c41ab72, 0xf98e8d31, 0x1003ee63,
  0x3fd9fb5, 0x30734d16, 0xe8964431, 0x29bb9b79,
  0x817f580, 0xfe083b9e, 0x974b5e85, 0x3b5729c2,
  0x2afea96, 0xf1573b4b, 0x308a1024, 0xaa94b92d,
  0x693fa93, 0x547ba3da, 0xac4f206c, 0x93f72ea9,
  0xcc44001, 0x37e27670, 0xf35a63d0, 0xd1cdbb92,
  0x7c7ee24, 0xfa267ee9, 0xf9cd9956, 0x6a6768d4,
  0x9e6a108, 0xf6ca4bd0, 0xa53cba9f, 0x526a523a,
  0xf46f0c8, 0xf131bd4c, 0x82800d48, 0xabff9214,
  0x40eabd4, 0xea0ef8f7, 0xdc3968d6, 0x54c3cb63,
  0x8855023, 0xaab73861, 0xff0bea2c, 0x139b9765,
  0x4a21279, 0x6b2aa29a, 0xf147cc3f, 0xc42edc1a,
  0xfe2f86f, 0x6d352047, 0xd3cac3e4, 0x35e5c389,
  0xe923727, 0x12fe3b32, 0x204295c5, 0x254a8b7a,
  0xc1d995d, 0x26a512d2, 0xa3e34033, 0x9a968df0,
  0x53447ed, 0x36cf4077, 0x189b03a7, 0x558790e8,
  0x01f921a, 0x840f260c, 0x93dd2b86, 0x12f69cb0,
  0x117d93a, 0xcb2cbc2b, 0xd41e3aed, 0x5ff6ec75,
  0x607290d, 0xd41adb92, 0x64f94ba7, 0xaff720f7,
  0x6bf1d5d, 0xc8e36c6d, 0x7095bab5, 0xdfbf7b0d,
  0x01ddeea, 0xe8f262da, 0xf589512f, 0xc2ecac5d,
  0xbe29d98, 0xff8b5a2e, 0x18e7279e, 0x6ad24dcb,
  0x2b3b9b1, 0x6f5227d8, 0x076d7553, 0x6c5856e2,
  0x995f655, 0xe9fcf5a6, 0x83671b70, 0xaf3aed1e,
  0xac340f0, 0x5c7008b4, 0x14651282, 0x8bf855b9,
  0x4a933af, 0x829b87f1, 0x9a673070, 0xb19da64f,
  0x77d8f36, 0x584c9fdc, 0xa9e52c0d, 0x6da5e13d,
  0xae1051f, 0xe85e976f, 0xfeac2d9a, 0x19c46754,
  0x1cba6f3, 0xaf21bc31, 0x16b6a8d4, 0xe08b0fdb,
  0x97e6e54, 0x5da499ae, 0xab472e19, 0xc2491f2e,
  0xc08c563, 0xe91b131b, 0xc8e22451, 0x6995c8fe,
  0x7042718, 0x01043738, 0xc7d88b28, 0x2d9f330f,
  0x4b3aae5, 0xf1e705ba, 0xc5b8ee59, 0xa8ba4e8f,
  0x55f65a2, 0xa1899e41, 0x296243c8, 0x1e502bf2,
  0x20080de, 0x841d2239, 0x37b082af, 0xbdd7f7da,
  0x4075090, 0x1dc7dc49, 0x5cd3c69a, 0x7fb13b62,
  0xb382bf1, 0xa0cfbc2f, 0x9eca4dc1, 0xb9355453,
  0x5d0dd24, 0x834f4d8e, 0xe9b136b2, 0xe7b8738d,
  0x1c91d41, 0x8cb3ddb5, 0xdc600590, 0x607cff55,
  0x2ca7675, 0x4622a8e4, 0x9340e414, 0xcb44928a,
  0xa9e791c, 0x68849920, 0xc5b5fcd8, 0xbc352269,
  0x3ab13cf, 0xaa3cbbd0, 0x1abacc64, 0x623b5b49,
  0xcc8c4c3, 0x3c8f2f70, 0x3e584a28, 0x9316d24d,
  0xfe315a2, 0x10f0ba7a, 0xed15a523, 0x4f987369,
  0x7aa4a4a, 0x90eaf98f, 0xcf0af610, 0x1b38f4e7,
  0x19df72d, 0xd8306808, 0xd54e25ac, 0x76b79c6d,
  0x58110cf, 0x06a3e5f2, 0x873a6039, 0xf52684e3,
  0xecf39c3, 0x7cbb2759, 0xe280d361, 0x91e8471a,
  0xa67cdd3, 0x17cac3be, 0xfc9eff1f, 0x71abdf49,
  0x6168624, 0xb68f86f7, 0x67a8e72a, 0xe746911d,
  0xca48fd7, 0x8f3cc436, 0x3a3851a8, 0x30a7e26e,
  0xca49308, 0xb598ef74, 0x49ef167a, 0xa9e17632,
  0x0f7308a, 0xf156efed, 0xcf799645, 0xbae4b85a,
  0xecba3fe, 0xd97f861d, 0xc164af62, 0xb1aca42f,
  0xf249576, 0x83d1bf4e, 0x2f486a9c, 0xd3b53cc2,
  0x17d7c26, 0xd95ddae1, 0x76c1a2f5, 0xf8af6782,
  0xdbaece4, 0x010b2b53, 0x049be200, 0xd9fd0d1a,
  0x37d7e6c, 0x5b848651, 0x203c98c7, 0x669681b0,
  0x683086f, 0xdd0ee8ab, 0x5dbe008b, 0xe5d0690d,
  0x23dd758, 0x6b34acbc, 0x4b2b3e65, 0xcc7b56c1,
  0x196b0a0, 0x7b065105, 0xb731b01a, 0xd37daa16,
  0xf77816b, 0x3c9fa546, 0x81dfadb8, 0x39b1fb8b
};

constexpr unsigned int NAME_HASH(const char* name) {
  unsigned int nameHash=0xffffffff;
  for (const char* i=name; *i; i++) {
    nameHash=(nameHash>>8)^hashTable[(unsigned char)*i];
  }
  return nameHash;
}

#define CONSIDER(x) case NAME_HASH(#x): return &x; break;

DivMacroStruct* DivMacroInt::structByName(const String& name) {
  unsigned int hash=NAME_HASH(name.c_str());

  switch (hash) {
    CONSIDER(vol)
    CONSIDER(arp)
    CONSIDER(duty)
    CONSIDER(wave)
    CONSIDER(pitch)
    CONSIDER(ex1)
    CONSIDER(ex2)
    CONSIDER(ex3)
    CONSIDER(alg)
    CONSIDER(fb)
    CONSIDER(fms)
    CONSIDER(ams)
    CONSIDER(panL)
    CONSIDER(panR)
    CONSIDER(phaseReset)
    CONSIDER(ex4)
    CONSIDER(ex5)
    CONSIDER(ex6)
    CONSIDER(ex7)
    CONSIDER(ex8)
  }

  return NULL;
}

#undef CONSIDER