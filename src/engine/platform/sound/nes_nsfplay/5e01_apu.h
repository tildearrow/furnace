#ifndef _I5E01_APU_H_
#define _I5E01_APU_H_
#include "5e01_dmc.h"

namespace xgm
{
  /** Upper half of APU **/
  class I5E01_APU
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

  public:
    int option[OPT_END];        // 各種オプション
    int mask;
    int sm[2][2];

    unsigned int gclock;
    unsigned char reg[0x20];
    double rate, clock;

    int square_table[32];     // nonlinear mixer
    int square_linear;        // linear mix approximation

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
    int calc_sqr (int ch, unsigned int clocks);

  public:
      int out[2];
      I5E01_APU ();
     ~I5E01_APU ();

    // // !! fetch frequency directly instead of through GetTrackInfo()
    double GetFrequencyPulse1() const;
    double GetFrequencyPulse2() const;

    void FrameSequence(int s);

    void Reset ();
    void Tick (unsigned int clocks);
    unsigned int Render (int b[2]);
    bool Read (unsigned int adr, unsigned int & val, unsigned int id=0);
    bool Write (unsigned int adr, unsigned int val, unsigned int id=0);
    void SetRate (double rate);
    void SetClock (double clock);
    void SetOption (int id, int b);
    void SetMask(int m){ mask = m; }
    void SetStereoMix (int trk, short mixl, short mixr);
  };

}                               // namespace

#endif
