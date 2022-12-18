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

#ifndef _DISPATCH_H
#define _DISPATCH_H

#include <stdlib.h>
#include <string.h>
#include <vector>
#include "config.h"
#include "chipUtils.h"

#define ONE_SEMITONE 2200

#define DIV_NOTE_NULL 0x7fffffff

#define addWrite(a,v) regWrites.push_back(DivRegWrite(a,v));

// HOW TO ADD A NEW COMMAND:
// add it to this enum. then see playback.cpp.
// there is a const char* cmdName[] array, which contains the command
// names as strings for the commands (and other debug stuff).
//
// if you miss it, the program will crash or misbehave at some point.
//
// the comments are: (arg1, arg2) -> val
// not all commands have a return value
enum DivDispatchCmds {
  DIV_CMD_NOTE_ON=0, // (note)
  DIV_CMD_NOTE_OFF,
  DIV_CMD_NOTE_OFF_ENV,
  DIV_CMD_ENV_RELEASE,
  DIV_CMD_INSTRUMENT, // (ins, force)
  DIV_CMD_VOLUME, // (vol)
  DIV_CMD_GET_VOLUME, // () -> vol
  DIV_CMD_GET_VOLMAX, // () -> volMax
  DIV_CMD_NOTE_PORTA, // (target, speed) -> 2 if target reached
  DIV_CMD_PITCH, // (pitch)
  DIV_CMD_PANNING, // (left, right)
  DIV_CMD_LEGATO, // (note)
  DIV_CMD_PRE_PORTA, // (inPorta, isPortaOrSlide)
  DIV_CMD_PRE_NOTE, // used in C64 (note)

  // these will be used in ROM export.
  // do NOT implement!
  DIV_CMD_HINT_VIBRATO, // (speed, depth)
  DIV_CMD_HINT_VIBRATO_RANGE, // (range)
  DIV_CMD_HINT_VIBRATO_SHAPE, // (shape)
  DIV_CMD_HINT_PITCH, // (pitch)
  DIV_CMD_HINT_ARPEGGIO, // (note1, note2)
  DIV_CMD_HINT_VOLUME, // (vol)
  DIV_CMD_HINT_VOL_SLIDE, // (amount, oneTick)
  DIV_CMD_HINT_PORTA, // (target, speed)
  DIV_CMD_HINT_LEGATO, // (note)

  DIV_CMD_SAMPLE_MODE, // (enabled)
  DIV_CMD_SAMPLE_FREQ, // (frequency)
  DIV_CMD_SAMPLE_BANK, // (bank)
  DIV_CMD_SAMPLE_POS, // (pos)
  DIV_CMD_SAMPLE_DIR, // (direction)

  DIV_CMD_FM_HARD_RESET, // (enabled)
  DIV_CMD_FM_LFO, // (speed)
  DIV_CMD_FM_LFO_WAVE, // (waveform)
  DIV_CMD_FM_TL, // (op, value)
  DIV_CMD_FM_AM, // (op, value)
  DIV_CMD_FM_AR, // (op, value)
  DIV_CMD_FM_DR, // (op, value)
  DIV_CMD_FM_SL, // (op, value)
  DIV_CMD_FM_D2R, // (op, value)
  DIV_CMD_FM_RR, // (op, value)
  DIV_CMD_FM_DT, // (op, value)
  DIV_CMD_FM_DT2, // (op, value)
  DIV_CMD_FM_RS, // (op, value)
  DIV_CMD_FM_KSR, // (op, value)
  DIV_CMD_FM_VIB, // (op, value)
  DIV_CMD_FM_SUS, // (op, value)
  DIV_CMD_FM_WS, // (op, value)
  DIV_CMD_FM_SSG, // (op, value)
  DIV_CMD_FM_REV, // (op, value)
  DIV_CMD_FM_EG_SHIFT, // (op, value)
  DIV_CMD_FM_FB, // (value)
  DIV_CMD_FM_MULT, // (op, value)
  DIV_CMD_FM_FINE, // (op, value)
  DIV_CMD_FM_FIXFREQ, // (op, value)
  DIV_CMD_FM_EXTCH, // (enabled)
  DIV_CMD_FM_AM_DEPTH, // (depth)
  DIV_CMD_FM_PM_DEPTH, // (depth)

  DIV_CMD_GENESIS_LFO, // unused?
  
  DIV_CMD_ARCADE_LFO, // unused?

  DIV_CMD_STD_NOISE_FREQ, // (freq)
  DIV_CMD_STD_NOISE_MODE, // (mode)

  DIV_CMD_WAVE, // (waveform)
  
  DIV_CMD_GB_SWEEP_TIME, // (time)
  DIV_CMD_GB_SWEEP_DIR, // (direction)

  DIV_CMD_PCE_LFO_MODE, // (mode)
  DIV_CMD_PCE_LFO_SPEED, // (speed)

  DIV_CMD_NES_SWEEP, // (direction, value)
  DIV_CMD_NES_DMC, // (value)

  DIV_CMD_C64_CUTOFF, // (value)
  DIV_CMD_C64_RESONANCE, // (value)
  DIV_CMD_C64_FILTER_MODE, // (value)
  DIV_CMD_C64_RESET_TIME, // (value)
  DIV_CMD_C64_RESET_MASK, // (mask)
  DIV_CMD_C64_FILTER_RESET, // (value)
  DIV_CMD_C64_DUTY_RESET, // (value)
  DIV_CMD_C64_EXTENDED, // (value)
  DIV_CMD_C64_FINE_DUTY, // (value)
  DIV_CMD_C64_FINE_CUTOFF, // (value)

  DIV_CMD_AY_ENVELOPE_SET,
  DIV_CMD_AY_ENVELOPE_LOW,
  DIV_CMD_AY_ENVELOPE_HIGH,
  DIV_CMD_AY_ENVELOPE_SLIDE,
  DIV_CMD_AY_NOISE_MASK_AND, // (value)
  DIV_CMD_AY_NOISE_MASK_OR, // (value)
  DIV_CMD_AY_AUTO_ENVELOPE, // (value)
  DIV_CMD_AY_IO_WRITE, // (port, value)
  DIV_CMD_AY_AUTO_PWM,

  DIV_CMD_FDS_MOD_DEPTH,
  DIV_CMD_FDS_MOD_HIGH,
  DIV_CMD_FDS_MOD_LOW,
  DIV_CMD_FDS_MOD_POS,
  DIV_CMD_FDS_MOD_WAVE,

  DIV_CMD_SAA_ENVELOPE, // (value)

  DIV_CMD_AMIGA_FILTER, // (enabled)
  DIV_CMD_AMIGA_AM, // (enabled)
  DIV_CMD_AMIGA_PM, // (enabled)

  DIV_CMD_LYNX_LFSR_LOAD, // (value)
  
  DIV_CMD_QSOUND_ECHO_FEEDBACK,
  DIV_CMD_QSOUND_ECHO_DELAY,
  DIV_CMD_QSOUND_ECHO_LEVEL,
  DIV_CMD_QSOUND_SURROUND,

  DIV_CMD_X1_010_ENVELOPE_SHAPE,
  DIV_CMD_X1_010_ENVELOPE_ENABLE,
  DIV_CMD_X1_010_ENVELOPE_MODE,
  DIV_CMD_X1_010_ENVELOPE_PERIOD,
  DIV_CMD_X1_010_ENVELOPE_SLIDE,
  DIV_CMD_X1_010_AUTO_ENVELOPE,
  DIV_CMD_X1_010_SAMPLE_BANK_SLOT,

  DIV_CMD_WS_SWEEP_TIME,
  DIV_CMD_WS_SWEEP_AMOUNT,

  DIV_CMD_N163_WAVE_POSITION,
  DIV_CMD_N163_WAVE_LENGTH,
  DIV_CMD_N163_WAVE_MODE,
  DIV_CMD_N163_WAVE_LOAD,
  DIV_CMD_N163_WAVE_LOADPOS,
  DIV_CMD_N163_WAVE_LOADLEN,
  DIV_CMD_N163_WAVE_LOADMODE,
  DIV_CMD_N163_CHANNEL_LIMIT,
  DIV_CMD_N163_GLOBAL_WAVE_LOAD,
  DIV_CMD_N163_GLOBAL_WAVE_LOADPOS,
  DIV_CMD_N163_GLOBAL_WAVE_LOADLEN,
  DIV_CMD_N163_GLOBAL_WAVE_LOADMODE,

  DIV_CMD_SU_SWEEP_PERIOD_LOW, // (which, val)
  DIV_CMD_SU_SWEEP_PERIOD_HIGH, // (which, val)
  DIV_CMD_SU_SWEEP_BOUND, // (which, val)
  DIV_CMD_SU_SWEEP_ENABLE, // (which, val)
  DIV_CMD_SU_SYNC_PERIOD_LOW,
  DIV_CMD_SU_SYNC_PERIOD_HIGH,

  DIV_CMD_ADPCMA_GLOBAL_VOLUME,

  DIV_CMD_SNES_ECHO,
  DIV_CMD_SNES_PITCH_MOD,
  DIV_CMD_SNES_INVERT,
  DIV_CMD_SNES_GAIN_MODE,
  DIV_CMD_SNES_GAIN,
  DIV_CMD_SNES_ECHO_ENABLE,
  DIV_CMD_SNES_ECHO_DELAY,
  DIV_CMD_SNES_ECHO_VOL_LEFT,
  DIV_CMD_SNES_ECHO_VOL_RIGHT,
  DIV_CMD_SNES_ECHO_FEEDBACK,
  DIV_CMD_SNES_ECHO_FIR,

  DIV_CMD_NES_ENV_MODE,
  DIV_CMD_NES_LENGTH,
  DIV_CMD_NES_COUNT_MODE,

  DIV_CMD_MACRO_OFF, // (which)
  DIV_CMD_MACRO_ON, // (which)

  DIV_ALWAYS_SET_VOLUME, // () -> alwaysSetVol

  DIV_CMD_MAX
};

struct DivCommand {
  DivDispatchCmds cmd;
  unsigned char chan, dis;
  int value, value2;
  DivCommand(DivDispatchCmds c, unsigned char ch, int val, int val2):
    cmd(c),
    chan(ch),
    dis(ch),
    value(val),
    value2(val2) {}
  DivCommand(DivDispatchCmds c, unsigned char ch, int val):
    cmd(c),
    chan(ch),
    dis(ch),
    value(val),
    value2(0) {}
  DivCommand(DivDispatchCmds c, unsigned char ch):
    cmd(c),
    chan(ch),
    dis(ch),
    value(0),
    value2(0) {}
};

struct DivDelayedCommand {
  int ticks;
  DivCommand cmd;
};

struct DivRegWrite {
  /**
   * an address of 0xffffxx00 indicates a Furnace specific command.
   * the following addresses are available:
   * - 0xffffxx00: start sample playback
   *   - xx is the instance ID
   *   - data is the sample number
   * - 0xffffxx01: set sample rate
   *   - xx is the instance ID
   *   - data is the sample rate
   * - 0xffffxx02: stop sample playback
   *   - xx is the instance ID
   * - 0xffffxx03: set sample playback direction
   *   - x is the instance ID
   * - 0xffffffff: reset
   */
  unsigned int addr;
  unsigned short val;
  DivRegWrite(unsigned int a, unsigned short v):
    addr(a), val(v) {}
};

struct DivDelayedWrite {
  int time;
  DivRegWrite write;
  DivDelayedWrite(int t, unsigned int a, unsigned short v):
    time(t),
    write(a,v) {}
};

struct DivDispatchOscBuffer {
  bool follow;
  unsigned int rate;
  unsigned short needle;
  unsigned short readNeedle;
  unsigned short followNeedle;
  short data[65536];

  DivDispatchOscBuffer():
    follow(true),
    rate(65536),
    needle(0),
    readNeedle(0),
    followNeedle(0) {
    memset(data,0,65536*sizeof(short));
  }
};

class DivEngine;
class DivMacroInt;

class DivDispatch {
  protected:
    DivEngine* parent;
    std::vector<DivRegWrite> regWrites;
    /**
     * please honor these variables if needed.
     */
    bool skipRegisterWrites, dumpWrites;
  public:
    /**
     * the rate the samples are provided.
     * the engine shall resample to the output rate.
     * you have to initialize this one during init() or setFlags().
     */
    int rate;
    
    /**
     * the actual chip's clock.
     * you have to initialize this one during init() or setFlags().
     */
    int chipClock;

    /**
     * fill a buffer with sound data.
     * @param bufL the left or mono channel buffer.
     * @param bufR the right channel buffer.
     * @param start the start offset.
     * @param len the amount of samples to fill.
     */
    virtual void acquire(short* bufL, short* bufR, size_t start, size_t len);

    /**
     * fill a write stream with data (e.g. for software-mixed PCM).
     * @param stream the write stream.
     * @param rate stream rate (e.g. 44100 for VGM).
     * @param len number of samples.
     */
    virtual void fillStream(std::vector<DivDelayedWrite>& stream, int sRate, size_t len);

    /**
     * send a command to this dispatch.
     * @param c a DivCommand.
     * @return a return value which varies depending on the command.
     */
    virtual int dispatch(DivCommand c);

    /**
     * reset the state of this dispatch.
     */
    virtual void reset();

    /**
     * ticks this dispatch.
     * @param sysTick whether the engine has ticked (if not then this may be a sub-tick used in low-latency mode).
     */
    virtual void tick(bool sysTick=true);

    /**
     * get the state of a channel.
     * @return a pointer, or NULL.
     */
    virtual void* getChanState(int chan);

    /**
     * get the DivMacroInt of a chanmel.
     * @return a pointer, or NULL.
     */
    virtual DivMacroInt* getChanMacroInt(int chan);

    /**
     * get an oscilloscope buffer for a channel.
     * @return a pointer to a DivDispatchOscBuffer, or NULL if not supported.
     */
    virtual DivDispatchOscBuffer* getOscBuffer(int chan);
    
    /**
     * get the register pool of this dispatch.
     * @return a pointer, or NULL.
     */
    virtual unsigned char* getRegisterPool();

    /**
     * get the size of the register pool of this dispatch.
     * @return the size.
     */
    virtual int getRegisterPoolSize();

    /**
     * get the bit depth of the register pool of this dispatch.
     * If the result is 16, it should be casted to unsigned short.
     * @return the depth. Default value is 8
     */
    virtual int getRegisterPoolDepth();

    /**
     * get this dispatch's state. DO NOT IMPLEMENT YET.
     * @return a pointer to the dispatch's state. must be deallocated manually!
     */
    virtual void* getState();

    /**
     * set this dispatch's state. DO NOT IMPLEMENT YET.
     * @param state a pointer to a state pertaining to this dispatch,
     * or NULL if this dispatch does not support state saves.
     */
    virtual void setState(void* state);

    /**
     * mute a channel.
     * @param ch the channel to mute.
     * @param mute whether to mute or unmute.
     */
    virtual void muteChannel(int ch, bool mute);

    /**
     * test whether this dispatch outputs audio in two channels.
     * @return whether it does.
     */
    virtual bool isStereo();

    /**
     * test whether sending a key off command to a channel should reset arp too.
     * @param ch the channel in question.
     * @return whether it does.
     */
    virtual bool keyOffAffectsArp(int ch);

    /**
     * test whether sending a key off command to a channel should reset slides too.
     * @param ch the channel in question.
     * @return whether it does.
     */
    virtual bool keyOffAffectsPorta(int ch);

    /**
     * get the lowest note in a portamento.
     * @param ch the channel in question.
     * @return the lowest note.
     */
    virtual int getPortaFloor(int ch);

    /**
     * get the required amplification level of this dispatch's output.
     * @return the amplification level.
     */
    virtual float getPostAmp();

    /**
     * check whether DC offset correction is required.
     * @return truth.
     */
    virtual bool getDCOffRequired();

    /**
     * check whether PRE_NOTE command is desired.
     * @return truth.
     */
    virtual bool getWantPreNote();

    /**
     * get minimum chip clock.
     * @return clock in Hz, or 0 if custom clocks are not supported.
     */
    virtual int getClockRangeMin();

    /**
     * get maximum chip clock.
     * @return clock in Hz, or 0 if custom clocks are not supported.
     */
    virtual int getClockRangeMax();

    /**
     * set the chip flags.
     * @param flags a DivConfig containing chip flags.
     */
    virtual void setFlags(const DivConfig& flags);

    /**
     * set skip reg writes.
     */
    virtual void setSkipRegisterWrites(bool value);

    /**
     * notify instrument change.
     */
    virtual void notifyInsChange(int ins);

    /**
     * notify wavetable change.
     */
    virtual void notifyWaveChange(int wave);

    /**
     * notify deletion of an instrument.
     */
    virtual void notifyInsDeletion(void* ins);

    /**
     * notify that playback stopped.
     */
    virtual void notifyPlaybackStop();

    /**
     * force-retrigger instruments.
     */
    virtual void forceIns();

    /**
     * enable register dumping.
     */
    virtual void toggleRegisterDump(bool enable);

    /**
     * get register writes.
     */
    std::vector<DivRegWrite>& getRegisterWrites();

    /**
     * poke a register.
     * @param addr address.
     * @param val value.
     */
    virtual void poke(unsigned int addr, unsigned short val);

    /**
     * poke a register.
     * @param wlist a vector containing DivRegWrites.
     */
    virtual void poke(std::vector<DivRegWrite>& wlist);

    /**
     * get available registers.
     * @return an array of C strings, terminated by NULL; or NULL if none available.
     */
    virtual const char** getRegisterSheet();

    /**
     * Get sample memory buffer.
     * @param index the memory index.
     * @return a pointer to sample memory, or NULL.
     */
    virtual const void* getSampleMem(int index = 0);

    /**
     * Get sample memory capacity.
     * @param index the memory index.
     * @return memory capacity in bytes, or 0 if memory doesn't exist.
     */
    virtual size_t getSampleMemCapacity(int index = 0);

    /**
     * get sample memory name.
     * @param index the memory index.
     * @return a name, or NULL if it doesn't have any name in particular.
     */
    virtual const char* getSampleMemName(int index=0);

    /**
     * Get sample memory usage.
     * @param index the memory index.
     * @return memory usage in bytes.
     */
    virtual size_t getSampleMemUsage(int index = 0);

    /**
     * check whether sample has been loaded in memory.
     * @param memory index.
     * @param sample the sample in question.
     * @return whether it did.
     */
    virtual bool isSampleLoaded(int index, int sample);
    

    /**
     * Render samples into sample memory.
     * @param sysID the chip's index in the chip list.
     */
    virtual void renderSamples(int sysID);

    /**
     * initialize this DivDispatch.
     * @param parent the parent DivEngine.
     * @param channels the number of channels to acquire.
     * @param sugRate the suggested rate. this may change, so don't rely on it.
     * @param flags a DivConfig containing chip flags.
     * @return the number of channels allocated.
     */
    virtual int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);

    /**
     * quit the DivDispatch.
     */
    virtual void quit();

    virtual ~DivDispatch();
};

// custom chip clock helper define. put in setFlags, but before rate is set.
#define CHECK_CUSTOM_CLOCK \
  if (flags.getInt("customClock",0)>0) { \
    chipClock=flags.getInt("customClock",getClockRangeMin()); \
    if (chipClock>getClockRangeMax()) chipClock=getClockRangeMax(); \
    if (chipClock<getClockRangeMin()) chipClock=getClockRangeMin(); \
  }

// pitch calculation:
// - a DivDispatch usually contains four variables per channel:
//   - baseFreq: this changes on new notes, legato, arpeggio and slides.
//   - pitch: this changes with DIV_CMD_PITCH (E5xx/04xy).
//   - freq: this is the result of combining baseFreq and pitch using DivEngine::calcFreq().
//   - freqChanged: whether baseFreq and/or pitch have changed, and a frequency recalculation is required on the next tick.
// - the following definitions will help you calculate baseFreq.
// - to use them, define CHIP_DIVIDER and/or CHIP_FREQBASE in your code (not in the header though!).
#define NOTE_PERIODIC(x) round(parent->calcBaseFreq(chipClock,CHIP_DIVIDER,x,true))
#define NOTE_PERIODIC_NOROUND(x) parent->calcBaseFreq(chipClock,CHIP_DIVIDER,x,true)
#define NOTE_FREQUENCY(x) parent->calcBaseFreq(chipClock,CHIP_FREQBASE,x,false)

// this is a special case definition. only use it for f-num/block-based chips.
#define NOTE_FNUM_BLOCK(x,bits) parent->calcBaseFreqFNumBlock(chipClock,CHIP_FREQBASE,x,bits)

// this is for volume scaling calculation.
#define VOL_SCALE_LINEAR(x,y,range) (((x)*(y))/(range))
#define VOL_SCALE_LOG(x,y,range) (CLAMP(((x)+(y))-(range),0,(range)))
#define VOL_SCALE_LINEAR_BROKEN(x,y,range) ((parent->song.newVolumeScaling)?(VOL_SCALE_LINEAR(x,y,range)):(VOL_SCALE_LOG(x,y,range)))
#define VOL_SCALE_LOG_BROKEN(x,y,range) ((parent->song.newVolumeScaling)?(VOL_SCALE_LOG(x,y,range)):(VOL_SCALE_LINEAR(x,y,range)))

// these are here for convenience.
// it is encouraged to use these, since you get an exact value this way.
// - NTSC colorburst: 3.58MHz
// - PAL colorburst: 4.43MHz
#define COLOR_NTSC (315000000.0/88.0)
#define COLOR_PAL (283.75*15625.0+25.0)

#define CLAMP_VAR(x,xMin,xMax) \
  if (x<xMin) x=xMin; \
  if (x>xMax) x=xMax;

#define NEW_ARP_STRAT (parent->song.linearPitch==2 && !parent->song.oldArpStrategy)
#define HACKY_LEGATO_MESS chan[c.chan].std.arp.will && !chan[c.chan].std.arp.mode && !NEW_ARP_STRAT

#endif
