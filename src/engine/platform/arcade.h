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

#ifndef _ARCADE_H
#define _ARCADE_H
#include "fmshared_OPM.h"
#include <queue>
#include "../../../extern/opm/opm.h"
#include "sound/ymfm/ymfm_opm.h"

class DivArcadeInterface: public ymfm::ymfm_interface {

};

class DivPlatformArcade: public DivPlatformOPM {
  protected:
    const unsigned short chanOffs[8]={
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
    };

    struct Channel: public FMChannel {
      unsigned char chVolL, chVolR;
      Channel():
        FMChannel(),
        chVolL(1),
        chVolR(1) {}
    };
    Channel chan[8];
    DivDispatchOscBuffer* oscBuf[8];
    opm_t fm;
    int baseFreqOff;
    unsigned char amDepth, pmDepth;

    ymfm::ym2151* fm_ymfm;
    ymfm::ym2151::output_data out_ymfm;
    DivArcadeInterface iface;

    bool useYMFM;

    bool isMuted[8];

    int octave(int freq);
    int toFreq(int freq);

    void acquire_nuked(short* bufL, short* bufR, size_t start, size_t len);
    void acquire_ymfm(short* bufL, short* bufR, size_t start, size_t len);

    friend void putDispatchChan(void*,int,int);
    friend void putDispatchChip(void*,int);
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    DivMacroInt* getChanMacroInt(int ch);
    void notifyInsChange(int ins);
    void setFlags(const DivConfig& flags);
    bool isStereo();
    void setYMFM(bool use);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformArcade();
};
#endif
