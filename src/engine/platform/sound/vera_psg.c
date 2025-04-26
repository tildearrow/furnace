// Commander X16 Emulator
// Copyright (c) 2020 Frank van den Hoef
// All rights reserved. License: 2-clause BSD

// Chip revisions
// 0: V  0.3.0
// 1: V 47.0.0 (9-bit volume, phase reset on mute)
// 2: V 47.0.2 (Pulse Width XOR on Saw and Triangle)

#include "vera_psg.h"

#include <stdlib.h>
#include <string.h>

enum waveform {
	WF_PULSE = 0,
	WF_SAWTOOTH,
	WF_TRIANGLE,
	WF_NOISE,
};

static uint16_t volume_lut[64] = {0, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8, 9, 9, 10, 11, 11, 12, 13, 14, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 28, 29, 31, 33, 35, 37, 39, 42, 44, 47, 50, 52, 56, 59, 63};
static uint16_t volume_lut_47[64] = {
	  0,                                           4,   8,  12,
	 16,  17,  18,  20,  21,  22,  23,  25,  26,  28,  30,  31,
	 33,  35,  37,  40,  42,  45,  47,  50,  53,  56,  60,  63,
	 67,  71,  75,  80,  85,  90,  95, 101, 107, 113, 120, 127,
	135, 143, 151, 160, 170, 180, 191, 202, 214, 227, 241, 255,
	270, 286, 303, 321, 341, 361, 382, 405, 429, 455, 482, 511
};

void
psg_reset(struct VERA_PSG* psg)
{
	memset(psg->channels, 0, sizeof(psg->channels));
	psg->noiseOut = 0;
	psg->noiseState = 1;
}

void
psg_writereg(struct VERA_PSG* psg, uint8_t reg, uint8_t val)
{
	reg &= 0x3f;

	int ch  = reg / 4;
	int idx = reg & 3;

	switch (idx) {
		case 0: psg->channels[ch].freq = (psg->channels[ch].freq & 0xFF00) | val; break;
		case 1: psg->channels[ch].freq = (psg->channels[ch].freq & 0x00FF) | (val << 8); break;
		case 2: {
			psg->channels[ch].right  = (val & 0x80) != 0;
			psg->channels[ch].left   = (val & 0x40) != 0;
			psg->channels[ch].volume = ((psg->chipType < 1) ? volume_lut : volume_lut_47)[val & 0x3F];
			break;
		}
		case 3: {
			psg->channels[ch].pw       = val & 0x3F;
			psg->channels[ch].waveform = val >> 6;
			break;
		}
	}
}

static inline void
render(struct VERA_PSG* psg, int16_t *left, int16_t *right)
{
	int l = 0;
	int r = 0;

	for (int i = 0; i < 16; i++) {
		// In FPGA implementation, noise values are generated every system clock and
		// the channel update is run sequentially. So, even if both two channels are
		// fetching a noise value in the same sample, they should have different values
		psg->noiseOut = ((psg->noiseOut << 1) | (psg->noiseState & 1)) & 63;
		psg->noiseState = (psg->noiseState << 1) | (((psg->noiseState >> 1) ^ (psg->noiseState >> 2) ^ (psg->noiseState >> 4) ^ (psg->noiseState >> 15)) & 1);

		struct VERAChannel *ch = &psg->channels[i];

		unsigned new_phase = (ch->phase + ch->freq) & 0x1FFFF;
		if ((psg->chipType >= 1) && (!ch->left && !ch->right)) {
			new_phase = 0;
		}
		if ((psg->chipType < 3) ? (ch->phase & 0x10000) != (new_phase & 0x10000) : (ch->phase & 0x10000) && !(new_phase & 0x10000)) {
			ch->noiseval = (psg->chipType < 1) ? psg->noiseOut : (psg->noiseState >> 1) & 0x3f;
		}
		ch->phase = new_phase;

		uint8_t v = 0;
		switch (ch->waveform) {
			case WF_PULSE: v = (ch->phase >> 10) > ch->pw ? 0 : 63; break;
			case WF_SAWTOOTH: v = (ch->phase >> 11) ^ (psg->chipType < 2 ? 0 : (ch->pw ^ 0x3f) & 0x3f); break;
			case WF_TRIANGLE: v = ((ch->phase & 0x10000) ? (~(ch->phase >> 10) & 0x3F) : ((ch->phase >> 10) & 0x3F)) ^ (psg->chipType < 2 ? 0 : (ch->pw ^ 0x3f) & 0x3f); break;
			case WF_NOISE: v = ch->noiseval; break;
		}
		int8_t sv = (v ^ 0x20);
		if (sv & 0x20) {
			sv |= 0xC0;
		}

		int val = (int)sv * (int)ch->volume;

		if (ch->left) {
			l += (psg->chipType < 1) ? val : val >> 3;
		}
		if (ch->right) {
			r += (psg->chipType < 1) ? val : val >> 3;
		}

		if (ch->left || ch->right) {
			ch->lastOut = (psg->chipType < 1) ? val << 3 : val;
		} else {
			ch->lastOut = 0;
		}
	}

	*left  = l;
	*right = r;
}

void
psg_render(struct VERA_PSG* psg, int16_t *bufL, int16_t *bufR, unsigned num_samples)
{
	while (num_samples--) {
		render(psg, bufL, bufR);
		bufL++;
		bufR++;
	}
}
