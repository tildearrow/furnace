enum DivDispatchCmds {
  DIV_CMD_NOTE_ON=0,
  DIV_CMD_NOTE_OFF,
  DIV_CMD_INSTRUMENT,
  DIV_CMD_VOLUME,
  DIV_CMD_PITCH_UP,
  DIV_CMD_PITCH_DOWN,
  DIV_CMD_PITCH_TO
};

struct DivCommand {
  DivDispatchCmds cmd;
};

struct DivDelayedCommand {
  int ticks;
  DivCommand cmd;
};

class DivDispatch {
  public:
    virtual void acquire(float& l, float& r);
    virtual int dispatch(DivCommand c);

    /**
     * initialize this DivDispatch.
     * @param channels the number of channels to acquire.
     * @return the number of channels allocated.
     */
    virtual int init(int channels);
};
