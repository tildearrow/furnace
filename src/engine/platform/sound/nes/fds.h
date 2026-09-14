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

#ifndef FDS_H_
#define FDS_H_

#include "common.h"

enum fds_operations { FDS_OP_NONE, FDS_OP_READ, FDS_OP_WRITE };

struct _fds {
  // snd
  BYTE enabled_snd_reg;
  struct _fds_snd {
    struct _fds_snd_wave {
      BYTE data[64];
      BYTE writable;
      BYTE volume;

      BYTE index;
      int32_t counter;

    /* ------------------------------------------------------- */
    /* questi valori non e' necessario salvarli nei savestates */
    /* ------------------------------------------------------- */
    /* */ BYTE clocked;                                     /* */
    /* ------------------------------------------------------- */
    } wave;
    struct _fds_snd_envelope {
      BYTE speed;
      BYTE disabled;
    } envelope;
    struct _fds_snd_main {
      BYTE silence;
      WORD frequency;

      SWORD output;
    } main;
    struct _fds_snd_volume {
      BYTE speed;
      BYTE mode;
      BYTE increase;

      BYTE gain;
      uint32_t counter;
    } volume;
    struct _fds_snd_sweep {
      SBYTE bias;
      BYTE mode;
      BYTE increase;
      BYTE speed;

      BYTE gain;
      uint32_t counter;
    } sweep;
    struct _fds_snd_modulation {
      SBYTE data[64];
      WORD frequency;
      BYTE disabled;

      BYTE index;
      int32_t counter;
      SWORD mod;
    } modulation;
  } snd;
};

void extcl_apu_tick_FDS(struct _fds* fds);
void fds_reset(struct _fds* fds);

#undef EXTERNC

#endif /* FDS_H_ */
