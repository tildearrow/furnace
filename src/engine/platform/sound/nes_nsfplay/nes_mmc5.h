#ifndef _NES_MMC5_H_
#define _NES_MMC5_H_

namespace xgm
{
  class NES_MMC5
  {
  public:
    enum
    { OPT_NONLINEAR_MIXER=0, OPT_PHASE_REFRESH, OPT_END };

  protected:
    int option[OPT_END];
    int mask;
    int sm[2][3]; // stereo panning
    unsigned char ram[0x6000 - 0x5c00];
    unsigned char reg[8];
    unsigned char mreg[2];
    unsigned char pcm; // PCM channel
    bool pcm_mode; // PCM channel

    unsigned int scounter[2];            // frequency divider
    unsigned int sphase[2];              // phase counter

    unsigned int duty[2];
    unsigned int volume[2];
    unsigned int freq[2];
    int out[3];
    bool enable[2];

    bool envelope_disable[2];   // エンベロープ有効フラグ
    bool envelope_loop[2];      // エンベロープループ
    bool envelope_write[2];
    int envelope_div_period[2];
    int envelope_div[2];
    int envelope_counter[2];

    int length_counter[2];

    int frame_sequence_count;

    double clock, rate;
    int calc_sqr (int i, unsigned int clocks);
    int square_table[32];
    int pcm_table[256];
  public:
      NES_MMC5 ();
     ~NES_MMC5 ();

    void FrameSequence ();
    void TickFrameSequence (unsigned int clocks);

    void Reset ();
    void Tick (unsigned int clocks);
    unsigned int Render (int b[2]);
    bool Write (unsigned int adr, unsigned int val, unsigned int id=0);
    bool Read (unsigned int adr, unsigned int & val, unsigned int id=0);
    void SetOption (int id, int b);
    void SetClock (double);
    void SetRate (double);
    void SetMask (int m){ mask = m; }
    void SetStereoMix (int trk, short mixl, short mixr);
  };

}

#endif
