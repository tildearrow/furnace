/*
 * scsp_waveforms.c — Built-in waveform generators for SCSP.
 *
 * Ported from saturn_kit.py's gen_* functions.
 * All produce band-limited waveforms normalized to [-1.0, 1.0].
 */

#include "scsp_waveforms.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ── Helpers ──────────────────────────────────────────────────── */

static void normalize(float *buf, int n)
{
    float peak = 0.0f;
    for (int i = 0; i < n; i++) {
        float a = fabsf(buf[i]);
        if (a > peak) peak = a;
    }
    if (peak > 0.0f) {
        for (int i = 0; i < n; i++)
            buf[i] /= peak;
    }
}

static void gen_additive(float *out, int n, const float *harmonics, const float *amps, int nh)
{
    memset(out, 0, n * sizeof(float));
    for (int h = 0; h < nh; h++) {
        float harm = harmonics[h];
        float amp  = amps[h];
        for (int i = 0; i < n; i++) {
            out[i] += amp * sinf(2.0f * (float)M_PI * harm * i / n);
        }
    }
    normalize(out, n);
}

/* ── Generators ───────────────────────────────────────────────── */

static void gen_sine(float *out, int n)
{
    for (int i = 0; i < n; i++)
        out[i] = sinf(2.0f * (float)M_PI * i / n);
}

static void gen_sawtooth(float *out, int n)
{
    /* Band-limited via 15 harmonics: sum((-1)^(h+1) / h * sin(h*x)) */
    float h[15], a[15];
    for (int i = 0; i < 15; i++) {
        h[i] = (float)(i + 1);
        a[i] = ((i + 1) % 2 == 0 ? -1.0f : 1.0f) / (float)(i + 1);
    }
    gen_additive(out, n, h, a, 15);
}

static void gen_square(float *out, int n)
{
    /* Band-limited: odd harmonics only, amplitude 1/h */
    float h[8], a[8];
    for (int i = 0; i < 8; i++) {
        h[i] = (float)(2 * i + 1);
        a[i] = 1.0f / (float)(2 * i + 1);
    }
    gen_additive(out, n, h, a, 8);
}

static void gen_triangle(float *out, int n)
{
    /* Band-limited: odd harmonics, amplitude (-1)^((h-1)/2) / h^2 */
    float h[8], a[8];
    for (int i = 0; i < 8; i++) {
        int harm = 2 * i + 1;
        h[i] = (float)harm;
        a[i] = (i % 2 == 0 ? 1.0f : -1.0f) / (float)(harm * harm);
    }
    gen_additive(out, n, h, a, 8);
}

static void gen_organ(float *out, int n)
{
    /* Drawbar-style additive */
    float h[] = {1, 2, 3, 4, 6, 8, 10};
    float a[] = {1.0f, 0.8f, 0.6f, 0.3f, 0.2f, 0.15f, 0.1f};
    gen_additive(out, n, h, a, 7);
}

static void gen_brass(float *out, int n)
{
    /* Strong odd harmonics with slight even */
    float h[] = {1, 2, 3, 4, 5, 6, 7, 9};
    float a[] = {1.0f, 0.3f, 0.7f, 0.15f, 0.5f, 0.1f, 0.3f, 0.15f};
    gen_additive(out, n, h, a, 8);
}

static void gen_strings(float *out, int n)
{
    /* Sawtooth with rolled-off highs: amplitude 1/h^1.2 */
    float h[20], a[20];
    for (int i = 0; i < 20; i++) {
        h[i] = (float)(i + 1);
        a[i] = 1.0f / powf((float)(i + 1), 1.2f);
    }
    gen_additive(out, n, h, a, 20);
}

static void gen_piano(float *out, int n)
{
    /* Harmonics with slight inharmonicity */
    float h[] = {1, 2, 3, 4, 5, 6, 7, 8};
    float a[] = {1.0f, 0.7f, 0.4f, 0.25f, 0.15f, 0.1f, 0.08f, 0.05f};
    gen_additive(out, n, h, a, 8);
}

static void gen_flute(float *out, int n)
{
    /* Fundamental + weak overtones */
    float h[] = {1, 2, 3};
    float a[] = {1.0f, 0.15f, 0.05f};
    gen_additive(out, n, h, a, 3);
}

static void gen_bass(float *out, int n)
{
    /* Strong fundamental + low harmonics */
    float h[] = {1, 2, 3, 4};
    float a[] = {1.0f, 0.5f, 0.2f, 0.1f};
    gen_additive(out, n, h, a, 4);
}

/* ── Public API ───────────────────────────────────────────────── */

static const char *wave_names[SCSP_NUM_BUILTINS] = {
    "Sine", "Sawtooth", "Square", "Triangle", "Organ",
    "Brass", "Strings", "Piano", "Flute", "Bass"
};

const char *scsp_wave_name(int type)
{
    if (type >= 0 && type < SCSP_NUM_BUILTINS) return wave_names[type];
    return "Custom";
}

typedef void (*gen_func_t)(float *out, int n);

static const gen_func_t generators[SCSP_NUM_BUILTINS] = {
    gen_sine, gen_sawtooth, gen_square, gen_triangle, gen_organ,
    gen_brass, gen_strings, gen_piano, gen_flute, gen_bass
};

void scsp_gen_waveform(int type, float *out, int length)
{
    if (type >= 0 && type < SCSP_NUM_BUILTINS) {
        generators[type](out, length);
    } else {
        /* Unknown type: generate silence */
        memset(out, 0, length * sizeof(float));
    }
}

void scsp_write_waveform(uint8_t *ram, int offset, const float *samples, int length)
{
    for (int i = 0; i < length; i++) {
        int16_t val = (int16_t)(samples[i] * 32767.0f);
        if (val > 32767) val = 32767;
        if (val < -32768) val = -32768;
        /* Little-endian (WASM/native LE) */
        ram[offset + i * 2]     = (uint8_t)(val & 0xFF);
        ram[offset + i * 2 + 1] = (uint8_t)((val >> 8) & 0xFF);
    }
}

int scsp_load_builtins(uint8_t *ram, int *offsets)
{
    float buf[SCSP_WAVE_LEN];
    int offset = 0;

    for (int t = 0; t < SCSP_NUM_BUILTINS; t++) {
        offsets[t] = offset;
        scsp_gen_waveform(t, buf, SCSP_WAVE_LEN);
        scsp_write_waveform(ram, offset, buf, SCSP_WAVE_LEN);
        offset += SCSP_WAVE_BYTES;
    }

    return offset; /* next free byte */
}
