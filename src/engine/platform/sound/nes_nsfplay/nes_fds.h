#ifndef _NES_FDS_H_
#define _NES_FDS_H_
#include "../device.h"

namespace xgm {

class TrackInfoFDS : public TrackInfoBasic
{
public:
    INT16 wave[64];
    virtual IDeviceInfo *Clone(){ return new TrackInfoFDS(*this); }
};

class NES_FDS : public ISoundChip
{
public:
    enum
    {
        OPT_CUTOFF=0,
        OPT_4085_RESET,
        OPT_WRITE_PROTECT,
        OPT_END
    };

protected:
    double rate, clock;
    int mask;
    INT32 sm[2]; // stereo mix
    INT32 fout; // current output
    TrackInfoFDS trkinfo;
    int option[OPT_END];

    bool master_io;
    UINT32 master_vol;
    UINT32 last_freq; // for trackinfo
    UINT32 last_vol;  // for trackinfo

    // two wavetables
    enum { TMOD=0, TWAV=1 };
    INT32 wave[2][64];
    UINT32 freq[2];
    UINT32 phase[2];
    bool wav_write;
    bool wav_halt;
    bool env_halt;
    bool mod_halt;
    UINT32 mod_pos;
    UINT32 mod_write_pos;

    // two ramp envelopes
    enum { EMOD=0, EVOL=1 };
    bool env_mode[2];
    bool env_disable[2];
    UINT32 env_timer[2];
    UINT32 env_speed[2];
    UINT32 env_out[2];
    UINT32 master_env_speed;

    // 1-pole RC lowpass filter
    INT32 rc_accum;
    INT32 rc_k;
    INT32 rc_l;

public:
    NES_FDS ();
    virtual ‾ NES_FDS ();

    virtual void Reset ();
    virtual void Tick (UINT32 clocks);
    virtual UINT32 Render (INT32 b[2]);
    virtual bool Write (UINT32 adr, UINT32 val, UINT32 id=0);
    virtual bool Read (UINT32 adr, UINT32 & val, UINT32 id=0);
    virtual void SetRate (double);
    virtual void SetClock (double);
    virtual void SetOption (int, int);
    virtual void SetMask(int m){ mask = m&1; }
    virtual void SetStereoMix (int trk, INT16 mixl, INT16 mixr);
    virtual ITrackInfo *GetTrackInfo(int trk);
};

} // namespace xgm

#endif
