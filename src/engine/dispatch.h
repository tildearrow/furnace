#ifndef _DISPATCH_H
#define _DISPATCH_H

#include <stdlib.h>

#define ONE_SEMITONE 2200

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

  DIV_ALWAYS_SET_VOLUME,

  DIV_CMD_MAX
};

struct DivCommand {
  DivDispatchCmds cmd;
  unsigned char chan;
  int value, value2;
  DivCommand(DivDispatchCmds c, unsigned char ch, int val, int val2):
    cmd(c),
    chan(ch),
    value(val),
    value2(val2) {}
  DivCommand(DivDispatchCmds c, unsigned char ch, int val):
    cmd(c),
    chan(ch),
    value(val),
    value2(0) {}
  DivCommand(DivDispatchCmds c, unsigned char ch):
    cmd(c),
    chan(ch),
    value(0),
    value2(0) {}
};

struct DivDelayedCommand {
  int ticks;
  DivCommand cmd;
};

class DivEngine;

class DivDispatch {
  protected:
    DivEngine* parent;
  public:
    /**
     * the rate the samples are provided.
     * the engine shall resample to the output rate.
     */
    int rate;
    virtual void acquire(short* bufL, short* bufR, size_t start, size_t len);
    virtual int dispatch(DivCommand c);
    virtual void tick();

    virtual bool isStereo();
    virtual bool keyOffAffectsArp(int ch);

    /**
     * initialize this DivDispatch.
     * @param parent the parent DivEngine.
     * @param channels the number of channels to acquire.
     * @param sugRate the suggested rate. this may change, so don't rely on it.
     * @return the number of channels allocated.
     */
    virtual int init(DivEngine* parent, int channels, int sugRate, bool pal);
};
#endif
