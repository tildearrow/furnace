/*
	playlist: playlist logic

	copyright 1995-2023 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Michael Hipp, outsourced/reorganized by Thomas Orgis

	If we officially support Windows again, we should have this reworked to really cope with Windows paths, too.
*/

/* Need random(). */
#define _DEFAULT_SOURCE
#define _BSD_SOURCE

#include "mpg123app.h"
#include "sysutil.h"
#include "getlopt.h" /* for loptind */
#include "term.h" /* for term_restore */
#include "playlist.h"
#include "httpget.h"
#include "streamdump.h"
#include "local.h"
#include "metaprint.h"
#include <time.h> /* For srand(). */
#include "common/debug.h"

#ifdef HAVE_RANDOM
#define RAND random
#define SRAND srandom
#else
#define RAND rand
#define SRAND srand
#endif

enum playlist_type { UNKNOWN = 0, M3U, PLS, NO_LIST };

enum playflag
{
	PL_IS_UTF8 = 1 // if we really know the contents are UTF-8-encooded
,	PL_HIT_END = 2
,	PL_STDIN_USED = 4 // If the playlist itself or an input file was '-' (stdin not usable for terminal.).
,	PL_NO_RANDOM = 8
};

typedef struct listitem
{
	char* url; /* the filename */
	char freeit; /* if it was allocated and should be free()d here */
	size_t playcount; /* has been played as ...th track in overall counting */
} listitem;

typedef struct playlist_struct
{
	struct stream *file; /* the current playlist stream */
	size_t entry; /* entry in the playlist file */
	size_t playcount; /* overall track counter for playback */
	long loop;    /* repeat a track n times */
	size_t size;
	size_t fill;
	size_t pos; /* (next) position, internal use */
	size_t num; /* current track number */
	size_t alloc_step;
	struct listitem* list;
	mpg123_string linebuf;
	mpg123_string dir;
	enum playlist_type type;
	unsigned int flags;
} playlist_struct;

/* one global instance... add a pointer to this to every function definition and you have OO-style... */
static playlist_struct pl;

static int add_next_file (int argc, char *argv[], int args_utf8);
static void shuffle_playlist(void);
static void init_playlist(void);
static int add_copy_to_playlist(char* new_entry);
static int add_to_playlist(char* new_entry, char freeit);

/* used to be init_input */
void prepare_playlist(int argc, char** argv, int args_utf8, int *is_utf8)
{
	/*
		fetch all playlist entries ... I don't consider playlists to be an endless stream.
		If you want to intentionally hang mpg123 on some other prog that may take infinite time to produce the full list (perhaps load tracks on demand), then just use the remote control and let that program print "load filename" instead of "filename".
		We may even provide a simple wrapper script that emulates the old playlist reading behaviour (for files and stdin, http playlists are actually a strong point on reading the list in _before_ starting playback since http connections don't last forever).
	*/
	init_playlist();
	while (add_next_file(argc, argv, args_utf8)) {}
	if(pl.file)
	{
		stream_close(pl.file);
		pl.file = NULL;
	}
	if(param.verbose > 1)
	{
		fprintf(stderr, "\nplaylist in normal order:\n");
		print_playlist(stderr, 0);
		fprintf(stderr, "\n");
	}
	if(param.shuffle == 1) shuffle_playlist();
	/* Don't need these anymore, we have copies! */
	mpg123_free_string(&pl.linebuf);
	mpg123_free_string(&pl.dir);
	if(is_utf8)
		*is_utf8 = (pl.flags & PL_IS_UTF8) != 0;
}

/* Return a random number >= 0 and < n */
static size_t rando(size_t n)
{
	long ran;
	long limit = RAND_MAX - (RAND_MAX % (long)n);
	if(n<2) return 0; /* Better settle that here than in an endless loop... */
	do{ ran = RAND(); }while( ran >= limit );
	return (size_t)(ran%n);
}

char *get_next_file(void)
{
	struct listitem *newitem = NULL;

	/* Zero looping is nothing, as is nothing at all. */
	if(pl.fill == 0 || param.loop == 0)
		return NULL;

	++pl.playcount;

	/* normal order, just pick next thing */
	if(param.shuffle < 2)
	{
		do
		{
			if(pl.pos < pl.fill)
			{
				newitem = &pl.list[pl.pos];
				pl.num = pl.pos+1;
			}
			else newitem = NULL;
			/* if we have rounds left, decrease loop, else reinit loop because it's a new track */
			if(pl.loop > 0) --pl.loop; /* loop for current track... */
			if(pl.loop == 0)
			{
				pl.loop = param.loop;
				++pl.pos;
			}
		} while(pl.loop == 0 && newitem != NULL);
	}
	else
	{
		// Handle looping first, but only if there is a random track selection
		// Also applies to continue mode.
		if(!(pl.num && ((pl.loop > 0 && --pl.loop) || pl.loop < 0)) && !(pl.flags & PL_NO_RANDOM))
		{
			/* Randomly select the next track. */
			do /* limiting randomness: don't repeat too early */
			{
				pl.pos = rando(pl.fill);
			} while( pl.list[pl.pos].playcount
				&& (pl.playcount - pl.list[pl.pos].playcount) <= pl.fill/2 );
			pl.loop = param.loop;
		}

		newitem = &pl.list[pl.pos];
		pl.num = pl.pos+1;
		pl.flags &= ~PL_NO_RANDOM; // The random blocking works only once.
	}

	/* "-" is STDIN, "" is dumb, NULL is nothing */
	if(newitem != NULL)
	{
		/* Remember the playback position of the track. */
		newitem->playcount = pl.playcount;
		return newitem->url;
	}
	else
	{
		pl.flags |= PL_HIT_END;
		return NULL;
	}
}

size_t playlist_pos(size_t *total, long *loop)
{
	if(total)
		*total = pl.fill;
	if(loop)
		*loop = pl.loop;
	return pl.flags & PL_HIT_END ? pl.fill+1 : pl.num;
}

void playlist_jump(mpg123_ssize_t incr)
{
	size_t off = incr < 0 ? -incr : incr;

	pl.loop = 0; /* This loop is done, always. */
	/* Straight or shuffled lists can be jumped around in. */
	if(pl.fill && param.shuffle < 2)
	{
		debug3("jump %zu (%ld) + %" PRIiMAX, pl.pos, pl.loop, (intmax_t)incr);
		if(pl.pos)
			--pl.pos;
		/* Now we're at the _current_ position. */
		if(incr < 0)
			pl.pos -= off > pl.pos ? pl.pos : off;
		else
		{
			if(off >= pl.fill - pl.pos)
				pl.pos = pl.fill; /* Any value >= pl.fill would actually be OK. */
			else
				pl.pos += off;
		}
		debug2("jumped %zu (%ld)", pl.pos, pl.loop);
	}
}

/* Directory jumping based on comparing the directory part of the playlist
   URLs. */
static int cmp_dir(const char* patha, const char* pathb)
{
	size_t dirlen[2];
	dirlen[0] = dir_length(patha);
	dirlen[1] = dir_length(pathb);
	return (dirlen[0] < dirlen[1])
	?	-1
	:	( dirlen[0] > dirlen[1]
		?	1
		:	memcmp(patha, pathb, dirlen[0])
		);
}

void playlist_next_dir(void)
{
	if(pl.fill && param.shuffle < 2)
	{
		size_t npos = pl.pos ? pl.pos-1 : 0;
		do ++npos;
		while(npos < pl.fill && !cmp_dir(pl.list[npos-1].url, pl.list[npos].url));
		pl.pos = npos;
	}
	pl.loop = 0;
}

void playlist_prev_dir(void)
{
	if(pl.fill && param.shuffle < 2)
	{
		size_t npos = pl.pos ? pl.pos-1 : 0;
		/* 1. Find end of previous directory. */
		if(npos && npos < pl.fill)
			do --npos;
			while(npos && !cmp_dir(pl.list[npos+1].url, pl.list[npos].url));
		/* npos == the last track of previous directory */
		/* 2. Find the first track of this directory */
		if(npos < pl.fill)
			while(npos && !cmp_dir(pl.list[npos-1].url, pl.list[npos].url))
				--npos;
		pl.pos = npos;
	}
	pl.loop = 0;
}

/* It doesn't really matter on program exit, but anyway...
   Make sure you don't free() an item of argv! */
void free_playlist(void)
{
	if(pl.list != NULL)
	{
		debug("going to free() the playlist");
		while(pl.fill)
		{
			--pl.fill;
			debug1("free()ing entry %lu", (unsigned long)pl.fill);
			if(pl.list[pl.fill].freeit) free(pl.list[pl.fill].url);
		}
		free(pl.list);
		pl.list = NULL;
		pl.size = 0;
		debug("free()d the playlist");
	}
	mpg123_free_string(&pl.linebuf);
	mpg123_free_string(&pl.dir);
}

/* the constructor... */
static void init_playlist(void)
{
	SRAND(time(NULL));
	pl.file = NULL;
	pl.entry = 0;
	pl.playcount = 0;
	pl.size = 0;
	pl.fill = 0;
	pl.pos = 0;
	pl.num = 0;
	pl.list = NULL;
	pl.alloc_step = 10;
	mpg123_init_string(&pl.dir);
	mpg123_init_string(&pl.linebuf);
	pl.type = UNKNOWN;
	pl.flags = 0;
	pl.loop = param.loop;
	if(APPFLAG(MPG123APP_CONTINUE) && param.listentry > 0)
	{
		pl.pos = param.listentry - 1;
		pl.flags |= PL_NO_RANDOM; // Skip random selection for first track.
	}
}

int playlist_stdin(void)
{
	return (pl.flags & PL_STDIN_USED) != 0;
}

/*
	slightly modified find_next_file from mpg123.c
	now doesn't return the next entry but adds it to playlist struct
	returns 1 if it found something, 0 on end
*/
static int add_next_file (int argc, char *argv[], int args_utf8)
{
	int firstline = 0;

	if(args_utf8)
		pl.flags |= PL_IS_UTF8;
	else
		pl.flags &= ~PL_IS_UTF8;

	/* hack for url that has been detected as track, not playlist */
	if(pl.type == NO_LIST) return 0;

	/* Get playlist dirname to append it to the files in playlist */
	if (param.listname)
	{
		char* slashpos;
		/* Oh, right... that doesn't look good for Windows... */
		if ((slashpos=strrchr(param.listname, '/')))
		{
			/* up to and including /, with space for \0 */
			if(mpg123_resize_string(&pl.dir, 2 + slashpos - param.listname))
			{
				memcpy(pl.dir.p, param.listname, pl.dir.size-1);
				pl.dir.p[pl.dir.size-1] = 0;
			}
			else
			{
				error("cannot allocate memory for list directory!");
				pl.dir.size = 0;
			}
		}
	}

	if (param.listname || pl.file)
	{
		size_t line_offset = 0;
		pl.flags &= ~PL_IS_UTF8; // Playlist files in env encoding (HTTP lists should be ASCII-clean).
		if(!pl.file)
		{
			pl.file = stream_open(param.listname);
			if(pl.file)
			{
				firstline = 1; /* just opened */
				if(pl.file->fd == STDIN_FILENO)
				{
					pl.flags |= PL_STDIN_USED;
					param.listname = NULL;
				}
			}
			pl.entry = 0;
#ifdef NET123
			if(pl.file && pl.file->nh)
			{
				debug1("htd.content_type.p: %p", (void*) pl.file->htd.content_type.p);
				if(!APPFLAG(MPG123APP_IGNORE_MIME) && pl.file->htd.content_type.p != NULL)
				{
					int mimi;
					debug1("htd.content_type.p value: %s", pl.file->htd.content_type.p);
					mimi = debunk_mime(pl.file->htd.content_type.p);

					if(mimi & IS_M3U)
						pl.type = M3U;
					else if(mimi & IS_PLS)
						pl.type = PLS;
					else
					{
						if(mimi & IS_FILE)
						{
							stream_close(pl.file);
							pl.file = NULL;
							pl.type = NO_LIST;
							if(param.listentry < 0)
							{
								printf("#note you gave me a file url, no playlist, so...\n#entry 1\n");
								print_outstr(stdout, param.listname, args_utf8, stdout_is_term);
								printf("\n");
								return 0;
							}
							else
							{
								fprintf(stderr, "Note: MIME type indicates that this is no playlist but an mpeg audio file... reopening as such.\n");
								add_to_playlist(param.listname, 0);
								return 1;
							}
						}
						char *ptmp = NULL;
						outstr(&ptmp, pl.file->htd.content_type.p, 1, stderr_is_term);
						error1( "Unknown playlist MIME type %s; maybe "PACKAGE_NAME
							" can support it in future if you report this to the maintainer."
						,	PSTR(ptmp) );
						free(ptmp);
						stream_close(pl.file);
						pl.file = NULL;
					}
				}
			}
#endif
			if(!pl.file)
			{
				param.listname = NULL; // why?
				error("failed to open playlist file");
				return 0;
			} else if(param.verbose)
			{
				fprintf(stderr, "Using playlist from ");
				print_outstr( stderr, param.listname ? param.listname : "standard input"
				,	args_utf8, stderr_is_term );
				fprintf(stderr, " ...\n");
			}
		}
		/* reading the file line by line */
		while(pl.file && stream_getline(pl.file, &pl.linebuf) > 0)
		{
			/* a bit of fuzzyness */
			if(firstline)
			{
				if(pl.type == UNKNOWN)
				{
					if(!strcmp("[playlist]", pl.linebuf.p))
					{
						if(param.verbose)
							fprintf(stderr, "Note: detected Shoutcast/Winamp PLS playlist\n");
						pl.type = PLS;
						continue;
					}
					else if
					(
						(!strncasecmp("#M3U", pl.linebuf.p ,4))
						||
						(!strncasecmp("#EXTM3U", pl.linebuf.p ,7))
						||
						(param.listname != NULL && (strrchr(param.listname, '.')) != NULL && !strcasecmp(".m3u", strrchr(param.listname, '.')))
					)
					{
						if(param.verbose) fprintf(stderr, "Note: detected M3U playlist type\n");
						pl.type = M3U;
					}
					else
					{
						if(param.verbose) fprintf(stderr, "Note: guessed M3U playlist type\n");
						pl.type = M3U;
					}
				}
				else
				{
					if(param.verbose)
					{
						fprintf(stderr, "Note: Interpreting as ");
						switch(pl.type)
						{
							case M3U: fprintf(stderr, "M3U"); break;
							case PLS: fprintf(stderr, "PLS (Winamp/Shoutcast)"); break;
							default: fprintf(stderr, "???");
						}
						fprintf(stderr, " playlist\n");
					}
				}
				firstline = 0;
			}
#if !defined(WIN32)
			{
				size_t i;
				/* convert \ to / (from MS-like directory format) */
				for (i=0;pl.linebuf.p[i]!='\0';i++)
				{
					if (pl.linebuf.p[i] == '\\')	pl.linebuf.p[i] = '/';
				}
			}
#endif
			if (pl.linebuf.p[0]=='\0')
				continue; // skip empty lines...
			if (((pl.type == M3U) && (pl.linebuf.p[0]=='#')))
			{
				/* a comment line in m3u file */
				if(param.listentry < 0)
				{
					print_outstr(stdout, pl.linebuf.p, 0, stdout_is_term);
					printf("\n");
				}
				continue;
			}

			/* real filename may start at an offset */
			line_offset = 0;
			/* extract path out of PLS */
			if(pl.type == PLS)
			{
				if(!strncasecmp("File", pl.linebuf.p, 4))
				{
					/* too lazy to really check for file number... would have to change logic to support unordered file entries anyway */
					char* in_line;
					if((in_line = strchr(pl.linebuf.p+4, '=')) != NULL)
					{
						/* FileN=? */
						if(in_line[1] != 0)
						{
							++in_line;
							line_offset = (size_t) (in_line-pl.linebuf.p);
						}
						else
						{
							fprintf(stderr, "Warning: Invalid PLS line (empty filename) - corrupt playlist file?\n");
							continue;
						}
					}
					else
					{
						fprintf(stderr, "Warning: Invalid PLS line (no '=' after 'File') - corrupt playlist file?\n");
						continue;
					}
				}
				else
				{
					if(param.listentry < 0)
					{
						printf("#metainfo ");
						print_outstr(stdout, pl.linebuf.p, 0, stdout_is_term);
						printf("\n");
					}
					continue;
				}
			}

			/* make paths absolute */
			/* Windows knows absolute paths with c: in front... should handle this if really supporting win32 again */
			if
			(
				(pl.dir.p != NULL)
				&& (pl.linebuf.p[line_offset]!='/')
				&& (pl.linebuf.p[line_offset]!='\\')
				&& strncmp(pl.linebuf.p+line_offset, "http://", 7)
				&& strncmp(pl.linebuf.p+line_offset, "https://", 8)
				&& strncmp(pl.linebuf.p+line_offset, "file://", 7)
			)
			{
				size_t need;
				need = pl.dir.size + strlen(pl.linebuf.p+line_offset);
				if(pl.linebuf.size < need)
				{
					if(!mpg123_resize_string(&pl.linebuf, need))
					{
						error("unable to enlarge linebuf for appending path! skipping");
						continue;
					}
				}
				/* move to have the space at beginning */
				memmove(pl.linebuf.p+pl.dir.size-1, pl.linebuf.p+line_offset, strlen(pl.linebuf.p+line_offset)+1);
				/* prepend path */
				memcpy(pl.linebuf.p, pl.dir.p, pl.dir.size-1);
				line_offset = 0;
			}
			++pl.entry;
			if(param.listentry < 0)
			{
				printf("#entry %zu\n", pl.entry);
				print_outstr(stdout, pl.linebuf.p+line_offset, 0, stdout_is_term);
				printf("\n");
			}
			else if((param.listentry == 0) || (param.listentry == pl.entry) || APPFLAG(MPG123APP_CONTINUE))
			{
				add_copy_to_playlist(pl.linebuf.p+line_offset);
				return 1;
			}
		}
	}
	if(loptind < argc)
	{
		add_to_playlist(argv[loptind++], 0);
		return 1;
	}
	return 0;
}

static void shuffle_playlist(void)
{
	size_t loop;
	size_t rannum;
	if(pl.fill >= 2)
	{
		/* Refer to bug 1777621 for discussion on that.
		   It's Durstenfeld... */
		for (loop = 0; loop < pl.fill; loop++)
		{
			struct listitem tmp;
			rannum = loop + rando(pl.fill-loop);
			/*
				Small test on your binary operation skills (^ is XOR):
				a = b^(a^b)
				b = (a^b)^(b^(a^b))
				And, understood? ;-)
				
				pl.list[loop] ^= pl.list[rannum];
				pl.list[rannum] ^= pl.list[loop];
				pl.list[loop] ^= pl.list[rannum];
				
				But since this is not allowed with pointers and any speed gain questionable (well, saving _some_ memory...), doing it the lame way:
			*/
			tmp = pl.list[rannum];
			pl.list[rannum] = pl.list[loop];
			pl.list[loop] = tmp;
		}
	}

	if(param.verbose > 1)
	{
		/* print them */
		fprintf(stderr, "\nshuffled playlist:\n");
		print_playlist(stderr, 0);
		fprintf(stderr, "\n");
	}
}

void print_playlist(FILE* out, int showpos)
{
	size_t loop;
	int is_term = term_width(fileno(out)) >= 0;
	for (loop = 0; loop < pl.fill; loop++)
	{
		char *pre = "";
		if(showpos)
		pre = (loop+1==pl.num) ? "> " : "  ";

		fprintf(out, "%s", pre);
		print_outstr(out, pl.list[loop].url, 0, is_term);
		fprintf(out, "\n");
	}
}


static int add_copy_to_playlist(char* new_entry)
{
	char* cop;
	if((cop = (char*) malloc(strlen(new_entry)+1)) != NULL)
	{
		strcpy(cop, new_entry);
		return add_to_playlist(cop, 1);
	}
	else return 0;
}

/* add new entry to playlist - no string copy, just the pointer! */
static int add_to_playlist(char* new_entry, char freeit)
{
	if(pl.fill == pl.size)
	{
		struct listitem* tmp = NULL;
		/* enlarge the list */
		tmp = (struct listitem*) INT123_safe_realloc(pl.list, (pl.size + pl.alloc_step) * sizeof(struct listitem));
		if(!tmp)
		{
			error("unable to allocate more memory for playlist");
			perror("");
			return 0;
		}
		else
		{
			pl.list = tmp;
			pl.size += pl.alloc_step;
		}
	}
	/* paranoid */
	if(pl.fill < pl.size)
	{
		if(!strcmp(new_entry, "-") || !strcmp(new_entry, "/dev/stdin"))
			pl.flags |= PL_STDIN_USED;
		pl.list[pl.fill].freeit = freeit;
		pl.list[pl.fill].url = new_entry;
		pl.list[pl.fill].playcount = 0;
		++pl.fill;
	}
	else
	{
		error("playlist memory still too small?!");
		return 0;
	}
	return 1;
}

