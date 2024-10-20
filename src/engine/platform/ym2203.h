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

#ifndef _YM2203_H
#define _YM2203_H

#include "fmshared_OPN.h"
#include "sound/ymfm/ymfm_opn.h"
extern "C" {
#include "../../../extern/YM2608-LLE/fmopna_2608.h"
}

#include "ay.h"

class DivPlatformYM2203: public DivPlatformOPN {
  protected:
    const unsigned short chanOffs[3]={
      0x00, 0x01, 0x02
    };

    const unsigned char konOffs[3]={
      0, 1, 2
    };

    OPNChannel chan[7];
    DivDispatchOscBuffer* oscBuf[7];
    bool isMuted[7];
    ym3438_t fm_nuked;
    ymfm::ym2203* fm;
    ymfm::ym2203::output_data fmout;
    DivOPNInterface iface;
    fmopna_t fm_lle;
    unsigned int dacVal;
    unsigned int dacVal2;
    int dacOut[2];
    int rssOut[6];
    bool lastSH;
    bool lastSH2;
    bool lastS;
  
    DivPlatformAY8910* ay;
    unsigned char sampleBank;

    bool extMode, noExtMacros;
    unsigned char prescale, nukedMult;

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
    void setFlags(const DivConfig& flags);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    DivPlatformYM2203():
      DivPlatformOPN(2, 3, 6, 6, 6, 4720270.0, 36, 16, false, 6),
      prescale(0x2d),
      isCSM(0) {}
    ~DivPlatformYM2203();
};
#endif
