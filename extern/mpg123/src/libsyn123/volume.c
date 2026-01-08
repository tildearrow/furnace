/*
	volume: libsyn123 audio volume handling

	copyright 2018 by the mpg123 project
	licensed under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org

	initially written by Thomas Orgis
*/

#define NO_GROW_BUF
#define NO_SMAX
#include "syn123_int.h"
#include "../common/debug.h"
// Avoid conflict with pragmas in isnan() and friends.
#undef warning

static const double db_min = -SYN123_DB_LIMIT;
static const double db_max =  SYN123_DB_LIMIT;

double attribute_align_arg syn123_db2lin(double db)
{
	if(isnan(db) || isless(db, db_min))
		db = db_min;
	else if(isgreater(db, db_max))
		db = db_max;
	return pow(10, db/20);
}

double attribute_align_arg syn123_lin2db(double volume)
{
	double db;
	if(isnan(volume) || islessequal(volume, 0.))
		db = db_min;
	else
	{
		db = 20*log10(volume);
		if(isgreater(db, db_min))
		{
			if(isgreater(db, db_max))
				db = db_max;
		}
		else
			db = db_min;
	}
	return db;
}


int attribute_align_arg
syn123_amp( void* buf, int encoding, size_t samples
,	double volume, double offset, size_t *clipped, syn123_handle *sh )
{
	size_t clips = 0;
	int err = 0;
	if(!buf)
	{
		err = SYN123_BAD_BUF;
		goto amp_end;
	}
	switch(encoding)
	{
		// This is close to FMA, but only that. It's FAM.
		#define AMP_LOOP(type) \
			for(size_t i=0; i<samples; ++i) \
				((type*)buf)[i] = (type)volume * (((type*)buf)[i] + (type)offset);
		case MPG123_ENC_FLOAT_32:
			AMP_LOOP(float)
			goto amp_end;
		case MPG123_ENC_FLOAT_64:
			AMP_LOOP(double)
			goto amp_end;
		#undef AMP_LOOP
	}
	if(!sh)
	{
		err = SYN123_BAD_ENC;
		goto amp_end;
	}
	else
	{
		char *cbuf = buf;
		int mixenc = syn123_mixenc(encoding, encoding);
		int mixframe = MPG123_SAMPLESIZE(mixenc);
		int inframe = MPG123_SAMPLESIZE(encoding);
		if(!mixenc || !mixframe || !inframe)
		{
			err = SYN123_BAD_CONV;
			goto amp_end;
		}
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
				err = syn123_amp( sh->workbuf, mixenc, block
				,	volume, offset, NULL, NULL );
				if(err)
					return err;
				size_t clips_block = 0;
				err = syn123_conv(
					cbuf, encoding, inframe*block
				,	sh->workbuf, mixenc, mixframe*block
				,	NULL, &clips_block, NULL );
				clips += clips_block;
			}
			if(err)
			{
				mdebug("conv error: %i", err);
				err = SYN123_BAD_CONV; // SYN123_WEIRD also an option
				goto amp_end;
			}
			cbuf += block*inframe;
			samples -= block;
		}
	}
amp_end:
	if(clipped)
		*clipped = clips;
	return err;
}
