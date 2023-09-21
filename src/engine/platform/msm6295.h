/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

#ifndef _MSM6295_H
#define _MSM6295_H

#include "../dispatch.h"
#include "../../fixedQueue.h"
#include "vgsound_emu/src/msm6295/msm6295.hpp"

class DivPlatformMSM6295: public DivDispatch, public vgsound_emu_mem_intf {
  protected:
    struct Channel: public SharedChannel<int> {
      bool furnacePCM;
      int sample;
      Channel():
        SharedChannel<int>(8),
        furnacePCM(false),
        sample(-1) {}
    };
    Channel chan[4];
    DivDispatchOscBuffer* oscBuf[4];
    bool isMuted[4];
    struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      unsigned short delay;
      QueuedWrite(): addr(0), val(0), delay(96) {}
      QueuedWrite(unsigned short a, unsigned char v, unsigned short d=96):
        addr(a),
        val(v),
        delay(d) {}
    };
    FixedQueue<QueuedWrite,256> writes;
    msm6295_core msm;

    unsigned char* adpcmMem;
    size_t adpcmMemLen;
    bool sampleLoaded[256];
    unsigned char sampleBank;

    int delay, updateOsc;

    bool rateSel=false, rateSelInit=false, isBanked=false;

    unsigned int bank[4];
    struct BankedPhrase {
      unsigned char bank=0;
      unsigned char phrase=0;
      BankedPhrase():
        bank(0),
        phrase(0) {}
    } bankedPhrase[256];
  
    friend void putDispatchChip(void*,int);
    friend void putDispatchChan(void*,int,int);
  
  public:
    virtual u8 read_byte(u32 address) override;
    virtual void acquire(short** buf, size_t len) override;
    virtual int dispatch(DivCommand c) override;
    virtual void* getChanState(int chan) override;
    virtual DivMacroInt* getChanMacroInt(int ch) override;
    virtual DivDispatchOscBuffer* getOscBuffer(int chan) override;
    virtual unsigned char* getRegisterPool() override;
    virtual int getRegisterPoolSize() override;
    virtual void reset() override;
    virtual void forceIns() override;
    virtual void tick(bool sysTick=true) override;
    virtual void muteChannel(int ch, bool mute) override;
    virtual bool keyOffAffectsArp(int ch) override;
    virtual float getPostAmp() override;
    virtual void notifyInsChange(int ins) override;
    virtual void notifyInsDeletion(void* ins) override;
    virtual void poke(unsigned int addr, unsigned short val) override;
    virtual void poke(std::vector<DivRegWrite>& wlist) override;
    virtual void setFlags(const DivConfig& flags) override;
    virtual const char** getRegisterSheet() override;
    virtual const void* getSampleMem(int index) override;
    virtual size_t getSampleMemCapacity(int index) override;
    virtual size_t getSampleMemUsage(int index) override;
    virtual bool isSampleLoaded(int index, int sample) override;
    virtual void renderSamples(int chipID) override;

    virtual int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags) override;
    virtual void quit() override;
    DivPlatformMSM6295():
      DivDispatch(),
      vgsound_emu_mem_intf(),
      msm(*this) {}
    ~DivPlatformMSM6295();
};
#endif
