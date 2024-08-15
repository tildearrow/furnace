// THIS IS A MODIFIED VERSION OF POTATOR'S SOUND EMULATION CORE
// MODIFIED BY AART1256 IN 2024

#include "supervision.h"

#include <string.h>

uint32 UNSCALED_CLOCK = 4000000;
#define SV_SAMPLE_RATE ((UNSCALED_CLOCK)/64)
#define SV_DEC_TICK ((SV_SAMPLE_RATE)/60)

uint32 decrement_tick = 0;

uint8 supervision_dma_mem[65536];

void supervision_sound_set_clock(uint32 clock) {
    UNSCALED_CLOCK = clock;
}

void supervision_memorymap_registers_write(uint32 Addr, uint8 Value)
{
    switch (Addr & 0x1fff) {
        case 0x10: case 0x11: case 0x12: case 0x13:
        case 0x14: case 0x15: case 0x16: case 0x17:
            supervision_sound_wave_write(((Addr & 0x4) >> 2), Addr & 3, Value);
            break;
        case 0x18:
        case 0x19:
        case 0x1a:
        case 0x1b:
        case 0x1c:
            supervision_sound_dma_write(Addr & 0x07, Value);
            break;
        case 0x28:
        case 0x29:
        case 0x2a:
            supervision_sound_noise_write(Addr & 0x07, Value);
            break;
    }
}

uint32 ch_mask = 15;
uint32 flags = 0b00000001;

void supervision_set_mute_mask(uint32 mask) {
    ch_mask = mask;
}

void supervision_sound_set_flags(uint32 flags_set)
{
    flags = flags_set;
}

typedef struct {
    uint8 reg[4];
    int on;
    uint8 waveform, volume;
    uint16 pos, size;
    uint16 count;
} SVISION_CHANNEL;
SVISION_CHANNEL m_channel[2];
// For clear sound (no grating), sync with m_channel
SVISION_CHANNEL ch[2];

typedef struct  {
    uint8 reg[3];
    int on, right, left, play;
    uint8 type; // 6 - 7-Bit, 14 - 15-Bit
    uint16 state;
    uint8 value, volume;
    uint16 count;
    real pos, step;
} SVISION_NOISE;
SVISION_NOISE m_noise;

typedef struct  {
    uint8 reg[5];
    int on, right, left;
    uint32 ca14to16;
    uint16 start;
    uint16 size;
    real pos, step;
} SVISION_DMA;
SVISION_DMA m_dma;

void supervision_sound_reset(void)
{
    memset(m_channel, 0, sizeof(m_channel));
    memset(&m_noise,  0, sizeof(m_noise)  );
    memset(&m_dma,    0, sizeof(m_dma)    );

    memset(ch,        0, sizeof(ch)       );
    decrement_tick = 0;
    ch_mask = 15;
}

void supervision_sound_stream_update(uint8 *stream, uint32 len)
{
    size_t i, j;
    SVISION_CHANNEL *channel;
    uint8 s = 0;
    uint8 *left  = stream + 0;
    uint8 *right = stream + 1;
    uint8 *chout = stream + 2;

    for (i = 0; i < len >> 1; i++, left += 2, right += 2) {
        *left = *right = 0;
        for (channel = m_channel, j = 0; j < 2; j++, channel++) {
            chout[j] = 0;
            if (ch[j].size != 0) {
                if (ch[j].on || channel->count != 0) {
                    BOOL on = FALSE;
                    switch (ch[j].waveform) {
                        case 0: // 12.5%
                            on = ch[j].pos < (28 * ch[j].size) >> 5;
                            break;
                        case 1: // 25%
                            on = ch[j].pos < (24 * ch[j].size) >> 5;
                            break;
                        case 2: // 50%
                            on = ch[j].pos < ch[j].size / 2;
                            break;
                        case 3: // 75%
                            on = ch[j].pos < ch[j].size / 4;
                            // MESS/MAME:  <= (9 * ch[j].size) >> 5;
                            break;
                    }
                    s = on ? (ch[j].volume)<<2 : 0;
                    s = ((ch_mask>>(3-j))&1)?s:0;
                    if (flags&1) {
                        if (j == 0)
                            *right += s;
                        else
                            *left += s;
                    } else {
                        *left += s;
                        *right += s;
                    }
                    chout[j] = s;
                }
                ch[j].pos++;
                if (ch[j].pos >= ch[j].size) {
                    ch[j].pos = 0;
                    // Transition from off to on
                    if (channel->on) {
                        memcpy(&ch[j], channel, sizeof(ch[j]));
                        channel->on = FALSE;
                    }
                }
            }
        }

        if (m_noise.on && (m_noise.play || m_noise.count != 0)) {
            s = (m_noise.value * m_noise.volume) << 2;
            s = ch_mask&1?s:0;
            chout[3] = 0;
            if (m_noise.left) {
                *left += s;
                chout[3] = s;
            }
            if (m_noise.right) {
                *right += s;
                chout[3] = s;
            }
            m_noise.pos += m_noise.step;
            while (m_noise.pos >= 1.0) { // if/while difference - Pacific Battle
                // LFSR: x^2 + x + 1
                uint16 feedback;
                m_noise.value = m_noise.state & 1;
                feedback = ((m_noise.state >> 1) ^ m_noise.state) & 0x0001;
                feedback <<= m_noise.type;
                m_noise.state = (m_noise.state >> 1) | feedback;
                m_noise.pos -= 1.0;
            }
        }

        chout[2] = 0;
        if (m_dma.on) {
            uint8 sample;
            uint16 addr = m_dma.start + (uint16)m_dma.pos / 2;
            if (addr >= 0x8000 && addr < 0xc000) {
                sample = supervision_dma_mem[(addr & 0x3fff) | m_dma.ca14to16];
            }
            if (((uint16)m_dma.pos) & 1)
                s = (sample & 0xf);
            else
                s = (sample & 0xf0) >> 4;
            s <<= 2;
            s = ((ch_mask>>1)&1)?s:0;
            chout[2] = 0;
            if (m_dma.left) {
                *left += s;
                chout[2] = s;
            }
            if (m_dma.right) {
                *right += s;
                chout[2] = s;
            }
            m_dma.pos += m_dma.step;
            if (m_dma.pos >= m_dma.size) {
                m_dma.on = FALSE;
            }
        }

        if (decrement_tick > SV_DEC_TICK) {
            decrement_tick = 0;
            supervision_sound_decrement();
        }
        decrement_tick++;
    }
}

void supervision_sound_decrement(void)
{
    if (m_channel[0].count > 0)
        m_channel[0].count--;
    if (m_channel[1].count > 0)
        m_channel[1].count--;
    if (m_noise.count > 0)
        m_noise.count--;
}

void supervision_sound_wave_write(int which, int offset, uint8 data)
{
    SVISION_CHANNEL *channel = &m_channel[which];

    channel->reg[offset] = data;
    switch (offset) {
        case 0:
        case 1: {
            uint16 size;
            size = channel->reg[0] | ((channel->reg[1] & 7) << 8);
            // if size == 0 then channel->size == 0
			if (size)
                channel->size = (uint16)(((real)SV_SAMPLE_RATE) * ((real)((size + 1) << 5)) / ((real)UNSCALED_CLOCK));
            else
                channel->size = 0;
            channel->pos = 0;
            // Popo Team
            if (channel->count != 0 || ch[which].size == 0 || channel->size == 0) {
                ch[which].size = channel->size;
                if (channel->count == 0)
                    ch[which].pos = 0;
            }
        }
            break;
        case 2:
            channel->on       =  data & 0x40;
            channel->waveform = (data & 0x30) >> 4;
            channel->volume   =  data & 0x0f;
            if (!channel->on || ch[which].size == 0 || channel->size == 0) {
                uint16 pos = ch[which].pos;
                memcpy(&ch[which], channel, sizeof(ch[which]));
                if (channel->count != 0) // Journey to the West
                    ch[which].pos = pos;
            }
            break;
        case 3:
            channel->count = data + 1;
            ch[which].size = channel->size; // Sonny Xpress!
            break;
    }
}

void supervision_sound_dma_write(int offset, uint8 data)
{
    m_dma.reg[offset] = data;
    switch (offset) {
        case 0:
        case 1:
            m_dma.start = (m_dma.reg[0] | (m_dma.reg[1] << 8));
            break;
        case 2:
            m_dma.size = (data ? data : 0x100) * 32; // Number of 4-bit samples
            break;
        case 3:
            // Test games: Classic Casino, SSSnake
            m_dma.step = ((real)UNSCALED_CLOCK) / ((real)SV_SAMPLE_RATE * (256 << (data & 3)));
            // MESS/MAME. Wrong
            //m_dma.step  = UNSCALED_CLOCK / (256.0 * SV_SAMPLE_RATE * (1 + (data & 3)));
            m_dma.right = data & 4;
            m_dma.left  = data & 8;
            m_dma.ca14to16 = ((data & 0x70) >> 4) << 14;
            break;
        case 4:
            m_dma.on = data & 0x80;
            if (m_dma.on) {
                m_dma.pos = 0.0;
            }
            break;
    }
}

void supervision_sound_noise_write(int offset, uint8 data)
{
    m_noise.reg[offset] = data;
    switch (offset) {
        case 0: {
            uint32 divisor = 8 << (data >> 4);
			if (divisor)
                m_noise.step = ((real)UNSCALED_CLOCK) / ((real)SV_SAMPLE_RATE * divisor);
            else
                m_noise.step = 0;

            m_noise.step = ((real)UNSCALED_CLOCK) / ((real)SV_SAMPLE_RATE * divisor);
            m_noise.volume = data & 0xf;
        }
            break;
        case 1:
            m_noise.count = data + 1;
            break;
        case 2:
            m_noise.type  = (data & 1) ? 14 : 6;
            m_noise.play  =  data & 2;
            m_noise.right =  data & 4;
            m_noise.left  =  data & 8;
            m_noise.on    =  data & 0x10; /* honey bee start */
            m_noise.state = 1;
            break;
    }
    m_noise.pos = 0.0;
}
