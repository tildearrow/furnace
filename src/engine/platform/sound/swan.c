/*
WonderSwan sound core

Copyright (c) 2025 Adrian "asie" Siekierka

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include <stdio.h>
#include <string.h>
#include "swan.h"

#define HYPERV_ENABLE 0x0080

#define SND_NOISE_ENABLE 0x10
#define SND_NOISE_RESET  0x08

#define SND_CH1_ENABLE 0x01
#define SND_CH2_ENABLE 0x02
#define SND_CH3_ENABLE 0x04
#define SND_CH4_ENABLE 0x08
#define SND_CH2_VOICE    0x20
#define SND_CH3_SWEEP    0x40
#define SND_CH4_NOISE    0x80

#define SND_OUT_HEADPHONES        0x80
#define SND_OUT_HEADPHONES_ENABLE 0x08
#define SND_OUT_SPEAKER_ENABLE    0x01

#define SND_VOL_CH2_LEFT_HALF  0x08
#define SND_VOL_CH2_LEFT_FULL  0x01
#define SND_VOL_CH2_RIGHT_HALF 0x02
#define SND_VOL_CH2_RIGHT_FULL 0x01

#define SND_TEST_CH_VOICE   0x20
#define SND_TEST_FAST_SWEEP 0x02
#define SND_TEST_HOLD_CH    0x01

static const uint8_t lfsr_tap_bits[8] = { 14, 10, 13, 4, 8, 6, 9, 11 };

void swan_sound_init(swan_sound_t *snd, bool headphones) {
    memset(snd, 0, sizeof(swan_sound_t));

    snd->out_ctrl = headphones ? 0x80 : 0x00;
}

uint8_t swan_sound_in(swan_sound_t *snd, uint16_t port) {
    switch (port & 0x1FF) {
    case 0x6A: return snd->hyper_ctrl;
    case 0x6B: return snd->hyper_ctrl >> 8;
    case 0x80: return snd->frequency[0];
    case 0x81: return snd->frequency[0] >> 8;
    case 0x82: return snd->frequency[1];
    case 0x83: return snd->frequency[1] >> 8;
    case 0x84: return snd->frequency[2];
    case 0x85: return snd->frequency[2] >> 8;
    case 0x86: return snd->frequency[3];
    case 0x87: return snd->frequency[3] >> 8;
    case 0x88: return snd->volume[0];
    case 0x89: return snd->volume[1];
    case 0x8A: return snd->volume[2];
    case 0x8B: return snd->volume[3];
    case 0x8C: return snd->sweep_amount;
    case 0x8D: return snd->sweep_ticks;
    case 0x8E: return snd->noise_ctrl;
    case 0x8F: return snd->wave_address;
    case 0x90: return snd->ch_ctrl;
    case 0x91: return snd->out_ctrl;
    case 0x92: return snd->noise_lfsr;
    case 0x93: return snd->noise_lfsr >> 8;
    case 0x94: return snd->voice_volume;
    case 0x95: return snd->test_flags;
    case 0x96: return snd->synth_output_right;
    case 0x97: return snd->synth_output_right >> 8;
    case 0x98: return snd->synth_output_left;
    case 0x99: return snd->synth_output_left >> 8;
    case 0x9A: return snd->synth_output_mono;
    case 0x9B: return snd->synth_output_mono >> 8;
    default: return 0;
    }
}

void swan_sound_out(swan_sound_t *snd, uint16_t port, uint8_t value) {
    switch (port & 0x1FF) {
    case 0x6A: snd->hyper_ctrl = (snd->hyper_ctrl & 0xFF00) | value; break;
    case 0x6B: snd->hyper_ctrl = (snd->hyper_ctrl & 0x0FFF) | ((value & 0x70) << 8); break;
    case 0x80: snd->frequency[0] = (snd->frequency[0] & 0xFF00) | value; break;
    case 0x81: snd->frequency[0] = (snd->frequency[0] & 0xFF) | ((value & 0x7) << 8); break;
    case 0x82: snd->frequency[1] = (snd->frequency[1] & 0xFF00) | value; break;
    case 0x83: snd->frequency[1] = (snd->frequency[1] & 0xFF) | ((value & 0x7) << 8); break;
    case 0x84: snd->frequency[2] = (snd->frequency[2] & 0xFF00) | value; break;
    case 0x85: snd->frequency[2] = (snd->frequency[2] & 0xFF) | ((value & 0x7) << 8); break;
    case 0x86: snd->frequency[3] = (snd->frequency[3] & 0xFF00) | value; break;
    case 0x87: snd->frequency[3] = (snd->frequency[3] & 0xFF) | ((value & 0x7) << 8); break;
    case 0x88: snd->volume[0] = value; break;
    case 0x89: snd->volume[1] = value; break;
    case 0x8A: snd->volume[2] = value; break;
    case 0x8B: snd->volume[3] = value; break;
    case 0x8C: snd->sweep_amount = value; break;
    case 0x8D: snd->sweep_ticks = value & 0x1F; break;
    case 0x8E:
        snd->noise_ctrl = value & 0x17;
        if (value & SND_NOISE_RESET) {
            snd->noise_lfsr = 0;
        }
        break;
    case 0x8F: snd->wave_address = value; break;
    case 0x90: snd->ch_ctrl = value; break;
    case 0x91: snd->out_ctrl = (snd->out_ctrl & 0x80) | (value & 0x0F); break;
    case 0x94: snd->voice_volume = value; break;
    case 0x95: snd->test_flags = value; break;
    }
}

void swan_sound_tick(swan_sound_t *snd, uint32_t cycles) {
    for (int ch = 0; ch < 4; ch++) {
        if (snd->ch_ctrl & (1 << ch)) {
            snd->period_counter[ch] += cycles;
            uint32_t step = 2048 - snd->frequency[ch];
            while (snd->period_counter[ch] >= step) {
                snd->sample_index[ch] = (snd->sample_index[ch] + 1) & 0x1F;
                snd->period_counter[ch] -= step;

                // Update noise counter
                if (ch == 3 && (snd->noise_ctrl & SND_NOISE_ENABLE)) {
                    uint8_t lfsr_new = (1 ^ (snd->noise_lfsr >> 7) ^ (snd->noise_lfsr >> lfsr_tap_bits[snd->noise_ctrl & 7])) & 0x1;
                    snd->noise_lfsr = (snd->noise_lfsr << 1) | lfsr_new;
                }
            }
        }
    }

    if (snd->ch_ctrl & SND_CH3_SWEEP) {
        snd->sweep_counter += cycles;
        uint32_t step = ((snd->test_flags & SND_TEST_FAST_SWEEP) ? 1 : 8192) * (snd->sweep_ticks + 1);
        while (snd->sweep_counter >= step) {
            snd->frequency[2] = (snd->frequency[2] + snd->sweep_amount) & 0x7FF;
            snd->sweep_counter -= step;
        }
    }
}

static inline void sample_render_channel(swan_sound_t *snd, uint8_t ch, uint8_t sample) {
    snd->ch_output_right[ch] = sample * (snd->volume[ch] & 0xF);
    snd->ch_output_left[ch]  = sample * (snd->volume[ch] >> 4);
}

static inline void wavetable_render_channel(swan_sound_t *snd, uint8_t ch) {
    uint8_t index = snd->sample_index[ch];
    uint8_t addr = (ch << 4) | (index >> 1);
    sample_render_channel(snd, ch, (index & 1) ? (snd->wave_ram[addr] >> 4) : (snd->wave_ram[addr] & 0xF));
}

static void voice_render_channel2(swan_sound_t *snd) {
    if (snd->voice_volume & SND_VOL_CH2_RIGHT_FULL) {
        snd->ch_output_right[1] = snd->volume[1];
    } else if (snd->voice_volume & SND_VOL_CH2_RIGHT_HALF) {
        snd->ch_output_right[1] = snd->volume[1] >> 1;
    }
    if (snd->voice_volume & SND_VOL_CH2_LEFT_FULL) {
        snd->ch_output_left[1] = snd->volume[1];
    } else if (snd->voice_volume & SND_VOL_CH2_LEFT_HALF) {
        snd->ch_output_left[1] = snd->volume[1] >> 1;
    }
}

// See https://ws.nesdev.org/wiki/Sound
void swan_sound_sample(swan_sound_t *snd) {
    // Calculate synthesizer channels
    if (!(snd->test_flags & SND_TEST_HOLD_CH)) {
        for (int i = 0; i < 4; i++) {
            snd->ch_output_left[i] = 0;
            snd->ch_output_right[i] = 0;
        }

        if (snd->ch_ctrl & SND_CH1_ENABLE) {
            wavetable_render_channel(snd, 0);
        }

        if (snd->ch_ctrl & SND_CH2_ENABLE) {
            if (snd->ch_ctrl & SND_CH2_VOICE) {
                voice_render_channel2(snd);
            } else {
                wavetable_render_channel(snd, 1);
            }
        }

        if (snd->ch_ctrl & SND_CH3_ENABLE) {
            wavetable_render_channel(snd, 2);
        }

        if (snd->ch_ctrl & SND_CH4_ENABLE) {
            if (snd->ch_ctrl & SND_CH4_NOISE) {
                sample_render_channel(snd, 3, (snd->noise_lfsr & 1) * 15);
            } else {
                wavetable_render_channel(snd, 3);
            }
        }
    }

    // Sum synthesizer channels
    if (snd->test_flags & SND_TEST_CH_VOICE) {
        if (snd->ch_ctrl & SND_CH2_VOICE) {
            snd->synth_output_left = snd->ch_output_left[1] * 5;
            snd->synth_output_right = snd->ch_output_right[1] * 5;
        } else {
            snd->synth_output_left = 0;
            snd->synth_output_right = 0;
        }
    } else {
        snd->synth_output_left =
            snd->ch_output_left[0] +
            snd->ch_output_left[1] +
            snd->ch_output_left[2] +
            snd->ch_output_left[3];

        snd->synth_output_right =
            snd->ch_output_right[0] +
            snd->ch_output_right[1] +
            snd->ch_output_right[2] +
            snd->ch_output_right[3];
    }

    // Mix speaker output
    snd->synth_output_mono = (snd->synth_output_left & 0x3FF) + (snd->synth_output_right & 0x3FF);
    if (snd->out_ctrl & SND_OUT_SPEAKER_ENABLE) {
        snd->output_speaker = (snd->synth_output_mono >> ((snd->out_ctrl >> 1) & 3)) & 0xFF;
    } else {
        snd->output_speaker = 0;
    }

    // Mix headphone output
    if (snd->out_ctrl & SND_OUT_HEADPHONES_ENABLE) {
        snd->output_left = snd->synth_output_left << 5;
        snd->output_right = snd->synth_output_right << 5;
        if (snd->hyper_ctrl & HYPERV_ENABLE) {
            snd->output_left += snd->hyper_output_left;
            snd->output_right += snd->hyper_output_right;
        }
    } else {
        snd->output_left = 0;
        snd->output_right = 0;
    }
}
