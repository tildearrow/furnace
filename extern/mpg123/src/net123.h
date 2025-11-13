/*
	net123: network (HTTP(S)) streaming for mpg123 using external code

	copyright 2022 by the mpg123 project --
	free software under the terms of the LGPL 2.1,
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis

	This is supposed to be a binding to some system facilty to
	get HTTP/HTTPS streaming going. The goal is to not be responsible
	for HTTP protocol handling or even TLS in mpg123 code.
	Maybe this could also stream via other protocols ... maybe even
	SSH, whatever.

	For POSIX platforms, this shall refer to external binaries that
	do all the network stuff. For others, some system library or
	other facility shall provide the minimal required functionality.

	We could support seeking using HTTP ranges. But the first step is
	to be able to just make a request to a certain URL and get the
	server response headers followed by the data (be it a playlist file
	or the MPEG stream itself).

	We need to support:
	- client headers (ICY yes or no, client name)
	- HTTP auth parameters

	The idea is that this just handles the network and protocol part
	of fetching data from an URL, returning

	<server response headers>
	<empty line>
	<stream data>

	via net123_read(). Header part is with <cr><lf>, just passing through
	what the server gives should be OK. The only HTTP thing mpg123 shall do
	is to parse headers.
*/
#ifndef _MPG123_NET123_H_
#define _MPG123_NET123_H_

#include "config.h"
#include <sys/types.h>

// stream handle for differing implementations, build-time plugins
struct net123_handle_struct
{
	void *parts; // custom internal data
	// callbacks
	size_t (*read)(struct net123_handle_struct *nh, void *buf, size_t bufsize);
	void (*close)(struct net123_handle_struct *nh);
};
typedef struct net123_handle_struct net123_handle;

extern const char *net123_backends[];

// Open stream from URL, preparing output such that net123_read()
// later on gets the response header lines followed by one empty line
// and then the raw data.
// client_head contains header lines to send with the request, without
// line ending

// Variant for the external binding.
#ifdef NET123_EXEC
net123_handle *net123_open_exec(const char *url, const char * const *client_head);
#endif
#ifdef NET123_WININET
net123_handle *net123_open_wininet(const char *url, const char * const *client_head);
#endif
#ifdef NET123_WINHTTP
net123_handle *net123_open_winhttp(const char *url, const char * const *client_head);
#endif

#endif
