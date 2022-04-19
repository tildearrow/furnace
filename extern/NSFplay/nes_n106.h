#ifndef _NES_N106_H_
#define _NES_N106_H_
#include "../device.h"

namespace xgm {

class TrackInfoN106 : public TrackInfoBasic
{
public:
    int wavelen;
    INT16 wave[256];
    virtual IDeviceInfo *Clone(){ return new TrackInfoN106(*this); }
};

class NES_N106:public ISoundChip
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
    INT32 sm[2][8]; // stereo mix
    INT32 fout[8]; // current output
    TrackInfoN106 trkinfo[8];
    int option[OPT_END];

    bool master_disable;
    UINT32 reg[0x80]; // all state is contained here
    unsigned int reg_select;
    bool reg_advance;
    int tick_channel;
    int tick_clock;
    int render_channel;
    int render_clock;
    int render_subclock;

    // convenience functions to interact with regs
    inline UINT32 get_phase (int channel);
    inline UINT32 get_freq (int channel);
    inline UINT32 get_off (int channel);
    inline UINT32 get_len (int channel);
    inline INT32  get_vol (int channel);
    inline INT32  get_sample (UINT32 index);
    inline int    get_channels ();
    // for storing back the phase after modifying
    inline void   set_phase (UINT32 phase, int channel);

public:
    NES_N106 ();
    ~NES_N106 ();

    virtual void Reset ();
    virtual void Tick (UINT32 clocks);
    virtual UINT32 Render (INT32 b[2]);
    virtual bool Write (UINT32 adr, UINT32 val, UINT32 id=0);
    virtual bool Read (UINT32 adr, UINT32 & val, UINT32 id=0);
    virtual void SetRate (double);
    virtual void SetClock (double);
    virtual void SetOption (int, int);
    virtual void SetMask (int m);
    virtual void SetStereoMix (int trk, INT16 mixl, INT16 mixr);
    virtual ITrackInfo *GetTrackInfo(int trk);
};

} // namespace xgm

#endif
