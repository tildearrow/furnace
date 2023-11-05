// license:BSD-3-Clause
// copyright-holders:R. Belmont

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>

class es5503_core {
public:
	// construction/destruction
	es5503_core(uint32_t clock);
    ~es5503_core();

	// channels must be a power of two
	void set_channels(int channels) { output_channels = channels; }

	uint8_t read(uint8_t offset);
	void write(uint8_t offset, uint8_t data);
	uint8_t read_byte(uint32_t offset);
	void halt_osc(int onum, int type, uint32_t *accumulator, int resshift);
	void fill_audio_buffer(short* left, short* right, size_t len);

	unsigned char* sampleMem;
    size_t sampleMemLen;

private:
	enum
	{
		MODE_FREE = 0,
		MODE_ONESHOT = 1,
		MODE_SYNCAM = 2,
		MODE_SWAP = 3
	};

	struct ES5503Osc
	{
		uint16_t freq;
		uint16_t wtsize;
		uint8_t  control;
		uint8_t  vol;
		uint8_t  data;
		uint32_t wavetblpointer;
		uint8_t  wavetblsize;
		uint8_t  resolution;

		uint32_t accumulator;
		uint8_t  irqpend;
	};

	ES5503Osc oscillators[32];

	int8_t  oscsenabled;      // # of oscillators enabled
	int   rege0;            // contents of register 0xe0

	uint8_t m_channel_strobe;

	int output_channels;
	uint32_t output_rate;

	std::vector<int32_t> m_mix_buffer;

	//void halt_osc(int onum, int type, uint32_t *accumulator, int resshift);
};