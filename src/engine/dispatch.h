#ifndef _DISPATCH_H
#define _DISPATCH_H

#define ONE_SEMITONE 2200

enum DivDispatchCmds {
  DIV_CMD_NOTE_ON=0,
  DIV_CMD_NOTE_OFF,
  DIV_CMD_INSTRUMENT,
  DIV_CMD_VOLUME,
  DIV_CMD_NOTE_PORTA,
  DIV_CMD_PITCH,
  DIV_CMD_PANNING,
  DIV_CMD_LEGATO,

  DIV_CMD_SAMPLE_MODE,

  DIV_CMD_FM_TL,
  DIV_CMD_FM_AR,
  DIV_CMD_FM_FB,
  DIV_CMD_FM_MULT,
  DIV_CMD_FM_EXTCH,

  DIV_CMD_GENESIS_LFO,
  
  DIV_CMD_ARCADE_LFO,

  DIV_CMD_STD_NOISE_FREQ,
  DIV_CMD_STD_NOISE_MODE
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
    virtual void acquire(int& l, int& r);
    virtual int dispatch(DivCommand c);
    virtual void tick();

    /**
     * initialize this DivDispatch.
     * @param parent the parent DivEngine.
     * @param channels the number of channels to acquire.
     * @param sugRate the suggested rate. this may change, so don't rely on it.
     * @return the number of channels allocated.
     */
    virtual int init(DivEngine* parent, int channels, int sugRate);
};
#endif
