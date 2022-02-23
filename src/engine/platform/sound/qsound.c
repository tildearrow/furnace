/*

	Capcom DL-1425 QSound emulator
	==============================

	by superctr (Ian Karlsson)
	with thanks to Valley Bell

	2018-05-12 - 2018-05-15

*/

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "qsound.h"

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

// ============================================================================

static const int16_t qsound_dry_mix_table[33] = {
	-16384,-16384,-16384,-16384,-16384,-16384,-16384,-16384,
	-16384,-16384,-16384,-16384,-16384,-16384,-16384,-16384,
	-16384,-14746,-13107,-11633,-10486,-9175,-8520,-7209,
	-6226,-5226,-4588,-3768,-3277,-2703,-2130,-1802,
	0
};

static const int16_t qsound_wet_mix_table[33] = {
	0,-1638,-1966,-2458,-2949,-3441,-4096,-4669,
	-4915,-5120,-5489,-6144,-7537,-8831,-9339,-9830,
	-10240,-10322,-10486,-10568,-10650,-11796,-12288,-12288,
	-12534,-12648,-12780,-12829,-12943,-13107,-13418,-14090,
	-16384
};

static const int16_t qsound_linear_mix_table[33] = {
	-16379,-16338,-16257,-16135,-15973,-15772,-15531,-15251,
	-14934,-14580,-14189,-13763,-13303,-12810,-12284,-11729,
	-11729,-11144,-10531,-9893,-9229,-8543,-7836,-7109,
	-6364,-5604,-4829,-4043,-3246,-2442,-1631,-817,
	0
};

static const int16_t qsound_filter_data[5][95] = {
	{	// d53 - 0
		0,0,0,6,44,-24,-53,-10,59,-40,-27,1,39,-27,56,127,174,36,-13,49,
		212,142,143,-73,-20,66,-108,-117,-399,-265,-392,-569,-473,-71,95,-319,-218,-230,331,638,
		449,477,-180,532,1107,750,9899,3828,-2418,1071,-176,191,-431,64,117,-150,-274,-97,-238,165,
		166,250,-19,4,37,204,186,-6,140,-77,-1,1,18,-10,-151,-149,-103,-9,55,23,
		-102,-97,-11,13,-48,-27,5,18,-61,-30,64,72,0,0,0,
	},
	{	// db2 - 1 - default left filter
		0,0,0,85,24,-76,-123,-86,-29,-14,-20,-7,6,-28,-87,-89,-5,100,154,160,
		150,118,41,-48,-78,-23,59,83,-2,-176,-333,-344,-203,-66,-39,2,224,495,495,280,
		432,1340,2483,5377,1905,658,0,97,347,285,35,-95,-78,-82,-151,-192,-171,-149,-147,-113,
		-22,71,118,129,127,110,71,31,20,36,46,23,-27,-63,-53,-21,-19,-60,-92,-69,
		-12,25,29,30,40,41,29,30,46,39,-15,-74,0,0,0,
	},
	{	// e11 - 2 - default right filter
		0,0,0,23,42,47,29,10,2,-14,-54,-92,-93,-70,-64,-77,-57,18,94,113,
		87,69,67,50,25,29,58,62,24,-39,-131,-256,-325,-234,-45,58,78,223,485,496,
		127,6,857,2283,2683,4928,1328,132,79,314,189,-80,-90,35,-21,-186,-195,-99,-136,-258,
		-189,82,257,185,53,41,84,68,38,63,77,14,-60,-71,-71,-120,-151,-84,14,29,
		-8,7,66,69,12,-3,54,92,52,-6,-15,-2,0,0,0,
	},
	{	// e70 - 3
		0,0,0,2,-28,-37,-17,0,-9,-22,-3,35,52,39,20,7,-6,2,55,121,
		129,67,8,1,9,-6,-16,16,66,96,118,130,75,-47,-92,43,223,239,151,219,
		440,475,226,206,940,2100,2663,4980,865,49,-33,186,231,103,42,114,191,184,116,29,
		-47,-72,-21,60,96,68,31,32,63,87,76,39,7,14,55,85,67,18,-12,-3,
		21,34,29,6,-27,-49,-37,-2,16,0,-21,-16,0,0,0,
	},
	{	// ecf - 4
		0,0,0,48,7,-22,-29,-10,24,54,59,29,-36,-117,-185,-213,-185,-99,13,90,
		83,24,-5,23,53,47,38,56,67,57,75,107,16,-242,-440,-355,-120,-33,-47,152,
		501,472,-57,-292,544,1937,2277,6145,1240,153,47,200,152,36,64,134,74,-82,-208,-266,
		-268,-188,-42,65,74,56,89,133,114,44,-3,-1,17,29,29,-2,-76,-156,-187,-151,
		-85,-31,-5,7,20,32,24,-5,-20,6,48,62,0,0,0,
	}
};

static const int16_t qsound_filter_data2[209] = {
	// f2e - following 95 values used for "disable output" filter
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,

	// f73 - following 45 values used for "mode 2" filter (overlaps with f2e)
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,
	-371,-196,-268,-512,-303,-315,-184,-76,276,-256,298,196,990,236,1114,-126,4377,6549,791,

	// fa0 - filtering disabled (for 95-taps) (use fa3 or fa4 for mode2 filters)
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,-16384,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static const int16_t adpcm_step_table[16] = {
	154, 154, 128, 102, 77, 58, 58, 58,
	58, 58, 58, 58, 77, 102, 128, 154
};

// DSP states
enum {
	STATE_INIT1		= 0x288,
	STATE_INIT2		= 0x61a,
	STATE_REFRESH1	= 0x039,
	STATE_REFRESH2	= 0x04f,
	STATE_NORMAL1	= 0x314,
	STATE_NORMAL2 	= 0x6b2,
};

enum {
	PANTBL_LEFT		= 0,
	PANTBL_RIGHT	= 1,
	PANTBL_DRY		= 0,
	PANTBL_WET		= 1,
};

static void init_pan_tables(struct qsound_chip *chip);
static void init_register_map(struct qsound_chip *chip);

static void state_init(struct qsound_chip *chip);
static void state_refresh_filter_1(struct qsound_chip *chip);
static void state_refresh_filter_2(struct qsound_chip *chip);
static void state_normal_update(struct qsound_chip *chip);

static inline int16_t get_sample(struct qsound_chip *chip, uint16_t bank,uint16_t address);
static inline int16_t* get_filter_table(struct qsound_chip *chip, uint16_t offset);
static inline int16_t pcm_update(struct qsound_chip *chip, int voice_no, int32_t *echo_out);
static inline void adpcm_update(struct qsound_chip *chip, int voice_no, int nibble);
static inline int16_t echo(struct qsound_echo *r,int32_t input);
static inline int32_t fir(struct qsound_fir *f, int16_t input);
static inline int32_t delay(struct qsound_delay *d, int32_t input);
static inline void delay_update(struct qsound_delay *d);

// ============================================================================

long qsound_start(struct qsound_chip *chip, int clock)
{
	memset(chip,0,sizeof(*chip));

	init_pan_tables(chip);
	init_register_map(chip);

	return clock / 2 / 1248; // DSP program uses 1248 machine cycles per iteration
}

void qsound_reset(struct qsound_chip *chip)
{
	chip->ready_flag = 0;
	chip->out[0] = chip->out[1] = 0;
	chip->state = 0;
	chip->state_counter = 0;
}

uint8_t qsound_stream_update(struct qsound_chip *chip, int16_t **outputs, int samples)
{
	// Clear the buffers
	memset(outputs[0], 0, samples * sizeof(*outputs[0]));
	memset(outputs[1], 0, samples * sizeof(*outputs[1]));

	for (int i = 0; i < samples; i ++)
	{
		qsound_update(chip);
		outputs[0][i] = chip->out[0];
		outputs[1][i] = chip->out[1];
	}
  return 0;
}

uint8_t qsound_w(struct qsound_chip *chip, uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			chip->data_latch = (chip->data_latch & 0x00ff) | (data << 8);
			break;
		case 1:
			chip->data_latch = (chip->data_latch & 0xff00) | data;
			break;
		case 2:
			qsound_write_data(chip, data, chip->data_latch);
			break;
		default:
			break;
	}
  return 0;
}

uint8_t qsound_r(struct qsound_chip *chip)
{
	// ready bit (0x00 = busy, 0x80 == ready)
	return chip->ready_flag;
}

void qsound_write_data(struct qsound_chip *chip, uint8_t address, uint16_t data)
{
	uint16_t *destination = chip->register_map[address];
	if(destination)
		*destination = data;
	chip->ready_flag = 0;
}

uint16_t qsound_read_data(struct qsound_chip *chip, uint8_t address)
{
	uint16_t data = 0;
	uint16_t *source = chip->register_map[address];
	if(source)
		data = *source;
	return data;
}

// ============================================================================

static void init_pan_tables(struct qsound_chip *chip)
{
	int i;
	for(i=0;i<33;i++)
	{
		// dry mixing levels
		chip->pan_tables[PANTBL_LEFT][PANTBL_DRY][i] = qsound_dry_mix_table[i];
		chip->pan_tables[PANTBL_RIGHT][PANTBL_DRY][i] = qsound_dry_mix_table[32-i];
		// wet mixing levels
		chip->pan_tables[PANTBL_LEFT][PANTBL_WET][i] = qsound_wet_mix_table[i];
		chip->pan_tables[PANTBL_RIGHT][PANTBL_WET][i] = qsound_wet_mix_table[32-i];
		// linear panning, only for dry component. wet component is muted.
		chip->pan_tables[PANTBL_LEFT][PANTBL_DRY][i+0x30] = qsound_linear_mix_table[i];
		chip->pan_tables[PANTBL_RIGHT][PANTBL_DRY][i+0x30] = qsound_linear_mix_table[32-i];
	}
}

static void init_register_map(struct qsound_chip *chip)
{
	int i;

	// unused registers
	for(i=0;i<256;i++)
		chip->register_map[i] = NULL;

	// PCM registers
	for(i=0;i<16;i++) // PCM voices
	{
		chip->register_map[(i<<3)+0] = (uint16_t*)&chip->voice[(i+1)%16].bank; // Bank applies to the next channel
		chip->register_map[(i<<3)+1] = (uint16_t*)&chip->voice[i].addr; // Current sample position and start position.
		chip->register_map[(i<<3)+2] = (uint16_t*)&chip->voice[i].rate; // 4.12 fixed point decimal.
		chip->register_map[(i<<3)+3] = (uint16_t*)&chip->voice[i].phase;
		chip->register_map[(i<<3)+4] = (uint16_t*)&chip->voice[i].loop_len;
		chip->register_map[(i<<3)+5] = (uint16_t*)&chip->voice[i].end_addr;
		chip->register_map[(i<<3)+6] = (uint16_t*)&chip->voice[i].volume;
		chip->register_map[(i<<3)+7] = NULL;	// unused
		chip->register_map[i+0x80] = (uint16_t*)&chip->voice_pan[i];
		chip->register_map[i+0xba] = (uint16_t*)&chip->voice[i].echo;
	}

	// ADPCM registers
	for(i=0;i<3;i++) // ADPCM voices
	{
		// ADPCM sample rate is fixed to 8khz. (one channel is updated every third sample)
		chip->register_map[(i<<2)+0xca] = (uint16_t*)&chip->adpcm[i].start_addr;
		chip->register_map[(i<<2)+0xcb] = (uint16_t*)&chip->adpcm[i].end_addr;
		chip->register_map[(i<<2)+0xcc] = (uint16_t*)&chip->adpcm[i].bank;
		chip->register_map[(i<<2)+0xcd] = (uint16_t*)&chip->adpcm[i].volume;
		chip->register_map[i+0xd6] = (uint16_t*)&chip->adpcm[i].flag; // non-zero to start ADPCM playback
		chip->register_map[i+0x90] = (uint16_t*)&chip->voice_pan[16+i];
	}

	// QSound registers
	chip->register_map[0x93] = (uint16_t*)&chip->echo.feedback;
	chip->register_map[0xd9] = (uint16_t*)&chip->echo.end_pos;
	chip->register_map[0xe2] = (uint16_t*)&chip->delay_update; // non-zero to update delays
	chip->register_map[0xe3] = (uint16_t*)&chip->next_state;
	for(i=0;i<2;i++)  // left, right
	{
		// Wet
		chip->register_map[(i<<1)+0xda] = (uint16_t*)&chip->filter[i].table_pos;
		chip->register_map[(i<<1)+0xde] = (uint16_t*)&chip->wet[i].delay;
		chip->register_map[(i<<1)+0xe4] = (uint16_t*)&chip->wet[i].volume;
		// Dry
		chip->register_map[(i<<1)+0xdb] = (uint16_t*)&chip->alt_filter[i].table_pos;
		chip->register_map[(i<<1)+0xdf] = (uint16_t*)&chip->dry[i].delay;
		chip->register_map[(i<<1)+0xe5] = (uint16_t*)&chip->dry[i].volume;
	}
}

static inline int16_t get_sample(struct qsound_chip *chip, uint16_t bank,uint16_t address)
{
	uint32_t rom_addr;
	uint8_t sample_data;

	if (! chip->rom_mask)
		return 0;	// no ROM loaded
	if (! (bank & 0x8000))
		return 0;	// ignore attempts to read from DSP program ROM

	bank &= 0x7FFF;
	rom_addr = (bank << 16) | (address << 0);

	sample_data = chip->rom_data[rom_addr];

	return (int16_t)((sample_data << 8) | (sample_data << 0));	// MAME currently expands the 8 bit ROM data to 16 bits this way.
}

static inline int16_t* get_filter_table(struct qsound_chip *chip, uint16_t offset)
{
	int index;

	if (offset >= 0xf2e && offset < 0xfff)
		return (int16_t*)&qsound_filter_data2[offset-0xf2e];	// overlapping filter data

	index = (offset-0xd53)/95;
	if(index >= 0 && index < 5)
		return (int16_t*)&qsound_filter_data[index];	// normal tables

	return NULL;	// no filter found.
}

/********************************************************************/

// updates one DSP sample
void qsound_update(struct qsound_chip *chip)
{
	switch(chip->state)
	{
		default:
		case STATE_INIT1:
		case STATE_INIT2:
			state_init(chip); return;
		case STATE_REFRESH1:
			state_refresh_filter_1(chip); return;
		case STATE_REFRESH2:
			state_refresh_filter_2(chip); return;
		case STATE_NORMAL1:
		case STATE_NORMAL2:
			state_normal_update(chip); return;
	}
}

// Initialization routine
static void state_init(struct qsound_chip *chip)
{
	int mode = (chip->state == STATE_INIT2) ? 1 : 0;
	int i;

	// we're busy for 4 samples, including the filter refresh.
	if(chip->state_counter >= 2)
	{
		chip->state_counter = 0;
		chip->state = chip->next_state;
		return;
	}
	else if(chip->state_counter == 1)
	{
		chip->state_counter++;
		return;
	}

	memset(chip->voice, 0, sizeof(chip->voice));
	memset(chip->adpcm, 0, sizeof(chip->adpcm));
	memset(chip->filter, 0, sizeof(chip->filter));
	memset(chip->alt_filter, 0, sizeof(chip->alt_filter));
	memset(chip->wet, 0, sizeof(chip->wet));
	memset(chip->dry, 0, sizeof(chip->dry));
	memset(&chip->echo, 0, sizeof(chip->echo));

	for(i=0;i<19;i++)
	{
		chip->voice_pan[i] = 0x120;
		chip->voice_output[i] = 0;
	}

	for(i=0;i<16;i++)
		chip->voice[i].bank = 0x8000;
	for(i=0;i<3;i++)
		chip->adpcm[i].bank = 0x8000;

	if(mode == 0)
	{
		// mode 1
		chip->wet[0].delay = 0;
		chip->dry[0].delay = 46;
		chip->wet[1].delay = 0;
		chip->dry[1].delay = 48;
		chip->filter[0].table_pos = 0xdb2;
		chip->filter[1].table_pos = 0xe11;
		chip->echo.end_pos = 0x554 + 6;
		chip->next_state = STATE_REFRESH1;
	}
	else
	{
		// mode 2
		chip->wet[0].delay = 1;
		chip->dry[0].delay = 0;
		chip->wet[1].delay = 0;
		chip->dry[1].delay = 0;
		chip->filter[0].table_pos = 0xf73;
		chip->filter[1].table_pos = 0xfa4;
		chip->alt_filter[0].table_pos = 0xf73;
		chip->alt_filter[1].table_pos = 0xfa4;
		chip->echo.end_pos = 0x53c + 6;
		chip->next_state = STATE_REFRESH2;
	}

	chip->wet[0].volume = 0x3fff;
	chip->dry[0].volume = 0x3fff;
	chip->wet[1].volume = 0x3fff;
	chip->dry[1].volume = 0x3fff;

	chip->delay_update = 1;
	chip->ready_flag = 0;
	chip->state_counter = 1;
}

// Updates filter parameters for mode 1
static void state_refresh_filter_1(struct qsound_chip *chip)
{
	const int16_t *table;

	for(int ch=0; ch<2; ch++)
	{
		chip->filter[ch].delay_pos = 0;
		chip->filter[ch].tap_count = 95;

		table = get_filter_table(chip,chip->filter[ch].table_pos);
		if (table != NULL)
			memcpy(chip->filter[ch].taps, table, 95 * sizeof(int16_t));
	}

	chip->state = chip->next_state = STATE_NORMAL1;
}

// Updates filter parameters for mode 2
static void state_refresh_filter_2(struct qsound_chip *chip)
{
	const int16_t *table;

	for(int ch=0; ch<2; ch++)
	{
		chip->filter[ch].delay_pos = 0;
		chip->filter[ch].tap_count = 45;

		table = get_filter_table(chip,chip->filter[ch].table_pos);
		if (table != NULL)
			memcpy(chip->filter[ch].taps, table, 45 * sizeof(int16_t));

		chip->alt_filter[ch].delay_pos = 0;
		chip->alt_filter[ch].tap_count = 44;

		table = get_filter_table(chip,chip->alt_filter[ch].table_pos);
		if (table != NULL)
			memcpy(chip->alt_filter[ch].taps, table, 44 * sizeof(int16_t));
	}

	chip->state = chip->next_state = STATE_NORMAL2;
}

// Updates a PCM voice. There are 16 voices, each are updated every sample
// with full rate and volume control.
static inline int16_t pcm_update(struct qsound_chip *chip, int voice_no, int32_t *echo_out)
{
	struct qsound_voice* v = &chip->voice[voice_no];

	int32_t new_phase;
	int16_t output = 0;

	if(!(chip->mute_mask & (1<<voice_no)))
	{
		// Read sample from rom and apply volume
		output = (v->volume * get_sample(chip, v->bank, v->addr))>>14;
		*echo_out += (output * v->echo)<<2;
	}

	// Add delta to the phase and loop back if required
	new_phase = v->rate + ((v->addr<<12) | (v->phase>>4));

	if((new_phase>>12) >= v->end_addr)
		new_phase -= (v->loop_len<<12);

	new_phase = CLAMP(new_phase, -0x8000000, 0x7FFFFFF);
	v->addr = new_phase>>12;
	v->phase = (new_phase<<4)&0xffff;

	return output;
}

// Updates an ADPCM voice. There are 3 voices, one is updated every sample
// (effectively making the ADPCM rate 1/3 of the master sample rate), and
// volume is set when starting samples only.
// The ADPCM algorithm is supposedly similar to Yamaha ADPCM. It also seems
// like Capcom never used it, so this was not emulated in the earlier QSound
// emulators.
static inline void adpcm_update(struct qsound_chip *chip, int voice_no, int nibble)
{
	struct qsound_adpcm *v = &chip->adpcm[voice_no];

	int32_t delta;
	int8_t step;

	if(!nibble)
	{
		// Mute voice when it reaches the end address.
		if(v->cur_addr == v->end_addr)
			v->cur_vol = 0;

		// Playback start flag
		if(v->flag)
		{
			chip->voice_output[16+voice_no] = 0;
			v->flag = 0;
			v->step_size = 10;
			v->cur_vol = v->volume;
			v->cur_addr = v->start_addr;
		}

		// get top nibble
		step = get_sample(chip, v->bank, v->cur_addr) >> 8;
	}
	else
	{
		// get bottom nibble
		step = get_sample(chip, v->bank, v->cur_addr++) >> 4;
	}

	// shift with sign extend
	step >>= 4;

	// delta = (0.5 + abs(v->step)) * v->step_size
	delta = ((1+abs(step<<1)) * v->step_size)>>1;
	if(step <= 0)
		delta = -delta;
	delta += chip->voice_output[16+voice_no];
	delta = CLAMP(delta,-32768,32767);

	if(chip->mute_mask & (1<<(16+voice_no)))
		chip->voice_output[16+voice_no] = 0;
	else
		chip->voice_output[16+voice_no] = (delta * v->cur_vol)>>16;

	v->step_size = (adpcm_step_table[8+step] * v->step_size) >> 6;
	v->step_size = CLAMP(v->step_size, 1, 2000);
}

// The echo effect is pretty simple. A moving average filter is used on
// the output from the delay line to smooth samples over time.
static inline int16_t echo(struct qsound_echo *r,int32_t input)
{
	// get average of last 2 samples from the delay line
	int32_t new_sample;
	int32_t old_sample = r->delay_line[r->delay_pos];
	int32_t last_sample = r->last_sample;

	r->last_sample = old_sample;
	old_sample = (old_sample+last_sample) >> 1;

	// add current sample to the delay line
	new_sample = input + ((old_sample * r->feedback)<<2);
	r->delay_line[r->delay_pos++] = new_sample>>16;

	if(r->delay_pos >= r->length)
		r->delay_pos = 0;

	return old_sample;
}

// Process a sample update
static void state_normal_update(struct qsound_chip *chip)
{
	int v, ch;
	int32_t echo_input = 0;
	int16_t echo_output;

	chip->ready_flag = 0x80;

	// recalculate echo length
	if(chip->state == STATE_NORMAL2)
		chip->echo.length = chip->echo.end_pos - 0x53c;
	else
		chip->echo.length = chip->echo.end_pos - 0x554;

	chip->echo.length = CLAMP(chip->echo.length, 0, 1024);

	// update PCM voices
	for(v=0; v<16; v++)
	{
		chip->voice_output[v] = pcm_update(chip, v, &echo_input);
	}

	// update ADPCM voices (one every third sample)
	adpcm_update(chip, chip->state_counter % 3, chip->state_counter / 3);

	echo_output = echo(&chip->echo,echo_input);

	// now, we do the magic stuff
	for(ch=0; ch<2; ch++)
	{
		// Echo is output on the unfiltered component of the left channel and
		// the filtered component of the right channel.
		int32_t wet = (ch == 1) ? echo_output<<16 : 0;
		int32_t dry = (ch == 0) ? echo_output<<16 : 0;
		int32_t output = 0;

		for(int v=0; v<19; v++)
		{
			uint16_t pan_index = chip->voice_pan[v]-0x110;
			if(pan_index > 97)
				pan_index = 97;

			// Apply different volume tables on the dry and wet inputs.
			dry -= (chip->voice_output[v] * chip->pan_tables[ch][PANTBL_DRY][pan_index])<<2;
			wet -= (chip->voice_output[v] * chip->pan_tables[ch][PANTBL_WET][pan_index])<<2;
		}

		// Apply FIR filter on 'wet' input
		wet = fir(&chip->filter[ch], wet >> 16);

		// in mode 2, we do this on the 'dry' input too
		if(chip->state == STATE_NORMAL2)
			dry = fir(&chip->alt_filter[ch], dry >> 16);

		// output goes through a delay line and attenuation
		output = (delay(&chip->wet[ch], wet) + delay(&chip->dry[ch], dry));

		// DSP round function
		output = (output + 0x2000) >> 14;
		chip->out[ch] = CLAMP(output, -0x7fff, 0x7fff);

		if(chip->delay_update)
		{
			delay_update(&chip->wet[ch]);
			delay_update(&chip->dry[ch]);
		}
	}

	chip->delay_update = 0;

	// after 6 samples, the next state is executed.
	chip->state_counter++;
	if(chip->state_counter > 5)
	{
		chip->state_counter = 0;
		chip->state = chip->next_state;
	}
}

// Apply the FIR filter used as the Q1 transfer function
static inline int32_t fir(struct qsound_fir *f, int16_t input)
{
	int32_t output = 0, tap = 0;

	for(; tap < (f->tap_count-1); tap++)
	{
		output -= (f->taps[tap] * f->delay_line[f->delay_pos++])<<2;

		if(f->delay_pos >= f->tap_count-1)
			f->delay_pos = 0;
	}

	output -= (f->taps[tap] * input)<<2;

	f->delay_line[f->delay_pos++] = input;
	if(f->delay_pos >= f->tap_count-1)
		f->delay_pos = 0;

	return output;
}

// Apply delay line and component volume
static inline int32_t delay(struct qsound_delay *d, int32_t input)
{
	int32_t output;

	d->delay_line[d->write_pos++] = input>>16;
	if(d->write_pos >= 51)
		d->write_pos = 0;

	output = d->delay_line[d->read_pos++]*d->volume;
	if(d->read_pos >= 51)
		d->read_pos = 0;

	return output;
}

// Update the delay read position to match new delay length
static inline void delay_update(struct qsound_delay *d)
{
	int16_t new_read_pos = (d->write_pos - d->delay) % 51;
	if(new_read_pos < 0)
		new_read_pos += 51;

	d->read_pos = new_read_pos;
}

