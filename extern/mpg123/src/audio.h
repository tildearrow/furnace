/*
	audio: audio output interface

	This is what is left after separating out libout123.

	copyright ?-2015 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Michael Hipp
*/

/* 
 * Audio 'LIB' defines
 */


#ifndef _MPG123_AUDIO_H_
#define _MPG123_AUDIO_H_

#include "compat/compat.h"
#include "mpg123.h"
#include "out123.h"

#define pitch_rate(rate)	(param.pitch == 0 ? (rate) : (long) ((param.pitch+1.0)*(rate)))
#define bpitch_rate(rate)	(param.pitch == 0 ? (rate) : (long) ((rate)/(param.pitch+1.0)))


void audio_cleanup(void);
int audio_setup(out123_handle *ao, mpg123_handle *mh);
int audio_prepare( out123_handle *ao, mpg123_handle *mh
,	long rate, int channels, int encoding );
size_t audio_play(out123_handle *ao, void *buffer, size_t bytes);

mpg123_string* audio_enclist(void);
void print_capabilities(out123_handle *ao, mpg123_handle *mh);

/*
	Twiddle audio output rate to yield speedup/down (pitch) effect.
	The actually achieved pitch value is stored in param.pitch.
	Returns 1 if pitch setting succeeded, 0 otherwise.
*/
int set_pitch(mpg123_handle *fr, out123_handle *ao, double new_pitch);
// Enable/disable software mute state.
int set_mute(out123_handle *ao, int mutestate);

#endif

