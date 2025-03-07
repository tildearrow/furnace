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

const BYTE filler_attrib[4] = {0x00, 0x55, 0xAA, 0xFF};
BYTE prg_ram_mode;

void map_init_MMC5(struct _mmc5* mmc5) {
	memset(mmc5,0,sizeof(struct _mmc5));

	mmc5->S3.frequency = 1;
	mmc5->S4.frequency = 1;
	mmc5->S3.length.enabled = 0;
	mmc5->S3.length.value = 0;
	mmc5->S4.length.enabled = 0;
	mmc5->S4.length.value = 0;
}
void extcl_cpu_wr_mem_MMC5(struct _mmc5* mmc5, WORD address, BYTE value) {
	if (address < 0x5000) {
		return;
	}

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
			}
			mmc5->clocked = TRUE;
			return;
		case 0x5011:
			mmc5->pcm.amp = value;
			mmc5->pcm.output = 0;
			if (mmc5->pcm.enabled) {
				mmc5->pcm.output = mmc5->pcm.amp;
			}
			mmc5->clocked = TRUE;
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
void extcl_apu_tick_MMC5(struct _mmc5* mmc5) {
        // SQUARE 3 TICK
	if (!(--mmc5->S3.frequency)) {
		square_output(mmc5->S3, 0)
		mmc5->S3.frequency = (mmc5->S3.timer + 1) << 1;
		mmc5->S3.sequencer = (mmc5->S3.sequencer + 1) & 0x07;
		mmc5->clocked = TRUE;
	}

        // SQUARE 4 TICK
	if (!(--mmc5->S4.frequency)) {
		square_output(mmc5->S4, 0)
		mmc5->S4.frequency = (mmc5->S4.timer + 1) << 1;
		mmc5->S4.sequencer = (mmc5->S4.sequencer + 1) & 0x07;
		mmc5->clocked = TRUE;
	}
}
