/*
	dump_seekindex: Scan a mpeg file and dump its seek index.

	This is example code only sensible to be considered in the public domain.
	Initially written by Patrick Dehne.
*/

#include <mpg123.h>
#include <stdio.h>
#include <inttypes.h>

int main(int argc, char **argv)
{
	mpg123_handle *m;
	off_t* offsets;
	off_t step;
	size_t fill, i;

	if(argc != 2)
	{
		fprintf(stderr, "\nI will dump the frame index of an MPEG audio file.\n");
		fprintf(stderr, "\nUsage: %s <mpeg audio file>\n\n", argv[0]);
		return -1;
	}
#if MPG123_API_VERSION < 46
	// Newer versions of the library don't need that anymore, but it is safe
	// to have the no-op call present for compatibility with old versions.
	 mpg123_init();
#endif
	m = mpg123_new(NULL, NULL);
	mpg123_param(m, MPG123_RESYNC_LIMIT, -1, 0);
	mpg123_param(m, MPG123_INDEX_SIZE, -1, 0);
	mpg123_open(m, argv[1]);
	mpg123_scan(m);

	mpg123_index(m, &offsets, &step, &fill);
	for(i=0; i<fill;i++) {
		printf("Frame number %"PRIiMAX": file offset %"PRIiMAX"\n", (intmax_t)(i * step), (intmax_t)offsets[i]);
	}

	mpg123_close(m);
	mpg123_delete(m);
	return 0;
}
