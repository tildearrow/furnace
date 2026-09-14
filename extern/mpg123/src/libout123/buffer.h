/*
	buffer.h: output buffer

	copyright 1999-2015 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Daniel Kobras / Oliver Fromme
*/

/*
 * Application specific interaction between main and buffer
 * process. This is much less generic than the functions in
 * xfermem so I chose to put it in buffer.[hc].
 * 01/28/99 [dk]
 */

#ifndef _MPG123_BUFFER_H_
#define _MPG123_BUFFER_H_

#include "out123_int.h"
#include "../compat/compat.h"

int  INT123_buffer_init(out123_handle *ao, size_t bytes);
void INT123_buffer_exit(out123_handle *ao);

/* Messages with payload. */

int INT123_buffer_sync_param(out123_handle *ao);
int INT123_buffer_open(out123_handle *ao, const char* driver, const char* device);
int INT123_buffer_encodings(out123_handle *ao);
int INT123_buffer_formats( out123_handle *ao, const long *rates, int ratecount
                  , int minchannels, int maxchannels
                  , struct mpg123_fmt **fmtlist );
int INT123_buffer_start(out123_handle *ao);
void INT123_buffer_ndrain(out123_handle *ao, size_t bytes);

/* Simple messages to be deal with after playback. */

void INT123_buffer_stop(out123_handle *ao);
void INT123_buffer_close(out123_handle *ao);
void INT123_buffer_continue(out123_handle *ao);
/* Still undecided if that one is to be used anywhere. */
void INT123_buffer_ignore_lowmem(out123_handle *ao);
void INT123_buffer_drain(out123_handle *ao);
void INT123_buffer_end(out123_handle *ao);

/* Simple messages with interruption of playback. */

void INT123_buffer_pause(out123_handle *ao);
void INT123_buffer_drop(out123_handle *ao);

/* The actual work: Hand over audio data. */
size_t INT123_buffer_write(out123_handle *ao, void *buffer, size_t bytes);

/* Thin wrapper over xfermem giving the current buffer fill. */
size_t INT123_buffer_fill(out123_handle *ao);

#endif
