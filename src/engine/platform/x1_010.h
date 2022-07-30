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

#include "sampleshared.h"
#include "../dispatch.h"
#include "../engine.h"
#include "../macroInt.h"
#include "../waveSynth.h"
#include "sound/x1_010/x1_010.hpp"

class DivPlatformX1_010: public DivDispatch, public DivPlatformSample, public x1_010_mem_intf {
  struct Channel {
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
    int freq, baseFreq, pitch, pitch2, note;
    int wave, sample, ins;
    unsigned char pan, autoEnvNum, autoEnvDen;
    bool active, insChanged, envChanged, freqChanged, keyOn, keyOff, inPorta, furnacePCM, pcm;
    int vol, outVol, lvol, rvol;
    unsigned char waveBank;
    Envelope env;
    DivMacroInt std;
    DivWaveSynth ws;
    void reset() {
        freq = baseFreq = pitch = pitch2 = note = 0;
        wave = sample = ins = -1;
        pan = 255;
        autoEnvNum = autoEnvDen = 0;
        active = false;
        insChanged = envChanged = freqChanged = true;
        keyOn = keyOff = inPorta = furnacePCM = pcm = false;
        vol = outVol = lvol = rvol = 15;
        waveBank = 0;
    }
    void macroInit(DivInstrument* which) {
      std.init(which);
      pitch2=0;
    }
    Channel():
      freq(0), baseFreq(0), pitch(0), pitch2(0), note(0),
      wave(-1), sample(-1), ins(-1),
      pan(255), autoEnvNum(0), autoEnvDen(0),
      active(false), insChanged(true), envChanged(true), freqChanged(false), keyOn(false), keyOff(false), inPorta(false), furnacePCM(false), pcm(false),
      vol(15), outVol(15), lvol(15), rvol(15),
      waveBank(0) {}
  };
  Channel chan[16];
  DivDispatchOscBuffer* oscBuf[16];
  bool isMuted[16];
  bool stereo=false;
  unsigned char* sampleMem;
  size_t sampleMemLen;
  x1_010_core x1_010;
  unsigned char regPool[0x2000];
  double NoteX1_010(int ch, int note);
  void updateWave(int ch);
  void updateEnvelope(int ch);
  friend void putDispatchChan(void*,int,int);
  public:
    virtual u8 read_byte(u32 address) override;
    virtual void acquire(short* bufL, short* bufR, size_t start, size_t len) override;
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
    virtual bool isStereo() override;
    virtual bool keyOffAffectsArp(int ch) override;
    virtual void setFlags(unsigned int flags) override;
    virtual void notifyWaveChange(int wave) override;
    virtual void notifyInsDeletion(void* ins) override;
    virtual void poke(unsigned int addr, unsigned short val) override;
    virtual void poke(std::vector<DivRegWrite>& wlist) override;
    virtual const void* getSampleMem(int index = 0) override;
    virtual size_t getSampleMemCapacity(int index = 0) override;
    virtual size_t getSampleMemUsage(int index = 0) override;
    virtual void renderSamples() override;
    virtual const char** getRegisterSheet() override;
    virtual const char* getEffectName(unsigned char effect) override;
    virtual int init(DivEngine* parent, int channels, int sugRate, unsigned int flags) override;
    virtual void quit() override;
    DivPlatformX1_010():
      DivDispatch(),
      DivPlatformSample(),
      x1_010_mem_intf(),
      x1_010(*this) {}
    ~DivPlatformX1_010();
};

#endif
