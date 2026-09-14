#ifndef _MPG123_H_HEXTXT
#define _MPG123_H_HEXTXT
/*
	hextxt: hex or printf text output (ASCII/UTF-8)

	copyright 2017 by the mpg123 project
	                  - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis
*/

#include "out123_int.h"

int  hex_formats (out123_handle *ao);
int  txt_formats (out123_handle *ao);
int  hex_open    (out123_handle *ao);
int  txt_open    (out123_handle *ao);
int  hextxt_close(out123_handle *ao);
int  hex_write   (out123_handle *ao, unsigned char *buf, int len);
int  txt_write   (out123_handle *ao, unsigned char *buf, int len);
void hextxt_drain(out123_handle *ao);

#endif
