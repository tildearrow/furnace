#include "nes_mmc5.h"

namespace xgm
{

  NES_MMC5::NES_MMC5 ()
  {
    cpu = NULL;
    SetClock (DEFAULT_CLOCK);
    SetRate (DEFAULT_RATE);
    option[OPT_NONLINEAR_MIXER] = true;
    option[OPT_PHASE_REFRESH] = true;
    frame_sequence_count = 0;

    // square nonlinear mix, same as 2A03
    square_table[0] = 0;
    for(int i=1;i<32;i++) 
        square_table[i]=(INT32)((8192.0*95.88)/(8128.0/i+100));

    // 2A03 style nonlinear pcm mix with double the bits
    //pcm_table[0] = 0;
    //INT32 wd = 22638;
    //for(int d=1;d<256; ++d)
    //    pcm_table[d] = (INT32)((8192.0*159.79)/(100.0+1.0/((double)d/wd)));

    // linear pcm mix (actual hardware seems closer to this)
    pcm_table[0] = 0;
    double pcm_scale = 32.0;
    for (int d=1; d<256; ++d)
        pcm_table[d] = (INT32)(double(d) * pcm_scale);

    // stereo mix
    for(int c=0;c<2;++c)
        for(int t=0;t<3;++t)
            sm[c][t] = 128;
  }

  NES_MMC5::~NES_MMC5 ()
  {
  }

  void NES_MMC5::Reset ()
  {
    int i;

    scounter[0] = 0;
    scounter[1] = 0;
    sphase[0] = 0;
    sphase[1] = 0;

    envelope_div[0] = 0;
    envelope_div[1] = 0;
    length_counter[0] = 0;
    length_counter[1] = 0;
    envelope_counter[0] = 0;
    envelope_counter[1] = 0;
    frame_sequence_count = 0;

    for (i = 0; i < 8; i++)
      Write (0x5000 + i, 0);

    Write(0x5015, 0);

    for (i = 0; i < 3; ++i)
        out[i] = 0;

    mask = 0;
    pcm = 0; // PCM channel
    pcm_mode = false; // write mode

    SetRate(rate);
  }

  void NES_MMC5::SetOption (int id, int val)
  {
    if(id<OPT_END) option[id] = val;
  }

  void NES_MMC5::SetClock (double c)
  {
    this->clock = c;
  }

  void NES_MMC5::SetRate (double r)
  {
    rate = r ? r : DEFAULT_RATE;
  }

  void NES_MMC5::FrameSequence ()
  {
    // 240hz clock
    for (int i=0; i < 2; ++i)
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

    // MMC5 length counter is clocked at 240hz, unlike 2A03
    for (int i=0; i < 2; ++i)
    {
        if (!envelope_loop[i] && (length_counter[i] > 0))
            --length_counter[i];
    }
  }

  INT32 NES_MMC5::calc_sqr (int i, UINT32 clocks)
  {
    static const INT16 sqrtbl[4][16] = {
      {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0},
      {1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };

    scounter[i] += clocks;
    while (scounter[i] > freq[i])
    {
        sphase[i] = (sphase[i] + 1) & 15;
        scounter[i] -= (freq[i] + 1);
    }

    INT32 ret = 0;
    if (length_counter[i] > 0)
    {
        // note MMC5 does not silence the highest 8 frequencies like APU,
        // because this is done by the sweep unit.

        int v = envelope_disable[i] ? volume[i] : envelope_counter[i];
        ret = sqrtbl[duty[i]][sphase[i]] ? v : 0;
    }
    
    return ret;
  }

  void NES_MMC5::TickFrameSequence (UINT32 clocks)
  {
      frame_sequence_count += clocks;
      while (frame_sequence_count > 7458)
      {
          FrameSequence();
          frame_sequence_count -= 7458;
      }
  }

  void NES_MMC5::Tick (UINT32 clocks)
  {
    out[0] = calc_sqr(0, clocks);
    out[1] = calc_sqr(1, clocks);
    out[2] = pcm;
  }

  UINT32 NES_MMC5::Render (INT32 b[2])
  {
    out[0] = (mask & 1) ? 0 : out[0];
    out[1] = (mask & 2) ? 0 : out[1];
    out[2] = (mask & 4) ? 0 : out[2];

    INT32 m[3];

    if(option[OPT_NONLINEAR_MIXER])
    {
        // squares nonlinear
        INT32 voltage = square_table[out[0] + out[1]];
        m[0] = out[0] << 6;
        m[1] = out[1] << 6;
        INT32 ref = m[0] + m[1];
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

        // pcm nonlinear
        m[2] = pcm_table[out[2]];
    }
    else
    {
        // squares
        m[0] = out[0] << 6;
        m[1] = out[1] << 6;

        // pcm channel
        m[2] = out[2] << 5;
    }

    // note polarity is flipped on output

    b[0]  = m[0] * -sm[0][0];
    b[0] += m[1] * -sm[0][1];
    b[0] += m[2] * -sm[0][2];
    b[0] >>= 7;

    b[1]  = m[0] * -sm[1][0];
    b[1] += m[1] * -sm[1][1];
    b[1] += m[2] * -sm[1][2];
    b[1] >>= 7;

    return 2;
  }

  bool NES_MMC5::Write (UINT32 adr, UINT32 val, UINT32 id)
  {
    int ch;

    static const UINT8 length_table[32] = {
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

    if ((0x5c00 <= adr) && (adr < 0x5ff0))
    {
      ram[adr & 0x3ff] = val;
      return true;
    }
    else if ((0x5000 <= adr) && (adr < 0x5008))
    {
      reg[adr & 0x7] = val;
    }

    switch (adr)
    {
    case 0x5000:
    case 0x5004:
      ch = (adr >> 2) & 1;
      volume[ch] = val & 15;
      envelope_disable[ch] = (val >> 4) & 1;
      envelope_loop[ch] = (val >> 5) & 1;
      envelope_div_period[ch] = (val & 15);
      duty[ch] = (val >> 6) & 3;
      break;

    case 0x5002:
    case 0x5006:
      ch = (adr >> 2) & 1;
      freq[ch] = val + (freq[ch] & 0x700);
      if (scounter[ch] > freq[ch]) scounter[ch] = freq[ch];
      break;

    case 0x5003:
    case 0x5007:
      ch = (adr >> 2) & 1;
      freq[ch] = (freq[ch] & 0xff) + ((val & 7) << 8);
      if (scounter[ch] > freq[ch]) scounter[ch] = freq[ch];
      // phase reset
      if (option[OPT_PHASE_REFRESH])
        sphase[ch] = 0;
      envelope_write[ch] = true;
      if (enable[ch])
      {
        length_counter[ch] = length_table[(val >> 3) & 0x1f];
      }
      break;

    // PCM channel control
    case 0x5010:
      pcm_mode = ((val & 1) != 0); // 0 = write, 1 = read
      break;

    // PCM channel control
    case 0x5011:
      if (!pcm_mode)
      {
          val &= 0xFF;
          if (val != 0) pcm = val;
      }
      break;

    case 0x5015:
      enable[0] = (val & 1) ? true : false;
      enable[1] = (val & 2) ? true : false;
      if (!enable[0])
          length_counter[0] = 0;
      if (!enable[1])
          length_counter[1] = 0;
      break;

    case 0x5205:
      mreg[0] = val;
      break;

    case 0x5206:
      mreg[1] = val;
      break;

    default:
      return false;

    }
    return true;
  }

  bool NES_MMC5::Read (UINT32 adr, UINT32 & val, UINT32 id)
  {
    // in PCM read mode, reads from $8000-$C000 automatically load the PCM output
    if (pcm_mode && (0x8000 <= adr) && (adr < 0xC000) && cpu)
    {
        pcm_mode = false; // prevent recursive entry
        UINT32 pcm_read;
        cpu->Read(adr, pcm_read);
        pcm_read &= 0xFF;
        if (pcm_read != 0)
            pcm = pcm_read;
        pcm_mode = true;
    }

    if ((0x5000 <= adr) && (adr < 0x5008))
    {
        val = reg[adr&0x7];
        return true;
    }
    else if(adr == 0x5015)
    {
        val = (enable[1]?2:0)|(enable[0]?1:0);
        return true;
    }

    if ((0x5c00 <= adr) && (adr < 0x5ff0))
    {
      val = ram[adr & 0x3ff];
      return true;
    }
    else if (adr == 0x5205)
    {
      val = (mreg[0] * mreg[1]) & 0xff;
      return true;
    }
    else if (adr == 0x5206)
    {
      val = (mreg[0] * mreg[1]) >> 8;
      return true;
    }

    return false;
  }

  void NES_MMC5::SetStereoMix(int trk, xgm::INT16 mixl, xgm::INT16 mixr)
  {
      if (trk < 0) return;
      if (trk > 2) return;
      sm[0][trk] = mixl;
      sm[1][trk] = mixr;
  }

  ITrackInfo *NES_MMC5::GetTrackInfo(int trk)
  {
    assert(trk<3);

    if (trk < 2) // square
    {
        trkinfo[trk]._freq = freq[trk];
        if(freq[trk])
          trkinfo[trk].freq = clock/16/(freq[trk] + 1);
        else
          trkinfo[trk].freq = 0;

        trkinfo[trk].output = out[trk];
        trkinfo[trk].max_volume = 15;
        trkinfo[trk].volume = volume[trk]+(envelope_disable[trk]?0:0x10);
        trkinfo[trk].key = (envelope_disable[trk]?(volume[trk]>0): (envelope_counter[trk]>0));
        trkinfo[trk].tone = duty[trk];
    }
    else // pcm
    {
        trkinfo[trk]._freq = 0;
        trkinfo[trk].freq = 0;
        trkinfo[trk].output = out[2];
        trkinfo[trk].max_volume = 255;
        trkinfo[trk].volume = pcm;
        trkinfo[trk].key = 0;
        trkinfo[trk].tone = pcm_mode ? 1 : 0;
    }

    return &trkinfo[trk];
  }

  // pcm read mode requires CPU read access
  void NES_MMC5::SetCPU(NES_CPU* cpu_)
  {
      cpu = cpu_;
  }

}// namespace
