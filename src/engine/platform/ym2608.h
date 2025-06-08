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

#ifndef _YM2608_H
#define _YM2608_H

#include "fmshared_OPN.h"
#include "sound/ymfm/ymfm_opn.h"
extern "C" {
#include "../../../extern/YM2608-LLE/fmopna_2608.h"
}

#include "ay.h"

class DivYM2608Interface: public DivOPNInterface {
  public:
    unsigned char* adpcmBMem;
    int sampleBank;
    uint8_t ymfm_external_read(ymfm::access_class type, uint32_t address);
    void ymfm_external_write(ymfm::access_class type, uint32_t address, uint8_t data);
    DivYM2608Interface(): adpcmBMem(NULL), sampleBank(0) {}
};

class DivPlatformYM2608: public DivPlatformOPN {
  protected:
    const unsigned short chanOffs[6]={
      0x00, 0x01, 0x02, 0x100, 0x101, 0x102
    };

    const unsigned char konOffs[6]={
      0, 1, 2, 4, 5, 6
    };

    OPNChannelStereo chan[17];
    DivDispatchOscBuffer* oscBuf[17];
    bool isMuted[17];
    ym3438_t fm_nuked;
    ymfm::ym2608* fm;
    ymfm::ym2608::output_data fmout;
    fmopna_t fm_lle;
    unsigned int dacVal;
    unsigned int dacVal2;
    int dacOut[2];
    int rssOut[6];
    bool lastSH;
    bool lastSH2;
    bool lastS;
    unsigned char cas, ras, rssCycle, rssSubCycle;
    unsigned int adMemAddr;

    unsigned char* adpcmBMem;
    size_t adpcmBMemLen;
    DivYM2608Interface iface;
    unsigned int sampleOffB[256];
    bool sampleLoaded[256];
  
    DivPlatformAY8910* ay;
    unsigned char sampleBank;
    unsigned char writeRSSOff, writeRSSOn;
    int globalRSSVolume;

    bool extMode, noExtMacros;
    unsigned char prescale, nukedMult, memConfig;

    DivMemoryComposition memCompo;
  
    double NOTE_OPNB(int ch, int note);
    double NOTE_ADPCMB(int note);

    friend void putDispatchChip(void*,int);

    inline void commitState(int ch, DivInstrument* ins);

    void acquire_combo(short** buf, size_t len);
    void acquire_ymfm(short** buf, size_t len);
    void acquire_lle(short** buf, size_t len);

  public:
    unsigned char isCSM;
    void acquire(short** buf, size_t len);
    void fillStream(std::vector<DivDelayedWrite>& stream, int sRate, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    virtual unsigned short getPan(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    int getOutputCount();
    bool keyOffAffectsArp(int ch);
    void notifyInsChange(int ins);
    virtual void notifyInsDeletion(void* ins);
    void setSkipRegisterWrites(bool val);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const void* getSampleMem(int index);
    size_t getSampleMemCapacity(int index);
    size_t getSampleMemUsage(int index);
    bool isSampleLoaded(int index, int sample);
    const DivMemoryComposition* getMemCompo(int index);
    void renderSamples(int chipID);
    void setFlags(const DivConfig& flags);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void setCSM(bool isCSM);
    void quit();
    DivPlatformYM2608():
      DivPlatformOPN(2, 6, 9, 15, 16, 9440540.0, 72, 32, false, 16),
      prescale(0x2d),
      isCSM(0) {}
    ~DivPlatformYM2608();
};
#endif
