/*
	scan: Estimate length (sample count) of a mpeg file and compare to length from exact scan.

	This is example code only sensible to be considered in the public domain.
	Initially written by Thomas Orgis
*/

/* Note the lack of error checking here.
   While it would be nicer to inform the user about troubles, libmpg123 is designed _not_ to bite you on operations with invalid handles , etc.
  You just jet invalid results on invalid operations... */

#include <mpg123.h>
#include <stdio.h>

/** The whole operation. */
int main(int argc, char **argv)
{
	mpg123_handle *m;
	int i;
	if(argc < 2)
	{
		fprintf(stderr, "\nI will give you the estimated and exact sample lengths of MPEG audio files.\n");
		fprintf(stderr, "\nUsage: %s <mpeg audio file list>\n\n", argv[0]);
		return -1;
	}
#if MPG123_API_VERSION < 46
	mpg123_init();
#endif
	m = mpg123_new(NULL, NULL);
	mpg123_param(m, MPG123_RESYNC_LIMIT, -1, 0); /* New in library version 0.0.1 . */
	for(i = 1; i < argc; ++i)
	{
		off_t a, b;
		
		mpg123_open(m, argv[i]);

		a = mpg123_length(m);		
		mpg123_scan(m);
		b = mpg123_length(m);

		mpg123_close(m);

		printf("File %i: estimated %li vs. scanned %li\n", i, (long)a, (long)b);
	}

	mpg123_delete(m);
	return 0;
}
