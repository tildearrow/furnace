/*
 * scsp_waveforms.h — Built-in waveform generators for SCSP FM synthesis.
 *
 * All built-in waveforms are 1024 samples (required by SCSP FM math).
 * Custom waveforms can be any length for pure PCM use.
 */

#ifndef SCSP_WAVEFORMS_H
#define SCSP_WAVEFORMS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Waveform type IDs */
#define SCSP_WAVE_SINE      0
#define SCSP_WAVE_SAWTOOTH  1
#define SCSP_WAVE_SQUARE    2
#define SCSP_WAVE_TRIANGLE  3
#define SCSP_WAVE_ORGAN     4
#define SCSP_WAVE_BRASS     5
#define SCSP_WAVE_STRINGS   6
#define SCSP_WAVE_PIANO     7
#define SCSP_WAVE_FLUTE     8
#define SCSP_WAVE_BASS      9
#define SCSP_NUM_BUILTINS   10

#define SCSP_WAVE_LEN       1024  /* all built-ins are 1024 samples */
#define SCSP_WAVE_BYTES     (SCSP_WAVE_LEN * 2)  /* 2048 bytes per waveform (16-bit) */

/* Waveform name lookup */
const char *scsp_wave_name(int type);

/*
 * Generate a built-in waveform into a float buffer.
 * Output: `length` samples in [-1.0, 1.0], normalized to peak = 1.0.
 */
void scsp_gen_waveform(int type, float *out, int length);

/*
 * Write a float waveform to SCSP RAM as little-endian int16.
 * ram: pointer to SCSP sound RAM
 * offset: byte offset in RAM (must be even)
 * samples: float samples in [-1.0, 1.0]
 * length: number of samples
 */
void scsp_write_waveform(uint8_t *ram, int offset, const float *samples, int length);

/*
 * Load all built-in waveforms into SCSP RAM starting at offset 0.
 * Returns the next free byte offset after all built-ins.
 * offsets[]: filled with the byte offset of each waveform (SCSP_NUM_BUILTINS entries).
 */
int scsp_load_builtins(uint8_t *ram, int *offsets);

#ifdef __cplusplus
}
#endif

#endif /* SCSP_WAVEFORMS_H */
