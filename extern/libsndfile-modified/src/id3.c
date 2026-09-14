/*
** Copyright (C) 2010-2017 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include	"sfconfig.h"

#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
#include	<ctype.h>

#include	"sndfile.h"
#include	"sfendian.h"
#include	"common.h"
#include	"id3.h"

#if HAVE_MPEG
#include	<lame/lame.h>

struct id3v1_genre_handler_userdata
{	int number ;
	const char *ret ;
} ;

static void
id3v1_genre_handler (int number, const char *description, void *userdata)
{	struct id3v1_genre_handler_userdata *data = (struct id3v1_genre_handler_userdata *) userdata ;
	if (data->number == number)
		data->ret = description ;
}

const char *
id3_lookup_v1_genre (int number)
{	struct id3v1_genre_handler_userdata data ;

	data.number = number ;
	data.ret = NULL ;
	id3tag_genre_list (id3v1_genre_handler, &data) ;

	return data.ret ;
}

#else /* HAVE_MPEG */

const char *
id3_lookup_v1_genre (int UNUSED (number))
{	return NULL ;
	}

#endif

int
id3_skip (SF_PRIVATE * psf)
{	unsigned char	buf [10] ;
	int	offset ;

	memset (buf, 0, sizeof (buf)) ;
	psf_binheader_readf (psf, "pb", 0, buf, 10) ;

	if (buf [0] == 'I' && buf [1] == 'D' && buf [2] == '3')
	{	psf->id3_header.minor_version = buf [3] ;
		offset = buf [6] & 0x7f ;
		offset = (offset << 7) | (buf [7] & 0x7f) ;
		offset = (offset << 7) | (buf [8] & 0x7f) ;
		offset = (offset << 7) | (buf [9] & 0x7f) ;

		/*
		** ID3 count field is how many bytes of ID3v2 header FOLLOW the ten
		** bytes of header magic and offset, NOT the total ID3v2 header len.
		*/
		psf->id3_header.len = offset + 10 ;
		psf->id3_header.offset = psf->fileoffset ;

		psf_log_printf (psf, "  ID3v2.%d header length :	%d\n----------------------------------------\n",
			psf->id3_header.minor_version, psf->id3_header.len) ;

		/* Never want to jump backwards in a file. */
		if (offset < 0)
			return 0 ;

		/* Position ourselves at the new file offset. */
		if (psf->fileoffset + psf->id3_header.len < psf->filelength)
		{	psf_binheader_readf (psf, "p!", psf->id3_header.len) ;
			psf->fileoffset += psf->id3_header.len ;
			return 1 ;
			} ;
		} ;

	return 0 ;
} /* id3_skip */

const char *
id3_process_v2_genre (const char *genre)
{	int num = 0 ;
	char c ;
	const char *ptr ;

	if (!genre)
		return NULL ;

	/*
	** Genre may require more processing.
	**
	** It is allowed to have numeric references to the genre table from ID3v1.
	** We'll just convert the simple case here, strings of the format "(nnn)".
	*/
	ptr = genre ;
	if (ptr [0] == '(' && (c = *++ ptr) && isdigit (c))
	{	num = c - '0' ;
		while ((c == *++ ptr) && isdigit (c))
			num = num * 10 + (c - '0') ;
		if (c == ')' && (c = *++ ptr) == '\0' && num < 256)
			if ((ptr = id3_lookup_v1_genre (num)))
				return ptr ;
		} ;

	return genre ;
} /* id3_process_v2_genre */
