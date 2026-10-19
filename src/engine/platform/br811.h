/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

#ifndef _BR811_H
#define _BR811_H

#include "../dispatch.h"
#include "sound/br811.h"

// BR811: chip de som do console ficticio Ompreza (1984).
// 5 canais. Cada canal escolhe uma "fonte" (0-7):
//   0 = Onda A, 1 = Onda B, 2-7 = ruido (taxas diferentes).
class DivPlatformBR811: public DivDispatch {
  struct Channel: public SharedChannel {
    int source;     // fonte do som, 0 a 7
    int period;     // periodo de 12 bits que vai para o chip
    bool dirty;     // precisa reescrever os registradores do chip
    bool pcm;       // canal 5: o instrumento atual e um sample (PCM)
    int sample;     // canal 5: numero do sample em uso
    Channel(bool linear=true):
      SharedChannel(15,linear),
      source(0),
      period(0),
      dirty(true),
      pcm(false),
      sample(-1) {}
  };
  Channel chan[5];
  DivDispatchOscBuffer* oscBuf[5];
  bool isMuted[5];
  DivPitchTable pitchTable;

  unsigned char regPool[19];
  br811::Chip chip;

  // memoria de samples (a ROM do cartucho, 64 KB de enderecamento)
  unsigned char* sampleMem;
  size_t sampleMemLen;
  unsigned int* sampleOff;
  unsigned int* sampleLenBytes;
  bool* sampleLoaded;
  DivMemoryComposition memCompo;
  bool pcmActive;

  void commit(int ch);
  void chipWrite(int ch, int reg, unsigned char val);
  void chipWritePcm(int reg, unsigned char val);
  void pcmStart(int ch);
  void pcmStop();

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);
  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    SharedChannel* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    bool keyOffAffectsArp(int ch);
    void setFlags(const DivConfig& flags);
    void notifyInsDeletion(void* ins);
    void notifyPitchTable(int sample=-1);
    unsigned int getMaxFreq(int ch);
    int getOutputCount();
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const void* getSampleMem(int index);
    size_t getSampleMemCapacity(int index);
    size_t getSampleMemUsage(int index);
    bool isSampleLoaded(int index, int sample);
    const DivMemoryComposition* getMemCompo(int index);
    void renderSamples(int chipID);
    bool getDCOffRequired();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    DivPlatformBR811();
    ~DivPlatformBR811();
};

#endif
