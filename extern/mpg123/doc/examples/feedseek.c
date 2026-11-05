/*
	feedseek: test program for libmpg123, showing how to use fuzzy seeking in feeder mode

	This is example code only sensible to be considered in the public domain.

	It takes MPEG data from standard input and feeds that to libmpg123, combined
	with a fuzzy seek. Libmpg123 could access stdin in other ways directly, but
	usage of the feeder API is the point here.
*/

#define _POSIX_C_SOURCE 200112L /**< POSIX standard for ftello. */
#include <mpg123.h>
#include <out123.h>
#include <stdlib.h>
#include <stdio.h>

#define INBUFF  16384 * 2 * 2 /**< input buffer size */

/** Error handling helper, end program if condition is not met. 
 *  Yes, a goto for error handling. Controlled cleanup in a final section instead
 *  of just jumping out of the program.
 */
#define CHECK(cond, ...) if(!(cond)){ fprintf(stderr,  __VA_ARGS__); goto bad_end; }

/** The whole operation. */
int main(int argc, char **argv)
{
	unsigned char buf[INBUFF];
	mpg123_handle *m = NULL;
	out123_handle *o = NULL;
	const char *driver = NULL;
	char *device = NULL;
	int ret = 0;
	off_t inoffset;
	off_t cur_inoffset;
	off_t seek_point = 0;
	off_t seek_target;

	// 0. Argument parsing.

	if(argc < 2)
	{
		fprintf(stderr, "Usage:\n\n  %s <offset> [outfile] < <MPEG input data>\n", argv[0]);
		fprintf( stderr, "\n"
			"This will decode standard input to the default audio device or the\n"
			"given output file as decoded WAV after seeking to the given PCM sample\n"
			"offset to demonstrate fuzzy seeking\n" );
		return 1;
	}

	seek_point = atol(argv[1]);
	CHECK(seek_point >= 0, "No negative offset, please.\n")

	if(argc >= 3)
	{
		driver = "wav";
		device = argv[2];
	}

	// 1. Initialize libmpg123 and libout123 handles.

#if MPG123_API_VERSION < 46
	// Newer versions of the library don't need that anymore, but it is safe
	// to have the no-op call present for compatibility with old versions.
	mpg123_init();
#endif

	m = mpg123_new(NULL, &ret);
	CHECK(m != NULL, "Unable to create mpg123 handle: %s\n", mpg123_plain_strerror(ret))

	o = out123_new();
	CHECK(o != NULL, "Unable to create out123 handle.\n")

	ret = out123_open(o, driver, device);
	CHECK(ret == OUT123_OK, "Failed to open output device: %s\n", out123_strerror(o))

	mpg123_param(m, MPG123_VERBOSE, 2, 0);

	mpg123_param(m, MPG123_FLAGS, MPG123_FUZZY | MPG123_SEEKBUFFER | MPG123_GAPLESS, 0);
	CHECK(ret == MPG123_OK, "Unable to set library options: %s\n", mpg123_plain_strerror(ret))

	// 2. Open the feeder, feed data until libmpg123 can tell us where to go in the input
	//    for the desired output offset.

	ret = mpg123_open_feed(m);
	CHECK(ret == MPG123_OK, "Unable open feed: %s\n", mpg123_plain_strerror(ret))

	fprintf(stderr, "\nDetermining input offset for seek ...\n");
	/* That condition is tricky... parentheses are crucial... */
	while( (seek_target = mpg123_feedseek(m, seek_point, SEEK_SET, &inoffset))
		 == MPG123_NEED_MORE )
	{
		fprintf(stderr, " *** need more data before deciding on fuzzy input offset ***\n");
		size_t len = fread(buf, 1, INBUFF, stdin);
		if(len < 0 || (len == 0 && (feof(stdin) || ferror(stdin))))
			break;
		ret = mpg123_feed(m, buf, len);
		CHECK(ret == MPG123_OK, "Error feeding the decoder: %s", mpg123_strerror(m))
	}
	CHECK(ret == MPG123_OK, "Feedseek failed: %s\n", mpg123_strerror(m))
	fprintf( stderr, "Fuzzy seek to %lld, actually to %lld.\n"
	,	(long long)seek_point, (long long)seek_target );
	CHECK(inoffset >=0 , "Bogus input offset: %lld\n", (long long)inoffset)

	// 3. Go to the indicated input offset.

	fprintf(stderr, "\nSeeking to input offset ...\n");
	// In a normal file, we would call fseek() here, for stdin, we just consume data.
	while( (cur_inoffset=ftello(stdin)) < inoffset && !ferror(stdin) )
	{
		CHECK(cur_inoffset >= 0, "Cannot tell input position.\n")
		off_t block = inoffset - cur_inoffset;
		if(block > INBUFF)
			block = INBUFF;
		if(fread(buf, 1, block, stdin) == 0 && feof(stdin))
			break;
	}
	CHECK( ftello(stdin) == inoffset, "Input seeking failed: %lld != %lld\n"
	,	(long long)ftello(stdin), (long long)inoffset )

	// 4. Feed the decoder from that point on and get the decoded audio.

	fprintf(stderr, "\nStarting decode...\n");
	while(1)
	{
		size_t len = fread(buf, sizeof(unsigned char), INBUFF, stdin);
		if(len <= 0)
			break;
		ret = mpg123_feed(m, buf, len);

		while(ret != MPG123_ERR && ret != MPG123_NEED_MORE)
		{
			off_t num;
			unsigned char *audio;
			size_t bytes;
			ret = mpg123_decode_frame(m, &num, &audio, &bytes);
			if(ret == MPG123_NEW_FORMAT)
			{
				long rate;
				int channels, enc;
				mpg123_getformat(m, &rate, &channels, &enc);
				fprintf(stderr
				,	"New format: %li Hz, %i channels, encoding value %i\n"
				,	rate, channels, enc );
				ret = out123_start(o, rate, channels, enc);
				CHECK(ret == OUT123_OK, "Cannot (re)start audio output with given format.\n")
			}
			CHECK(out123_play(o, audio, bytes) == bytes, "Output error: %s", out123_strerror(o))
		}

		CHECK(ret != MPG123_ERR, "Error: %s", mpg123_strerror(m))
	}

	fprintf(stderr, "Finished\n");

good_end: // Everything went fine: go directly to the cleanup section.
	goto end;
bad_end: // Some unspecified error, set error state and clean up.
	ret = -1;
end: // Clean up and return.
	out123_del(o);
	mpg123_delete(m);
	return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}
