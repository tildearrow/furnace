// Commander X16 Emulator
// Copyright (c) 2020 Frank van den Hoef
// All rights reserved. License: 2-clause BSD

#include "vera_pcm.h"
#include <stdio.h>

static uint8_t volume_lut[16] = {0, 1, 2, 3, 4, 5, 6, 8, 11, 14, 18, 23, 30, 38, 49, 64};

static void
fifo_reset(struct VERA_PCM* pcm)
{
	pcm->fifo_wridx = 0;
	pcm->fifo_rdidx = 0;
	pcm->fifo_cnt   = 0;
}

void
pcm_reset(struct VERA_PCM* pcm)
{
	fifo_reset(pcm);
	pcm->ctrl  = 0;
	pcm->rate  = 0;
	pcm->cur_l = 0;
	pcm->cur_r = 0;
	pcm->phase = 0;
}

void
pcm_write_ctrl(struct VERA_PCM* pcm, uint8_t val)
{
	if (val & 0x80) {
		fifo_reset(pcm);
	}

	pcm->ctrl = val & 0x3F;
}

uint8_t
pcm_read_ctrl(struct VERA_PCM* pcm)
{
	uint8_t result = pcm->ctrl;
	if (pcm->fifo_cnt == sizeof(pcm->fifo)) {
		result |= 0x80;
	}
	return result;
}

void
pcm_write_rate(struct VERA_PCM* pcm, uint8_t val)
{
	pcm->rate = val;
}

uint8_t
pcm_read_rate(struct VERA_PCM* pcm)
{
	return pcm->rate;
}

void
pcm_write_fifo(struct VERA_PCM* pcm, uint8_t val)
{
	if (pcm->fifo_cnt < sizeof(pcm->fifo)) {
		pcm->fifo[pcm->fifo_wridx++] = val;
		if (pcm->fifo_wridx == sizeof(pcm->fifo)) {
			pcm->fifo_wridx = 0;
		}
		pcm->fifo_cnt++;
	}
}

static uint8_t
read_fifo(struct VERA_PCM* pcm)
{
	if (pcm->fifo_cnt == 0) {
		return 0;
	}
	uint8_t result = pcm->fifo[pcm->fifo_rdidx++];
	if (pcm->fifo_rdidx == sizeof(pcm->fifo)) {
		pcm->fifo_rdidx = 0;
	}
	pcm->fifo_cnt--;
	return result;
}

bool
pcm_is_fifo_almost_empty(struct VERA_PCM* pcm)
{
	return pcm->fifo_cnt < 1024;
}

void
pcm_render(struct VERA_PCM* pcm, int16_t* buf_l, int16_t* buf_r, unsigned num_samples)
{
	while (num_samples--) {
		uint8_t old_phase = pcm->phase;
		pcm->phase += pcm->rate;
		if ((old_phase & 0x80) != (pcm->phase & 0x80)) {
			switch ((pcm->ctrl >> 4) & 3) {
				case 0: { // mono 8-bit
					pcm->cur_l = (int16_t)read_fifo(pcm) << 8;
					pcm->cur_r = pcm->cur_l;
					break;
				}
				case 1: { // stereo 8-bit
					pcm->cur_l = read_fifo(pcm) << 8;
					pcm->cur_r = read_fifo(pcm) << 8;
					break;
				}
				case 2: { // mono 16-bit
					pcm->cur_l = read_fifo(pcm);
					pcm->cur_l |= read_fifo(pcm) << 8;
					pcm->cur_r = pcm->cur_l;
					break;
				}
				case 3: { // stereo 16-bit
					pcm->cur_l = read_fifo(pcm);
					pcm->cur_l |= read_fifo(pcm) << 8;
					pcm->cur_r = read_fifo(pcm);
					pcm->cur_r |= read_fifo(pcm) << 8;
					break;
				}
			}
		}

		*(buf_l++) = ((int)pcm->cur_l * (int)volume_lut[pcm->ctrl & 0xF]) >> 6;
		*(buf_r++) = ((int)pcm->cur_r * (int)volume_lut[pcm->ctrl & 0xF]) >> 6;
	}
}
