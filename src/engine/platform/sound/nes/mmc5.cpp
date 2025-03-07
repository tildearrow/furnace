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

#include <string.h>
#include "apu.h"
#include "mmc5.h"

enum { MODE0, MODE1, MODE2, MODE3 };
enum { CHR_S, CHR_B };
enum { SPLIT_LEFT, SPLIT_RIGHT = 0x40 };

BYTE prg_ram_mode;

void map_init_MMC5(struct _mmc5* mmc5) {
  memset(mmc5,0,sizeof(struct _mmc5));

  mmc5->S3.frequency = 1;
  mmc5->S4.frequency = 1;
  mmc5->S3.length.enabled = 0;
  mmc5->S3.length.value = 0;
  mmc5->S4.length.enabled = 0;
  mmc5->S4.length.value = 0;

  mmc5->timestamp = 0;
  mmc5->lastSample = 0;
  mmc5->bb = NULL;
  mmc5->oscBuf[0] = NULL;
  mmc5->oscBuf[1] = NULL;
  mmc5->oscBuf[2] = NULL;

  mmc5->S3.timer = 2048;
  mmc5->S4.timer = 2048;
}
void extcl_cpu_wr_mem_MMC5(struct _mmc5* mmc5, int ts, WORD address, BYTE value) {
  if (address < 0x5000) {
    return;
  }

  extcl_apu_tick_MMC5(mmc5,ts);

  switch (address) {
    case 0x5000:
      square_reg0(mmc5->S3);
      return;
    case 0x5001:
      /* lo sweep non e' utilizzato */
      return;
    case 0x5002:
      square_reg2(mmc5->S3);
      return;
    case 0x5003:
      square_reg3(mmc5->S3,0);
      return;
    case 0x5004:
      square_reg0(mmc5->S4);
      return;
    case 0x5005:
      /* lo sweep non e' utilizzato */
      return;
    case 0x5006:
      square_reg2(mmc5->S4);
      return;
    case 0x5007:
      square_reg3(mmc5->S4,0);
      return;
    case 0x5010:
      mmc5->pcm.enabled = ~value & 0x01;
      mmc5->pcm.output = 0;
      if (mmc5->pcm.enabled) {
        mmc5->pcm.output = mmc5->pcm.amp;
        mmc5->oscBuf[2]->putSample(mmc5->timestamp,mmc5->muted[2]?0:(mmc5->pcm.output<<7));
      }
      //mmc5->clocked = TRUE;
      return;
    case 0x5011:
      mmc5->pcm.amp = value;
      mmc5->pcm.output = 0;
      if (mmc5->pcm.enabled) {
        mmc5->pcm.output = mmc5->pcm.amp;
        mmc5->oscBuf[2]->putSample(mmc5->timestamp,mmc5->muted[2]?0:(mmc5->pcm.output<<7));
      }
      //mmc5->clocked = TRUE;
      return;
    case 0x5015:
      if (!(mmc5->S3.length.enabled = value & 0x01)) {
        mmc5->S3.length.value = 0;
      }
      if (!(mmc5->S4.length.enabled = value & 0x02)) {
        mmc5->S4.length.value = 0;
      }
      return;
  }
}
void extcl_length_clock_MMC5(struct _mmc5* mmc5) {
  length_run(mmc5->S3)
  length_run(mmc5->S4)
}
void extcl_envelope_clock_MMC5(struct _mmc5* mmc5) {
  envelope_run(mmc5->S3)
  envelope_run(mmc5->S4)
}
void extcl_apu_tick_MMC5(struct _mmc5* mmc5, int len) {
  if (len<=mmc5->timestamp) return;

  int rem=len-mmc5->timestamp;
  if (rem>1) {
    // output now just in case
    int sample=mmc5->muted[0]?0:(mmc5->S3.output*10);
    if (!mmc5->muted[1]) {
      sample+=mmc5->S4.output*10;
    }
    if (!mmc5->muted[2]) {
      sample+=mmc5->pcm.output*2;
    }
    if (sample!=mmc5->lastSample) {  
      blip_add_delta(mmc5->bb,mmc5->timestamp,sample-mmc5->lastSample);
      mmc5->lastSample=sample;
    }
  }

  while (rem>0) {
    // predict advance
    int advance=rem;
    if (advance>mmc5->S3.frequency) {
      advance=mmc5->S3.frequency;
    }
    if (advance>mmc5->S4.frequency) {
      advance=mmc5->S4.frequency;
    }
    if (advance<1) advance=1;

    int postTS=mmc5->timestamp+advance-1;

    // SQUARE 3 TICK
    if (!(mmc5->S3.frequency-=advance)) {
      square_output(mmc5->S3, 0)
      mmc5->S3.frequency = (mmc5->S3.timer + 1) << 1;
      mmc5->S3.sequencer = (mmc5->S3.sequencer + 1) & 0x07;
      mmc5->oscBuf[0]->putSample(postTS,mmc5->muted[0]?0:(mmc5->S3.output<<11));
    }

    // SQUARE 4 TICK
    if (!(mmc5->S4.frequency-=advance)) {
      square_output(mmc5->S4, 0)
      mmc5->S4.frequency = (mmc5->S4.timer + 1) << 1;
      mmc5->S4.sequencer = (mmc5->S4.sequencer + 1) & 0x07;
      mmc5->oscBuf[1]->putSample(postTS,mmc5->muted[1]?0:(mmc5->S4.output<<11));
    }

    // output sample
    mmc5->timestamp=postTS;
    int sample=mmc5->muted[0]?0:(mmc5->S3.output*10);
    if (!mmc5->muted[1]) {
      sample+=mmc5->S4.output*10;
    }
    if (!mmc5->muted[2]) {
      sample+=mmc5->pcm.output*2;
    }
    if (sample!=mmc5->lastSample) {  
      blip_add_delta(mmc5->bb,mmc5->timestamp,sample-mmc5->lastSample);
      mmc5->lastSample=sample;
    }
    
    rem-=advance;
    mmc5->timestamp++;
  }
  mmc5->timestamp=len;
}
