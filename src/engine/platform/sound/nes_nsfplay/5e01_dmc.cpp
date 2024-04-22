#include "5e01_dmc.h"
#include "5e01_apu.h"

#include "common.h"
#include <assert.h>
#include <cstdlib>

namespace xgm
{
  const unsigned int I5E01_DMC::wavlen_table[2][32] = {
  { // NTSC
    4,5,6,7,9,12,15,19,23,29,37,46,58,72,91,114,142,178,222,278,348,435,544,681,851,1064,1331,1664,2081,2602,3253,4068
  },
  { // PAL
    3,4,6,7,9,12,15,18,23,29,36,45,56,70,88,110,137,171,213,266,332,414,517,644,804,1003,1251,1560,1946,2428,3028,3778
  } };

  const unsigned int I5E01_DMC::freq_table[2][16] = {
  { // NTSC
    428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54
  },
  { // PAL
    398, 354, 316, 298, 276, 236, 210, 198, 176, 148, 132, 118,  98, 78, 66, 50
  } };

  const unsigned int BITREVERSE[256] = {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF,
  };

  I5E01_DMC::I5E01_DMC() : GETA_BITS(20)
  {
    SetClock(DEFAULT_CLOCK);
    SetRate(DEFAULT_RATE);
    SetPal(false);
    option[OPT_ENABLE_4011] = 1;
    option[OPT_ENABLE_PNOISE] = 1;
    option[OPT_UNMUTE_ON_RESET] = 1;
    option[OPT_DPCM_ANTI_CLICK] = 0;
    option[OPT_NONLINEAR_MIXER] = 1;
    option[OPT_RANDOMIZE_NOISE] = 1;
    option[OPT_RANDOMIZE_TRI] = 1;
    option[OPT_TRI_MUTE] = 1;
    option[OPT_DPCM_REVERSE] = 0;
    tnd_table[0][0][0][0] = 0;
    tnd_table[1][0][0][0] = 0;

    apu = NULL;
    frame_sequence_count = 0;
    frame_sequence_length = 7458;
    frame_sequence_steps = 4;

    for (int c = 0;c < 2;++c)
      for (int t = 0;t < 3;++t)
        sm[c][t] = 128;
  }


  I5E01_DMC::~I5E01_DMC()
  {
  }

  void I5E01_DMC::SetStereoMix(int trk, short mixl, short mixr)
  {
    if (trk < 0) return;
    if (trk > 2) return;
    sm[0][trk] = mixl;
    sm[1][trk] = mixr;
  }

  void I5E01_DMC::FrameSequence(int s)
  {
    //DEBUG_OUT("FrameSequence: %d\n",s);

    if (s > 3) return; // no operation in step 4

    if (apu)
    {
      apu->FrameSequence(s);
    }

    if (s == 0 && (frame_sequence_steps == 4))
    {
      if (frame_irq_enable) frame_irq = true;
    }

    // 240hz clock
    {
      // triangle linear counter
      if (linear_counter_halt)
      {
        linear_counter = linear_counter_reload;
      }
      else
      {
        if (linear_counter > 0) --linear_counter;
      }
      if (!linear_counter_control)
      {
        linear_counter_halt = false;
      }

      // noise envelope
      bool divider = false;
      if (envelope_write)
      {
        envelope_write = false;
        envelope_counter = 15;
        envelope_div = 0;
      }
      else
      {
        ++envelope_div;
        if (envelope_div > envelope_div_period)
        {
          divider = true;
          envelope_div = 0;
        }
      }
      if (divider)
      {
        if (envelope_loop && envelope_counter == 0)
          envelope_counter = 15;
        else if (envelope_counter > 0)
          --envelope_counter;
      }
    }

    // 120hz clock
    if ((s & 1) == 0)
    {
      // triangle length counter
      if (!linear_counter_control && (length_counter[0] > 0))
        --length_counter[0];

      // noise length counter
      if (!envelope_loop && (length_counter[1] > 0))
        --length_counter[1];
    }

  }

  // 三角波チャンネルの計算 戻り値は0-15
  unsigned int I5E01_DMC::calc_tri(unsigned int clocks)
  {
    static unsigned int wavtbl[4][32] =
    {
      {
       15,14,13,12,11,10, 9, 8,
        7, 6, 5, 4, 3, 2, 1, 0,
        0, 1, 2, 3, 4, 5, 6, 7,
        8, 9,10,11,12,13,14,15,
      },
      {
       11,11,11,11,10,10,10,10,
        9, 9, 9, 9, 8, 8, 8, 8,
        7, 7, 7, 7, 6, 6, 6, 6,
        5, 5, 5, 5, 4, 4, 4, 4,
      },
      {
       11,11,11,11,11,11,11,11,
       11,11,11,11,11,11,11,11,
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4,
      },
      {
        8, 9,10,12,13,14,14,15,
       15,15,14,14,13,12,10, 9,
        8, 6, 5, 3, 2, 1, 1, 0,
        0, 0, 1, 1, 2, 3, 5, 6,
      },
    };

    if (linear_counter > 0 && length_counter[0] > 0
      && (!option[OPT_TRI_MUTE] || tri_freq > 0))
    {
      counter[0] -= clocks;
      while (counter[0] < 0)
      {
        tphase = (tphase + 1) & 31;
        counter[0] += (tri_freq + 1);
      }
    }

    unsigned int ret = wavtbl[tduty][tphase];
    return ret;
  }

  // ノイズチャンネルの計算 戻り値は0-127
  // 低サンプリングレートで合成するとエイリアスノイズが激しいので
  // ノイズだけはこの関数内で高クロック合成し、簡易なサンプリングレート
  // 変換を行っている。
  unsigned int I5E01_DMC::calc_noise(unsigned int clocks)
  {
    unsigned int env = envelope_disable ? noise_volume : envelope_counter;
    if (length_counter[1] < 1) env = 0;

    unsigned int last = (noise & 0x4000) ? 0 : env;
    if (clocks < 1) return last;

    // simple anti-aliasing (noise requires it, even when oversampling is off)
    unsigned int count = 0;
    unsigned int accum = counter[1] * last; // samples pending from previous calc
    unsigned int accum_clocks = counter[1];
#ifdef _DEBUG
    int start_clocks = counter[1];
#endif
    if (counter[1] < 0) // only happens on startup when using the randomize noise option
    {
      accum = 0;
      accum_clocks = 0;
    }

    counter[1] -= clocks;
    assert(nfreq > 0); // prevent infinite loop
    while (counter[1] < 0)
    {
      // tick the noise generator
      unsigned int feedback = (noise & 1) ^ ((noise & noise_tap) ? 1 : 0);
      noise = (noise >> 1) | (feedback << 14);

      last = (noise & 0x4000) ? 0 : env;
      accum += (last * nfreq);
      counter[1] += nfreq;
      ++count;
      accum_clocks += nfreq;
    }

    if (count < 1) // no change over interval, don't anti-alias
    {
      return last;
    }

    accum -= (last * counter[1]); // remove these samples which belong in the next calc
    accum_clocks -= counter[1];
#ifdef _DEBUG
    if (start_clocks >= 0) assert(accum_clocks == clocks); // these should be equal
#endif

    unsigned int average = accum / accum_clocks;
    assert(average <= 15); // above this would indicate overflow
    return average;
  }

  // Tick the DMC for the number of clocks, and return output counter;
  unsigned int I5E01_DMC::calc_dmc(unsigned int clocks)
  {
    counter[2] -= clocks;
    assert(dfreq > 0); // prevent infinite loop
    while (counter[2] < 0)
    {
      counter[2] += dfreq;

      if (data > 0x100) // data = 0x100 when shift register is empty
      {
        if (!empty)
        {
          if ((data & 1) && (damp < 63))
            damp++;
          else if (!(data & 1) && (0 < damp))
            damp--;
        }
        data >>= 1;
      }

      if (data <= 0x100) // shift register is empty
      {
        if (dlength > 0)
        {
          memory(daddress, data);
          // (checking for the 3-cycle case would require sub-instruction emulation)
          data &= 0xFF; // read 8 bits
          if (option[OPT_DPCM_REVERSE]) data = BITREVERSE[data];
          data |= 0x10000; // use an extra bit to signal end of data
          empty = false;
          daddress = ((daddress + 1) & 0xFFFF) | 0x8000;
          --dlength;
          if (dlength == 0)
          {
            if (mode & 1) // looped DPCM = auto-reload
            {
              daddress = ((adr_reg << 6) | 0xC000);
              dlength = (len_reg << 4) + 1;
            }
            else if (mode & 2) // IRQ and not looped
            {
              irq = true;
            }
          }
        }
        else
        {
          data = 0x10000; // DMC will do nothing
          empty = true;
        }
      }
    }

    return (damp << 1) + dac_lsb;
  }

  void I5E01_DMC::TickFrameSequence(unsigned int clocks)
  {
    frame_sequence_count += clocks;
    while (frame_sequence_count > frame_sequence_length)
    {
      FrameSequence(frame_sequence_step);
      frame_sequence_count -= frame_sequence_length;
      ++frame_sequence_step;
      if (frame_sequence_step >= frame_sequence_steps)
        frame_sequence_step = 0;
    }
  }

  void I5E01_DMC::Tick(unsigned int clocks)
  {
    out[0] = calc_tri(clocks);
    out[1] = calc_noise(clocks);
    out[2] = calc_dmc(clocks);
  }

  unsigned int I5E01_DMC::Render(int b[2])
  {
    out[0] = (mask & 1) ? 0 : out[0];
    out[1] = (mask & 2) ? 0 : out[1];
    out[2] = (mask & 4) ? 0 : out[2];

    int m[3];
    m[0] = tnd_table[0][out[0]][0][0];
    m[1] = tnd_table[0][0][out[1]][0];
    m[2] = tnd_table[0][0][0][out[2]];

    if (option[OPT_NONLINEAR_MIXER])
    {
      int ref = m[0] + m[1] + m[2];
      int voltage = tnd_table[1][out[0]][out[1]][out[2]];
      if (ref)
      {
        for (int i = 0; i < 3; ++i)
          m[i] = (m[i] * voltage) / ref;
      }
      else
      {
        for (int i = 0; i < 3; ++i)
          m[i] = voltage;
      }
    }

    // anti-click nullifies any 4011 write but preserves nonlinearity
    if (option[OPT_DPCM_ANTI_CLICK])
    {
      if (dmc_pop) // $4011 will cause pop this frame
      {
        // adjust offset to counteract pop
        dmc_pop_offset += dmc_pop_follow - m[2];
        dmc_pop = false;

        // prevent overflow, keep headspace at edges
        const int OFFSET_MAX = (1 << 30) - (4 << 16);
        if (dmc_pop_offset > OFFSET_MAX) dmc_pop_offset = OFFSET_MAX;
        if (dmc_pop_offset < -OFFSET_MAX) dmc_pop_offset = -OFFSET_MAX;
      }
      dmc_pop_follow = m[2]; // remember previous position

      m[2] += dmc_pop_offset; // apply offset

      // TODO implement this in a better way
      // roll off offset (not ideal, but prevents overflow)
      if (dmc_pop_offset > 0) --dmc_pop_offset;
      else if (dmc_pop_offset < 0) ++dmc_pop_offset;
    }

    b[0] = m[0] * sm[0][0];
    b[0] += m[1] * sm[0][1];
    b[0] += m[2] * sm[0][2];
    b[0] >>= 7;

    b[1] = m[0] * sm[1][0];
    b[1] += m[1] * sm[1][1];
    b[1] += m[2] * sm[1][2];
    b[1] >>= 7;

    return 2;
  }

  void I5E01_DMC::SetClock(double c)
  {
    clock = c;
  }

  void I5E01_DMC::SetRate(double r)
  {
    rate = (unsigned int)(r ? r : DEFAULT_RATE);
  }

  void I5E01_DMC::SetPal(bool is_pal)
  {
    pal = (is_pal ? 1 : 0);
    // set CPU cycles in frame_sequence
    frame_sequence_length = is_pal ? 8314 : 7458;
  }

  void I5E01_DMC::SetAPU(I5E01_APU* apu_)
  {
    apu = apu_;
  }

  // Initializing TRI, NOISE, DPCM mixing table
  void I5E01_DMC::InitializeTNDTable(double wt, double wn, double wd) {

    // volume adjusted by 0.95 based on empirical measurements
    const double MASTER = 8192.0 * 0.95;
    // truthfully, the nonlinear curve does not appear to match well
    // with my tests. Do more testing of the APU/DMC DAC later.
    // this value keeps the triangle consistent with measured levels,
    // but not necessarily the rest of this APU channel,
    // because of the lack of a good DAC model, currently.

    { // Linear Mixer
      for (int t = 0; t < 16; t++) {
        for (int n = 0; n < 16; n++) {
          for (int d = 0; d < 128; d++) {
            tnd_table[0][t][n][d] = (unsigned int)(MASTER * (3.0 * t + 2.0 * n + d) / 208.0);
          }
        }
      }
    }
    { // Non-Linear Mixer
      tnd_table[1][0][0][0] = 0;
      for (int t = 0; t < 16; t++) {
        for (int n = 0; n < 16; n++) {
          for (int d = 0; d < 128; d++) {
            if (t != 0 || n != 0 || d != 0)
              tnd_table[1][t][n][d] = (unsigned int)((MASTER * 159.79) / (100.0 + 1.0 / ((double)t / wt + (double)n / wn + (double)d / wd)));
          }
        }
      }
    }

  }

  void I5E01_DMC::Reset()
  {
    int i;
    mask = 0;

    InitializeTNDTable(8227, 12241, 22638);

    counter[0] = 0;
    counter[1] = 0;
    counter[2] = 0;
    tphase = 0;
    tduty = 0;
    nfreq = wavlen_table[0][0];
    dfreq = freq_table[0][0];
    tri_freq = 0;
    linear_counter = 0;
    linear_counter_reload = 0;
    linear_counter_halt = 0;
    linear_counter_control = 0;
    noise_volume = 0;
    noise = 0;
    noise_tap = 0;
    envelope_loop = 0;
    envelope_disable = 0;
    envelope_write = 0;
    envelope_div_period = 0;
    envelope_div = 0;
    envelope_counter = 0;
    enable[0] = 0;
    enable[1] = 0;
    length_counter[0] = 0;
    length_counter[1] = 0;
    frame_irq = false;
    frame_irq_enable = false;
    frame_sequence_count = 0;
    frame_sequence_steps = 4;
    frame_sequence_step = 0;

    for (i = 0; i < 0x0F; i++)
      Write(0x4108 + i, 0);
    Write(0x4117, 0x40);

    irq = false;
    Write(0x4115, 0x00);
    if (option[OPT_UNMUTE_ON_RESET])
      Write(0x4115, 0x0f);

    out[0] = out[1] = out[2] = 0;
    damp = 0;
    dmc_pop = false;
    dmc_pop_offset = 0;
    dmc_pop_follow = 0;
    dac_lsb = 0;
    data = 0x100;
    empty = true;
    adr_reg = 0;
    dlength = 0;
    len_reg = 0;
    daddress = 0;
    noise = 1;
    noise_tap = (1 << 1);

    SetRate(rate);
  }

  void I5E01_DMC::SetMemory(std::function<void(unsigned short, unsigned int&)> r)
  {
    memory = r;
  }

  void I5E01_DMC::SetOption(int id, int val)
  {
    if (id < OPT_END)
    {
      option[id] = val;
      if (id == OPT_NONLINEAR_MIXER)
        InitializeTNDTable(8227, 12241, 22638);
    }
  }

  bool I5E01_DMC::Write(unsigned int adr, unsigned int val, unsigned int id)
  {
    static const unsigned char length_table[32] = {
        0x0A, 0xFE,
        0x14, 0x02,
        0x28, 0x04,
        0x50, 0x06,
        0xA0, 0x08,
        0x3C, 0x0A,
        0x0E, 0x0C,
        0x1A, 0x0E,
        0x0C, 0x10,
        0x18, 0x12,
        0x30, 0x14,
        0x60, 0x16,
        0xC0, 0x18,
        0x48, 0x1A,
        0x10, 0x1C,
        0x20, 0x1E
    };

    // hack
    adr+=0x100;

    if (adr == 0x4115)
    {
      enable[0] = (val & 4) ? true : false;
      enable[1] = (val & 8) ? true : false;

      if (!enable[0])
      {
        length_counter[0] = 0;
      }
      if (!enable[1])
      {
        length_counter[1] = 0;
      }

      if ((val & 16) && dlength == 0)
      {
        daddress = (0xC000 | (adr_reg << 6));
        dlength = (len_reg << 4) + 1;
      }
      else if (!(val & 16))
      {
        dlength = 0;
      }

      irq = false;

      reg[adr - 0x4108] = val;
      return true;
    }

    if (adr == 0x4117)
    {
      //DEBUG_OUT("4017 = %02X\n", val);
      frame_irq_enable = ((val & 0x40) != 0x40);
      if (frame_irq_enable) frame_irq = false;

      frame_sequence_count = 0;
      if (val & 0x80)
      {
        frame_sequence_steps = 5;
        frame_sequence_step = 0;
        FrameSequence(frame_sequence_step);
        ++frame_sequence_step;
      }
      else
      {
        frame_sequence_steps = 4;
        frame_sequence_step = 1;
      }
    }

    if (adr < 0x4108 || 0x4113 < adr)
      return false;

    reg[adr - 0x4108] = val & 0xff;

    //DEBUG_OUT("$%04X %02X\n", adr, val);

    switch (adr)
    {

      // tri

    case 0x4108:
      linear_counter_control = (val >> 7) & 1;
      linear_counter_reload = val & 0x7F;
      break;

    case 0x4109:
      tduty = val & 3;
      break;

    case 0x410a:
      tri_freq = val | (tri_freq & 0x700);
      break;

    case 0x410b:
      tri_freq = (tri_freq & 0xff) | ((val & 0x7) << 8);
      linear_counter_halt = true;
      if (enable[0])
      {
        length_counter[0] = length_table[(val >> 3) & 0x1f];
      }
      break;

      // noise

    case 0x410c:
      noise_volume = val & 15;
      envelope_div_period = val & 15;
      envelope_disable = (val >> 4) & 1;
      envelope_loop = (val >> 5) & 1;
      break;

    case 0x410d:
      break;

    case 0x410e:
      if (option[OPT_ENABLE_PNOISE])
        noise_tap = (val & 0x80) ? (1 << 6) : (1 << 1);
      else
        noise_tap = (1 << 1);
      nfreq = wavlen_table[pal][val & 31];
      break;

    case 0x410f:
      if (enable[1])
      {
        length_counter[1] = length_table[(val >> 3) & 0xf];
      }
      envelope_write = true;
      break;

      // dmc

    case 0x4110:
      mode = (val >> 6) & 3;
      if (!(mode & 2))
      {
        irq = false;
      }
      dfreq = freq_table[pal][val & 15];
      break;

    case 0x4111:
      if (option[OPT_ENABLE_4011])
      {
        damp = (val >> 1) & 0x3f;
        dac_lsb = val & 1;
        dmc_pop = true;
      }
      break;

    case 0x4112:
      adr_reg = val & 0xff;
      // ここでdaddressは更新されない
      break;

    case 0x4113:
      len_reg = val & 0xff;
      // ここでlengthは更新されない
      break;

    default:
      return false;
    }

    return true;
  }

  bool I5E01_DMC::Read(unsigned int adr, unsigned int& val, unsigned int id)
  {
    if (adr == 0x4115)
    {
      val |= (irq ? 0x80 : 0)
        | (frame_irq ? 0x40 : 0)
        | ((dlength > 0) ? 0x10 : 0)
        | (length_counter[1] ? 0x08 : 0)
        | (length_counter[0] ? 0x04 : 0)
        ;

      frame_irq = false;
      return true;
    }
    else if (0x4108 <= adr && adr <= 0x4114)
    {
      val |= reg[adr - 0x4108];
      return true;
    }
    else
      return false;
  }
} // namespace
