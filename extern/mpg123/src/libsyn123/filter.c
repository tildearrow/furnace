/*
	filter: apply IIR/FIR filters using Direct Form II

	copyright 2019 by the mpg123 project
	licensed under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org

	initially written by Thomas Orgis

	Since I spent a lot of time tuning filters for my resampler, I figured
	I should add generic digital filters to the syn123 API. There are
	multiple independent implementations of filters inside the resampler,
	using hardcoded parameters. They should be functionally equivalent to
	this generic code. Let it serve as a reference.

	The code should be straightforward to read, but is messed up a bit
	by the task to be runtime-tunable for single or double precision.
	Also, the coefficients are stored multiple times in a shifted manner
	to avoid shifting the filter history around and keeping efficient
	coefficient access. I guess this is a pretty standard optimization
	which worked well for the resampler.

	I use macros. Don't be scared.
*/

#define NO_GROW_BUF
#define NO_SMAX
#define NO_SMIN
#include "syn123_int.h"

// 0 ... ORDER
// b0 stored
// a0 == 1
// ringbuffer-optimized storage of the others
// b[order][order]
// a[order][order]
// historic values, ringbuffer
// w[channels][order]
// So, one piece of storage of size order*(2*order+channels).

// Theoretical limit on maximum order: data block must fit into size_t.
// Checking overflow of order*order*2 + order*channels, in that order.
#define ORDER_TOO_BIG(order, channels) \
	((order) == 0 || (channels) == 0) \
	?	0 \
	:	( (order) > SIZE_MAX/2/(order) \
		?	1 \
		:	( (order) > (SIZE_MAX-(order)*(order)*2)/(channels) \
			?	1 \
			:	0 \
			)\
		)

// size of filter data block
#define F_DATABLOCK(f, channels) ((f).order*(2*(f).order+channels))

// base addresses of b and a coefficient sets, ringbuffered
#define Fb(f) ((f).data)
#define Fa(f) ((f).data+(f).order*(f).order)
// address of currently active coefficient set
// the base is either f.b or f.a (which may be NULL)
#define Fc(f, base) ((base) ? ((base)+(f).order*(f).n1) : NULL)
// base address of current channel's history values
#define Fw(f, c) ((f).data+F_DATABLOCK((f), (c)))

#define FILTER_STRUCT(name, type) \
struct name \
{ \
	int flow; /* if the filter has had first data and is in flow now */ \
	unsigned int order; /* filter order */ \
	unsigned int n1; /* current position of most recent past */ \
	type init_scale; /* scale for first sample as endless history */ \
	type b0; /* coefficient b_0 */ \
	type *data; /* dynamic coefficient and history data */ \
	type *b;    /* b coefficients b_1 ... b_N, pointer into the above */ \
	type *a;    /* same for a, or NULL if non-recursive */ \
};

FILTER_STRUCT(f_filter, float)
FILTER_STRUCT(d_filter, double)

// Initialization of Direct Form II history.
// Return a constant value for w[n] that represents an endless history of given insample.
// Given the recursive definition: w[n] = x[n] - sum_i(a[i] * w[n-i])
// Given my ansatz of endless stream of x[n-i] = x[n] and hence w[n-i] = w[n], it follows
// that
//       w[n] = x[n] - sum_i(a[i] * w[n]) = x[n] - sum_i(a[i]) * w[n]
//  <=>  (1 + sum_i(a[i])) w[n] = x[n]
//  <=>  w[n] = 1 / (1 + sum_i(a[i])) * x[n]
// Looks simple. Just a scale value.
#define INIT_SCALE(name, type) \
static type name(unsigned int order, type *filter_a) \
{ \
	type asum = 1.; \
	if(filter_a) \
		for(unsigned int i=0; i<order; ++i) \
			asum += filter_a[i]; \
	return (asum > 1e-12 || asum < -1e-12) ? (type)1./asum : 0; \
}

INIT_SCALE(d_init_scale, double)
INIT_SCALE(f_init_scale, float)

// Static ring buffer index used for filter history and coefficients.
// i: offset
// n1: current position of the first entry
// s: size of the ring buffer
#define RING_INDEX(i, n1, s) ( ((n1)+(i)) % s )
// In the context of a filter: stored offset n1, configured order.
#define F_RING_INDEX(f, i) RING_INDEX(i, (f).n1, (f).order)

int attribute_align_arg
syn123_setup_filter( syn123_handle *sh
,	int append, unsigned int order, double *b, double *a
,	int mixenc, int channels, int init_firstval )
{
	if(!sh)
		return SYN123_BAD_HANDLE;
	if(!append)
	{
		syn123_drop_filter(sh, sh->fc.count);
		if(sh->fc.count)
			return SYN123_WEIRD;
	}
	if(sh->fc.count)
	{
		if(!mixenc)
			mixenc = sh->fc.mixenc;
		if(!channels)
			channels = sh->fc.channels;
	}
	if(channels < 1)
		return SYN123_BAD_FMT;
	if(mixenc != MPG123_ENC_FLOAT_32 && mixenc != MPG123_ENC_FLOAT_64)
		return SYN123_BAD_ENC;
	if(!b)
		return SYN123_NO_DATA;
	if(a && a[0] != 1.)
		return SYN123_BAD_DATA;
	if(ORDER_TOO_BIG(order, channels) || sh->fc.count == SIZE_MAX)
		return SYN123_OVERFLOW;

	if(sh->fc.maxcount == sh->fc.count)
	{
		#define EXPAND_FILTERS(list, stype) \
		{ \
			struct stype *tmp = realloc( list \
			,	sizeof(struct stype)*(sh->fc.maxcount+1) ); \
			if(!tmp) \
				return SYN123_DOOM; \
			list = tmp; \
		}
		if(mixenc == MPG123_ENC_FLOAT_32)
			EXPAND_FILTERS(sh->fc.ff, f_filter)
		else
			EXPAND_FILTERS(sh->fc.df, d_filter)
		// Storage for filter structs increased, nobody can take that away now.
		#undef EXPAND_FILTERS
		++sh->fc.maxcount;
	}
	// Got enough struct storage, try to fill the filter.
	#define MAKE_FILTER(list, stype, scaler, type) \
	{ \
		struct stype *tmp = list+sh->fc.count; \
		tmp->flow = 0; \
		tmp->order = order; \
		tmp->b0 = b[0]; \
		tmp->data = malloc(sizeof(type)*F_DATABLOCK(*tmp, channels)); \
		if(!tmp->data) \
			return SYN123_DOOM; \
		tmp->b = Fb(*tmp); \
		tmp->a = a ? Fa(*tmp) : NULL; \
		for(tmp->n1=0; tmp->n1<order; ++tmp->n1) \
		{ \
			type *f_b = Fc(*tmp, tmp->b); \
			type *f_a = Fc(*tmp, tmp->a); \
			for(unsigned int o=0; o<order; ++o) \
			{ \
				unsigned int ri = F_RING_INDEX(*tmp, o); \
				f_b[ri] = b[o+1]; \
				if(f_a) \
					f_a[ri] = a[o+1]; \
			} \
		} \
		tmp->n1 = 0; \
		tmp->init_scale = init_firstval ? scaler(order, tmp->a) : 0.; \
		if(tmp->init_scale == 0.) \
		{ \
			for(int c=0; c<channels; ++c) \
			{ \
				type *w = Fw(*tmp, c); \
				for(unsigned int o=0; o<order; ++o) \
					w[o] = (type)0.; \
			} \
			tmp->flow = 1; \
		} \
	}
	if(mixenc == MPG123_ENC_FLOAT_32)
		MAKE_FILTER(sh->fc.ff, f_filter, f_init_scale, float)
	else
		MAKE_FILTER(sh->fc.df, d_filter, d_init_scale, double)
	#undef MAKE_FILTER
	// Got fresh filter with memory. Count it.
	++sh->fc.count;
	// Finish off things that cannot error out.
	sh->fc.mixenc = mixenc;
	sh->fc.channels = channels;

	return SYN123_OK;
}

int attribute_align_arg
syn123_query_filter( syn123_handle *sh, size_t position
,	size_t *count, unsigned int *order, double *b, double *a
,	int *mixenc, int *channels, int *init_firstval )
{
	if(!sh)
		return SYN123_BAD_HANDLE;
	if( (order || b || a || mixenc || channels || init_firstval)
	&&	position >= sh->fc.count )
		return SYN123_NO_DATA;
	if(count)
		*count = sh->fc.count;
	if(!sh->fc.count)
		return SYN123_OK;
	if(channels)
		*channels = sh->fc.channels;
	if(mixenc)
		*mixenc = sh->fc.mixenc;

	#define FILTER_INFO(filter, type) \
	{ \
		if(order) \
			*order = filter.order; \
		if(init_firstval) \
			*init_firstval = filter.init_scale != 0. ? 1 : 0; \
		if(b || a) \
		{ \
			if(b) \
				b[0] = filter.b0; \
			if(a) \
				a[0] = 1.; \
			for(unsigned int o=0; o<filter.order; ++o) \
			{ \
				if(b) \
					b[1+o] = filter.b[o]; \
				if(a) \
					a[1+o] = filter.a ? filter.a[o] : 0.; \
			} \
		} \
	}
	if(sh->fc.mixenc == MPG123_ENC_FLOAT_32)
		FILTER_INFO(sh->fc.ff[position], float)
	else
		FILTER_INFO(sh->fc.df[position], double)
	#undef FILTER_INFO
	return SYN123_OK;
}

// This just forgets the filter's own storage, not the storage for
// the filter structs. That would need realloc(), which might fail.
void attribute_align_arg
syn123_drop_filter(syn123_handle *sh, size_t count)
{
	if(!sh)
		return;
	if(count > sh->fc.count)
		count = sh->fc.count;
	// Free data of indicated filters.
	for(size_t i=0; i<count; ++i)
		free( sh->fc.mixenc == MPG123_ENC_FLOAT_32
		?	(void*)(sh->fc.ff[--sh->fc.count].data)
		:	(void*)(sh->fc.df[--sh->fc.count].data) );
	// count decreased now, maxcount still there
}

#define APPLY_ONE_FILTER(name, stype, type) \
static void name( struct stype *f \
,	int channels, type *audio, size_t samples ) \
{ \
	if(!samples) \
		return; \
	if(!f->flow) \
	{ \
		for(int c=0; c<channels; ++c) \
		{ \
			type iv = f->init_scale*audio[c]; \
			type *w = Fw(*f, c); \
			for(unsigned int o=0; o<f->order; ++o) \
				w[o] = iv; \
		} \
		f->n1 = 0; \
		f->flow = 1; \
	} \
	/* Leave loop ordering and mono/stereo optimization to the future.*/ \
	/* This is what is most logical for interleaved audio and the n1 logic */ \
	if(f->a) /* IIR, possibly */ \
	{ \
		for(size_t s=0; s<samples; ++s) \
		{ \
			/* Working with the current value of n1. */ \
			type *b = Fc(*f, f->b); \
			type *a = Fc(*f, f->a); \
			unsigned int next_n1 = F_RING_INDEX(*f, f->order-1); \
			for(int c=0; c<channels; ++c) \
			{ \
				type ny = 0; \
				type nw = 0; \
				type *w = Fw(*f, c); \
				for(unsigned int i=0; i<f->order; ++i) \
				{ \
					ny += w[i]*b[i]; \
					nw -= w[i]*a[i]; \
				} \
				nw += audio[c]; \
				ny += f->b0 * nw; \
				w[next_n1] = nw; \
				audio[c] = ny; \
			} \
			f->n1 = next_n1; \
			audio += channels; \
		} \
	} else /* FIR */ \
	{ \
		for(size_t s=0; s<samples; ++s) \
		{ \
			/* Working with the current value of n1. */ \
			type *b = Fc(*f, f->b); \
			unsigned int next_n1 = F_RING_INDEX(*f, f->order-1); \
			for(int c=0; c<channels; ++c) \
			{ \
				type ny = 0; \
				type nw = audio[c]; \
				type *w = Fw(*f, c); \
				for(unsigned int i=0; i<f->order; ++i) \
					ny += w[i]*b[i]; \
				ny += f->b0 * nw; \
				w[next_n1] = nw; \
				audio[c] = ny; \
			} \
			f->n1 = next_n1; \
			audio += channels; \
		} \
	} \
}

APPLY_ONE_FILTER(apply_filter_float, f_filter, float)
APPLY_ONE_FILTER(apply_filter_double, d_filter, double)

// Finally, the filter application.
int attribute_align_arg
syn123_filter(syn123_handle *sh, void* buf, int encoding, size_t samples)
{
	if(!sh)
		return SYN123_BAD_HANDLE;
	if(!sh->fc.count) // No filter means no change, all good.
		return SYN123_OK;
	#define APPLY_FILTERS(list, func) \
		for(unsigned int i=0; i<sh->fc.count; ++i) \
			func(list+i, sh->fc.channels, buf, samples)
	if(encoding == sh->fc.mixenc)
	{
		if(sh->fc.mixenc == MPG123_ENC_FLOAT_32)
			APPLY_FILTERS(sh->fc.ff, apply_filter_float);
		else
			APPLY_FILTERS(sh->fc.df, apply_filter_double);
	} else
	{
		return SYN123_BAD_ENC;
	}
	#undef APPLY_FILTERS
	return SYN123_OK;
}
