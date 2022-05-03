#ifndef _NES_FDS_H_
#define _NES_FDS_H_

namespace xgm {

class NES_FDS
{
public:
    enum
    {
        OPT_CUTOFF=2000,
        OPT_4085_RESET,
        OPT_WRITE_PROTECT,
        OPT_END
    };

protected:
    double rate, clock;
    int mask;
    int sm[2]; // stereo mix
    int fout; // current output
    int option[OPT_END];

    bool master_io;
    unsigned int master_vol;
    unsigned int last_freq; // for trackinfo
    unsigned int last_vol;  // for trackinfo

    // two wavetables
    enum { TMOD=0, TWAV=1 };
    int wave[2][64];
    unsigned int freq[2];
    unsigned int phase[2];
    bool wav_write;
    bool wav_halt;
    bool env_halt;
    bool mod_halt;
    unsigned int mod_pos;
    unsigned int mod_write_pos;

    // two ramp envelopes
    enum { EMOD=0, EVOL=1 };
    bool env_mode[2];
    bool env_disable[2];
    unsigned int env_timer[2];
    unsigned int env_speed[2];
    unsigned int env_out[2];
    unsigned int master_env_speed;

    // 1-pole RC lowpass filter
    int rc_accum;
    int rc_k;
    int rc_l;

public:
    NES_FDS ();
    ~NES_FDS ();

    void Reset ();
    void Tick (unsigned int clocks);
    unsigned int Render (int b[2]);
    bool Write (unsigned int adr, unsigned int val, unsigned int id=0);
    bool Read (unsigned int adr, unsigned int & val, unsigned int id=0);
    void SetRate (double);
    void SetClock (double);
    void SetOption (int, int);
    void SetMask(int m){ mask = m&1; }
    void SetStereoMix (int trk, short mixl, short mixr);
};

} // namespace xgm

#endif
