// THIS IS A MODIFIED VERSION OF POTATOR'S SOUND EMULATION CORE
// MODIFIED BY AART1256 IN 2024

#include "supervision.h"

#include <string.h>

#define SV_SAMPLE_RATE ((svision->UNSCALED_CLOCK)/64)
#define SV_DEC_TICK ((SV_SAMPLE_RATE)/60)

void supervision_sound_set_clock(struct svision_t *svision, uint32 clock) {
    svision->UNSCALED_CLOCK = clock;
}

void supervision_memorymap_registers_write(struct svision_t *svision, uint32 Addr, uint8 Value)
{
    switch (Addr & 0x1fff) {
        case 0x10: case 0x11: case 0x12: case 0x13:
        case 0x14: case 0x15: case 0x16: case 0x17:
            supervision_sound_wave_write(svision, ((Addr & 0x4) >> 2), Addr & 3, Value);
            break;
        case 0x18:
        case 0x19:
        case 0x1a:
        case 0x1b:
        case 0x1c:
            supervision_sound_dma_write(svision, Addr & 0x07, Value);
            break;
        case 0x28:
        case 0x29:
        case 0x2a:
            supervision_sound_noise_write(svision, Addr & 0x07, Value);
            break;
    }
}

void supervision_set_mute_mask(struct svision_t *svision, uint8 mask) {
    svision->ch_mask = mask;
}

void supervision_sound_set_flags(struct svision_t *svision, uint8 flags_set)
{
    svision->flags = flags_set;
}

void supervision_sound_reset(struct svision_t *svision)
{
    memset(svision->m_channel, 0, sizeof(svision->m_channel));
    memset(&svision->m_noise,  0, sizeof(svision->m_noise)  );
    memset(&svision->m_dma,    0, sizeof(svision->m_dma)    );

    memset(svision->ch,        0, sizeof(svision->ch)       );
    svision->decrement_tick = 0;
    svision->ch_mask = 15;
}

void supervision_sound_stream_update(struct svision_t *svision, uint8 *stream, uint32 len)
{
    size_t i, j;
    SVISION_CHANNEL *channel;
    uint8 s = 0;
    uint8 *left  = stream + 0;
    uint8 *right = stream + 1;
    uint8 *chout = stream + 2;

    for (i = 0; i < len >> 1; i++, left += 2, right += 2) {
        *left = *right = 0;
        for (channel = svision->m_channel, j = 0; j < 2; j++, channel++) {
            chout[j] = 0;
            if (svision->ch[j].size != 0) {
                if (svision->ch[j].on || channel->count != 0) {
                    BOOL on = 0;
                    switch (svision->ch[j].waveform) {
                        case 0: // 12.5%
                            on = svision->ch[j].pos < (28 * svision->ch[j].size) >> 5;
                            break;
                        case 1: // 25%
                            on = svision->ch[j].pos < (24 * svision->ch[j].size) >> 5;
                            break;
                        case 2: // 50%
                            on = svision->ch[j].pos < svision->ch[j].size / 2;
                            break;
                        case 3: // 75%
                            on = svision->ch[j].pos < svision->ch[j].size / 4;
                            // MESS/MAME:  <= (9 * svision->ch[j].size) >> 5;
                            break;
                    }
                    s = on ? (svision->ch[j].volume)<<2 : 0;
                    s = ((svision->ch_mask>>(3-j))&1)?s:0;
                    if (svision->flags&1) {
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
                svision->ch[j].pos++;
                if (svision->ch[j].pos >= svision->ch[j].size) {
                    svision->ch[j].pos = 0;
                    // Transition from off to on
                    if (channel->on) {
                        memcpy(&svision->ch[j], channel, sizeof(svision->ch[j]));
                        channel->on = 0;
                    }
                }
            }
        }

        if (svision->m_noise.on && (svision->m_noise.play || svision->m_noise.count != 0)) {
            s = (svision->m_noise.value * svision->m_noise.volume) << 2;
            s = svision->ch_mask&1?s:0;
            chout[3] = 0;
            if (svision->m_noise.left) {
                *left += s;
                chout[3] = s;
            }
            if (svision->m_noise.right) {
                *right += s;
                chout[3] = s;
            }
            svision->m_noise.pos += svision->m_noise.step;
            while (svision->m_noise.pos >= 1.0) { // if/while difference - Pacific Battle
                // LFSR: x^2 + x + 1
                uint16 feedback;
                svision->m_noise.value = svision->m_noise.state & 1;
                feedback = ((svision->m_noise.state >> 1) ^ svision->m_noise.state) & 0x0001;
                feedback <<= svision->m_noise.type;
                svision->m_noise.state = (svision->m_noise.state >> 1) | feedback;
                svision->m_noise.pos -= 1.0;
            }
        }

        chout[2] = 0;
        if (svision->m_dma.on) {
            uint8 sample;
            uint16 addr = svision->m_dma.start + (uint16)svision->m_dma.pos / 2;
            if (addr >= 0x8000 && addr < 0xc000) {
                sample = svision->supervision_dma_mem[(addr & 0x3fff) | svision->m_dma.ca14to16];
            }
            if (((uint16)svision->m_dma.pos) & 1)
                s = (sample & 0xf);
            else
                s = (sample & 0xf0) >> 4;
            s <<= 2;
            s = ((svision->ch_mask>>1)&1)?s:0;
            chout[2] = 0;
            if (svision->m_dma.left) {
                *left += s;
                chout[2] = s;
            }
            if (svision->m_dma.right) {
                *right += s;
                chout[2] = s;
            }
            svision->m_dma.pos += svision->m_dma.step;
            if (svision->m_dma.pos >= svision->m_dma.size) {
                svision->m_dma.on = 0;
            }
        }

        if (svision->decrement_tick > SV_DEC_TICK) {
            svision->decrement_tick = 0;
            supervision_sound_decrement(svision);
        }
        svision->decrement_tick++;
    }
}

void supervision_sound_decrement(struct svision_t *svision)
{
    if (svision->m_channel[0].count > 0)
        svision->m_channel[0].count--;
    if (svision->m_channel[1].count > 0)
        svision->m_channel[1].count--;
    if (svision->m_noise.count > 0)
        svision->m_noise.count--;
}

void supervision_sound_wave_write(struct svision_t *svision, int which, int offset, uint8 data)
{
    SVISION_CHANNEL *channel = &svision->m_channel[which];

    channel->reg[offset] = data;
    switch (offset) {
        case 0:
        case 1: {
            uint16 size;
            size = channel->reg[0] | ((channel->reg[1] & 7) << 8);
            // if size == 0 then channel->size == 0
			if (size)
                channel->size = (uint16)(((real)SV_SAMPLE_RATE) * ((real)((size + 1) << 5)) / ((real)svision->UNSCALED_CLOCK));
            else
                channel->size = 0;
            channel->pos = 0;
            // Popo Team
            if (channel->count != 0 || svision->ch[which].size == 0 || channel->size == 0) {
                svision->ch[which].size = channel->size;
                if (channel->count == 0)
                    svision->ch[which].pos = 0;
            }
        }
            break;
        case 2:
            channel->on       =  data & 0x40;
            channel->waveform = (data & 0x30) >> 4;
            channel->volume   =  data & 0x0f;
            if (!channel->on || svision->ch[which].size == 0 || channel->size == 0) {
                uint16 pos = svision->ch[which].pos;
                memcpy(&svision->ch[which], channel, sizeof(svision->ch[which]));
                if (channel->count != 0) // Journey to the West
                    svision->ch[which].pos = pos;
            }
            break;
        case 3:
            channel->count = data + 1;
            svision->ch[which].size = channel->size; // Sonny Xpress!
            break;
    }
}

void supervision_sound_dma_write(struct svision_t *svision, int offset, uint8 data)
{
    svision->m_dma.reg[offset] = data;
    switch (offset) {
        case 0:
        case 1:
            svision->m_dma.start = (svision->m_dma.reg[0] | (svision->m_dma.reg[1] << 8));
            break;
        case 2:
            svision->m_dma.size = (data ? data : 0x100) * 32; // Number of 4-bit samples
            break;
        case 3:
            // Test games: Classic Casino, SSSnake
            svision->m_dma.step = ((real)svision->UNSCALED_CLOCK) / ((real)SV_SAMPLE_RATE * (256 << (data & 3)));
            // MESS/MAME. Wrong
            //svision->m_dma.step  = svision->UNSCALED_CLOCK / (256.0 * SV_SAMPLE_RATE * (1 + (data & 3)));
            svision->m_dma.right = data & 4;
            svision->m_dma.left  = data & 8;
            svision->m_dma.ca14to16 = ((data & 0x70) >> 4) << 14;
            break;
        case 4:
            svision->m_dma.on = data & 0x80;
            if (svision->m_dma.on) {
                svision->m_dma.pos = 0.0;
            }
            break;
    }
}

void supervision_sound_noise_write(struct svision_t *svision, int offset, uint8 data)
{
    svision->m_noise.reg[offset] = data;
    switch (offset) {
        case 0: {
            uint32 divisor = 8 << (data >> 4);
			if (divisor)
                svision->m_noise.step = ((real)svision->UNSCALED_CLOCK) / ((real)SV_SAMPLE_RATE * divisor);
            else
                svision->m_noise.step = 0;

            svision->m_noise.step = ((real)svision->UNSCALED_CLOCK) / ((real)SV_SAMPLE_RATE * divisor);
            svision->m_noise.volume = data & 0xf;
        }
            break;
        case 1:
            svision->m_noise.count = data + 1;
            break;
        case 2:
            svision->m_noise.type  = (data & 1) ? 14 : 6;
            svision->m_noise.play  =  data & 2;
            svision->m_noise.right =  data & 4;
            svision->m_noise.left  =  data & 8;
            svision->m_noise.on    =  data & 0x10; /* honey bee start */
            svision->m_noise.state = 1;
            break;
    }
    svision->m_noise.pos = 0.0;
}
