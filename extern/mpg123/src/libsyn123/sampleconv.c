/*
	sampleconv: libsyn123 sample conversion functions

	copyright 2018 by the mpg123 project
	licensed under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org

	initially written by Thomas Orgis

	This code directly contains wave generators and the generic entry
	code for libsyn123. The waves started the whole thing and stay here
	for now. Other signal generators go into separate files.

	This is the part of the mpg123 codebase where C99 first got into use
	for real, but still with some caution.
*/

#define NO_GROW_BUF
#define NO_SMAX
#define RAND_XORSHIFT32
#include "syn123_int.h"
#include "../common/sample.h"
#include "../common/debug.h"
// Avoid conflict with pragmas in isnan() and friends.
#undef warning

/* Conversions between native byte order encodings. */

#include "g711_impl.h"

/* Some trivial conversion functions. No need for another library. */

/* 1. From double/float to various. */

// We have symmetric signals with +/- 2^(n-1)-1 in mind, nevertheless
// clipping the negative side at -2^(n-1) for lossless conversion of
// any input in integer encoding.
// There might be possible optimizations. We hope for some auto-vectorization.
// And also, nowadays, CPUs can do so much while they wait for memory ...
// Dither noise is always a parameter, hopefully optimized away if zero
// in the ditherless code.
#define CONV(name, ftype, type, maxval) \
static type name(ftype d, float dither, size_t *clips) \
{ \
	type imax = maxval; \
	if(isnan(d)) \
	{ \
		d = 0.; \
		++*clips; \
	} \
	d *= imax; \
	d += dither; \
	if(d>=0) \
	{ \
		d += 0.5; \
		if(d > imax){ ++*clips; return    imax; } \
		else        {           return (type)d; } \
	} \
	else \
	{ \
		d -= 0.5; \
		if(d < (-imax-1)){++*clips; return (-imax-1); } \
		else             {          return   (type)d; } \
	} \
}

/* If there is an actual 24 bit integer with cleared last byte, */
/* single precision should be accurate. Otherwise, double is needed. */
CONV(d_s32, double, int32_t, 2147483647L)
CONV(f_s16, float, int16_t, 32767)
CONV(f_s8,  float, int8_t,  127)

// If the compiler can inline two levels ... the dithering code
// should be optimized away for the non-dithered calls.
static uint8_t         f_u8(float f, float dither, size_t *clips)
{
	return CONV_SU8(f_s8(f,dither,clips));
}
static uint16_t       f_u16(float f, float dither, size_t *clips)
{
	return CONV_SU16(f_s16(f,dither,clips));
}
static uint32_t       d_u32(double d, float dither, size_t *clips)
{
	return CONV_SU32(d_s32(d,dither,clips));
}
static unsigned char f_alaw(float f, float dither, size_t *clips)
{
	return linear2alaw(f_s16(f,dither,clips));
}
static unsigned char f_ulaw(float f, float dither, size_t *clips)
{
	return linear2ulaw(f_s16(f,dither,clips));
}

// 2. From various to double/float.
// I used to limit the most negative number to -1 here, but that
// counters the careful non-clipping of that value when converting
// from float. When I want to reproduce the whole range of given
// integer input, I shall just divide here.

static double s32_d(int32_t n)
{
	return (double)n/2147483647.;
}
static float  s16_f(int16_t n)
{
	return (float)n/32767.f;
}
static float  s8_f (int8_t n)
{
	return (float)n/127.f;
}
static float  u8_f (uint8_t u)      { return s8_f(CONV_US8(u));     }
static float  u16_f(uint16_t u)     { return s16_f(CONV_US16(u));   }
static double u32_d(uint32_t u)     { return s32_d(CONV_US32(u));   }
static float alaw_f(unsigned char n){ return s16_f(alaw2linear(n)); }
static float ulaw_f(unsigned char n){ return s16_f(ulaw2linear(n)); }

size_t attribute_align_arg
syn123_clip(void *buf, int encoding, size_t samples)
{
	if(!buf)
		return 0;

	size_t clipped = 0;
	#define CLIPCODE(type) \
	{ \
		type *p = buf; \
		for(size_t i=0; i<samples; ++i) \
			if(isnan(p[i]))     { p[i] =  0.0; ++clipped; } \
			else if(p[i] < -1.0){ p[i] = -1.0; ++clipped; } \
			else if(p[i] > +1.0){ p[i] = +1.0; ++clipped; } \
	}
	switch(encoding)
	{
		case MPG123_ENC_FLOAT_32:
			CLIPCODE(float)
		break;
		case MPG123_ENC_FLOAT_64:
			CLIPCODE(double)
		break;
	}
	#undef CLIPCODE
	return clipped;
}

size_t attribute_align_arg
syn123_soft_clip( void *buf, int encoding, size_t samples
,	double limit, double width, syn123_handle *sh )
{
	if(!buf)
		return 0;
	if(width > limit)
		limit = width;
	size_t clipped = 0;
	// I wonder how well branch predictors work with this tangle.
	#define CLIPCODE(type) \
	{ \
		type w = (type)width; \
		type ww = w*w; \
		type w21 = 2*w-(type)limit; \
		type h = (type)limit-w; \
		type l = -(type)limit+w; \
		type *p = buf; \
		for(size_t i=0; i<samples; ++i) \
		{ \
			if(isnan(p[i]))  { p[i] = (type)0.;                     ++clipped; } \
			else if(p[i] > h){ p[i] =  (type)limit - ww/(w21+p[i]); ++clipped; } \
			else if(p[i] < l){ p[i] = -(type)limit + ww/(w21-p[i]); ++clipped; } \
		} \
	}
	switch(encoding)
	{
		case MPG123_ENC_FLOAT_32:
			CLIPCODE(float)
		break;
		case MPG123_ENC_FLOAT_64:
			CLIPCODE(double)
		break;
	}
	#undef CLIPCODE
	if(sh)
	{
		char *cbuf = buf;
		int mixenc = syn123_mixenc(encoding, encoding);
		int mixframe = MPG123_SAMPLESIZE(mixenc);
		int inframe = MPG123_SAMPLESIZE(encoding);
		if(!mixenc || !mixframe || !inframe)
			return 0;
		// Use the whole workbuf, both halves.
		int mbufblock = 2*bufblock*sizeof(double)/mixframe;
		mdebug("mbufblock=%i (enc %i)", mbufblock, mixenc);
		while(samples)
		{
			int block = (int)smin(samples, mbufblock);
			int err = syn123_conv(
				sh->workbuf, mixenc, sizeof(sh->workbuf)
			,	cbuf, encoding, inframe*block
			,	NULL, NULL, NULL );
			if(!err)
			{
				clipped += syn123_soft_clip(sh->workbuf, mixenc, block, limit, width, NULL);
				// No additional clipping can happen here.
				err = syn123_conv(
					cbuf, encoding, inframe*block
				,	sh->workbuf, mixenc, mixframe*block
				,	NULL, NULL, NULL );
			}
			if(err)
			{
				mdebug("conv error: %i", err);
				return clipped; // If there really is an error, this should be zero, still.
			}
			cbuf += block*inframe;
			samples -= block;
		}
	}
	return clipped;
}

// All together in a happy matrix game, but only directly to/from
// double or float.

#define FROM_FLT(type) \
{ type *tsrc = src; type *tend = tsrc+samples; char *tdest = dst; \
switch(dst_enc) \
{ \
	case MPG123_ENC_SIGNED_16: \
		for(; tsrc!=tend; ++tsrc, tdest+=2) \
			*(int16_t*)tdest = f_s16(*tsrc, 0, &clips); \
	break; \
	case MPG123_ENC_SIGNED_32: \
		for(; tsrc!=tend; ++tsrc, tdest+=4) \
			*(int32_t*)tdest = d_s32(*tsrc, 0, &clips); \
	break; \
	case MPG123_ENC_SIGNED_24: \
		for(; tsrc!=tend; ++tsrc, tdest+=3) \
		{ \
			union { int32_t i; char c[4]; } tmp; \
			tmp.i = d_s32(*tsrc, 0, &clips); \
			DROP4BYTE(tdest, tmp.c); \
		} \
	break; \
	case MPG123_ENC_SIGNED_8: \
		for(; tsrc!=tend; ++tsrc, tdest+=1) \
			*(int8_t*)tdest = f_s8(*tsrc, 0, &clips); \
	break; \
	case MPG123_ENC_ULAW_8: \
		for(; tsrc!=tend; ++tsrc, tdest+=1) \
			*(unsigned char*)tdest = f_ulaw(*tsrc, 0, &clips); \
	break; \
	case MPG123_ENC_ALAW_8: \
		for(; tsrc!=tend; ++tsrc, tdest+=1) \
			*(unsigned char*)tdest = f_alaw(*tsrc, 0, &clips); \
	break; \
	case MPG123_ENC_UNSIGNED_8: \
		for(; tsrc!=tend; ++tsrc, tdest+=1) \
			*(uint8_t*)tdest = f_u8(*tsrc, 0, &clips); \
	break; \
	case MPG123_ENC_UNSIGNED_16: \
		for(; tsrc!=tend; ++tsrc, tdest+=2) \
			*(uint16_t*)tdest = f_u16(*tsrc, 0, &clips); \
	break; \
	case MPG123_ENC_UNSIGNED_24: \
		for(; tsrc!=tend; ++tsrc, tdest+=3) \
		{ \
			union { uint32_t i; char c[4]; } tmp; \
			tmp.i = d_u32(*tsrc, 0, &clips); \
			DROP4BYTE(tdest, tmp.c); \
		} \
	break; \
	case MPG123_ENC_UNSIGNED_32: \
		for(; tsrc!=tend; ++tsrc, tdest+=4) \
			*(uint32_t*)tdest = d_u32(*tsrc, 0, &clips); \
	break; \
	case MPG123_ENC_FLOAT_32: \
		for(; tsrc!=tend; ++tsrc, tdest+=4) \
			*((float*)tdest) = (float)*tsrc; \
	break; \
	case MPG123_ENC_FLOAT_64: \
		for(; tsrc!=tend; ++tsrc, tdest+=8) \
			*(double*)tdest = *tsrc; \
	break; \
	default: \
		return SYN123_BAD_CONV; \
}}


static float dither_noise(uint32_t *seed)
{
	return rand_xorshift32(seed) + rand_xorshift32(seed);
}

// Dithering to 24 bit needs attention: I think just
// adding 255*[-1:1] before cutting off the last byte should do the trick.
#define NOISE dither_noise(&sh->dither_seed)
#define FROM_FLT_DITHER(type) \
{ type *tsrc = src; type *tend = tsrc+samples; char *tdest = dst; \
switch(dst_enc) \
{ \
	case MPG123_ENC_SIGNED_16: \
		for(; tsrc!=tend; ++tsrc, tdest+=2) \
			*(int16_t*)tdest = f_s16(*tsrc, NOISE, &clips); \
	break; \
	case MPG123_ENC_SIGNED_32: \
		for(; tsrc!=tend; ++tsrc, tdest+=4) \
			*(int32_t*)tdest = d_s32(*tsrc, NOISE, &clips); \
	break; \
	case MPG123_ENC_SIGNED_24: \
		for(; tsrc!=tend; ++tsrc, tdest+=3) \
		{ \
			union { int32_t i; char c[4]; } tmp; \
			tmp.i = d_s32(*tsrc, NOISE*255, &clips); \
			DROP4BYTE(tdest, tmp.c); \
		} \
	break; \
	case MPG123_ENC_SIGNED_8: \
		for(; tsrc!=tend; ++tsrc, tdest+=1) \
			*(int8_t*)tdest = f_s8(*tsrc, NOISE, &clips); \
	break; \
	case MPG123_ENC_ULAW_8: \
		for(; tsrc!=tend; ++tsrc, tdest+=1) \
			*(unsigned char*)tdest = f_ulaw(*tsrc, NOISE, &clips); \
	break; \
	case MPG123_ENC_ALAW_8: \
		for(; tsrc!=tend; ++tsrc, tdest+=1) \
			*(unsigned char*)tdest = f_alaw(*tsrc, NOISE, &clips); \
	break; \
	case MPG123_ENC_UNSIGNED_8: \
		for(; tsrc!=tend; ++tsrc, tdest+=1) \
			*(uint8_t*)tdest = f_u8(*tsrc, NOISE, &clips); \
	break; \
	case MPG123_ENC_UNSIGNED_16: \
		for(; tsrc!=tend; ++tsrc, tdest+=2) \
			*(uint16_t*)tdest = f_u16(*tsrc, NOISE, &clips); \
	break; \
	case MPG123_ENC_UNSIGNED_24: \
		for(; tsrc!=tend; ++tsrc, tdest+=3) \
		{ \
			union { uint32_t i; char c[4]; } tmp; \
			tmp.i = d_u32(*tsrc, NOISE*255, &clips); \
			DROP4BYTE(tdest, tmp.c); \
		} \
	break; \
	case MPG123_ENC_UNSIGNED_32: \
		for(; tsrc!=tend; ++tsrc, tdest+=4) \
			*(uint32_t*)tdest = d_u32(*tsrc, NOISE, &clips); \
	break; \
	case MPG123_ENC_FLOAT_32: \
		for(; tsrc!=tend; ++tsrc, tdest+=4) \
			*((float*)tdest) = (float)*tsrc; \
	break; \
	case MPG123_ENC_FLOAT_64: \
		for(; tsrc!=tend; ++tsrc, tdest+=8) \
			*(double*)tdest = *tsrc; \
	break; \
	default: \
		return SYN123_BAD_CONV; \
}}

#define TO_FLT(type) \
{ type* tdest = dst; type* tend = tdest + samples; char * tsrc = src; \
switch(src_enc) \
{ \
	case MPG123_ENC_SIGNED_16: \
		for(; tdest!=tend; ++tdest, tsrc+=2) \
			*tdest = s16_f(*(int16_t*)tsrc); \
	break; \
	case MPG123_ENC_SIGNED_32: \
		for(; tdest!=tend; ++tdest, tsrc+=4) \
			*tdest = (type)s32_d(*(int32_t*)tsrc); \
	break; \
	case MPG123_ENC_SIGNED_24: \
		for(; tdest!=tend; ++tdest, tsrc+=3) \
		{ \
			union { int32_t i; char c[4]; } tmp; \
			ADD4BYTE(tmp.c, tsrc); \
			*tdest = (type)s32_d(tmp.i); \
		} \
	break; \
	case MPG123_ENC_SIGNED_8: \
		for(; tdest!=tend; ++tdest, tsrc+=1) \
			*tdest = s8_f(*(int8_t*)tsrc); \
	break; \
	case MPG123_ENC_ULAW_8: \
		for(; tdest!=tend; ++tdest, tsrc+=1) \
			*tdest = ulaw_f(*(unsigned char*)tsrc); \
	break; \
	case MPG123_ENC_ALAW_8: \
		for(; tdest!=tend; ++tdest, tsrc+=1) \
			*tdest = alaw_f(*(unsigned char*)tsrc); \
	break; \
	case MPG123_ENC_UNSIGNED_8: \
		for(; tdest!=tend; ++tdest, tsrc+=1) \
			*tdest = u8_f(*(uint8_t*)tsrc); \
	break; \
	case MPG123_ENC_UNSIGNED_16: \
		for(; tdest!=tend; ++tdest, tsrc+=2) \
			*tdest = u16_f(*(uint16_t*)tsrc); \
	break; \
	case MPG123_ENC_UNSIGNED_24: \
		for(; tdest!=tend; ++tdest, tsrc+=3) \
		{ \
			union { uint32_t i; char c[4]; } tmp; \
			ADD4BYTE(tmp.c, tsrc); \
			*tdest = (type)u32_d(tmp.i); \
		} \
	break; \
	case MPG123_ENC_UNSIGNED_32: \
		for(; tdest!=tend; ++tdest, tsrc+=4) \
			*tdest = (type)(u32_d(*(uint32_t*)tsrc)); \
	break; \
	case MPG123_ENC_FLOAT_32: \
		for(; tdest!=tend; ++tdest, tsrc+=4) \
			*tdest = *(float*)tsrc; \
	break; \
	case MPG123_ENC_FLOAT_64: \
		for(; tdest!=tend; ++tdest, tsrc+=8) \
			*tdest = (type)(*(double*)tsrc); \
	break; \
	default: \
		return SYN123_BAD_CONV; \
}}

int attribute_align_arg
syn123_mixenc(int src_enc, int dst_enc)
{
	int isize = MPG123_SAMPLESIZE(src_enc);
	int osize = MPG123_SAMPLESIZE(dst_enc);
	if(!isize || !osize)
		return 0;
	else // Only if both sides warrant high precision, double is used.
		return ( src_enc != MPG123_ENC_FLOAT_32 && isize > 3 &&
			dst_enc != MPG123_ENC_FLOAT_32 && osize > 3 )
		?	MPG123_ENC_FLOAT_64
		:	MPG123_ENC_FLOAT_32;	
}

// Return non-zero if the indicated conversion will involve roundoff errors
// that can be improved using dithering.
static int need_dither(int from, int to)
{
	// No dithering when converting to float ...
	if(to & MPG123_ENC_FLOAT)
		return 0;
	// ... and always dither when going from float to integer.
	if(from & MPG123_ENC_FLOAT)
		return 1;
	// When converting from integer to integer, rounding occurs if the
	// precision goes down.
	unsigned int fromsize = MPG123_SAMPLESIZE(from);
	unsigned int tosize   = MPG123_SAMPLESIZE(to);
	if(to & MPG123_ENC_8)
	{
		// The funky 8 bit encodings encode 16 bit values.
		if(to == MPG123_ENC_ALAW_8 || to == MPG123_ENC_ULAW_8)
			tosize = 2;
	}
	// If the output encoding width is same or bigger, no rounding occurs.
	return (tosize < fromsize) ? 1 : 0;
}

int attribute_align_arg
syn123_conv( void * MPG123_RESTRICT dst, int dst_enc, size_t dst_size
,	void * MPG123_RESTRICT src, int src_enc, size_t src_bytes
,	size_t *dst_bytes, size_t *clipped, syn123_handle *sh )
{
	size_t srcframe = MPG123_SAMPLESIZE(src_enc);
	size_t dstframe = MPG123_SAMPLESIZE(dst_enc);
	// Take the provided do_dither setting and make sure to have it cleared.
	int do_dither = 0;
	if(sh)
	{
		do_dither = sh->do_dither;
		sh->do_dither = 0;
	}
	if(!srcframe || !dstframe)
		return SYN123_BAD_ENC;
	size_t samples = src_bytes/srcframe;
	size_t clips = 0;
	mdebug( "conv from %i (%i) to %i (%i), %zu into %zu bytes, dither %i/%i"
	,	src_enc, MPG123_SAMPLESIZE(src_enc)
	,	dst_enc, MPG123_SAMPLESIZE(dst_enc)
	,	src_bytes, dst_size
	,	sh ? sh->dither : 0, do_dither );
	if(!dst || !src)
		return SYN123_BAD_BUF;
	if(samples*srcframe != src_bytes)
		return SYN123_BAD_CHOP;
	if(samples*dstframe > dst_size)
		return SYN123_BAD_SIZE;
	if(src_enc == dst_enc)
		memcpy(dst, src, samples*dstframe);
	// Always shortcut for converting to float, not considering dither.
	else if(dst_enc & MPG123_ENC_FLOAT)
	{
		debug("to_flt");
		if(dst_enc == MPG123_ENC_FLOAT_64)
			TO_FLT(double)
		else if(dst_enc == MPG123_ENC_FLOAT_32)
			TO_FLT(float)
		else
			return SYN123_BAD_CONV;
	}
	// If converting from float, possibly add dither noise.
	else if(src_enc & MPG123_ENC_FLOAT)
	{
		// Either this is a direct call and dithering is decided by given encodings
		// or this is an intermediate conversion called from syn123_conv() itself,
		// where the outer instance decided to enforce (1) or disable (-1) dithering.
		if( sh && sh->dither 
		&&	( do_dither == 1
			||	(do_dither == 0 && need_dither(src_enc, dst_enc))
			) )
		{
			debug("from_flt dither");
			if(src_enc == MPG123_ENC_FLOAT_64)
				FROM_FLT_DITHER(double)
			else if(src_enc == MPG123_ENC_FLOAT_32)
				FROM_FLT_DITHER(float)
			else
				return SYN123_BAD_CONV;
		}
		else
		{
			debug("from_flt");
			if(src_enc == MPG123_ENC_FLOAT_64)
				FROM_FLT(double)
			else if(src_enc == MPG123_ENC_FLOAT_32)
				FROM_FLT(float)
			else
				return SYN123_BAD_CONV;
		}
	}
	else if(sh)
	{
		char *cdst = dst;
		char *csrc = src;
		int mixenc = syn123_mixenc(src_enc, dst_enc);
		int mixframe = MPG123_SAMPLESIZE(mixenc);
		if(!mixenc || !mixframe)
			return SYN123_BAD_CONV;
		// Use the whole workbuf, both halves.
		int mbufblock = 2*bufblock*sizeof(double)/mixframe;
		mdebug("mbufblock=%i (enc %i)", mbufblock, mixenc);
		// Abuse the handle workbuf for intermediate storage.
		size_t samples_left = samples;
		// The final conversion needs to apply dithering depending on
		// actual input and output encodings.
		// The intermediate calls get dithering on/off enforced via sh->do_dither.
		if(!do_dither)
			do_dither = sh->dither && need_dither(src_enc, dst_enc) ? 1 : -1;
		while(samples_left)
		{
			int block = (int)smin(samples_left, mbufblock);
			size_t clipped_now = 0;
			int err = syn123_conv(
				sh->workbuf, mixenc, sizeof(sh->workbuf)
			,	csrc, src_enc, srcframe*block
			,	NULL, NULL, NULL );
			sh->do_dither = do_dither; // possibly dither now
			if(!err)
				err = syn123_conv(
					cdst, dst_enc, dstframe*block
				,	sh->workbuf, mixenc, mixframe*block
				,	NULL, &clipped_now, sh );
			if(err)
			{
				mdebug("conv error: %i", err);
				return SYN123_BAD_CONV;
			}
			clips += clipped_now;
			cdst += dstframe*block;
			csrc += srcframe*block;
			samples_left -= block;
		}
	} else
		return SYN123_BAD_CONV;
	if(dst_bytes)
		*dst_bytes = dstframe*samples;
	if(clipped)
		*clipped = clips;
	return SYN123_OK;
}

// Any compiler worth its salt should be able to unroll the
// fixed loops resultung from this macro. And yes, I should
// measure that. Goal is the hardware memory bandwidth.
// Any impact from putting the channel loop to the outside?

#define BYTEMULTIPLY(dest, src, bytes, count, channels) \
{ \
	for(size_t i=0; i<count; ++i) \
	{ \
		for(int j=0; j<channels; ++j) \
		for(size_t b=0; b<bytes; ++b) \
			((char*)dest)[(i*channels+j)*bytes+b] = ((char*)src)[i*bytes+b]; \
	} \
}

#define BYTEINTERLEAVE(dest, src, bytes, count, channels) \
{ \
	for(size_t i=0; i<count; ++i) \
	{ \
		for(int j=0; j<channels; ++j) \
		for(size_t b=0; b<bytes; ++b) \
			((char*)dest)[(i*channels+j)*bytes+b] = ((char**)src)[j][i*bytes+b]; \
	} \
}

#define BYTEDEINTERLEAVE(dest, src, bytes, count, channels) \
{ \
	for(size_t i=0; i<count; ++i) \
	{ \
		for(int j=0; j<channels; ++j) \
		for(size_t b=0; b<bytes; ++b) \
			((char**)dest)[j][i*bytes+b] = ((char*)src)[(i*channels+j)*bytes+b]; \
	} \
}

/* Special case of multiplying a mono stream. */
void attribute_align_arg
syn123_mono2many( void * MPG123_RESTRICT dst, void * MPG123_RESTRICT src
, int channels, size_t samplesize, size_t samplecount )
{
#ifndef SYN123_NO_CASES
	switch(channels)
	{
		case 1:
			memcpy(dst, src, samplesize*samplecount);
		break;
		case 2:
			switch(samplesize)
			{
				case 1:
					BYTEMULTIPLY(dst, src, 1, samplecount, 2)
				break;
				case 2:
					BYTEMULTIPLY(dst, src, 2, samplecount, 2)
				break;
				case 3:
					BYTEMULTIPLY(dst, src, 3, samplecount, 2)
				break;
				case 4:
					BYTEMULTIPLY(dst, src, 4, samplecount, 2)
				break;
				default:
					BYTEMULTIPLY(dst, src, samplesize, samplecount, 2)
				break;
			}
		break;
		default:
			switch(samplesize)
			{
				case 1:
					BYTEMULTIPLY(dst, src, 1, samplecount, channels)
				break;
				case 2:
					BYTEMULTIPLY(dst, src, 2, samplecount, channels)
				break;
				case 3:
					BYTEMULTIPLY(dst, src, 3, samplecount, channels)
				break;
				case 4:
					BYTEMULTIPLY(dst, src, 4, samplecount, channels)
				break;
				default:
					BYTEMULTIPLY(dst, src, samplesize, samplecount, channels)
				break;
			}
		break;
	}
#else
	BYTEMULTIPLY(dst, src, samplesize, samplecount, channels)
#endif
}

void attribute_align_arg
syn123_interleave(void * MPG123_RESTRICT dst, void ** MPG123_RESTRICT src
,	int channels, size_t samplesize, size_t samplecount)
{
#ifndef SYN123_NO_CASES
	switch(channels)
	{
		case 1:
			memcpy(dst, src, samplesize*samplecount);
		break;
		case 2:
			switch(samplesize)
			{
				case 1:
					BYTEINTERLEAVE(dst, src, 1, samplecount, 2)
				break;
				case 2:
					BYTEINTERLEAVE(dst, src, 2, samplecount, 2)
				break;
				case 3:
					BYTEINTERLEAVE(dst, src, 3, samplecount, 2)
				break;
				case 4:
					BYTEINTERLEAVE(dst, src, 4, samplecount, 2)
				break;
				default:
					BYTEINTERLEAVE(dst, src, samplesize, samplecount, 2)
				break;
			}
		break;
		default:
			switch(samplesize)
			{
				case 1:
					BYTEINTERLEAVE(dst, src, 1, samplecount, channels)
				break;
				case 2:
					BYTEINTERLEAVE(dst, src, 2, samplecount, channels)
				break;
				case 3:
					BYTEINTERLEAVE(dst, src, 3, samplecount, channels)
				break;
				case 4:
					BYTEINTERLEAVE(dst, src, 4, samplecount, channels)
				break;
				default:
					BYTEINTERLEAVE(dst, src, samplesize, samplecount, channels)
				break;
			}
		break;
	}
#else
	BYTEINTERLEAVE(dst, src, samplesize, samplecount, channels)
#endif
}

void attribute_align_arg
syn123_deinterleave(void ** MPG123_RESTRICT dst, void * MPG123_RESTRICT src
,	int channels, size_t samplesize, size_t samplecount)
{
#ifndef SYN123_NO_CASES
	switch(channels)
	{
		case 1:
			memcpy(dst[0], src, samplesize*samplecount);
		break;
		case 2:
			switch(samplesize)
			{
				case 1:
					BYTEDEINTERLEAVE(dst, src, 1, samplecount, 2)
				break;
				case 2:
					BYTEDEINTERLEAVE(dst, src, 2, samplecount, 2)
				break;
				case 3:
					BYTEDEINTERLEAVE(dst, src, 3, samplecount, 2)
				break;
				case 4:
					BYTEDEINTERLEAVE(dst, src, 4, samplecount, 2)
				break;
				default:
					BYTEDEINTERLEAVE(dst, src, samplesize, samplecount, 2)
				break;
			}
		break;
		default:
			switch(samplesize)
			{
				case 1:
					BYTEDEINTERLEAVE(dst, src, 1, samplecount, channels)
				break;
				case 2:
					BYTEDEINTERLEAVE(dst, src, 2, samplecount, channels)
				break;
				case 3:
					BYTEDEINTERLEAVE(dst, src, 3, samplecount, channels)
				break;
				case 4:
					BYTEDEINTERLEAVE(dst, src, 4, samplecount, channels)
				break;
				default:
					BYTEDEINTERLEAVE(dst, src, samplesize, samplecount, channels)
				break;
			}
		break;
	}
#else
	BYTEDEINTERLEAVE(dst, src, samplesize, samplecount, channels)
#endif
}

#define MIX_CODE(type,scc,dcc) \
	for(size_t i=0; i<samples; ++i) \
	{ \
		for(int dc=0; dc<dcc; ++dc) \
		{ \
			for(int sc=0; sc<scc; ++sc) \
				dst[SYN123_IOFF(i,dc,dcc)] += \
					(type)mixmatrix[SYN123_IOFF(dc,sc,scc)] * src[SYN123_IOFF(i,sc,scc)]; \
		} \
	}

// I decided against optimizing for mixing factors of 1, unity matrix ...
// A floating point multiplication is not that expensive and there might
// be value in the runtime of the function not depending on the matrix
// values, only on channel setup and sample count.
// I might trust the compiler here to hardcode the case value for
// src/dst_channels. But why take the risk?
#ifndef SYN123_NO_CASES
#define SYN123_MIX_FUNC(type) \
	switch(src_channels) \
	{ \
		case 1: \
			switch(dst_channels) \
			{ \
				case 1: \
					MIX_CODE(type,1,1) \
				break; \
				case 2: \
					MIX_CODE(type,1,2) \
				break; \
				default: \
					MIX_CODE(type,1,dst_channels) \
			} \
		break; \
		case 2: \
			switch(dst_channels) \
			{ \
				case 1: \
					MIX_CODE(type,2,1) \
				break; \
				case 2: \
					MIX_CODE(type,2,2) \
				break; \
				default: \
					MIX_CODE(type,2,dst_channels) \
			} \
		break; \
		default: \
			MIX_CODE(type,src_channels, dst_channels) \
	}
#else
#define SYN123_MIX_FUNC(type) \
	MIX_CODE(type, src_channels, dst_channels)
#endif

static void syn123_mix_f32( float * MPG123_RESTRICT dst, int dst_channels
,	float * MPG123_RESTRICT src, int src_channels
,	const double * MPG123_RESTRICT mixmatrix
,	size_t samples )
{
	debug("syn123_mix_f32");
	SYN123_MIX_FUNC(float)
}

static void syn123_mix_f64( double * MPG123_RESTRICT dst, int dst_channels
,	double * MPG123_RESTRICT src, int src_channels
,	const double * MPG123_RESTRICT mixmatrix
,	size_t samples )
{
	debug("syn123_mix_f64");
	SYN123_MIX_FUNC(double)
}

int attribute_align_arg
syn123_mix( void * MPG123_RESTRICT dst, int dst_enc, int dst_channels
,	void * MPG123_RESTRICT src, int src_enc, int src_channels
,	const double * mixmatrix
,	size_t samples, int silence, size_t *clipped, syn123_handle *sh )
{
	int err = 0;
	size_t clips = 0;
	if(src_channels < 1 || dst_channels < 1)
	{
		err = SYN123_BAD_FMT;
		goto mix_end;
	}
	if(!dst || !src || !mixmatrix)
	{
		err = SYN123_BAD_BUF;
		goto mix_end;
	}
	if(dst_enc == src_enc) switch(dst_enc)
	{
		case MPG123_ENC_FLOAT_32:
			if(silence)
				for(size_t i=0; i<samples*dst_channels; ++i)
					((float*)dst)[i] = 0;
			syn123_mix_f32( dst, dst_channels, src, src_channels
			,	mixmatrix, samples );
			goto mix_end;
		break;
		case MPG123_ENC_FLOAT_64:
			if(silence)
				for(size_t i=0; i<samples*dst_channels; ++i)
					((double*)dst)[i] = 0;
			syn123_mix_f64( dst, dst_channels, src, src_channels
			,	mixmatrix, samples );
			goto mix_end;
		break;
	}
	// If still here: Some conversion needed.
	if(!sh)
	{
		err = SYN123_BAD_ENC;
		goto mix_end;
	}
	else
	{
		debug("mix with conversion");
		char *cdst = dst;
		char *csrc = src;
		int srcframe = MPG123_SAMPLESIZE(src_enc)*src_channels;
		int dstframe = MPG123_SAMPLESIZE(dst_enc)*dst_channels;
		if(!srcframe || !dstframe)
		{
			err = SYN123_BAD_ENC;
			goto mix_end;
		}
		int mixenc = syn123_mixenc(src_enc, dst_enc);
		int mixinframe = MPG123_SAMPLESIZE(mixenc)*src_channels;
		int mixoutframe = MPG123_SAMPLESIZE(mixenc)*dst_channels;
		int mixframe = mixinframe > mixoutframe ? mixinframe : mixoutframe;
		if(!mixenc || !mixinframe || !mixoutframe)
		{
			err = SYN123_BAD_CONV;
			goto mix_end;
		}
		// Mix from buffblock[0] to buffblock[1].
		int mbufblock = bufblock*sizeof(double)/mixframe;
		mdebug("mbufblock=%i (enc %i)", mbufblock, mixenc);
		// Need at least one sample per round to avoid endless loop.
		// Of course, we would prefer more, but it's your fault for
		// giving an excessive amount of channels without handling conversion
		// beforehand.
		if(mbufblock < 1)
		{
			err = SYN123_BAD_CONV;
			goto mix_end;
		}
		while(samples)
		{
			int block = (int)smin(samples, mbufblock);
			err = syn123_conv( sh->workbuf[0], mixenc, sizeof(sh->workbuf[0])
			,	csrc, src_enc, srcframe*block
			,	NULL, NULL, NULL );
			if(err)
				goto mix_end;
			// Initialize to zero or convert from old output signal.
			if(silence)
			{
				if(mixenc == MPG123_ENC_FLOAT_32)
					for(int i=0; i<block*dst_channels; ++i)
						((float*)(sh->workbuf[1]))[i] = 0.;
				else
					for(int i=0; i<block*dst_channels; ++i)
						sh->workbuf[1][i] = 0.;
			}
			else
				err = syn123_conv( sh->workbuf[1], mixenc, sizeof(sh->workbuf[1])
				,	cdst, dst_enc, dstframe*block
				,	NULL, NULL, NULL );
			// Now mix one work buffer into the other.
			if(!err)
				err = syn123_mix( sh->workbuf[1], mixenc, dst_channels
				,	sh->workbuf[0], mixenc, src_channels, mixmatrix, block
				,	0, NULL, NULL );
			if(err)
				goto mix_end;
			// And convert to the final output format.
			size_t clips_block = 0;
			err = syn123_conv(
					cdst, dst_enc, dstframe*block
				,	sh->workbuf[1], mixenc, mixoutframe*block
				,	NULL, &clips_block, NULL );
			clips += clips_block;
			if(err)
				goto mix_end;
			cdst += dstframe*block;
			csrc += srcframe*block;
			samples -= block;
		}
	}
mix_end:
	if(clipped)
		*clipped = clips;
	return err;
}

/* All the byte-swappery for those little big endian boxes. */

#include "../common/swap_bytes_impl.h"

void attribute_align_arg
syn123_swap_bytes(void* buf, size_t samplesize, size_t samplecount)
{
	swap_bytes(buf, samplesize, samplecount);
}

void attribute_align_arg
syn123_host2le(void *buf, size_t samplesize, size_t samplecount)
{
#ifdef WORDS_BIGENDIAN
	swap_bytes(buf, samplesize, samplecount);
#endif
}

void attribute_align_arg
syn123_host2be(void *buf, size_t samplesize, size_t samplecount)
{
#ifndef WORDS_BIGENDIAN
	swap_bytes(buf, samplesize, samplecount);
#endif
}

void attribute_align_arg
syn123_le2host(void *buf, size_t samplesize, size_t samplecount)
{
#ifdef WORDS_BIGENDIAN
	swap_bytes(buf, samplesize, samplecount);
#endif
}

void attribute_align_arg
syn123_be2host(void *buf, size_t samplesize, size_t samplecount)
{
#ifndef WORDS_BIGENDIAN
	swap_bytes(buf, samplesize, samplecount);
#endif
}
