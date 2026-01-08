/*
	common: misc stuff... audio flush, status display...

	copyright ?-2023 by the mpg123 project
	free software under the terms of the LGPL 2.1

	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Michael Hipp
*/

/* Need snprintf. */
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#include "mpg123app.h"
#include "out123.h"
#include <sys/stat.h>
#include "common.h"
#include "terms.h"
#include "metaprint.h"

#include "common/debug.h"

enum player_state playstate = STATE_PLAYING;
const char playsym[STATE_COUNT] = { '>', '_', '=', '?' };
int muted = 0;
// On LFS conversion trouble with large files, print_stat() gets disabled.
// Some heuristic re-enables it (when you print headers).
static int print_stat_disabled = FALSE;

const char* rva_name[3] = { "off", "mix", "album" };
static const char* rva_statname[3] = { "---", "mix", "alb" };
static const char *modes[5] = {"Stereo", "Joint-Stereo", "Dual-Channel", "Single-Channel", "Invalid" };
static const char *smodes[5] = { "stereo", "j-s", "dual", "mono", "o.O" };
static const char *layers[4] = { "Unknown" , "I", "II", "III" };
static const char *versions[4] = {"1.0", "2.0", "2.5", "x.x" };
static const int samples_per_frame[4][4] =
{
	{ -1,384,1152,1152 },	/* MPEG 1 */
	{ -1,384,1152,576 },	/* MPEG 2 */
	{ -1,384,1152,576 },	/* MPEG 2.5 */
	{ -1,-1,-1,-1 },		/* Unknown */
};

/* concurring to print_rheader... here for control_generic */
const char* remote_header_help = "S <mpeg-version> <layer> <sampling freq> <mode(stereo/mono/...)> <mode_ext> <framesize> <stereo> <copyright> <error_protected> <emphasis> <bitrate> <extension> <vbr(0/1=yes/no)>";
void print_remote_header(mpg123_handle *mh)
{
	struct mpg123_frameinfo i;
	mpg123_info(mh, &i);
	if(i.mode >= 4 || i.mode < 0) i.mode = 4;
	if(i.version >= 3 || i.version < 0) i.version = 3;
	generic_sendmsg("S %s %d %ld %s %d %d %d %d %d %d %d %d %d",
		versions[i.version],
		i.layer,
		i.rate,
		modes[i.mode],
		i.mode_ext,
		i.framesize,
		i.mode == MPG123_M_MONO ? 1 : 2,
		i.flags & MPG123_COPYRIGHT ? 1 : 0,
		i.flags & MPG123_CRC ? 1 : 0,
		i.emphasis,
		i.bitrate,
		i.flags & MPG123_PRIVATE ? 1 : 0,
		i.vbr);
	print_stat_disabled=FALSE;
}

void print_header(mpg123_handle *mh)
{
	struct mpg123_frameinfo i;
	mpg123_info(mh, &i);
	if(i.mode > 4 || i.mode < 0) i.mode = 4;
	if(i.version > 3 || i.version < 0) i.version = 3;
	if(i.layer > 3 || i.layer < 0) i.layer = 0;
	fprintf(stderr,"MPEG %s, Layer: %s, Freq: %ld, mode: %s, modext: %d, BPF : %d\n", 
		versions[i.version],
		layers[i.layer], i.rate,
		modes[i.mode],i.mode_ext,i.framesize);
	fprintf(stderr,"Channels: %d, copyright: %s, original: %s, CRC: %s, emphasis: %d.\n",
		i.mode == MPG123_M_MONO ? 1 : 2,i.flags & MPG123_COPYRIGHT ? "Yes" : "No",
		i.flags & MPG123_ORIGINAL ? "Yes" : "No", i.flags & MPG123_CRC ? "Yes" : "No",
		i.emphasis);
	fprintf(stderr,"Bitrate: ");
	switch(i.vbr)
	{
		case MPG123_CBR:
			if(i.bitrate) fprintf(stderr, "%d kbit/s", i.bitrate);
			else fprintf(stderr, "%d kbit/s (free format)", (int)((double)(i.framesize+4)*8*i.rate*0.001/samples_per_frame[i.version][i.layer]+0.5));
			break;
		case MPG123_VBR: fprintf(stderr, "VBR"); break;
		case MPG123_ABR: fprintf(stderr, "%d kbit/s ABR", i.abr_rate); break;
		default: fprintf(stderr, "???");
	}
	fprintf(stderr, " Extension value: %d\n",	i.flags & MPG123_PRIVATE ? 1 : 0);
	print_stat_disabled=FALSE;
}

void print_header_compact(mpg123_handle *mh)
{
	struct mpg123_frameinfo i;
	mpg123_info(mh, &i);
	if(i.mode > 4 || i.mode < 0) i.mode = 4;
	if(i.version > 3 || i.version < 0) i.version = 3;
	if(i.layer > 3 || i.layer < 0) i.layer = 0;
	
	fprintf(stderr,"MPEG %s L %s ", versions[i.version], layers[i.layer]);
	switch(i.vbr)
	{
		case MPG123_CBR:
			if(i.bitrate) fprintf(stderr, "cbr%d", i.bitrate);
			else fprintf(stderr, "cbr%d", (int)((double)i.framesize*8*i.rate*0.001/samples_per_frame[i.version][i.layer]+0.5));
			break;
		case MPG123_VBR: fprintf(stderr, "vbr"); break;
		case MPG123_ABR: fprintf(stderr, "abr%d", i.abr_rate); break;
		default: fprintf(stderr, "???");
	}
	fprintf(stderr," %ld %s\n", i.rate, smodes[i.mode]);
	print_stat_disabled=FALSE;
}

unsigned int roundui(double val)
{
	double base = floor(val);
	return (unsigned int) ((val-base) < 0.5 ? base : base + 1 );
}

/* Split into mm:ss.xx or hh:mm:ss, depending on value. */
static void settle_time(double tim, unsigned long *times, char *sep)
{
	if(tim >= 3600.)
	{
		*sep = ':';
		times[0] = (unsigned long) tim/3600;
		tim -= times[0]*3600;
		times[1] = (unsigned long) tim/60;
		tim -= times[1]*60;
		times[2] = (unsigned long) tim;
	}
	else
	{
		*sep = '.';
		times[0] = (unsigned long) tim/60;
		times[1] = (unsigned long) tim%60;
		times[2] = (unsigned long) (tim*100)%100;
	}
}

/* Print output buffer fill. */
void print_buf(const char* prefix, out123_handle *ao)
{
	long rate;
	int framesize;
	double tim;
	unsigned long times[3];
	char timesep;
	size_t buffsize;

	buffsize = out123_buffered(ao);
	if(out123_getformat(ao, &rate, NULL, NULL, &framesize))
		return;
	tim = (double)(buffsize/framesize)/rate;
	settle_time(tim, times, &timesep);
	fprintf( stderr, "\r%s[%02lu:%02lu%c%02lu]"
	,	prefix, times[0], times[1], timesep, times[2] );
}

// This is a massively complicated function just for telling where we are.
// Blame buffering. Blame format conversion. Blame the universe.
int position_info( mpg123_handle *fr, off_t offset, out123_handle *ao
,	off_t *frame, off_t *frame_remain
,	double *seconds, double *seconds_remain, double *seconds_buffered, double *seconds_total )
{
	size_t buffered;
	off_t decoded;
	double elapsed;
	double remain;
	double length;
	off_t frameo;
	off_t frames;
	off_t rframes;
	int framesize;
	int spf;
	long inrate;
	long rate;
	if(mpg123_getformat(fr, &inrate, NULL, NULL) || inrate < 1)
		return -1;
	if(out123_getformat(ao, &rate, NULL, NULL, &framesize) || rate < 1 || framesize < 1)
		return -1;
	buffered = out123_buffered(ao)/framesize;
	decoded  = mpg123_tell(fr);
	length   = (double)mpg123_length(fr)/inrate;
	frameo   = mpg123_tellframe(fr);
	frames   = mpg123_framelength(fr);
	spf      = mpg123_spf(fr);
	if(decoded < 0 || length < 0 || frameo < 0 || frames <= 0 || spf <= 0)
	{
		merror("Failed to gather position data: %s", mpg123_strerror(fr));
		return -1;
	}
	frameo += offset;
	if(frameo < 0)
		frameo = 0;
	/* Some sensible logic around offsets and time.
	   Buffering makes the relationships between the numbers non-trivial. */
	rframes = frames-frameo;
	// May be negative, a countdown. Buffer only confuses in paused (looping) mode, though.
	elapsed = (double)(decoded + offset*spf)/inrate - (double)(playstate==STATE_LOOPING ? 0 : buffered)/rate;
	remain  = elapsed > 0 ? length - elapsed : length;

	if(frame)
		*frame = frameo;
	if(frame_remain)
		*frame_remain = rframes;
	if(seconds)
		*seconds = elapsed;
	if(seconds_remain)
		*seconds_remain = remain;
	if(seconds_buffered)
		*seconds_buffered = (double)buffered/rate;
	if(seconds_total)
		*seconds_total = length;

	return 0;
}


/* Note about position info with buffering:
   Negative positions mean that the previous track is still playing from the
   buffer. It's a countdown. The frame counter always relates to the last
   decoded frame, what entered the buffer right now. */
void print_stat(mpg123_handle *fr, long offset, out123_handle *ao, int draw_bar
,	struct parameter *param)
{
	if(print_stat_disabled)
		return;
	static int old_term_width = -1;
	double basevol, realvol;
	double elapsed;
	double remain;
	double bufsec;
	double length;
	off_t frame;
	off_t rframes;
	struct mpg123_frameinfo mi;
	char linebuf[256];
	char *line = NULL;

#ifndef __OS2__
#ifndef _WIN32
#ifndef GENERIC
/* Only generate new stat line when stderr is ready... don't overfill... */
	{
		struct timeval t;
		fd_set serr;
		int n,errfd = fileno(stderr);

		t.tv_sec=t.tv_usec=0;

		FD_ZERO(&serr);
		FD_SET(errfd,&serr);
		n = select(errfd+1,NULL,&serr,NULL,&t);
		if(n <= 0) return;
	}
#endif
#endif
#endif
	if(position_info(fr, offset, ao, &frame, &rframes, &elapsed, &remain, &bufsec, &length))
	{
		debug("position_info() failed");
		print_stat_disabled=TRUE;
		return;
	}
	if(  MPG123_OK == mpg123_info(fr, &mi)
	  && MPG123_OK == mpg123_getvolume(fr, &basevol, &realvol, NULL) )
	{
		char framefmt[10];
		char framestr[2][32];
		int linelen;
		int maxlen;
		int len;
		int ti;
		/* Deal with overly long times. */
		double tim[3];
		unsigned long times[3][3];
		char timesep[3];
		char sign[3] = {' ', ' ', ' '};

		/* 255 is enough for the data I prepare, if there is no terminal width to
		   fill */
		maxlen  = term_width(STDERR_FILENO);
		if(draw_bar && maxlen > 0 && maxlen < old_term_width)
		{
			// Hack about draw_bar: That's the normal print_stat that is not followed by
			// metadata anyway. No need to double things.
			print_stat(fr, offset, ao, 0, param);
			if(param->verbose > 2)
				fprintf(stderr,"Note: readjusting for smaller terminal (%d to %d)\n", old_term_width, maxlen);
			fprintf(stderr, "\n\n\n");
			if(param->verbose > 1)
				print_header(fr);
			else
				print_header_compact(fr);
			print_id3_tag(fr, param->long_id3, stderr, maxlen);
		}
		if(draw_bar)
			old_term_width = maxlen;
		linelen = maxlen > 0 ? maxlen : (sizeof(linebuf)-1);
		line = linelen >= sizeof(linebuf)
		?	malloc(linelen+1) /* Only malloc if it is a really long line. */
		:	linebuf; /* Small buffer on stack is enough. */

		tim[0] = elapsed;
		tim[1] = remain;
		tim[2] = bufsec;
		for(ti=0; ti<3; ++ti)
		{
			if(tim[ti] < 0.){ sign[ti] = '-'; tim[ti] = -tim[ti]; }
			settle_time(tim[ti], times[ti], &timesep[ti]);
		}
		/* Taking pains to properly size the frame number fields. */
		len = snprintf( framefmt, sizeof(framefmt)
		,	"%%0%d" PRIiMAX, (int)log10(frame+rframes)+1 );
		if(len < 0 || len >= sizeof(framefmt))
			memcpy(framefmt, "%05" PRIiMAX, sizeof("%05" PRIiMAX));
		snprintf( framestr[0], sizeof(framestr[0])-1, framefmt, (intmax_t)frame);
		framestr[0][sizeof(framestr[0])-1] = 0;
		snprintf( framestr[1], sizeof(framestr[1])-1, framefmt, (intmax_t)rframes);
		framestr[1][sizeof(framestr[1])-1] = 0;
		/* Now start with the state line. */
		memset(line, 0, linelen+1); /* Always one zero more. */
		/* Start with position info. */
		len = snprintf( line, linelen
		,	"%c %s+%s %c%02lu:%02lu%c%02lu+%02lu:%02lu%c%02lu"
		,	playsym[playstate]
		,	framestr[0], framestr[1]
		,	sign[0]
		,	times[0][0], times[0][1], timesep[0], times[0][2]
		,	times[1][0], times[1][1], timesep[1], times[1][2]
		);
		/* Just cut it. */
		if(len >= linelen)
			len=linelen;
		if(len >= 0 && param->usebuffer && len < linelen )
		{ /* Buffer info. */
			int len_add = snprintf( line+len, linelen-len
			,	" [%02lu:%02lu%c%02lu]"
			,	times[2][0], times[2][1], timesep[2], times[2][2] );
			if(len_add > 0)
				len += len_add;
		}
		if(len >= 0 && len < linelen)
		{ /* Volume info. */
			int len_add = snprintf( line+len, linelen-len
			,	" %s %03u%c%03u"
			,	rva_statname[param->rva]
			,	roundui(basevol*100), muted ? 'm' : '=' 
			,	roundui(realvol*100) );
			if(len_add > 0)
				len += len_add;
		}
		if(len >= 0 && len < linelen)
		{ /* Bitrate. */
			int len_add = snprintf( line+len, linelen-len
			,	" %3d kb/s", mi.bitrate );
			if(len_add > 0)
				len += len_add;
		}
		if(len >= 0 && len < linelen)
		{ /* Size of frame in bytes. */
			int len_add = snprintf( line+len, linelen-len
			,	" %4d B", mi.framesize );
			if(len_add > 0)
				len += len_add;
		}
		if(len >= 0 && len < linelen)
		{ /* Size of frame in bytes. */
			int len_add = 0;
			long res = 0;
			if(mpg123_getstate(fr, MPG123_ACCURATE, &res, NULL) == MPG123_OK)
				len_add = snprintf( line+len, linelen-len
				,	" %s", res ? "acc" : "fuz" );
			if(len_add > 0)
				len += len_add;
		}
		if(len >= 0 && len < linelen)
		{ /* Size of frame in bytes. */
			int len_add = 0;
			long res = mpg123_clip(fr);
			if(res >= 0)
				len_add = snprintf( line+len, linelen-len
				,	" %4ld clip", res );
			if(len_add > 0)
				len += len_add;
		}
		if(len >= 0 && len < linelen)
		{ /* Size of frame in bytes. */
			int len_add = 0;
			len_add = snprintf( line+len, linelen-len
			,	" p%+.3f", param->pitch );
			if(len_add > 0)
				len += len_add;
		}
		if(len >= 0)
		{
			if(maxlen > 0 && len > maxlen)
			{
				/* Emergency cut to avoid terminal scrolling. */
				int i;
				/* Blank a word that would have been cut off. */
				for(i=maxlen; i>=0; --i)
				{
					char old = line[i];
					line[i] = ' ';
					if(old == ' ')
						break;
				}
				line[maxlen] = 0;
				len = maxlen;
			}
			/* Ensure that it is filled with spaces if we got some line length.
			   Shouldn't we always fill to maxlen? */
			if(maxlen > 0)
				memset(line+len, ' ', linelen-len);
			draw_bar = draw_bar && term_have_fun(STDERR_FILENO,param->term_visual);
			/* Use inverse color to draw a progress bar. */
			if(maxlen > 0 && draw_bar)
			{
				char old;
				int barlen = 0;
				if(length > 0 && elapsed > 0)
				{
					if(elapsed < length)
						barlen = (int)((double)elapsed/length * maxlen);
					else
						barlen = maxlen;
				}
				old = line[barlen];
				fprintf(stderr, "\x1b[7m");
				line[barlen] = 0;
				fprintf(stderr, "\r%s", line);
				line[barlen] = old;
				fprintf(stderr, "\x1b[0m");
				fprintf(stderr, "%s", line+barlen);
			}
			else
			fprintf(stderr, "\r%s", line);
		}
	}
	if(line && line != linebuf)
		free(line);
}

void clear_stat(void)
{
	int len = term_width(STDERR_FILENO);
	if(len > 0)
	{
		char fmt[20];
		int flen;
		if( (flen=snprintf(fmt, sizeof(fmt), "\r%%%ds\r", len)) > 0
		  && flen < sizeof(fmt) )
			fprintf(stderr, fmt, " ");
	}
}
