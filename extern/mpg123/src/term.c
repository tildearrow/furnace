/*
	term: terminal control

	copyright ?-2023 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Michael Hipp
*/

#include "mpg123app.h"
#include <ctype.h>

#include "term.h"
#include "terms.h"
#include "common.h"
#include "playlist.h"
#include "metaprint.h"
#include "common/debug.h"

static int term_enable = 0;
static const char *extrabreak = "";
static int seeking = FALSE;

extern out123_handle *ao;

static const int helplen = 18;
#define HELPFMT "%-18s"

/* Hm, next step would be some system in this, plus configurability...
   Two keys for everything? It's just stop/pause for now... */
struct keydef { const char key; const char key2; const char* desc; };
struct keydef term_help[] =
{
	 { MPG123_STOP_KEY,  ' ', "(un)pause playback" }
	,{ MPG123_NEXT_KEY,    0, "next track" }
	,{ MPG123_PREV_KEY,    0, "previous track" }
	,{ MPG123_NEXT_DIR_KEY, 0, "next directory" }
	,{ MPG123_PREV_DIR_KEY, 0, "previous directory" }
	,{ MPG123_BACK_KEY,    0, "back to beginning" }
	,{ MPG123_LOOP_KEY,    0, "A-B loop" }
	,{ MPG123_PAUSE_KEY,   0, "preset loop" }
	,{ MPG123_FORWARD_KEY, 0, "forward" }
	,{ MPG123_REWIND_KEY,  0, "rewind" }
	,{ MPG123_FAST_FORWARD_KEY, 0, "fast forward" }
	,{ MPG123_FAST_REWIND_KEY,  0, "fast rewind" }
	,{ MPG123_FINE_FORWARD_KEY, 0, "fine forward" }
	,{ MPG123_FINE_REWIND_KEY,  0, "fine rewind" }
	,{ MPG123_VOL_UP_KEY,   0, "volume up" }
	,{ MPG123_VOL_DOWN_KEY, 0, "volume down" }
	,{ MPG123_VOL_MUTE_KEY, 0, "(un)mute volume" }
	,{ MPG123_RVA_KEY,      0, "cycle RVA modes" }
	,{ MPG123_VERBOSE_KEY,  0, "cycle verbosity" }
	,{ MPG123_PLAYLIST_KEY, 0, "show playlist" }
	,{ MPG123_TAG_KEY,      0, "tag info" }
	,{ MPG123_MPEG_KEY,     0, "MPEG header info" }
	,{ MPG123_PITCH_UP_KEY, MPG123_PITCH_BUP_KEY, "pitch up + ++" }
	,{ MPG123_PITCH_DOWN_KEY, MPG123_PITCH_BDOWN_KEY, "pitch down - --" }
	,{ MPG123_PITCH_ZERO_KEY, 0, "zero pitch" }
	,{ MPG123_BOOKMARK_KEY, 0, "print bookmark" }
	,{ MPG123_HELP_KEY,     0, "this help" }
	,{ MPG123_QUIT_KEY,     0, "quit" }
	,{ MPG123_EQ_RESET_KEY,    0, "flat equalizer" }
	,{ MPG123_EQ_SHOW_KEY,     0, "show equalizer" }
	,{ MPG123_BASS_UP_KEY,     0, "more bass" }
	,{ MPG123_BASS_DOWN_KEY,   0, "less bass" }
	,{ MPG123_MID_UP_KEY,      0, "more mids" }
	,{ MPG123_MID_DOWN_KEY,    0, "less mids" }
	,{ MPG123_TREBLE_UP_KEY,   0, "more treble" }
	,{ MPG123_TREBLE_DOWN_KEY, 0, "less treble" }
};

/* initialze terminal */
int term_init(void)
{
	const char hide_cursor[] = "\x1b[?25l";
	debug("term_init");

	if(term_have_fun(STDERR_FILENO, param.term_visual))
		fprintf(stderr, "%s", hide_cursor);

	if(param.verbose)
		extrabreak = "\n";
	debug1("param.term_ctrl: %i", param.term_ctrl);
	if(!param.term_ctrl)
		return 0;

	term_enable = 0;
	errno = 0;
	if(term_setup() < 0)
	{
		if(errno)
			merror("failed to set up terminal: %s", INT123_strerror(errno));
		else
			error("failed to set up terminal");
		return -1;
	}
	term_enable = 1;
	return 0;
}

void term_hint(void)
{
	if(term_enable)
		fprintf(stderr, "\nTerminal control enabled, press 'h' for listing of keys and functions.\n\n");
}

static void term_handle_input(mpg123_handle *, out123_handle *, int);

// A-B looping sets pause_cycle at runtime baseon the difference to
// pause_begin. Keeping the broken wording for 'pause' for now. To the
// outside world, it is 'looping'.
static double pause_begin = -1;
static off_t pause_cycle;

static int print_index(mpg123_handle *mh)
{
	int err;
	size_t c, fill;
	off_t *index;
	off_t  step;
	err = mpg123_index(mh, &index, &step, &fill);
	if(err == MPG123_ERR)
	{
		fprintf(stderr, "Error accessing frame index: %s\n", mpg123_strerror(mh));
		return err;
	}
	for(c=0; c < fill;++c) 
		fprintf(stderr, "[%lu] %lu: %li (+%li)\n",
		(unsigned long) c,
		(unsigned long) (c*step), 
		(long) index[c], 
		(long) (c ? index[c]-index[c-1] : 0));
	return MPG123_OK;
}

static off_t offset = 0;

void term_new_track(void)
{
	playstate = STATE_PLAYING;
	pause_begin = -1;
}

/* Go back to the start for the cyclic pausing. */
void pause_recycle(mpg123_handle *fr)
{
	/* Take care not to go backwards in time in steps of 1 frame
		 That is what the +1 is for. */
	pause_cycle=(off_t)(param.pauseloop/mpg123_tpf(fr));
	offset-=pause_cycle;
}

off_t term_control(mpg123_handle *fr, out123_handle *ao)
{
	offset = 0;
	debug2("control for frame: %" PRIiMAX ", enable: %i", (intmax_t)mpg123_tellframe(fr), term_enable);
	if(!term_enable) return 0;

	if(playstate==STATE_LOOPING)
	{
		/* pause_cycle counts the remaining frames _after_ this one, thus <0, not ==0 . */
		if(--pause_cycle < 0)
			pause_recycle(fr);
	}

	do
	{
		off_t old_offset = offset;
		term_handle_input(fr, ao, seeking);
		if((offset < 0) && (-offset > framenum)) offset = - framenum;
		if(param.verbose && offset != old_offset)
			print_stat(fr,offset,ao,1,&param);
	} while (!intflag && playstate==STATE_STOPPED);

	/* Make the seeking experience with buffer less annoying.
	   No sound during seek, but at least it is possible to go backwards. */
	if(offset)
	{
		if((offset = mpg123_seek_frame(fr, offset, SEEK_CUR)) >= 0)
		debug1("seeked to %" PRIiMAX, (intmax_t)offset);
		else error1("seek failed: %s!", mpg123_strerror(fr));
		/* Buffer resync already happened on un-stop? */
		/* if(param.usebuffer) audio_drop(ao);*/
	}
	return 0;
}

/* Stop playback while seeking if buffer is involved. */
static void seekmode(mpg123_handle *mh, out123_handle *ao)
{
	if(param.usebuffer && playstate!=STATE_STOPPED)
	{
		int channels = 0;
		int encoding = 0;
		int pcmframe;
		off_t back_samples = 0;

		playstate = STATE_STOPPED;
		out123_pause(ao);
		if(param.verbose)
			print_stat(mh, 0, ao, 0, &param);
		mpg123_getformat(mh, NULL, &channels, &encoding);
		pcmframe = out123_encsize(encoding)*channels;
		if(pcmframe > 0)
			back_samples = out123_buffered(ao)/pcmframe;
		if(param.verbose > 2)
			fprintf(stderr, "\nseeking back %" PRIiMAX " samples from %" PRIiMAX "\n"
			, (intmax_t)back_samples, (intmax_t)mpg123_tell(mh));
		mpg123_seek(mh, -back_samples, SEEK_CUR);
		out123_drop(ao);
		if(param.verbose > 2)
			fprintf(stderr, "\ndropped, now at %" PRIiMAX "\n"
			,	(intmax_t)mpg123_tell(mh));
		fprintf(stderr, "%s", MPG123_STOPPED_STRING);
		if(param.verbose)
			print_stat(mh, 0, ao, 1, &param);
	}
}

static void print_term_help(struct keydef *def)
{
	if(def->key2)
	{
		if(isspace(def->key2))
			fprintf(stderr, "%c '%c'", def->key, def->key2);
		else
			fprintf(stderr, "%c  %c ", def->key, def->key2);
	}
	else fprintf(stderr, "%c    ", def->key);

	fprintf(stderr, " " HELPFMT, def->desc);
}

static void term_handle_key(mpg123_handle *fr, out123_handle *ao, char val)
{
	debug1("term_handle_key: %c", val);
	switch(val)
	{
	case MPG123_BACK_KEY:
		out123_pause(ao);
		out123_drop(ao);
		// Revisit: What does that really achieve?
		if(playstate==STATE_LOOPING)
			pause_cycle=(int)(param.pauseloop/mpg123_tpf(fr));

		if(mpg123_seek_frame(fr, 0, SEEK_SET) < 0)
		error1("Seek to begin failed: %s", mpg123_strerror(fr));

		framenum=0;
	break;
	case MPG123_NEXT_KEY:
		out123_pause(ao);
		out123_drop(ao);
		next_track();
	break;
	case MPG123_NEXT_DIR_KEY:
		out123_pause(ao);
		out123_drop(ao);
		next_dir();
	break;
	case MPG123_QUIT_KEY:
		debug("QUIT");
		if(playstate==STATE_STOPPED)
		{
			if(param.verbose)
				print_stat(fr,0,ao,0,&param);

			playstate=STATE_PLAYING; // really necessary/sensible?
			out123_pause(ao); /* no chance for annoying underrun warnings */
			out123_drop(ao);
		}
		set_intflag();
		offset = 0;
	break;
	case MPG123_LOOP_KEY:
	// In paused (looping) state, the loop key ends the loop just like the other one.
	// Otherwise, it starts playback and enters A-? mode. If in A-? mode, it
	// sets the loop interval and then again falls through.
	if(playstate != STATE_LOOPING)
	{
		playstate = STATE_AB;
		// Careful with positioning, output might have 
		long outrate = 0;
		int outframesize = 0;
		long inrate = 0;
		if(out123_getformat(ao, &outrate, NULL, NULL, &outframesize) || outrate==0)
			break;
		if(mpg123_getformat(fr, &inrate, NULL, NULL) || inrate==0)
			break;

		double position = (double)mpg123_tell(fr)/inrate + (double)out123_buffered(ao)/(outrate * outframesize);
		if(pause_begin < 0)
		{
			pause_begin = position;
			if(param.verbose)
				print_stat(fr, 0, ao, 1, &param);
			else
				fprintf(stderr, "%s", MPG123_AB_STRING);
			break;
		} else if(position <= pause_begin)
		{
			// Pathological situation: You seeked around, whatever. No loop.
			playstate=STATE_LOOPING; // Let fall-through fix up things.
		}
		{
			param.pauseloop = (position > pause_begin) ? (position-pause_begin) : mpg123_tpf(fr);
			// Fall throuth to start looping.
		}
	}
	case MPG123_PAUSE_KEY:
	{
		playstate = playstate == STATE_LOOPING ? STATE_PLAYING : STATE_LOOPING;
		pause_begin = -1;
		size_t buffered = out123_buffered(ao);
		out123_pause(ao); /* underrun awareness */
		out123_drop(ao);
		if(playstate == STATE_LOOPING)
		{
			// Make output buffer react immediately, dropping decoded audio
			// and (at least trying to) seeking back in input.
			out123_param_float(ao, OUT123_PRELOAD, 0.);
			if(buffered)
			{
				int framesize = 1;
				if(!out123_getformat(ao, NULL, NULL, NULL, &framesize))
				{
					buffered /= framesize;
					mpg123_seek(fr, -buffered, SEEK_CUR);
				}
			}
			pause_recycle(fr);
		}
		else
			out123_param_float(ao, OUT123_PRELOAD, param.preload);
		if(param.verbose)
			print_stat(fr, 0, ao, 1, &param);
		else
			fprintf(stderr, "%s", (playstate == STATE_LOOPING) ? MPG123_PAUSED_STRING : MPG123_EMPTY_STRING);
	}
	break;
	case MPG123_STOP_KEY:
	case ' ':
		/* TODO: Verify/ensure that there is no "chirp from the past" when
		   seeking while stopped. */
		if(playstate == STATE_LOOPING)
			offset -= pause_cycle;
		playstate = playstate == STATE_STOPPED ? STATE_PLAYING : STATE_STOPPED;
		if(playstate == STATE_STOPPED)
			out123_pause(ao);
		else
		{
			if(offset) /* If position changed, old is outdated. */
				out123_drop(ao);
			/* No out123_continue(), that's triggered by out123_play(). */
		}
		if(param.verbose)
			print_stat(fr, 0, ao, 1, &param);
		else
			fprintf(stderr, "%s", (playstate==STATE_STOPPED) ? MPG123_STOPPED_STRING : MPG123_EMPTY_STRING);
	break;
	case MPG123_FINE_REWIND_KEY:
		seekmode(fr, ao);
		offset--;
	break;
	case MPG123_FINE_FORWARD_KEY:
		seekmode(fr, ao);
		offset++;
	break;
	case MPG123_REWIND_KEY:
		seekmode(fr, ao);
		  offset-=10;
	break;
	case MPG123_FORWARD_KEY:
		seekmode(fr, ao);
		offset+=10;
	break;
	case MPG123_FAST_REWIND_KEY:
		seekmode(fr, ao);
		offset-=50;
	break;
	case MPG123_FAST_FORWARD_KEY:
		seekmode(fr, ao);
		offset+=50;
	break;
	case MPG123_VOL_UP_KEY:
		mpg123_volume_change_db(fr, +1);
	break;
	case MPG123_VOL_DOWN_KEY:
		mpg123_volume_change_db(fr, -1);
	break;
	case MPG123_VOL_MUTE_KEY:
		set_mute(ao, muted=!muted);
	break;
	case MPG123_EQ_RESET_KEY:
		mpg123_reset_eq(fr);
	break;
	case MPG123_EQ_SHOW_KEY:
	{
		if(param.verbose)
			print_stat(fr,0,ao,0,&param);
		// Assuming only changes happen via terminal control, these 3 values
		// are what counts.
		fprintf( stderr, "%s\nbass:   %.3f\nmid:    %.3f\ntreble: %.3f\n\n"
		,	extrabreak
		,	mpg123_geteq(fr, MPG123_LEFT, 0)
		,	mpg123_geteq(fr, MPG123_LEFT, 1)
		,	mpg123_geteq(fr, MPG123_LEFT, 2)
		);
	}
	break;
	case MPG123_BASS_UP_KEY:
		mpg123_eq_change(fr, MPG123_LR, 0, 0, +1);
	break;
	case MPG123_BASS_DOWN_KEY:
		mpg123_eq_change(fr, MPG123_LR, 0, 0, -1);
	break;
	case MPG123_MID_UP_KEY:
		mpg123_eq_change(fr, MPG123_LR, 1, 1, +1);
	break;
	case MPG123_MID_DOWN_KEY:
		mpg123_eq_change(fr, MPG123_LR, 1, 1, -1);
	break;
	case MPG123_TREBLE_UP_KEY:
		mpg123_eq_change(fr, MPG123_LR, 2, 31, +1);
	break;
	case MPG123_TREBLE_DOWN_KEY:
		mpg123_eq_change(fr, MPG123_LR, 2, 31, -1);
	break;
	case MPG123_PITCH_UP_KEY:
	case MPG123_PITCH_BUP_KEY:
	case MPG123_PITCH_DOWN_KEY:
	case MPG123_PITCH_BDOWN_KEY:
	case MPG123_PITCH_ZERO_KEY:
	{
		double new_pitch = param.pitch;
		switch(val) /* Not tolower here! */
		{
			case MPG123_PITCH_UP_KEY:    new_pitch += MPG123_PITCH_VAL;  break;
			case MPG123_PITCH_BUP_KEY:   new_pitch += MPG123_PITCH_BVAL; break;
			case MPG123_PITCH_DOWN_KEY:  new_pitch -= MPG123_PITCH_VAL;  break;
			case MPG123_PITCH_BDOWN_KEY: new_pitch -= MPG123_PITCH_BVAL; break;
			case MPG123_PITCH_ZERO_KEY:  new_pitch = 0.0; break;
		}
		if(param.verbose)
			print_stat(fr,0,ao,0,&param);
		set_pitch(fr, ao, new_pitch);
		if(param.verbose > 1)
			fprintf(stderr, "\nNew pitch: %f\n", param.pitch);
		if(param.verbose)
			print_stat(fr,0,ao,1,&param);
	}
	break;
	case MPG123_VERBOSE_KEY:
		param.verbose++;
		if(param.verbose > VERBOSE_MAX)
		{
			param.verbose = 0;
			clear_stat();
			extrabreak = "";
		} else
			extrabreak = "\n";
		mpg123_param(fr, MPG123_VERBOSE, param.verbose, 0);
	break;
	case MPG123_RVA_KEY:
		if(++param.rva > MPG123_RVA_MAX) param.rva = 0;
		mpg123_param(fr, MPG123_RVA, param.rva, 0);
		mpg123_volume_change(fr, 0.);
		if(param.verbose)
			print_stat(fr,0,ao,1,&param);
	break;
	case MPG123_PREV_KEY:
		out123_pause(ao);
		out123_drop(ao);

		prev_track();
	break;
	case MPG123_PREV_DIR_KEY:
		out123_pause(ao);
		out123_drop(ao);
		prev_dir();
	break;
	case MPG123_PLAYLIST_KEY:
		if(param.verbose)
			print_stat(fr,0,ao,0,&param);
		fprintf(stderr, "%s\nPlaylist (\">\" indicates current track):\n", param.verbose ? "\n" : "");
		print_playlist(stderr, 1);
		fprintf(stderr, "\n");
	break;
	case MPG123_TAG_KEY:
		if(param.verbose)
			print_stat(fr,0,ao,0,&param);
		fprintf(stderr, "%s", extrabreak);
		print_id3_tag(fr, param.long_id3, stderr, term_width(STDERR_FILENO));
	break;
	case MPG123_MPEG_KEY:
		if(param.verbose)
			print_stat(fr,0,ao,0,&param);
		fprintf(stderr, "%s", extrabreak);
		if(param.verbose > 1)
			print_header(fr);
		else
			print_header_compact(fr);
	break;
	case MPG123_HELP_KEY:
	{ /* This is more than the one-liner before, but it's less spaghetti. */
		int i;
		if(param.verbose)
			print_stat(fr,0,ao,0,&param);
		fprintf(stderr,"%s\n -= terminal control keys =-\n\n", extrabreak);
		int linelen = term_width(STDERR_FILENO);
		int colwidth = helplen+6;
		int columns = linelen > colwidth ? ((linelen+2)/(colwidth+2)) : 1;
		int j = 0;
		for(i=0; i<(sizeof(term_help)/sizeof(struct keydef)); ++i)
		{
			if(j)
				fprintf(stderr, "  ");
			print_term_help(term_help+i);
			j = (j+1)%columns;
			if(!j)
				fprintf(stderr, "\n");
		}
		fprintf(stderr, "\n\nNumber row jumps in 10%% steps.\n\n");
	}
	break;
	case MPG123_FRAME_INDEX_KEY:
	case MPG123_VARIOUS_INFO_KEY:
		if(param.verbose)
		{
			print_stat(fr,0,ao,0,&param);
			fprintf(stderr, "\n");
		}
		switch(val) /* because of tolower() ... */
		{
			case MPG123_FRAME_INDEX_KEY:
			print_index(fr);
			{
				long accurate;
				if(mpg123_getstate(fr, MPG123_ACCURATE, &accurate, NULL) == MPG123_OK)
				fprintf(stderr, "Accurate position: %s\n", (accurate == 0 ? "no" : "yes"));
				else
				error1("Unable to get state: %s", mpg123_strerror(fr));
			}
			break;
			case MPG123_VARIOUS_INFO_KEY:
			{
				const char* curdec = mpg123_current_decoder(fr);
				if(curdec == NULL) fprintf(stderr, "Cannot get decoder info!\n");
				else fprintf(stderr, "Active decoder: %s\n", curdec);
			}
		}
	break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{
		off_t len;
		int num;
		num = val == '0' ? 10 : val - '0';
		--num; /* from 0 to 9 */

		/* Do not swith to seekmode() here, as we are jumping once to a
		   specific position. Dropping buffer contents is enough and there
		   is no race filling the buffer or waiting for more incremental
		   seek orders. */
		len = mpg123_length(fr);
		out123_pause(ao);
		out123_drop(ao);
		if(len > 0)
			mpg123_seek(fr, (off_t)( (num/10.)*len ), SEEK_SET);
		else
			error("Not seeking as track length cannot be determined.");
	}
	break;
	case MPG123_BOOKMARK_KEY:
		continue_msg("BOOKMARK");
	break;
	default:
		;
	}
}

static void term_handle_input(mpg123_handle *fr, out123_handle *ao, int do_delay)
{
	char val;
	if(term_get_key(playstate==STATE_STOPPED, do_delay, &val))
	{
		term_handle_key(fr, ao, val);
	}
}

void term_exit(void)
{
	mdebug("term_enable=%i", term_enable);
	const char cursor_restore[] = "\x1b[?25h";
	/* Bring cursor back. */
	if(term_have_fun(STDERR_FILENO, param.term_visual))
		fprintf(stderr, "%s", cursor_restore);

	if(!term_enable) return;

	term_restore();
}
