/*
	audio: audio output interface

	This wraps a bit of out123 now, with a layer of resampling using syn123.

	copyright ?-2020 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Michael Hipp

	Pitching can work in three ways:
	- fixed (native) decoder rate, varied hardware playback rate
	- fixed hardware playback rate, any decoder rate, resampler
	- pitched NtoM decoder rate, fixed hardware

	Just because ...

	If you force an output rate, either the NtoM resampler in libmpg123 or the syn123
	resampler wrapped here adapts the data.

	TODO: If the proper resampler is configured, it should fill in unsupported output
	rates automatically ... or not? Right now, I have the rule that you don't get
	the expensive resampler unless you forced an output rate. But you do get the
	libmpg123 NtoM resampling now if it finds only a working output rate that is
	reachable via that. Well, imposing a resampler that needs more CPU time than
	the decoder is just something that should not happen without being called for.
	So I rather provide a warning to the user that the NtoM resampler has been
	triggered. Yes, inform the user.
*/

#include <errno.h>
#include "mpg123app.h"
#include "audio.h"
#include "out123.h"
#include "syn123.h"
#include "common.h"
#include "metaprint.h"
#include "sysutil.h"

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include "common/debug.h"

static syn123_handle *sh = NULL;
static struct mpg123_fmt outfmt = { .encoding=0, .rate=0, .channels=0 };
static int outch = 0; // currently used number of output channels

// A convoluted way to say outch*4, for semantic clarity.
#define RESAMPLE_FRAMESIZE(ch) ((ch)*MPG123_SAMPLESIZE(MPG123_ENC_FLOAT_32))
#define OUTPUT_FRAMESIZE(ch)   ((ch)*MPG123_SAMPLESIZE(outfmt.encoding))
// Resampler buffers, first for resampling output, then for conversion.
// Instead of sizing them for any possible input rate, I'll try to
// loop in the output wrapper over a fixed buffer size. The syn123 resampler
// can tell me how many samples to feed to avoid exceeding the output buffer.
static char *resample_buffer = NULL;
static char* resample_outbuf = NULL;
static size_t resample_block = 0;
// A good buffer size:
// 1152*48/44.1*2*4 = 10032 ... let's go 16K.
// This should work for final output data, too.
// We'll loop over pieces if the buffer size is not enough for upsampling.
static size_t resample_bytes = 1<<16;
int do_resample = 0;
int do_resample_now = 0; // really apply resampler for current stream.

/* Quick-shot paired table setup with remembering search in it.
   this is for storing pairs of output sampling rate and decoding
   sampling rate. */
struct ratepair { long a; long b; };
static long *outrates = NULL;
static struct ratepair *unpitch = NULL;


static int audio_capabilities(out123_handle *ao, mpg123_handle *mh);

#define CLEAN_POINTER(p, func) if(p) func(p); p = NULL;
void audio_cleanup(void)
{
	CLEAN_POINTER(outrates, free)
	CLEAN_POINTER(unpitch, free)
	CLEAN_POINTER(sh, syn123_del)
	CLEAN_POINTER(resample_outbuf, free)
	CLEAN_POINTER(resample_buffer, free)
}

int audio_setup(out123_handle *ao, mpg123_handle *mh)
{
	do_resample = (param.force_rate > 0 && param.resample);
	resample_block = 0;
	// Settle formats.
	if(audio_capabilities(ao, mh))
		return -1;
	// Prepare resample.
	// Resampling only for forced rate for now. In future, pitching should
	// also be handled.
	if(do_resample)
	{
		int err;
		sh = syn123_new(outfmt.rate, 1, outfmt.encoding, 0, &err);
		if(!sh)
		{
			merror("Cannot initialize syn123: %s\n", syn123_strerror(err));
			return -1;
		}
		resample_buffer = malloc(resample_bytes*10);
		resample_outbuf = malloc(resample_bytes*10);
		if(!resample_buffer || !resample_outbuf)
			return -1;
	}
	return 0;
}

int audio_prepare( out123_handle *ao, mpg123_handle *mh
,	long rate, int channels, int encoding )
{
	mdebug( "audio_prepare %ld Hz / %ld Hz, %i ch, enc %s"
	,	rate, outfmt.rate, channels, out123_enc_name(encoding) );
	if(do_resample && param.pitch == 0. && rate == outfmt.rate)
	{
		do_resample_now = 0;
		if(param.verbose > 1)
			fprintf(stderr, "Note: resampler disabled for native rate\n");
	} else if(do_resample)
	{
		do_resample_now = 1;
		// Smooth option could be considered once pitching is implemented with the
		// resampler.The existing state might fit the coming data if this is two
		// seamless tracks. If not, it's jut the first few samples that differ
		// significantly depending on which data went through the resampler
		// previously.
		int err = syn123_setup_resample( sh, pitch_rate(rate), outfmt.rate, channels
		,	(param.resample < 2), 0 );
		if(err)
		{
			merror("failed to set up resampler: %s", syn123_strerror(err));
			return -1;
		}
		outch = channels;
		// We can store a certain ammount of frames in the resampler buffer
		// and the final output buffer after conversion.
		size_t frames = resample_bytes / (
			RESAMPLE_FRAMESIZE(channels) > OUTPUT_FRAMESIZE(channels)
			?	RESAMPLE_FRAMESIZE(channels)
			:	OUTPUT_FRAMESIZE(channels) );
		// Minimum amount of input samples to fill the buffer.
		resample_block = syn123_resample_fillcount(pitch_rate(rate), outfmt.rate, frames);
		if(!resample_block)
			return -1; // WTF? No comment.
		if(param.verbose > 1)
			fprintf(stderr, "Note: resampler setup: %ld Hz -> %ld Hz\n", pitch_rate(rate), outfmt.rate);
		rate     = outfmt.rate;
		encoding = outfmt.encoding;
	} else if(outfmt.rate)
		rate = outfmt.rate; // That's pitching with NtoM.
	else
	{
		struct mpg123_frameinfo fi;
		static int ntom_warn = 0;
		if( !ntom_warn && !param.quiet &&
			MPG123_OK == mpg123_info(mh, &fi) && fi.rate != rate )
		{
			fprintf(stderr,
				"\nWarning: You triggered the NtoM drop-sample resampler inside libmpg123.\n"
				"Warning: You could trade CPU for quality by forcing a supported output rate.\n" );
			ntom_warn = 1;
		}
		rate = pitch_rate(rate); // That's plain hardware pitching.
	}
	if(param.verbose > 1)
	{
		const char* encname = out123_enc_name(encoding);
		fprintf( stderr // No extra line break, as this is a follow-up note.
		,	"Note: Hardware output format %li Hz, %i channels, encoding %s.\n"
		,	rate, channels, encname ? encname : "???" );
	}
	return out123_start(ao, rate, channels, encoding);
}

// Loop over blocks with the resampler, think about intflag.
size_t audio_play(out123_handle *ao, void *buffer, size_t bytes)
{
	if(do_resample_now)
	{
		int fs = RESAMPLE_FRAMESIZE(outch);
		size_t pcmframes = bytes/fs;
		size_t done = 0;
		while(pcmframes && !intflag)
		{
			size_t block = resample_block > pcmframes
			?	pcmframes
			:	resample_block;
			size_t oblock = syn123_resample( sh, (float*)resample_buffer
			,	(float*)((char*)buffer+done), block );
			if(!oblock)
				break;
			size_t obytes = 0;
			if(syn123_conv( resample_outbuf, outfmt.encoding, resample_bytes
			,	resample_buffer, MPG123_ENC_FLOAT_32, oblock*fs
			,	&obytes, NULL, sh ))
				break;
			size_t oplay = out123_play(ao, resample_outbuf, obytes);
			if(oplay < obytes)
			{
				// Need to translate that. How many input samples got played,
				// actually? A bit of roundoff error doesn't hurt, so let's just
				// wing it. Close is enough.
				size_t iframes = (size_t)((double)oplay/obytes*block);
				while(iframes >= block)
					--iframes;
				done += iframes*fs;
				break;
			}
			pcmframes -= block;
			done      += block*fs;
		}
		return done;
	}
	else
		return out123_play(ao, buffer, bytes);
}

mpg123_string* audio_enclist(void)
{
	int i;
	mpg123_string *list;
	size_t enc_count = 0;
	const int *enc_codes = NULL;

	/* Only the encodings supported by libmpg123 build
	   Those returned by out123_enc_list() are a superset. */
	mpg123_encodings(&enc_codes, &enc_count);
	if((list = malloc(sizeof(*list))))
		mpg123_init_string(list);
	/* Further calls to mpg123 string lib are hardened against NULL. */
	for(i=0;i<enc_count;++i)
	{
		if(i>0)
			mpg123_add_string(list, " ");
		mpg123_add_string(list, out123_enc_name(enc_codes[i]));
	}
	return list;
}

static void capline(mpg123_handle *mh, long rate, struct mpg123_fmt *outfmt)
{
	int enci;
	const int  *encs;
	size_t      num_encs;
	mpg123_encodings(&encs, &num_encs);
	fprintf(stderr," %5ld |", outfmt ? outfmt->rate : rate);
	for(enci=0; enci<num_encs; ++enci)
	{
		int fmt = outfmt
		?	(encs[enci] == outfmt->encoding ? outfmt->channels : 0)
		:	mpg123_format_support(mh, rate, encs[enci]);
		switch(fmt)
		{
			case MPG123_MONO:               fprintf(stderr, "   M  "); break;
			case MPG123_STEREO:             fprintf(stderr, "   S  "); break;
			case MPG123_MONO|MPG123_STEREO: fprintf(stderr, "  M/S "); break;
			default:                        fprintf(stderr, "      ");
		}
	}
	fprintf(stderr, "\n");
}

void print_capabilities(out123_handle *ao, mpg123_handle *mh)
{
	int r,e;
	const long *rates;
	size_t      num_rates;
	const int  *encs;
	size_t      num_encs;
	char *name;
	char *dev;
	out123_driver_info(ao, &name, &dev);
	mpg123_rates(&rates, &num_rates);
	mpg123_encodings(&encs, &num_encs);
	fprintf(stderr,"\nAudio driver: %s\nAudio device: ", name);
	print_outstr(stderr, dev, 0, stderr_is_term);
	fprintf(stderr, "\n");
	fprintf( stderr, "%s", "Audio capabilities:\n"
		"(matrix of [S]tereo or [M]ono support for sample format and rate in Hz)\n"
		"\n"
		" rate  |" );
	for(e=0;e<num_encs;e++)
	{
		const char *encname = out123_enc_name(encs[e]);
		fprintf(stderr," %4s ", encname ? encname : "???");
	}

	fprintf(stderr,"\n -------");
	for(e=0;e<num_encs;e++) fprintf(stderr,"------");

	fprintf(stderr, "\n");
	for(r=0; r<num_rates; ++r) capline(mh, rates[r], NULL);

	if(param.force_rate)
	{
		fprintf(stderr," -------");
		for(e=0;e<num_encs;e++) fprintf(stderr,"------");
		fprintf(stderr, "\n");
		if(do_resample)
			capline(mh, 0, &outfmt);
		else
			capline(mh,  bpitch_rate(param.force_rate), NULL);
	}
	fprintf(stderr,"\n");
	if(do_resample)
	{
		if(param.pitch != 0.)
			fprintf( stderr, "Resampler with pitch: %g\n"
			,	param.pitch );
		else
			fprintf(stderr, "Resampler configured.\n");
		fprintf( stderr, "%s\n%s\n"
		,	"Decoding to f32 as intermediate if needed."
		,	"Resampler output format is in the last line." );
	}
	else if(param.force_rate)
		fprintf( stderr
		,	"%s rate forced. Resulting format support shown in last line.\n"
		,	param.pitch != 0. ? "Pitched decoder" : "Decoder" );
	else if(param.pitch != 0.)
		fprintf( stderr, "Actual output rates adjusted by pitch value %g.\n"
		,	param.pitch );
}


long brate(struct ratepair *table, long arate, int count, int *last)
{
	int i = 0;
	int j;
	for(j=0; j<2; ++j)
	{
		i = i ? 0 : *last;
		for(; i<count; ++i) if(table[i].a == arate)
		{
			*last = i;
			return table[i].b;
		}
	}
	return 0;
}

// Return one of the given list of encodings matching the mask,
// in order. Zero if none.
static int match_enc(int mask, const int *enc_list, size_t enc_count)
{
	int enc = 0;
	for(size_t i=0; i<enc_count; ++i)
	{
		if((enc_list[i] & mask) == enc_list[i])
		{
			enc = enc_list[i];
			break;
		}
	}
	return enc;
}

/* This uses the currently opened audio device, queries its caps.
   In case of buffered playback, this works _once_ by querying the buffer for the caps before entering the main loop. */
static int audio_capabilities(out123_handle *ao, mpg123_handle *mh)
{
	int force_fmt = 0;
	size_t ri;
	/* Pitching introduces a difference between decoder rate and playback rate. */
	long decode_rate;
	const long *rates;
	struct mpg123_fmt *outfmts = NULL;
	int fmtcount;
	size_t num_rates;
	if(param.pitch < -0.99)
		param.pitch = -0.99;
	long ntom_rate = do_resample ? 0 : bpitch_rate(param.force_rate);
	outfmt.rate = param.force_rate;
	outfmt.channels = 0;
	outfmt.encoding = 0;

	debug("audio_capabilities");
	mpg123_rates(&rates, &num_rates);

	mpg123_format_none(mh); /* Start with nothing. */

	if(do_resample && param.verbose > 2)
		fprintf( stderr
		,	"Note: decoder always forced to %s encoding for resampler\n"
		,	out123_enc_name(MPG123_ENC_FLOAT_32) );

	if(param.force_encoding != NULL)
	{
		if(!param.quiet)
			fprintf(stderr, "Note: forcing output encoding %s\n", param.force_encoding);

		force_fmt = out123_enc_byname(param.force_encoding);
		if(!force_fmt)
		{
			char *fs = NULL;
			outstr(&fs, param.force_encoding, 0, stderr_is_term);
			error1("Failed to find an encoding to match requested \"%s\"!\n"
			,	PSTR(fs));
			free(fs);
			return -1; /* No capabilities at all... */
		}
		else if(param.verbose > 2)
			fprintf(stderr, "Note: forcing encoding code 0x%x (%s)\n"
			,	(unsigned)force_fmt, out123_enc_name(force_fmt));
	}

	// A possible optimization for resampling mode is to keep existing output
	// format support configured and don't even interrupt the output device at
	// all. If you change pitch, you just change a number for the resampler.
	// But currently, the idea of re-opening the output device on format
	// changes is rather ingrained in mpg123.

	if(do_resample)
	{
		// If really doing the extra resampling, output will always run with
		// this setup, regardless of decoder.
		int enc1 = out123_encodings(ao, outfmt.rate, 1);
		int enc2 = out123_encodings(ao, outfmt.rate, 2);
		if(force_fmt)
		{
			enc1 &= force_fmt;
			enc2 &= force_fmt;
		} else
		{
			long propflags = 0;
			out123_getparam_int(ao, OUT123_PROPFLAGS, &propflags);
			if(!(propflags & OUT123_PROP_LIVE))
			{
				fmtcount = out123_formats(ao, NULL, 0, 1, 2, &outfmts);
				if(fmtcount == 1 && outfmts[0].encoding > 0)
				{
					const char *encname = out123_enc_name(outfmts[0].encoding);
					if(param.verbose > 1)
						fprintf( stderr, "Note: honouring non-live default encoding of %s\n"
						,	encname ? encname : "???" );
					enc1 &= outfmts[0].encoding;
					enc2 &= outfmts[0].encoding;
				}
				free(outfmts);
				outfmts = NULL;
			} else if(param.verbose > 1)
				fprintf(stderr, "Note: negotiating the best encoding with live sink\n");
		}
		mdebug("enc mono=0x%x stereo=0x%x", (unsigned)enc1, (unsigned)enc2);
		if(!enc1 && !enc2)
		{
			error("Output device does not support forced rate and/or encoding.");
			return -1;
		} else if(enc1 && enc2)
		{
			// Should be the normal case: Mono and Stereo with the same formats.
			// We'll store the common subset, which should normally be all.
			outfmt.encoding = enc1 & enc2;
			outfmt.channels = MPG123_MONO|MPG123_STEREO;
			if(!outfmt.encoding)
			{
				error("No common decodings for mono and stereo output. Too weird.");
				return -1;
			}
		} else if(enc1)
		{ // Mono only.
			outfmt.encoding = enc1;
			outfmt.channels = 1;
		} else
		{ // Stereo only.
			outfmt.encoding = enc2;
			outfmt.channels = 2;
		}
		int pref_enc[] = { MPG123_ENC_FLOAT_32
		,	MPG123_ENC_SIGNED_32, MPG123_ENC_UNSIGNED_32
		,	MPG123_ENC_SIGNED_24, MPG123_ENC_UNSIGNED_24
		,	MPG123_ENC_SIGNED_16, MPG123_ENC_UNSIGNED_16 };
		int nenc = match_enc(outfmt.encoding, pref_enc, sizeof(pref_enc)/sizeof(int));
		if(!nenc)
		{
			const int *encs;
			size_t num_encs;
			mpg123_encodings(&encs, &num_encs);
			nenc = match_enc(outfmt.encoding, encs, num_encs);
		}
		if(nenc)
			outfmt.encoding = nenc;
		else
		{
			merror( "Found no encoding to match mask 0x%08x."
			,	(unsigned int)outfmt.encoding );
			if(force_fmt)
				error("Perhaps your forced output encoding is not supported.");
			return -1;
		}
		const char *encname = out123_enc_name(outfmt.encoding);
		if(param.verbose > 1)
			for(int ch=MPG123_MONO; ch<=MPG123_STEREO; ++ch)
				if(outfmt.channels & ch)
					fprintf(stderr, "Note: output format %li Hz, %s, %s\n"
					,	outfmt.rate, ch==MPG123_MONO ? "mono" : "stereo"
					,	encname ? encname : "???" );
	}

	// Either enable or disable rate forcing, whith ntom_rate non-zero or not.
	if(mpg123_param(mh, MPG123_FORCE_RATE, ntom_rate, 0) != MPG123_OK)
	{
		merror("Cannot force NtoM rate: %s", mpg123_strerror(mh));
		return -1;
	}

	if(ntom_rate)
	{
		// Only that one rate is enforced. Nothing else needs to be checked.
		// For pitching, ntom_rate has been adjusted. The output uses outfmt.rate.
		// Need to tell mpg123 about the forced rate to make it work.
		for(int ch=1; ch<=2; ++ch)
		{
			int fmts = out123_encodings(ao, outfmt.rate, ch);
			if(param.verbose > 2)
				fprintf( stderr
				,	"Note: output support for %li Hz, %s: 0x%x\n"
				,	outfmt.rate, ch==MPG123_MONO ? "mono" : "stereo", fmts );
			if(force_fmt)
				fmts = ((fmts & force_fmt) == force_fmt) ? force_fmt : 0;
			mpg123_format(mh, ntom_rate, ch, fmts);
		}
	} else if(do_resample)
	{
		// Support any decoding rate with float output for the resampler and also
		// direct decoding to confiugred output format.
		// One twist: Disable high rates with signal that the resampler will throw
		// away anyway. This includes pitch. 22040 Hz output rate with pitch 0.5
		// still wants the full 44100 Hz input data, as original signal up to
		// 22040 Hz will be heard as up to 11020 Hz. So we want pitch_rate()
		// to be above outfmt.rate. Final resampling ratio not above 2.
		for(ri=0; ri<num_rates; ++ri)
		{
			if(rates[ri] > 12000 && pitch_rate(rates[ri]) > outfmt.rate*2)
				break;
			int fmt = (param.pitch == 0. && rates[ri] == outfmt.rate)
			? outfmt.encoding
			: MPG123_ENC_FLOAT_32;
			mpg123_format(mh, rates[ri], outfmt.channels, fmt);
		}
	} else
	{
		// Finally, the old style, direct decoding to possibly pitched output.
		if(!outrates)
			outrates = malloc(sizeof(*rates)*num_rates);
		if(!unpitch)
			unpitch  = malloc(sizeof(*unpitch)*num_rates);
		if(!outrates || !unpitch)
		{
			CLEAN_POINTER(outrates, free)
			CLEAN_POINTER(unpitch, free)
			error("DOOM");
			return -1;
		}
		for(ri = 0; ri<num_rates; ri++)
		{
			decode_rate   = rates[ri];
			outrates[ri]  = pitch_rate(decode_rate);
			unpitch[ri].a = outrates[ri];
			unpitch[ri].b = decode_rate;
		}
		/* Actually query formats possible with given rates. */
		fmtcount = out123_formats(ao, outrates, num_rates, 1, 2, &outfmts);
		// Remember: First one is a default format, then come my rates.
		if(fmtcount > 0)
		{
			int fi;
			int unpitch_i = 0;
			if(param.verbose > 1 && outfmts[0].encoding > 0)
			{
				const char *encname = out123_enc_name(outfmts[0].encoding);
				fprintf(stderr, "Note: default format %li Hz, %i channels, %s\n"
				,	outfmts[0].rate, outfmts[0].channels
				,	encname ? encname : "???" );
			}
			for(fi=1; fi<fmtcount; ++fi)
			{
				int fmts = outfmts[fi].encoding;
				if(param.verbose > 2)
					fprintf( stderr
					,	"Note: output support for %li Hz, %i channels: 0x%x\n"
					,	outfmts[fi].rate, outfmts[fi].channels, outfmts[fi].encoding );
				if(force_fmt)
					fmts = ((fmts & force_fmt) == force_fmt) ? force_fmt : 0;
				decode_rate = brate(unpitch, outfmts[fi].rate, num_rates, &unpitch_i);
				mpg123_format(mh, decode_rate, outfmts[fi].channels, fmts);
			}
		}
		free(outfmts);
	}

	if(param.verbose > 1) print_capabilities(ao, mh);

	return 0;
}

int set_pitch(mpg123_handle *fr, out123_handle *ao, double new_pitch)
{
	double old_pitch = param.pitch;
	long rate;
	int channels, format;
	int smode = 0;

	/* Be safe, check support. */
	if(mpg123_getformat(fr, &rate, &channels, &format) != MPG123_OK)
	{
		/* We might just not have a track handy. */
		error("There is no current audio format, cannot apply pitch. This might get fixed in future.");
		return 0;
	}

	if(outfmt.rate && !do_resample)
	{
		error("Runtime pitching requires either proper resampler or flexible hardware rate.");
		return 0;
	}

	param.pitch = new_pitch;

	if(channels == 1) smode = MPG123_MONO;
	if(channels == 2) smode = MPG123_STEREO;

	out123_stop(ao);
	/* Remember: This takes param.pitch into account. */
	audio_capabilities(ao, fr);
	if(!do_resample && !(mpg123_format_support(fr, rate, format) & smode))
	{

		/* Note: When using --pitch command line parameter, you can go higher
		   because a lower decoder sample rate is automagically chosen.
		   Here, we'd need to switch decoder rate during track... good? */
		error("Reached a hardware limit there with pitch!");
		param.pitch = old_pitch;
		audio_capabilities(ao, fr);
	}
	else
		mpg123_decoder(fr, NULL);
	return audio_prepare(ao, fr, rate, channels, format);
}

int set_mute(out123_handle *ao, int mutestate)
{
	return out123_param( ao
	,	mutestate ? OUT123_ADD_FLAGS : OUT123_REMOVE_FLAGS
	,	OUT123_MUTE, 0, NULL );
}
