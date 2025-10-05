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

#ifndef _YM2610SHARED_H
#define _YM2610SHARED_H

#include "fmshared_OPN.h"
#include "../engine.h"
#include "../../ta-log.h"
#include "ay.h"
#include "sound/ymfm/ymfm.h"
#include "sound/ymfm/ymfm_opn.h"
#include <string.h>
extern "C" {
#include "../../../extern/YM2608-LLE/fmopna_2610.h"
}

#define CHIP_FREQBASE fmFreqBase
#define CHIP_DIVIDER fmDivBase

class DivYM2610Interface: public DivOPNInterface {
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

class DivPlatformYM2610Base: public DivPlatformOPN {
  protected:
    OPNChannelStereo chan[17];
    DivDispatchOscBuffer* oscBuf[17];
    bool isMuted[17];

    ym3438_t fm_nuked;
    ymfm::ym2610b* fm;
    ymfm::ym2610b::output_data fmout;
    DivPlatformAY8910* ay;
    fmopna_2610_t fm_lle;
    unsigned int dacVal;
    unsigned int dacVal2;
    int dacOut[2];
    int rssOut[6];
    bool lastSH;
    bool lastSH2;
    bool lastS;
    unsigned char rmpx, pmpx, roe, poe, rssCycle, rssSubCycle;
    unsigned int adMemAddrA;
    unsigned int adMemAddrB;
  
    unsigned char* adpcmAMem;
    size_t adpcmAMemLen;
    unsigned char* adpcmBMem;
    size_t adpcmBMemLen;
    DivYM2610Interface iface;

    unsigned int* sampleOffA;
    unsigned int* sampleOffB;

    unsigned char sampleBank;
  
    bool extMode, noExtMacros;

    bool* sampleLoaded[2];
  
    unsigned char writeADPCMAOff, writeADPCMAOn;
    int globalADPCMAVolume;

    DivMemoryComposition memCompoA;
    DivMemoryComposition memCompoB;

    double NOTE_OPNB(int ch, int note) {
      if (ch>=adpcmBChanOffs) { // ADPCM
        return NOTE_ADPCMB(note);
      } else if (ch>=psgChanOffs) { // PSG
        return NOTE_PERIODIC(note);
      }
      // FM
      return NOTE_FNUM_BLOCK(note,11,chan[ch].state.block);
    }
    double NOTE_ADPCMB(int note) {
      if (chan[adpcmBChanOffs].sample>=0 && chan[adpcmBChanOffs].sample<parent->song.sampleLen) {
        double off=65535.0*(double)(parent->getSample(chan[adpcmBChanOffs].sample)->centerRate)/parent->getCenterRate();
        return parent->calcBaseFreq((double)chipClock/144,off,note,false);
      }
      return 0;
    }
  
  public:
    void fillStream(std::vector<DivDelayedWrite>& stream, int sRate, size_t len) {
      ay->fillStream(stream,sRate,len);
    }

    void reset() {
      writeADPCMAOff=0;
      writeADPCMAOn=0;
      globalADPCMAVolume=0x3f;

      OPN2_Reset(&fm_nuked);
      OPN2_SetChipType(&fm_nuked,ym3438_mode_opn);

      memset(&fm_lle,0,sizeof(fmopna_2610_t));

      if (useCombo==2) {
        fm_lle.input.cs=1;
        fm_lle.input.rd=0;
        fm_lle.input.wr=0;
        fm_lle.input.a0=0;
        fm_lle.input.a1=0;
        fm_lle.input.data=0;
        fm_lle.input.rad=0;
        fm_lle.input.pad=0;
        fm_lle.input.test=1;

        fm_lle.input.ic=1;
        for (size_t h=0; h<576; h++) {
          FMOPNA_2610_Clock(&fm_lle,0);
          FMOPNA_2610_Clock(&fm_lle,1);
        }

        fm_lle.input.ic=0;
        for (size_t h=0; h<576; h++) {
          FMOPNA_2610_Clock(&fm_lle,0);
          FMOPNA_2610_Clock(&fm_lle,1);
        }

        fm_lle.input.ic=1;
        for (size_t h=0; h<576; h++) {
          FMOPNA_2610_Clock(&fm_lle,0);
          FMOPNA_2610_Clock(&fm_lle,1);
        }

        for (int i=0; i<6; i++) {
          rssOut[i]=0;
        }

        dacVal=0;
        dacVal2=0;
        dacOut[0]=0;
        dacOut[1]=0;
        lastSH=0;
        lastSH2=0;
        lastS=0;
        rmpx=0;
        pmpx=0;
        roe=0;
        poe=0;
        rssCycle=0;
        rssSubCycle=0;
        adMemAddrA=0;
        adMemAddrB=0;
      }

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

    int getOutputCount() {
      return 2;
    }

    const void* getSampleMem(int index) {
      return index == 0 ? adpcmAMem : index == 1 ? adpcmBMem : NULL;
    }

    size_t getSampleMemCapacity(int index) {
      return index == 0 ? 16777216 : index == 1 ? 16777216 : 0;
    }

    const char* getSampleMemName(int index=0) {
      return index == 0 ? "ADPCM-A" : index == 1 ? "ADPCM-B" : NULL;
    }

    size_t getSampleMemUsage(int index) {
      return index == 0 ? adpcmAMemLen : index == 1 ? adpcmBMemLen : 0;
    }

    bool isSampleLoaded(int index, int sample) {
      if (index<0 || index>1) return false;
      if (sample<0 || sample>32767) return false;
      return sampleLoaded[index][sample];
    }
    
    const DivMemoryComposition* getMemCompo(int index) {
      if (index==0) return &memCompoA;
      if (index==1) return &memCompoB;
      return NULL;
    }

    void renderSamples(int sysID) {
      memset(adpcmAMem,0,getSampleMemCapacity(0));
      memset(sampleOffA,0,32768*sizeof(unsigned int));
      memset(sampleOffB,0,32768*sizeof(unsigned int));
      memset(sampleLoaded[0],0,32768*sizeof(bool));
      memset(sampleLoaded[1],0,32768*sizeof(bool));

      memCompoA=DivMemoryComposition();
      memCompoA.name="ADPCM-A";

      memCompoB=DivMemoryComposition();
      memCompoB.name="ADPCM-B";

      size_t memPos=0;
      for (int i=0; i<parent->song.sampleLen; i++) {
        DivSample* s=parent->song.sample[i];
        if (!s->renderOn[0][sysID]) {
          sampleOffA[i]=0;
          continue;
        }

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
          sampleLoaded[0][i]=true;
        }
        sampleOffA[i]=memPos;
        memCompoA.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE,"Sample",i,memPos,memPos+paddedLen));
        memPos+=paddedLen;
      }
      adpcmAMemLen=memPos+256;

      memCompoA.used=adpcmAMemLen;
      memCompoA.capacity=getSampleMemCapacity(0);

      memset(adpcmBMem,0,getSampleMemCapacity(1));

      memPos=0;
      for (int i=0; i<parent->song.sampleLen; i++) {
        DivSample* s=parent->song.sample[i];
        if (!s->renderOn[1][sysID]) {
          sampleOffB[i]=0;
          continue;
        }

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
          sampleLoaded[1][i]=true;
        }
        sampleOffB[i]=memPos;
        memCompoB.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE,"Sample",i,memPos,memPos+paddedLen));
        memPos+=paddedLen;
      }
      adpcmBMemLen=memPos+256;

      memCompoB.used=adpcmBMemLen;
      memCompoB.capacity=getSampleMemCapacity(1);
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
      CHECK_CUSTOM_CLOCK;
      noExtMacros=flags.getBool("noExtMacros",false);
      fbAllOps=flags.getBool("fbAllOps",false);
      ssgVol=flags.getInt("ssgVol",128);
      fmVol=flags.getInt("fmVol",256);
      if (useCombo==2) {
        rate=chipClock/144;
      } else {
        rate=fm->sample_rate(chipClock);
      }
      for (int i=0; i<17; i++) {
        oscBuf[i]->setRate(rate);
      }
    }

    int init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
      parent=p;
      ayFlags.set("chipType",1);
      dumpWrites=false;
      skipRegisterWrites=false;
      for (int i=0; i<17; i++) {
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
      fm->set_fidelity(ymfm::OPN_FIDELITY_MED);
      setFlags(flags);
      // YM2149, 2MHz
      ay=new DivPlatformAY8910(true,chipClock,32,144);
      ay->setCore(0);
      ay->init(p,3,sugRate,ayFlags);
      ay->toggleRegisterDump(true);
      return 0;
    }

    void quit() {
      for (int i=0; i<17; i++) {
        delete oscBuf[i];
      }
      ay->quit();
      delete ay;
      delete[] adpcmAMem;
      delete[] adpcmBMem;
    }

    DivPlatformYM2610Base(int ext, int psg, int adpcmA, int adpcmB, int chanCount):
      DivPlatformOPN(ext,psg,adpcmA,adpcmB,chanCount,9440540.0, 72, 32, false, 16) {
      sampleOffA=new unsigned int[32768];
      sampleOffB=new unsigned int[32768];
      sampleLoaded[0]=new bool[32768];
      sampleLoaded[1]=new bool[32768];
    }
    ~DivPlatformYM2610Base() {
      delete[] sampleOffA;
      delete[] sampleOffB;
      delete[] sampleLoaded[0];
      delete[] sampleLoaded[1];
    }
};

#endif
