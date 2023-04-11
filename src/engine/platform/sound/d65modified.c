/*

============================================================================

NEC D65010G031 sound emulator
by cam900

This file is licensed under zlib license.

============================================================================

zlib License

(C) 2023-present cam900 and contributors

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

============================================================================

TODO:
- needs hardware test

ALTERED VERSION!!!
ALTERED VERSION!!!
ALTERED VERSION!!!
ALTERED VERSION!!!
ALTERED VERSION!!!
ALTERED VERSION!!!
ALTERED VERSION!!!
ALTERED VERSION!!!
ALTERED VERSION!!!
ALTERED VERSION!!!
ALTERED VERSION!!!
ALTERED VERSION!!!
ALTERED VERSION!!!
ALTERED VERSION!!!


THIS IS **NOT** NOT NOT NOT!!!! THE ORIGINAL SOFTWARE
IT ISN'T
THE MODIFICATIONS THAT WERE MADE ARE:

1. FIX VOLUMES - APPARENTLY THE SQUARES HAVE DIFFERENT VOLUMES (thanks forple)

*/

#include "d65modified.h"
#include <stdlib.h>

static int d65010g031_max(int a, int b) { return (a > b) ? a : b; }

int d65010g031_square_tick(struct d65010g031_square_t *square, const int cycle)
{
	if (square->period > 0)
	{
		const int period = square->period;
		square->counter += cycle;
		while (square->counter >= period)
		{
			square->counter -= period;
			square->out ^= 1;
		}
		return square->out;
	}
	return 0;
}

// this is the bit I altered
// THIS IS **NOT** THE ORIGINAL SOFTWARE! I am plainly marking it as such!
const int d65Volumes[3]={
  3840,
  5120,
  8192
};

int d65010g031_sound_tick(struct d65010g031_t *d65010g031, const int cycle)
{
	int out = 0;
	for (int i = 0; i < 3; i++)
	{
		d65010g031->out[i] = 0;
	}
	if (d65010g031->ctrl & 2)
	{
		if (d65010g031->ctrl & 1)
		{
			int sout[3] = {
				d65010g031_square_tick(&d65010g031->square[0], cycle),
				d65010g031_square_tick(&d65010g031->square[1], cycle),
				d65010g031_square_tick(&d65010g031->square[2], cycle),
			};
			d65010g031->out[0] = (sout[0] ^ sout[1]) ? d65Volumes[0] : -d65Volumes[0];
			d65010g031->out[1] = (sout[1] ^ sout[2]) ? d65Volumes[1] : -d65Volumes[1];
			d65010g031->out[2] = (sout[2] ? d65Volumes[2] : -d65Volumes[2]);
		}
		else
		{
			for (int i = 0; i < 3; i++)
			{
				d65010g031->out[i] = d65010g031_square_tick(&d65010g031->square[i], cycle)?d65Volumes[i]:-d65Volumes[i];
			}
		}
		out = d65010g031->out[0] + d65010g031->out[1] + d65010g031->out[2];
	}
	return out;
}

void d65010g031_reset(struct d65010g031_t *d65010g031)
{
	for (int i = 0; i < 3; i++)
	{
		d65010g031->square[i].period = 0;
		d65010g031->square[i].counter = 0;
		d65010g031->square[i].out = 0;
	}
	d65010g031->ctrl = 0;
}

void d65010g031_write(struct d65010g031_t *d65010g031, const unsigned char a, const unsigned char d)
{
	switch (a)
	{
		case 3:
			d65010g031->ctrl = d;
			break;
		default:
			unsigned char per = (unsigned char)(~d) & 0x3f;
			if ((per == 0) && (d65010g031->square[a].period != 0))
			{
				d65010g031->square[a].out ^= 1;
			}
			d65010g031->square[a].period = per;
			break;
	}
}
