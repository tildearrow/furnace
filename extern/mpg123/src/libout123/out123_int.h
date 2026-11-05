/*
	out123_int: internal header for libout123

	copyright ?-2023 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Michael Hipp (some traces left)
*/

#ifndef _MPG123_OUT123_INT_H_
#define _MPG123_OUT123_INT_H_

#define MPG123_ENUM_API

#include "config.h"
#include "../common/abi_align.h"
#include "../compat/compat.h"
#include "out123.h"
#include "module.h"

#ifndef NOXFERMEM
#include "xfermem.h"
#endif

/* 3% rate tolerance */
#define AUDIO_RATE_TOLERANCE	  3

/* Keep those internally? To the outside, it's just a selection of
   driver modules. */
enum {
	DECODE_TEST,  /* "test" */
	DECODE_AUDIO, /* gone */
	DECODE_FILE,  /* "raw" */
	DECODE_BUFFER, /* internal use only, if at all */
	DECODE_WAV,    /* wav */
	DECODE_AU,     /* au */
	DECODE_CDR,    /* cdr */
	DECODE_AUDIOFILE /* internal use only, if at all */
};

/* Playback states mostly for the buffer process.
   Maybe also used in main program. */
enum playstate
{
	play_dead = 0 /* nothing playing, nothing loaded */
,	play_stopped  /* driver present, but no device configured/opened */
/* The ordering is used, state > play_stopped means some device is opened. */
,	play_paused   /* paused, ready to continue, device still active */
,	play_live     /* playing right now */
};

struct out123_struct
{
	enum out123_error errcode;
#ifndef NOXFERMEM
	/* If buffer_pid >= 0, there is a separate buffer process actually
	   handling everything, this instance here is then only a proxy. */
	int buffer_pid;
	int buffer_fd[2];
	txfermem *buffermem;
#endif

	int fn;			/* filenumber */
	void *userptr;	/* driver specific pointer */
	
	/* Callbacks */
	int (*open)(out123_handle *);
	int (*get_formats)(out123_handle *);
	int (*write)(out123_handle *, unsigned char *,int);
	void (*flush)(out123_handle *); /* flush == drop != drain */
	void (*drain)(out123_handle *);
	int (*close)(out123_handle *);
	void (*deinit)(out123_handle *);
	// Enumerate the available devices, if possible. NULL pointer if module doesn't support this.
	int (*enumerate)(out123_handle *, int (*store_device)(void *devlist
	,	const char *name, const char *description), void *devlist);

	/* the loaded that has set the above */
	mpg123_module_t *module;

	char *name;	    /* optional name of this instance */
	char *realname; /* name possibly changed by backend */
	char *driver;	/* driver (module) name */
	char *device;	/* device name */
	int   flags;	/* some bits; namely headphone/speaker/line */
	long rate;		/* sample rate */
	long gain;		/* output gain */
	int channels;	/* number of channels */
	int format;		/* encoding (TODO: rename this to "encoding"!) */
	int framesize;	/* Output needs data in chunks of framesize bytes. */
	unsigned char zerosample[8]; /* Zero in current encoding, max 64 bit. */
	enum playstate state; /* ... */
	int auxflags;	/* For now just one: quiet mode (for probing). */
	int propflags;	/* Property flags, set by driver. */
	double preload;	/* buffer fraction to preload before play */
	int verbose;	/* verbosity to stderr */
	double device_buffer; /* device buffer in seconds */
	char *bindir;	/* OUT123_BINDIR */
/* TODO int intflag;   ... is it really useful/necessary from the outside? */
};

/* Lazy. */
#define AOQUIET ((ao->auxflags | ao->flags) & OUT123_QUIET)
#define AOVERBOSE(v) (!AOQUIET && ao->verbose >= (v))
#define GOOD_WRITEVAL(fd, val)     (INT123_unintr_write(fd, &(val), sizeof((val))) == sizeof((val)))
#define GOOD_WRITEBUF(fd, addr, n) (INT123_unintr_write(fd, (addr), (n)) == (n))
#define GOOD_READVAL(fd, val)      (INT123_unintr_read(fd, &(val), sizeof((val))) == sizeof((val)))
#define GOOD_READBUF(fd, addr, n)  (INT123_unintr_read(fd, (addr), (n)) == (n))

struct audio_format_name {
	int  val;
	char *name;
	char *sname;
};

#endif

