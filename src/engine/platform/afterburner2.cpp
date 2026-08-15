/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

#include "afterburner2.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>
#include <stdlib.h>

// CHIP_CLOCK_HZ / (64.0 * period)
#define CHIP_DIVIDER 64

const char* regCheatSheetAfterburner2[]={
  "PSGVol012", "00",
  "PSG3Ctrl",  "02",
  "PSGWave",   "04",
  "PSGDuty",   "06",
  "PSG1Tone",  "08",
  "PSG2Tone",  "0A",
  "PSG3Tone",  "0C",
  "Wavetable", "10",
  NULL
};

const char** DivPlatformAfterburner2::getRegisterSheet() {
  return regCheatSheetAfterburner2;
}

unsigned char* DivPlatformAfterburner2::getRegisterPool() {
  return regPool;
}

int DivPlatformAfterburner2::getRegisterPoolSize() {
  return 32;
}

int DivPlatformAfterburner2::getOutputCount() {
  return 2;
}

unsigned char DivPlatformAfterburner2::getWavetableSample(int step) {
  step&=31;
  // 2 steps (4-bit each) packed per byte, 16 bytes total ($9010-$901F)
  unsigned char byteVal=regPool[0x10+(step>>1)];
  int shift=(1-(step&1))*4;
  return (byteVal>>shift)&0x0f;
}

void DivPlatformAfterburner2::updateWave() {
  for (int i=0; i<16; i++) {
    int nibble1=ws.output[i<<1]&15;
    int nibble2=ws.output[(i<<1)+1]&15;
    rWrite(0x10+i,(nibble1<<4)|nibble2);
  }
}

void DivPlatformAfterburner2::rWrite(unsigned short addr, unsigned char val) {
  if (addr>=32) return;
  regPool[addr]=val;
  if (dumpWrites) addWrite(0x9000+addr,val);
}

void DivPlatformAfterburner2::acquire(short** buf, size_t len) {
  for (int i=0; i<AFTB2_NUM_CHANS; i++) oscBuf[i]->begin(len);

  // --- pull current control-register state ---
  unsigned short v012=(regPool[0x00]<<8)|regPool[0x01];
  masterVol=(v012>>8)&0x0f;
  psgVol[0]=(v012>>4)&0x0f;
  psgVol[1]=v012&0x0f;

  unsigned short v3ctrl=(regPool[0x02]<<8)|regPool[0x03];
  psgVol[2]=(v3ctrl>>8)&0x0f;
  psg3WaveEnable=((v3ctrl>>4)&0x0f)!=0;

  unsigned short waveSel=(regPool[0x04]<<8)|regPool[0x05];
  unsigned char waveType[3]={
    (unsigned char)((waveSel>>8)&0x03),
    (unsigned char)((waveSel>>4)&0x03),
    (unsigned char)(waveSel&0x03)
  };
  unsigned char psg2Bitwise=(waveSel>>12)&0x0f; // 0: off, 1: AND, 2: NAND, 3: OR, 4: NOR, 5: XOR, 6: XNOR

  unsigned short dutyReg=(regPool[0x06]<<8)|regPool[0x07];
  unsigned char duties[3]={
    (unsigned char)((dutyReg>>8)&0x0f),
    (unsigned char)((dutyReg>>4)&0x0f),
    (unsigned char)(dutyReg&0x0f)
  };

  float mGain=masterVol/15.0f;

  for (size_t i=0; i<len; i++) {
    float raw[3]={0.0f,0.0f,0.0f};

    for (int ch=0; ch<3; ch++) {
      // silence when key is up (or period is zero)
      if (!chan[ch].active || chan[ch].freq==0) {
        raw[ch]=0.0f;
        oscBuf[ch]->putSample(i,0);
        continue;
      }
      unsigned short period=chan[ch].freq;
      double freq=(double)chipClock/(64.0*period);
      double phaseStep=freq/rate;
      double prevPhase=chan[ch].phase;
      chan[ch].phase=fmod(chan[ch].phase+phaseStep,1.0);
      bool wrapped=(chan[ch].phase<prevPhase);

      if (ch==2 && psg3WaveEnable) {
        int idx=((int)(chan[ch].phase*32.0)&31);
        raw[ch]=(getWavetableSample(idx)/15.0f)*2.0f-1.0f;
      } else {
        float duty=(duties[ch]>0)?(duties[ch]/15.0f):0.5f;
        switch (waveType[ch]) {
          case 0: // square
            raw[ch]=(chan[ch].phase<duty)?1.0f:-1.0f;
            break;
          case 1: { // saw
            float saw=(float)(chan[ch].phase*2.0-1.0);
            if (duties[ch]!=0) saw=-saw; // saw flip
            raw[ch]=saw;
            break;
          }
          default: // noise: 1-bit output, re-rolled once per cycle (clocked
                   // by the channel's tone period like the other waveforms),
                   // biased by duty just like the square wave's pulse width
            if (wrapped || i==0) {
              chan[ch].noiseBit=((float)(rand()%1000)/1000.0f<duty)?1.0f:-1.0f;
            }
            raw[ch]=chan[ch].noiseBit;
            break;
        }
      }
    }

    // PSG2 bitwise-with-PSG3: quantize both channels' current amplitude to
    // the chip's native 4-bit resolution and combine them with a real
    // bitwise op, sample by sample. Using only each signal's sign (a 1-bit
    // gate) made PSG3 act like a square wave regardless of its actual shape
    // -- this way a saw/wavetable's full ramp of levels comes through.
    if (psg2Bitwise!=0) {
      unsigned char v2q=(unsigned char)((raw[1]*0.5f+0.5f)*15.0f+0.5f)&0x0f;
      unsigned char v3q=(unsigned char)((raw[2]*0.5f+0.5f)*15.0f+0.5f)&0x0f;
      unsigned char combined;
      switch (psg2Bitwise) {
        case 1: combined=v2q&v3q; break;              // AND
        case 2: combined=(~(v2q&v3q))&0x0f; break;    // NAND
        case 3: combined=v2q|v3q; break;              // OR
        case 4: combined=(~(v2q|v3q))&0x0f; break;    // NOR
        case 5: combined=v2q^v3q; break;              // XOR
        case 6: combined=(~(v2q^v3q))&0x0f; break;    // XNOR
        default: combined=v2q; break;
      }
      raw[1]=(combined/15.0f)*2.0f-1.0f;
    }

    // lower mix gain so PSG levels match other chips better
    float mixL=0.0f, mixR=0.0f;
    for (int ch=0; ch<3; ch++) {
      float vol=(psgVol[ch]/15.0f)*mGain;
      float s=isMuted[ch]?0.0f:(raw[ch]*vol);
      mixL+=s*0.25f;
      mixR+=s*0.25f;
      oscBuf[ch]->putSample(i,(short)(raw[ch]*vol*16384.0f));
    }

    if (mixL>1.0f) mixL=1.0f; else if (mixL<-1.0f) mixL=-1.0f;
    if (mixR>1.0f) mixR=1.0f; else if (mixR<-1.0f) mixR=-1.0f;
    buf[0][i]=(short)(mixL*32767.0f);
    buf[1][i]=(short)(mixR*32767.0f);
  }

  for (int i=0; i<AFTB2_NUM_CHANS; i++) oscBuf[i]->end(len);
}

void DivPlatformAfterburner2::tick(bool sysTick) {
  for (int i=0; i<3; i++) {
    chan[i].std.next();

    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LINEAR(chan[i].vol,chan[i].std.vol.val,15);
    }
    if (chan[i].std.duty.had) {
      chan[i].duty=chan[i].std.duty.val&0x0f;
    }
    if (chan[i].std.wave.had) {
      // fallback shape (square/saw/noise) used when wave-enable is off;
      // for channel 2 this is set alongside waveIdx below so switching shapes
      // still works even while the wavetable is disabled.
      chan[i].wave=chan[i].std.wave.val&0x03;
    }
    if (chan[i].std.arp.had && !chan[i].inPorta) {
      chan[i].baseFreq=chan[i].calcBaseFreq(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      chan[i].freqChanged=true;
    }
    if (chan[i].std.pitch.had) {
      // "relative" pitch macro mode accumulates onto pitch2 each tick
      // instead of overwriting it -- this was previously missing, which is
      // why relative pitch macros had no audible effect.
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-32768,32767);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.wave.had && i==2) {
      if (chan[i].waveIdx!=chan[i].std.wave.val || ws.activeChanged()) {
        chan[i].waveIdx=chan[i].std.wave.val;
        ws.changeWave1(chan[i].waveIdx);
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
    }
    // EX1 = PSG2 Bitwise mode (0: off, 1: AND, 2: NAND, 3: OR, 4: NOR, 5: XOR, 6: XNOR)
    if (i==1 && chan[i].std.ex1.had) {
      int m=chan[i].std.ex1.val;
      psg2Bitwise=(unsigned char)(m<0?0:(m>6?6:m));
    }
    // EX2 = Wave Enable (for PSG3)
    if (i==2 && chan[i].std.ex2.had) {
      psg3WaveEnable=chan[i].std.ex2.val!=0;
    }
    if (i==2 && chan[i].active) {
      if (ws.tick()) {
        updateWave();
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
    }

    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=chan[i].calcFreq();
      if (chan[i].freq>0xffff) chan[i].freq=0xffff;
      // tones placed at 0x08+ so they don't collide with the control packing at 0x00-0x07
      rWrite(0x08+i*2,chan[i].freq>>8);
      rWrite(0x09+i*2,chan[i].freq&0xff);
      chan[i].keyOn=false;
      chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }

  // Pack volume registers: $9000 = 0x0V12 (master, PSG1, PSG2), $9001 = 0x0VWO (PSG3, wave en, offset)
  // Zero volume for inactive channels so the hardware (and acquire) stop sounding on note-off
  unsigned char master = masterVol & 0x0f;
  unsigned char v1 = (chan[0].active ? chan[0].outVol : 0) & 0x0f;
  unsigned char v2 = (chan[1].active ? chan[1].outVol : 0) & 0x0f;
  unsigned char v3 = (chan[2].active ? chan[2].outVol : 0) & 0x0f;
  unsigned char wEn = psg3WaveEnable ? 0x0f : 0;
  // regPool is byte-oriented; acquire reads big-endian words
  rWrite(0x00, master); // high of $9000
  rWrite(0x01, (v1 << 4) | v2); // low of $9000
  rWrite(0x02, v3); // high of $9001
  rWrite(0x03, (wEn << 4)); // low of $9001

  // Pack wave select $9002 = 0xM123 (M=PSG2 bitwise mode 0-6 in high nibble, then 2-bit waves)
  unsigned char andFlag = psg2Bitwise & 0x0f;
  unsigned char w0 = chan[0].wave & 0x03;
  unsigned char w1 = chan[1].wave & 0x03;
  unsigned char w2 = chan[2].wave & 0x03;
  rWrite(0x04, (andFlag << 4) | w0); // high of waveSel: high nibble = PSG2 bitwise mode
  rWrite(0x05, (w1 << 4) | w2); // low

  // Pack duty $9003 = 0x0123
  unsigned char d0 = chan[0].duty & 0x0f;
  unsigned char d1 = chan[1].duty & 0x0f;
  unsigned char d2 = chan[2].duty & 0x0f;
  rWrite(0x06, d0); // high
  rWrite(0x07, (d1 << 4) | d2); // low
}

int DivPlatformAfterburner2::dispatch(DivCommand c) {
  if (c.chan>=AFTB2_NUM_CHANS) return 0;

  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=chan[c.chan].calcBaseFreq(c.value);
        chan[c.chan].note=c.value;
        chan[c.chan].freqChanged=true;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_AFTERBURNER2));
      if (c.chan==2) {
        if (chan[c.chan].waveIdx<0) {
          chan[c.chan].waveIdx=0;
          ws.changeWave1(chan[c.chan].waveIdx);
        }
        ws.init(parent->getIns(chan[c.chan].ins,DIV_INS_AFTERBURNER2),32,15,chan[c.chan].insChanged);
      }
      if (!chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].ins=c.value;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
    case DIV_CMD_GET_VOLMAX:
      return 15;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_WAVE:
      // Waveform select (0=square, 1=saw, 2=noise). For PSG3, if wave enable
      // is on this is ignored in favor of the wavetable. Wavetable contents
      // themselves are uploaded via the WaveSynth / wave editor into $9010-$9017.
      chan[c.chan].wave=c.value&0x03;
      if (c.chan==2) {
        if (chan[c.chan].waveIdx!=c.value || ws.activeChanged()) {
          chan[c.chan].waveIdx=c.value;
          ws.changeWave1(chan[c.chan].waveIdx);
        }
      }
      break;
    case DIV_CMD_STD_NOISE_MODE:
      chan[c.chan].duty = c.value&0x0F;
      break;
    case DIV_CMD_AFTB2_PSG2_BITWISE: {
      int m=c.value;
      psg2Bitwise=(unsigned char)(m<0?0:(m>6?6:m));
      break;
    }
    case DIV_CMD_AFTB2_WAVE_ENABLE:
      psg3WaveEnable=(c.value!=0);
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=chan[c.chan].calcBaseFreq(c.value2);
      bool return2=false;
      if (destFreq>chan[c.chan].baseFreq) {
        chan[c.chan].baseFreq+=c.value;
        if (chan[c.chan].baseFreq>=destFreq) { chan[c.chan].baseFreq=destFreq; return2=true; }
      } else {
        chan[c.chan].baseFreq-=c.value;
        if (chan[c.chan].baseFreq<=destFreq) { chan[c.chan].baseFreq=destFreq; return2=true; }
      }
      chan[c.chan].freqChanged=true;
      if (return2) {
        chan[c.chan].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=chan[c.chan].calcBaseFreq(c.value);
      chan[c.chan].note=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_PRE_PORTA:
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
    case DIV_CMD_MACRO_RESTART:
      chan[c.chan].std.restart(c.value);
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformAfterburner2::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformAfterburner2::forceIns() {
  for (int i=0; i<AFTB2_NUM_CHANS; i++) chan[i].insChanged=true;
  updateWave();
}

SharedChannel* DivPlatformAfterburner2::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformAfterburner2::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformAfterburner2::getOscBuffer(int ch) {
  return oscBuf[ch];
}

void DivPlatformAfterburner2::reset() {
  for (int i=0; i<AFTB2_NUM_CHANS; i++) {
    chan[i]=DivPlatformAfterburner2::Channel(parent->song.compatFlags.linearPitch);
    chan[i].std.setEngine(parent);
    chan[i].pitchTable=&pitchTable;
  }
  ws.setEngine(parent);
  ws.init(NULL,32,15,false);
  if (dumpWrites) addWrite(0xffffffff,0);

  memset(regPool,0,32);
  // initial full volume, no AND, no wave: matches packing in tick()
  regPool[0x00]=0x0f; // master
  regPool[0x01]=0xff; // v1=15, v2=15
  regPool[0x02]=0x0f; // v3=15
  regPool[0x03]=0x00; // no wave en/offset
  regPool[0x04]=0x00; // no AND, wave0=0
  regPool[0x05]=0x00;
  regPool[0x06]=0x00; // duties 0
  regPool[0x07]=0x00;
  masterVol=15;
  psgVol[0]=15;
  psgVol[1]=15;
  psgVol[2]=15;
  psg2Bitwise=0;
  psg3WaveEnable=false;
  updateWave();
}

void DivPlatformAfterburner2::setFlags(const DivConfig& flags) {
  chipClock=4000000;
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/8;
  for (int i=0; i<AFTB2_NUM_CHANS; i++) {
    if (oscBuf[i]!=NULL) oscBuf[i]->setRate(rate);
  }
  notifyPitchTable();
}

void DivPlatformAfterburner2::notifyInsDeletion(void* ins) {
  for (int i=0; i<AFTB2_NUM_CHANS; i++) chan[i].std.notifyInsDeletion((DivInstrument*)ins);
}

void DivPlatformAfterburner2::notifyWaveChange(int wave) {
  if (chan[2].waveIdx==wave) {
    ws.changeWave1(wave);
    updateWave();
    if (!chan[2].keyOff && chan[2].active) chan[2].keyOn=true;
  }
}

void DivPlatformAfterburner2::getPaired(int ch, std::vector<DivChannelPair>& ret) {
  // PSG2 shows a bubble pointing at PSG3 while a bitwise mode is active,
  // matching how C64 (ring/sync) and POKEY (AUDCTL) hint at cross-channel
  // coupling in the pattern view.
  if (ch==1 && psg2Bitwise!=0) {
    switch (psg2Bitwise) {
      case 1: ret.push_back(DivChannelPair(_("AND"),2)); break;
      case 2: ret.push_back(DivChannelPair(_("NAND"),2)); break;
      case 3: ret.push_back(DivChannelPair(_("OR"),2)); break;
      case 4: ret.push_back(DivChannelPair(_("NOR"),2)); break;
      case 5: ret.push_back(DivChannelPair(_("XOR"),2)); break;
      case 6: ret.push_back(DivChannelPair(_("XNOR"),2)); break;
      default: break;
    }
  }
}

void DivPlatformAfterburner2::notifyPitchTable(int sample) {
  pitchTable.init(parent->song.tuning,chipClock,CHIP_DIVIDER,0xffff,true,parent->song.compatFlags.linearPitch);
}

unsigned int DivPlatformAfterburner2::getMaxFreq(int ch) {
  return 0xffff;
}

void DivPlatformAfterburner2::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformAfterburner2::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformAfterburner2::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<AFTB2_NUM_CHANS; i++) {
    oscBuf[i]=new DivDispatchOscBuffer;
    isMuted[i]=false;
  }
  setFlags(flags);
  reset();
  return AFTB2_NUM_CHANS;
}

void DivPlatformAfterburner2::quit() {
  for (int i=0; i<AFTB2_NUM_CHANS; i++) delete oscBuf[i];
}

DivPlatformAfterburner2::~DivPlatformAfterburner2() {
}
