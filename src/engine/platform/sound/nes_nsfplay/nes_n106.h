#ifndef _NES_N106_H_
#define _NES_N106_H_

namespace xgm {


class NES_N106
{
public:
    enum
    {
        OPT_SERIAL = 0,
        OPT_PHASE_READ_ONLY = 1,
        OPT_LIMIT_WAVELENGTH = 2,
        OPT_END
    };

protected:
    double rate, clock;
    int mask;
    int sm[2][8]; // stereo mix
    int fout[8]; // current output
    int option[OPT_END];

    bool master_disable;
    unsigned int reg[0x80]; // all state is contained here
    unsigned int reg_select;
    bool reg_advance;
    int tick_channel;
    int tick_clock;
    int render_channel;
    int render_clock;
    int render_subclock;

    // convenience functions to interact with regs
    inline unsigned int get_phase (int channel);
    inline unsigned int get_freq (int channel);
    inline unsigned int get_off (int channel);
    inline unsigned int get_len (int channel);
    inline int  get_vol (int channel);
    inline int  get_sample (unsigned int index);
    inline int    get_channels ();
    // for storing back the phase after modifying
    inline void   set_phase (unsigned int phase, int channel);

public:
    NES_N106 ();
    ~NES_N106 ();

    virtual void Reset ();
    virtual void Tick (unsigned int clocks);
    virtual unsigned int Render (int b[2]);
    virtual bool Write (unsigned int adr, unsigned int val, unsigned int id=0);
    virtual bool Read (unsigned int adr, unsigned int & val, unsigned int id=0);
    virtual void SetRate (double);
    virtual void SetClock (double);
    virtual void SetOption (int, int);
    virtual void SetMask (int m);
    virtual void SetStereoMix (int trk, short mixl, short mixr);
};

} // namespace xgm

#endif
