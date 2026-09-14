/*
	metaprint: display of meta data (including filtering of UTF8 to ASCII)

	copyright 2006-2020 by the mpg123 project
	free software under the terms of the LGPL 2.1

	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis
*/

#ifndef MPG123_METAPRINT_H
#define MPG123_METAPRINT_H

#include "mpg123.h"
#include <stdio.h>

// Tired of typing the printing safeguard. Print the string contents
// or an empty string if there are none.
#define MPGSTR(ms) ((ms).fill ? (ms).p : "")
#define PSTR(ms) ((ms) ? (ms) : "")

void print_id3_tag(mpg123_handle *mh, int long_meta, FILE *out, int linelimit);
void print_icy(mpg123_handle *mh, FILE *out);

// Set to true if lyrics shall be printed.
extern int meta_show_lyrics;

#endif
