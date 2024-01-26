#ifndef PWRNOISE_H
#define PWRNOISE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	bool enable;
	bool am;

	uint16_t period;
	uint16_t period_counter;

	uint8_t octave;
	uint16_t octave_counter;

	uint8_t tapa;
	uint8_t tapb;
	bool tapb_enable;

	uint16_t lfsr;
	uint8_t vol;

	uint8_t out_latch;
	uint8_t prev;
} noise_channel_t;

typedef struct {
	bool enable;
	uint8_t flags;

	uint16_t period;
	uint16_t period_counter;

	uint8_t octave;
	uint16_t octave_counter;

	uint8_t alength;
	uint8_t blength;
	uint16_t a;
	uint16_t b;
	bool portion;

	uint8_t aoffset;
	uint8_t boffset;

	uint8_t accum;
	uint8_t vol;

	uint8_t out_latch;
  uint8_t prev;
} slope_channel_t;

typedef struct {
	uint8_t flags;
	uint8_t gpioa;
	uint8_t gpiob;
	
	noise_channel_t n1;
	noise_channel_t n2;
	noise_channel_t n3;
	slope_channel_t s;
} power_noise_t;

void pwrnoise_reset(power_noise_t *pn);
void pwrnoise_step(power_noise_t *pn, uint16_t cycles, int16_t *left, int16_t *right);
void pwrnoise_write(power_noise_t *pn, uint8_t reg, uint8_t val);

#ifdef __cplusplus
}
#endif

#endif