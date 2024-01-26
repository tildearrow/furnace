/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#pragma once

#ifndef _ES5503_H
#define _ES5503_H

#include "../dispatch.h"
#include "../../fixedQueue.h"
#include "../waveSynth.h"
#include "sound/es5503.h"

class DivPlatformES5503: public DivDispatch {
  struct Channel: public SharedChannel<int16_t> {
    int sample;
    unsigned int panleft, panright;
    bool pcm;
    int16_t wave;
    int macroVolMul;
    unsigned int wave_pos, wave_size;
    unsigned char osc_mode;
    bool softpan_channel;
    uint8_t output; //8 outputs on the chip, but Apple IIGS seems to use only two: 0=left, 1=right
    uint8_t address_bus_res; //basically octave shifter, can be automatically manipulated to have better frequency range
    //e.g. frequency higher than 0xffff => try to use lower acc bits and recalc until freq is lower than 0xffff (lower this number), 
    //frequency lower than let's say 0x100 => try to use higher acc bits in the same fashion (higher the number until freq is higher than
    //0x100; if you can't go higher stop at highest (here 0b111) and recalc frequecny, thus you have the higher precision possible)
    DivWaveSynth ws;
    int wavetable_block;
    Channel():
      SharedChannel<int16_t>(255),
      sample(-1),
      panleft(255),
      panright(255),
      pcm(false),
      wave(-1),
      macroVolMul(255),
      wave_pos(0),
      wave_size(256),
      osc_mode(0),
      output(0),
      address_bus_res(0b010),
      wavetable_block(-1) {
        softpan_channel = false;
      }
  };
  Channel chan[32];
  DivDispatchOscBuffer* oscBuf[32];
  bool isMuted[32];
  bool antiClickEnabled;
  bool mono;
  struct QueuedWrite {
    unsigned int addr;
    unsigned int val;
    QueuedWrite(): addr(0), val(0) {}
    QueuedWrite(unsigned int a, unsigned int v): addr(a), val(v) {}
  };
  FixedQueue<QueuedWrite,1024> writes;
  unsigned char lastPan;

  int curChan;
  unsigned char sampleBank, lfoMode, lfoSpeed;

  uint32_t sampleOffsets[256];
  bool sampleLoaded[256];
  uint32_t sampleLengths[256];
  uint32_t sampleMemLen;
  bool free_block[256];

  int reserved_blocks; //how many blocks of 256 bytes reserved for wavetables
  uint32_t wavetable_blocks_offsets[32]; //addresses
  bool wavetable_block_occupied[32];

  es5503_core es5503;
  unsigned char regPool[256];

  void updateWave(int ch);
  int is_enough_continuous_memory(int actualLength);
  int count_free_blocks();
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);
  public:
    void acquire(short** buf, size_t len);
    void changeNumOscs(uint8_t num_oscs);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void writeSampleMemoryByte(unsigned int address, unsigned char value);
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    int getOutputCount();
    bool keyOffAffectsArp(int ch);
    void setFlags(const DivConfig& flags);
    DivChannelPair getPaired(int ch);
    void notifyWaveChange(int wave);
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    const void* getSampleMem(int index = 0);
    bool getLegacyAlwaysSetVolume();
    size_t getSampleMemCapacity(int index = 0);
    bool isSampleLoaded(int index, int sample);
    size_t getSampleMemUsage(int index = 0);
    void renderSamples(int chipID);
};

#endif
