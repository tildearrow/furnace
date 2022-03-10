// Commander X16 Emulator
// Copyright (c) 2020 Frank van den Hoef
// All rights reserved. License: 2-clause BSD

#include "vera_psg.h"

#include <stdlib.h>
#include <string.h>

enum waveform {
	WF_PULSE = 0,
	WF_SAWTOOTH,
	WF_TRIANGLE,
	WF_NOISE,
};

static uint8_t volume_lut[64] = {0, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8, 9, 9, 10, 11, 11, 12, 13, 14, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 28, 29, 31, 33, 35, 37, 39, 42, 44, 47, 50, 52, 56, 59, 63};

void
psg_reset(struct VERA_PSG* psg)
{
	memset(psg->channels, 0, sizeof(psg->channels));
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
			psg->channels[ch].volume = volume_lut[val & 0x3F];
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
		struct VERAChannel *ch = &psg->channels[i];

		unsigned new_phase = (ch->phase + ch->freq) & 0x1FFFF;
		if ((ch->phase & 0x10000) != (new_phase & 0x10000)) {
			ch->noiseval = rand() & 63;
		}
		ch->phase = new_phase;

		uint8_t v = 0;
		switch (ch->waveform) {
			case WF_PULSE: v = (ch->phase >> 10) > ch->pw ? 0 : 63; break;
			case WF_SAWTOOTH: v = ch->phase >> 11; break;
			case WF_TRIANGLE: v = (ch->phase & 0x10000) ? (~(ch->phase >> 10) & 0x3F) : ((ch->phase >> 10) & 0x3F); break;
			case WF_NOISE: v = ch->noiseval; break;
		}
		int8_t sv = (v ^ 0x20);
		if (sv & 0x20) {
			sv |= 0xC0;
		}

		int val = (int)sv * (int)ch->volume;

		if (ch->left) {
			l += val;
		}
		if (ch->right) {
			r += val;
		}
	}

	*left  = l;
	*right = r;
}

void
psg_render(struct VERA_PSG* psg, int16_t *buf, unsigned num_samples)
{
	while (num_samples--) {
		render(psg, &buf[0], &buf[1]);
		buf += 2;
	}
}
