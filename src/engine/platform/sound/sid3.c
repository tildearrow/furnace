#include "sid3.h"

#include <math.h>

#define SAFETY_HEADER if(sid3 == NULL) return;

enum State { ATTACK, DECAY_SUSTAIN, RELEASE }; //for envelope

#ifndef M_PI
#  define M_PI    3.14159265358979323846
#endif

//these 4 ones are util only
double square(double x) {
  return fmod(x, (2 * M_PI)) >= M_PI ? -1 : 1;
}
double triangle(double x) {
  return asin(sin(x)) / (M_PI / 2);
}
double saw(double x) {
  return atan(tan(x / 2)) / (M_PI / 2);
}
double rectSquare(double x) {
  return square(x) > 0 ? square(x) : 0;
}
//===========================

double sinus(double x) { //taken from waveEdit.cpp of Furnace tracker
  return sin(x);
}
double rectSin(double x) {
  return sin(x) > 0 ? sin(x) : 0;
}
double absSin(double x) {
  return fabs(sin(x));
}

double quartSin(double x) {
  return absSin(x) * rectSquare(2 * x);
}
double squiSin(double x) {
  return sin(x) >= 0 ? sin(2 * x) : 0;
}
double squiAbsSin(double x) {
  return fabs(squiSin(x));
}

double rectSaw(double x) {
  return saw(x) > 0 ? saw(x) : 0;
}
double absSaw(double x) {
  return saw(x) < 0 ? saw(x) + 1 : saw(x);
}


double cubSaw(double x) {
  return pow(saw(x), 3);
}
double rectCubSaw(double x) {
  return pow(rectSaw(x), 3);
}
double absCubSaw(double x) {
  return pow(absSaw(x), 3);
}

double cubSine(double x) {
  return pow(sin(x), 3);
}
double rectCubSin(double x) {
  return pow(rectSin(x), 3);
}
double absCubSin(double x) {
  return pow(absSin(x), 3);
}
double quartCubSin(double x) {
  return pow(quartSin(x), 3);
}
double squishCubSin(double x) {
  return pow(squiSin(x), 3);
}
double squishAbsCubSin(double x) {
  return pow(squiAbsSin(x), 3);
}

double rectTri(double x) {
  return triangle(x) > 0 ? triangle(x) : 0;
}
double absTri(double x) {
  return fabs(triangle(x));
}
double quartTri(double x) {
  return absTri(x) * rectSquare(2 * x);
}
double squiTri(double x) {
  return sin(x) >= 0 ? triangle(2 * x) : 0;
}
double absSquiTri(double x) {
  return fabs(squiTri(x));
}

double cubTriangle(double x) {
  return pow(triangle(x), 3);
}
double cubRectTri(double x) {
  return pow(rectTri(x), 3);
}
double cubAbsTri(double x) {
  return pow(absTri(x), 3);
}
double cubQuartTri(double x) {
  return pow(quartTri(x), 3);
}
double cubSquiTri(double x) {
  return pow(squiTri(x), 3);
}
double absCubSquiTri(double x) {
  return fabs(cubSquiTri(x));
}

typedef double (*WaveFunc) (double a);

WaveFunc waveFuncs[]={
  sinus,
  rectSin,
  absSin,
  quartSin,
  squiSin,
  squiAbsSin,
  
  rectSaw,
  absSaw,
  
  cubSaw,
  rectCubSaw,
  absCubSaw,
  
  cubSine,
  rectCubSin,
  absCubSin,
  quartCubSin,
  squishCubSin,
  squishAbsCubSin,

  rectTri,
  absTri,
  quartTri,
  squiTri,
  absSquiTri,

  cubTriangle,
  cubRectTri,
  cubAbsTri,
  cubQuartTri,
  cubSquiTri,
  absCubSquiTri
};

SID3* sid3_create()
{
    SID3* sid3 = (SID3*)malloc(sizeof(SID3));

    memset(sid3, 0, sizeof(SID3));

    for(int i = 0; i < SID3_NUM_SPECIAL_WAVES; i++)
    {
        for(int j = 0; j < SID3_SPECIAL_WAVE_LENGTH; j++)
        {
            sid3->special_waves[i][j] = waveFuncs[i]((double)j / (double)SID3_SPECIAL_WAVE_LENGTH);
        }
    }

    return sid3;
}

void sid3_reset(SID3* sid3)
{
    SAFETY_HEADER

    for(int i = 0; i < SID3_NUM_CHANNELS - 1; i++)
    {
        memset(&sid3->chan[i], 0, sizeof(sid3_channel));
        sid3->chan[i].accumulator = 0;
        sid3->chan[i].adsr.a = 0x8;
        sid3->chan[i].adsr.d = 0x8;
        sid3->chan[i].adsr.s = 0x80;
        sid3->chan[i].adsr.r = 0x07;
        sid3->chan[i].adsr.vol = 0xf0;
        sid3->chan[i].adsr.hold_zero = true;
        //....
    }

    //TODO: wavetable chan
}

void sid3_gate_bit(uint8_t gate_next, uint8_t gate, sid3_channel_adsr* adsr)
{
    // The rate counter is never reset, thus there will be a delay before the
    // envelope counter starts counting up (attack) or down (release).

    // Gate bit on: Start attack, decay, sustain.
    if (!gate && gate_next) 
    {
        adsr->state = ATTACK;
        adsr->rate_period = adsr->a << 8; //todo: make it properly

        // Switching to attack state unlocks the zero freeze.
        adsr->hold_zero = false;

        adsr->rate_counter = 0;
        adsr->exponential_counter = 0;
        //envelope_counter = 0;

        if(adsr->envelope_counter == 0xff)
        {
            adsr->envelope_counter--; //idk why it happens, but when envelope has max sustain and I retrigger with new note it just becomes silent so this is the only solution I found so far
        }
    }
    // Gate bit off: Start release.
    else if (gate && !gate_next) 
    {
        adsr->state = RELEASE;
        adsr->rate_period = adsr->r << 8; //todo: make it properly

        adsr->rate_counter = 0;
        adsr->exponential_counter = 0;
    }

    //gate = gate_next;
}

void sid3_adsr_clock(sid3_channel_adsr* adsr)
{
    if(adsr->rate_counter < adsr->rate_period)
    {
        adsr->rate_counter++;
    }

    if(adsr->rate_counter > adsr->rate_period)
    {
        adsr->rate_counter = adsr->rate_period; //so you can do alternating writes (e.g. writing attack 10-11-10-11-... results in the somewhat average envelope speed)
    }

    if (adsr->rate_counter != adsr->rate_period) {
        return;
    }

    adsr->rate_counter = 0;

    // The first envelope step in the attack state also resets the exponential
    // counter. This has been verified by sampling ENV3.
    //
    if (adsr->state == ATTACK || ++adsr->exponential_counter == adsr->exponential_counter_period)
    {
        adsr->exponential_counter = 0;

        // Check whether the envelope counter is frozen at zero.
        if (adsr->hold_zero) 
        {
            return;
        }

        switch (adsr->state) 
        {
            case ATTACK:
            // The envelope counter can flip from 0xff to 0x00 by changing state to
            // release, then to attack. The envelope counter is then frozen at
            // zero; to unlock this situation the state must be changed to release,
            // then to attack. This has been verified by sampling ENV3.
            //
            adsr->envelope_counter++;
            adsr->envelope_counter &= 0xff;

            if (adsr->envelope_counter == 0xff) 
            {
                adsr->state = DECAY_SUSTAIN;
                adsr->rate_period = (adsr->d << 8); //todo: do it properly
            }
            break;
            case DECAY_SUSTAIN:
                if (adsr->envelope_counter != (adsr->s)) 
                {
                    --adsr->envelope_counter;
                }
            break;
            case RELEASE:
                // The envelope counter can flip from 0x00 to 0xff by changing state to
                // attack, then to release. The envelope counter will then continue
                // counting down in the release state.
                // This has been verified by sampling ENV3.
                // NB! The operation below requires two's complement integer.
                //
                //--envelope_counter &= 0xff;
                adsr->envelope_counter--;
            break;
        }
            
            // Check for change of exponential counter period.
        switch (adsr->envelope_counter) 
        {
            case 0xff:
            adsr->exponential_counter_period = 1;
            break;
            case 0x5d:
            adsr->exponential_counter_period = 2;
            break;
            case 0x36:
            adsr->exponential_counter_period = 4;
            break;
            case 0x1a:
            adsr->exponential_counter_period = 8;
            break;
            case 0x0e:
            adsr->exponential_counter_period = 16;
            break;
            case 0x06:
            adsr->exponential_counter_period = 30;
            break;
            case 0x00:
            adsr->exponential_counter_period = 1;

            // When the envelope counter is changed to zero, it is frozen at zero.
            // This has been verified by sampling ENV3.
            adsr->hold_zero = true;
            break;
        }
    }
}

int sid3_adsr_output(sid3_channel_adsr* adsr, int input)
{
    return (int)((int64_t)input * (int64_t)adsr->envelope_counter * (int64_t)adsr->vol / (int64_t)SID3_MAX_VOL);
}

void sid3_write(SID3* sid3, uint8_t address, uint8_t data)
{
    SAFETY_HEADER

    uint8_t channel = address / SID3_REGISTERS_PER_CHANNEL;

    if(channel >= SID3_NUM_CHANNELS) return;

    switch(address % SID3_REGISTERS_PER_CHANNEL) //NB: only works if registers for each channel are the same and their addresses
    //are x + y*n, where x is address of channel 0 register, y is number of current channel and n is how many registers you have 
    //for each channel
    {
        case 0:
        {
            if(channel != SID3_NUM_CHANNELS - 1)
            {
                uint8_t prev_flags = sid3->chan[channel].flags & SID3_CHAN_ENABLE_GATE;
                sid3->chan[channel].flags = data;
                sid3_gate_bit(sid3->chan[channel].flags & SID3_CHAN_ENABLE_GATE, prev_flags, &sid3->chan[channel].adsr);
            }
            else
            {
                uint8_t prev_flags = sid3->chan[channel].flags & SID3_CHAN_ENABLE_GATE;
                sid3->wave_chan.flags = data;
                sid3_gate_bit(sid3->wave_chan.flags & SID3_CHAN_ENABLE_GATE, prev_flags, &sid3->wave_chan.adsr);
            }
            break;
        }
        case 1:
        {
            if(channel != SID3_NUM_CHANNELS - 1)
            {
                sid3->chan[channel].adsr.a = data;
            }
            else
            {
                sid3->wave_chan.adsr.a = data;
            }
            break;
        }
        case 2:
        {
            if(channel != SID3_NUM_CHANNELS - 1)
            {
                sid3->chan[channel].adsr.d = data;
            }
            else
            {
                sid3->wave_chan.adsr.d = data;
            }
            break;
        }
        case 3:
        {
            if(channel != SID3_NUM_CHANNELS - 1)
            {
                sid3->chan[channel].adsr.s = data;
            }
            else
            {
                sid3->wave_chan.adsr.s = data;
            }
            break;
        }
        case 4:
        {
            if(channel != SID3_NUM_CHANNELS - 1)
            {
                sid3->chan[channel].adsr.sr = data;
            }
            else
            {
                sid3->wave_chan.adsr.sr = data;
            }
            break;
        }
        case 5:
        {
            if(channel != SID3_NUM_CHANNELS - 1)
            {
                sid3->chan[channel].adsr.r = data;
            }
            else
            {
                sid3->wave_chan.adsr.r = data;
            }
            break;
        }
        case 6:
        {
            if(channel != SID3_NUM_CHANNELS - 1)
            {
                sid3->chan[channel].waveform = data;
            }
            else
            {
                sid3->wave_chan.mode = data;
            }
            break;
        }
        case 7:
        {
            if(channel != SID3_NUM_CHANNELS - 1)
            {
                sid3->chan[channel].pw &= 0x00ff;
                sid3->chan[channel].pw |= data << 8;
            }
            else
            {
                sid3->wave_chan.wave_address = data;
            }
            break;
        }
        case 8:
        {
            if(channel != SID3_NUM_CHANNELS - 1)
            {
                sid3->chan[channel].pw &= 0xff00;
                sid3->chan[channel].pw |= data;
            }
            else
            {
                sid3->wave_chan.wavetable[sid3->wave_chan.wave_address] = data;
            }
            break;
        }
        case 9:
        {
            if(channel != SID3_NUM_CHANNELS - 1)
            {
                sid3->chan[channel].special_wave = data;
            }
            break;
        }
        case 10:
        {
            if(channel != SID3_NUM_CHANNELS - 1)
            {
                sid3->chan[channel].frequency &= 0x00ffff;
                sid3->chan[channel].frequency |= data << 16;
            }
            else
            {
                sid3->wave_chan.frequency &= 0x00ffff;
                sid3->wave_chan.frequency |= data << 16;
            }
            break;
        }
        case 11:
        {
            if(channel != SID3_NUM_CHANNELS - 1)
            {
                sid3->chan[channel].frequency &= 0xff00ff;
                sid3->chan[channel].frequency |= data << 8;
            }
            else
            {
                sid3->wave_chan.frequency &= 0xff00ff;
                sid3->wave_chan.frequency |= data << 8;
            }
            break;
        }
        case 12:
        {
            if(channel != SID3_NUM_CHANNELS - 1)
            {
                sid3->chan[channel].frequency &= 0xffff00;
                sid3->chan[channel].frequency |= data;
            }
            else
            {
                sid3->wave_chan.frequency &= 0xffff00;
                sid3->wave_chan.frequency |= data;
            }
            break;
        }
        default: break;
    }
}

uint16_t sid3_pulse(uint32_t acc, uint16_t pw) // 0-FFFF pulse width range
{
    return (((acc >> ((SID3_ACC_BITS - 16))) >= ((pw == 0xffff ? pw + 1 : pw)) ? (0xffff) : 0));
}

uint16_t sid3_saw(uint32_t acc) 
{
    return (acc >> (SID3_ACC_BITS - 16)) & (0xffff);
}

uint16_t sid3_triangle(uint32_t acc) 
{
    return (((acc < (1 << (SID3_ACC_BITS - 1))) ? ~acc : acc) >> (SID3_ACC_BITS - 17));
}

uint16_t sid3_get_waveform(sid3_channel* ch)
{
    switch(ch->mix_mode)
    {
        case SID3_MIX_8580:
        {
            switch(ch->waveform)
            {
                case SID3_WAVE_TRIANGLE:
                {
                    return sid3_triangle(ch->accumulator);
                    break;
                }
                case SID3_WAVE_SAW:
                {
                    return sid3_saw(ch->accumulator);
                    break;
                }
                case SID3_WAVE_PULSE:
                {
                    return sid3_pulse(ch->accumulator, ch->pw);
                    break;
                }
            }
            break;
        }
        default: break;
    }
}

void sid3_clock(SID3* sid3)
{
    SAFETY_HEADER

    sid3->output_l = sid3->output_r = 0;

    for(int i = 0; i < SID3_NUM_CHANNELS - 1; i++)
    {
        sid3_channel* ch = &sid3->chan[i];
        
        uint32_t prev_acc = ch->accumulator;

        ch->accumulator += ch->frequency;

        if(ch->accumulator & (1 << SID3_ACC_BITS))
        {
            ch->sync_bit = 1;
        }
        else
        {
            ch->sync_bit = 0;
        }

        ch->accumulator &= SID3_ACC_MASK;

        //todo: phase mod

        int waveform = sid3_get_waveform(ch);
        waveform -= 0x7fff;

        sid3_adsr_clock(&ch->adsr);
        sid3->output_l += sid3_adsr_output(&ch->adsr, waveform / 1024);
        sid3->output_r += sid3_adsr_output(&ch->adsr, waveform / 1024);

        sid3->channel_output[i] = sid3_adsr_output(&ch->adsr, waveform / 1024);
    }
}

void sid3_set_is_muted(SID3* sid3, uint8_t ch, bool mute)
{
    SAFETY_HEADER
}

void sid3_free(SID3* sid3)
{
    SAFETY_HEADER

    free(sid3);
}