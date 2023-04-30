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

*/

#ifndef _D65010G031_EMU_H
#define _D65010G031_EMU_H

#ifdef __cplusplus
extern "C"
{
#endif

struct d65010g031_square_t
{
	unsigned char period; // Period (0 = Off)
	int counter; // clock counter
	signed short out; // output
};

struct d65010g031_t
{
	struct d65010g031_square_t square[3];
	signed short out[3];
	unsigned char ctrl;
};

int d65010g031_square_tick(struct d65010g031_square_t *square, const int cycle);

int d65010g031_sound_tick(struct d65010g031_t *d65010g031, const int cycle);

void d65010g031_reset(struct d65010g031_t *d65010g031);

void d65010g031_write(struct d65010g031_t *d65010g031, const unsigned char a, const unsigned char d);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _D65010G031_EMU_H
