//
// I5E01 2A03
//
#include <assert.h>
#include "5e01_apu.h"
#include "common.h"

namespace xgm
{
  void I5E01_APU::sweep_sqr(int i)
  {
    int shifted = freq[i] >> sweep_amount[i];
    if (i == 0 && sweep_mode[i]) shifted += 1;
    sfreq[i] = freq[i] + (sweep_mode[i] ? -shifted : shifted);
    //DEBUG_OUT("shifted[%d] = %d (%d >> %d)\n",i,shifted,freq[i],sweep_amount[i]);
  }

  void I5E01_APU::FrameSequence(int s)
  {
    //DEBUG_OUT("FrameSequence(%d)\n",s);

    if (s > 3) return; // no operation in step 4

    // 240hz clock
    for (int i = 0; i < 2; ++i)
    {
      bool divider = false;
      if (envelope_write[i])
      {
        envelope_write[i] = false;
        envelope_counter[i] = 15;
        envelope_div[i] = 0;
      }
      else
      {
        ++envelope_div[i];
        if (envelope_div[i] > envelope_div_period[i])
        {
          divider = true;
          envelope_div[i] = 0;
        }
      }
      if (divider)
      {
        if (envelope_loop[i] && envelope_counter[i] == 0)
          envelope_counter[i] = 15;
        else if (envelope_counter[i] > 0)
          --envelope_counter[i];
      }
    }

    // 120hz clock
    if ((s & 1) == 0)
      for (int i = 0; i < 2; ++i)
      {
        if (!envelope_loop[i] && (length_counter[i] > 0))
          --length_counter[i];

        if (sweep_enable[i])
        {
          //DEBUG_OUT("Clock sweep: %d\n", i);

          --sweep_div[i];
          if (sweep_div[i] <= 0)
          {
            sweep_sqr(i); // calculate new sweep target

            //DEBUG_OUT("sweep_div[%d] (0/%d)\n",i,sweep_div_period[i]);
            //DEBUG_OUT("freq[%d]=%d > sfreq[%d]=%d\n",i,freq[i],i,sfreq[i]);

            if (freq[i] >= 8 && sfreq[i] < 0x800 && sweep_amount[i] > 0) // update frequency if appropriate
            {
              freq[i] = sfreq[i] < 0 ? 0 : sfreq[i];
            }
            sweep_div[i] = sweep_div_period[i] + 1;

            //DEBUG_OUT("freq[%d]=%d\n",i,freq[i]);
          }

          if (sweep_write[i])
          {
            sweep_div[i] = sweep_div_period[i] + 1;
            sweep_write[i] = false;
          }
        }
      }

  }

  int I5E01_APU::calc_sqr(int i, unsigned int clocks)
  {
    static const short sqrtbl[4][16] = {
      {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0}
    };

    scounter[i] -= clocks;
    while (scounter[i] < 0)
    {
      sphase[i] = (sphase[i] + 1) & 15;
      scounter[i] += freq[i] + 1;
    }

    int ret = 0;
    if (length_counter[i] > 0 &&
      freq[i] >= 8 &&
      sfreq[i] < 0x800
      )
    {
      int v = envelope_disable[i] ? volume[i] : envelope_counter[i];
      ret = sqrtbl[duty[i]][sphase[i]] ? v : 0;
    }

    return ret;
  }

  bool I5E01_APU::Read(unsigned int adr, unsigned int& val, unsigned int id)
  {
    if (0x4100 <= adr && adr < 0x4108)
    {
      val |= reg[adr & 0x7];
      return true;
    }
    else if (adr == 0x4115)
    {
      val |= (length_counter[1] ? 2 : 0) | (length_counter[0] ? 1 : 0);
      return true;
    }
    else
      return false;
  }

  void I5E01_APU::Tick(unsigned int clocks)
  {
    out[0] = calc_sqr(0, clocks);
    out[1] = calc_sqr(1, clocks);
  }

  // 生成される波形の振幅は0-8191
  unsigned int I5E01_APU::Render(int b[2])
  {
    out[0] = (mask & 1) ? 0 : out[0];
    out[1] = (mask & 2) ? 0 : out[1];

    int m[2];

    if (option[OPT_NONLINEAR_MIXER])
    {
      int voltage = square_table[out[0] + out[1]];
      m[0] = out[0] << 6;
      m[1] = out[1] << 6;
      int ref = m[0] + m[1];
      if (ref > 0)
      {
        m[0] = (m[0] * voltage) / ref;
        m[1] = (m[1] * voltage) / ref;
      }
      else
      {
        m[0] = voltage;
        m[1] = voltage;
      }
    }
    else
    {
      m[0] = (out[0] * square_linear) / 15;
      m[1] = (out[1] * square_linear) / 15;
    }

    b[0] = m[0] * sm[0][0];
    b[0] += m[1] * sm[0][1];
    b[0] >>= 7;

    b[1] = m[0] * sm[1][0];
    b[1] += m[1] * sm[1][1];
    b[1] >>= 7;

    return 2;
  }

  I5E01_APU::I5E01_APU()
  {
    SetClock(DEFAULT_CLOCK);
    SetRate(DEFAULT_RATE);
    option[OPT_UNMUTE_ON_RESET] = true;
    option[OPT_PHASE_REFRESH] = true;
    option[OPT_NONLINEAR_MIXER] = true;
    option[OPT_DUTY_SWAP] = false;
    option[OPT_NEGATE_SWEEP_INIT] = false;

    square_table[0] = 0;
    for (int i = 1;i < 32;i++)
      square_table[i] = (int)((8192.0 * 95.88) / (8128.0 / i + 100));

    square_linear = square_table[15]; // match linear scale to one full volume square of nonlinear

    for (int c = 0;c < 2;++c)
      for (int t = 0;t < 2;++t)
        sm[c][t] = 128;
  }

  I5E01_APU::~I5E01_APU()
  {
  }

  void I5E01_APU::Reset()
  {
    int i;
    gclock = 0;
    mask = 0;

    for (int i = 0; i < 2; ++i)
    {
      scounter[i] = 0;
      sphase[i] = 0;
      duty[i] = 0;
      volume[i] = 0;
      freq[i] = 0;
      sfreq[i] = 0;
      sweep_enable[i] = 0;
      sweep_mode[i] = 0;
      sweep_write[i] = 0;
      sweep_div_period[i] = 0;
      sweep_div[i] = 1;
      sweep_amount[i] = 0;
      envelope_disable[i] = 0;
      envelope_loop[i] = 0;
      envelope_write[i] = 0;
      envelope_div_period[i] = 0;
      envelope_div[0] = 0;
      envelope_counter[i] = 0;
      length_counter[i] = 0;
      enable[i] = 0;
    }

    for (i = 0x4100; i < 0x4108; i++)
      Write(i, 0);

    Write(0x4115, 0);
    if (option[OPT_UNMUTE_ON_RESET])
      Write(0x4115, 0x0f);
    if (option[OPT_NEGATE_SWEEP_INIT])
    {
      Write(0x4101, 0x08);
      Write(0x4105, 0x08);
    }

    for (i = 0; i < 2; i++)
      out[i] = 0;

    SetRate(rate);
  }

  void I5E01_APU::SetOption(int id, int val)
  {
    if (id < OPT_END) option[id] = val;
  }

  void I5E01_APU::SetClock(double c)
  {
    clock = c;
  }

  void I5E01_APU::SetRate(double r)
  {
    rate = r ? r : DEFAULT_RATE;
  }

  void I5E01_APU::SetStereoMix(int trk, short mixl, short mixr)
  {
    if (trk < 0) return;
    if (trk > 1) return;
    sm[0][trk] = mixl;
    sm[1][trk] = mixr;
  }

  double I5E01_APU::GetFrequencyPulse1() const    // // !!
  {
    if (!(length_counter[0] > 0 &&
      freq[0] >= 8 &&
      sfreq[0] < 0x800))
      return 0.0;
    return clock / 16 / (freq[0] + 1);
  }

  double I5E01_APU::GetFrequencyPulse2() const    // // !!
  {
    if (!(length_counter[1] > 0 &&
      freq[1] >= 8 &&
      sfreq[1] < 0x800))
      return 0.0;
    return clock / 16 / (freq[1] + 1);
  }

  bool I5E01_APU::Write(unsigned int adr, unsigned int val, unsigned int id)
  {
    int ch;

    // hack
    adr+=0x100;

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

    if (0x4100 <= adr && adr < 0x4108)
    {
      //DEBUG_OUT("$%04X = %02X\n",adr,val);

      adr &= 0xf;
      ch = adr >> 2;
      switch (adr)
      {
      case 0x0:
      case 0x4:
        volume[ch] = val & 15;
        envelope_disable[ch] = (val >> 4) & 1;
        envelope_loop[ch] = (val >> 5) & 1;
        envelope_div_period[ch] = (val & 15);
        duty[ch] = (val >> 6) & 3;
        if (option[OPT_DUTY_SWAP])
        {
          if (duty[ch] == 1) duty[ch] = 2;
          else if (duty[ch] == 2) duty[ch] = 1;
        }
        break;

      case 0x1:
      case 0x5:
        sweep_enable[ch] = (val >> 7) & 1;
        sweep_div_period[ch] = (((val >> 4) & 7));
        sweep_mode[ch] = (val >> 3) & 1;
        sweep_amount[ch] = val & 7;
        sweep_write[ch] = true;
        sweep_sqr(ch);
        break;

      case 0x2:
      case 0x6:
        freq[ch] = val | (freq[ch] & 0x700);
        sweep_sqr(ch);
        break;

      case 0x3:
      case 0x7:
        freq[ch] = (freq[ch] & 0xFF) | ((val & 0x7) << 8);
        if (option[OPT_PHASE_REFRESH])
          sphase[ch] = 0;
        envelope_write[ch] = true;
        if (enable[ch])
        {
          length_counter[ch] = length_table[(val >> 3) & 0x1f];
        }
        sweep_sqr(ch);
        break;

      default:
        return false;
      }
      reg[adr] = val;
      return true;
    }
    else if (adr == 0x4115)
    {
      enable[0] = (val & 1) ? true : false;
      enable[1] = (val & 2) ? true : false;

      if (!enable[0])
        length_counter[0] = 0;
      if (!enable[1])
        length_counter[1] = 0;

      reg[adr - 0x4100] = val;
      return true;
    }

    // 4017 is handled in nes_dmc.cpp
    //else if (adr == 0x4017)
    //{
    //}

    return false;
  }
} // namespace xgm;
