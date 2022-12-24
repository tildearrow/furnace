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

#ifndef _YM2203_H
#define _YM2203_H

#include "fmshared_OPN.h"
#include "sound/ymfm/ymfm_opn.h"

#include "ay.h"

class DivYM2203Interface: public ymfm::ymfm_interface {

};

class DivPlatformYM2203: public DivPlatformOPN {
  protected:
    const unsigned short chanOffs[3]={
      0x00, 0x01, 0x02
    };

    const unsigned char konOffs[3]={
      0, 1, 2
    };

    OPNChannel chan[6];
    DivDispatchOscBuffer* oscBuf[6];
    bool isMuted[6];
    ym3438_t fm_nuked;
    ymfm::ym2203* fm;
    ymfm::ym2203::output_data fmout;
    DivYM2203Interface iface;
  
    DivPlatformAY8910* ay;
    unsigned char sampleBank;

    bool extMode, noExtMacros;
    unsigned char prescale, nukedMult;

    friend void putDispatchChip(void*,int);

    void acquire_combo(short* bufL, short* bufR, size_t start, size_t len);
    void acquire_ymfm(short* bufL, short* bufR, size_t start, size_t len);

  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
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
    bool isStereo();
    bool keyOffAffectsArp(int ch);
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    void setSkipRegisterWrites(bool val);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    void setFlags(const DivConfig& flags);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    DivPlatformYM2203():
      DivPlatformOPN(2, 3, 6, 6, 6, 4720270.0, 36, 16),
      prescale(0x2d) {}
    ~DivPlatformYM2203();
};
#endif
