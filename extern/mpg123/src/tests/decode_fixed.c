#include "../compat/compat.h"
#include <mpg123.h>
#include "../common/debug.h"


int work(mpg123_handle *mh, int ch, int enc)
{
	int16_t buffer[16348];
	long rate;
	int channels, encoding;
	if(mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK)
		return 1;
	if(channels != ch || encoding != enc)
	{
		error("channel/encoding mismatch");
		return 1;
	}
	fprintf(stderr, "openend fixed track with %ld %d %d\n"
	,	rate, channels, encoding );
	int ret;
	size_t done;
	while( (ret=mpg123_read(mh, buffer, sizeof(buffer), &done)) == MPG123_OK)
	{
		if(!done)
			continue;
		fprintf(stderr, "Got first sample %d and last sample %d out of %zu\n"
		,	buffer[0], buffer[done/sizeof(*buffer)-1], done/sizeof(*buffer) );
		INT123_unintr_write(STDOUT_FILENO, buffer, done);
	}
	if(ret != MPG123_DONE)
	{
		merror( "wrong mpg123_read() return value %d (%s, %s)"
		,	ret, mpg123_plain_strerror(ret), mpg123_strerror(mh) );
		return 1;
	} else
	return 0;
}

int main(int argc, char **argv)
{
	int ret = 1;
	int enc = MPG123_ENC_SIGNED_16;
	if(!mpg123_feature2(MPG123_FEATURE_OUTPUT_16BIT))
	{
		error("Libmpg123 doesn't do 16 bit output, cannot test.");
		return 1;
	}
	if(argc < 2)
	{
		printf("Gimme a MPEG file name...\n");
		return 0;
	}
	// Intentionally no error checking here. Let's see how the lib deals with it.
	mpg123_handle *mh = mpg123_new(NULL, NULL);
	// Try to set decoder to generic to avoid memory sanitizer freaking out about
	// asssembly synth routines.
	mpg123_decoder(mh, "generic");
	// Let's fix it to mono.
	if(mpg123_open_fixed(mh, argv[1], MPG123_MONO, enc) == MPG123_OK)
		ret = work(mh, 1, enc);
	mpg123_delete(mh);
	return ret;
}
