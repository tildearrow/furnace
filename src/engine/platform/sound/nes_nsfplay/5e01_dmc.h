#ifndef _I5E01_DMC_H_
#define _I5E01_DMC_H_

#include <functional>

namespace xgm
{
  class I5E01_APU; // forward declaration

  /** Bottom Half of APU **/
  class I5E01_DMC
  {
  public:
    enum
    {
      OPT_ENABLE_4011 = 0,
      OPT_ENABLE_PNOISE,
      OPT_UNMUTE_ON_RESET,
      OPT_DPCM_ANTI_CLICK,
      OPT_NONLINEAR_MIXER,
      OPT_RANDOMIZE_NOISE,
      OPT_TRI_MUTE,
      OPT_RANDOMIZE_TRI,
      OPT_DPCM_REVERSE,
      OPT_END
    };
  protected:
    const int GETA_BITS;

    // DPCM.
    static const unsigned int freq_table[2][16];

    // Noise.
    static const unsigned int wavlen_table[2][32];

    unsigned int tnd_table[2][16][16][128];

    int option[OPT_END];
    int mask;
    int sm[2][3];
    unsigned char reg[0x10];
    unsigned int len_reg;
    unsigned int adr_reg;
    std::function<void(unsigned short, unsigned int&)> memory;
    unsigned int daddress;
    unsigned int dlength;
    unsigned int data;
    bool empty;
    short damp;
    int dac_lsb;
    bool dmc_pop;
    int dmc_pop_offset;
    int dmc_pop_follow;
    double clock;
    unsigned int rate;
    int pal;
    int mode;
    bool irq;

    int counter[3];  // frequency dividers
    int tphase;        // triangle phase
    int tduty;         // wave shape
    unsigned int nfreq;      // noise frequency
    unsigned int dfreq;      // DPCM frequency

    unsigned int tri_freq;
    int linear_counter;
    int linear_counter_reload;
    bool linear_counter_halt;
    bool linear_counter_control;

    int noise_volume;
    unsigned int noise, noise_tap;

    // noise envelope
    bool envelope_loop;
    bool envelope_disable;
    bool envelope_write;
    int envelope_div_period;
    int envelope_div;
    int envelope_counter;

    bool enable[2]; // tri/noise enable
    int length_counter[2]; // 0=tri, 1=noise


    // frame sequencer
    I5E01_APU* apu; // apu is clocked by DMC's frame sequencer
    int frame_sequence_count;  // current cycle count
    int frame_sequence_length; // CPU cycles per FrameSequence
    int frame_sequence_step;   // current step of frame sequence
    int frame_sequence_steps;  // 4/5 steps per frame
    bool frame_irq;
    bool frame_irq_enable;

    inline unsigned int calc_tri(unsigned int clocks);
    inline unsigned int calc_dmc(unsigned int clocks);
    inline unsigned int calc_noise(unsigned int clocks);

  public:
    unsigned int out[3];
    I5E01_DMC();
    ~I5E01_DMC();

    void InitializeTNDTable(double wt, double wn, double wd);
    void SetPal(bool is_pal);
    void SetAPU(I5E01_APU* apu_);
    void SetMemory(std::function<void(unsigned short, unsigned int&)> r);
    void FrameSequence(int s);
    int GetDamp() { return (damp << 1) | dac_lsb; }
    void TickFrameSequence(unsigned int clocks);

    void Reset();
    void Tick(unsigned int clocks);
    unsigned int Render(int b[2]);
    bool Write(unsigned int adr, unsigned int val, unsigned int id = 0);
    bool Read(unsigned int adr, unsigned int& val, unsigned int id = 0);
    void SetRate(double rate);
    void SetClock(double rate);
    void SetOption(int, int);
    void SetMask(int m) { mask = m; }
    void SetStereoMix(int trk, short mixl, short mixr);
  };

}

#endif
