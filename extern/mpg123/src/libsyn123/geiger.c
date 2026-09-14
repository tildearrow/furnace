/*
	Geiger-Mueller counter simulator.
	(c)2016-2018 Thomas Orgis <thomas@orgis.org>, licensed MIT-style.
	Essentially: This is just a bit of code that you can use as you please.
	See full license text at the bottom. As part of the mpg123 distribution,
	the GNU LPGL version 2.1 can be used instead.

	This generates random pulses with a configured likelihood (event
	activity) that drive a simulated speaker to emit the characteristic
	ticking sound (as opposed to a perfect digital click).

	This is adapted from a stand-alone demonstration program to be
	directly available to out123 and others via libsyn123.
*/

#include "config.h"

#define FILL_PERIOD
#define RAND_XORSHIFT32
#define NO_SMAX
#define NO_SMIN
#include "syn123_int.h"

static void white_generator(syn123_handle *sh, int samples)
{
	for(int i=0; i<samples; ++i)
		sh->workbuf[1][i] = 2*rand_xorshift32(&sh->seed);
}

int attribute_align_arg
syn123_setup_white(syn123_handle *sh, unsigned long seed, size_t *period)
{
	if(!sh)
		return SYN123_BAD_HANDLE;
	syn123_setup_silence(sh);
	sh->seed = seed;
	sh->generator = white_generator;
	int ret = fill_period(sh);
	sh->seed = seed;
	if(ret != SYN123_OK)
		syn123_setup_silence(sh);
	if(period)
		*period = sh->samples;
	return ret;
}

/*
	Brain dump for the speaker model:

	Simulating a simple speaker as dampened oscillator.
	This should result in somewhat interesting impulse response.
	I do not bother with a lower frequency bound (people may wish to
	apply a highpass filter to simulate a small tinny speaker).

		F = - k x

	There is a mass attached, carrying the momentum. A new Geiger
	impulse just adds a big load of momentum and the oscillator
	works with that. A simple Euler scheme should be enough here.

		dx = v dt
		dv = F/m dt

	We need some friction here. Let's use simple quadratic (air)
	friction with coefficient r, (r v^2) being a force opposing the
	current movement.

		dv = - (k x + sign(v) r v^2) / m dt

	About the impulse from the counter ... it needs to be incorporated
	as an additional force G that is applied over a certain time period.
	I need an impulse shape. How does the cascade look again?
	Does a gauss curve do the trick? Or a jump with linear decay? Or simply
	a box impulse? I'll try with a box impulse, relating to a certain
	energy of a gas discharge. then, there's a dead time interval where
	new particles are not detected at all. After that, a time period where
	new discharges are weaker (linear ramp as simplest model).

	Let's use that model and solve it in the simplest manner:

		dx = v dt
		dv = (G(t) - k x(t) - sign(v(t)) r v(t)^2) / m dt

	Some fixed friction goes on top to bring the speaker really down to
	zero.
*/

struct geigerspace
{
	// Time is counted in sampling intervals of this size.
	// (inverse of sampling rate).
	double time_interval;
	// Strength of the last event, compared to full scale.
	// This is for capturing an event detected in the
	// recovery period.
	double event_strength;
	// Age of last event in intervals. Based on this, the discharge
	// force is applied or not, strength of a new discharge computed
	// (if event_age > dead time).
	// Scaling of discharge force, in relation to speaker movement.
	double force_scale;
	// Intervals since last event, if it is recent.
	long event_age;
	// Number of intervals for that a new event is not detected.
	// This is also the duration of the discharge.
	double dead_s;
	long dead_time;
	// Number of intervals till recovery after dead time.
	long recover_time;
	// Threshold for random value to jump over.
	float thres;

	// Oscillator properties.
	double mass;     // mass (inertia) of speaker in kg
	double spring;   // spring constant k in N/m
	double friction; // friction coefficient in kg/(s m)
	double friction_fixed; // Speed-independent friction.

	// State variables.
	double pos;   // position of speaker [-1:1]
	double speed; // speed of speaker movement
};

static double sign(double val)
{
	return val < 0 ? -1. : +1;
}

// Advance speaker position by one time interval, applying the given force.
static double speaker(struct geigerspace *gs, double force)
{
	double dx, dv;
	// Some maximal numeric time step to work nicely for small sampling
	// rates. Unstable numerics make a sound, too.
	double euler_step = 1e-5;
	long steps = 0;
	do
	{
		double step = gs->time_interval-steps*euler_step;
		if(step > euler_step)
			step = euler_step;
		// dx = v dt
		// dv = (G(t) - k x(t) - sign(v(t)) r v(t)^2) / m dt
		dx = gs->speed*step;
		dv = ( force - gs->spring*gs->pos
		   - sign(gs->speed)*gs->friction*gs->speed*gs->speed )
		   / gs->mass * step;
		gs->pos   += dx;
		gs->speed += dv;
		// Apply some fixed friction to get down to zero.
		if(gs->speed)
		{
			double ff = -sign(gs->speed)*gs->friction_fixed/gs->mass*step;
			if(sign(gs->speed+ff) == sign(gs->speed))
				gs->speed += ff;
			else
				gs->speed /= 2;
		}
	} while(++steps*euler_step < gs->time_interval);
	return gs->pos;
}

// The logic of the Geiger-Mueller counter.
// Given an event (or absence of) for this time interval, return
// the discharge force to apply to the speaker.
static double discharge_force(struct geigerspace *gs, int event)
{
	double strength = 0.;
	if(gs->event_age >= 0) // If there was a recent event.
	{
		// Apply current event force until dead time is reached.
		if(++gs->event_age > gs->dead_time)
		{
			long newtime = gs->event_age - gs->dead_time;
			if(newtime < gs->recover_time)
			{
				gs->event_strength = 1.*newtime/gs->recover_time;
			}
			else // Possible event after full recovery.
			{
				gs->event_age = -1;
				gs->event_strength = 1.;
			}
		}
		else // Still serving the old event.
		{
			strength = gs->event_strength;
			event = 0;
		}
	}
	if(event)
	{
		gs->event_age = 0;
		strength = gs->event_strength;
	}
	return strength*gs->force_scale;
}

static void geiger_init(struct geigerspace *gs, double activity, long rate)
{
	if(activity < 0)
		activity = 0.;
	gs->time_interval = 1./rate;
	gs->event_strength = 1.;
	gs->event_age = -1;
	// In the order of 100 us. Chose a large time to produce sensible results down
	// to 8000 Hz sampling rate.
	gs->dead_s = 0.0002;
	gs->dead_time = (long)(gs->dead_s*rate+0.5);
	gs->recover_time = 2*gs->dead_time;
	// Let's artitrarily define maximum speaker displacement
	// as 1 mm and specify values accordingly.
	gs->pos = 0.;
	gs->speed = 0.;
	// Mass and spring control the self-oscillation frequency.
	gs->mass = 0.02;
	gs->spring = 1000000;
	// Some dynamic friction is necessary to keep the unstable numeric
	// solution in check. A kind of lowpass filter, too, of course.
	gs->friction = 0.02;
	// Some hefty fixed friction prevents self-oscillation of speaker
	// (background sine wave). But you want _some_ of it.
	gs->friction_fixed = 20000;
	// Experimenting, actually. Some relation to the speaker.
	gs->force_scale = 50000.*gs->mass*0.001/(4.*gs->dead_s*gs->dead_s);

	float event_likelihood = (float)(activity*gs->time_interval);
	if(event_likelihood > 1.f)
		event_likelihood = 1.f;
	gs->thres = 1.f-event_likelihood;
}

static void geiger_generator(syn123_handle *sh, int samples)
{
	struct geigerspace *gs = sh->handle;
	for(int i=0; i<samples; ++i)
		sh->workbuf[1][i] = speaker( gs
		,	discharge_force(gs, (rand_xorshift32(&sh->seed)+0.5)>gs->thres) );
	// Soft clipping as speaker property. It can only move so far.
	// Of course this could be produced by a nicely nonlinear force, too.
	syn123_soft_clip( sh->workbuf[1], MPG123_ENC_FLOAT_64, samples
	,	1., 0.1, NULL );
}

int attribute_align_arg
syn123_setup_geiger( syn123_handle *sh, double activity, unsigned long seed
,	size_t *period )
{
	int ret = SYN123_OK;
	if(!sh)
		return SYN123_BAD_HANDLE;
	syn123_setup_silence(sh);
	struct geigerspace *handle = malloc(sizeof(*handle));
	if(!handle)
		return SYN123_DOOM;
	sh->seed = seed;
	geiger_init(handle, activity, sh->fmt.rate);
	sh->handle = handle;
	sh->generator = geiger_generator;
	// Fill period buffer, re-init generator for cleanliness.
	ret = fill_period(sh);
	if(ret)
		goto setup_geiger_end;
	if(sh->samples)
	{
		sh->seed = seed;
		geiger_init(handle, activity, sh->fmt.rate);
	}

setup_geiger_end:
	if(ret != SYN123_OK)
		syn123_setup_silence(sh);
	if(period)
		*period = sh->samples;
	return ret;
}

// Full license text:
// Copyright (c) 2016-2018 Thomas Orgis
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
