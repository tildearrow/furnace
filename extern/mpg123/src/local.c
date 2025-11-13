	/*
	local: some stuff for localisation, safe terminal printout

	This is about determining if we got UTF-8 locale and
	checking output terminal properties, along with subsequent string
	transformations for safe printout.

	copyright 2008-2021 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis, based on a patch by Thorsten Glaser.
*/

// wchar stuff
#define _XOPEN_SOURCE 600
#define _POSIX_C_SOURCE 200112L

#include "config.h"

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#ifdef HAVE_LANGINFO_H
#include <langinfo.h>
#endif
#include "compat/compat.h"

#include "local.h"

#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <wincon.h>
#endif

#include "common/debug.h"

int utf8force = 0; // enforce UTF-8 workings
int utf8env = 0; // produce UTF-8 text output
int utf8loc = 0; // have actual UTF-8 locale (so that mbstowcs() works)

//static int term_is_fun = -1;

static const char joker_symbol = '?';
static const char *uni_repl = "\xef\xbf\xbd";
static const int uni_repl_len = 3;

/* Check some language variable for UTF-8-ness. */
static int is_utf8(const char *lang);

void check_locale(void)
{
	if(utf8force)
		utf8env = 1;
	else
	{
		const char *cp;

		/* Check for env vars in proper oder. */
		if((cp = getenv("LC_ALL")) == NULL && (cp = getenv("LC_CTYPE")) == NULL)
		cp = getenv("LANG");

		if(is_utf8(cp))
			utf8env = 1;
	}

#if defined(HAVE_SETLOCALE) && defined(LC_CTYPE)
	/* To query, we need to set from environment... */
	if(
		   is_utf8(setlocale(LC_CTYPE, ""))
		// If enforced, try to set an UTF-8 locale that hopefully exists.
		|| (utf8force && is_utf8(setlocale(LC_CTYPE, "C.UTF-8")))
		|| (utf8force && is_utf8(setlocale(LC_CTYPE, "en_US.UTF-8")))
	)
	{
		utf8env = 1;
		utf8loc = 1;
	}
#endif
#if defined(HAVE_NL_LANGINFO) && defined(CODESET)
	/* ...langinfo works after we set a locale, eh? So it makes sense after setlocale, if only. */
	if(is_utf8(nl_langinfo(CODESET)))
	{
		utf8env = 1;
		utf8loc = 1;
	}
#endif

	debug2("UTF-8 env %i: locale: %i", utf8env, utf8loc);
}

static int is_utf8(const char *lang)
{
	if(lang == NULL) return 0;

	/* Now, if the variable mentions UTF-8 anywhere, in some variation, the locale is UTF-8. */
	if(   strstr(lang, "UTF-8") || strstr(lang, "utf-8")
	   || strstr(lang, "UTF8")  || strstr(lang, "utf8")  )
	return 1;
	else
	return 0;
}

// Moved encoding stuff over from metaprint.c and removed references to libmpg123,
// meaning no mpg123_string for you!

int unknown2utf8(char **dest, const char *source, int len)
{
	if(!dest)
		return -1;
	if(!source)
	{
		*dest = INT123_safer_realloc(*dest, 0);
		return -1;
	}
	size_t count = len < 0 ? strlen(source) : (size_t)len;
	// Make a somewhat proper UTF-8 string out of this. Testing for valid
	// UTF-8 is futile. It will be some unspecified legacy 8-bit encoding.
	// I am keeping C0 chars, but replace everything above 7 bits with
	// the Unicode replacement character as most custom 8-bit encodings
	// placed some symbols into the C1 range, we just don't know which.
	size_t ulen = 1; // trailing zero
	for(size_t i=0; i<count; ++i)
	{
		unsigned char c = ((unsigned char*)source)[i];
		if(!c)
			break;
		ulen += c >= 0x80 ? uni_repl_len : 1;
	}

	if(NULL == (*dest = INT123_safer_realloc(*dest, ulen)))
		return -1;

	unsigned char *p = (unsigned char*)*dest;
	for(size_t i=0; i<count; ++i)
	{
		unsigned char c = ((unsigned char*)source)[i];
		if(!c)
			break;
		if(c >= 0x80)
		{
			for(int r=0; r<uni_repl_len; ++r)
				*p++ = uni_repl[r];
		}
		else
			*p++ = c;
	}
	*p = 0;
	return 0;
}

static void ascii_space(unsigned char *c, int *wasspace)
{
	switch(*c)
	{
		case '\f':
		case '\r':
		case '\n':
		case '\t':
		case '\v':
			if(!*wasspace)
				*c = ' '; // Will be dropped by < 0x20 check otherwise.
			*wasspace = 1;
		break;
		default:
			*wasspace = 0;
	}
}

// Filter C1 control chars, using c2lead state.
#define ASCII_C1(c, append) \
	if(c2lead) \
	{ \
		if((c) >= 0x80 && (c) <= 0x9f) \
		{ \
			c2lead = 0; \
			continue; \
		} \
		else \
		{ \
			append; \
		} \
	} \
	c2lead = ((c) == 0xc2); \
	if(c2lead) \
		continue;

// return: strlen+1 of result, 0 on error
// If ret is 0, *dest will be freed and NULL.

static size_t utf8_ascii_work(char **dest_, const char *source
,	int keep_nonprint)
{
	if(!dest_)
		return 0;
	if(!source)
	{
		*dest_ = INT123_safer_realloc(*dest_, 0);
		return 0;
	}

	char *dest = *dest_;
	size_t source_fill = strlen(source)+1;
	size_t spos = 0;
	size_t dlen = 1; // At least a zero.
	unsigned char *p;

	// Find length of ASCII string (count non-continuation bytes).
	// Do _not_ change this to mpg123_strlen()!
	// It needs to match the loop below. 
	// No UTF-8 continuation byte 0x10??????, nor control char.
#define ASCII_PRINT_SOMETHING(c) \
	(((c) & 0xc0) != 0x80 && (keep_nonprint || ((c) != 0x7f && (c) >= 0x20)))
	int c2lead = 0;
	int wasspace = 0;
	for(spos=0; spos < source_fill; ++spos)
	{
		unsigned char c = ((unsigned char*)source)[spos];
		if(!keep_nonprint)
			ascii_space(&c, &wasspace);
		ASCII_C1(c, ++dlen);
		if(ASCII_PRINT_SOMETHING(c))
			++dlen;
	}
	// Do nothing with nothing or if allocation fails. Neatly catches overflow
	// of ++dlen.
	if(!dlen || !(dest=INT123_safer_realloc(dest, dlen)))
		goto utf8_ascii_bad;
	p = (unsigned char*)dest;
	c2lead = 0;
	wasspace = 0;
	for(spos=0; spos < source_fill; ++spos)
	{
		unsigned char c = ((unsigned char*)source)[spos];
		if(!keep_nonprint)
			ascii_space(&c, &wasspace);
		ASCII_C1(c, *p++ = joker_symbol)
		if(!ASCII_PRINT_SOMETHING(c))
			continue;
		else if(c & 0x80) // UTF-8 lead byte 0x11??????
			c = joker_symbol;
		*p++ = c;
	}
#undef ASCII_PRINT_SOMETHING
	// Always close the string.
	if(dlen)
		dest[dlen-1] = 0;
	goto utf8_ascii_end;
utf8_ascii_bad:
	dest = INT123_safer_realloc(dest, 0);
utf8_ascii_end:
	*dest_ = dest;
	return dest ? strlen(dest)+1 : 0;
}

// Reduce UTF-8 data to 7-bit ASCII, dropping non-printable characters.
// Non-printable ASCII == everything below 0x20 (space), including
// line breaks.
// Also: 0x7f (DEL) and the C1 chars. The C0 and C1 chars should just be
// dropped, not rendered. Or should they?
static size_t utf8_ascii_print(char **dest, const char *source)
{
	return utf8_ascii_work(dest, source, 0);
}

// Same as above, but keeping non-printable and control chars in the
// 7 bit realm.
static size_t utf8_ascii(char **dest, const char *source)
{
	return utf8_ascii_work(dest, source, 1);
}

size_t utf8outstr(char **dest_, const char *source, int to_terminal)
{
	if(!dest_)
		return 0;
	if(!source)
	{
		*dest_ = INT123_safer_realloc(*dest_, 0);
		return 0;
	}
	char *dest = *dest_;
	size_t width = 0;
	size_t source_fill = strlen(source)+1;

	if(utf8env)
	{
#if defined(HAVE_MBSTOWCS) && defined(HAVE_WCSWIDTH) && \
    defined(HAVE_ISWPRINT) && defined(HAVE_WCSTOMBS)
		if(utf8loc && to_terminal)
		{
			// Best case scenario: Convert to wide string, filter,
			// compute printing width.
			size_t wcharlen = mbstowcs(NULL, source, 0);
			if(wcharlen == (size_t)-1)
				goto utf8outstr_bad;
			if(wcharlen+1 > SIZE_MAX/sizeof(wchar_t))
				goto utf8outstr_bad;
			wchar_t *pre = malloc(sizeof(wchar_t)*(wcharlen+1));
			wchar_t *flt = malloc(sizeof(wchar_t)*(wcharlen+1));
			if(!pre || !flt)
			{
				free(flt);
				free(pre);
				goto utf8outstr_bad;
			}
			if(mbstowcs(pre, source, wcharlen+1) == wcharlen)
			{
				size_t nwl = 0;
				int wasspace = 0;
				for(size_t i=0;  i<wcharlen; ++i)
				{
					// Turn any funky space sequence (including line breaks) into
					// one normal space.
					if(iswspace(pre[i]) && pre[i] != ' ')
					{
						if(!wasspace)
							flt[nwl++] = ' ';
						wasspace = 1;
					} else // Anything non-printing is skipped.
					{
						if(iswprint(pre[i]))
							flt[nwl++] = pre[i];
						wasspace = 0;
					}
				}
				flt[nwl] = 0;
				int columns = wcswidth(flt, nwl);
				size_t bytelen = wcstombs(NULL, flt, 0);
				if(
					columns >= 0 && bytelen != (size_t)-1
					&& (dest=INT123_safer_realloc(dest, bytelen+1))
					&& wcstombs(dest, flt, bytelen+1) == bytelen
				){
					width = columns;
				}
				else
					dest=INT123_safer_realloc(dest, 0);
			}
			free(flt);
			free(pre);
		}
		else
#endif
		if(to_terminal)
		{
			// Only filter C0 and C1 control characters.
			// That is, 0x01 to 0x19 (keeping 0x20, space) and 0x7f (DEL) to 0x9f.
			// Since the input and output is UTF-8, we'll keep that intact.
			// C1 is mapped to 0xc280 till 0xc29f.
			dest = INT123_safer_realloc(dest, source_fill);
			if(!dest)
				goto utf8outstr_bad;
			size_t dest_fill = 0;
			int c2lead = 0;
			int wasspace = 0;
			unsigned char *p = (unsigned char*)dest;
			for(size_t i=0; i<source_fill; ++i)
			{
				unsigned char c = ((unsigned char*)source)[i];
				ascii_space(&c, &wasspace);
				ASCII_C1(c, *p++ = 0xc2)
				if(c && c < 0x20)
					continue; // no C0 control chars, except space
				if(c == 0x7f)
					continue; // also no DEL
				*p++ = c;
				if(!c)
					break; // Up to zero is enough.
				// Assume each 7 bit char and each sequence start make one character.
				// So only continuation bytes need to be ignored.
				if((c & 0xc0) != 0x80)
					++width;
			}
			// Make damn sure that it ends.
			dest_fill = (char*)p - dest;
			dest[dest_fill-1] = 0;
		} else
		{
			dest = INT123_safer_realloc(dest, source_fill);
			if(!dest)
				goto utf8outstr_bad;
			size_t dest_fill = 0;
			unsigned char *p = (unsigned char*)dest;
			for(size_t i=0; i<source_fill; ++i)
			{
				unsigned char c = ((unsigned char*)source)[i];
				*p++ = c;
				if(!c)
					break; // Up to zero is enough.
				// Actual width should not matter that much for non-terminal,
				// as we should use less formatting in that case, but anyway.
				if((c & 0xc0) != 0x80)
					++width;
			}
			dest_fill = (char*)p - dest;
			dest[dest_fill-1] = 0;
		}
	} else if(to_terminal)
	{
		// Last resort: just 7-bit ASCII.
		width = to_terminal
		?	utf8_ascii_print(&dest, source)
		:	utf8_ascii(&dest, source);
		if(!width)
			goto utf8outstr_bad;
		--width;
	}

	goto utf8outstr_end;
utf8outstr_bad:
	dest = INT123_safer_realloc(dest, 0);
	width = 0;
utf8outstr_end:
	*dest_ = dest;
	return width;
}

#undef ASCII_C1

// I tried saving some malloc using provided work buffers, but
// realized that the path of Unicode transformations is so full
// of them regardless.
// Can this include all the necessary logic?
// - If UTF-8 input: Use utf8outstr(), which includes terminal switch.
// - If not:
// -- If terminal: construct safe UTF-8, pass on to outstr().
// -- If not: assume env encoding, unprocessed string that came
//    from the environment.

int outstr(char **dest, const char *str, int is_utf8, int is_term)
{
	if(!dest)
		return -1;
	if(!str)
	{
		*dest = INT123_safer_realloc(*dest, 0);
		return -1;
	}
	int ret = 0;
	if(is_utf8 || utf8env)
	{
		utf8outstr(dest, str, is_term);
		if(*dest)
			ret = -1;
	} else if(is_term)
	{
		char *usrc = NULL;
		ret = unknown2utf8(&usrc, str, -1);
		if(!ret)
		{
			utf8outstr(dest, usrc, is_term);
			if(!*dest)
				ret = -1;
		}
		free(usrc);
	} else
	{
		*dest = INT123_compat_strdup(str);
		if(!*dest)
			ret = -1;
	}
	return ret;
}

int print_outstr(FILE *out, const char *str, int is_utf8, int is_term)
{
	int ret = 0;
	if(!str)
		return -1;
	char *outbuf = NULL;
	ret = outstr(&outbuf, str, is_utf8, is_term);
	if(outbuf)
	{
		ret = fprintf(out, "%s", outbuf);
		free(outbuf);
	}
	return ret;
}
