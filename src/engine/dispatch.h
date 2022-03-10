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
#include <vector>

#define ONE_SEMITONE 2200

#define DIV_NOTE_NULL 0x7fffffff

#define addWrite(a,v) regWrites.push_back(DivRegWrite(a,v));

// HOW TO ADD A NEW COMMAND:
// add it to this enum. then see playback.cpp.
// there is a const char* cmdName[] array, which contains the command
// names as strings for the commands (and other debug stuff).
//
// if you miss it, the program will crash or misbehave at some point.
enum DivDispatchCmds {
  DIV_CMD_NOTE_ON=0,
  DIV_CMD_NOTE_OFF,
  DIV_CMD_NOTE_OFF_ENV,
  DIV_CMD_ENV_RELEASE,
  DIV_CMD_INSTRUMENT,
  DIV_CMD_VOLUME,
  DIV_CMD_GET_VOLUME,
  DIV_CMD_GET_VOLMAX,
  DIV_CMD_NOTE_PORTA,
  DIV_CMD_PITCH,
  DIV_CMD_PANNING,
  DIV_CMD_LEGATO,
  DIV_CMD_PRE_PORTA,
  DIV_CMD_PRE_NOTE, // used in C64

  DIV_CMD_SAMPLE_MODE,
  DIV_CMD_SAMPLE_FREQ,
  DIV_CMD_SAMPLE_BANK,

  DIV_CMD_FM_LFO,
  DIV_CMD_FM_LFO_WAVE,
  DIV_CMD_FM_TL,
  DIV_CMD_FM_AR,
  DIV_CMD_FM_FB,
  DIV_CMD_FM_MULT,
  DIV_CMD_FM_EXTCH,
  DIV_CMD_FM_AM_DEPTH,
  DIV_CMD_FM_PM_DEPTH,

  DIV_CMD_GENESIS_LFO,
  
  DIV_CMD_ARCADE_LFO,

  DIV_CMD_STD_NOISE_FREQ,
  DIV_CMD_STD_NOISE_MODE,

  DIV_CMD_WAVE,
  
  DIV_CMD_GB_SWEEP_TIME,
  DIV_CMD_GB_SWEEP_DIR,

  DIV_CMD_PCE_LFO_MODE,
  DIV_CMD_PCE_LFO_SPEED,

  DIV_CMD_NES_SWEEP,

  DIV_CMD_C64_CUTOFF,
  DIV_CMD_C64_RESONANCE,
  DIV_CMD_C64_FILTER_MODE,
  DIV_CMD_C64_RESET_TIME,
  DIV_CMD_C64_RESET_MASK,
  DIV_CMD_C64_FILTER_RESET,
  DIV_CMD_C64_DUTY_RESET,
  DIV_CMD_C64_EXTENDED,
  DIV_CMD_C64_FINE_DUTY,
  DIV_CMD_C64_FINE_CUTOFF,

  DIV_CMD_AY_ENVELOPE_SET,
  DIV_CMD_AY_ENVELOPE_LOW,
  DIV_CMD_AY_ENVELOPE_HIGH,
  DIV_CMD_AY_ENVELOPE_SLIDE,
  DIV_CMD_AY_NOISE_MASK_AND,
  DIV_CMD_AY_NOISE_MASK_OR,
  DIV_CMD_AY_AUTO_ENVELOPE,

  DIV_CMD_SAA_ENVELOPE,

  DIV_CMD_LYNX_LFSR_LOAD,
  
  DIV_CMD_QSOUND_ECHO_FEEDBACK,
  DIV_CMD_QSOUND_ECHO_DELAY,
  DIV_CMD_QSOUND_ECHO_LEVEL,

  DIV_CMD_WS_SWEEP_TIME,
  DIV_CMD_WS_SWEEP_AMOUNT,

  DIV_ALWAYS_SET_VOLUME,

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
   * - 0xffffffff: reset
   */
  unsigned int addr;
  unsigned short val;
  DivRegWrite(unsigned int a, unsigned short v):
    addr(a), val(v) {}
};

class DivEngine;

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
     */
    int rate;
    
    /**
     * the actual chip's clock.
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
     */
    virtual void tick();

    /**
     * get the state of a channel.
     * @return a pointer, or NULL.
     */
    virtual void* getChanState(int chan);
    
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
     * get a description of a dispatch-specific effect.
     * @param effect the effect.
     * @return the description, or NULL if effect is invalid.
     */
     virtual const char* getEffectName(unsigned char effect);

    /**
     * set the chip flags.
     * @param flags the flags. see song.h for possible values.
     */
    virtual void setFlags(unsigned int flags);

    /**
     * set skip reg writes.
     */
    void setSkipRegisterWrites(bool value);

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
     * initialize this DivDispatch.
     * @param parent the parent DivEngine.
     * @param channels the number of channels to acquire.
     * @param sugRate the suggested rate. this may change, so don't rely on it.
     * @param flags the chip flags. see song.h for possible values.
     * @return the number of channels allocated.
     */
    virtual int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);

    /**
     * quit the DivDispatch.
     */
     virtual void quit();

     virtual ~DivDispatch();
};

#define NOTE_PERIODIC(x) parent->calcBaseFreq(chipClock,CHIP_DIVIDER,x,true)
#define NOTE_FREQUENCY(x) parent->calcBaseFreq(chipClock,CHIP_FREQBASE,x,false)

#define COLOR_NTSC (315000000.0/88.0)
#define COLOR_PAL (283.75*15625.0+25.0)

#endif
