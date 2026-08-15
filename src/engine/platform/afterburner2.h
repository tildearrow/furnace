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

#ifndef _AFTERBURNER2_H
#define _AFTERBURNER2_H

#include "../dispatch.h"
#include "../macroInt.h"
#include "../waveSynth.h"

// channel layout: 0-2: PSG1-3 (square/saw/1-bit noise, PSG3 also has wavetable)
#define AFTB2_NUM_CHANS 3

class DivPlatformAfterburner2: public DivDispatch {
  struct Channel: public SharedChannel {
    unsigned char duty;   // 4-bit duty / saw-flip flag
    unsigned char wave;   // 0: square, 1: saw, 2: noise (shape select, effect 0x10)
    int waveIdx;          // PSG3 only: index of the loaded wavetable asset (-1: none yet)
    double phase;         // phase accumulator, 0..1
    float noiseBit;       // held 1-bit noise output, re-rolled once per cycle
    Channel(bool linear=true):
      SharedChannel(15,linear),
      duty(0),
      wave(0),
      waveIdx(-1),
      phase(0.0),
      noiseBit(-1.0f) {}
  };
  Channel chan[AFTB2_NUM_CHANS];
  DivDispatchOscBuffer* oscBuf[AFTB2_NUM_CHANS];
  bool isMuted[AFTB2_NUM_CHANS];
  DivWaveSynth ws;

  // mirrors $9000-$901F (regPool index = real addr - 0x9000)
  // $9010-$901F: 32 x 4-bit wavetable steps, 2 steps packed per byte
  unsigned char regPool[32];

  unsigned char masterVol;
  unsigned char psgVol[3];
  unsigned char psg2Bitwise; // 0: off, 1: AND, 2: NAND, 3: OR, 4: NOR, 5: XOR, 6: XNOR (with PSG3)
  bool psg3WaveEnable;

  DivPitchTable pitchTable;

  void rWrite(unsigned short addr, unsigned char val);
  unsigned char getWavetableSample(int step);
  void updateWave();

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
    const char** getRegisterSheet();
    int getOutputCount();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    void setFlags(const DivConfig& flags);
    void notifyWaveChange(int wave);
    void getPaired(int chan, std::vector<DivChannelPair>& ret);
    void notifyInsDeletion(void* ins);
    void notifyPitchTable(int sample=-1);
    unsigned int getMaxFreq(int ch);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformAfterburner2();
};

#endif
