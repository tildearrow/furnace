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

#include <string.h>
#include "apu.h"

void apu_tick(struct NESAPU* a, BYTE *hwtick) {
	/* sottraggo il numero di cicli eseguiti */
	a->apu.cycles--;
	/*
	 * questo flag sara' a TRUE solo nel ciclo
	 * in cui viene eseguito il length counter.
	 */
	a->apu.length_clocked = FALSE;
	/*
	 * se e' settato il delay del $4017, essendo
	 * questo il ciclo successivo, valorizzo il
	 * registro.
	 */
#if defined (VECCHIA_GESTIONE_JITTER)
	if (r4017.jitter.delay) {
		r4017.jitter.delay = FALSE;
		r4017_jitter();
	}
#else
	if (a->r4017.jitter.delay) {
		a->r4017.jitter.delay = FALSE;
		r4017_jitter(0)
	}
	r4017_reset_frame()
#endif

	/* quando apu.cycles e' a 0 devo eseguire uno step */
	if (!a->apu.cycles) {
		switch (a->apu.step) {
			case 0:
				/*
				 * nel mode 1 devo eseguire il
				 * length counter e lo sweep.
				 */
				if (a->apu.mode == APU_48HZ) {
					length_clock()
					sweep_clock()
				}
				envelope_clock()
				/* triangle's linear counter */
				linear_clock()
				/* passo al prossimo step */
				apu_change_step(++a->apu.step);
				break;
			case 1:
				/* nel mode 0 devo eseguire il length counter */
				if (a->apu.mode == APU_60HZ) {
					length_clock()
					sweep_clock()
				}
				envelope_clock()
				/* triangle's linear counter */
				linear_clock()
				/* passo al prossimo step */
				apu_change_step(++a->apu.step);
				break;
			case 2:
				/*
				 * nel mode 1 devo eseguire il
				 * length counter e lo sweep.
				 */
				if (a->apu.mode == APU_48HZ) {
					length_clock()
					sweep_clock()
				}
				envelope_clock()
				/* triangle's linear counter */
				linear_clock()
				/* passo al prossimo step */
				apu_change_step(++a->apu.step);
				break;
			case 3:
				/*
				 * gli step 3, 4 e 5 settano il bit 6 del $4015
				 * ma solo nel 4 genero un IRQ.
				 */
				if (a->apu.mode == APU_60HZ) {
					/*
					 * se e' a 0 il bit 6 del $4017 (interrupt
					 * inhibit flag) allora devo generare un IRQ.
					 */
					if (!(a->r4017.value & 0x40)) {
						/* setto il bit 6 del $4015 */
						a->r4015.value |= 0x40;
					}
				} else {
					/* nel mode 1 devo eseguire l'envelope */
					envelope_clock()
					/* triangle's linear counter */
					linear_clock()
				}
				/* passo al prossimo step */
				apu_change_step(++a->apu.step);
				break;
			case 4:
				/*
				 * gli step 3, 4 e 5 settano il bit 6 del $4015
				 * ma solo nel 4 genero un IRQ.
				 */
				if (a->apu.mode == APU_60HZ) {
					length_clock()
					sweep_clock()
					envelope_clock()
					/* triangle's linear counter */
					linear_clock()
					/*
					 * se e' a 0 il bit 6 del $4017 (interrupt
					 * inhibit flag) allora devo generare un IRQ.
					 */
					if (!(a->r4017.value & 0x40)) {
						/* setto il bit 6 del $4015 */
						a->r4015.value |= 0x40;
					}
				}
				/* passo al prossimo step */
				apu_change_step(++a->apu.step);
				break;
			case 5:
				/*
				 * gli step 3, 4 e 5 settano il bit 6 del $4015
				 * ma solo nel 4 genero un IRQ.
				 */
				if (a->apu.mode == APU_60HZ) {
					/*
					 * se e' a 0 il bit 6 del $4017 (interrupt
					 * inhibit flag) allora devo generare un IRQ.
					 */
					if (!(a->r4017.value & 0x40)) {
						/* setto il bit 6 del $4015 */
						a->r4015.value |= 0x40;
					}
					a->apu.step++;
				} else {
					/* nel mode 1 devo ricominciare il ciclo */
					a->apu.step = 0;
				}
				/* passo al prossimo step */
				apu_change_step(a->apu.step);
				break;
			case 6:
				/* da qui ci passo solo nel mode 0 */
				envelope_clock()
				/* triangle's linear counter */
				linear_clock()
				/* questo e' il passaggio finale del mode 0 */
				a->apu.step = 1;
				/* passo al prossimo step */
				apu_change_step(a->apu.step);
				break;
		}
	}

	/*
	 * eseguo un ticket per ogni canale
	 * valorizzandone l'output.
	 */
	square_tick(a->S1, 0, a->apu)
	square_tick(a->S2, 0, a->apu)
	triangle_tick()
	noise_tick()
	dmc_tick()

        // TODO
	/*if (snd_apu_tick) {
		snd_apu_tick();
	}*/

	a->r4011.cycles++;
}
void apu_turn_on(struct NESAPU* a, BYTE apu_type) {
	memset(&a->apu, 0x00, sizeof(a->apu));
	memset(&a->r4015, 0x00, sizeof(a->r4015));
	memset(&a->r4017, 0x00, sizeof(a->r4017));
	/* azzero tutte le variabili interne dei canali */
	memset(&a->S1, 0x00, sizeof(a->S1));
	memset(&a->S2, 0x00, sizeof(a->S2));
	memset(&a->TR, 0x00, sizeof(a->TR));
	memset(&a->NS, 0x00, sizeof(a->NS));
	memset(&a->DMC, 0x00, sizeof(a->DMC));
	/* al reset e' sempre settato a 60Hz */
	a->apu.mode = APU_60HZ;
  /* per favore non fatemi questo... e' terribile */
	a->apu.type = apu_type;
	apu_change_step(a->apu.step);
	/* valori iniziali dei vari canali */
	a->S1.frequency = 1;
	a->S1.sweep.delay = 1;
	a->S1.sweep.divider = 1;
	a->S2.frequency = 1;
	a->S2.sweep.delay = 1;
	a->S2.sweep.divider = 1;
	a->TR.frequency = 1;
	a->TR.sequencer = 0;
	a->NS.frequency = 1;
	a->NS.shift = 1;
	a->DMC.frequency = 1;
	a->DMC.empty = TRUE;
	a->DMC.silence = TRUE;
	a->DMC.counter_out = 8;
	// sembra che l'address del DMC al power on dia valorizzato a 0xC000
	// e la lunghezza del sample sia settato a 1 byte.
	// http://forums.nesdev.com/viewtopic.php?f=3&t=18278
	a->DMC.length = 1;
	a->DMC.address_start = 0xC000;
  a->apu.odd_cycle = 0;
  // come non viene inizializzato? Vorrei qualche spiegazione...
  a->r4011.frames = 0;
}
