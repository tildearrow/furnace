#include <string.h>
#include "nextsound.h"

void next_writereg(struct NEXTChip* chip, unsigned char addr, unsigned char val) {
	addr &= 0xf; 
	int ch = addr / 4;
	int idx = addr & 3;
	switch (idx) {
		// FreqLx
		case 0: chip->chan[ch].freq = (chip->chan[ch].freq & 0xff00) | val; break;
		// FreqHx
		case 1: chip->chan[ch].freq = (chip->chan[ch].freq & 0x00ff) | (val << 8); break;
		// ChCTLx
		case 2:
			chip->chan[ch].noise = (val & 0xc0) >> 6;
			chip->chan[ch].vol = (val & 0x1f);
			break;
		// NoResx (writing a 1 to bit 8 here will cause the states to reset)
		case 3:
			chip->chan[ch].lfsr_divider = (val & 0x7f);
			if (val & 0x80) {
				chip->chan[ch].lfsr_state = 1;
				chip->chan[ch].output = 1;
				chip->chan[ch].counter = 0;
				chip->chan[ch].lfsr_counter = 0;
			}
			break;
	}
}

void next_render(struct NEXTChip* chip) {
	int sum = 0;
	for (int i = 0; i < 4; i++) {
		// square wave generator
		chip->chan[i].counter--; // square wave counter
		if (chip->chan[i].counter <= 0) {
			chip->chan[i].counter = chip->chan[i].freq;
			if (chip->chan[i].noise && chip->chan[i].noise != 3) {
				switch (chip->chan[i].noise) {
				case 1:
					chip->chan[i].lfsr_state = (chip->chan[i].lfsr_state >> 1) | (((chip->chan[i].lfsr_state >> 0) ^ (chip->chan[i].lfsr_state >> 3) & 1) << 15);
					chip->chan[i].lfsr_state &= 0xffff;
					chip->chan[i].carrier = chip->chan[i].lfsr_state & 1;
					break;
				case 2:
					chip->chan[i].lfsr_state = (chip->chan[i].lfsr_state >> 1) | ((chip->chan[i].lfsr_state & 1) << 15);
					chip->chan[i].lfsr_state &= 0xffff;
					chip->chan[i].carrier = chip->chan[i].lfsr_state & 1;
					break;
				}
			} else {
				chip->chan[i].carrier ^= 1;
			}
		}
		// noise mode 3 square wave generator
		if (chip->chan[i].noise == 3) {
			chip->chan[i].lfsr_counter--; // noise mode 3 counter (we repurpose the LFSR counter used for divider)
			if (chip->chan[i].lfsr_counter <= 0) {
				chip->chan[i].lfsr_counter = chip->chan[i].lfsr_mode3_sign ? (chip->chan[i].freq - chip->chan[i].lfsr_divider) : (chip->chan[i].freq + chip->chan[i].lfsr_divider);
				//chip->chan[i].lfsr_state = ((chip->chan[i].lfsr_state >> 1) | (chip->chan[i].lfsr_state & 1) << 1); // rather unconventional, but it does work
				chip->chan[i].lfsr_state = chip->chan[i].lfsr_state ^ 1;
			}
			chip->chan[i].modulator = ((chip->chan[i].carrier ^ chip->chan[i].lfsr_state) & 1);
			//modulator = chip->chan[i].lfsr_state;
		}
		// noise divider
		if (chip->chan[i].lfsr_divider && (chip->chan[i].noise != 3)) {
			chip->chan[i].lfsr_counter++;
			if (chip->chan[i].lfsr_counter >= (chip->chan[i].lfsr_divider << 4)) {
				chip->chan[i].lfsr_counter = 0;
				chip->chan[i].lfsr_state = 1;
			}
		}
		// output stage
		chip->chan[i].output = (chip->chan[i].noise == 3) ? chip->chan[i].modulator : chip->chan[i].carrier;
		chip->chan[i].real_output = chip->chan[i].output * (0x7f * NEXT_dac_table[(int)(0x7f*((double)chip->chan[i].vol/31.0f))]);
		sum += chip->chan[i].real_output;
		chip->chan[i].old_modulator = chip->chan[i].modulator;
		chip->chan[i].old_carrier = chip->chan[i].carrier;
	}
	chip->buffer = sum / 4;
}

void next_reset(struct NEXTChip* chip) {
	memset(chip->chan, 0, sizeof(chip->chan));
	for (int i = 0; i < 4; i++) {
		chip->chan[i].lfsr_state = 1;
	}
	chip->buffer = 0;
}