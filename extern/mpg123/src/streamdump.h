/*
	streamdump: I/O interposing for reading input streams

	This initially was about dumping a copy of the input data, as read
	by libmpg123. Now it is a generic interposer to wrap around plain file
	and network stream reading.

	copyright 2010-2022 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis
*/

#ifndef STREAMDUMP_H
#define STREAMDUMP_H

#include "mpg123app.h"

#ifdef NET123
#include "net123.h"
#endif
#ifdef NETWORK
#include "httpget.h"
#endif

// The stream is either addressed via file descriptor or net123 handle.
struct stream
{
	char buf[256]; // buffer for getline
	char *bufp; // read pointer in buffer
	int fill; // bytes in buffer
	int fd; // if > 0: plain file descriptor or win32 net socket
	struct httpdata htd;
#ifdef NET123
	net123_handle *nh; // if != null: a net123 stream
#endif
};

// Open a stream resource, creating and returning a new handle.
struct stream * stream_open(const char *url);
// Read lines, with arbitrary line end, which is stripped.
// Return number of bytes in line (including closing zero byte) or error < 0.
// End of file returns zero, consequently.
mpg123_ssize_t stream_getline(struct stream *sd, mpg123_string *line);
void stream_close(struct stream *sd);

// Use an open stream object to optionally prepare the dump and
// link up input to the mpg123 handle.
// Can be called repeatedly to switch input methods with new
// stream objects, but keeping a stream dump file open.
// Return value is 0 for no error, -1 when bad.
int dump_setup(struct stream *sd, mpg123_handle *mh);
// Just close the dump output, not touching anything else.
void dump_close(void);

#endif
