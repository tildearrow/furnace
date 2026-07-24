#ifndef KLATTSCH_KLATTSCH_C_H
#define KLATTSCH_KLATTSCH_C_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  KLATTSCH_PARAM_F0 = 0,
  KLATTSCH_PARAM_VOICING,
  KLATTSCH_PARAM_F1,
  KLATTSCH_PARAM_BW1,
  KLATTSCH_PARAM_A1,
  KLATTSCH_PARAM_F2,
  KLATTSCH_PARAM_BW2,
  KLATTSCH_PARAM_A2,
  KLATTSCH_PARAM_F3,
  KLATTSCH_PARAM_BW3,
  KLATTSCH_PARAM_A3,
  KLATTSCH_PARAM_GAIN,
  KLATTSCH_PARAM_VIBRATO_DEPTH,
  KLATTSCH_PARAM_VIBRATO_RATE,
  KLATTSCH_PARAM_TREMOLO_DEPTH,
  KLATTSCH_PARAM_TREMOLO_RATE,
  KLATTSCH_PARAM_ASPIRATION,
  KLATTSCH_PARAM_TILT,
  KLATTSCH_PARAM_EFFORT,
  KLATTSCH_PARAM_COUNT
} klattsch_param_id;

typedef struct {
  uint32_t mask;
  float values[KLATTSCH_PARAM_COUNT];
} klattsch_param_update;

typedef struct {
  uint64_t at_sample;
  uint32_t transition_samples;
  klattsch_param_update target;
} klattsch_schedule_event;

typedef struct klattsch_synth klattsch_synth;

klattsch_synth* klattsch_synth_new(uint32_t sample_rate, uint32_t noise_seed);
void klattsch_synth_free(klattsch_synth* s);

void klattsch_synth_reset(klattsch_synth* s);
void klattsch_synth_set_target(klattsch_synth* s,
                               const klattsch_param_update* update,
                               uint32_t transition_samples);
void klattsch_synth_set_schedule(klattsch_synth* s,
                                 const klattsch_schedule_event* events,
                                 size_t count);
void klattsch_synth_process(klattsch_synth* s, float* mono_out, size_t frames);
void klattsch_synth_add_to(klattsch_synth* s, float* mono_out, size_t frames, float gain);

#ifdef __cplusplus
}
#endif

#endif  // KLATTSCH_KLATTSCH_C_H
