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

#ifndef _POWERNOISE_H
#define _POWERNOISE_H

#include "../dispatch.h"
#include "../../../extern/pwrnoise/pwrnoise.h"

class DivPlatformPowerNoise: public DivDispatch {
  struct SlopePortion {
    unsigned char len, offset;
    bool clip, reset, dir;

    SlopePortion(unsigned char l, unsigned char o, bool c, bool r, bool d):
      len(l),
      offset(o),
      clip(c),
      reset(r),
      dir(d) {}
    
    SlopePortion():
      len(0),
      offset(0),
      clip(false),
      reset(false),
      dir(false) {}
  };
  
  struct Channel: public SharedChannel<signed char> {
    int fNum;
    unsigned short initLFSR;
    unsigned char octave, pan, tapA, tapB, octaveOff;
    bool slope, am, tapBEnable, keyOn, keyOff;
    SlopePortion slopeA, slopeB;
    
    Channel():
      SharedChannel<signed char>(15),
      fNum(0),
      initLFSR(0x5555),
      octave(0),
      pan(255),
      tapA(1),
      tapB(2),
      octaveOff(0),
      slope(false),
      am(false),
      tapBEnable(false),
      keyOn(false),
      keyOff(false),
      slopeA(255,1,false,false,false),
      slopeB() {}
  };
  
  Channel chan[4];
  DivDispatchOscBuffer* oscBuf[4];
  bool isMuted[4];
  unsigned char regPool[32];
  
  int coreQuality;
  power_noise_t pn;
  
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);
  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    unsigned short getPan(int chan);
    DivChannelModeHints getModeHints(int chan);
    bool getDCOffRequired();
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    int getOutputCount();
    bool hasSoftPan(int ch);
    bool keyOffAffectsArp(int ch);
    void setFlags(const DivConfig& flags);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    void setCoreQuality(unsigned char q);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformPowerNoise();
};

#endif
