/*
	syn123_int: internal header for libsyn123

	copyright 2018-2023 by the mpg123 project,
	licensed under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org

	initially written by Thomas Orgis
*/

#ifndef _MPG123_SYN123_INT_H_
#define _MPG123_SYN123_INT_H_

#define _ISOC99_SOURCE

#include "config.h"
#ifdef LFS_LARGEFILE_64
#define _LARGEFILE64_SOURCE
#endif
#include "../common/abi_align.h"
#include "../compat/compat.h"

#define SYN123_NO_LARGEFUNC
#include "syn123.h"
// A little hack to help MSVC not having ssize_t, duplicated in external header with largefuncs.
#ifdef _MSC_VER
typedef ptrdiff_t syn123_ssize_t;
#else
typedef ssize_t syn123_ssize_t;
#endif


// Generally, a number of samples we work on in one go to
// allow the compiler to know our loops.
// An enum is the best integer constant you can define in plain C.
// This sets an upper limit to the number of channels in some functions.
enum { bufblock = 512 };

struct syn123_wave
{
	enum syn123_wave_id id;
	int backwards; /* TRUE or FALSE */
	double freq; /* actual frequency */
	double phase; /* current phase */
};

struct syn123_sweep
{
	struct syn123_wave wave;
	double f1, f2; // begin/end frequencies (or logarithms of the same)
	enum syn123_sweep_id id;
	size_t i; // position counter
	size_t d; // duration
	size_t post; // amount of samples after sweep to finish period
	double endphase; // phase for continuing, just after sweep end
};

// Only a forward declaration of the resampler state. An instance
// is allocated and a pointer stored if it is configured. The
// resampler is pretty disjunct from the other parts of syn123.
// Not sure if synergies will emerge eventually.
struct resample_data;

struct filter_chain
{
	int mixenc;   // double or float
	int channels;
	size_t count; // of filters
	size_t maxcount; // storage allocated for that many, to avoid
	                 // realloc in syn123_drop_filter()
	// Only one type is configured!
	struct d_filter *df; // double precision filters
	struct f_filter *ff; // single precision filters
};

struct syn123_struct
{
	// Temporary storage in internal precision.
	// This is a set of two to accomodate x and y=function(x, y).
	// Working in blocks reduces function call overhead and gives
	// chance of vectorization.
	// This may also be used as buffer for data with output encoding,
	// exploiting the fact that double is the biggest data type we
	// handle, also with the biggest alignment.
	double workbuf[2][bufblock];
	struct mpg123_fmt fmt;
	int dither; // if dithering is activated for the handle
	int do_dither; // flag for recursive calls of syn123_conv()
	uint32_t dither_seed;
	// Pointer to a generator function that writes a bit of samples
	// into workbuf[1], possibly using workbuf[0] internally.
	// Given count of samples <= bufblock!
	void (*generator)(syn123_handle*, int);
	// Generator configuration.
	// wave generator
	size_t wave_count;
	struct syn123_wave* waves;
	// pink noise, maybe others: simple structs that can be
	// simply free()d
	void* handle;
	uint32_t seed; // random seed for some RNGs
	// Extraction of initially-computed waveform from buffer.
	void *buf;      // period buffer
	size_t bufs;    // allocated size of buffer in bytes
	size_t maxbuf;  // maximum period buffer size in bytes
	size_t samples; // samples (PCM frames) in period buffer
	size_t offset;  // offset in buffer for extraction helper
	struct resample_data *rd; // resampler data, if initialized
	struct filter_chain fc;
};

#ifndef NO_SMIN
static size_t smin(size_t a, size_t b)
{
	return a < b ? a : b;
}
#endif

#ifndef NO_SMAX
static size_t smax(size_t a, size_t b)
{
	return a > b ? a : b;
}
#endif

#ifndef NO_GROW_BUF
// Grow period buffer to at least given size.
// Content is not preserved.
static void grow_buf(syn123_handle *sh, size_t bytes)
{
	if(sh->bufs >= bytes)
		return;
	if(sh->buf)
		free(sh->buf);
	sh->buf = NULL;
	if(bytes && bytes <= sh->maxbuf)
		sh->buf = malloc(bytes);
	sh->bufs = sh->buf ? bytes : 0;
}
#endif

#ifdef FILL_PERIOD
static int fill_period(syn123_handle *sh)
{
	sh->samples = 0;
	if(!sh->maxbuf)
		return SYN123_OK;
	size_t samplesize = MPG123_SAMPLESIZE(sh->fmt.encoding);
	size_t buffer_samples = sh->maxbuf/samplesize;
	grow_buf(sh, buffer_samples*samplesize);
	if(buffer_samples > sh->bufs/samplesize)
		return SYN123_DOOM;
	int outchannels = sh->fmt.channels;
	sh->fmt.channels = 1;
	size_t buffer_bytes = syn123_read(sh, sh->buf, buffer_samples*samplesize);
	sh->fmt.channels = outchannels;
	if(buffer_bytes != buffer_samples*samplesize)
		return SYN123_WEIRD;
	sh->samples = buffer_samples;
	return SYN123_OK;
}
#endif

#ifdef RAND_XORSHIFT32
// Borrowing the random number algorithm from the libmpg123 dither code.
// xorshift random number generator
// See http://www.jstatsoft.org/v08/i14/paper on XOR shift random number generators.
static float rand_xorshift32(uint32_t *seed)
{
	union
	{
		uint32_t i;
		float f;
	} fi;
	
	fi.i = *seed;
	fi.i ^= (fi.i<<13);
	fi.i ^= (fi.i>>17);
	fi.i ^= (fi.i<<5);
	*seed = fi.i;
	/* scale the number to [-0.5, 0.5] */
#ifdef IEEE_FLOAT
	fi.i = (fi.i>>9)|0x3f800000;
	fi.f -= 1.5f;
#else
	fi.f = (double)fi.i / 4294967295.0;
	fi.f -= 0.5f;
#endif
	return fi.f;
}
#endif

#endif
