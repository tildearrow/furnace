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

#ifndef _MULTIPCM_H
#define _MULTIPCM_H

#include "../dispatch.h"
#include "../../fixedQueue.h"
#include "sound/ymf278b/ymf278.h"

class DivYMW258MemoryInterface: public MemoryInterface {
  public:
    unsigned char* memory;
    DivYMW258MemoryInterface(unsigned size_) : memory(NULL), size(size_) {};
    byte operator[](unsigned address) const override {
      if (memory && address<size) {
        return memory[address];
      }
      return 0;
    };
    unsigned getSize() const override { return size; };
    void write(unsigned address, byte value) override {};
    void clear(byte value) override {};
  private:
    unsigned size;
};

class DivPlatformMultiPCM: public DivDispatch {
  protected:
    struct Channel: public SharedChannel<int> {
      unsigned int freqH, freqL;
      int sample;
      bool writeCtrl, levelDirect;
      int lfo, vib, am;
      int pan;
      int macroVolMul;
      Channel():
        SharedChannel<int>(0x7f),
        freqH(0),
        freqL(0),
        sample(-1),
        writeCtrl(false),
        levelDirect(true),
        lfo(0),
        vib(0),
        am(0),
        pan(0),
        macroVolMul(64) {}
    };
    Channel chan[28];
    DivDispatchOscBuffer* oscBuf[28];
    bool isMuted[28];
    struct QueuedWrite {
      unsigned int addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(): addr(0), val(0), addrOrVal(false) {}
      QueuedWrite(unsigned int a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
    FixedQueue<QueuedWrite,4096> writes;

    unsigned char* pcmMem;
    size_t pcmMemLen;
    DivYMW258MemoryInterface pcmMemory;
    unsigned int* sampleOff;
    bool* sampleLoaded;
  
    int delay, curChan, curAddr;

    unsigned char regPool[224];
  
    short oldWrites[224];
    short pendingWrites[224];

    // chips
    YMW258 pcm;

    DivMemoryComposition memCompo;

    friend void putDispatchChip(void*,int);
    friend void putDispatchChan(void*,int,int);

    void renderInstruments();
  
  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    unsigned short getPan(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    int mapVelocity(int ch, float vel);
    float getGain(int ch, int vol);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    int getOutputCount();
    bool keyOffAffectsArp(int ch);
    bool keyOffAffectsPorta(int ch);
    bool getLegacyAlwaysSetVolume();
    void toggleRegisterDump(bool enable);
    void setFlags(const DivConfig& flags);
    void notifyInsChange(int ins);
    void notifyInsAddition(int sysID);
    void notifyInsDeletion(void* ins);
    int getPortaFloor(int ch);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    size_t getSampleMemNum();
    const void* getSampleMem(int index);
    size_t getSampleMemCapacity(int index);
    size_t getSampleMemUsage(int index);
    bool hasSamplePtrHeader(int index=0);
    bool isSampleLoaded(int index, int sample);
    const DivMemoryComposition* getMemCompo(int index);
    void renderSamples(int chipID);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    DivPlatformMultiPCM();
    ~DivPlatformMultiPCM();
};
#endif
