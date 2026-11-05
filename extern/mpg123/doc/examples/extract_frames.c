/*
	extract_frams: utlize the framebyframe API and mpg123_framedata to extract the MPEG frames out of a stream (strip off anything else).

	This is example code only sensible to be considered in the public domain.
	Initially written by Thomas Orgis.
*/

#include <mpg123.h>

/* unistd.h is not available under MSVC, 
 io.h defines the read and write functions */
#ifndef _MSC_VER
#include <unistd.h>
#else
#include <io.h>
#endif

#include <stdio.h>
#include <string.h>

/** The actual work on the existing handle. */
int do_work(mpg123_handle *m);

/** The main program. */
int main(int argc, char **argv)
{
	int ret = 0;
	mpg123_handle *m;

#if MPG123_API_VERSION < 46
	// Newer versions of the library don't need that anymore, but it is safe
	// to have the no-op call present for compatibility with old versions.
	mpg123_init();
#endif
	m = mpg123_new(NULL, &ret);

	if(m == NULL)
	{
		fprintf(stderr, "Cannot create handle: %s", mpg123_plain_strerror(ret));
	}
	else
	{
		fprintf(stderr, "I'll take your dirty MPEG audio from standard input and will write the extracted pure MPEG data to standard output.\n");
		if(argc > 1 && strcmp(argv[1], "--noinfo") == 0)
		{
			fprintf(stderr, "Enabling parsing/consuming of the Info frame so that it will not appear in output.\n");
			ret = mpg123_param(m, MPG123_REMOVE_FLAGS, MPG123_IGNORE_INFOFRAME, 0.);
		}
		else
		{
			fprintf(stderr, "If you'd have given --noinfo as argument, I would omit a LAME/Xing info frame.\n");
			ret = mpg123_param(m, MPG123_ADD_FLAGS, MPG123_IGNORE_INFOFRAME, 0.);
		}
		if(ret == 0) ret = do_work(m);

		if(ret != 0) fprintf(stderr, "Some error occured: %s\n", mpg123_strerror(m));


		mpg123_delete(m); /* Closes, too. */
	}

	return ret;
}

int do_work(mpg123_handle *m)
{
	int ret;
	size_t count = 0;
	ret = mpg123_open_fd(m, STDIN_FILENO);
	if(ret != MPG123_OK) return ret;

	ssize_t wret = 0;
	while( !wret && (ret = mpg123_framebyframe_next(m)) == MPG123_OK || ret == MPG123_NEW_FORMAT )
	{
		unsigned long header;
		unsigned char *bodydata;
		size_t bodybytes;
		if(mpg123_framedata(m, &header, &bodydata, &bodybytes) == MPG123_OK)
		{
			/* Need to extract the 4 header bytes from the native storage in the correct order. */
			unsigned char hbuf[4];
			int i;
			for(i=0; i<4; ++i) hbuf[i] = (unsigned char) ((header >> ((3-i)*8)) & 0xff);

			/* Now write out both header and data, fire and forget. */
			wret = write(STDOUT_FILENO, hbuf, 4);
			if(!wret)
				wret = write(STDOUT_FILENO, bodydata, bodybytes);
			fprintf(stderr, "%zu: header 0x%08lx, %zu body bytes\n", ++count, header, bodybytes);
		}
	}

	if(ret != MPG123_DONE)
		fprintf(stderr, "Some error occured (non-fatal?): %s\n", mpg123_strerror(m));
	if(wret)
	{
		fprintf(stderr, "Write error.\n");
		ret = MPG123_ERR;
	}

	fprintf(stderr, "Done with %zu MPEG frames.\n", count);

	return 0;
}
