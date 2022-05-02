#ifndef _NES_APU_H_
#define _NES_APU_H_
#include "../device.h"
#include "nes_dmc.h"

namespace xgm
{
  /** Upper half of APU **/
  class NES_APU : public ISoundChip
  {
  public:
    enum
    {
        OPT_UNMUTE_ON_RESET=0,
        OPT_PHASE_REFRESH,
        OPT_NONLINEAR_MIXER,
        OPT_DUTY_SWAP,
        OPT_NEGATE_SWEEP_INIT,
        OPT_END };

    enum
    { SQR0_MASK = 1, SQR1_MASK = 2, };

  protected:
    int option[OPT_END];        // 各種オプション
    int mask;
    INT32 sm[2][2];

    UINT32 gclock;
    UINT8 reg[0x20];
    INT32 out[2];
    double rate, clock;

    INT32 square_table[32];     // nonlinear mixer
    INT32 square_linear;        // linear mix approximation

    int scounter[2];            // frequency divider
    int sphase[2];              // phase counter

    int duty[2];
    int volume[2];
    int freq[2];
    int sfreq[2];

    bool sweep_enable[2];
    bool sweep_mode[2];
    bool sweep_write[2];
    int sweep_div_period[2];
    int sweep_div[2];
    int sweep_amount[2];

    bool envelope_disable[2];
    bool envelope_loop[2];
    bool envelope_write[2];
    int envelope_div_period[2];
    int envelope_div[2];
    int envelope_counter[2];

    int length_counter[2];

    bool enable[2];

    void sweep_sqr (int ch); // calculates target sweep frequency
    INT32 calc_sqr (int ch, UINT32 clocks);
    TrackInfoBasic trkinfo[2];

  public:
      NES_APU ();
     ‾NES_APU ();

    void FrameSequence(int s);

    virtual void Reset ();
    virtual void Tick (UINT32 clocks);
    virtual UINT32 Render (INT32 b[2]);
    virtual bool Read (UINT32 adr, UINT32 & val, UINT32 id=0);
    virtual bool Write (UINT32 adr, UINT32 val, UINT32 id=0);
    virtual void SetRate (double rate);
    virtual void SetClock (double clock);
    virtual void SetOption (int id, int b);
    virtual void SetMask(int m){ mask = m; }
    virtual void SetStereoMix (int trk, xgm::INT16 mixl, xgm::INT16 mixr);
    virtual ITrackInfo *GetTrackInfo(int trk);
  };

}                               // namespace

#endif
