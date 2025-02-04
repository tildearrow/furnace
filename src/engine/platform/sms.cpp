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

#include "sms.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>

#define rWrite(a,v) {if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);}}}

const char* regCheatSheetSN[]={
  "DATA", "0",
  NULL
};

const char* regCheatSheetGG[]={
  "DATA", "0",
  "Stereo", "1",
  NULL
};

const char** DivPlatformSMS::getRegisterSheet() {
  return stereo?regCheatSheetGG:regCheatSheetSN;
}

float DivPlatformSMS::getPostAmp() {
  return 1.5f;
}

void DivPlatformSMS::poolWrite(unsigned short a, unsigned char v) {
  if (a) {
    regPool[9]=v;
  } else {
    if (v>=0x80) {
      regPool[(v>>4)&7]&=~15;
      regPool[(v>>4)&7]|=v&15;
      chanLatch=(v>>5)&3;
    } else {
      regPool[chanLatch<<1]&=15;
      regPool[chanLatch<<1]|=((v&15)<<4);
      regPool[1+(chanLatch<<1)]&=15;
      regPool[1+(chanLatch<<1)]|=v&0xf0;
    }
  }
}

void DivPlatformSMS::acquire_nuked(short** buf, size_t len) {
  int oL=0;
  int oR=0;
  for (size_t h=0; h<len; h++) {
    if (!writes.empty()) {
      QueuedWrite w=writes.front();
      if (w.addr==0) {
        YMPSG_Write(&sn_nuked,w.val);
      } else if (w.addr==1) {
        YMPSG_WriteStereo(&sn_nuked,w.val);
      }

      poolWrite(w.addr,w.val);

      writes.pop();
    }
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_GetOutput(&sn_nuked,&oL,&oR);
    if (oL<-32768) oL=-32768;
    if (oL>32767) oL=32767;
    if (oR<-32768) oR=-32768;
    if (oR>32767) oR=32767;
    buf[0][h]=oL;
    if (stereo) buf[1][h]=oR;
    for (int i=0; i<4; i++) {
      if (isMuted[i]) {
        oscBuf[i]->data[oscBuf[i]->needle++]=0;
      } else {
        oscBuf[i]->data[oscBuf[i]->needle++]=sn_nuked.vol_table[sn_nuked.volume_out[i]]*3;
      }
    }
  }
}

void DivPlatformSMS::acquire_mame(short** buf, size_t len) {
  while (!writes.empty()) {
    QueuedWrite w=writes.front();
    if (stereo && (w.addr==1))
      sn->stereo_w(w.val);
    else if (w.addr==0) {
      sn->write(w.val);
    }

    poolWrite(w.addr,w.val);

    writes.pop();
  }
  for (size_t h=0; h<len; h++) {
    short* outs[2]={
      &buf[0][h],
      stereo?(&buf[1][h]):NULL
    };
    sn->sound_stream_update(outs,1);
    for (int i=0; i<4; i++) {
      if (isMuted[i]) {
        oscBuf[i]->data[oscBuf[i]->needle++]=0;
      } else {
        oscBuf[i]->data[oscBuf[i]->needle++]=sn->get_channel_output(i)*3;
      }
    }
  }
}

void DivPlatformSMS::acquire(short** buf, size_t len) {
  if (nuked) {
    acquire_nuked(buf,len);
  } else {
    acquire_mame(buf,len);
  }
}

double DivPlatformSMS::NOTE_SN(int ch, int note) {
  double CHIP_DIVIDER=toneDivider;
  if (ch==3) CHIP_DIVIDER=noiseDivider;
  if (parent->song.linearPitch==2 || !easyNoise) {
    return NOTE_PERIODIC(note);
  }
  int easyStartingPeriod=16;
  int easyThreshold=round(12.0*log((chipClock/(easyStartingPeriod*CHIP_DIVIDER))/(0.0625*parent->song.tuning))/log(2.0))-3;
  if (note>easyThreshold) {
    return MAX(0,easyStartingPeriod-(note-easyThreshold));
  }
  return NOTE_PERIODIC(note);
}

int DivPlatformSMS::snCalcFreq(int ch) {
  double CHIP_DIVIDER=toneDivider;
  if (ch==3) CHIP_DIVIDER=noiseDivider;
  int easyStartingPeriod=16;
  int easyThreshold=round(128.0*12.0*log((chipClock/(easyStartingPeriod*CHIP_DIVIDER))/(0.0625*parent->song.tuning))/log(2.0))-384+64;
  int curFreq=chan[ch].baseFreq+chan[ch].pitch+chan[ch].pitch2+(chan[ch].arpOff<<7);
  if (chan[ch].fixedArp) {
    curFreq=chan[ch].baseNoteOverride<<7;
  }
  if (parent->song.linearPitch==2 && easyNoise && curFreq>easyThreshold) {
    int ret=(((easyStartingPeriod<<7))-(curFreq-(easyThreshold)))>>7;
    if (ret<0) ret=0;
    return ret;
  }
  return parent->calcFreq(chan[ch].baseFreq,chan[ch].pitch,chan[ch].fixedArp?chan[ch].baseNoteOverride:chan[ch].arpOff,chan[ch].fixedArp,true,0,chan[ch].pitch2,chipClock,CHIP_DIVIDER);
}

void DivPlatformSMS::tick(bool sysTick) {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LOG(chan[i].std.vol.val,chan[i].vol,15);
      if (chan[i].outVol<0) chan[i].outVol=0;
      // old formula
      // ((chan[i].vol&15)*MIN(15,chan[i].std.vol.val))>>4;
      chan[i].writeVol=true;
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        // TODO: add compatibility flag. this is horrible.
        int areYouSerious=parent->calcArp(chan[i].note,chan[i].std.arp.val);
        if (!easyNoise) while (areYouSerious>0x60) areYouSerious-=12;
        chan[i].baseFreq=NOTE_SN(i,areYouSerious);
        chan[i].actualNote=areYouSerious;
        chan[i].freqChanged=true;
      }
    }
    if (i==3) {
      if (chan[i].std.duty.had) {
        if (chan[i].std.duty.val!=snNoiseMode || parent->song.snDutyReset) {
          snNoiseMode=chan[i].std.duty.val;
          if (chan[i].std.duty.val<2) {
            chan[3].freqChanged=false;
          }
          updateSNMode=true;
        }
      }
      if (chan[i].std.phaseReset.had) {
        if (chan[i].std.phaseReset.val==1) {
          updateSNMode=true;
        }
      }
    }
    if (stereo) {
      if (chan[i].std.panL.had) {
        lastPan&=~(0x11<<i);
        lastPan|=((chan[i].std.panL.val&1)<<i)|(((chan[i].std.panL.val>>1)&1)<<(i+4));
        rWrite(1,lastPan);
      }
    }
    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-32768,32767);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }
  }
  for (int i=0; i<3; i++) {
    if (chan[i].freqChanged) {
      chan[i].freq=snCalcFreq(i);
      if (chan[i].freq>1023) chan[i].freq=1023;
      if (parent->song.snNoLowPeriods) {
        if (chan[i].freq<8) chan[i].freq=1;
      } else {
        if (chan[i].freq<0) chan[i].freq=0;
      }
      //if (chan[i].actualNote>0x5d) chan[i].freq=0x01;
      rWrite(0,0x80|i<<5|(chan[i].freq&15));
      rWrite(0,chan[i].freq>>4);
      // what?
      /*if (i==2 && snNoiseMode&2) {
        chan[3].baseFreq=chan[2].baseFreq;
        chan[3].actualNote=chan[2].actualNote;
      }*/
      chan[i].freqChanged=false;
    }
  }
  if (chan[3].freqChanged || updateSNMode) {
    chan[3].freq=snCalcFreq(3);
    //parent->calcFreq(chan[3].baseFreq,chan[3].pitch,chan[3].fixedArp?chan[3].baseNoteOverride:chan[3].arpOff,chan[3].fixedArp,true,0,chan[3].pitch2,chipClock,noiseDivider);
    if (chan[3].freq>1023) chan[3].freq=1023;
    if (parent->song.snNoLowPeriods) {
      if (chan[3].actualNote>0x5d) chan[3].freq=0x01;
    }
    if (chan[3].freq<0) chan[3].freq=0;
    if (snNoiseMode&2) { // take period from channel 3
      if (updateSNMode || resetPhase) {
        if (snNoiseMode&1) {
          rWrite(0,0xe7);
        } else {
          rWrite(0,0xe3);
        }
        if (updateSNMode) {
          rWrite(0,0xdf);
        }
      }
      
      if (chan[3].freqChanged) {
        rWrite(0,0xc0|(chan[3].freq&15));
        rWrite(0,chan[3].freq>>4);
      }
    } else { // 3 fixed values
      unsigned char value;
      // TODO: new arp?
      if (chan[3].std.arp.had) {
        value=parent->calcArp(chan[3].note,chan[3].std.arp.val)%12;
      } else { // pardon?
        value=chan[3].note%12;
      }
      if (value<3) {
        value=2-value;
        if (value!=oldValue || updateSNMode || resetPhase) {
          oldValue=value;
          rWrite(0,0xe0|value|((snNoiseMode&1)<<2));
        }
      }
    }
    chan[3].freqChanged=false;
    updateSNMode=false;
  }
  for (int i=0; i<4; i++) {
    if (chan[i].writeVol) {
      rWrite(0,0x90|(i<<5)|(isMuted[i]?15:(15-(chan[i].outVol&15))));
      chan[i].writeVol=false;
    }
    if (chan[i].keyOff) {
      rWrite(0,0x9f|i<<5);
      chan[i].keyOff=false;
    }
  }
}

int DivPlatformSMS::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_SN(c.chan,c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
        chan[c.chan].actualNote=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOff=false;
      //if (!parent->song.brokenOutVol2) {
        chan[c.chan].writeVol=true;
        chan[c.chan].outVol=chan[c.chan].vol;
        //rWrite(0,0x90|c.chan<<5|(isMuted[c.chan]?15:(15-(chan[c.chan].vol&15))));
      //}
      chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_STD));
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
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
      chan[c.chan].ins=c.value;
      //chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_STD));
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
        }
        if (chan[c.chan].active) {
          chan[c.chan].writeVol=true;
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
      int destFreq=NOTE_SN(c.chan,c.value2);
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
    case DIV_CMD_STD_NOISE_MODE:
      snNoiseMode=(c.value&1)|((c.value&16)>>3);
      updateSNMode=true;
      break;
    case DIV_CMD_PANNING: {
      if (stereo) {
        if (c.chan>3) c.chan=3;
        lastPan&=~(0x11<<c.chan);
        int pan=0;
        if (c.value>0) pan|=0x10;
        if (c.value2>0) pan|=0x01;
        if (pan==0) pan=0x11;
        lastPan|=pan<<c.chan;
        rWrite(1,lastPan);
      }
      break;
    }
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_SN(c.chan,c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      chan[c.chan].actualNote=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_STD));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_SN(c.chan,chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
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

void DivPlatformSMS::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (chan[ch].active) chan[ch].writeVol=true;
}

void DivPlatformSMS::forceIns() {
  for (int i=0; i<4; i++) {
    if (chan[i].active) {
      chan[i].insChanged=true;
      chan[i].freqChanged=true;
    }
  }
  updateSNMode=true;
}

void* DivPlatformSMS::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformSMS::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformSMS::getPan(int ch) {
  if (!stereo) return 0;
  unsigned char p=lastPan&(0x11<<ch);
  return ((p&0xf0)?0x100:0)|((p&0x0f)?1:0);
}

DivDispatchOscBuffer* DivPlatformSMS::getOscBuffer(int ch) {
  return oscBuf[ch];
}

int DivPlatformSMS::mapVelocity(int ch, float vel) {
  return round(15.0*pow(vel,0.33));
}

float DivPlatformSMS::getGain(int ch, int vol) {
  if (vol==0) return 0;
  return 1.0/pow(10.0,(float)(15-vol)*2.0/20.0);
}

unsigned char* DivPlatformSMS::getRegisterPool() {
  return regPool;
}

int DivPlatformSMS::getRegisterPoolSize() {
  return stereo?9:8;
}

void DivPlatformSMS::reset() {
  memset(regPool,0,16);
  chanLatch=0;
  while (!writes.empty()) writes.pop();
  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformSMS::Channel();
    chan[i].std.setEngine(parent);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  sn->device_start();
  YMPSG_Init(&sn_nuked,isRealSN,12,isRealSN?13:15,isRealSN?16383:32767);
  snNoiseMode=3;
  rWrite(0,0xe7);
  updateSNMode=true;
  oldValue=0xff;
  lastPan=0xff;
  if (stereo) {
    rWrite(1,0xff);
  }
}

int DivPlatformSMS::getOutputCount() {
  return stereo?2:1;
}

bool DivPlatformSMS::keyOffAffectsArp(int ch) {
  return true;
}

bool DivPlatformSMS::keyOffAffectsPorta(int ch) {
  return true;
}

bool DivPlatformSMS::getLegacyAlwaysSetVolume() {
  return false;
}

int DivPlatformSMS::getPortaFloor(int ch) {
  return 12;
}

void DivPlatformSMS::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformSMS::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformSMS::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

void DivPlatformSMS::setFlags(const DivConfig& flags) {
  switch (flags.getInt("clockSel",0)) {
    case 1:
      chipClock=COLOR_PAL*4.0/5.0;
      break;
    case 2:
      chipClock=4000000;
      break;
    case 3:
      chipClock=COLOR_NTSC/2.0;
      break;
    case 4:
      chipClock=3000000;
      break;
    case 5:
      chipClock=2000000;
      break;
    case 6:
      chipClock=COLOR_NTSC/8.0;
      break;
    default:
      chipClock=COLOR_NTSC;
      break;
  }
  CHECK_CUSTOM_CLOCK;
  resetPhase=!flags.getBool("noPhaseReset",false);
  easyNoise=!flags.getBool("noEasyNoise",false);
  divider=16;
  toneDivider=64.0;
  noiseDivider=64.0;
  if (sn!=NULL) delete sn;
  switch (flags.getInt("chipType",0)) {
    case 1: // TI SN76489
      sn=new sn76489_device();
      isRealSN=true;
      stereo=false;
      noiseDivider=60.0; // 64 for match to tone frequency on non-Sega PSG but compatibility
      break;
    case 2: // TI+Atari
      sn=new sn76496_base_device(0x4000, 0x0f35, 0x01, 0x02, true, false, 1/*8*/, false, true);
      isRealSN=true;
      stereo=false;
      noiseDivider=60.0;
      break;
    case 3: // Game Gear (not fully emulated yet!)
      sn=new gamegear_device();
      isRealSN=false;
      stereo=true;
      break;
    case 4: // TI SN76489A
      sn=new sn76489a_device();
      isRealSN=false; // TODO
      stereo=false;
      noiseDivider=60.0;
      break;
    case 5: // TI SN76496
      sn=new sn76496_device();
      isRealSN=false; // TODO
      stereo=false;
      noiseDivider=60.0;
      break;
    case 6: // NCR 8496
      sn=new ncr8496_device();
      isRealSN=false;
      stereo=false;
      noiseDivider=60.0;
      break;
    case 7: // Tandy PSSJ 3-voice sound
      sn=new pssj3_device();
      isRealSN=false;
      stereo=false;
      noiseDivider=60.0;
      break;
    case 8: // TI SN94624
      sn=new sn94624_device();
      isRealSN=true;
      stereo=false;
      divider=2;
      toneDivider=8.0;
      noiseDivider=7.5;
      break;
    case 9: // TI SN76494
      sn=new sn76494_device();
      isRealSN=false; // TODO
      stereo=false;
      divider=2;
      toneDivider=8.0;
      noiseDivider=7.5;
      break;
    default: // Sega
      sn=new segapsg_device();
      isRealSN=false;
      stereo=false;
      break;
  }

  rate=chipClock/divider;
  for (int i=0; i<4; i++) {
    oscBuf[i]->rate=rate;
  }
}

void DivPlatformSMS::setNuked(bool value) {
  nuked=value;
}

int DivPlatformSMS::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  resetPhase=false;
  oldValue=0xff;
  lastPan=0xff;
  for (int i=0; i<4; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  sn=NULL;
  setFlags(flags);
  reset();
  return 4;
}

void DivPlatformSMS::quit() {
  for (int i=0; i<4; i++) {
    delete oscBuf[i];
  }
  if (sn!=NULL) delete sn;
}

DivPlatformSMS::~DivPlatformSMS() {
}
