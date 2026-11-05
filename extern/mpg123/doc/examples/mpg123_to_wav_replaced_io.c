/*
	mpg123_to_wav_replaced_io.c

	This is example code only sensible to be considered in the public domain.
	Initially written by Nicholas Humfrey (moved to handle I/O by Thomas Orgis).

	This example program demonstrates how to use libmpg123 to decode a file to WAV (writing via libout123), while doing the I/O (read and seek) with custom callback functions.
	This should cater for any situation where you have some special means to get to the data (like, mmapped files / plain buffers in memory, funky network streams).

	Disregarding format negotiations, the basic synopsis is:

	mpg123_init()
	mpg123_new()
	mpg123_replace_reader_handle()

	mpg123_open_handle()
	mpg123_read()
	mpg123_close()

	mpg123_delete()
	mpg123_exit()
*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <mpg123.h>
#include <out123.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef _MSC_VER
#include <unistd.h>
#else
#include <io.h>
#endif
#include <string.h>

void usage(const char *cmd)
{
	printf("Usage: %s <input> <output>\n", cmd);
	exit(99);
}

void cleanup(mpg123_handle *mh, out123_handle *ao)
{
	out123_del(ao);
	/* It's really to late for error checks here;-) */
	mpg123_close(mh);
	mpg123_delete(mh);
	mpg123_exit();
}

/* Simple handle for you private I/O data. */
struct ioh { int fd; };

/* The callback functions; simple wrappers over standard I/O.
   They could be anything you like... */

static mpg123_ssize_t read_cb(void *handle, void *buf, size_t sz)
{
	mpg123_ssize_t ret;
	struct ioh *h = handle;
	errno = 0;
	ret = read(h->fd, buf, sz);
	if(ret < 0) fprintf(stderr, "read error: %s\n", strerror(errno));

	return ret;
}

static off_t lseek_cb(void *handle, off_t offset, int whence)
{
	off_t ret;
	struct ioh *h = handle;
	ret = lseek(h->fd, offset, whence);
	if(ret < 0) fprintf(stderr, "seek error: %s\n", strerror(errno));

	return ret;
}

/* The cleanup handler is called on mpg123_close(), it can cleanup your part of the mess... */
void cleanup_cb(void *handle)
{
	struct ioh *h = handle;
	close(h->fd);
	h->fd = -1;
}


int main(int argc, char *argv[])
{
	mpg123_handle *mh = NULL;
	out123_handle *ao = NULL;
	unsigned char* buffer = NULL;
	size_t buffer_size = 0;
	size_t done = 0;
	int  channels = 0, encoding = 0;
	long rate = 0;
	int  err  = MPG123_OK;
	off_t samples = 0;
	struct ioh *iohandle;

	if (argc!=3) usage(argv[0]);
	printf( "Input file: %s\n", argv[1]);
	printf( "Output file: %s\n", argv[2]);

#if MPG123_API_VERSION < 46
	// Newer versions of the library don't need that anymore, but it is safe
	// to have the no-op call present for compatibility with old versions.
	err = mpg123_init();
#endif

	errno = 0;
	iohandle = malloc(sizeof(struct ioh));
	iohandle->fd = open(argv[1], O_RDONLY);
	if(iohandle->fd < 0)
	{
		fprintf(stderr, "Cannot open input file (%s).\n", strerror(errno));
		return -1;
	}

	if( err != MPG123_OK || (mh = mpg123_new(NULL, &err)) == NULL
	    /* Let mpg123 work with the file, that excludes MPG123_NEED_MORE messages. */
	    || mpg123_replace_reader_handle(mh, read_cb, lseek_cb, cleanup_cb) != MPG123_OK
	    || mpg123_open_handle(mh, iohandle) != MPG123_OK
	    /* Peek into track and get first output format. */
	    || mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK )
	{
		fprintf( stderr, "Trouble with mpg123: %s\n",
		         mh==NULL ? mpg123_plain_strerror(err) : mpg123_strerror(mh) );
		cleanup(mh, ao);
		return -1;
	}

	if(encoding != MPG123_ENC_SIGNED_16)
	{ /* Signed 16 is the default output format anyways; it would actually by only different if we forced it.
	     So this check is here just for this explanation. */
		cleanup(mh, ao);
		fprintf(stderr, "Bad encoding: 0x%x!\n", encoding);
		return -2;
	}
	/* Ensure that this output format will not change (it could, when we allow it). */
	mpg123_format_none(mh);
	mpg123_format(mh, rate, channels, encoding);

	printf("Creating 16bit WAV with %i channels and %liHz.\n", channels, rate);
	if(
		!(ao = out123_new())
	||	out123_open(ao, "wav", argv[2])
	||	out123_start(ao, rate, channels, encoding)
	)
	{
		fprintf(stderr, "Cannot create / start output: %s\n"
		,	out123_strerror(ao));
		cleanup(mh, ao);
		return -1;
	}

	/* Buffer could be almost any size here, mpg123_outblock() is just some recommendation.
	   Important, especially for sndfile writing, is that the size is a multiple of sample size. */
	buffer_size = mpg123_outblock( mh );
	buffer = malloc( buffer_size );

	do
	{
		err = mpg123_read( mh, buffer, buffer_size, &done );
		out123_play(ao, buffer, done);
		samples += done/sizeof(short);
		/* We are not in feeder mode, so MPG123_OK, MPG123_ERR and MPG123_NEW_FORMAT are the only possibilities.
		   We do not handle a new format, MPG123_DONE is the end... so abort on anything not MPG123_OK. */
	} while (done && err==MPG123_OK);

	free(buffer);

	if(err != MPG123_DONE)
	fprintf( stderr, "Warning: Decoding ended prematurely because: %s\n",
	         err == MPG123_ERR ? mpg123_strerror(mh) : mpg123_plain_strerror(err) );

	samples /= channels;
	printf("%li samples written.\n", (long)samples);
	cleanup(mh, ao);
	free(iohandle);
	return 0;
}
