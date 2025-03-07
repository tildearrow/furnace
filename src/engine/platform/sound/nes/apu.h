/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

// additional modifications by tildearrow for furnace

#ifndef APU_H_
#define APU_H_

#include "common.h"

enum dmc_types_of_dma { DMC_NORMAL, DMC_CPU_WRITE, DMC_R4014, DMC_NNL_DMA };
enum apu_channels { APU_S1, APU_S2, APU_TR, APU_NS, APU_DMC, APU_EXTRA, APU_MASTER };
enum apu_mode { APU_60HZ, APU_48HZ };

#define apu_pre_amp 1.4f

/* length counter */
#define length_run(channel)\
	/*\
	 * se non e' settato il flag halt e il length\
	 * counter non e' 0 allora devo decrementarlo.\
	 */\
	if (!channel.length.halt && channel.length.value) {\
		channel.length.value--;\
	}
#define length_clock()\
	a->apu.length_clocked = TRUE;\
	length_run(a->S1)\
	length_run(a->S2)\
	length_run(a->TR)\
	length_run(a->NS)
/* envelope */
#define envelope_run(channel)\
	if (channel.envelope.enabled) {\
		channel.envelope.enabled = FALSE;\
		channel.envelope.counter = 15;\
		channel.envelope.delay = (channel.envelope.divider + 1);\
	} else if (!(--channel.envelope.delay)) {\
		channel.envelope.delay = (channel.envelope.divider + 1);\
		if (channel.envelope.counter | channel.length.halt) {\
			channel.envelope.counter = (channel.envelope.counter - 1) & 0x0F;\
		}\
	}
#define envelope_volume(channel)\
	/* setto il volume */\
	if (!channel.length.value) {\
		channel.volume = 0;\
	} else if (channel.envelope.constant_volume) {\
		channel.volume = channel.envelope.divider;\
	} else {\
		channel.volume = channel.envelope.counter;\
	}
#define envelope_clock()\
	envelope_run(a->S1)\
	envelope_run(a->S2)\
	envelope_run(a->NS)
/* sweep */
#define sweep_run(channel, negative_adjust)\
	if (!(--channel.sweep.delay)) {\
		channel.sweep.delay = (channel.sweep.divider + 1);\
		if (channel.sweep.enabled && channel.sweep.shift && (channel.timer >= 8)) {\
			SWORD offset = channel.timer >> channel.sweep.shift;\
			if (channel.sweep.negate) {\
				channel.timer += ((SWORD) negative_adjust - offset);\
			} else if ((channel.timer + offset) <= 0x800) {\
				channel.timer += offset;\
			}\
		}\
		sweep_silence(channel)\
	}\
	if (channel.sweep.reload) {\
		channel.sweep.reload = FALSE;\
		channel.sweep.delay = (channel.sweep.divider + 1);\
	}
#define sweep_silence(channel)\
{\
	WORD offset = channel.timer >> channel.sweep.shift;\
	channel.sweep.silence = FALSE;\
	if ((channel.timer <= 8) || (!channel.sweep.negate && ((channel.timer + offset) >= 0x800))) {\
		channel.sweep.silence = TRUE;\
	}\
}
#define sweep_clock()\
	sweep_run(a->S1, -1)\
	sweep_run(a->S2,  0)
/* linear counter */
#define linear_clock()\
	if (a->TR.linear.halt) {\
		a->TR.linear.value = a->TR.linear.reload;\
	} else if (a->TR.linear.value) {\
		a->TR.linear.value--;\
	}\
	if (!a->TR.length.halt) {\
		a->TR.linear.halt = FALSE;\
	}
/* output */
#define square_output(square, swap)\
{\
	envelope_volume(square)\
	if (square.sweep.silence) {\
		square.output = 0;\
	} else {\
		square.output = square_duty[swap][square.duty][square.sequencer] * square.volume;\
	}\
}
#define triangle_output()\
	/*\
	 * ai 2 cicli piu' bassi del timer, la frequenza\
	 * risultante e' troppo alta (oltre i 20 kHz,\
	 * quindi non udibile), percio' la taglio.\
	 */\
	a->TR.output = triangle_duty[a->TR.sequencer];\
	if (a->TR.timer < 2) {\
		a->TR.output = triangle_duty[8];\
	}
#define noise_output()\
	envelope_volume(a->NS)\
	a->NS.output = 0;\
	if (a->NS.length.value && !(a->NS.shift & 0x0001)) {\
		a->NS.output = a->NS.volume;\
	}
#define dmc_output()\
	a->DMC.output = a->DMC.counter & 0x7F
/* tick */

#define apu_change_step(index)\
	a->apu.cycles += apuPeriod[a->apu.mode][a->apu.type][index]
#if defined (VECCHIA_GESTIONE_JITTER)
#define r4017_jitter()\
	a->r4017.value = (a->r4017.jitter.value & 0xC0);\
	/*\
	 * se il bit 7 e' a zero, devo attivare la\
	 * modalita' NTSC, se a uno quella PAL.\
	 */\
	if (a->r4017.value & 0x80) {\
		a->apu.mode = APU_48HZ;\
	} else {\
		a->apu.mode = APU_60HZ;\
	}\
	if (a->r4017.value & 0x40) {\
		/* azzero il bit 6 del $4015 */\
		a->r4015.value &= 0xBF;\
		/* questo non e' affatto necessario sul forno */\
	}\
	/* riavvio il frame audio */\
	a->apu.step = a->apu.cycles = 0;\
	apu_change_step(a->apu.step)
#else
#define r4017_jitter(apc)\
	a->r4017.value = (a->r4017.jitter.value & 0xC0);\
	a->r4017.reset_frame_delay = 1;\
	if (a->apu.cycles == apc) {\
		if (a->apu.mode == APU_48HZ) {\
			a->r4017.reset_frame_delay += 1;\
		} else {\
			a->r4017.reset_frame_delay += 2;\
		}\
	}\
	/*\
	 * se il bit 7 e' a zero, devo attivare la\
	 * modalita' NTSC, se a uno quella PAL.\
	 */\
	if (a->r4017.value & 0x80) {\
		a->apu.mode = APU_48HZ;\
	} else {\
		a->apu.mode = APU_60HZ;\
	}\
	if (a->r4017.value & 0x40) {\
		/* azzero il bit 6 del $4015 */\
		a->r4015.value &= 0xBF;\
		/* questo non e' affatto necessario sul forno */\
	}
#define r4017_reset_frame()\
	if (a->r4017.reset_frame_delay && (--a->r4017.reset_frame_delay == 0)) {\
		/* riavvio il frame audio */\
		a->apu.step = a->apu.cycles = 0;\
		apu_change_step(a->apu.step);\
	}
#endif
#define square_reg0(square)\
	/* duty */\
	square.duty = value >> 6;\
	/* length counter */\
	square.length.halt = value & 0x20;\
	/* envelope */\
	square.envelope.constant_volume = value & 0x10;\
	square.envelope.divider = value & 0x0F
#define square_reg1(square)\
	/* sweep */\
	square.sweep.reload = TRUE;\
	square.sweep.divider = (value >> 4) & 0x07;\
	square.sweep.shift = value & 0x07;\
	square.sweep.enabled = value & 0x80;\
	square.sweep.negate = value & 0x08
#define square_reg2(square)\
	/* timer (low 8 bits) */\
	square.timer = (square.timer & 0x0700) | value
#define square_reg3(square,length_clocked)\
	/* length counter */\
	/*\
	 * se non disabilitato, una scrittura in\
	 * questo registro, carica immediatamente il\
	 * length counter del canale, tranne nel caso\
	 * in cui la scrittura avvenga nello stesso\
	 * momento del clock di un length counter e\
	 * con il length diverso da zero.\
	 */\
	if (square.length.enabled && !(length_clocked && square.length.value)) {\
		square.length.value = length_table[value >> 3];\
	}\
	/* envelope */\
	square.envelope.enabled = TRUE;\
	/* timer (high 3 bits) */\
	square.timer = (square.timer & 0x00FF) | ((value & 0x07) << 8);\
	/*The correct behaviour is to reset the duty cycle sequencers but not the clock dividers*/\
	/*square.frequency = 1;*/\
	/* sequencer */\
	square.sequencer = 0
#define init_nla_table(p, t)\
{\
	WORD i;\
	for (i = 0; i < LENGTH(nla_table.pulse); i++) {\
		double vl = 95.52 / (8128.0 / (double) i + 100.0);\
		nla_table.pulse[i] = (vl * p);\
	}\
	for (i = 0; i < LENGTH(nla_table.tnd); i++) {\
		double vl = 163.67 / (24329.0 / (double) i + 100.0);\
		nla_table.tnd[i] = (vl * t);\
	}\
}
#define _apu_channel_volume_adjust(ch, index)\
	((ch))
#define s1_out(a)\
	(a->muted[0] ? 0 : _apu_channel_volume_adjust(a->S1.output, APU_S1))
#define s2_out(a)\
	(a->muted[1] ? 0 : _apu_channel_volume_adjust(a->S2.output, APU_S2))
#define tr_out(a)\
	(a->muted[2] ? 0 : _apu_channel_volume_adjust(a->TR.output, APU_TR))
#define ns_out(a)\
	(a->muted[3] ? 0 : _apu_channel_volume_adjust(a->NS.output, APU_NS))
#define dmc_out(a)\
	(a->muted[4] ? 0 : _apu_channel_volume_adjust(a->DMC.output, APU_DMC))
#define extra_out(ch)\
	(ch * cfg->apu.channel[APU_EXTRA])
#define pulse_output(a)\
	nla_table.pulse[(int) (s1_out(a) + s2_out(a))]
#define tnd_output(a)\
	nla_table.tnd[(int) ((tr_out(a) * 3) + (ns_out(a) * 2) + dmc_out(a))]

typedef struct _config_apu {
	BYTE channel[APU_MASTER + 1];
	double volume[APU_MASTER + 1];
} _config_apu;
typedef struct _apu {
	BYTE mode;
	BYTE type;
	BYTE step;
	BYTE length_clocked;
	BYTE DMC;
	SWORD cycles;

  int cpu_cycles;
  int cpu_opcode_cycle;
  BYTE odd_cycle;

	/* ------------------------------------------------------- */
	/* questi valori non e' necessario salvarli nei savestates */
	/* ------------------------------------------------------- */
	/* */ BYTE clocked;                                     /* */
	/* ------------------------------------------------------- */
} _apu;
typedef struct _r4011 {
	BYTE value;
	DBWORD frames;
	DBWORD cycles;
	SWORD output;
} _r4011;
typedef struct _r4015 {
	BYTE value;
} _r4015;
typedef struct _r4017 {
	BYTE value;
	struct _r4017_litter {
		BYTE value;
		BYTE delay;
	} jitter;
	BYTE reset_frame_delay;
} _r4017;
typedef struct _envelope {
	BYTE enabled;
	BYTE divider;
	BYTE counter;
	BYTE constant_volume;
	SBYTE delay;
} _envelope;
typedef struct _sweep {
	BYTE enabled;
	BYTE negate;
	BYTE divider;
	BYTE shift;
	BYTE reload;
	BYTE silence;
	SBYTE delay;
} _sweep;
typedef struct _length_counter {
	BYTE value;
	BYTE enabled;
	BYTE halt;
} _length_counter;
typedef struct _linear_counter {
	BYTE value;
	BYTE reload;
	BYTE halt;
} _linear_counter;
typedef struct _apuSquare {
	/* timer */
	DBWORD timer;
	/* ogni quanti cicli devo generare un output */
	WORD frequency;
	/* duty */
	BYTE duty;
	/* envelope */
	_envelope envelope;
	/* volume */
	BYTE volume;
	/* sequencer */
	BYTE sequencer;
	/* sweep */
	_sweep sweep;
	/* length counter */
	_length_counter length;
	/* output */
	SWORD output;
} _apuSquare;
typedef struct _apuTriangle {
	/* timer */
	DBWORD timer;
	/* ogni quanti cicli devo generare un output */
	WORD frequency;
	/* linear counter */
	_linear_counter linear;
	/* length counter */
	_length_counter length;
	/* sequencer */
	BYTE sequencer;
	/* output */
	SWORD output;
} _apuTriangle;
typedef struct _apuNoise {
	/* timer */
	DBWORD timer;
	/* ogni quanti cicli devo generare un output */
	WORD frequency;
	/* envelope */
	_envelope envelope;
	/* specifico del noise */
	BYTE mode;
	/* volume */
	BYTE volume;
	/* shift register */
	WORD shift;
	/* length counter */
	_length_counter length;
	/* sequencer */
	BYTE sequencer;
	/* output */
	SWORD output;
} _apuNoise;
typedef struct _apuDMC {
	/* ogni quanti cicli devo generare un output */
	WORD frequency;

	WORD remain;
	BYTE irq_enabled;
	BYTE loop;
	BYTE rate_index;
	WORD address_start;
	DBWORD address;
	WORD length;
	BYTE counter;
	BYTE empty;
	BYTE buffer;

	/* DMA */
	BYTE dma_cycle;

	/* output unit */
	BYTE silence;
	BYTE shift;
	BYTE counter_out;

	/* output */
	SWORD output;

	/* misc */
	BYTE tick_type;
}  _apuDMC;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC struct _nla_table {
	  SWORD pulse[32];
	  SWORD tnd[203];
};

extern struct _nla_table nla_table;

EXTERNC struct NESAPU {
  _apu apu;
  _r4011 r4011;
  _r4015 r4015;
  _r4017 r4017;
  _apuSquare S1, S2;
  _apuTriangle TR;
  _apuNoise NS;
  _apuDMC DMC;
  void* readDMCUser;
  unsigned char (*readDMC)(void*,unsigned short);
  unsigned char muted[5];
};

/* apuPeriod[mode][type][cycles] */
static const WORD apuPeriod[2][3][7] = {
	/*
	 * Mode 0: 4-step sequence
	 * Action      Envelopes &     Length Counter& Interrupt   Delay to next
	 *             Linear Counter  Sweep Units     Flag        NTSC     PAL   Dendy
	 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	 * $4017=$00   -               -               -           7459    8315    7459
	 * Step 1      Clock           -               -           7456    8314    7456
	 * Step 2      Clock           Clock           -           7458    8312    7458
	 * Step 3      Clock           -               -           7458    8314    7458
	 * Step 4      Clock           Clock       Set if enabled  7458    8314    7458
	 */
	{
		{7459, 7456, 7458, 7457, 1, 1, 7457},
		{8315, 8314, 8312, 8313, 1, 1, 8313},
		{7459, 7456, 7458, 7457, 1, 1, 7457}
	},
	/*
	 * Mode 1: 5-step sequence
	 * Action      Envelopes &     Length Counter& Interrupt   Delay to next
	 *             Linear Counter  Sweep Units     Flag        NTSC     PAL   Dendy
	 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	 * $4017=$80   -               -               -              1       1       1
	 * Step 1      Clock           Clock           -           7458    8314    7458
	 * Step 2      Clock           -               -           7456    8314    7456
	 * Step 3      Clock           Clock           -           7458    8312    7458
	 * Step 4      Clock           -               -           7458    8314    7458
	 * Step 5      -               -               -           7452    8312    7452
	 *
	 * Note:
	 * il 7452 e il 8312 dello step 5 diventano 7451 e 8311
	 * nella mia tabella perche' il ciclo mancante lo eseguo
	 * all'inizio del ciclo successivo.
	 */
	{
		{1, 7458, 7456, 7458, 7458, 7451, 0},
		{1, 8314, 8314, 8312, 8314, 8311, 0},
		{1, 7458, 7456, 7458, 7458, 7451, 0}
	}
};

/* la tabella con i valori da caricare nel length counter del canale */
static const BYTE length_table[32] = {
	0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06,
	0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
	0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
	0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E
};

static const BYTE square_duty[2][4][8] = {
	{
		{ 1,  0,  0,  0,  0,  0,  0,  0},
		{ 1,  1,  0,  0,  0,  0,  0,  0},
		{ 1,  1,  1,  1,  0,  0,  0,  0},
		{ 0,  0,  1,  1,  1,  1,  1,  1}
	},
	{
		{ 1,  0,  0,  0,  0,  0,  0,  0},
		{ 1,  1,  1,  1,  0,  0,  0,  0},
		{ 1,  1,  0,  0,  0,  0,  0,  0},
		{ 0,  0,  1,  1,  1,  1,  1,  1}
	},
};

static const BYTE triangle_duty[32] = {
	0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08,
	0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
};

static const WORD noise_timer[3][16] = {
	{
		0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0060, 0x0080, 0x00A0,
		0x00CA, 0x00FE, 0x017C, 0x01FC, 0x02FA, 0x03F8, 0x07F2, 0x0FE4
	},
	{
		0x0004, 0x0007, 0x000E, 0x001E, 0x003C, 0x0058, 0x0076, 0x0094,
		0x00BC, 0x00EC, 0x0162, 0x01D8, 0x02C4, 0x03B0, 0x0762, 0x0EC2
	},
	{
		0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0060, 0x0080, 0x00A0,
		0x00CA, 0x00FE, 0x017C, 0x01FC, 0x02FA, 0x03F8, 0x07F2, 0x0FE4
	}
};

static const WORD dmc_rate[3][16] = {
	{
		0x01AC, 0x017C, 0x0154, 0x0140, 0x011E, 0x00FE, 0x00E2, 0x00D6,
		0x00BE, 0x00A0, 0x008E, 0x0080, 0x006A, 0x0054, 0x0048, 0x0036
	},
	{
		0x018E, 0x0162, 0x013C, 0x012A, 0x0114, 0x00EC, 0x00D2, 0x00C6,
		0x00B0, 0x0094, 0x0084, 0x0076, 0x0062, 0x004E, 0x0042, 0x0032
	},
	{
		0x01AC, 0x017C, 0x0154, 0x0140, 0x011E, 0x00FE, 0x00E2, 0x00D6,
		0x00BE, 0x00A0, 0x008E, 0x0080, 0x006A, 0x0054, 0x0048, 0x0036
	}
};

EXTERNC void apu_tick(struct NESAPU* a, BYTE *hwtick);
EXTERNC void apu_turn_on(struct NESAPU* a, BYTE apu_type);

#undef EXTERNC

#endif /* APU_H_ */
