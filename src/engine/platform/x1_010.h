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

#ifndef _X1_010_H
#define _X1_010_H

#include "../dispatch.h"
#include "../engine.h"
#include "../waveSynth.h"
#include "vgsound_emu/src/x1_010/x1_010.hpp"

class DivPlatformX1_010: public DivDispatch, public vgsound_emu_mem_intf {
  struct Channel: public SharedChannel<int> {
    struct Envelope {
      struct EnvFlag {
        unsigned char envEnable : 1;
        unsigned char envOneshot : 1;
        unsigned char envSplit : 1;
        unsigned char envHinvR : 1;
        unsigned char envVinvR : 1;
        unsigned char envHinvL : 1;
        unsigned char envVinvL : 1;
        void reset() {
          envEnable=0;
          envOneshot=0;
          envSplit=0;
          envHinvR=0;
          envVinvR=0;
          envHinvL=0;
          envVinvL=0;
        }
        EnvFlag():
          envEnable(0),
          envOneshot(0),
          envSplit(0),
          envHinvR(0),
          envVinvR(0),
          envHinvL(0),
          envVinvL(0) {}
      };
      int shape, period, slide, slidefrac;
      EnvFlag flag;
      void reset() {
        shape=-1;
        period=0;
        flag.reset();
      }
      Envelope():
        shape(-1),
        period(0),
        slide(0),
        slidefrac(0) {}
    };
    int fixedFreq;
    int wave, sample;
    unsigned char pan, autoEnvNum, autoEnvDen;
    bool envChanged, furnacePCM, pcm;
    int lvol, rvol;
    int macroVolMul;
    unsigned char waveBank;
    unsigned int bankSlot;
    Envelope env;
    DivWaveSynth ws;
    void reset() {
        freq=baseFreq=pitch=pitch2=note=0;
        wave=sample=ins=-1;
        pan=255;
        autoEnvNum=autoEnvDen=0;
        active=false;
        insChanged=envChanged=freqChanged=true;
        keyOn=keyOff=inPorta=furnacePCM=pcm=false;
        vol=outVol=lvol=rvol=15;
        waveBank=0;
    }
    Channel():
      SharedChannel<int>(15),
      fixedFreq(0),
      wave(-1),
      sample(-1),
      pan(255),
      autoEnvNum(0),
      autoEnvDen(0),
      envChanged(true),
      furnacePCM(false),
      pcm(false),
      lvol(15),
      rvol(15),
      macroVolMul(15),
      waveBank(0),
      bankSlot(0) {}
  };
  Channel chan[16];
  DivDispatchOscBuffer* oscBuf[16];
  bool isMuted[16];
  bool stereo=false;
  unsigned char* sampleMem;
  size_t sampleMemLen;
  unsigned char sampleBank;
  x1_010_core x1_010;

  bool isBanked=false;
  unsigned int bankSlot[8];
  unsigned int sampleOffX1[256];
  bool sampleLoaded[256];

  unsigned char regPool[0x2000];
  double NoteX1_010(int ch, int note);
  void updateWave(int ch);
  void updateEnvelope(int ch);
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);
  public:
    u8 read_byte(u32 address);
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
    void setFlags(const DivConfig& flags);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const void* getSampleMem(int index = 0);
    size_t getSampleMemCapacity(int index = 0);
    size_t getSampleMemUsage(int index = 0);
    bool isSampleLoaded(int index, int sample);
    void renderSamples(int chipID);
    const char** getRegisterSheet();
    void setBanked(bool banked);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    DivPlatformX1_010():
      DivDispatch(),
      vgsound_emu_mem_intf(),
      x1_010(*this) {}
    ~DivPlatformX1_010();
};

#endif
