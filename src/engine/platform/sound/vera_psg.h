// Commander X16 Emulator
// Copyright (c) 2020 Frank van den Hoef
// All rights reserved. License: 2-clause BSD

#pragma once

#include <stdint.h>
#include <stdbool.h>

struct VERAChannel {
	uint16_t freq;
	uint16_t volume;
	bool     left, right;
	uint8_t  pw;
	uint8_t  waveform;

	unsigned phase;
  int lastOut;
	uint8_t  noiseval;
};

struct VERA_PSG {
  unsigned int chipType, noiseState, noiseOut;
  struct VERAChannel channels[16];
};

void psg_reset(struct VERA_PSG* psg);
void psg_writereg(struct VERA_PSG* psg, uint8_t reg, uint8_t val);
void psg_render(struct VERA_PSG* psg, int16_t *bufL, int16_t *bufR, unsigned num_samples);
