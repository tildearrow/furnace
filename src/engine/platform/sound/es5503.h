// license:BSD-3-Clause
// copyright-holders:R. Belmont

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>

#include "../../dispatch.h"

class es5503_core {
public:
	// construction/destruction
	void es5503_core_init(uint32_t clock, DivDispatchOscBuffer** oscBuf, uint8_t oscsenabled);
    void es5503_core_free();
	void update_num_osc(DivDispatchOscBuffer** oscBuf, uint8_t oscsenabled);

	// channels must be a power of two
	void set_channels(int channels) { output_channels = channels; }

	uint8_t read(uint8_t offset);
	void write(uint8_t offset, uint8_t data);
	uint8_t read_byte(uint32_t offset);
	void halt_osc(int onum, int type, uint32_t *accumulator, int resshift);
	void fill_audio_buffer(short** buf, size_t len);

	unsigned char* sampleMem;
	//unsigned char sampleMem[65536 * 2];
    size_t sampleMemLen;

	uint32_t clock;
	uint8_t  oscsenabled;      // # of oscillators enabled

	bool mono;

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
	int   rege0;            // contents of register 0xe0

	uint8_t m_channel_strobe;

	int output_channels;
	uint32_t output_rate;

	DivDispatchOscBuffer* oscBuf[32];

	short my_buf[8][4096 * 2];

	void put_in_buffer(int32_t value, uint32_t pos, uint32_t chan);
};