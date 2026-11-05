#ifndef H_LOCAL
#define H_LOCAL
/*
	local: some stuff for localisation

	Currently, this is just about determining if we got UTF-8 locale and
	checking output terminal properties.

	copyright 2008-2020 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis, based on a patch by Thorsten Glaser.

	This got extended for the distinciton between a vague wish for UTF-8 in/output
	encoding from environment variables (utf8env) and a system actually being
	configured to correcty work with it (utf8loc). The configured locale is needed
	to enable proper (?) length computation and control character filtering.

	The fallback shall be either dumb UTF-8 with just filtering C0 and C1 control
	characters, or even full-on stripping to 7-bit ASCII.
*/

/* Pulled in by mpg123app.h! */

/* Set this to enforce UTF-8 output (utf8env will be 1, but utf8loc may not) */
extern int utf8force;

/* This is 1 if check_locale found some UTF-8 hint, 0 if not. */
extern int utf8env;

/* This is 1 if check_locale configured a locale with UTF-8 charset */
extern int utf8loc;

/* Check/set locale, set the utf8env variable.
   After calling this, a locale for LC_CTYPE should be configured,
   but one that is based on UTF-8 only if utf8loc is set. */
void check_locale(void);

/* Return non-zero if full terminal fun is desired/possible. */
int term_have_fun(int fd, int want_visuals);

/* Return width of terminal associated with given descriptor,
   -1 when there is none. */
int term_width(int fd);

// Prepare a string that is safe and sensible to print to the console.
// Input: UTF-8 or 7-bit ASCII string
// Output: sanitized output string for UTF-8 or ASCII, control characters
//   dropped (including line breaks)
// Modifer: Output is destined for a terminal and non-printing characters
// shall be filtered out. Otherwise, things are just adjusted for non-utf8
// locale.
// Return value: An estimate of the printed string width. In an UTF-8
// locale, this should often be right, but there never is a guarantee
// with Unicode. If the whole operation failed, dest will be NULL.
size_t utf8outstr(char **dest, const char *source, int to_terminal);
// Take an unspecified (assumed ASCII-based) encoding and at least
// construct valid UTF-8 with 7-bit ASCII and replacement characters.
// This looses C1 control characters.
// If count is >= 0, it is used instead of strlen(source), enabling
// processing of data without closing zero byte.
// Returns 0 if all went well, as do the others following.
// On error, the dest memory is freed and the pointer nulled.
int unknown2utf8(char **dest, const char *source, int count);
// Wrapper around the above for printing the string to some stream.
// Return value is directly from fprintf or also -1 if there was trouble
// processing the string.
// If the output is a terminal as indicated by is_terminal, this applies
// all the filtering to avoid non-print characters, non-utf8 stuff is reduced
// to safe ASCII. If not printing to a terminal, only utf8 is reduced for
// non-utf8-locales. Without explicit is_utf8, the string is assumed to
// match the current environment. So it's taken as UTF-8 for UTF-8 environment
// and something ASCII-based otherwise (triggering ASCII C0 and C1 filtering).
int outstr(char **dest, const char *str, int is_utf8, int is_terminal);
int print_outstr(FILE *out, const char *str, int is_utf8, int is_terminal);

#endif
