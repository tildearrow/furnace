/*
	libsyn123: libsyn123 entry code and wave generators

	copyright 2017-2023 by the mpg123 project
	licensed under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org

	initially written by Thomas Orgis

	This code directly contains wave generators and the generic entry
	code for libsyn123. The waves started the whole thing and stay here
	for now. Other signal generators go into separate files.

	This code is testament to the ultimate programmer hybris: Just add
	something because it looks easy, a fun dirty hack. Then spend days
	doing it properly, including fun like squeezing multiple waves of
	differing shapes into periodic packets. Oh, and caring about
	vectorization for the heck of it. Offering sample formats down to
	8 bit alaw. Madness.
*/

#include "syn123_int.h"
#include "../version.h"
#include "../common/sample.h"
#include "../common/debug.h"

const char * attribute_align_arg syn123_distversion(unsigned int *major, unsigned int *minor, unsigned int *patch)
{
	if(major)
		*major = MPG123_MAJOR;
	if(minor)
		*minor = MPG123_MINOR;
	if(patch)
		*patch = MPG123_PATCH;
	return MPG123_VERSION;
}

unsigned int attribute_align_arg syn123_libversion(unsigned int *patch)
{
	if(patch)
		*patch = SYN123_PATCHLEVEL;
	return SYN123_API_VERSION;
}

static const double freq_error = 1e-4;
/* For our precisions, that value will always be good enough. */
static const double twopi = 2.0*3.14159265358979323846;
static struct syn123_wave defwave = { SYN123_WAVE_SINE, FALSE, 440., 0. };

/* round floating point to size_t */
static size_t round2size(double a)
{
	return a < 0 ? 0 : (size_t)(a+0.5);
}

/* fractional part, relating to frequencies (so long matches) */
static double myfrac(double a)
{
	return a-(long)a;
}

/* arbitrary phase -> [0..1) */
static double phasefrac(double p)
{
	return p-floor(p);
}

/*
	Given a set of wave frequencies, compute an approximate common
	period for the combined signal. Invalid frequencies are set to
	the error bound for some sanity.
	TODO: sort waves and begin with highest freq for stable/best result
*/
static double common_samples_per_period( long rate, size_t count
,	struct syn123_wave *waves, size_t size_limit )
{
	double spp = 0;
	size_t i;
	for(i=0; i<count; ++i)
	{
		double sppi;
		size_t periods = 1;
		/* Limiting sensible frequency range. */
		if(waves[i].freq < freq_error)
			waves[i].freq = freq_error;
		if(waves[i].freq > rate/2)
			waves[i].freq = rate/2;
		sppi = fabs((double)rate/waves[i].freq);
		debug2("freq=%g sppi=%g", waves[i].freq, sppi);
		if(spp == 0)
			spp = sppi;
		while
		(
			(periods+1)*spp <= size_limit &&
			fabs( myfrac(periods*spp / sppi) ) > freq_error
		)
			periods++;
		spp*=periods;
		debug3( "samples_per_period + %f Hz = %g (%zu periods)"
		,	waves[i].freq, spp, periods );
	}
	return spp;
}

/* Compute a good size of a table covering the common period for all waves. */
static size_t tablesize( long rate, size_t count
,	struct syn123_wave *waves, size_t size_limit )
{
	size_t ts, nts;
	double fts, tolerance;
	double samples_per_period;
	size_t periods;

	samples_per_period = common_samples_per_period( rate, count
	,	waves, size_limit );
	tolerance = freq_error*samples_per_period;

	periods = 0;
	do
	{
		periods++;
		fts = periods*samples_per_period;
		ts  = round2size(fts);
		nts = round2size((periods+1)*samples_per_period);
	}
	while(fabs(fts-ts) > periods*tolerance && nts <= size_limit);

	/* Ensure size limit. Even it is ridiculous. */
	ts = smin(ts, size_limit);
	debug1("table size: %zu", ts);
	return ts;
}

/* The wave functions. Argument is the phase normalised to the period. */
/* The argument is guaranteed to be 0 <= p < 1. */

/* _________ */
/*           */
static double wave_none(double p)
{
	return 0;
}

/*   __       */
/*  /  \      */
/*      \__/  */
static double wave_sine(double p)
{
	return sin(twopi*p);
}

/*      ___   */
/*  ___|      */
static double wave_square(double p)
{
	return (p < 0.5 ? -1 : 1);
}

/*    1234    Avoid jump from zero at beginning. */
/*    /\      */
/*      \/    */
static double wave_triangle(double p)
{
	return 4*p < 1
	?	4*p        /* 1 */
	:	( 4*p < 3
		?	2.-4*p  /* 2 and 3 */
		:	-4+4*p  /* 4 */
		);
}

/*   /|    Avoid jump from zero ... */
/*    |/   */
static double wave_sawtooth(double p)
{
	return 2*p < 1 ? 2*p : -2+2*p;
}

/*    _    */
/* __/ \__ */
/*         */
static double wave_gauss(double p)
{
	double v = p-0.5;
	return exp(-30*v*v);
}

/*    _      */
/*  _/ -___  */
/*           */
/* p**2*exp(-a*p**2) */
/* Scaling: maximum at sqrt(1/a), value 1/a*exp(-1). */
static double wave_pulse(double p)
{
	return p*p*exp(-50*p*p)/0.00735758882342885;
}

/*  _     */
/* / -___ */
/*        */
/* p**2*exp(-a*p) */
/* Scaling: maximum at 4/a, value 4/a**2*exp(-2). */
static double wave_shot(double p)
{
	return p*p*exp(-100*p)/5.41341132946451e-05;
}

// Actual wave worker function, to be used to give up to
// bufblock samples in one go. This takes a vector of phases
// and multiplies the resulting amplitudes into the output buffer.
static void evaluate_wave( double outbuf[bufblock], size_t samples
,	enum syn123_wave_id id, double phase[bufblock] )
{
	// Ensuring that the inner loop is inside the switch.
	// Compilers might be smart enough, but it is not hard
	// to write it down the right way from the beginning.
	#define PHASE phase[pi]
	#define PI_LOOP( code ) \
		for(size_t pi=0; pi<samples; ++pi) \
			outbuf[pi] *= code;
	switch(id)
	{
		case SYN123_WAVE_FLAT:
			PI_LOOP( wave_none(PHASE) )
		break;
		case SYN123_WAVE_SINE:
			PI_LOOP( wave_sine(PHASE) )
		break;
		case SYN123_WAVE_SQUARE:
			PI_LOOP( wave_square(PHASE) )
		break;
		case SYN123_WAVE_TRIANGLE:
			PI_LOOP( wave_triangle(PHASE) )
		break;
		case SYN123_WAVE_SAWTOOTH:
			PI_LOOP( wave_sawtooth(PHASE) )
		break;
		case SYN123_WAVE_GAUSS:
			PI_LOOP( wave_gauss(PHASE) )
		break;
		case SYN123_WAVE_PULSE:
			PI_LOOP( wave_pulse(PHASE) )
		break;
		case SYN123_WAVE_SHOT:
			PI_LOOP( wave_shot(PHASE) )
		break;
		default:
			PI_LOOP( wave_none(PHASE) )
	}
	#undef PI_LOOP
	#undef PHASE
}

static const char* wave_names[] =
{
	"flat", "sine", "square", "triangle"
,	"sawtooth", "gauss", "pulse", "shot"
};

const char* attribute_align_arg
syn123_wave_name(int id)
{
	if(id < 0 || id >= sizeof(wave_names)/sizeof(char*))
		return "???";
	else
		return wave_names[id];
}

int attribute_align_arg
syn123_wave_id(const char *name)
{
	if(name)
		for(int i=0; i<sizeof(wave_names)/sizeof(char*); ++i)
			if(!strcmp(name, wave_names[i]))
				return i;
	return SYN123_WAVE_INVALID;
}

const char* syn123_strerror(int errcode)
{
	switch(errcode)
	{
		case SYN123_OK:
			return "no error";
		case SYN123_BAD_HANDLE:
			return "bad handle";
		case SYN123_BAD_FMT:
			return "bad format";
		case SYN123_BAD_ENC:
			return "bad encoding";
		case SYN123_BAD_CONV:
			return "unsupported conversion";
		case SYN123_BAD_SIZE:
			return "wrong buffer size";
		case SYN123_BAD_BUF:
			return "bad buffer pointer";
		case SYN123_BAD_CHOP:
			return "byte count not matching sample boundaries";
		case SYN123_DOOM:
			return "out of memory";
		case SYN123_WEIRD:
			return "Call the Ghostbusters!";
		case SYN123_BAD_FREQ:
			return "Bad signal frequency given.";
		case SYN123_BAD_SWEEP:
			return "Invalid sweep curve given.";
		case SYN123_OVERFLOW:
			return "An integer overflow occured.";
		case SYN123_NO_DATA:
			return "Not enough data.";
		case SYN123_BAD_DATA:
			return "Bad data given.";
		default:
			return "unkown error";
	}
}

// I first introduced the vectorization-friendly aproach with a precomputed
// buffer of phases as a (premature) optimization, but it turns out that
// this separation is really useful for computing frequency sweeps.
// The computation of phases, including varying frequency, really is a separate
// task from actually evaluating the wave functions.
// The code is actually smaller and better abstracted that way.

static void add_some_wave( double outbuf[bufblock], size_t samples
,	enum syn123_wave_id id, double pps, double phase
,	double workbuf[bufblock] )
{
	for(size_t pi=0; pi<samples; ++pi)
		workbuf[pi] = phasefrac(pi*pps+phase);
	evaluate_wave(outbuf, samples, id, workbuf);
}

// Fit waves into given table size.
static void wave_fit_table( size_t samples
, long rate, struct syn123_wave *wave )
{
	double pps = wave->freq/rate;
	debug3("wave_fit_table %zu %ld %g", samples, rate, wave->freq);
	size_t periods = smax(round2size(pps*samples), 1);
	pps = (double)periods/samples;
	wave->freq = pps*rate;
	debug4( "final wave: %c %i @ %g Hz + %g"
	,	wave->backwards ? '<' : '>', wave->id, wave->freq, wave->phase );
}

// Evaluate an additional wave into the given buffer (multiplying
// with existing values in output.
static void wave_add_buffer( double outbuf[bufblock], size_t samples
,	long rate, struct syn123_wave *wave, double workbuf[bufblock] )
{
	double pps = wave->freq/rate;
	debug3("wave_add_buffer %zu %ld %g", samples, rate, wave->freq);
	debug4( "adding wave: %c %i @ %g Hz + %g"
	,	wave->backwards ? '<' : '>', wave->id, wave->freq, wave->phase );
	if(wave->backwards)
		pps = -pps;
	add_some_wave( outbuf, samples
	,	wave->id, pps, wave->phase, workbuf );
	// Advance the wave.	
	wave->phase = phasefrac(wave->phase+samples*pps);
}

// The most basic generator of all.
static void silence_generator(syn123_handle *sh, int samples)
{
	for(int i=0; i<samples; ++i)
		sh->workbuf[1][i] = 0;
}

// Clear the handle of generator data structures.
// Well, except the one generating silence.
int attribute_align_arg
syn123_setup_silence(syn123_handle *sh)
{
	if(!sh)
		return SYN123_BAD_HANDLE;
	sh->generator = silence_generator;
	if(sh->wave_count && sh->waves)
		free(sh->waves);
	sh->waves = NULL;
	sh->wave_count = 0;
	if(sh->handle)
		free(sh->handle);
	sh->handle = NULL;
	sh->samples = 0;
	sh->offset = 0;
	return SYN123_OK;
}

// The generator function is called upon to fill sh->workbuf[1] with the
// given number of samples in double precision. It may use sh->workbuf[0]
// for its own purposes.
static void wave_generator(syn123_handle *sh, int samples)
{
	/* Initialise to zero amplitude. */
	for(int i=0; i<samples; ++i)
		sh->workbuf[1][i] = 1;
	/* Add individual waves. */
	for(size_t c=0; c<sh->wave_count; ++c)
		wave_add_buffer( sh->workbuf[1], samples, sh->fmt.rate, sh->waves+c
		,	sh->workbuf[0] );
}

/* Build internal table, allocate external table, convert to that one, */
/* adjusting sample storage format and channel count. */
int attribute_align_arg
syn123_setup_waves( syn123_handle *sh, size_t count
,	int *id, double *freq, double *phase, int *backwards
,	size_t *common_period )
{
	int ret = SYN123_OK;

	if(!sh)
		return SYN123_BAD_HANDLE;
	syn123_setup_silence(sh);

	if(!count)
	{
		count = 1;
		id = NULL;
		freq = NULL;
		phase = NULL;
		backwards = NULL;
	}

	sh->waves = malloc(sizeof(struct syn123_wave)*count);
	if(!sh->waves)
		return SYN123_DOOM;
	for(size_t c=0; c<count; ++c)
	{
		sh->waves[c].id = id ? id[c] : defwave.id;
		sh->waves[c].backwards = backwards ? backwards[c] : defwave.backwards;
		sh->waves[c].freq = freq ? freq[c] : defwave.freq;
		sh->waves[c].phase = phase ? phase[c] : defwave.phase;
	}
	sh->wave_count = count;
	sh->generator = wave_generator;

	if(sh->maxbuf)
	{
		// 1. Determine buffer size to use.
		size_t samplesize = MPG123_SAMPLESIZE(sh->fmt.encoding);
		size_t size_limit = sh->maxbuf / samplesize;
		size_t buffer_samples = tablesize(sh->fmt.rate, count, sh->waves
		,	size_limit);
		// 2. Actually allocate the buffer.
		grow_buf(sh, buffer_samples*samplesize);
		if(sh->bufs/samplesize < buffer_samples)
		{
			ret = SYN123_DOOM;
			goto setup_wave_end;
		}
		// 2. Adjust the waves to fit into the buffer.
		for(size_t c=0; c<count; ++c)
		{
			wave_fit_table(buffer_samples, sh->fmt.rate, sh->waves+c);
			if(freq)
				freq[c] = sh->waves[c].freq;
		}
		// 3. fill the buffer using workbuf as intermediate. As long as
		// the buffer is not ready, we can just use syn123_read() to fill it.
		// Just need to ensure mono storage. Once sh->samples is set, the
		// buffer is used.
		int outchannels = sh->fmt.channels;
		sh->fmt.channels = 1;
		size_t buffer_bytes = syn123_read(sh, sh->buf, buffer_samples*samplesize);
		sh->fmt.channels = outchannels;
		// 4. Restore wave phases to the beginning, for tidyness.
		for(size_t c=0; c<count; ++c)
			sh->waves[c].phase = phase ? phase[c] : defwave.phase;
		// 5. Last check for sanity.
		if(buffer_bytes != buffer_samples*samplesize)
		{
			ret = SYN123_WEIRD;
			goto setup_wave_end;
		}
		sh->samples = buffer_samples;
	}

setup_wave_end:
	if(ret != SYN123_OK)
		syn123_setup_silence(sh);
	else
	{
		if(common_period)
			*common_period = sh->samples;
	}
	return ret;
}

int attribute_align_arg
syn123_query_waves( syn123_handle *sh, size_t *count
,	int *id, double *freq, double *phase, int *backwards
,	size_t *common_period )
{
	if(!sh)
		return SYN123_BAD_HANDLE;
	if(count)
		*count = sh->wave_count;
	if((id || freq || phase || backwards || common_period) && !sh->waves)
		return SYN123_NO_DATA;
	for(size_t c=0; c<sh->wave_count; ++c)
	{
		if(id)
			id[c] = sh->waves[c].id;
		if(backwards)
			backwards[c] = sh->waves[c].backwards;
		if(freq)
			freq[c] = sh->waves[c].freq;
		if(phase)
			phase[c] = sh->waves[c].phase;
	}
	if(common_period)
		*common_period = sh->samples;
	return SYN123_OK;
}

// Given time normalized to the sweep duration, return the
// current frequency.

// With the power of analytical integration, we can compute
// the proper phase instead of error-prone summing.
// Given the normalized time: t = i/d
// Wallclock time: w = i/r   (100 samples with rate 100 Hz = 1 s time)
// Ratio:  w = i/r = t*d/r
//        dw = d/r dt
// dp = f(t) * dw = d/r f(t) dt
//  p = (d-1)/r int_0^t f(t) dt = d/r (F(t) - F(0))
// linear: f(t) = f1 + t*(f2-f1); F = f1*t + t**2 * (f2-f1)/2
// quad:   f(t) = f1 + t**2*(f2-f1); F = f1*t + t**3 * (f2-f1)/3
// exp:    f(t) = exp(log(f1) + t*(log(f2)-log(f1)))
//         F(t) = 1/(log(f2)-log(f1)) exp(log(f1) + t*(log(f2)-log(f1)))

// These are the unscaled expressions, just F(t)-F(0).

static double sweep_phase_lin(double t, double f1, double f2)
{
	return f1*t + t*t*0.5*(f2-f1);
}

static double sweep_phase_quad(double t, double f1, double f2)
{
	return f1*t+t*t*t*(1./3)*(f2-f1);
}

// Be sure to only call if f2-f1 has some minimal value.
static double sweep_phase_exp(double t, double logf1, double logf2)
{
	return 1./(logf2-logf1) * (exp(logf1 + t*(logf2-logf1)) - exp(logf1));
}

// Return phases after given offset of samples from now, including
// phase recovery.
// This is the central logic for periodic sweeps.
static void sweep_phase( syn123_handle *sh, size_t off
,	double buf[bufblock], int count )
{
	struct syn123_sweep *sw = sh->handle;
	// Working on a local offset, not touching sw->i.
	mdebug("computing position: (%zu + %zu) %% (%zu + %zu)", sw->i, off, sw->d, sw->post);
	size_t pos = (sw->i + off) % (sw->d + sw->post);
	int boff = 0;
	while(count)
	{
		// Finish the current sweep+post cycle.
		// 0 <= sw->i < sw->d+sw->post
		// 1. Fill phases for actual sweep, if some of it is left.
		//    This includes the endpoint, one sample after configured length,
		//    if so many samples are desired. Note that pos == sw->d will not
		//    happen unless sw->post > 0.
		int sweep_s = pos <= sw->d
		?	(int)smin(sw->d+1 - pos, count)
		:	0;
		// normalized time here to reduce code redundancy
		for(int i=0; i<sweep_s; ++i)
			buf[boff+i] = (double)pos++/sw->d;
		// actual phase computation, with inner loops for auto-vectorization
		switch(sw->id)
		{
			case SYN123_SWEEP_LIN:
				for(int i=0; i<sweep_s; ++i)
					buf[boff+i] = sweep_phase_lin(buf[boff+i], sw->f1, sw->f2);
			break;
			case SYN123_SWEEP_QUAD:
				for(int i=0; i<sweep_s; ++i)
					buf[boff+i] = sweep_phase_quad(buf[boff+i], sw->f1, sw->f2);
			break;
			case SYN123_SWEEP_EXP:
				for(int i=0; i<sweep_s; ++i)
					buf[boff+i] = sweep_phase_exp(buf[boff+i], sw->f1, sw->f2);
			break;
			default:
				for(int i=0; i<sweep_s; ++i)
					buf[boff+i] = 0;
		}
		// scaling from normalized time to sampled proper time
		for(int i=0; i<sweep_s; ++i)
			buf[boff++] *= (double)sw->d/sh->fmt.rate;
		count -= sweep_s;
		// 2. Fill phases for post sweep phase, if reached.
		if(pos > sw->d)
		{
			int post_s = (int)smin(sw->d + sw->post - pos, count);
			double pps = sw->wave.freq/sh->fmt.rate;
			for(int i=0; i<post_s; ++i)
				buf[boff++] = sw->endphase - sw->wave.phase + (pos++ - sw->d)*pps;
			count -= post_s;
		}
		// Now we can possibly wrap around. Actually, we have to
		// hit the boundary exactly.
		if(pos == sw->d + sw->post)
			pos = 0;
	}
	// Turn all phases into fractions and invert if going backwards.
	if(sw->wave.backwards)
		for(int i=0; i<boff; ++i)
			buf[i] = phasefrac(-buf[i]-sw->wave.phase);
	else
		for(int i=0; i<boff; ++i)
			buf[i] = phasefrac(buf[i]+sw->wave.phase);
}

static void sweep_generator(syn123_handle *sh, int samples)
{
	struct syn123_sweep *sw = sh->handle;
	// Precompute phases into work buffer.
	sweep_phase(sh, 0, sh->workbuf[0], samples);
	// Initialise output to zero amplitude and multiply by the wave.
	for(int i=0; i<samples; ++i)
		sh->workbuf[1][i] = 1.;
	evaluate_wave(sh->workbuf[1], samples, sw->wave.id, sh->workbuf[0]);
	// Advance.
	sw->i = (sw->i+samples) % (sw->d + sw->post);
}

int attribute_align_arg
syn123_setup_sweep( syn123_handle* sh
,	int wave_id, double phase, int backwards
,	int sweep_id, double *f1, double *f2, int smooth, size_t duration
,	double *endphase, size_t *period, size_t *buffer_period )
{
	double real_f1;
	int ret = SYN123_OK;
	struct syn123_sweep *sw;
	if(!sh)
		return SYN123_BAD_HANDLE;
	syn123_setup_silence(sh);
	if(!duration) // Empty sweep is silence.
	{
		if(endphase)
			*endphase = phase;
		if(period)
			*period = 0;
		return SYN123_OK;
	}
	if( sweep_id != SYN123_SWEEP_LIN && sweep_id != SYN123_SWEEP_QUAD &&
		sweep_id != SYN123_SWEEP_EXP )
		return SYN123_BAD_SWEEP;

	sh->handle = sw = malloc(sizeof(struct syn123_sweep));
	if(!sh->handle)
	{
		ret = SYN123_DOOM;
		goto setup_sweep_end;
	}
	sw->f1 = (!f1 || *f1 <= 0.) ? defwave.freq : *f1;
	real_f1 = sw->f1;
	sw->f2 = (!f2 || *f2 <= 0.) ? defwave.freq : *f2;
	if( sw->f1 < freq_error || sw->f2 < freq_error )
	{
		ret = SYN123_BAD_FREQ;
		goto setup_sweep_end;
	}
	// Also, don't try exp sweep for very small difference.
	// We need 1/log(f2/f1) ...
	if(sweep_id == SYN123_SWEEP_EXP && (fabs(sw->f2 - sw->f1) < freq_error))
	{
		ret = SYN123_BAD_FREQ;
		goto setup_sweep_end;
	}

	sw->wave.id = wave_id;
	sw->wave.backwards = 0; // Set to true value later.
	sw->wave.freq = sw->f2; // We'll use that later for continuation.
	sw->wave.phase = phase; // Beginning phase offset, not updated.
	sw->id = sweep_id;
	// Store the logarithms for exponential sweep.
	if(sweep_id == SYN123_SWEEP_EXP)
	{
		sw->f1 = log(sw->f1);
		sw->f2 = log(sw->f2);
	}
	sw->i = 0;
	sw->d = duration;
	sw->post = 1; // Needed to be set for end phases.
	if(sw->d + sw->post < sw->d || sw->d + sw->post < sw->post)
	{
		ret = SYN123_OVERFLOW;
		goto setup_sweep_end;
	}
	// The last phase served by the sweep, and the one after that that
	// tryly concludes the sweep (f==f2, not infinitesimally smaller).
	mdebug("computing endphase, with 2 points from %zu - 1 on", duration);
	sweep_phase(sh, duration-1, sh->workbuf[0], 2);
	double before_endphase = sh->workbuf[0][0];
	sw->endphase = sh->workbuf[0][1];
	sw->post = 0; // Reset that again, only increasing if really needed.
	// The phase that would smoothly continue the sweep, one sample
	// after the last one.
	// We want confirmed zero crossing between before_endphase and
	// endphase. This only matters for resolved frequencies, though.
	if(smooth && 2*sw->f2 < sh->fmt.rate &&
		phasefrac(sw->endphase-phase) > phasefrac(before_endphase-phase) )
	{
		// Start from before_endphase on, to reach the actual point where
		// the signal truly reaches f2. Anything on top is with constant
		// f2.
		double poststeps = phasefrac(1.-before_endphase+phase)
		*	((double)sh->fmt.rate/sw->wave.freq);
		// We want the integer part, except in the unlikely
		// case that it is an exact integer: Then we want one less. The
		// actual zero crossing is _after_ the post samples.
		size_t prepos = (size_t)poststeps;
		if(prepos && prepos == poststeps)
			prepos--;
		sw->post = prepos;
	}
	size_t fulldur = sw->d + sw->post;
	if(fulldur < sw->d || fulldur < sw->post)
	{
		ret = SYN123_OVERFLOW;
		goto setup_sweep_end;
	}
	// Now we can go backwards. Otherwise the above computation would
	// be more confusing.
	sw->wave.backwards = backwards;

	sh->generator = sweep_generator;

	size_t samplesize = MPG123_SAMPLESIZE(sh->fmt.encoding);
	if(sh->maxbuf && sh->maxbuf >= samplesize*fulldur)
	{
		grow_buf(sh, samplesize*fulldur);
		if(sh->bufs/samplesize < fulldur)
		{
			ret = SYN123_DOOM; // Actually, also overflow.
			goto setup_sweep_end;
		}
		// Fill the buffer using workbuf as intermediate. As long as
		// the buffer is not ready, we can just use syn123_read() to fill it.
		// Just need to ensure mono storage. Once sh->samples is set, the
		// buffer is used.
		int outchannels = sh->fmt.channels;
		sh->fmt.channels = 1;
		size_t buffer_bytes = syn123_read(sh, sh->buf, fulldur*samplesize);
		sh->fmt.channels = outchannels;
		// Restore sweep state.
		sw->i = 0;
		// Last check for sanity.
		if(buffer_bytes != fulldur*samplesize)
		{
			ret = SYN123_WEIRD;
			goto setup_sweep_end;
		}
		sh->samples = fulldur;
	}

setup_sweep_end:
	if(ret != SYN123_OK)
		syn123_setup_silence(sh);
	else
	{
		mdebug("done with sweep setup %g %g", real_f1, sw->wave.freq);
		if(period)
			*period = fulldur;
		if(endphase)
			*endphase = sw->endphase;
		if(f1)
			*f1 = real_f1;
		if(f2)
			*f2 = sw->wave.freq;
		if(buffer_period)
			*buffer_period = sh->samples;
	}
	return ret;
}

syn123_handle* attribute_align_arg
syn123_new(long rate, int channels, int encoding
,	size_t maxbuf, int *err)
{
	int myerr = SYN123_OK;
	syn123_handle *sh = NULL;
	size_t sbytes = MPG123_SAMPLESIZE(encoding);

	if(!sbytes)
	{
		myerr = SYN123_BAD_ENC;
		goto syn123_new_end;
	}
	if(rate < 1 || channels < 1)
	{
		myerr = SYN123_BAD_FMT;
		goto syn123_new_end;
	}

	sh = malloc(sizeof(syn123_handle));
	if(!sh){ myerr = SYN123_DOOM; goto syn123_new_end; }

	sh->fmt.rate = rate;
	sh->fmt.channels = channels;
	sh->fmt.encoding = encoding;
	sh->buf     = NULL;
	sh->bufs    = 0;
	sh->maxbuf  = maxbuf;
	sh->samples = 0;
	sh->offset  = 0;
	sh->wave_count = 0;
	sh->waves = NULL;
	sh->handle = NULL;
	syn123_setup_silence(sh);
	sh->rd = NULL;
	sh->dither = 0;
	sh->do_dither = 0;
	sh->dither_seed = 0;
	sh->fc.count = sh->fc.maxcount = 0;
	sh->fc.df = NULL;
	sh->fc.ff = NULL;

syn123_new_end:
	if(err)
		*err = myerr;
	if(myerr)
	{
		syn123_del(sh);
		sh = NULL;
	}
	return sh;
}

void attribute_align_arg
syn123_del(syn123_handle* sh)
{
	if(!sh)
		return;
	syn123_setup_silence(sh);
	syn123_setup_resample(sh, 0, 0, 0, 0, 0);
	syn123_drop_filter(sh, sh->fc.count);
	if(sh->fc.ff)
		free(sh->fc.ff);
	if(sh->fc.df)
		free(sh->fc.df);
	if(sh->buf)
		free(sh->buf);
	free(sh);
}

// Copy from period buffer or generate on the fly.
size_t attribute_align_arg
syn123_read( syn123_handle *sh, void *dest, size_t dest_bytes )
{
	char *cdest = dest; /* Want to do arithmetic. */
	size_t samplesize, framesize;
	size_t dest_samples;
	size_t extracted = 0;

	if(!sh)
		return 0;
	samplesize = MPG123_SAMPLESIZE(sh->fmt.encoding);
	framesize  = samplesize*sh->fmt.channels;
	dest_samples = dest_bytes/framesize;
	if(sh->samples) // Got buffered samples to work with.
	{
		while(dest_samples)
		{
			size_t block = smin(dest_samples, sh->samples - sh->offset);
			debug3("offset: %zu block: %zu out of %zu", sh->offset, block, sh->samples);
			syn123_mono2many(cdest, (char*)sh->buf+sh->offset*samplesize
			,	sh->fmt.channels, samplesize, block );
			cdest  += framesize*block;
			sh->offset += block;
			sh->offset %= sh->samples;
			dest_samples -= block;
			extracted    += block;
		}
	}
	else // Compute directly, employing the work buffers.
	{
		while(dest_samples)
		{
			int block = (int)smin(dest_samples, bufblock);
			debug2( "out offset: %ld block: %i"
			,	(long)(cdest-(char*)dest)/framesize, block );
			// Compute data into workbuf[1], possibly using workbuf[0]
			// in the process.
			// TODO for the future: Compute only in single precision if
			// it is enough.
			sh->generator(sh, block);
			// Convert to external format, mono. We are abusing workbuf[0] here,
			// because it is big enough.
			// The converter does not use workbuf if converting from float. Dither is
			// added on the fly.
			int err = syn123_conv(
				sh->workbuf[0], sh->fmt.encoding, sizeof(sh->workbuf[0])
			,	sh->workbuf[1], MPG123_ENC_FLOAT_64, sizeof(double)*block
			,	NULL, NULL, NULL );
			if(err)
			{
				debug1("conv error: %i", err);
				break;
			}
			syn123_mono2many( cdest, sh->workbuf[0]
			,	sh->fmt.channels, samplesize, block );
			cdest += framesize*block;
			dest_samples -= block;
			extracted += block;
		}
	}
	debug1("extracted: %zu", extracted);
	return extracted*framesize;
}

int attribute_align_arg
syn123_dither(syn123_handle *sh, int dither, unsigned long *seed)
{
	if(!sh)
		return SYN123_BAD_HANDLE;
	// So far we only know 1 or 0 as choices.
	sh->dither = dither ? 1 : 0;
	sh->dither_seed = (seed && *seed) ? *seed : 2463534242UL;
	if(seed)
		*seed = sh->dither_seed;
	return SYN123_OK;
}
