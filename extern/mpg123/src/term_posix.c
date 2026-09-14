/*
	term_posix: POSIX-specifc terminal functionality

	HAVE_TERMIOS is a prerequisite.

	copyright 2008-2023 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis
*/

// ctermid
#define _XOPEN_SOURCE 600
#define _POSIX_C_SOURCE 200112L

#include "config.h"

#ifdef __OS2__
// Hoping for properly working termios in some future (?!), but until then,
// we need keyboard access bypassing that.
#define INCL_KBD
#define INCL_DOSPROCESS
#include <os2.h>
#endif

#include "compat/compat.h"

#ifndef HAVE_TERMIOS
#error "No TERMIOS? Here?"
#endif

// for param struct
#include "mpg123app.h"

#include <termios.h>
#include <sys/ioctl.h>

#include "terms.h"

#include "common/debug.h"

static int term_is_fun = -1;
// This now always refers to a ;freshly opened terminal descriptor (e.g. /dev/tty).
// Printouts to stderr are independent of this.
static int term_fd = -1;
static struct termios old_tio;

/* Buffered key from a signal or whatnot.
   We ignore the null character... */
static char prekey = 0;

int term_have_fun(int fd, int want_visuals)
{
	if(term_is_fun > -1)
		return term_is_fun;
	else
		term_is_fun = 0;
	if(term_width(fd) > 0 && want_visuals)
	{
		/* Only play with non-dumb terminals. */
		char *tname = INT123_compat_getenv("TERM");
		if(tname)
		{
			if(strcmp(tname, "") && strcmp(tname, "dumb"))
				term_is_fun = 1;
			free(tname);
		}
	}
	return term_is_fun;
}

/* Also serves as a way to detect if we have an interactive terminal. */
int term_width(int fd)
{
#ifdef __OS2__
	int s[2];
	_scrsize (s);
	// It seems like we cannot really use the last character of the
	// term and have to stop one short to avoid advancing a line.
	if (s[0] >= 0)
		return s[0] - 1;
#else
	struct winsize geometry;
	geometry.ws_col = 0;
	if(ioctl(fd, TIOCGWINSZ, &geometry) >= 0)
		return (int)geometry.ws_col;
#endif
	return -1;
}

static int term_setup_detail(struct termios *pattern);

static void term_sigcont(int sig)
{
	if(term_setup_detail(&old_tio) < 0)
	{
		debug("Can't set terminal attributes");
		return;
	}
}

static void term_sigusr(int sig)
{
	switch(sig)
	{
		case SIGUSR1: prekey=*param.term_usr1; break;
		case SIGUSR2: prekey=*param.term_usr2; break;
	}
}

/* This must call only functions safe inside a signal handler. */
static int term_setup_detail(struct termios *pattern)
{
	mdebug("setup on fd %d", term_fd);

	INT123_catchsignal(SIGCONT, term_sigcont);
	INT123_catchsignal(SIGUSR1, term_sigusr);
	INT123_catchsignal(SIGUSR2, term_sigusr);

	struct termios tio = *pattern;
	tio.c_lflag &= ~(ICANON|ECHO); 
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;
#ifdef __OS2__
	// Do not care for the error until OS/2 is known to work.
	tcsetattr(term_fd,TCSANOW,&tio);
	return 0;
#else
	return tcsetattr(term_fd,TCSANOW,&tio);
#endif
}

int term_setup(void)
{
	if(term_fd < 0)
	{
		const char *term_name;
#ifdef HAVE_CTERMID
		term_name = ctermid(NULL);
#else
		term_name = "/dev/tty";
#endif
		if(term_name)
			mdebug("accessing terminal for control via %s", term_name);
		else
		{
			error("no controlling terminal");
			return -1;
		}
		term_fd = open(term_name, O_RDONLY);
		if(term_fd < 0)
		{
			merror("failed to open terminal: %s", INT123_strerror(errno));
			return -1;
		}
	}

	if(tcgetattr(term_fd, &old_tio) < 0)
	{
		// For now, this always fails on OS/2, but they might fix things.
		// So just try to move on.
#ifndef __OS2__
		merror("failed to get terminal attributes: %s", INT123_strerror(errno));
		return -1;
#endif
	}

	errno = 0;
	if(term_setup_detail(&old_tio) < 0)
	{
		close(term_fd);
		term_fd = -1;
		if(errno)
			merror("failure setting terminal attributes: %s", INT123_strerror(errno));
		else
			error("failure setting terminal attributes");
		return -1;
	}
	return 0;
}

/* Get the next pressed key, if any.
   Returns 1 when there is a key, 0 if not. */
int term_get_key(int stopped, int do_delay, char *val)
{
#ifdef __OS2__
	KBDKEYINFO key;
	key.chChar = 0;
	key.chScan = 0;
	if(do_delay)
		DosSleep(10);
	if(!KbdCharIn(&key,(stopped) ? IO_WAIT : IO_NOWAIT,0) && key.chChar)
	{
		*val = key.chChar;
		return 1;
	}
#else
	fd_set r;
	struct timeval t;

	/* Shortcut: If some other means sent a key, use it. */
	if(prekey)
	{
		debug1("Got prekey: %c\n", prekey);
		*val = prekey;
		prekey = 0;
		return 1;
	}

	t.tv_sec=0;
	t.tv_usec=(do_delay) ? 10*1000 : 0;

	FD_ZERO(&r);
	FD_SET(term_fd,&r);
	/* No timeout if stopped */
	if(select(term_fd+1,&r,NULL,NULL,(stopped) ? NULL : &t) > 0 && FD_ISSET(term_fd,&r))
	{
		if(read(term_fd,val,1) <= 0)
		return 0; /* Well, we couldn't read the key, so there is none. */
		else
		return 1;
	}
#endif
	else return 0;
}

void term_restore(void)
{
	debug("reset attrbutes");
	tcsetattr(term_fd,TCSAFLUSH,&old_tio);

	if(term_fd > -1)
		close(term_fd);
	term_fd = -1;
}
