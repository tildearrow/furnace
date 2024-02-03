#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "pwrnoise.h"

#ifdef __cplusplus
extern "C" {
#endif

void pwrnoise_noise_write(noise_channel_t *chan, uint8_t reg, uint8_t val) {
	switch (reg & 0x1f) {
		case 1:
			chan->enable = (val & 0x80) != 0;
			chan->am = (val & 0x02) != 0;
			chan->tapb_enable = (val & 0x01) != 0;
			break;
		case 2:
			chan->period = (chan->period & 0xf00) | val;
			break;
		case 3:
			chan->period = (chan->period & 0xff) | (((uint16_t)val << 8) & 0xf00);
			chan->octave = val >> 4;
			break;
		case 4:
			chan->lfsr = (chan->lfsr & 0xff00) | val;
			break;
		case 5:
			chan->lfsr = (chan->lfsr & 0x00ff) | ((uint16_t)val << 8);
			break;
		case 6:
			chan->tapa = val >> 4;
			chan->tapb = val & 0x0f;
			break;
		case 7:
			chan->vol = val;
			break;
		default: break;
	}
}

void pwrnoise_noise_step(noise_channel_t *chan, uint16_t cycles) {
	if (!chan->enable) {
		chan->out_latch = 0;
		return;
	}
	
	chan->octave_counter += cycles;
	if (((cycles >= 2) && ((cycles >> (chan->octave + 1)) != 0)) || (!(((chan->octave_counter - 1) >> chan->octave) & 0x0001) && ((chan->octave_counter >> chan->octave) & 0x0001))) {
		chan->period_counter += (cycles >> (chan->octave + 1));
		if ((cycles >> (chan->octave + 1)) == 0) ++chan->period_counter;
		
		while (chan->period_counter >= 4096) {
			chan->prev = (uint8_t)(chan->lfsr >> 15);
			uint16_t in = ((chan->lfsr >> chan->tapa) ^ (chan->tapb_enable ? (chan->lfsr >> chan->tapb) : 0)) & 0x0001;
			chan->lfsr = (chan->lfsr << 1) | in;
			chan->period_counter -= 4096 - chan->period;
		}
	}
	
	chan->out_latch = chan->prev ? chan->vol : 0;
}

void pwrnoise_slope_write(slope_channel_t *chan, uint8_t reg, uint8_t val) {
	switch (reg & 0x1f) {
		case 0:
			chan->accum = val & 0x7f;
			break;
		case 1:
			chan->enable = (val & 0x80) != 0;
			if ((val & 0x40) != 0) {
				chan->a = 0;
				chan->b = 0;
				chan->portion = false;
			}
			chan->flags = val & 0x3f;
			break;
		case 2:
			chan->period = (chan->period & 0xf00) | val;
			break;
		case 3:
			chan->period = (chan->period & 0xff) | (((uint16_t)val << 8) & 0xf00);
			chan->octave = val >> 4;
			break;
		case 4:
			chan->alength = val;
			break;
		case 5:
			chan->blength = val;
			break;
		case 6:
			chan->aoffset = val >> 4;
			chan->boffset = val & 0x0f;
			break;
		case 7:
			chan->vol = val;
			break;
		default: break;
	}
}

void pwrnoise_slope_step(slope_channel_t *chan, uint16_t cycles, bool force_zero) {
	if (!chan->enable) {
		chan->out_latch = 0;
		return;
	}
	
	chan->octave_counter += cycles;
	if (((cycles >= 2) && ((cycles >> (chan->octave + 1)) != 0)) || (!(((chan->octave_counter - 1) >> chan->octave) & 0x0001) && ((chan->octave_counter >> chan->octave) & 0x0001))) {
		chan->period_counter += (cycles >> (chan->octave + 1));
		if ((cycles >> (chan->octave + 1)) == 0) ++chan->period_counter;
		
		while (chan->period_counter >= 4096) {
			if (!chan->portion) {
				if ((chan->flags & 0x02) != 0) chan->accum -= chan->aoffset;
				else chan->accum += chan->aoffset;
				
				if ((chan->flags & 0x20) != 0 && chan->accum > 0x7f) chan->accum = (chan->flags & 0x02) ? 0x00 : 0x7f;
				chan->accum &= 0x7f;
				
				if (++chan->a > chan->alength) {
					if ((chan->flags & 0x04) != 0) chan->accum = (chan->flags & 0x01) ? 0x7f : 0x00;
					chan->b = 0x00;
					chan->portion = true;
				}
			}
			else {
				if ((chan->flags & 0x01) != 0) chan->accum -= chan->boffset;
				else chan->accum += chan->boffset;
				
				if ((chan->flags & 0x10) != 0 && chan->accum > 0x7f) chan->accum = (chan->flags & 0x01) ? 0x00 : 0x7f;
				chan->accum &= 0x7f;
				
				if (++chan->b > chan->blength) {
					if ((chan->flags & 0x08) != 0) chan->accum = (chan->flags & 0x02) ? 0x7f : 0x00;
					chan->a = 0x00;
					chan->portion = false;
				}
			}
			
			chan->period_counter -= 4096 - chan->period;
			
			uint8_t left = chan->accum >> 3;
			uint8_t right = chan->accum >> 3;
	
			switch (chan->vol >> 4) {
				case 0:
				case 1:
					left >>= 1;
				case 2:
				case 3:
					left >>= 1;
				case 4:
				case 5:
				case 6:
				case 7:
					left >>= 1;
				default: break;
			}
			switch (chan->vol & 0xf) {
				case 0:
				case 1:
					right >>= 1;
				case 2:
				case 3:
					right >>= 1;
				case 4:
				case 5:
				case 6:
				case 7:
					right >>= 1;
				default: break;
			}
	
			left &= (chan->vol >> 4);
			right &= (chan->vol & 0xf);
			chan->prev = (left << 4) | right;
		}
	}
	
	chan->out_latch = force_zero ? 0 : chan->prev;
}

void pwrnoise_reset(power_noise_t *pn) {
	memset(pn, 0, sizeof(power_noise_t));
}

void pwrnoise_write(power_noise_t *pn, uint8_t reg, uint8_t val) {
	reg &= 0x1f;
	
	if (reg == 0x00) {
		pn->flags = val;
	}
	else if (reg == 0x08 && !(pn->flags & 0x20)) {
		pn->gpioa = val;
	}
	else if (reg == 0x10 && !(pn->flags & 0x40)) {
		pn->gpiob = val;
	}
	else if (reg < 0x08) {
		pwrnoise_noise_write(&pn->n1, reg % 8, val);
	}
	else if (reg < 0x10) {
		pwrnoise_noise_write(&pn->n2, reg % 8, val);
	}
	else if (reg < 0x18) {
		pwrnoise_noise_write(&pn->n3, reg % 8, val);
	}
	else {
		pwrnoise_slope_write(&pn->s, reg % 8, val);
	}
}

void pwrnoise_step(power_noise_t *pn, uint16_t cycles, int16_t *left, int16_t *right) {
	int32_t final_left, final_right;
	
	if ((pn->flags & 0x80) != 0) {
		pwrnoise_noise_step(&pn->n1, cycles);
		pwrnoise_noise_step(&pn->n2, cycles);
		pwrnoise_noise_step(&pn->n3, cycles);
		pwrnoise_slope_step(&pn->s, cycles, (pn->n1.am && !(pn->n1.prev)) || (pn->n2.am && !(pn->n2.prev)) || (pn->n3.am && !(pn->n3.prev)));
		
		final_left = (pn->n1.out_latch >> 4) + (pn->n2.out_latch >> 4) + (pn->n3.out_latch >> 4) + (pn->s.out_latch >> 4);
		final_right = (pn->n1.out_latch & 0xf) + (pn->n2.out_latch & 0xf) + (pn->n3.out_latch & 0xf) + (pn->s.out_latch & 0xf);
	}
	else {
		final_left = 0;
		final_right = 0;
	}
	
	*left = (int16_t)((final_left * 65535 / 63 - 32768) * (pn->flags & 0x7) / 7);
	*right = (int16_t)((final_right * 65535 / 63 - 32768) * (pn->flags & 0x7) / 7);
}

#ifdef __cplusplus
}
#endif
