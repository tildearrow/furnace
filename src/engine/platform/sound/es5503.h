// license:BSD-3-Clause
// copyright-holders:R. Belmont

#pragma once

#include <stdint.h>

class es5503_core {
public:
	// construction/destruction
	es5503_core();
    ~es5503_core();

	// channels must be a power of two
	void set_channels(int channels) { output_channels = channels; }

	uint8_t read(uint8_t offset);
	void write(uint8_t offset, uint8_t data);

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

	//void halt_osc(int onum, int type, uint32_t *accumulator, int resshift);
};