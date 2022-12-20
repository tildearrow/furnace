#ifndef MZPOKEYSND_H_
#define MZPOKEYSND_H_

#include <stdlib.h>

struct stPokeyState;

typedef int (*readout_t)(struct stPokeyState* ps);
typedef void (*event_t)(struct stPokeyState* ps, int p5v, int p4v, int p917v);

#ifdef NONLINEAR_MIXING
/* Change queue event value type */
typedef double qev_t;
#else
typedef unsigned char qev_t;
#endif

/* State variables for single Pokey Chip */
typedef struct stPokeyState
{
    int curtick;
    /* Poly positions */
    int poly4pos;
    int poly5pos;
    int poly17pos;
    int poly9pos;

    /* Main divider (64khz/15khz) */
    int mdivk;    /* 28 for 64khz, 114 for 15khz */

    /* Main switches */
    int selpoly9;
    int c0_hf;
    int c1_f0;
    int c2_hf;
    int c3_f2;

    /* SKCTL for two-tone mode */
    int skctl;

    /* Main output state */
    qev_t outvol_all;
    int forcero; /* Force readout */

    /* channel 0 state */

    readout_t readout_0;
    event_t event_0;

    int c0divpos;
    int c0divstart;   /* AUDF0 recalculated */
    int c0divstart_p; /* start value when c1_f0 */
    int c0diva;      /* AUDF0 register */

    int c0t1;         /* D - 5bit, Q goes to sw3 */
    int c0t2;         /* D - out sw2, Q goes to sw4 and t3 */
    int c0t3;         /* D - out t2, q goes to xor */

    int c0sw1;        /* in1 - 4bit, in2 - 17bit, out goes to sw2 */
    int c0sw2;        /* in1 - /Q t2, in2 - out sw1, out goes to t2 */
    int c0sw3;        /* in1 - +5, in2 - Q t1, out goes to C t2 */
    int c0sw4;        /* hi-pass sw */
    int c0vo;         /* volume only */

#ifndef NONLINEAR_MIXING
    int c0stop;       /* channel counter stopped */
#endif

    int vol0;

    int outvol_0;

    /* channel 1 state */

    readout_t readout_1;
    event_t event_1;

    int c1divpos;
    int c1divstart;
    int c1diva;

    int c1t1;
    int c1t2;
    int c1t3;

    int c1sw1;
    int c1sw2;
    int c1sw3;
    int c1sw4;
    int c1vo;

#ifndef NONLINEAR_MIXING
    int c1stop;      /* channel counter stopped */
#endif

    int vol1;

    int outvol_1;

    /* channel 2 state */

    readout_t readout_2;
    event_t event_2;

    int c2divpos;
    int c2divstart;
    int c2divstart_p;     /* start value when c1_f0 */
    int c2diva;

    int c2t1;
    int c2t2;

    int c2sw1;
    int c2sw2;
    int c2sw3;
    int c2vo;

#ifndef NONLINEAR_MIXING
    int c2stop;          /* channel counter stopped */
#endif

    int vol2;

    int outvol_2;

    /* channel 3 state */

    readout_t readout_3;
    event_t event_3;

    int c3divpos;
    int c3divstart;
    int c3diva;

    int c3t1;
    int c3t2;

    int c3sw1;
    int c3sw2;
    int c3sw3;
    int c3vo;

#ifndef NONLINEAR_MIXING
    int c3stop;          /* channel counter stopped */
#endif

    int vol3;

    int outvol_3;
} PokeyState;

void mzpokeysnd_process_16(PokeyState* ps, void* sndbuffer, int sndn);
void Update_pokey_sound_mz(PokeyState* ps, unsigned short addr, unsigned char val, unsigned char gain);

void ResetPokeyState(PokeyState* ps);

int MZPOKEYSND_Init(PokeyState* ps);

#endif /* MZPOKEYSND_H_ */
