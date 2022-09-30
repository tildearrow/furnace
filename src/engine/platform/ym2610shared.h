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

#ifndef _YM2610SHARED_H
#define _YM2610SHARED_H
#include "fmshared_OPN.h"
#include "../macroInt.h"
#include "../engine.h"
#include "../../ta-log.h"
#include "ay.h"
#include "sound/ymfm/ymfm.h"
#include "sound/ymfm/ymfm_opn.h"
#include <string.h>

#define CHIP_FREQBASE fmFreqBase
#define CHIP_DIVIDER fmDivBase

class DivYM2610Interface: public ymfm::ymfm_interface {
  public:
    unsigned char* adpcmAMem;
    unsigned char* adpcmBMem;
    int sampleBank;
    uint8_t ymfm_external_read(ymfm::access_class type, uint32_t address);
    void ymfm_external_write(ymfm::access_class type, uint32_t address, uint8_t data);
    DivYM2610Interface():
      adpcmAMem(NULL),
      adpcmBMem(NULL),
      sampleBank(0) {}
};

template<int ChanNum> class DivPlatformYM2610Base: public DivPlatformOPN {
  protected:
    struct Channel {
      DivInstrumentFM state;
      unsigned char freqH, freqL;
      int freq, baseFreq, pitch, pitch2, portaPauseFreq, note, ins;
      unsigned char psgMode, autoEnvNum, autoEnvDen;
      signed char konCycles;
      bool active, insChanged, freqChanged, keyOn, keyOff, portaPause, inPorta, furnacePCM, hardReset, opMaskChanged;
      int vol, outVol;
      int sample;
      unsigned char pan, opMask;
      int macroVolMul;
      DivMacroInt std;
      void macroInit(DivInstrument* which) {
        std.init(which);
        pitch2=0;
      }
      Channel():
        freqH(0),
        freqL(0),
        freq(0),
        baseFreq(0),
        pitch(0),
        pitch2(0),
        portaPauseFreq(0),
        note(0),
        ins(-1),
        psgMode(1),
        autoEnvNum(0),
        autoEnvDen(0),
        active(false),
        insChanged(true),
        freqChanged(false),
        keyOn(false),
        keyOff(false),
        portaPause(false),
        inPorta(false),
        furnacePCM(false),
        hardReset(false),
        opMaskChanged(false),
        vol(0),
        outVol(15),
        sample(-1),
        pan(3),
        opMask(15),
        macroVolMul(255) {}
    };

    struct OpChannel {
      DivMacroInt std;
      unsigned char freqH, freqL;
      int freq, baseFreq, pitch, pitch2, portaPauseFreq, ins;
      signed char konCycles;
      bool active, insChanged, freqChanged, keyOn, keyOff, portaPause, inPorta, mask;
      int vol;
      unsigned char pan;
      // UGLY
      OpChannel():
        freqH(0),
        freqL(0),
        freq(0),
        baseFreq(0),
        pitch(0),
        pitch2(0),
        portaPauseFreq(0),
        ins(-1),
        active(false),
        insChanged(true),
        freqChanged(false),
        keyOn(false),
        keyOff(false),
        portaPause(false),
        inPorta(false),
        mask(true),
        vol(0),
        pan(3) {}
    };
    Channel chan[ChanNum];
    DivDispatchOscBuffer* oscBuf[ChanNum];
    bool isMuted[ChanNum];

    ymfm::ym2610b* fm;
    ymfm::ym2610b::output_data fmout;
    DivPlatformAY8910* ay;
  
    unsigned char* adpcmAMem;
    size_t adpcmAMemLen;
    unsigned char* adpcmBMem;
    size_t adpcmBMemLen;
    DivYM2610Interface iface;

    unsigned int sampleOffA[256];
    unsigned int sampleOffB[256];

    unsigned char sampleBank;
  
    bool extMode;
  
    unsigned char writeADPCMAOff, writeADPCMAOn;
    int globalADPCMAVolume;

    const int extChanOffs, psgChanOffs, adpcmAChanOffs, adpcmBChanOffs;
    const int chanNum=ChanNum;

    double NOTE_OPNB(int ch, int note) {
      if (ch>=adpcmBChanOffs) { // ADPCM
        return NOTE_ADPCMB(note);
      } else if (ch>=psgChanOffs) { // PSG
        return NOTE_PERIODIC(note);
      }
      // FM
      return NOTE_FNUM_BLOCK(note,11);
    }
    double NOTE_ADPCMB(int note) {
      if (chan[adpcmBChanOffs].sample>=0 && chan[adpcmBChanOffs].sample<parent->song.sampleLen) {
        double off=65535.0*(double)(parent->getSample(chan[adpcmBChanOffs].sample)->centerRate)/8363.0;
        return parent->calcBaseFreq((double)chipClock/144,off,note,false);
      }
      return 0;
    }
  
  public:
    void reset() {
      writeADPCMAOff=0;
      writeADPCMAOn=0;
      globalADPCMAVolume=0x3f;

      ay->reset();
      ay->getRegisterWrites().clear();
      ay->flushWrites();
    }

    void muteChannel(int ch, bool mute) {
      isMuted[ch]=mute;
      if (ch>=adpcmBChanOffs) { // ADPCM-B
        immWrite(0x11,isMuted[ch]?0:(chan[ch].pan<<6));
      }
      if (ch>=adpcmAChanOffs) { // ADPCM-A
        immWrite(0x108+(ch-adpcmAChanOffs),isMuted[ch]?0:((chan[ch].pan<<6)|chan[ch].outVol));
        return;
      }
      if (ch>=psgChanOffs) { // PSG
        ay->muteChannel(ch-psgChanOffs,mute);
        return;
      }
    }

    bool isStereo() {
      return true;
    }

    const void* getSampleMem(int index) {
      return index == 0 ? adpcmAMem : index == 1 ? adpcmBMem : NULL;
    }

    size_t getSampleMemCapacity(int index) {
      return index == 0 ? 16777216 : index == 1 ? 16777216 : 0;
    }

    size_t getSampleMemUsage(int index) {
      return index == 0 ? adpcmAMemLen : index == 1 ? adpcmBMemLen : 0;
    }

    void renderSamples() {
      memset(adpcmAMem,0,getSampleMemCapacity(0));
      memset(sampleOffA,0,256*sizeof(unsigned int));
      memset(sampleOffB,0,256*sizeof(unsigned int));

      size_t memPos=0;
      for (int i=0; i<parent->song.sampleLen; i++) {
        DivSample* s=parent->song.sample[i];
        int paddedLen=(s->lengthA+255)&(~0xff);
        if ((memPos&0xf00000)!=((memPos+paddedLen)&0xf00000)) {
          memPos=(memPos+0xfffff)&0xf00000;
        }
        if (memPos>=getSampleMemCapacity(0)) {
          logW("out of ADPCM-A memory for sample %d!",i);
          break;
        }
        if (memPos+paddedLen>=getSampleMemCapacity(0)) {
          memcpy(adpcmAMem+memPos,s->dataA,getSampleMemCapacity(0)-memPos);
          logW("out of ADPCM-A memory for sample %d!",i);
        } else {
          memcpy(adpcmAMem+memPos,s->dataA,paddedLen);
        }
        sampleOffA[i]=memPos;
        memPos+=paddedLen;
      }
      adpcmAMemLen=memPos+256;

      memset(adpcmBMem,0,getSampleMemCapacity(1));

      memPos=0;
      for (int i=0; i<parent->song.sampleLen; i++) {
        DivSample* s=parent->song.sample[i];
        int paddedLen=(s->lengthB+255)&(~0xff);
        if ((memPos&0xf00000)!=((memPos+paddedLen)&0xf00000)) {
          memPos=(memPos+0xfffff)&0xf00000;
        }
        if (memPos>=getSampleMemCapacity(1)) {
          logW("out of ADPCM-B memory for sample %d!",i);
          break;
        }
        if (memPos+paddedLen>=getSampleMemCapacity(1)) {
          memcpy(adpcmBMem+memPos,s->dataB,getSampleMemCapacity(1)-memPos);
          logW("out of ADPCM-B memory for sample %d!",i);
        } else {
          memcpy(adpcmBMem+memPos,s->dataB,paddedLen);
        }
        sampleOffB[i]=memPos;
        memPos+=paddedLen;
      }
      adpcmBMemLen=memPos+256;
    }

    void setFlags(const DivConfig& flags) {
      switch (flags.getInt("clockSel",0)) {
        case 0x01:
          chipClock=24167829/3;
          break;
        default:
          chipClock=8000000.0;
          break;
      }
      rate=chipClock/16;
      for (int i=0; i<ChanNum; i++) {
        oscBuf[i]->rate=rate;
      }
    }

    int init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
      parent=p;
      ayFlags.set("chipType",1);
      dumpWrites=false;
      skipRegisterWrites=false;
      for (int i=0; i<ChanNum; i++) {
        isMuted[i]=false;
        oscBuf[i]=new DivDispatchOscBuffer;
      }
      adpcmAMem=new unsigned char[getSampleMemCapacity(0)];
      adpcmAMemLen=0;
      adpcmBMem=new unsigned char[getSampleMemCapacity(1)];
      adpcmBMemLen=0;
      iface.adpcmAMem=adpcmAMem;
      iface.adpcmBMem=adpcmBMem;
      iface.sampleBank=0;
      fm=new ymfm::ym2610b(iface);
      fm->set_fidelity(ymfm::OPN_FIDELITY_MAX);
      setFlags(flags);
      // YM2149, 2MHz
      ay=new DivPlatformAY8910(true,chipClock,32);
      ay->init(p,3,sugRate,ayFlags);
      ay->toggleRegisterDump(true);
      return 0;
    }

    void quit() {
      for (int i=0; i<ChanNum; i++) {
        delete oscBuf[i];
      }
      ay->quit();
      delete ay;
      delete[] adpcmAMem;
      delete[] adpcmBMem;
    }

    DivPlatformYM2610Base(int ext, int psg, int adpcmA, int adpcmB):
      DivPlatformOPN(9440540.0, 72, 32),
      extChanOffs(ext),
      psgChanOffs(psg),
      adpcmAChanOffs(adpcmA),
      adpcmBChanOffs(adpcmB) {}
};

#endif
