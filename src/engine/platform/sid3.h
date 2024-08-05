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

#ifndef _SID3_H
#define _SID3_H

#include "../dispatch.h"
#include "../../fixedQueue.h"
#include "sound/sid3.h"

class DivPlatformSID3: public DivDispatch {
  struct Channel: public SharedChannel<signed short> {
    int prevFreq;
    unsigned char wave, special_wave, attack, decay, sustain, sr, release;
    unsigned short duty;
    bool resetMask, resetFilter, resetDuty, gate, ring, sync, phase, oneBitNoise;
    bool phaseReset, envReset, phaseResetNoise;
    bool noiseFreqChanged;
    unsigned char vol;
    unsigned char noise_mode;
    unsigned char mix_mode;
    unsigned char ringSrc, syncSrc, phaseSrc;
    unsigned char panLeft, panRight;
    unsigned int noiseFreq;
    bool independentNoiseFreq;

    struct Filter 
    {
      unsigned short cutoff;
      unsigned char resonance;
      unsigned char output_volume;
      unsigned char distortion_level;
      unsigned char mode;
      bool enabled;
      unsigned char filter_matrix;

      Filter():
        cutoff(0),
        resonance(0),
        output_volume(0),
        distortion_level(0),
        mode(0),
        enabled(false),
        filter_matrix(0) {}
    } filt[SID3_NUM_FILTERS];

    int noise_baseNoteOverride;
    bool noise_fixedArp;
    int noise_arpOff;
    int noise_pitch2;
    bool noise_hasArp;
    bool noise_hasPitch;

    void handleArpNoise(int offset=0) 
    {
      DivMacroStruct& m = this->std.op[3].am;

      if (m.had) {
        noise_hasArp=true;

        if (m.val<0) {
          if (!(m.val&0x40000000)) {
            noise_baseNoteOverride=(m.val|0x40000000)+offset;
            noise_fixedArp=true;
          } else {
            noise_arpOff=m.val;
            noise_fixedArp=false;
          }
        } else {
          if (m.val&0x40000000) {
            noise_baseNoteOverride=(m.val&(~0x40000000))+offset;
            noise_fixedArp=true;
          } else {
            noise_arpOff=m.val;
            noise_fixedArp=false;
          }
        }
        noiseFreqChanged=true;
      }

      else
      {
        noise_hasArp=false;
      }
    }

    void handlePitchNoise()
    {
      DivMacroStruct& m = this->std.op[0].ar;

      if (m.had) {
        noise_hasPitch=true;

        if (m.mode) {
          noise_pitch2+=m.val;
          CLAMP_VAR(noise_pitch2,-131071,131071);
        } else {
          noise_pitch2=m.val;
        }
        noiseFreqChanged=true;
      }

      else
      {
        noise_hasPitch=false;
      }
    }

    Channel():
      SharedChannel<signed short>(SID3_MAX_VOL),
      prevFreq(0x1ffff),
      wave(0),
      special_wave(0),
      attack(0),
      decay(0),
      sustain(0),
      sr(0),
      release(0),
      duty(0),
      resetMask(false),
      resetFilter(false),
      resetDuty(false),
      gate(true),
      ring(false),
      sync(false),
      phase(false),
      oneBitNoise(false),
      phaseReset(false),
      envReset(false),
      phaseResetNoise(false),
      noiseFreqChanged(false),
      vol(SID3_MAX_VOL),
      noise_mode(0),
      mix_mode(0),
      ringSrc(0),
      syncSrc(0),
      phaseSrc(0),
      panLeft(0xff),
      panRight(0xff),
      noiseFreq(0),
      independentNoiseFreq(false) {}
  };
  Channel chan[SID3_NUM_CHANNELS];
  DivDispatchOscBuffer* oscBuf[SID3_NUM_CHANNELS];
  struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      QueuedWrite(): addr(0), val(0) {}
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v) {}
  };
  FixedQueue<QueuedWrite,SID3_NUM_REGISTERS * 4> writes;

  unsigned char writeOscBuf;

  SID3* sid3;
  unsigned char regPool[SID3_NUM_REGISTERS];

  bool isMuted[SID3_NUM_CHANNELS];
  
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  void updateFlags(int channel, bool gate);
  void updateFilter(int channel, int filter);
  void updateFreq(int channel);
  void updateNoiseFreq(int channel);
  void updateDuty(int channel);
  void updateEnvelope(int channel);
  void updatePanning(int channel);
  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    void setFlags(const DivConfig& flags);
    void notifyInsChange(int ins);
    float getPostAmp();
    bool getDCOffRequired();
    DivMacroInt* getChanMacroInt(int ch);
    DivChannelModeHints getModeHints(int chan);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    int getOutputCount();
    void quit();
    ~DivPlatformSID3();
};

#endif
