#ifndef _NES_DMC_H_
#define _NES_DMC_H_

#include "../device.h"
#include "../Audio/MedianFilter.h"
#include "../CPU/nes_cpu.h"

namespace xgm
{
  class NES_APU; // forward declaration

  /** Bottom Half of APU **/
  class NES_DMC:public ISoundChip
  {
  public:
    enum
    { 
      OPT_ENABLE_4011=0,
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
    static const UINT32 freq_table[2][16];
    static const UINT32 wavlen_table[2][16];
    UINT32 tnd_table[2][16][16][128];

    int option[OPT_END];
    int mask;
    INT32 sm[2][3];
    UINT8 reg[0x10];
    UINT32 len_reg;
    UINT32 adr_reg;
    IDevice *memory;
    UINT32 out[3];
    UINT32 daddress;
    UINT32 dlength;
    UINT32 data;
    bool empty;
    INT16 damp;
    int dac_lsb;
    bool dmc_pop;
    INT32 dmc_pop_offset;
    INT32 dmc_pop_follow;
    double clock;
    UINT32 rate;
    int pal;
    int mode;
    bool irq;

    INT32 counter[3];  // frequency dividers
    int tphase;        // triangle phase
    UINT32 nfreq;      // noise frequency
    UINT32 dfreq;      // DPCM frequency

    UINT32 tri_freq;
    int linear_counter;
    int linear_counter_reload;
    bool linear_counter_halt;
    bool linear_counter_control;

    int noise_volume;
    UINT32 noise, noise_tap;

    // noise envelope
    bool envelope_loop;
    bool envelope_disable;
    bool envelope_write;
    int envelope_div_period;
    int envelope_div;
    int envelope_counter;

    bool enable[2]; // tri/noise enable
    int length_counter[2]; // 0=tri, 1=noise

    TrackInfoBasic trkinfo[3];

    // frame sequencer
    NES_APU* apu; // apu is clocked by DMC's frame sequencer
    int frame_sequence_count;  // current cycle count
    int frame_sequence_length; // CPU cycles per FrameSequence
    int frame_sequence_step;   // current step of frame sequence
    int frame_sequence_steps;  // 4/5 steps per frame
    bool frame_irq;
    bool frame_irq_enable;

    NES_CPU* cpu; // IRQ needs CPU access

    inline UINT32 calc_tri (UINT32 clocks);
    inline UINT32 calc_dmc (UINT32 clocks);
    inline UINT32 calc_noise (UINT32 clocks);

  public:
      NES_DMC ();
     ‾NES_DMC ();

    void InitializeTNDTable(double wt, double wn, double wd);
    void SetPal (bool is_pal);
    void SetAPU (NES_APU* apu_);
    void SetMemory (IDevice * r);
    void FrameSequence(int s);
    int GetDamp(){ return (damp<<1)|dac_lsb ; }
    void TickFrameSequence (UINT32 clocks);

    virtual void Reset ();
    virtual void Tick (UINT32 clocks);
    virtual UINT32 Render (INT32 b[2]);
    virtual bool Write (UINT32 adr, UINT32 val, UINT32 id=0);
    virtual bool Read (UINT32 adr, UINT32 & val, UINT32 id=0);
    virtual void SetRate (double rate);
    virtual void SetClock (double rate);
    virtual void SetOption (int, int);
    virtual void SetMask(int m){ mask = m; }
    virtual void SetStereoMix (int trk, xgm::INT16 mixl, xgm::INT16 mixr);
    virtual ITrackInfo *GetTrackInfo(int trk);

    void SetCPU(NES_CPU* cpu_);
  };

}

#endif
