/*
	stringlists: creation of paired string lists for one-time consumption

	copyright 2015-2021 by the mpg123 project
	free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis

	Thomas did not want to introduce a list type complete with management
	functions just for returning driver module lists.
*/

#include "../compat/compat.h"
#include "out123.h"

static char* always_strdup(const char *in)
{
	char *out = in ? INT123_compat_strdup(in) : malloc(1);
	if(!in && out)
		out[0] = 0;
	return out;
}

/* Construction helper for paired string lists.
   Returns 0 on success. */
// Also converts NULL to empty string for safer use later.
int INT123_stringlists_add( char ***alist, char ***blist
                   , const char *atext, const char *btext, int *count)
{
	char *atextcopy = NULL;
	char *btextcopy = NULL;
	char **morealist = NULL;
	char **moreblist = NULL;

	/* If one of these succeeded, the old memory is gone, so always overwrite
	   the old pointer, worst case is wasted but not leaked memory in an
	   out-of-memory situation. */
	if((morealist = INT123_safe_realloc(*alist, sizeof(char*)*(*count+1))))
		*alist = morealist;
	if((moreblist = INT123_safe_realloc(*blist, sizeof(char*)*(*count+1))))
		*blist = moreblist;
	if(!morealist || !moreblist)
		return -1;

	if(
		(atextcopy = always_strdup(atext))
	&&	(btextcopy = always_strdup(btext))
	)
	{
		(*alist)[*count] = atextcopy;
		(*blist)[*count] = btextcopy;
		++*count;
		return 0;
	}
	else
	{
		free(btextcopy);
		free(atextcopy);
		return -1;
	}
}

void out123_stringlists_free(char **alist, char **blist, int count)
{
	if(alist)
	{
		for(int i=0; i<count; ++i)
			free(alist[i]);
		free(alist);
	}
	if(blist)
	{
		for(int i=0; i<count; ++i)
			free(blist[i]);
		free(blist);
	}
}
