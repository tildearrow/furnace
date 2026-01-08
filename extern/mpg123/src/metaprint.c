/*
	metaprint: display routines for ID3 tags (including filtering of UTF8 to ASCII)

	copyright 2006-2020 by the mpg123 project
	free software under the terms of the LGPL 2.1

	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis
*/

/* Need snprintf(). */
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
// wchar stuff
#define _XOPEN_SOURCE 600
#define _POSIX_C_SOURCE 200112L
#include "common.h"
#include "genre.h"

#include "metaprint.h"

#include "common/debug.h"

int meta_show_lyrics = 0;

/* Metadata name field texts with index enumeration. */
enum tagcode { TITLE=0, ARTIST, ALBUM, COMMENT, YEAR, GENRE, FIELDS };
static const char* name[FIELDS] =
{
	"Title"
,	"Artist"
,	"Album"
,	"Comment"
,	"Year"
,	"Genre"
};

/* Two-column printing: max length of left and right name strings.
   see print_id3 for what goes left or right.
   Choose namelen[0] >= namelen[1]! */
static const int namelen[2] = {7, 6};
/* Overhead is Name + ": " and also plus "  " for right column. */
/* pedantic C89 does not like:
const int overhead[2] = { namelen[0]+2, namelen[1]+4 }; */
static const int overhead[2] = { 9, 10 };

static size_t mpg_utf8outstr( mpg123_string *dest, mpg123_string *source
,	int to_terminal )
{
	size_t ret = utf8outstr( &(dest->p)
	,	(source && source->fill) ? source->p : NULL
	,	to_terminal );
	dest->size = dest->fill = dest->p ? strlen(dest->p)+1 : 0;
	return ret;
}

// If the given ID3 string is empty, possibly replace it with ID3v1 data.
static void id3_gap(mpg123_string *dest, int count, char *v1, size_t *len, int is_term)
{
	if(dest->fill)
		return;
	char *utf8tmp = NULL;
	// First construct some UTF-8 from the id3v1 data, then run through
	// the same filter as everything else.
	*len = unknown2utf8(&utf8tmp, v1, count) == 0 ? utf8outstr(&(dest->p), utf8tmp, is_term) : 0;
	dest->size = dest->fill = dest->p ? strlen(dest->p)+1 : 0;
	free(utf8tmp);
}

/* Print one metadata entry on a line, aligning the beginning. */
static void print_oneline( FILE* out
,	const mpg123_string *tag, enum tagcode fi, int long_mode )
{
	int ret;
	char fmt[14]; /* "%s:%-XXXs%s\n" plus one null */
	if(!tag[fi].fill && !long_mode)
		return;

	if(long_mode)
		fprintf(out, "\t");
	ret = snprintf( fmt, sizeof(fmt)-1, "%%s:%%-%ds%%s\n"
	,	1+namelen[0]-(int)strlen(name[fi]) );
	if(ret >= sizeof(fmt)-1)
		fmt[sizeof(fmt)-1] = 0;
	fprintf(out, fmt, name[fi], " ", tag[fi].fill ? tag[fi].p : "");
}

/*
	Print a pair of tag name-value pairs along each other in two columns or
	each on a line if that is not sensible.
	This takes a given length (in columns) into account, not just bytes.
	If that length would be computed taking grapheme clusters into account, things
	could be fine for the whole world of Unicode. So far we ride only on counting
	possibly multibyte characters (unless mpg123_strlen() got adapted meanwhile).
*/
static void print_pair
(
	FILE* out /* Output stream. */
,	const int *climit /* Maximum width of columns (two values). */
,	const mpg123_string *tag /* array of tag value strings */
,	const size_t *len /* array of character/column lengths */
,	enum tagcode f0, enum tagcode f1 /* field indices for column 0 and 1 */
){
	/* Two-column printout if things match, dumb printout otherwise. */
	if(  tag[f0].fill         && tag[f1].fill
	  && len[f0] <= (size_t)climit[0] && len[f1] <= (size_t)climit[1] )
	{
		int ret; // Need to store return value of snprintf to silence gcc.
		char cfmt[35]; /* "%s:%-XXXs%-XXXs  %s:%-XXXs%-XXXs\n" plus one extra null from snprintf */
		int chardiff[2];
		size_t bytelen;

		/* difference between character length and byte length */
		bytelen = strlen(tag[f0].p);
		chardiff[0] = len[f0] < bytelen ? bytelen-len[f0] : 0;
		bytelen = strlen(tag[f1].p);
		chardiff[1] = len[f1] < bytelen ? bytelen-len[f1] : 0;

		/* Two-column format string with added padding for multibyte chars. */
		ret = snprintf( cfmt, sizeof(cfmt)-1, "%%s:%%-%ds%%-%ds  %%s:%%-%ds%%-%ds\n"
		,	1+namelen[0]-(int)strlen(name[f0]), climit[0]+chardiff[0]
		,	1+namelen[1]-(int)strlen(name[f1]), climit[1]+chardiff[1] );
		if(ret >= sizeof(cfmt)-1)
			cfmt[sizeof(cfmt)-1] = 0;
		/* Actual printout of name and value pairs. */
		fprintf(out, cfmt, name[f0], " ", tag[f0].p, name[f1], " ", tag[f1].p);
	}
	else
	{
		print_oneline(out, tag, f0, FALSE);
		print_oneline(out, tag, f1, FALSE);
	}
}

/* Print tags... limiting the UTF-8 to ASCII, if necessary. */
void print_id3_tag(mpg123_handle *mh, int long_id3, FILE *out, int linelimit)
{
	enum tagcode ti;
	mpg123_string tag[FIELDS];
	mpg123_string genretmp;
	size_t len[FIELDS];
	mpg123_id3v1 *v1;
	mpg123_id3v2 *v2;
	int is_term = term_width(fileno(out)) >= 0;
	if(!is_term)
		long_id3 = 1;
	/* no memory allocated here, so return is safe */
	for(ti=0; ti<FIELDS; ++ti){ len[ti]=0; mpg123_init_string(&tag[ti]); }
	/* extract the data */
	mpg123_id3(mh, &v1, &v2);
	{
		// Ignore v1 data for Frankenstein streams. It is just somewhere in between.
		long frank;
		if(mpg123_getstate(mh, MPG123_FRANKENSTEIN, &frank, NULL) == MPG123_OK && frank)
			v1 = NULL;
	}

	/* Only work if something there... */
	if(v1 == NULL && v2 == NULL) return;

	if(v2 != NULL) /* fill from ID3v2 data */
	{
		len[TITLE]   = mpg_utf8outstr(&tag[TITLE],   v2->title,   is_term);
		len[ARTIST]  = mpg_utf8outstr(&tag[ARTIST],  v2->artist,  is_term);
		len[ALBUM]   = mpg_utf8outstr(&tag[ALBUM],   v2->album,   is_term);
		len[COMMENT] = mpg_utf8outstr(&tag[COMMENT], v2->comment, is_term);
		len[YEAR]    = mpg_utf8outstr(&tag[YEAR],    v2->year,    is_term);
	}
	if(v1 != NULL) /* fill gaps with ID3v1 data */
	{
		/* I _could_ skip the recalculation of fill ... */
		id3_gap(&tag[TITLE],   30, v1->title,   &len[TITLE],   is_term);
		id3_gap(&tag[ARTIST],  30, v1->artist,  &len[ARTIST],  is_term);
		id3_gap(&tag[ALBUM],   30, v1->album,   &len[ALBUM],   is_term);
		id3_gap(&tag[COMMENT], 30, v1->comment, &len[COMMENT], is_term);
		id3_gap(&tag[YEAR],    4,  v1->year,    &len[YEAR],    is_term);
	}
	// Genre is special... v1->genre holds an index, id3v2 genre may contain
	// indices in textual form and raw textual genres...
	mpg123_init_string(&genretmp);
	if(v2 && v2->genre && v2->genre->fill)
	{
		/*
			id3v2.3 says (id)(id)blabla and in case you want ot have (blabla) write ((blabla)
			also, there is
			(RX) Remix
			(CR) Cover
			id3v2.4 says
			"one or several of the ID3v1 types as numerical strings"
			or define your own (write strings), RX and CR 

			Now I am very sure that I'll encounter hellishly mixed up id3v2 frames, so try to parse both at once.
		*/
		size_t num = 0;
		size_t nonum = 0;
		size_t i;
		enum { nothing, number, outtahere } state = nothing;
		/* number\n -> id3v1 genre */
		/* (number) -> id3v1 genre */
		/* (( -> ( */
		debug1("interpreting genre: %s\n", v2->genre->p);
		for(i = 0; i < v2->genre->fill; ++i)
		{
			debug1("i=%lu", (unsigned long) i);
			switch(state)
			{
				case nothing:
					nonum = i;
					if(v2->genre->p[i] == '(')
					{
						num = i+1; /* number starting as next? */
						state = number;
						debug1("( before number at %lu?", (unsigned long) num);
					}
					else if(v2->genre->p[i] >= '0' && v2->genre->p[i] <= '9')
					{
						num = i;
						state = number;
						debug1("direct number at %lu", (unsigned long) num);
					}
					else state = outtahere;
				break;
				case number:
					/* fake number alert: (( -> ( */
					if(v2->genre->p[i] == '(')
					{
						nonum = i;
						state = outtahere;
						debug("no, it was ((");
					}
					else if(v2->genre->p[i] == ')' || v2->genre->p[i] == '\n' || v2->genre->p[i] == 0)
					{
						if(i-num > 0)
						{
							/* we really have a number */
							int gid;
							char* genre = "Unknown";
							v2->genre->p[i] = 0;
							gid = atoi(v2->genre->p+num);

							/* get that genre */
							if(gid >= 0 && gid <= genre_count) genre = genre_table[gid];
							debug1("found genre: %s", genre);

							if(genretmp.fill) mpg123_add_string(&genretmp, ", ");
							mpg123_add_string(&genretmp, genre);
							nonum = i+1; /* next possible stuff */
							state = nothing;
							debug1("had a number: %i", gid);
						}
						else
						{
							/* wasn't a number, nonum is set */
							state = outtahere;
							debug("no (num) thing...");
						}
					}
					else if(!(v2->genre->p[i] >= '0' && v2->genre->p[i] <= '9'))
					{
						/* no number at last... */
						state = outtahere;
						debug("nothing numeric here");
					}
					else
					{
						debug("still number...");
					}
				break;
				default: break;
			}
			if(state == outtahere) break;
		}
		/* Small hack: Avoid repeating genre in case of stuff like
			(144)Thrash Metal being given. The simple cases. */
		if(
			nonum < v2->genre->fill-1 &&
			(!genretmp.fill || strncmp(genretmp.p, v2->genre->p+nonum, genretmp.fill))
		)
		{
			if(genretmp.fill) mpg123_add_string(&genretmp, ", ");
			mpg123_add_string(&genretmp, v2->genre->p+nonum);
		}
	}
	else if(v1)
	{
		// Fill from v1 tag.
		if(mpg123_resize_string(&genretmp, 31))
		{
			if(v1->genre <= genre_count)
				strncpy(genretmp.p, genre_table[v1->genre], 30);
			else
				strncpy(genretmp.p,"Unknown",30);
			genretmp.p[30] = 0;
			genretmp.fill = strlen(genretmp.p)+1;
		}
	}
	// Finally convert to safe output string and get display width.
	len[GENRE] = mpg_utf8outstr(&tag[GENRE], &genretmp, is_term);
	mpg123_free_string(&genretmp);

	if(long_id3)
	{
		fprintf(out,"\n");
		/* print id3v2 */
		print_oneline(out, tag, TITLE,   TRUE);
		print_oneline(out, tag, ARTIST,  TRUE);
		print_oneline(out, tag, ALBUM,   TRUE);
		print_oneline(out, tag, YEAR,    TRUE);
		print_oneline(out, tag, GENRE,   TRUE);
		print_oneline(out, tag, COMMENT, TRUE);
		fprintf(out,"\n");
	}
	else
	{
		/* We are trying to be smart here and conserve some vertical space.
		   So we will skip tags not set, and try to show them in two parallel
		   columns if they are short, which is by far the most common case. */
		int climit[2];

		/* Adapt formatting width to terminal if possible. */
		if(linelimit < 0)
			linelimit=overhead[0]+30+overhead[1]+30; /* the old style, based on ID3v1 */
		if(linelimit > 200)
			linelimit = 200; /* Not too wide. Also for format string safety. */
		/* Divide the space between the two columns, not wasting any. */
		climit[1] = linelimit/2-overhead[0];
		climit[0] = linelimit-linelimit/2-overhead[1];
		debug3("linelimits: %i  < %i | %i >", linelimit, climit[0], climit[1]);

		if(climit[0] <= 0 || climit[1] <= 0)
		{
			/* Ensure disabled column printing, no play with signedness in comparisons. */
			climit[0] = 0;
			climit[1] = 0;
		}
		fprintf(out,"\n"); /* Still use one separator line. Too ugly without. */
		print_pair(out, climit, tag, len, TITLE,   ARTIST);
		print_pair(out, climit, tag, len, COMMENT, ALBUM );
		print_pair(out, climit, tag, len, YEAR,    GENRE );
	}
	for(ti=0; ti<FIELDS; ++ti) mpg123_free_string(&tag[ti]);

	if(v2 != NULL && meta_show_lyrics)
	{
		/* find and print texts that have USLT IDs */
		size_t i;
		for(i=0; i<v2->texts; ++i)
		{
			if(!memcmp(v2->text[i].id, "USLT", 4))
			{
				/* split into lines, ensure usage of proper local line end */
				size_t a=0;
				size_t b=0;
				char lang[4]; /* just a 3-letter ASCII code, no fancy encoding */
				mpg123_string innline;
				mpg123_string outline;
				mpg123_string *uslt = &v2->text[i].text;

				memcpy(lang, &v2->text[i].lang, 3);
				lang[3] = 0;
				fprintf(out, "Lyrics begin, language: %s; %s\n\n", lang,  v2->text[i].description.fill ? v2->text[i].description.p : "");

				mpg123_init_string(&innline);
				mpg123_init_string(&outline);
				while(a < uslt->fill)
				{
					b = a;
					while(b < uslt->fill && uslt->p[b] != '\n' && uslt->p[b] != '\r') ++b;
					/* Either found end of a line or end of the string (null byte) */
					mpg123_set_substring(&innline, uslt->p, a, b-a);
					mpg_utf8outstr(&outline, &innline, is_term);
					fprintf(out, " %s\n", outline.p);

					if(uslt->p[b] == uslt->fill) break; /* nothing more */

					/* Swallow CRLF */
					if(uslt->fill-b > 1 && uslt->p[b] == '\r' && uslt->p[b+1] == '\n') ++b;

					a = b + 1; /* next line beginning */
				}
				mpg123_free_string(&innline);
				mpg123_free_string(&outline);

				fprintf(out, "\nLyrics end.\n");
			}
		}
	}
	// A separator line just looks nicer.
	fprintf(out, "\n");
}

void print_icy(mpg123_handle *mh, FILE *outstream)
{
	int is_term = term_width(fileno(outstream)) >= 0;
	char* icy;
	if(MPG123_OK == mpg123_icy(mh, &icy))
	{
		mpg123_string in;
		mpg123_init_string(&in);
		if(mpg123_store_utf8(&in, mpg123_text_icy, (unsigned char*)icy, strlen(icy)+1))
		{
			char *out = NULL;
			utf8outstr(&out, in.p, is_term);
			if(out)
				fprintf(outstream, "\nICY-META: %s\n", out);

			free(out);
		}
		mpg123_free_string(&in);
	}
}
