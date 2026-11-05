/*
	textprint: Test filtering of text before terminal printout.

	copyright 2019-2020 by the mpg123 project
	free software under the terms of the LGPL 2.1

	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis
*/

#include "../mpg123app.h"
#include "../local.h"

// A number of UTF-8 strings to test.

struct test_string
{
	const char* in; // UTF-8 input including control chars etc.
	const char* ascii; // output reduced to ASCII
	const char* utf8hack; // output in UTF-8 without controls, hacked
	const char* utf8full; // output in UTF-8 only including printing chars
	size_t chars; // number of apparent characters (for utf8hack)
	size_t width; // proper printing width
};

// const char *bla = "..." is not constant enoug to server as initializer.
#define simple_string "Just a plain string without trouble."
#define simple_width 36
// o-Umlaut and a random Chinese character
#define utf_string "St\xc3\xb6rt mein \xe7\x8c\x95?"
#define utf_string_ascii "St?rt mein ??"
#define utf_width 14
#define utf_chars 13

const struct test_string test_input[] =
{
	{
		.in=simple_string
	,	.ascii=simple_string
	,	.utf8hack=simple_string
	,	.utf8full=simple_string
	,	.chars=simple_width
	,	.width=simple_width
	}
,	{
		.in="U\xcc\x88""bel\nist mir."
	,	.ascii="U?bel ist mir."
	,	.utf8hack="U\xcc\x88""bel ist mir."
	,	.utf8full="U\xcc\x88""bel ist mir."
	,	.chars=14
	,	.width=13
	}
,	{
		.in=utf_string
	,	.ascii=utf_string_ascii
	,	.utf8hack=utf_string
	,	.utf8full=utf_string
	,	.chars=utf_chars
	,	.width=utf_width
	}
,	{
		.in="Gimme\x0a  some\x0a\x0d""con\302\237tr\xc3\xb6l!\x7f\x7f""b \033]0;xtermed\x07"
	,	.ascii="Gimme   some contr?l!b ]0;xtermed"
	,	.utf8hack="Gimme   some contr\xc3\xb6l!b ]0;xtermed"
	,	.utf8full="Gimme   some contr\xc3\xb6l!b ]0;xtermed"
	,	.chars=33
	,	.width=33
	}
};


int main()
{
	char *dst = NULL;
	const size_t test_count = sizeof(test_input)/sizeof(*test_input);
	int err = 0;

	// First, without any locale check, we should work like in C locale.
	// Only ASCII is safe.
	fprintf(stderr, "Testing plain ASCII world.\n");
	for(size_t t=0; t<test_count; ++t)
	{
		int lerr = 0;
		fprintf(stderr, "string %zu: ", t);
		size_t w = utf8outstr(&dst, test_input[t].in, 1);
		size_t l = strlen(test_input[t].ascii);
		if(!dst)
		{
			++lerr;
			fprintf(stderr, "(conversion failed) ");
		}
		else if(w == l)
		{
			if(strcmp(test_input[t].ascii, dst))
			{
				++lerr;
				fprintf(stderr, "(mismatch) ");
			}
		}
		else
		{
			++lerr;
			fprintf(stderr, "(bad length %zu != %zu) ", w, l);
		}
		err += lerr;
		fprintf(stderr, "%s\n", lerr ? "FAIL" : "PASS");
	}

	// Now, the internal UTF-8 sanitation is employed, assuming no working
	// locale to help us.
	utf8env = 1;
	utf8loc = 0;
	fprintf(stderr, "Testing hacked UTF-8 world.\n");
	for(size_t t=0; t<test_count; ++t)
	{
		int lerr = 0;
		fprintf(stderr, "string %zu: ", t);
		size_t w = utf8outstr(&dst, test_input[t].in, 1);
		size_t l = test_input[t].chars;
		if(!dst)
		{
			++lerr;
			fprintf(stderr, "(conversion failed) ");
		}
		else if(w == l)
		{
			if(strcmp(test_input[t].utf8hack, dst))
			{
				++lerr;
				fprintf(stderr, "(mismatch) ");
			}
		}
		else
		{
			++lerr;
			fprintf(stderr, "(bad length %zu != %zu) ", w, l);
		}
		err += lerr;
		fprintf(stderr, "%s\n", lerr ? "FAIL" : "PASS");
	}

#if defined(HAVE_MBSTOWCS) && defined(HAVE_WCSWIDTH) && \
    defined(HAVE_ISWPRINT) && defined(HAVE_WCSTOMBS)
	// Finally, proper UTF-8 handling including iswprint() shall be undertaken.
	// The test is skipped if there is no UTF-8 locale to work with.
	utf8env = 0;
	utf8loc = 0;
	utf8force = 1;
	check_locale();
	if(utf8env && utf8loc)
	{
		fprintf(stderr, "Got locale, testing proper workings.\n");
		for(size_t t=0; t<test_count; ++t)
		{
			int lerr = 0;
			fprintf(stderr, "string %zu: ", t);
			size_t w = utf8outstr(&dst, test_input[t].in, 1);
			size_t l = test_input[t].width;
			if(!dst)
			{
				++lerr;
				fprintf(stderr, "(conversion failed) ");
			}
			else if(w == l)
			{
				if(strcmp(test_input[t].utf8full, dst))
				{
					++lerr;
					fprintf(stderr, "(mismatch) ");
				}
			}
			else
			{
				++lerr;
				fprintf(stderr, "(bad length %zu != %zu) ", w, l);
			}
			err += lerr;
			fprintf(stderr, "%s\n", lerr ? "FAIL" : "PASS");
			fprintf(stderr, "non-terminal non-filtering: ");
			lerr = 0;
			w = utf8outstr(&dst, test_input[t].in, 0);
			if(!dst || strcmp(test_input[t].in, dst))
				++lerr;
			err += lerr;
			fprintf(stderr, "%s\n", lerr ? "FAIL" : "PASS");
		}
	}
	else
#endif
		fprintf(stderr, "Skipped locale-based conversion\n");

	free(dst);

	printf("%s\n", err ? "FAIL" : "PASS");
	return err ? 1 : 0;
}
