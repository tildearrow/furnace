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

#include <stdio.h>
#include <string.h>
#include "fds.h"

enum { TRANSFERED_8BIT = 0x02, END_OF_HEAD = 0x40 };

static const BYTE volume_wave[4] = { 39, 26, 19, 15 };

void fds_reset(struct _fds* fds) {
  memset(fds,0,sizeof(struct _fds));
}

void extcl_apu_tick_FDS(struct _fds* fds) {
  SWORD freq;

  /* volume unit */
  if (fds->snd.volume.mode) {
    fds->snd.volume.gain = fds->snd.volume.speed;
  } else if (!fds->snd.envelope.disabled && fds->snd.envelope.speed) {
    if (fds->snd.volume.counter) {
      fds->snd.volume.counter--;
    } else {
      fds->snd.volume.counter = (fds->snd.envelope.speed << 3) * (fds->snd.volume.speed + 1);
      if (fds->snd.volume.increase) {
        if (fds->snd.volume.gain < 32) {
          fds->snd.volume.gain++;
        }
      } else if (fds->snd.volume.gain) {
        fds->snd.volume.gain--;
      }
    }
  }

  /* sweep unit */
  if (fds->snd.sweep.mode) {
    fds->snd.sweep.gain = fds->snd.sweep.speed;
  } else if (!fds->snd.envelope.disabled && fds->snd.envelope.speed) {
    if (fds->snd.sweep.counter) {
      fds->snd.sweep.counter--;
    } else {
      fds->snd.sweep.counter = (fds->snd.envelope.speed << 3) * (fds->snd.sweep.speed + 1);
      if (fds->snd.sweep.increase) {
        if (fds->snd.sweep.gain < 32) {
          fds->snd.sweep.gain++;
        }
      } else if (fds->snd.sweep.gain) {
        fds->snd.sweep.gain--;
      }
    }
  }

  /* modulation unit */
  freq = fds->snd.main.frequency;

  if (!fds->snd.modulation.disabled && fds->snd.modulation.frequency) {
    if ((fds->snd.modulation.counter -= fds->snd.modulation.frequency) < 0) {
      SWORD temp, temp2, a, d;
      SBYTE adj = fds->snd.modulation.data[fds->snd.modulation.index];

      fds->snd.modulation.counter += 65536;

      if (++fds->snd.modulation.index == 64) {
        fds->snd.modulation.index = 0;
      }

      if (adj == -4) {
        fds->snd.sweep.bias = 0;
      } else {
        fds->snd.sweep.bias += adj;
      }

      temp = fds->snd.sweep.bias * ((fds->snd.sweep.gain < 32) ? fds->snd.sweep.gain : 32);

      a = 64;
      d = 0;

      if (temp <= 0) {
        d = 15;
      } else if (temp < 3040) { //95 * 32
        a = 66;
        d = -31;
      }

      temp2 = a + (SBYTE) ((temp - d) / 16 - a);

      fds->snd.modulation.mod = freq * temp2 / 64;
    }

    if (freq) {
      freq += fds->snd.modulation.mod;
    }
  }

  /* main unit */
  if (fds->snd.main.silence) {
    fds->snd.main.output = 0;
    return;
  }

  if (freq && !fds->snd.wave.writable) {
    if ((fds->snd.wave.counter -= freq) < 0) {
      WORD level;

      fds->snd.wave.counter += 65536;

      level = (fds->snd.volume.gain < 32 ? fds->snd.volume.gain : 32)
        * volume_wave[fds->snd.wave.volume];

      /* valore massimo dell'output (63 * (39 * 32)) = 78624 */
      /*fds->snd.main.output = (fds->snd.wave.data[fds->snd.wave.index] * level) >> 4;*/
      fds->snd.main.output = (fds->snd.wave.data[fds->snd.wave.index] * level) >> 3;

      if (++fds->snd.wave.index == 64) {
        fds->snd.wave.index = 0;
      }

      fds->snd.wave.clocked = TRUE;
    }
  }
}
