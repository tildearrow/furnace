/*
 *	sndio: sndio audio output
 *
 * Copyright (c) 2008 Christian Weisgerber <naddy@openbsd.org>,
 *                    Alexandre Ratchov <alex@caoua.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "../out123_int.h"

#include <sndio.h>

#include "../../common/debug.h"

static int sndio_to_mpg123_enc(int sign, int bits)
{
	switch(bits)
	{
		case 8:
			return sign ? MPG123_ENC_SIGNED_8 : MPG123_ENC_UNSIGNED_8;
		case 16:
			return sign ? MPG123_ENC_SIGNED_16 : MPG123_ENC_UNSIGNED_16;
		case 24:
			return sign ? MPG123_ENC_SIGNED_24 : MPG123_ENC_UNSIGNED_24;
		case 32:
			return sign ? MPG123_ENC_SIGNED_32 : MPG123_ENC_UNSIGNED_32;
	}
	return -1;
}

static double rate_diff(long a, unsigned int b)
{
	double ar = (double)a;
	double br = (double)b;
	return ( a>b ? (ar-br)/ar : ( a <= 0 ? 1. : (br-ar)/ar ) );
}

static int mpg123_to_sndio_enc(int enc, unsigned int *sig, unsigned int *bits)
{
	switch(enc) {
	case -1:
		// Query default format only.
		break;
	case MPG123_ENC_SIGNED_32:
		*sig = 1;
		*bits = 32;
		break;
	case MPG123_ENC_UNSIGNED_32:
		*sig = 0;
		*bits = 32;
		break;
	case MPG123_ENC_SIGNED_24:
		*sig = 1;
		*bits = 24;
		break;
	case MPG123_ENC_UNSIGNED_24:
		*sig = 0;
		*bits = 24;
		break;
	case MPG123_ENC_SIGNED_16:
		*sig = 1;
		*bits = 16;
		break;
	case MPG123_ENC_UNSIGNED_16:
		*sig = 0;
		*bits = 16;
		break;
	case MPG123_ENC_UNSIGNED_8:
		*sig = 0;
		*bits = 8;
		break;
	case MPG123_ENC_SIGNED_8:
		*sig = 1;
		*bits = 8;
		break;
	default:
		return -1;
	}
	return 0;
}


// Just check if there seems some support for stereo audio,
// take maximum channel count otherwise.
static unsigned int guess_channels(struct sio_hdl *hdl)
{
	struct sio_cap cap;
	unsigned int maxchan = 0;
	unsigned int stereo_mask = 0;
	unsigned int all_conf = 0;
	if(!sio_getcap(hdl, &cap))
		return 0; // Zero is as good as nothing.
	// There is no specification of order in pchan[], So no guessing
	// about which index for stereo.
	for(int ci=0; ci<SIO_NCHAN; ++ci)
	{
		if(2 == cap.pchan[ci])
			stereo_mask |= 1 << ci; // Maybe even multiple entries, eh?
	}
	for(unsigned int i=0; i<cap.nconf; ++i)
	{
		all_conf |= cap.confs[i].pchan;
	}
	for(int ci=0; ci<SIO_NCHAN; ++ci)
	{
		if(all_conf & (1 << ci) && cap.pchan[ci] > maxchan)
			maxchan = cap.pchan[ci];
	}
	mdebug("maximum device channels: %u\n", maxchan);
	return all_conf & stereo_mask ? 2 : maxchan;
}

static int open_sndio(out123_handle *ao)
{
	struct sio_hdl *hdl;
	struct sio_par par;

	hdl = sio_open(ao->device /* NULL is fine */, SIO_PLAY, 0);
	if (hdl == NULL)
	{
		if(!AOQUIET)
			error("Got nothing from sio_open(). ");
		return -1;
	}

	sio_initpar(&par);

	par.le = SIO_LE_NATIVE;
	if(ao->format != -1)
	{
		mdebug("Actually opening with %d channels, rate %ld.", ao->channels, ao->rate);
		par.rate = ao->rate;
		par.pchan = ao->channels;
	} else
	{
		// Hack around buggy sndio versions up to 1.8.0 that fail to 
		// neuter the default value before handing to OSS driver.
		// Also need to re-open, as sndio likes errors to be fatal.
		if(!sio_setpar(hdl, &par))
		{
			sio_close(hdl);
			hdl = sio_open(ao->device, SIO_PLAY, 0);
			if(hdl == NULL)
			{
				if(!AOQUIET)
					error("Re-opening of device for channel guessing failed.");
				return -1;
			}
			par.pchan = guess_channels(hdl);
		}
	}

	if(mpg123_to_sndio_enc(ao->format, &par.sig, &par.bits))
	{
		if (!AOQUIET)
			error1("invalid sample format %d",
			    ao->format);
		sio_close(hdl);
		return -1;
	}

	if (!sio_setpar(hdl, &par) || !sio_getpar(hdl, &par)
		|| par.le != SIO_LE_NATIVE )
	{
		if(!AOQUIET)
			error("parameter setup  failure");
		sio_close(hdl);
		return -1;
	}
	if(ao->format == -1) // Store default format.
	{
		ao->format = sndio_to_mpg123_enc(par.sig, par.bits);
		ao->rate = par.rate;
		ao->channels = par.pchan;
	}
	else
	{
		 if( ao->format != sndio_to_mpg123_enc(par.sig, par.bits)
			|| ao->channels != par.pchan
			|| rate_diff(ao->rate, par.rate) > 0.005
			)
		{
			if(!AOQUIET)
				error("format not accepted as given");
			sio_close(hdl);
			return -1;
		}
		if(!sio_start(hdl))
		{
			if(!AOQUIET)
				error("cannot start");
			sio_close(hdl);
			return -1;
		}
	}
	ao->userptr = hdl;
	return 0;
}

static int get_formats_sndio(out123_handle *ao)
{
	struct sio_hdl *hdl = (struct sio_hdl *)ao->userptr;
	struct sio_cap cap;
	int fmt = 0;

	// Direct querying with sio_setpar()/sio_getpar is too slow, so let's
	// learn about the funky bitmask indexing of the capability stuff.
	if(!sio_getcap(hdl, &cap))
	{
		if(!AOQUIET)
			error("failure getting caps");
		return 0;
	}

	unsigned int rmask = 0;
	for(int ri=0; ri<SIO_NRATE; ++ri)
	{
		if(ao->rate == cap.rate[ri])
		{
			rmask |= 1 << ri;
			break;
		}
	}
	if(!rmask)
	{
		for(int ri=0; ri<SIO_NRATE; ++ri)
		{
			if(rate_diff(ao->rate, cap.rate[ri]) <= 0.005)
			{
				rmask |= 1 << ri;
				break;
			}
		}
	}

	unsigned int cmask = 0;
	for(int ci=0; ci<SIO_NCHAN; ++ci)
	{
		if(ao->channels == cap.pchan[ci])
		{
			cmask |= 1 << ci;
			break;
		}
	}

	if(!rmask || !cmask)
	{
		// no rate match, do the elaborate check.
		debug("cap table does not help, doing elaborate format check");
		static int fmts[] =
		{
			MPG123_ENC_SIGNED_8,  MPG123_ENC_UNSIGNED_8
		,	MPG123_ENC_SIGNED_16, MPG123_ENC_UNSIGNED_16
		,	MPG123_ENC_SIGNED_24, MPG123_ENC_UNSIGNED_24
		,	MPG123_ENC_SIGNED_32, MPG123_ENC_UNSIGNED_32
		};

		for(int i=0;i<sizeof(fmts)/sizeof(int);i++) 
		{
			struct sio_par par;
			sio_initpar(&par);
			par.le = SIO_LE_NATIVE;
			mpg123_to_sndio_enc(fmts[i], &par.sig, &par.bits);
			par.rate = ao->rate;
			par.pchan = ao->channels;
			if(sio_setpar(hdl, &par) && sio_getpar(hdl, &par))
			{
				if( par.le == SIO_LE_NATIVE
					&& fmts[i]  == sndio_to_mpg123_enc(par.sig, par.bits)
					&& ao->channels == par.pchan
					&& rate_diff(ao->rate, par.rate) <= 0.005
				)
				fmt |= fmts[i];
			}
		}
	} else
	{
		int menc[SIO_NENC];
		for(int ei=0; ei<SIO_NENC; ++ei)
		{
			if(cap.enc[ei].le != SIO_LE_NATIVE)
				menc[ei] = 0;
			else
				menc[ei] = sndio_to_mpg123_enc(cap.enc[ei].sig, cap.enc[ei].bits);
			if(menc[ei] < 0)
				menc[ei] = 0;
		}
		for(unsigned int i=0; i<cap.nconf; ++i)
		{
			if(cap.confs[i].pchan & cmask && cap.confs[i].rate & rmask)
			for(int ei=0; ei<SIO_NENC; ++ei)
			if(cap.confs[i].enc & (1<<ei))
				fmt |= menc[ei];
		}
	}
	return fmt;
}

static int write_sndio(out123_handle *ao, unsigned char *buf, int len)
{
	struct sio_hdl *hdl = (struct sio_hdl *)ao->userptr;
	int count;

	count = (int)sio_write(hdl, buf, len);
	if (count == 0 && sio_eof(hdl))
		return -1;
	return count;
}

static void flush_sndio(out123_handle *ao)
{
	return;
}

static int close_sndio(out123_handle *ao)
{
	struct sio_hdl *hdl = (struct sio_hdl *)ao->userptr;

	if(hdl)
		sio_close(hdl);
	return 0;
}

static int init_sndio(out123_handle* ao)
{
	if (ao == NULL)
		return -1;
	
	/* Set callbacks */
	ao->open = open_sndio;
	ao->flush = flush_sndio;	/* required */
	ao->write = write_sndio;
	ao->get_formats = get_formats_sndio;
	ao->close = close_sndio;

	/* Success */
	return 0;
}

/* 
	Module information data structure
*/
mpg123_module_t mpg123_output_module_info = {
	/* api_version */	MPG123_MODULE_API_VERSION,
	/* name */		"sndio",						
	/* description */	"Output audio using sndio library",
	/* revision */		"$Rev:$",						
	/* handle */		NULL,
	
	/* init_output */	init_sndio,						
};
