/*
WonderSwan sound core

Note: Neither Sound DMA nor Hyper Voice DMA is implemented.

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

#ifndef _DRIVER_SWAN_H
#define _DRIVER_SWAN_H

#include <stdbool.h>
#include <stdint.h>

typedef struct swan_sound {
   // Ports
   uint16_t frequency[4];
   uint8_t volume[4];
   int8_t sweep_amount;
   uint8_t sweep_ticks;
   uint8_t noise_ctrl;
   uint8_t wave_address;
   uint8_t ch_ctrl;
   uint8_t out_ctrl;
   uint16_t noise_lfsr;
   uint8_t voice_volume;
   // TODO: Implement test flag bits 7, 6, 4, 3, 2
   uint8_t test_flags;

   uint8_t wave_ram[64];

   uint16_t hyper_ctrl;

   // State
   uint8_t sample_index[4];
   uint32_t period_counter[4];
   uint32_t sweep_counter;

   // Outputs
   /// Individual channel outputs (range: 0 .. 255)
   int16_t ch_output_right[4];
   int16_t ch_output_left[4];

   /// Hyper Voice outputs (range: -32768 .. 32767)
   int16_t hyper_output_left;
   int16_t hyper_output_right;

   /// Stereo synth outputs (range: 0 .. 1023)
   uint16_t synth_output_right;
   uint16_t synth_output_left;

   /// Mono synth output (range: 0 .. 2047)
   uint16_t synth_output_mono;
 
   /// Headphones output (range: -32768 .. 32767)
   int16_t output_right;
   int16_t output_left;

   /// Internal speaker output (range: 0 .. 255)
   uint8_t output_speaker;
} swan_sound_t;

#ifdef __cplusplus
extern "C" {
#endif

void swan_sound_init(swan_sound_t *snd, bool headphones);
uint8_t swan_sound_in(swan_sound_t *snd, uint16_t port);
void swan_sound_out(swan_sound_t *snd, uint16_t port, uint8_t value);
void swan_sound_tick(swan_sound_t *snd);

#ifdef __cplusplus
}
#endif

#endif
