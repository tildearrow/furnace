#ifndef QSOUND_H
#define QSOUND_H

#ifdef __cplusplus
extern "C" {
#endif
/*

	Capcom DL-1425 QSound emulator
	==============================

	by superctr (Ian Karlsson)
	with thanks to Valley Bell

	2018-05-12 - 2018-05-15

*/

#include <stdint.h>

struct qsound_voice {
	uint16_t bank;
	int16_t addr; // top word is the sample address
	uint16_t phase;
	uint16_t rate;
	int16_t loop_len;
	int16_t end_addr;
	int16_t volume;
	int16_t echo;
};

struct qsound_adpcm {
	uint16_t start_addr;
	uint16_t end_addr;
	uint16_t bank;
	int16_t volume;
	uint16_t flag;
	int16_t cur_vol;
	int16_t step_size;
	uint16_t cur_addr;
};

// Q1 Filter
struct qsound_fir {
	int tap_count;	// usually 95
	int delay_pos;
	int16_t table_pos;
	int16_t taps[95];
	int16_t delay_line[95];
};

// Delay line
struct qsound_delay {
	int16_t delay;
	int16_t volume;
	int16_t write_pos;
	int16_t read_pos;
	int16_t delay_line[51];
};

struct qsound_echo {
	uint16_t end_pos;

	int16_t feedback;
	int16_t length;
	int16_t last_sample;
	int16_t delay_line[1024];
	int16_t delay_pos;
};

struct qsound_chip {

	unsigned long rom_mask;
	uint8_t *rom_data;

	uint32_t mute_mask;

	uint16_t data_latch;
	int16_t out[2];

	int16_t pan_tables[2][2][98];

	struct qsound_voice voice[16];
	struct qsound_adpcm adpcm[3];

	uint16_t voice_pan[16+3];
	int16_t voice_output[16+3];

	struct qsound_echo echo;

	struct qsound_fir filter[2];
	struct qsound_fir alt_filter[2];

	struct qsound_delay wet[2];
	struct qsound_delay dry[2];

	uint16_t state;
	uint16_t next_state;

	uint16_t delay_update;

	int state_counter;
	int ready_flag;

	uint16_t *register_map[256];
};

long qsound_start(struct qsound_chip *chip, int clock);
void qsound_reset(struct qsound_chip *chip);
void qsound_update(struct qsound_chip *chip);

void qsound_stream_update(struct qsound_chip *chip, int16_t **outputs, int samples);
void qsound_w(struct qsound_chip *chip, uint8_t offset, uint8_t data);
uint8_t qsound_r(struct qsound_chip *chip);
void qsound_write_data(struct qsound_chip *chip, uint8_t address, uint16_t data);
uint16_t qsound_read_data(struct qsound_chip *chip, uint8_t address);

#ifdef __cplusplus
};
#endif
#endif
