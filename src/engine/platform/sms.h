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

#ifndef _SMS_H
#define _SMS_H

#include "../dispatch.h"
#include "sound/sn76496.h"
extern "C" {
  #include "../../../extern/Nuked-PSG/ympsg.h"
}
#include "../../fixedQueue.h"

class DivPlatformSMS: public DivDispatch {
  struct Channel: public SharedChannel<signed char> {
    int actualNote;
    bool writeVol;
    Channel():
      SharedChannel<signed char>(15),
      actualNote(0),
      writeVol(false) {}
  };
  Channel chan[4];
  DivDispatchOscBuffer* oscBuf[4];
  bool isMuted[4];
  unsigned char lastPan;
  unsigned char oldValue; 
  unsigned char snNoiseMode;
  unsigned char regPool[16];
  unsigned char chanLatch;
  int lastOut[2];
  int divider=16;
  double toneDivider=64.0;
  double noiseDivider=64.0;
  bool updateSNMode;
  bool resetPhase;
  bool isRealSN;
  bool stereo;
  bool nuked;
  bool easyNoise;
  sn76496_base_device* sn;
  ympsg_t sn_nuked;
  struct QueuedWrite {
    unsigned short addr;
    unsigned char val;
    bool addrOrVal;
    QueuedWrite(): addr(0), val(0), addrOrVal(false) {}
    QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
  };
  FixedQueue<QueuedWrite,128> writes;
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  double NOTE_SN(int ch, int note);
  int snCalcFreq(int ch);
  void poolWrite(unsigned short a, unsigned char v);

  void acquire_nuked(short** buf, size_t len);
  void acquire_mame(blip_buffer_t** bb, size_t len);
  public:
    void acquire(short** buf, size_t len);
    void acquireDirect(blip_buffer_t** bb, size_t len);
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
    bool hasAcquireDirect();
    bool getLegacyAlwaysSetVolume();
    float getPostAmp();
    int getPortaFloor(int ch);
    void setFlags(const DivConfig& flags);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    void setNuked(bool value);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformSMS();
};

#endif
