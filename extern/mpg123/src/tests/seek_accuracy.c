/*
	seek-accuracy: Take some given mpeg file and validate that seeks are indeed accurate.

	copyright 2009-2023 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis

	arguments: decoder preframes testfile.mpeg
*/

#include "config.h"
#include <mpg123.h>
#include <stdio.h>
#include <fcntl.h>
#include "mpg123.h"
#include "../compat/compat.h"

#define SAMPLES 187
/* Cannot use the const value as fixed array size:-( */
const size_t samples = SAMPLES;

/* Use getlopt.c in future? But heck, don't bloat. */
enum theargs
{
	 arg_binname = 0
	,arg_decoder
	,arg_preframes
	,arg_file
	,arg_total
};

const char* filename;
int channels;
mpg123_handle *m;
size_t first_sample_errs = 0;


int check_seeking(size_t *errs);

int main(int argc, char **argv)
{
	int ret = 0;
	size_t errs[2] = {0, 0};
	size_t errs_ntom[2] = {0, 0};
	const char* decoder = "auto";
	const char* preframes = "10";

	if(argc < arg_total)
	{
		if(argc < 2)
		{
			/* Would at least need a file to use ... */
			fprintf(stderr, "\nUsage: %s <decoder> <preframes> <mpeg audio file>\n\n", argv[0]);
		return -1;
		}
		/* silently use defaults plus given file */
		filename = argv[argc-1];
	}
	else
	{
		decoder   = argv[arg_decoder];
		preframes = argv[arg_preframes];
		filename  = argv[arg_file];
	}
	m = mpg123_new(decoder, NULL);
	// Try to set decoder to generic to avoid memory sanitizer freaking out about
	// asssembly synth routines.
	mpg123_decoder(m, "generic");
	mpg123_param(m, MPG123_RESYNC_LIMIT, -1, 0);
	mpg123_param(m, MPG123_REMOVE_FLAGS, MPG123_GAPLESS, 0);

	if(mpg123_param(m, MPG123_PREFRAMES, atol(preframes), 0) == MPG123_OK)
	fprintf(stderr,"Testing library with preframes set to %li\n", atol(preframes));

	ret = check_seeking(errs);

	if(ret == 0)
	{
		fprintf(stderr,"Now with NtoM resampling to 33333 Hz!");
		mpg123_param(m, MPG123_FORCE_RATE, 33333, 0);
		ret = check_seeking(errs_ntom);
	}
	else fprintf(stderr, "Some failure occured! Unable to test properly!");

	fprintf(stderr,"\n");
	fprintf(stderr,"1to1 indexed seek errors: %zu / %zu\n", errs[0], samples);
	fprintf(stderr,"1to1 non-indexed seek errors: %zu / %zu\n", errs[1], samples);
	fprintf(stderr,"NtoM indexed seek errors: %zu / %zu\n", errs_ntom[0], samples);
	fprintf(stderr,"NtoM non-indexed seek errors: %zu / %zu\n", errs_ntom[1], samples);
	fprintf(stderr,"Errors in getting first sample again: %zu\n", first_sample_errs);
	fprintf(stderr,"\n");

	if(ret == 0)
	{

		if(first_sample_errs == 0 && errs[0] == 0 && errs[1] == 0 && errs_ntom[0] == 0 && errs_ntom[1] == 0)
		{
			printf("Congratulations, all seeks were accurate!\n");
			printf("But be warned: Not _all_ sample offsets have been tested. This result just means that the basic mechanism of sample-accurate seeking seems to work.\n");
		}
		else
		{
			printf("There have been some errors. For layer 3 files, this may be due to too low MPG123_PREFRAMES.\n");
			ret = -1;
		}

	}
	else fprintf(stderr, "Some bad failure during checking!\n");

	mpg123_delete(m);
	mpg123_exit();

	printf("%s\n", ret ? "FAIL" : "PASS");
	return ret;
}

int open_file()
{
	if( mpg123_open(m, filename) == MPG123_OK
	     &&
	    mpg123_getformat(m, NULL, &channels, NULL) == MPG123_OK )
	{
		fprintf(stderr,"Channels: %i\n", channels);
		return 0;
	}
	else
	{
		fprintf(stderr,"Opening file failed: %s\n", mpg123_strerror(m));
		return -1;
	}
}

/* Operation:
	Read through the whole file and remember selected samples and their position.
	Then, seek to the positions selectively and compare.
	Second mode: Check if this is identical to fresh decoding and seek (without frame index table)... less important now, focusing on the first one.
*/

struct seeko
{
	off_t position[SAMPLES];
	short left[SAMPLES];
	short right[SAMPLES];
};

/* select some positions to check seek accuracy on */
void fix_positions(struct seeko *so, off_t length)
{
	off_t step = (length-1)/(samples-1);
	size_t i;
	/* simple fixed stepping, in order (one could shuffle this) */
	for(i=0; i<samples-1; ++i)
	so->position[i] = i*step;

	/* plus the last one! */
	so->position[samples-1] = length-1;
}

int collect(struct seeko *so)
{
	off_t pos_count = 0;
	off_t length;
	int err = MPG123_OK;
	size_t posi = 0;
	mpg123_scan(m);
	length = mpg123_length(m);
	fprintf(stderr,"Estimated length: %" PRIiMAX "\n", (intmax_t)length);
	/* Compute the interesting positions */
	fix_positions(so, length);
	/*
		Default format is always 16bit int, rate does not matter.
		Let's just get the channel count and not bother.
	*/
	while(err == MPG123_OK)
	{
		short buff[1024]; /* choosing a non-divider of mpeg frame size on purpose */
		size_t got = 0;
		off_t buffsamples;
		err = mpg123_read(m, (unsigned char*)buff, 1024*sizeof(short), &got);
		buffsamples = got/(channels*sizeof(short));
		while(so->position[posi] < pos_count+buffsamples)
		{
			size_t i = (so->position[posi]-pos_count)*channels;
			fprintf(stderr,"got sample %zu (%" PRIiMAX ")\n", posi, (intmax_t)so->position[posi]);
			so->left[posi] = buff[i];
			if(channels == 2)
			so->right[posi] = buff[i+1];

			if(++posi >= samples) break;
		}
		if(posi >= samples) break;

		pos_count += buffsamples;
	}
	if(posi < samples && (err != MPG123_DONE && err != MPG123_OK))
	{
		fprintf(stderr,"An error occured (not done)?: %s\n", mpg123_strerror(m));
		return -1;
	}
	return 0;
}

/* We allow some minimal difference ... for subtle floating point effects. */
int same(short a, short b)
{
	return (abs(a-b) <= 2);
}

int check_sample(struct seeko *so, size_t i)
{
	short buf[2]; /* at max one left and right sample */
	size_t got = 0;

	if(mpg123_seek(m, so->position[i], SEEK_SET) != so->position[i])
	{
		fprintf(stderr,"Error seeking to %" PRIiMAX ": %s\n", (intmax_t)so->position[i],  mpg123_strerror(m));
		return -1;
	}

	if( mpg123_read(m, (unsigned char*)buf, channels*sizeof(short), &got) != MPG123_OK
		 ||
		 got/sizeof(short) != channels )
	{
		fprintf(stderr,"Error occured on reading sample %zu! (%s)\n", i, mpg123_strerror(m));
		return -1;
	}
	if(same(buf[0], so->left[i]) && (channels == 1 || same(buf[1], so->right[i])))
	{
		fprintf(stderr,"sample %zu PASS\n", i);
	}
	else
	{
		if(channels == 1) fprintf(stderr,"sample %zu FAIL (%i != %i)\n", i, buf[0], so->left[i]);
		else fprintf(stderr,"sample %zu FAIL (%i != %i || %i != %i)\n", i, buf[0], so->left[i], buf[1], so->right[i]);
		return -1;
	}
	return 0;
}

size_t check_positions(struct seeko *so)
{
	size_t i;
	size_t errs = 0;
	fprintf(stderr,"Seeking and comparing...\n");
	for(i=0; i<samples; ++i)
	{
		if(i == samples/2)
		{
			fprintf(stderr,"a little scan in between\n");
			mpg123_scan(m);
		}
		if(check_sample(so, i) != 0)
		++errs;
	}
	fprintf(stderr,"Check the first one again, seeking back from the end.\n");
	if(check_sample(so, 0) != 0)
	++first_sample_errs;

	return errs;
}

int check_seeking(size_t *errs)
{
	struct seeko so;
	int ret = 0;

	/* First opening. */
	if(open_file() != 0) return -1;

	fprintf(stderr,"Collecting samples...\n");
	ret = collect(&so);
	if(ret != 0) return ret;

	errs[0] = check_positions(&so);

	fprintf(stderr,"With fresh opening (empty seek index):\n");
	if(open_file() != 0) return -1;

	errs[1] = check_positions(&so);

	fprintf(stderr,"Another fresh Opening to see if first sample works:\n");
	if(open_file() != 0) return -1;
	if(check_sample(&so, 0) != 0)
	++first_sample_errs;

	return ret;
}
