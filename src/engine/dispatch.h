#ifndef _DISPATCH_H
#define _DISPATCH_H

#include <stdlib.h>
#include <vector>

#define ONE_SEMITONE 2200

#define DIV_NOTE_NULL 0x7fffffff

#define addWrite(a,v) regWrites.push_back(DivRegWrite(a,v));

enum DivDispatchCmds {
  DIV_CMD_NOTE_ON=0,
  DIV_CMD_NOTE_OFF,
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

  DIV_CMD_GENESIS_LFO,
  
  DIV_CMD_ARCADE_LFO,

  DIV_CMD_STD_NOISE_FREQ,
  DIV_CMD_STD_NOISE_MODE,

  DIV_CMD_WAVE,
  
  DIV_CMD_GB_SWEEP_TIME,
  DIV_CMD_GB_SWEEP_DIR,

  DIV_CMD_PCE_LFO_MODE,
  DIV_CMD_PCE_LFO_SPEED,

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
   * an address of 0xffffff00 indicates a Furnace specific command.
   * the following addresses are available:
   * - 0xffffff00: start sample playback
   *   - data is the sample number
   */
  unsigned int addr;
  unsigned char val;
  DivRegWrite(unsigned int a, unsigned char v):
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
     * get this dispatch's state.
     * @return a pointer to the dispatch's state. must be deallocated manually!
     */
    virtual void* getState();

    /**
     * set this dispatch's state.
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
     * set the region to PAL.
     * @param pal whether to set it to PAL.
     */
    virtual void setPAL(bool pal);

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
     * force-retrigger instruments.
     */
    virtual void forceIns();

    /**
     * enable register dumping.
     */
    void toggleRegisterDump(bool enable);

    /**
     * get register writes.
     */
    std::vector<DivRegWrite>& getRegisterWrites();

    /**
     * initialize this DivDispatch.
     * @param parent the parent DivEngine.
     * @param channels the number of channels to acquire.
     * @param sugRate the suggested rate. this may change, so don't rely on it.
     * @return the number of channels allocated.
     */
    virtual int init(DivEngine* parent, int channels, int sugRate, bool pal);

    /**
     * quit the DivDispatch.
     */
     virtual void quit();

     virtual ~DivDispatch();
};
#endif
