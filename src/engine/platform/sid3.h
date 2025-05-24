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

#ifndef _SID3_H
#define _SID3_H

#include "../dispatch.h"
#include "../../fixedQueue.h"
#include "../waveSynth.h"
#include "sound/sid3.h"

class DivPlatformSID3: public DivDispatch {
  struct Channel: public SharedChannel<signed short> {
    int prevFreq;
    unsigned char wave, special_wave, attack, decay, sustain, sr, release;
    int duty;
    bool resetMask, resetFilter, resetDuty, gate, ring, sync, phase, oneBitNoise;
    bool phaseReset, envReset, phaseResetNoise;
    bool noiseFreqChanged;
    unsigned char vol;
    unsigned char noise_mode;
    unsigned char mix_mode;
    unsigned char ringSrc, syncSrc, phaseSrc;
    unsigned char panLeft, panRight;
    int noiseFreq;
    bool independentNoiseFreq;
    unsigned int noiseLFSRMask;

    struct Filter 
    {
      int cutoff;
      unsigned char resonance;
      unsigned char output_volume;
      unsigned char distortion_level;
      unsigned char mode;
      bool enabled;
      unsigned char filter_matrix;

      short cutoff_slide;

      bool bindCutoffToNote; //cutoff scaling
      unsigned char bindCutoffToNoteStrength; //how much cutoff changes over e.g. 1 semitone
      unsigned char bindCutoffToNoteCenter; //central note of the cutoff change
      bool bindCutoffToNoteDir; //if we decrease or increase cutoff if e.g. we go upper in note space

      bool bindResonanceToNote;
      unsigned char bindResonanceToNoteStrength; //how much resonance changes over e.g. 1 semitone
      unsigned char bindResonanceToNoteCenter; //central note of the resonance change
      bool bindResonanceToNoteDir; //if we decrease or increase resonance if e.g. we go upper in note space

      Filter():
        cutoff(0),
        resonance(0),
        output_volume(0),
        distortion_level(0),
        mode(0),
        enabled(false),
        filter_matrix(0),
        cutoff_slide(0),
        bindCutoffToNote(false),
        bindCutoffToNoteStrength(0),
        bindCutoffToNoteCenter(0),
        bindCutoffToNoteDir(false),
        bindResonanceToNote(false),
        bindResonanceToNoteStrength(0),
        bindResonanceToNoteCenter(0),
        bindResonanceToNoteDir(false) {}
    } filt[SID3_NUM_FILTERS];

    int noise_baseNoteOverride;
    bool noise_fixedArp;
    int noise_arpOff;
    int noise_pitch2;
    bool noise_hasArp;
    bool noise_hasPitch;

    bool pcm;
    int wavetable;

    long long dacPeriod, dacRate, dacOut;
    unsigned long long dacPos;
    int dacSample;

    unsigned char phaseInv;
    unsigned char feedback;

    short pw_slide;

    short phase_reset_counter;
    short noise_phase_reset_counter;
    short envelope_reset_counter;

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
      prevFreq(0xffffff),
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
      independentNoiseFreq(false),
      noiseLFSRMask((1 << 29) | (1 << 5) | (1 << 3) | 1), //https://docs.amd.com/v/u/en-US/xapp052 for 30 bits: 30, 6, 4, 1
      pcm(false),
      wavetable(-1),
      dacPeriod(0),
      dacRate(0),
      dacOut(0),
      dacPos(0),
      dacSample(-1),
      phaseInv(0),
      feedback(0),
      pw_slide(0),
      phase_reset_counter(-1),
      noise_phase_reset_counter(-1),
      envelope_reset_counter(-1) {}
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
  DivWaveSynth ws;

  unsigned char writeOscBuf;

  SID3* sid3;
  unsigned char regPool[SID3_NUM_REGISTERS];

  bool isMuted[SID3_NUM_CHANNELS];

  unsigned char sampleTick; //used to update streamed sample and not clash with other reg writes at high rate samples
  bool updateSample;
  bool quarterClock;
  
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  void updateFlags(int channel, bool gate);
  void updateFilter(int channel, int filter);
  void updateFreq(int channel);
  void updateNoiseFreq(int channel);
  void updateNoiseLFSRMask(int channel);
  void updateDuty(int channel);
  void updateEnvelope(int channel);
  void updatePanning(int channel);
  void updateWave();

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
    void notifyWaveChange(int wave); 
    float getPostAmp();
    bool getDCOffRequired();
    unsigned short getPan(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    DivChannelModeHints getModeHints(int chan);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    int getOutputCount();
    void getPaired(int ch, std::vector<DivChannelPair>& ret);
    void quit();
    ~DivPlatformSID3();
};

#endif
