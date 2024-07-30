/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#include "sid3.h"
#include "../engine.h"
#include "IconsFontAwesome4.h"
#include <math.h>
#include "../../ta-log.h"

#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_FREQBASE 524288

const char* regCheatSheetSID3[]={
  "FreqL0", "00",
  "FreqH0", "01",
  "PWL0", "02",
  "PWH0Vol", "03",
  "Control0", "04",
  "AtkDcy0", "05",
  "StnRis0", "06",
  "FreqL1", "07",
  "FreqH1", "08",
  "PWL1", "09",
  "PWH1Vol", "0A",
  "Control1", "0B",
  "AtkDcy1", "0C",
  "StnRis1", "0D",
  "FreqL2", "0E",
  "FreqH2", "0F",
  "PWL2", "10",
  "PWH2Vol", "11",
  "Control2", "12",
  "AtkDcy2", "13",
  "StnRis2", "14",

  "FCL0Ctrl", "15",
  "FCH0", "16",
  "FilterRes0", "17",

  "FCL1Ctrl", "18",
  "FCH1", "19",
  "FilterRes1", "1A",

  "FCL2Ctrl", "1B",
  "FCH2", "1C",
  "FilterRes2", "1D",

  "NoiModeFrMSB01", "1E",
  "WaveMixModeFrMSB2", "1F",
  NULL
};

const char** DivPlatformSID3::getRegisterSheet() {
  return regCheatSheetSID3;
}

void DivPlatformSID3::acquire(short** buf, size_t len) 
{
  for (size_t i=0; i<len; i++) 
  {
    if (!writes.empty()) 
    {
      QueuedWrite w=writes.front();
      sid3_write(sid3, w.addr,w.val);
      regPool[w.addr % SID3_NUM_REGISTERS]=w.val;
      writes.pop();
    }
    
    sid3_clock(sid3);

    buf[0][i]=sid3->output_l;
    buf[1][i]=sid3->output_r;

    if (++writeOscBuf>=16) 
    {
      writeOscBuf=0;

      for(int j = 0; j < SID3_NUM_CHANNELS; j++)
      {
        oscBuf[j]->data[oscBuf[j]->needle++] = sid3->channel_output[j] / 4;
      }
    }
  }
}

void DivPlatformSID3::updateFilter(int channel) 
{
  rWrite(0x15 + 3 * channel,(chan[channel].filtCut&15) | ((chan[channel].filtControl & 7) << 4) | (chan[channel].filter << 7));
  rWrite(0x16 + 3 * channel,(chan[channel].filtCut >> 4));
  rWrite(0x17 + 3 * channel,chan[channel].filtRes);
}

void DivPlatformSID3::tick(bool sysTick) {
  for (int i=0; i<SID3_NUM_CHANNELS; i++) {
    chan[i].std.next();
  }
}

int DivPlatformSID3::dispatch(DivCommand c) {
  if (c.chan>SID3_NUM_CHANNELS - 1) return 0;
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_SID3);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].test=false;

      if (chan[c.chan].insChanged || chan[c.chan].resetDuty || ins->std.waveMacro.len>0) {
        chan[c.chan].duty=ins->c64.duty;
        rWrite(c.chan*7+2,chan[c.chan].duty&0xff);
        rWrite(c.chan*7+3,(chan[c.chan].duty>>8) | (chan[c.chan].outVol << 4));
      }
      if (chan[c.chan].insChanged) {
        /*chan[c.chan].wave = (ins->c64.noiseOn << 3) | (ins->c64.pulseOn << 2) | (ins->c64.sawOn << 1) | (int)(ins->c64.triOn);
        chan[c.chan].attack=ins->c64.a;
        chan[c.chan].decay=(ins->c64.s==15)?0:ins->c64.d;
        chan[c.chan].sustain=ins->c64.s;
        chan[c.chan].release=ins->c64.r;
        chan[c.chan].ring=ins->c64.ringMod;
        chan[c.chan].sync=ins->c64.oscSync;

        chan[c.chan].noise_mode = ins->sid3.noiseMode;
        chan[c.chan].mix_mode = ins->sid3.mixMode;*/
      }
      if (chan[c.chan].insChanged || chan[c.chan].resetFilter) {
        chan[c.chan].filter=ins->c64.toFilter;
        if (ins->c64.initFilter) {
          chan[c.chan].filtCut=ins->c64.cut;
          chan[c.chan].filtRes=ins->c64.res;
          chan[c.chan].filtControl=(int)(ins->c64.lp)|(ins->c64.bp<<1)|(ins->c64.hp<<2);
        }
        updateFilter(c.chan);
      }
      if (chan[c.chan].insChanged) {
        chan[c.chan].insChanged=false;
      }
      chan[c.chan].macroInit(ins);
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      //chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].std.release();
      break;
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].insChanged=true;
        chan[c.chan].ins=c.value;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
          chan[c.chan].vol=chan[c.chan].outVol;
          rWrite(c.chan*7+3,(chan[c.chan].duty>>8) | (chan[c.chan].vol << 4));
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      if (chan[c.chan].std.vol.has) {
        return chan[c.chan].vol;
      }
      return chan[c.chan].outVol;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_FREQUENCY(c.value2);
      bool return2=false;
      if (destFreq>chan[c.chan].baseFreq) {
        chan[c.chan].baseFreq+=c.value;
        if (chan[c.chan].baseFreq>=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      } else {
        chan[c.chan].baseFreq-=c.value;
        if (chan[c.chan].baseFreq<=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      }
      chan[c.chan].freqChanged=true;
      if (return2) {
        chan[c.chan].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta || parent->song.preNoteNoEffect) {
          chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_SID3));
          chan[c.chan].keyOn=true;
        }
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_FREQUENCY(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return SID3_MAX_VOL;
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

void DivPlatformSID3::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  sid3_set_is_muted(sid3,ch,mute);
}

void DivPlatformSID3::forceIns() {
  for (int i=0; i<SID3_NUM_CHANNELS; i++) {
    chan[i].insChanged=true;
    if (chan[i].active) {
      chan[i].keyOn=true;
      chan[i].freqChanged=true;
    }
    updateFilter(i);
  }
}

void DivPlatformSID3::notifyInsChange(int ins) {
  for (int i=0; i<SID3_NUM_CHANNELS; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformSID3::notifyInsDeletion(void* ins) {
  for (int i=0; i<SID3_NUM_CHANNELS; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void* DivPlatformSID3::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformSID3::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivChannelModeHints DivPlatformSID3::getModeHints(int ch) {
  DivChannelModeHints ret;
  ret.count=1;
  ret.hint[0]=ICON_FA_BELL_SLASH_O;
  ret.type[0]=0;
  if (ch == 2 && (chan[ch].filtControl & 8)) {
      ret.type[0] = 7;
  }
  else if (!chan[ch].gate) {
    ret.type[0]=4;
  }

  return ret;
}

DivDispatchOscBuffer* DivPlatformSID3::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformSID3::getRegisterPool() {
  return regPool;
}

int DivPlatformSID3::getRegisterPoolSize() {
  return SID3_NUM_REGISTERS;
}

float DivPlatformSID3::getPostAmp() {
  return 1.0f;
}

void DivPlatformSID3::reset() {
  while (!writes.empty()) writes.pop();
  for (int i=0; i<SID3_NUM_CHANNELS; i++) {
    chan[i]=DivPlatformSID3::Channel();
    chan[i].std.setEngine(parent);
    chan[i].vol = SID3_MAX_VOL;

    /*chan[i].filtControl = 7;
    chan[i].filtRes = 0;
    chan[i].filtCut = 4095;

    chan[i].noise_mode = 0;*/

    //rWrite(0x3 + 7 * i,0xf0);
  }

  sid3_reset(sid3);
  memset(regPool,0,SID3_NUM_REGISTERS);
}

int DivPlatformSID3::getOutputCount() {
  return 2;
}

void DivPlatformSID3::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformSID3::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

void DivPlatformSID3::setFlags(const DivConfig& flags) {
  switch (flags.getInt("clockSel",0)) {
    case 0x0: // NTSC C64
      chipClock=COLOR_NTSC*2.0/7.0;
      break;
    case 0x1: // PAL C64
      chipClock=COLOR_PAL*2.0/9.0;
      break;
    case 0x2: // SSI 2001
    default:
      chipClock=14318180.0/16.0;
      break;
  }
  CHECK_CUSTOM_CLOCK;
  rate=chipClock;
  for (int i=0; i<SID3_NUM_CHANNELS; i++) {
    oscBuf[i]->rate=rate/16;
  }
}

int DivPlatformSID3::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  writeOscBuf=0;
  
  for (int i=0; i<SID3_NUM_CHANNELS; i++) 
  {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }

  sid3 = sid3_create();

  setFlags(flags);

  reset();

  return SID3_NUM_CHANNELS;
}

void DivPlatformSID3::quit() {
  for (int i=0; i<SID3_NUM_CHANNELS; i++) 
  {
    delete oscBuf[i];
  }
  if (sid3!=NULL)
  {
    sid3_free(sid3);
    sid3 = NULL;
  }
}

DivPlatformSID3::~DivPlatformSID3() {
}