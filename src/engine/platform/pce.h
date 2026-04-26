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

// pce.h: DivDispatch for PC Engine/TurboGrafx-16.

#ifndef _PCE_H
#define _PCE_H

#include "../dispatch.h"
#include "../../fixedQueue.h"
#include "../waveSynth.h"
#include "sound/pce_psg.h"

/**
 * this is the code that implements PC Engine as a DivDispatch.
 * I am going to document this well as I often use this as a template when adding new chips.
 * feel free to strip out the comments in your implementation.
 *
 * if your chip resembles a Yamaha FM one, opl.h or ym*.h may suit you better.
 * otherwise this has pretty much everything. macros, wavetable and sample.
 *
 * we prefix each dispatch implementation class with DivPlatform. during some time I called these chips
 * "platforms" and the name stuck on.
 */
class DivPlatformPCE: public DivDispatch {
  /**
   * this is our Channel struct. it is based on SharedChannel and adds chip-specific state on top.
   */
  struct Channel: public SharedChannel {
    // anti-click related variables.
    // - anti-click is a trick which predicts the channel's current waveform position
    //   and adds that offset when updating the waveform.
    // - this effectively works around a quirk in certain chips (PCE and GameBoy are examples)
    //   which reset the waveform phase during a wave change.
    int antiClickPeriodCount, antiClickWavePos;
    // software PCM related variables.
    int dacPeriod, dacRate, dacOut;
    unsigned int dacPos;
    int dacSample;
    // panning (as a single digit).
    unsigned char pan;
    // noise mode and software PCM state.
    // deferredWaveUpdate is used when we're in PCM mode (we can't change the waveform there).
    // setPos is set when a sample offset effect is executed.
    bool noise, pcm, deferredWaveUpdate, setPos;
    // current waveform.
    signed short wave;
    // macroVolMul sets the multiplier for the volume macro.
    // - sample-supporting chips may use their own instrument type or the Generic Sample one for
    //   sample mode.
    // - Generic Sample has a volume macro range of 0-64. that's why this variable exists.
    int macroVolMul, noiseSeek;
    // wave synth - a tiny wave morphing engine for waveform effects.
    DivWaveSynth ws;
    // here's our constructor. notice how we set the default volume to maximum.
    Channel(bool linear=true):
      SharedChannel(31,linear),
      antiClickPeriodCount(0),
      antiClickWavePos(0),
      dacPeriod(0),
      dacRate(0),
      dacOut(0),
      dacPos(0),
      dacSample(-1),
      pan(255),
      noise(false),
      pcm(false),
      deferredWaveUpdate(false),
      setPos(false),
      wave(-1),
      macroVolMul(31),
      noiseSeek(0) {}
  };
  // channel state. change this number appropriately.
  Channel chan[6];
  // per-channel oscilloscope buffers. we allocate this on init() and free it on quit().
  DivDispatchOscBuffer* oscBuf[6];
  // channel mute status. set on init().
  bool isMuted[6];

  // this is a chip flag. we let the user disable anti-click.
  bool antiClickEnabled;
  // chip-specific state.
  bool updateLFO;
  // most chips don't allow us to make more than one write per sample or cycle.
  // we employ a queue to work around this.
  struct QueuedWrite {
    unsigned char addr;
    unsigned char val;
    QueuedWrite(): addr(0), val(9) {}
    QueuedWrite(unsigned char a, unsigned char v): addr(a), val(v) {}
  };
  FixedQueue<QueuedWrite,512> writes;

  // PCE is a bit unique. you've got a channel selection register which you must write to
  // before setting channel registers.
  int curChan;
  // chip-specific state.
  unsigned char lfoMode, lfoSpeed;

  // an instance of a PC Engine sound chip emulator.
  PCE_PSG* pce;

  // regPool contains a copy of all registers so we can display them in the GUI.
  unsigned char regPool[128];

  // private functions.
  void updateWave(int ch);
  // these two were used in the debug window. keep them here just in case.
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);
  public:
    void acquire(short** buf, size_t len);
    void acquireDirect(blip_buffer_t** bb, size_t len);
    int dispatch(DivCommand c);
    SharedChannel* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    unsigned short getPan(int chan);
    void getPaired(int ch, std::vector<DivChannelPair>& ret);
    DivChannelModeHints getModeHints(int chan);
    DivSamplePos getSamplePos(int ch);
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
    bool hasSoftPan(int ch);
    bool keyOffAffectsArp(int ch);
    bool hasAcquireDirect();
    void setFlags(const DivConfig& flags);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformPCE();
};

// check out pce.cpp for the code. have fun!

#endif
