/*
	pulse: audio output using PulseAudio server

	copyright 2006-2016 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Nicholas J. Humfrey
*/

#include "../out123_int.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Using simple API for playback, but now also async API for enumeration.
// Maybe we should switch to the latter for playback, too, let alone for
// PA_CONTEXT_NOAUTOSPAWN. Or we hack something only for probing.
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/pulseaudio.h>

#include "../../common/debug.h"

// Since we learned the async API for enumeration, let's abuse it for
// a simple check if a pulse server is active before accidentally
// starting one.

static void state_callback(pa_context *c, void *userdata)
{
	pa_context_state_t state;
	int *pa_ready = userdata;

	state = pa_context_get_state(c);
	switch(state)
	{
		case PA_CONTEXT_FAILED:
		case PA_CONTEXT_TERMINATED:
			*pa_ready = 2;
		break;
		case PA_CONTEXT_READY:
			*pa_ready = 1;
		break;
		default: break; // Make -Wall happy.
	}
}

static int check_for_server()
{
	pa_mainloop *pa_ml;
	pa_mainloop_api *pa_mlapi;
	pa_context *pa_ctx;
	int ret = 0;
	int pa_ready = 0;

	pa_ml = pa_mainloop_new();
	pa_mlapi = pa_mainloop_get_api(pa_ml);
	pa_ctx = pa_context_new(pa_mlapi, "out123 server check");
	if(!pa_context_connect(pa_ctx, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL))
	{
		debug("context connection worked, checking for readiness");
		pa_context_set_state_callback(pa_ctx, state_callback, &pa_ready);
		while(!pa_ready)
		{
			if(pa_ready == 0)
			{
				pa_mainloop_iterate(pa_ml, 1, NULL);
				continue;
			}
			if(pa_ready == 2)
			{
				ret = -1;
				break;
			}
			pa_mainloop_iterate(pa_ml, 1, NULL);
		}
		pa_context_disconnect(pa_ctx);
	} else
	{
		debug("pa_context_connect() failed right away");
		ret = -1;
	}

	pa_context_unref(pa_ctx);
	pa_mainloop_free(pa_ml);

	return ret;
}


static int open_pulse(out123_handle *ao)
{
	int err = 0;
	pa_simple* pas = NULL;
	pa_sample_spec ss;
	/* Check if already open ? */
	if (ao->userptr) {
		if(!AOQUIET)
			error("Pulse audio output is already open.");
		return -1;
	}

	/* Open an audio I/O stream. */
	/* When they are < 0, I shall set some default. */
	if(ao->rate < 0 || ao->format < 0 || ao->channels < 0)
	{
		ao->rate     = 44100;
		ao->channels = 2;
		ao->format   = MPG123_ENC_SIGNED_16;
	}

	/* Fill out pulse audio's data structure */
	ss.channels = ao->channels;
	ss.rate = ao->rate;

	switch(ao->format) {
		case MPG123_ENC_SIGNED_16:
#ifdef WORDS_BIGENDIAN
			ss.format=PA_SAMPLE_S16BE;
#else
			ss.format=PA_SAMPLE_S16LE;
#endif
		break;
		case MPG123_ENC_SIGNED_24:
#ifdef WORDS_BIGENDIAN
			ss.format=PA_SAMPLE_S24BE;
#else
			ss.format=PA_SAMPLE_S24LE;
#endif
		break;
		case MPG123_ENC_SIGNED_32:
#ifdef WORDS_BIGENDIAN
			ss.format=PA_SAMPLE_S32BE;
#else
			ss.format=PA_SAMPLE_S32LE;
#endif
		break;
		case MPG123_ENC_FLOAT_32:
#ifdef WORDS_BIGENDIAN
			ss.format=PA_SAMPLE_FLOAT32BE;
#else
			ss.format=PA_SAMPLE_FLOAT32LE;
#endif
		break;
		case MPG123_ENC_ALAW_8:
			ss.format=PA_SAMPLE_ALAW;
		break;
		case MPG123_ENC_ULAW_8:
			ss.format=PA_SAMPLE_ULAW;
		break;
		case MPG123_ENC_UNSIGNED_8:
			ss.format=PA_SAMPLE_U8;
		break;
		default:
			if(!AOQUIET)
				error1("Unsupported audio format: 0x%x", ao->format);
			return -1;
		break;
	}

	// It used to be a default config that pa_simple_new() just starts
	// an instance of pulseaudio if it is not running. This is undesirable
	// for probing. Libout123 should not change the system config like that.
	// Nowadays, you got socket activation on Linux with systemd instead,
	// anyway, but this little check should not hurt in the grand scheme. Also,
	// we could use it as starting point to convert to the async API if it
	// is deemed useful.
	if(check_for_server())
	{
		if(!AOQUIET)
			error("No PulseAudio running. I will not accidentally trigger starting one.");
		return -1;
	}

	/* Perform the open */
	pas = pa_simple_new(
			NULL,				/* Use the default server */
			ao->name,		/* Our application's name */
			PA_STREAM_PLAYBACK,
			ao->device,			/* Use the default device if NULL */
			"via out123",		/* Description of our stream */
			&ss,				/* Our sample format */
			NULL,				/* Use default channel map */
			NULL,				/* Use default buffering attributes */
			&err				/* Error result code */
	);

	if(pas == NULL)
	{
		if(!AOQUIET)
			error1("Failed to open pulse audio output: %s", pa_strerror(err));
		return -1;
	}

	/* Store the pointer */
	ao->userptr = (void*)pas;
	return 0;
}


static int get_formats_pulse(out123_handle *ao)
{
	return MPG123_ENC_SIGNED_16 | MPG123_ENC_SIGNED_24 |MPG123_ENC_SIGNED_32
	|	MPG123_ENC_FLOAT_32 | MPG123_ENC_ALAW_8 | MPG123_ENC_ULAW_8
	|	MPG123_ENC_UNSIGNED_8;
}


static int write_pulse(out123_handle *ao, unsigned char *buf, int len)
{
	pa_simple *pas = (pa_simple*)ao->userptr;
	int ret, err;
	/* Doesn't return number of bytes but just success or not. */
	ret = pa_simple_write( pas, buf, len, &err );
	if(ret<0)
	{
		if(!AOQUIET)
			error1("Failed to write audio: %s", pa_strerror(err));
		return -1;
	}
	return len; /* If successful, everything has been written. */
}

static int close_pulse(out123_handle *ao)
{
	pa_simple *pas = (pa_simple*)ao->userptr;

	if (pas) {
		int err = 0; /* Do we really want to handle errors here? End is the end. */
		pa_simple_drain(pas, &err);
		pa_simple_free(pas);
		ao->userptr = NULL;
	}
	
	return 0;
}

static void flush_pulse(out123_handle *ao)
{
	pa_simple *pas = (pa_simple*)ao->userptr;
	
	if (pas) {
		int err = 0;
		pa_simple_flush( pas, &err );	
		if(err && !AOQUIET)
			error1("Failed to flush audio: %s", pa_strerror(err));
	}
}

struct enumerate_data
{
	int (*store_device)(void *devlist
	,	const char *name, const char *description);
	void *devlist;
	int ret;
};

// Device enumeration is apparently not so simple. Seems to need the full API.
// I paraphrase the usage out of the example at
//   https://gist.github.com/andrewrk/6470f3786d05999fcb48
// and assume that the code is in public domain or at least generic enough
// since it just shows the public API.

static void sinklist_callback( pa_context *c
,	const pa_sink_info *l, int eol, void *userdata )
{
	struct enumerate_data *ed = userdata;
	if(!eol && !ed->ret)
		ed->ret = ed->store_device(ed->devlist, l->name, l->description);
}

static int enumerate_pulse( out123_handle *ao, int (*store_device)(void *devlist
,	const char *name, const char *description), void *devlist )
{
	pa_mainloop *pa_ml;
	pa_mainloop_api *pa_mlapi;
	pa_operation *pa_op = NULL;
	pa_context *pa_ctx;
	int state = 0;
	int pa_ready = 0;
	struct enumerate_data ed;
	ed.store_device = store_device;
	ed.devlist = devlist;
	ed.ret = 0;

	pa_ml = pa_mainloop_new();
	pa_mlapi = pa_mainloop_get_api(pa_ml);
	pa_ctx = pa_context_new(pa_mlapi, "out123 enumeration");
	if(pa_context_connect(pa_ctx, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL))
	{
		error("Connection to PulseAudio failed right away.");
		ed.ret = -1;
		goto enumerate_end;
	}

	pa_context_set_state_callback(pa_ctx, state_callback, &pa_ready);

	while(state < 2)
	{
		debug("in the loop");
		if(pa_ready == 0)
		{
			pa_mainloop_iterate(pa_ml, 1, NULL);
			continue;
		}
		if(pa_ready == 2)
		{
			if(!AOQUIET)
				error("Querying PulseAudio server failed.");
			ed.ret = -1;
			break;
		}
		switch(state)
		{
			case 0:
				pa_op = pa_context_get_sink_info_list( pa_ctx,
				sinklist_callback, &ed );
				++state;
			break;
			case 1:
				if(pa_operation_get_state(pa_op) == PA_OPERATION_DONE)
					goto enumerate_preend;
			break;
		}
		pa_mainloop_iterate(pa_ml, 1, NULL);
	}

enumerate_preend:
	if(pa_op)
		pa_operation_unref(pa_op);
	pa_context_disconnect(pa_ctx);

enumerate_end:
	pa_context_unref(pa_ctx);
	pa_mainloop_free(pa_ml);

	return ed.ret;
}

static int init_pulse(out123_handle* ao)
{
	if (ao==NULL) return -1;

	/* Set callbacks */
	ao->open = open_pulse;
	ao->flush = flush_pulse;
	ao->write = write_pulse;
	ao->get_formats = get_formats_pulse;
	ao->close = close_pulse;
	ao->enumerate = enumerate_pulse;

	/* Success */
	return 0;
}


/* 
	Module information data structure
*/
mpg123_module_t mpg123_output_module_info = {
	/* api_version */	MPG123_MODULE_API_VERSION,
	/* name */			"pulse",						
	/* description */	"Output audio using PulseAudio Server",
	/* revision */		"$Rev:$",						
	/* handle */		NULL,
	
	/* init_output */	init_pulse,						
};

