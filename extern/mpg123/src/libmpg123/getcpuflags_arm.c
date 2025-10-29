/*
	getcpuflags_arm: get cpuflags for ARM

	copyright 1995-2024 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Taihei Momma
*/

// For sigsetjmp, we need POSIX 2001
#define _POSIX_C_SOURCE 200112L
// For SA_RESTART, XSI is needed (or POSIX 2008).
// This here should include the above, keeping that for clarity.
#define _XOPEN_SOURCE 600

#include <setjmp.h>
#include <signal.h>
#include "mpg123lib_intern.h"
#include "getcpuflags.h"

extern void INT123_check_neon(void);

#ifndef _M_ARM
static sigjmp_buf jmpbuf;
#else
static jmp_buf jmpbuf;
#endif

static void mpg123_arm_catch_sigill(int sig)
{
#ifndef _M_ARM
	siglongjmp(jmpbuf, 1);
#else
	longjmp(jmpbuf, 1);
#endif
}

unsigned int INT123_getcpuflags(struct cpuflags* cf)
{
#ifndef _M_ARM
	struct sigaction act, act_old;
	act.sa_handler = mpg123_arm_catch_sigill;
	act.sa_flags = SA_RESTART;
	sigemptyset(&act.sa_mask);
	sigaction(SIGILL, &act, &act_old);
	
	cf->has_neon = 0;
	
	if(!sigsetjmp(jmpbuf, 1)) {
		INT123_check_neon();
		cf->has_neon = 1;
	}
	
	sigaction(SIGILL, &act_old, NULL);
#else
	cf->has_neon = 0;

	if (!setjmp(jmpbuf)) {
		signal(SIGILL, mpg123_arm_catch_sigill);
		INT123_check_neon();
		cf->has_neon = 1;
	}

	signal(SIGILL, SIG_DFL);
#endif
	
	return 0;
}
