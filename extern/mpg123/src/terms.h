#ifndef H_TERMS
#define H_TERMS
/*
	terms: terminal specifics

	This is a very lightweight terminal library, just the minimum to

	- get at the width of the terminal (if there is one)
	- be able to read single keys being pressed for control
	- maybe also switch off echoing of input

	copyright 2008-2022 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis and Jonathan Yong
*/

/* These two functions query terminal properties based on a terminal
   being connected to the specified file descriptor (either STDIN_FILENO
   or STDERR_FILENO). */

/* Return non-zero if full terminal fun is desired/possible. */
int term_have_fun(int fd, int want_visuals);

/* Return width of terminal associated with given descriptor,
   -1 when there is none. */
int term_width(int fd);

/*
	This is for more serious work with the terminal: It is sensible to
	open some internal handle and continue to operate on that on subsequent
	calls.
*/

/** Setup terminal for control work (things like disabling echo and
 *  buffering. You handle your handles internally.
 *  \return 0 on suceess, -1 on trouble
 */
int term_setup(void);

/** Restore terminal properties to what they were before we messed
 *  around. Failure is not an option.
 */
void term_restore(void);

/** Check for and return a key press event.
 *  \param do_delay Wait for up to 10 ms for a key event if true.
 *  \param val address to store character to
 *  \return 1 if there is a key, 0 if not
 */
int term_get_key(int stopped, int do_delay, char *val);

#endif
