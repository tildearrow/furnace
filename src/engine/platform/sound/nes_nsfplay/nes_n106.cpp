#include <cstring>
#include "nes_n106.h"
#include "common.h"

namespace xgm {

NES_N106::NES_N106 ()
{
    option[OPT_SERIAL] = 0;
    option[OPT_PHASE_READ_ONLY] = 0;
    option[OPT_LIMIT_WAVELENGTH] = 0;
    SetClock (DEFAULT_CLOCK);
    SetRate (DEFAULT_RATE);
    for (int i=0; i < 8; ++i)
    {
        sm[0][i] = 128;
        sm[1][i] = 128;
    }
    Reset();
}

NES_N106::~NES_N106 ()
{
}

void NES_N106::SetStereoMix (int trk, short mixl, short mixr)
{
    if (trk < 0 || trk >= 8) return;
    trk = 7-trk; // displayed channels are inverted
    sm[0][trk] = mixl;
    sm[1][trk] = mixr;
}


void NES_N106::SetClock (double c)
{
    clock = c;
}

void NES_N106::SetRate (double r)
{
    rate = r;
}

void NES_N106::SetMask (int m)
{
    // bit reverse the mask,
    // N163 waves are displayed in reverse order
    mask = 0
        | ((m & (1<<0)) ? (1<<7) : 0)
        | ((m & (1<<1)) ? (1<<6) : 0)
        | ((m & (1<<2)) ? (1<<5) : 0)
        | ((m & (1<<3)) ? (1<<4) : 0)
        | ((m & (1<<4)) ? (1<<3) : 0)
        | ((m & (1<<5)) ? (1<<2) : 0)
        | ((m & (1<<6)) ? (1<<1) : 0)
        | ((m & (1<<7)) ? (1<<0) : 0);
}

void NES_N106::SetOption (int id, int val)
{
    if (id<OPT_END) option[id] = val;
}

void NES_N106::Reset ()
{
    master_disable = false;
    ::memset(reg, 0, sizeof(reg));
    reg_select = 0;
    reg_advance = false;
    tick_channel = 0;
    tick_clock = 0;
    render_channel = 0;
    render_clock = 0;
    render_subclock = 0;

    for (int i=0; i<8; ++i) fout[i] = 0;

    Write(0xE000, 0x00); // master disable off
    Write(0xF800, 0x80); // select $00 with auto-increment
    for (unsigned int i=0; i<0x80; ++i) // set all regs to 0
    {
        Write(0x4800, 0x00);
    }
    Write(0xF800, 0x00); // select $00 without auto-increment
}

void NES_N106::Tick (unsigned int clocks)
{
    if (master_disable) return;

    int channels = get_channels();

    tick_clock += clocks;
    render_clock += clocks; // keep render in sync
    while (tick_clock > 0)
    {
        int channel = 7-tick_channel;

        unsigned int phase = get_phase(channel);
        unsigned int freq  = get_freq(channel);
        unsigned int len   = get_len(channel);
        unsigned int off   = get_off(channel);
        int  vol   = get_vol(channel);

        // accumulate 24-bit phase
        phase = (phase + freq) & 0x00FFFFFF;

        // wrap phase if wavelength exceeded
        unsigned int hilen = len << 16;
        while (phase >= hilen) phase -= hilen;

        // write back phase
        set_phase(phase, channel);

        // fetch sample (note: N163 output is centred at 8, and inverted w.r.t 2A03)
        int sample = 8 - get_sample(((phase >> 16) + off) & 0xFF);
        fout[channel] = sample * vol;

        // cycle to next channel every 15 clocks
        tick_clock -= 15;
        ++tick_channel;
        if (tick_channel >= channels)
            tick_channel = 0;
    }
}

unsigned int NES_N106::Render (int b[2])
{
    b[0] = 0;
    b[1] = 0;
    if (master_disable) return 2;

    int channels = get_channels();

    if (option[OPT_SERIAL]) // hardware accurate serial multiplexing
    {
        // this could be made more efficient than going clock-by-clock
        // but this way is simpler
        int clocks = render_clock;
        while (clocks > 0)
        {
            int c = 7-render_channel;
            if (0 == ((mask >> c) & 1))
            {
                b[0] += fout[c] * sm[0][c];
                b[1] += fout[c] * sm[1][c];
            }
            
            ++render_subclock;
            if (render_subclock >= 15) // each channel gets a 15-cycle slice
            {
                render_subclock = 0;
                ++render_channel;
                if (render_channel >= channels)
                    render_channel = 0;
            }
            --clocks;
        }

        // increase output level by 1 bits (7 bits already added from sm)
        b[0] <<= 1;
        b[1] <<= 1;

        // average the output
        if (render_clock > 0)
        {
            b[0] /= render_clock;
            b[1] /= render_clock;
        }
        render_clock = 0;
    }
    else // just mix all channels
    {
        for (int i = (8-channels); i<8; ++i)
        {
            if (0 == ((mask >> i) & 1))
            {
                b[0] += fout[i] * sm[0][i];
                b[1] += fout[i] * sm[1][i];
            }
        }

        // mix together, increase output level by 8 bits, roll off 7 bits from sm
        int MIX[9] = { 256/1, 256/1, 256/2, 256/3, 256/4, 256/5, 256/6, 256/6, 256/6 };
        b[0] = (b[0] * MIX[channels]) >> 7;
        b[1] = (b[1] * MIX[channels]) >> 7;
        // when approximating the serial multiplex as a straight mix, once the
        // multiplex frequency gets below the nyquist frequency an average mix
        // begins to sound too quiet. To approximate this effect, I don't attenuate
        // any further after 6 channels are active.
    }

    // 8 bit approximation of master volume
    // max N163 vol vs max APU square
    // unfortunately, games have been measured as low as 3.4x and as high as 8.5x
    // with higher volumes on Erika, King of Kings, and Rolling Thunder
    // and lower volumes on others. Using 6.0x as a rough "one size fits all".
    const double MASTER_VOL = 6.0 * 1223.0;
    const double MAX_OUT = 15.0 * 15.0 * 256.0; // max digital value
    const int GAIN = int((MASTER_VOL / MAX_OUT) * 256.0f);
    b[0] = (b[0] * GAIN) >> 8;
    b[1] = (b[1] * GAIN) >> 8;

    return 2;
}

bool NES_N106::Write (unsigned int adr, unsigned int val, unsigned int id)
{
    if (adr == 0xE000) // master disable
    {
        master_disable = ((val & 0x40) != 0);
        return true;
    }
    else if (adr == 0xF800) // register select
    {
        reg_select = (val & 0x7F);
        reg_advance = (val & 0x80) != 0;
        return true;
    }
    else if (adr == 0x4800) // register write
    {
        if (option[OPT_PHASE_READ_ONLY]) // old emulators didn't know phase was stored here
        {
            int c = 15 - (reg_select/8);
            int r = reg_select & 7;
            if (c < get_channels() &&
                (r == 1 ||
                 r == 3 ||
                 r == 5))
            {
                if (reg_advance)
                    reg_select = (reg_select + 1) & 0x7F;
                return true;
            }
        }
        if (option[OPT_LIMIT_WAVELENGTH]) // old emulators ignored top 3 bits of length
        {
            int c = 15 - (reg_select/8);
            int r = reg_select & 7;
            if (c < get_channels() && r == 4)
            {
                val |= 0xE0;
            }
        }
        reg[reg_select] = val;
        if (reg_advance)
            reg_select = (reg_select + 1) & 0x7F;
        return true;
    }
    return false;
}

bool NES_N106::Read (unsigned int adr, unsigned int & val, unsigned int id)
{
    if (adr == 0x4800) // register read
    {
        val = reg[reg_select];
        if (reg_advance)
            reg_select = (reg_select + 1) & 0x7F;
        return true;
    }
    return false;
}

//
// register decoding/encoding functions
// 

inline unsigned int NES_N106::get_phase (int channel)
{
    // 24-bit phase stored in channel regs 1/3/5
    channel = channel << 3;
    return (reg[0x41 + channel]      )
        +  (reg[0x43 + channel] << 8 )
        +  (reg[0x45 + channel] << 16);
}

inline unsigned int NES_N106::get_freq (int channel)
{
    // 19-bit frequency stored in channel regs 0/2/4
    channel = channel << 3;
    return ( reg[0x40 + channel]              )
        +  ( reg[0x42 + channel]         << 8 )
        +  ((reg[0x44 + channel] & 0x03) << 16);
}

inline unsigned int NES_N106::get_off (int channel)
{
    // 8-bit offset stored in channel reg 6
    channel = channel << 3;
    return reg[0x46 + channel];
}

inline unsigned int NES_N106::get_len (int channel)
{
    // 6-bit<<3 length stored obscurely in channel reg 4
    channel = channel << 3;
    return 256 - (reg[0x44 + channel] & 0xFC);
}

inline int NES_N106::get_vol (int channel)
{
    // 4-bit volume stored in channel reg 7
    channel = channel << 3;
    return reg[0x47 + channel] & 0x0F;
}

inline int NES_N106::get_sample (unsigned int index)
{
    // every sample becomes 2 samples in regs
    return (index&1) ?
        ((reg[index>>1] >> 4) & 0x0F) :
        ( reg[index>>1]       & 0x0F) ;
}

inline int NES_N106::get_channels ()
{
    // 3-bit channel count stored in reg 0x7F
    return ((reg[0x7F] >> 4) & 0x07) + 1;
}

inline void NES_N106::set_phase (unsigned int phase, int channel)
{
    // 24-bit phase stored in channel regs 1/3/5
    channel = channel << 3;
    reg[0x41 + channel] =  phase        & 0xFF;
    reg[0x43 + channel] = (phase >> 8 ) & 0xFF;
    reg[0x45 + channel] = (phase >> 16) & 0xFF;
}

} //namespace
