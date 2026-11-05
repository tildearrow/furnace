/*
	hextxt: hex or printf text output (ASCII/UTF-8)

	copyright 2017-2023 by the mpg123 project
	free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis

	This just adds something to directly compare with FhG sample
	outputs (hex) or to lazily pipe to a plotter (text, one column per
	channel).

	Hex mode supports all possible integer encodings (where I am resonably
	sure that endianess is defined). For text output, printing floats is
	no problem. Printout of high channel counts as well as 24 bit numbers
	is probably rather slow as individual numbers are pushed into a fprintf
	call each.
*/

#include "out123_int.h"
#include "hextxt.h"
#include "../common/debug.h"

int hex_formats(out123_handle *ao)
{
	return MPG123_ENC_ANY;
}

// This is about the same list as the above, but in case
// any new format is added, the text printout will need
// explicit changes, while hex printout does only care
// about a sample's byte count.
int txt_formats(out123_handle *ao)
{
	return
		MPG123_ENC_SIGNED_8
	|	MPG123_ENC_UNSIGNED_8
	|	MPG123_ENC_ULAW_8 // Printed as unsigned integer.
	|	MPG123_ENC_ALAW_8 // Printed as unsigned integer.
	|	MPG123_ENC_SIGNED_16
	|	MPG123_ENC_UNSIGNED_16
	|	MPG123_ENC_SIGNED_24
	|	MPG123_ENC_UNSIGNED_24
	|	MPG123_ENC_SIGNED_32
	|	MPG123_ENC_UNSIGNED_32
	|	MPG123_ENC_FLOAT_32
	|	MPG123_ENC_FLOAT_64;
}

static FILE* open_file(const char *path)
{
	if(!path || !strcmp("-",path) || !strcmp("",path))
		return stdout;
	else
		return INT123_compat_fopen(path, "w");
}

/* Hex output defaults to what FhG compliance files used. */
int hex_open(out123_handle *ao)
{
	if(ao->format < 0)
	{
		ao->rate = 44100;
		ao->channels = 1;
		ao->format = MPG123_ENC_SIGNED_24;
		return 0;
	}

	ao->userptr = open_file(ao->device);
	return ao->userptr ? 0 : -1;
}

/* Text output defaults to the usual, CD-like format. */
int txt_open(out123_handle *ao)
{
	if(ao->format < 0)
	{
		ao->rate = 44100;
		ao->channels = 2;
		ao->format = MPG123_ENC_SIGNED_16;
		return 0;
	}

	ao->userptr = ( (ao->format & txt_formats(ao)) == ao->format)
	?	open_file(ao->device)
	:	NULL;
	return ao->userptr ? 0 : -1;
}

int hextxt_close(out123_handle *ao)
{
	if(ao && ao->userptr)
	{
		FILE *fp = ao->userptr;
		ao->userptr = NULL;
		if(fp != stdout && INT123_compat_fclose(fp))
		{
			if(!AOQUIET)
				error1("problem closing the output: %s\n", INT123_strerror(errno));
			return -1;
		}
	}
	return 0;
}

int hex_write(out123_handle *ao, unsigned char *buf, int len)
{
	FILE *fp;
	int i, block, samples;

	if(!ao || !ao->userptr)
		return -1;
	fp = ao->userptr;

	block = out123_encsize(ao->format);
	samples = len/block;
	for(i=0; i<samples; ++i)
	{
		unsigned char *s = &buf[i*block];
		/* Printout always big-endian, beginning with the highest byte. */
		switch(block)
		{
#ifdef WORDS_BIGENDIAN
			case 1:
				fprintf(fp, "%02x\n", s[0]);
			break;
			case 2:
				fprintf(fp, "%02x%02x\n", s[0], s[1]);
			break;
			case 3:
				fprintf(fp, "%02x%02x%02x\n", s[0], s[1], s[2]);
			break;
			case 4:
				fprintf(fp, "%02x%02x%02x%02x\n", s[0], s[1], s[2], s[3]);
			break;
#else
			case 1:
				fprintf(fp, "%02x\n", s[0]);
			break;
			case 2:
				fprintf(fp, "%02x%02x\n", s[1], s[0]);
			break;
			case 3:
				fprintf(fp, "%02x%02x%02x\n", s[2], s[1], s[0]);
			break;
			case 4:
				fprintf(fp, "%02x%02x%02x%02x\n", s[3], s[2], s[1], s[0]);
			break;
#endif
		}
	}
	return i*block;
}

int txt_write(out123_handle *ao, unsigned char *buf, int len)
{
	FILE *fp;
	int i, block, frames;

	if(!ao || !ao->userptr)
		return -1;
	fp = ao->userptr;

	block = ao->framesize;
	frames = len/block;

	for(i=0; i<frames; ++i)
	{
		void *f = &buf[i*block];
		switch(ao->format)
		{
#define CHANPRINT(type, ptype, fmt) \
	{ \
		type *ff = f; \
		switch(ao->channels) \
		{ \
			case 1: \
				fprintf(fp, fmt"\n", (ptype)ff[0]); \
			break; \
			case 2: \
				fprintf(fp, fmt"\t"fmt"\n", (ptype)ff[0], (ptype)ff[1]); \
			break; \
			default: \
			{ \
				int c; \
				for(c=0; c<ao->channels; ++c) \
					fprintf(fp, "%s"fmt, c ? "\t" : "", (ptype)ff[c]); \
				fprintf(fp, "\n"); \
			} \
		} \
	}
/* Am I to paranoid about dancing around the type system? */
#ifdef WORDS_BIGENDIAN
#define CHANPRINT24(type, ptype, fmt) \
	{ \
		unsigned char *ff = f; \
		for(int c=0; c<ao->channels; ++c) \
		{ \
			uint32_t tmp = 0; \
			tmp |= ((uint32_t)(*(ff++))) << 24; \
			tmp |= ((uint32_t)(*(ff++))) << 16; \
			tmp |= ((uint32_t)(*(ff++))) << 8; \
			fprintf(fp, "%s"fmt, c ? "\t" : "", (ptype)(*((type*)&tmp)/256)); \
		} \
		fprintf(fp, "\n"); \
	}
#else
#define CHANPRINT24(type, ptype, fmt) \
	{ \
		unsigned char *ff = f; \
		for(int c=0; c<ao->channels; ++c) \
		{ \
			uint32_t tmp = 0; \
			tmp |= ((uint32_t)(*(ff++))) << 8; \
			tmp |= ((uint32_t)(*(ff++))) << 16; \
			tmp |= ((uint32_t)(*(ff++))) << 24; \
			fprintf(fp, "%s"fmt, c ? "\t" : "", (ptype)(*((type*)&tmp)/256)); \
		} \
		fprintf(fp, "\n"); \
	}
#endif
			case MPG123_ENC_SIGNED_8:
				CHANPRINT(char, int, "%d")
			break;
			case MPG123_ENC_UNSIGNED_8:
			case MPG123_ENC_ULAW_8:
			case MPG123_ENC_ALAW_8:
				CHANPRINT(unsigned char, int, "%u")
			break;
			case MPG123_ENC_SIGNED_16:
				CHANPRINT(int16_t, int, "%d")
			break;
			case MPG123_ENC_UNSIGNED_16:
				CHANPRINT(uint16_t, unsigned int, "%u")
			break;
			case MPG123_ENC_SIGNED_24:
				CHANPRINT24(int32_t, long, "%ld")
			break;
			case MPG123_ENC_UNSIGNED_24:
				CHANPRINT24(uint32_t, unsigned long, "%ld")
			break;
			case MPG123_ENC_SIGNED_32:
				CHANPRINT(int32_t, long, "%ld")
			break;
			case MPG123_ENC_UNSIGNED_32:
				CHANPRINT(uint32_t, unsigned long, "%lu")
			break;
			case MPG123_ENC_FLOAT_32:
				CHANPRINT(float, double, "%e")
			break;
			case MPG123_ENC_FLOAT_64:
				CHANPRINT(double, double, "%e")
			break;
		}
	}
	return i*block;
}

/* Draining is flushing to disk. Words do suck at times. */
void hextxt_drain(out123_handle *ao)
{
	if(!ao || !ao->userptr)
		return;
	if(fflush(ao->userptr) && !AOQUIET)
		error1("flushing failed: %s\n", INT123_strerror(errno));
}
