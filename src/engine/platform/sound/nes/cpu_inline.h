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

#ifndef CPU_INLINE_H_
#define CPU_INLINE_H_

#include <stdlib.h>
#include "apu.h"
#include "fds.h"

#define mod_cycles_op(op, vl) cpu.cycles op vl
#define r2006_during_rendering()\
  if (!ppu.vblank && r2001.visible && (ppu.frame_y > ppu_sclines.vint) &&\
    (ppu.screen_y < SCR_LINES)) {\
    _r2006_during_rendering()\
  } else {\
    r2006.value += r2000.r2006_inc;\
  }
#define _r2006_during_rendering()\
  r2006_inc()\
  if ((r2006.value & 0x1F) == 0x1F) {\
    r2006.value ^= 0x41F;\
  } else {\
    r2006.value++;\
  }

INLINE static void apu_wr_reg(struct NESAPU* a, int ts, WORD address, BYTE value) {
  apu_tick(a,ts);

  if (!(address & 0x0010)) {
    /* -------------------- square 1 --------------------*/
    if (address <= 0x4003) {
      if (address == 0x4000) {
        square_reg0(a->S1);
        return;
      }
      if (address == 0x4001) {
        square_reg1(a->S1);
        sweep_silence(a->S1)
        return;
      }
      if (address == 0x4002) {
        square_reg2(a->S1);
        sweep_silence(a->S1)
        return;
      }
      if (address == 0x4003) {
        square_reg3(a->S1,a->apu.length_clocked);
        sweep_silence(a->S1)
        return;
      }
      return;
    }
    /* -------------------- square 2 --------------------*/
    if (address <= 0x4007) {
      if (address == 0x4004) {
        square_reg0(a->S2);
        return;
      }
      if (address == 0x4005) {
        square_reg1(a->S2);
        sweep_silence(a->S2)
        return;
      }
      if (address == 0x4006) {
        square_reg2(a->S2);
        sweep_silence(a->S2)
        return;
      }
      if (address == 0x4007) {
        square_reg3(a->S2,a->apu.length_clocked);
        sweep_silence(a->S2)
        return;
      }
      return;
    }
    /* -------------------- triangle --------------------*/
    if (address <= 0x400B) {
      if (address == 0x4008) {
        /* length counter */
        /*
         * il triangle ha una posizione diversa per il
         * flag LCHalt.
         */
        a->TR.length.halt = value & 0x80;
        /* linear counter */
        a->TR.linear.reload = value & 0x7F;
        return;
      }
      if (address == 0x400A) {
        /* timer (low 8 bits) */
        a->TR.timer = (a->TR.timer & 0x0700) | value;
        return;
      }
      if (address == 0x400B) {
        /* length counter */
        /*
         * se non disabilitato, una scrittura in
         * questo registro, carica immediatamente il
         * length counter del canale, tranne nel caso
         * in cui la scrittura avvenga nello stesso
         * momento del clock di un length counter e
         * con il length diverso da zero.
         */
        if (a->TR.length.enabled && !(a->apu.length_clocked && a->TR.length.value)) {
          a->TR.length.value = length_table[value >> 3];
        }
        /* timer (high 3 bits) */
        a->TR.timer = (a->TR.timer & 0x00FF) | ((value & 0x07) << 8);
        /*
         * scrivendo in questo registro si setta
         * automaticamente l'halt flag del triangle.
         */
        a->TR.linear.halt = TRUE;
        return;
      }
      return;
    }
    /* --------------------- noise ----------------------*/
    if (address <= 0x400F) {
      if (address == 0x400C) {
        a->NS.length.halt = value & 0x20;
        /* envelope */
        a->NS.envelope.constant_volume = value & 0x10;
        a->NS.envelope.divider = value & 0x0F;
        return;
      }
      if (address == 0x400E) {
        a->NS.mode = value & 0x80;
        a->NS.timer = value & 0x0F;
        return;
      }
      if (address == 0x400F) {
        /*
         * se non disabilitato, una scrittura in
         * questo registro, carica immediatamente il
         * length counter del canale, tranne nel caso
         * in cui la scrittura avvenga nello stesso
         * momento del clock di un length counter e
         * con il length diverso da zero.
         */
        if (a->NS.length.enabled && !(a->apu.length_clocked && a->NS.length.value)) {
          a->NS.length.value = length_table[value >> 3];
        }
        /* envelope */
        a->NS.envelope.enabled = TRUE;
        return;
      }
      return;
    }
    return;
  } else {
    /* ---------------------- DMC -----------------------*/
    if (address <= 0x4013) {
      if (address == 0x4010) {
        a->DMC.irq_enabled = value & 0x80;
        /* se l'irq viene disabilitato allora... */
        if (!a->DMC.irq_enabled) {
          /* ...azzero l'interrupt flag del DMC */
          a->r4015.value &= 0x7F;
        }
        a->DMC.loop = value & 0x40;
        a->DMC.rate_index = value & 0x0F;
        return;
      }
      if (address == 0x4011) {
        BYTE save = a->DMC.counter;

        value &= 0x7F;

        /*
         * questa lo faccio perche' in alcuni giochi come Batman,
         * Ninja Gaiden 3, Castlevania II ed altri, producono
         * un popping del suono fastidioso;
         * from Fceu doc:
         * Why do some games make a popping sound (Batman, Ninja Gaiden 3,
         * Castlevania II etc.)? These games do a very crude drum imitation
         * by causing a large jump in the output level for a short period of
         * time via the register at $4011. The analog filters on a real
         * Famicom make it sound decent(better). I have not completely
         * emulated these filters.
         * (Xodnizel)
         */
        if (a->r4011.frames > 1) {
          a->r4011.output = (value - save) >> 3;
          a->DMC.counter = a->DMC.output = save + a->r4011.output;
        } else {
          a->DMC.counter = a->DMC.output = value;
        }
        a->oscBuf[4]->putSample(a->timestamp,a->muted[4]?0:(a->DMC.output<<8));

        a->r4011.cycles = a->r4011.frames = 0;
        a->r4011.value = value;
        return;
      }
      if (address == 0x4012) {
        a->DMC.address_start = (value << 6) | 0xC000;
        return;
      }
      if (address == 0x4013) {
        /* sample length */
        a->DMC.length = (value << 4) | 0x01;
        return;
      }
      return;
    }
    /* --------------------------------------------------*/
    if (address == 0x4015) {
      /*
       * 76543210
       * || |||||
       * || ||||+- Pulse channel 1's length counter enabled flag
       * || |||+-- Pulse channel 2's length counter enabled flag
       * || ||+--- Triangle channel's length counter enabled flag
       * || |+---- Noise channel's length counter enabled flag
       * || +----- If clear, the DMC's bytes remaining is set to 0,
       * ||        otherwise the DMC sample is restarted only if the
       * ||        DMC's bytes remaining is 0
       * |+------- Frame interrupt flag
       * +-------- DMC interrupt flag
       */
      /*
       * dopo la write il bit 7 (dmc flag) deve
       * essere azzerato mentre lascio inalterati
       * i bit 5 e 6.
       */
      a->r4015.value = (a->r4015.value & 0x60) | (value & 0x1F);
      /*
       * quando il flag di abilitazione del length
       * counter di ogni canale e' a 0, il counter
       * dello stesso canale e' immediatamente azzerato.
       */
      if (!(a->S1.length.enabled = a->r4015.value & 0x01)) {
        a->S1.length.value = 0;
      }
      if (!(a->S2.length.enabled = a->r4015.value & 0x02)) {
        a->S2.length.value = 0;
      }
      if (!(a->TR.length.enabled = a->r4015.value & 0x04)) {
        a->TR.length.value = 0;
      }
      if (!(a->NS.length.enabled = a->r4015.value & 0x08)) {
        a->NS.length.value = 0;
      }
      /*
       * se il bit 4 e' 0 allora devo azzerare i bytes
       * rimanenti del DMC, alrimenti devo riavviare
       * la lettura dei sample DMC solo nel caso che
       * in cui i bytes rimanenti siano a 0.
       */
      if (!(a->r4015.value & 0x10)) {
        a->DMC.remain = 0;
        a->DMC.empty = TRUE;
      } else if (!a->DMC.remain) {
        a->DMC.remain = a->DMC.length;
        a->DMC.address = a->DMC.address_start;
      }
      return;
    }

#if defined (VECCHIA_GESTIONE_JITTER)
    if (address == 0x4017) {
      /* APU frame counter */
      r4017.jitter.value = value;
      /*
       * nell'2A03 se la scrittura del $4017 avviene
       * in un ciclo pari, allora l'effettiva modifica
       * avverra' nel ciclo successivo.
       */
      if (cpu.odd_cycle) {
        r4017.jitter.delay = TRUE;
      } else {
        r4017.jitter.delay = FALSE;
        r4017_jitter();
      }
      return;
    }
#else
    if (address == 0x4017) {
      /* APU frame counter */
      a->r4017.jitter.value = value;
      /*
       * nell'2A03 se la scrittura del $4017 avviene
       * in un ciclo pari, allora l'effettiva modifica
       * avverra' nel ciclo successivo.
       */
      if (a->apu.odd_cycle) {
        a->r4017.jitter.delay = TRUE;
      } else {
        a->r4017.jitter.delay = FALSE;
        r4017_jitter(1)
        r4017_reset_frame()
      }
      return;
    }
#endif
  }

#if defined (DEBUG)
    //fprintf(stderr, "Alert: Attempt to write APU port %04X\n", address);
#endif

  return;
}

INLINE static BYTE fds_wr_mem(struct _fds* fds, WORD address, BYTE value) {
  if (address == 0x4023) {
    fds->enabled_snd_reg=value&0x02;
  }
  if ((address >= 0x4040) && (address <= 0x408A)) {
    if (fds->enabled_snd_reg) {
      if ((address >= 0x4040) && (address <= 0x407F)) {
        fds->snd.wave.data[address & 0x003F] = value & 0x3F;
        return (TRUE);
      }
      if (address == 0x4080) {
        fds->snd.volume.speed = value & 0x3F;
        fds->snd.volume.increase = value & 0x40;
        fds->snd.volume.mode = value & 0x80;
        return (TRUE);
      }
      if (address == 0x4082) {
        fds->snd.main.frequency = (fds->snd.main.frequency & 0xFF00) | value;
        return (TRUE);
      }
      if (address == 0x4083) {
        fds->snd.main.frequency = ((value & 0x0F) << 8) | (fds->snd.main.frequency & 0x00FF);
        fds->snd.envelope.disabled = value & 0x40;
        fds->snd.main.silence = value & 0x80;
        return (TRUE);
      }
      if (address == 0x4084) {
        fds->snd.sweep.speed = value & 0x3F;
        fds->snd.sweep.increase = value & 0x40;
        fds->snd.sweep.mode = value & 0x80;
        return (TRUE);
      }
      if (address == 0x4085) {
        fds->snd.sweep.bias = ((SBYTE) (value << 1)) / 2;
        fds->snd.modulation.index = 0;
        return (TRUE);
      }
      if (address == 0x4086) {
        fds->snd.modulation.frequency = (fds->snd.modulation.frequency & 0xFF00) | value;
        return (TRUE);
      }
      if (address == 0x4087) {
        fds->snd.modulation.frequency = ((value & 0x0F) << 8)
                  | (fds->snd.modulation.frequency & 0x00FF);
        fds->snd.modulation.disabled = value & 0x80;
        return (TRUE);
      }
      if (address == 0x4088) {
        BYTE i;

        // 0,2,4,6,-8,-6,-4,-2
        for (i = 0; i < 32; i++) {
          BYTE a = i << 1;

          if (i < 31) {
            fds->snd.modulation.data[a] = fds->snd.modulation.data[a + 2];
          } else {
            BYTE tmp = ((value & 0x03) | (0x3F * (value & 0x04)));
            fds->snd.modulation.data[a] = (SBYTE) tmp;
          }
          fds->snd.modulation.data[a + 1] = fds->snd.modulation.data[a];
        }
        return (TRUE);
      }
      if (address == 0x4089) {
        fds->snd.wave.writable = value & 0x80;
        fds->snd.wave.volume = value & 0x03;
        return (TRUE);
      }
      if (address == 0x408A) {
        fds->snd.envelope.speed = value;
        return (TRUE);
      }
    }
  }

  return (FALSE);
}
#endif /* CPU_INLINE_H_ */
