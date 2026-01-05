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

#ifndef _YM2610XSHARED_H
#define _YM2610XSHARED_H

#include "fmshared_OPN.h"
#include "../engine.h"
#include "../../ta-log.h"
#include "ay8930x.h"
#include "sound/ymfm/ymfm.h"
#include "sound/ymfm/ymfm_opnx.h"
#include <string.h>
extern "C" {
#include "../../../extern/YM2608-LLE/fmopna_2610.h"
}

#define CHIP_FREQBASE fmFreqBase
#define CHIP_DIVIDER fmDivBase

class DivYM2610XInterface: public DivOPNInterface {
  public:
    unsigned char* adpcmAMem;
    unsigned char* adpcmBMem;
    uint8_t ymfm_external_read(ymfm::access_class type, uint32_t address);
    void ymfm_external_write(ymfm::access_class type, uint32_t address, uint8_t data);
    DivYM2610XInterface():
      adpcmAMem(NULL),
      adpcmBMem(NULL) {}
};

class DivPlatformYM2610XBase: public DivPlatformOPN {
  protected:
    const unsigned short ADDR_PCLFO_R=0xc0;
    const unsigned short ADDR_PCLFO_FMD=0xc4;
    const unsigned short ADDR_PCLFO_AMD=0xc8;
    const unsigned short ADDR_PCLFO_CTL=0xcc;
    const unsigned short ADDR_PCLFO_NOI=0xd0;
    const unsigned short ADDR_PCLFO2_R=0xe0;
    const unsigned short ADDR_PCLFO2_FMD=0xe4;
    const unsigned short ADDR_PCLFO2_AMD=0xe8;
    const unsigned short ADDR_PCLFO2_CTL=0xec;
    const unsigned short ADDR_PCLFO2_NOI=0xf0;

    OPNChannelStereo chan[19];
    DivDispatchOscBuffer* oscBuf[19];
    bool isMuted[19];

    ymfm::ym2610x* fm;
    ymfm::ym2610x::output_data fmout;
    DivPlatformAY8930X* ay;
  
    unsigned char* adpcmAMem;
    size_t adpcmAMemLen;
    unsigned char* adpcmBMem;
    size_t adpcmBMemLen;
    DivYM2610XInterface iface;

    unsigned int* sampleOffA;
    unsigned int* sampleOffB;

    unsigned int ssgBank, adpcmABank, adpcmBBank;
  
    bool extMode;

    bool* sampleLoaded[2];
  
    unsigned char writeADPCMAOff, writeADPCMAOn, writeADPCMALoop;
    int globalADPCMAVolume;

    DivMemoryComposition memCompoA;
    DivMemoryComposition memCompoB;

    double NOTE_OPNX(int ch, int note) {
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
        return parent->calcBaseFreq((double)chipClock/576,off,note,false);
      }
      return 0;
    }

    inline void immWriteBanked(unsigned int a, unsigned short v) {
      if ((a&0x1ff)<=0x00f) {
        if (ssgBank!=((a>>9)&1)) {
          ssgBank=(a>>9)&1;
          immWrite(0x00f,0x01|(ssgBank<<1));
        }
        if (a==0x00f) {
          immWrite(0x00f,0x01|(ssgBank<<1)|(v&0xfc));
        } else {
          immWrite(a&0x00f,v);
        }
      } else if ((a&0x1ff)>=0x010 && (a&0x1ff)<=0x01f) {
        if (adpcmBBank!=((a>>9)&1)) {
          adpcmBBank=(a>>9)&1;
          immWrite(0x01f,0x01|(adpcmBBank<<1));
        }
        if (a==0x01f) {
          immWrite(0x01f,0x01|(adpcmBBank<<1)|(v&0xfc));
        } else {
          immWrite(a&0x01f,v);
        }
      } else if ((a&0x1ff)>=0x100 && (a&0x1ff)<=0x12f) {
        if (adpcmABank!=((a>>9)&1)) {
          adpcmABank=(a>>9)&1;
          immWrite(0x12f,0x01|(adpcmABank<<1));
        }
        if (a==0x12f) {
          immWrite(0x12f,0x01|(adpcmABank<<1)|(v&0xfc));
        } else {
          immWrite(a&0x13f,v);
        }
      } else {
        immWrite(a,v);
      }
    }

    inline void immWrite(unsigned int a, unsigned short v) {
      if (!skipRegisterWrites) {
        writes.push_back(QueuedWrite(a,v));
        if (dumpWrites) {
          addWrite(a,v);
        }
      }
    }
  
  public:
    void fillStream(std::vector<DivDelayedWrite>& stream, int sRate, size_t len) {
      ay->fillStream(stream,sRate,len);
    }

    void reset() {
      writeADPCMAOff=0;
      writeADPCMAOn=0;
      writeADPCMALoop=0;
      globalADPCMAVolume=0x3f;
      ssgBank=0;
      adpcmABank=0;
      adpcmBBank=0;

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

        int paddedLen=s->lengthA;
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

        int paddedLen=s->lengthB;
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
      chipClock=32000000.0;
      CHECK_CUSTOM_CLOCK;
      ssgVol=flags.getInt("ssgVol",128);
      fmVol=flags.getInt("fmVol",256);
      rate=fm->sample_rate(chipClock);
      for (int i=0; i<19; i++) {
        oscBuf[i]->setRate(rate);
      }
    }

    int init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
      parent=p;
      dumpWrites=false;
      skipRegisterWrites=false;
      for (int i=0; i<19; i++) {
        isMuted[i]=false;
        oscBuf[i]=new DivDispatchOscBuffer;
      }
      adpcmAMem=new unsigned char[getSampleMemCapacity(0)];
      adpcmAMemLen=0;
      adpcmBMem=new unsigned char[getSampleMemCapacity(1)];
      adpcmBMemLen=0;
      iface.adpcmAMem=adpcmAMem;
      iface.adpcmBMem=adpcmBMem;
      fm=new ymfm::ym2610x(iface);
      fm->set_fidelity(ymfm::OPNX_FIDELITY_MED);
      setFlags(flags);
      // AY8930X, 16MHz
      ay=new DivPlatformAY8930X(true,chipClock/2,32,576);
      ay->init(p,3,sugRate,ayFlags);
      ay->toggleRegisterDump(true);
      return 0;
    }

    void quit() {
      for (int i=0; i<19; i++) {
        delete oscBuf[i];
      }
      ay->quit();
      delete ay;
      delete[] adpcmAMem;
      delete[] adpcmBMem;
    }

    DivPlatformYM2610XBase(int ext, int psg, int adpcmA, int adpcmB, int chanCount):
      DivPlatformOPN(ext,psg,adpcmA,adpcmB,chanCount,37762160.0, 576, 2, false, 18) {
      sampleOffA=new unsigned int[32768];
      sampleOffB=new unsigned int[32768];
      sampleLoaded[0]=new bool[32768];
      sampleLoaded[1]=new bool[32768];
    }
    ~DivPlatformYM2610XBase() {
      delete[] sampleOffA;
      delete[] sampleOffB;
      delete[] sampleLoaded[0];
      delete[] sampleLoaded[1];
    }
};

#endif
