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

#ifndef _SAA_H
#define _SAA_H

#include "../dispatch.h"
#include "../../fixedQueue.h"
#include "../../../extern/SAASound/src/SAASound.h"

class DivPlatformSAA1099: public DivDispatch {
  protected:
    struct Channel: public SharedChannel<int> {
      unsigned char freqH, freqL;
      unsigned char psgMode;
      unsigned char pan;
      Channel():
        SharedChannel<int>(15),
        freqH(0),
        freqL(0),
        psgMode(1),
        pan(255) {}
    };
    Channel chan[6];
    DivDispatchOscBuffer* oscBuf[6];
    bool isMuted[6];
    struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(): addr(0), val(0), addrOrVal(false) {}
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
    FixedQueue<QueuedWrite,256> writes;
    int coreQuality;
    CSAASound* saa_saaSound;
    unsigned char regPool[32];
    unsigned char lastBusy;

    int delay;

    bool extMode;
  
    short oldWrites[16];
    short pendingWrites[16];
    short* saaBuf[2];
    size_t saaBufLen;
    unsigned char saaEnv[2];
    unsigned char saaNoise[2];
    friend void putDispatchChip(void*,int);
    friend void putDispatchChan(void*,int,int);

    void acquire_saaSound(short** buf, size_t len);
  
  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    unsigned short getPan(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    void setFlags(const DivConfig& flags);
    int getOutputCount();
    bool hasSoftPan(int ch);
    int getPortaFloor(int ch);
    bool keyOffAffectsArp(int ch);
    bool getLegacyAlwaysSetVolume();
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    void setCoreQuality(unsigned char q);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
};
#endif
