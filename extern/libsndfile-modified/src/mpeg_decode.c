/*
** Copyright (C) 2019 - 2021 Arthur Taylor <art@ified.ca>
** Copyright (C) 2019 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software ; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation ; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY ; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program ; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include	"sfconfig.h"

#include	<math.h>

#include	"sndfile.h"
#include	"common.h"
#include	"mpeg.h"

#if HAVE_MPEG

#include	"sfendian.h"
#include	"id3.h"

#include <mpg123.h>

typedef struct
{	mpg123_handle *pmh ;
	size_t header_remaining ;
} MPEG_DEC_PRIVATE ;

static int mpeg_dec_close (SF_PRIVATE *psf) ;
static sf_count_t mpeg_dec_seek (SF_PRIVATE *psf, int whence, sf_count_t count) ;

static ssize_t mpeg_dec_io_read (void *priv, void *buffer, size_t nbytes) ;
static off_t mpeg_dec_io_lseek (void *priv, off_t offset, int whence) ;

static ssize_t
mpeg_dec_io_read (void *priv, void *buffer, size_t nbytes)
{	SF_PRIVATE *psf = (SF_PRIVATE *) priv ;
	MPEG_DEC_PRIVATE *pmp3d = (MPEG_DEC_PRIVATE *) psf->codec_data ;

	if (pmp3d->header_remaining)
	{	if (pmp3d->header_remaining < nbytes)
			nbytes = pmp3d->header_remaining ;
		psf_binheader_readf (psf, "b", buffer, nbytes) ;
		pmp3d->header_remaining -= nbytes ;
		return nbytes ;
		} ;

	return psf_fread (buffer, 1, nbytes, psf) ;
} /* mpeg_dec_io_read */

static off_t
mpeg_dec_io_lseek (void *priv, off_t offset, int whence)
{	SF_PRIVATE *psf = (SF_PRIVATE *) priv ;

	return psf_fseek (psf, offset, whence) ;
} /* mpeg_dec_io_lseek */

static int
mpeg_dec_close (SF_PRIVATE *psf)
{	MPEG_DEC_PRIVATE *pmp3d = (MPEG_DEC_PRIVATE *) psf->codec_data ;

	if (pmp3d)
	{	if (pmp3d->pmh)
		{	mpg123_close (pmp3d->pmh) ;
			mpg123_delete (pmp3d->pmh) ;
			pmp3d->pmh = NULL ;
			}
		free (psf->codec_data) ;
		psf->codec_data = NULL ;
		} ;

	return 0 ;
} /* mpeg_dec_close */

static sf_count_t
mpeg_dec_decode (SF_PRIVATE *psf, float *ptr, sf_count_t len)
{	MPEG_DEC_PRIVATE *pmp3d = (MPEG_DEC_PRIVATE *) psf->codec_data ;
	size_t done ;
	int error ;

	error = mpg123_read (pmp3d->pmh, (unsigned char *) ptr, len * sizeof (float), &done) ;

	if (error == MPG123_OK || error == MPG123_DONE)
		return done / sizeof (float) ;

	if (error == MPG123_NEW_FORMAT)
	{	psf->error = SFE_MALFORMED_FILE ;
		return -1 ;
		} ;

	psf->error = SFE_INTERNAL ;
	return -1 ;
} /* mpeg_dec_decode */

static sf_count_t
mpeg_dec_read_s (SF_PRIVATE *psf, short *ptr, sf_count_t len)
{	BUF_UNION ubuf ;
	sf_count_t total, readlen ;
	void (*convert) (const float *, short *, int, int) ;
	const sf_count_t buflen = ARRAY_LEN (ubuf.fbuf) ;

	convert = (psf->add_clipping) ? psf_f2s_clip_array : psf_f2s_array ;
	for (total = 0 ; total < len ; total += readlen)
	{	readlen = mpeg_dec_decode (psf, ubuf.fbuf, SF_MIN (buflen, len - total)) ;
		if (readlen <= 0)
			break ;

		convert (ubuf.fbuf, ptr + total, readlen, SF_TRUE) ;
		} ;

	return total ;
} /*mpeg_dec_read_s */

static sf_count_t
mpeg_dec_read_i (SF_PRIVATE *psf, int *ptr, sf_count_t len)
{	BUF_UNION ubuf ;
	sf_count_t total, readlen ;
	void (*convert) (const float *, int *, int, int) ;
	const sf_count_t buflen = ARRAY_LEN (ubuf.fbuf) ;

	convert = (psf->add_clipping) ? psf_f2i_clip_array : psf_f2i_array ;
	for (total = 0 ; total < len ; total += readlen)
	{	readlen = mpeg_dec_decode (psf, ubuf.fbuf, SF_MIN (buflen, len - total)) ;
		if (readlen <= 0)
			break ;

		convert (ubuf.fbuf, ptr + total, readlen, SF_TRUE) ;
		} ;

	return total ;
} /* mpeg_dec_read_i */

static sf_count_t
mpeg_dec_read_f (SF_PRIVATE *psf, float *ptr, sf_count_t len)
{	sf_count_t readlen ;

	readlen = mpeg_dec_decode (psf, ptr, len) ;
	if (readlen <= 0)
		return 0 ;

	if (psf->norm_float == SF_FALSE)
		for (int i = 0 ; i < readlen ; i++)
		{	ptr [i] *= (1.0f * 0x8000) ;
			} ;

	return readlen ;
} /* mpeg_dec_read_f */

static inline void
f2d_array (const float *src, int count, double *dest, double normfact)
{	for (int i = 0 ; i < count ; i++)
	{	dest [i] = src [i] * normfact ;
		}
} /* f2d_array */

static sf_count_t
mpeg_dec_read_d (SF_PRIVATE *psf, double *ptr, sf_count_t len)
{	BUF_UNION ubuf ;
	sf_count_t total, readlen ;
	double normfact ;
	const sf_count_t buflen = ARRAY_LEN (ubuf.fbuf) ;

	normfact = (psf->norm_double == SF_TRUE) ? 1.0 : (1.0 * 0x8000) ;

	for (total = 0 ; total < len ; total += readlen)
	{	readlen = mpeg_dec_decode (psf, ubuf.fbuf, SF_MIN (buflen, len - total)) ;
		if (readlen <= 0)
			break ;

		f2d_array (ubuf.fbuf, readlen, ptr + total, normfact) ;
		} ;

	return total ;
} /* mpeg_dec_read_d */

static sf_count_t
mpeg_dec_seek (SF_PRIVATE *psf, int mode, sf_count_t count)
{	MPEG_DEC_PRIVATE *pmp3d = (MPEG_DEC_PRIVATE *) psf->codec_data ;
	off_t ret ;

	if (mode != SFM_READ || psf->file.mode != SFM_READ)
	{	psf->error = SFE_BAD_SEEK ;
		return PSF_SEEK_ERROR ;
		} ;

	ret = mpg123_seek (pmp3d->pmh, count, SEEK_SET) ;

	if (ret < 0)
		return PSF_SEEK_ERROR ;

	return (sf_count_t) ret ;
} /* mpeg_dec_seek */

static int
mpeg_dec_fill_sfinfo (SF_PRIVATE* psf, mpg123_handle *mh, SF_INFO *info)
{	int error ;
	int channels ;
	int encoding ;
	long rate ;
	off_t length ;

	error = mpg123_getformat (mh, &rate, &channels, &encoding) ;
	if (error != MPG123_OK)
		return error ;

	info->samplerate = rate ;
	info->channels = channels ;

	length = mpg123_length (mh) ;
	if (length <= 0 && !psf->is_pipe)
	{	if ((error = mpg123_scan (mh)) != MPG123_OK)
			return error ;
		length = mpg123_length (mh) ;
		}

	if (length >= 0)
	{	info->frames = length ;
		info->seekable = SF_TRUE ;
		}
	else
	{	info->frames = SF_COUNT_MAX ;
		info->seekable = SF_FALSE ;
		}

	/* Force 32-bit float samples. */
	if (encoding != MPG123_ENC_FLOAT_32)
	{	error = mpg123_format (mh, rate, channels, MPG123_ENC_FLOAT_32) ;
		} ;

	return error ;
} /* mpeg_dec_fill_sfinfo */

static void
mpeg_dec_print_frameinfo (SF_PRIVATE *psf, const struct mpg123_frameinfo *fi)
{	psf_log_printf (psf, "MPEG-1/2 Audio\n----------------------------------------\n") ;
	psf_log_printf (psf, "  MPEG version   : %s\n",
		fi->version == MPG123_1_0 ? "MPEG 1.0" :
		fi->version == MPG123_2_0 ? "MPEG 2.0" :
		fi->version == MPG123_2_5 ? "MPEG 2.5" : "???") ;
	psf_log_printf (psf, "  layer          : %d\n", fi->layer) ;
	psf_log_printf (psf, "  rate           : %d\n", fi->rate) ;
	psf_log_printf (psf, "  mode           : %s\n",
		fi->mode == MPG123_M_STEREO ? "stereo" :
		fi->mode == MPG123_M_JOINT ? "joint stereo" :
		fi->mode == MPG123_M_DUAL ? "dual channel" :
		fi->mode == MPG123_M_MONO ? "mono" : "???") ;
	psf_log_printf (psf, "  mode ext       : %d\n", fi->mode_ext) ;
	psf_log_printf (psf, "  framesize      : %d\n", fi->framesize) ;
	psf_log_printf (psf, "  crc            : %d\n", !! (fi->flags & MPG123_CRC)) ;
	psf_log_printf (psf, "  copyright flag : %d\n", !! (fi->flags & MPG123_COPYRIGHT)) ;
	psf_log_printf (psf, "  private flag   : %d\n", !! (fi->flags & MPG123_PRIVATE)) ;
	psf_log_printf (psf, "  original flag  : %d\n", !! (fi->flags & MPG123_ORIGINAL)) ;
	psf_log_printf (psf, "  emphasis       : %d\n", fi->emphasis) ;
	psf_log_printf (psf, "  bitrate mode   : ") ;
	switch (fi->vbr)
	{	case MPG123_CBR :
			psf_log_printf (psf, "constant\n") ;
			psf_log_printf (psf, "  bitrate        : %d kbps\n", fi->bitrate) ;
			break ;
		case MPG123_VBR :
			psf_log_printf (psf, "variable\n") ;
			break ;

		case MPG123_ABR :
			psf_log_printf (psf, "average\n") ;
			psf_log_printf (psf, "  ABR target     : %d\n", fi->abr_rate) ;
			break ;

		default :
			psf_log_printf (psf, "(%d) ???\n", fi->vbr) ;
			break ;
		} ;
} /* mpeg_dec_print_frameinfo */

/*
 * Like strlcpy, except the size argument is the maximum size of the input,
 * always null terminates the output string. Thus, up to size + 1 bytes may be
 * written.
 *
 * Returns the length of the copied string.
 */
static int
strcpy_inbounded (char *dest, size_t size, const char *src)
{	char *c = memccpy (dest, src, '\0', size) ;
	if (!c)
		c = dest + size ;
	*c = '\0' ;
	return c - dest ;
} /* strcpy_inbounded */

static void
mpeg_decoder_read_strings_id3v1 (SF_PRIVATE *psf, mpg123_id3v1 *tags)
{	const char *genre ;
	char buf [31] ;

	psf_log_printf (psf, "ID3v1 Tags\n") ;

	if (strcpy_inbounded (buf, ARRAY_LEN (tags->title), tags->title))
	{	psf_log_printf (psf, "  Title       : %s\n", buf) ;
		psf_store_string (psf, SF_STR_TITLE, buf) ;
		} ;

	if (strcpy_inbounded (buf, ARRAY_LEN (tags->artist), tags->artist))
	{	psf_log_printf (psf, "  Artist      : %s\n", buf) ;
		psf_store_string (psf, SF_STR_ARTIST, buf) ;
		} ;

	if (strcpy_inbounded (buf, ARRAY_LEN (tags->album), tags->album))
	{	psf_log_printf (psf, "  Album       : %s\n", buf) ;
		psf_store_string (psf, SF_STR_ALBUM, buf) ;
		} ;

	if (strcpy_inbounded (buf, ARRAY_LEN (tags->year), tags->year))
	{	psf_log_printf (psf, "  Year        : %s\n", buf) ;
		psf_store_string (psf, SF_STR_DATE, buf) ;
		} ;

	if (strcpy_inbounded (buf, ARRAY_LEN (tags->comment), tags->comment))
	{	psf_log_printf (psf, "  Comment     : %s\n", buf) ;
		psf_store_string (psf, SF_STR_COMMENT, buf) ;
		} ;

	/* ID3v1.1 Tracknumber */
	if (tags->comment [28] == '\0' && tags->comment [29] != '\0')
	{	snprintf (buf, ARRAY_LEN (buf), "%hhu", (unsigned char) tags->comment [29]) ;
		psf_log_printf (psf, "  Tracknumber : %s\n", buf) ;
		psf_store_string (psf, SF_STR_TRACKNUMBER, buf) ;
		} ;

	if ((genre = id3_lookup_v1_genre (tags->genre)) != NULL)
	{	psf_log_printf (psf, "  Genre       : %s\n", genre) ;
		psf_store_string (psf, SF_STR_GENRE, genre) ;
		} ;
} /* mpeg_decoder_read_strings_id3v1 */

static void
mpeg_decoder_read_strings_id3v2 (SF_PRIVATE *psf, mpg123_id3v2 *tags)
{	mpg123_text *text_frame ;
	size_t i ;
	uint32_t marker ;
	const char *title		= NULL ;
	const char *copyright	= NULL ;
	const char *software	= NULL ;
	const char *artist		= NULL ;
	const char *comment		= NULL ;
	const char *date		= NULL ;
	const char *album		= NULL ;
	const char *license		= NULL ;
	const char *tracknumber	= NULL ;
	const char *genre		= NULL ;
	const char *tlen		= NULL ;

	psf_log_printf (psf, "ID3v2 Tags\n") ;

	// Read the parsed text tags
	for (i = 0 ; i < tags->texts ; i++)
	{	text_frame = &tags->text [i] ;
		psf_log_printf (psf, "  %.4s        : %s\n", text_frame->id, text_frame->text.p) ;

		// Thankfully mpg123 translates v2.2 3-byte frames to v2.3 4-byte for us.
		marker = MAKE_MARKER (text_frame->id [0], text_frame->id [1],
			text_frame->id [2], text_frame->id [3]) ;

		/* Use our own map of frame types to metadata for text frames */
		switch (marker)
		{	case MAKE_MARKER ('T', 'I', 'T', '2') :
				title = text_frame->text.p ;
				break ;

			case MAKE_MARKER ('T', 'C', 'O', 'P') :
				copyright = text_frame->text.p ;
				break ;

			case MAKE_MARKER ('T', 'E', 'N', 'C') :
			case MAKE_MARKER ('T', 'S', 'S', 'E') :
				software = text_frame->text.p ;
				break ;

			case MAKE_MARKER ('T', 'P', 'E', '1') :
				artist = text_frame->text.p ;
				break ;

			case MAKE_MARKER ('T', 'A', 'L', 'B') :
				album = text_frame->text.p ;
				break ;

			case MAKE_MARKER ('T', 'R', 'C', 'K') :
				tracknumber = text_frame->text.p ;
				break ;

			case MAKE_MARKER ('T', 'Y', 'E', 'R') :
			case MAKE_MARKER ('T', 'D', 'R', 'C') :
			/* TODO (maybe)
			case MAKE_MARKER ('T', 'D', 'A', 'T') :
			case MAKE_MARKER ('T', 'I', 'M', 'E') :
			case MAKE_MARKER ('T', 'D', 'R', 'A') :
			*/
				date = text_frame->text.p ;
				break ;

			case MAKE_MARKER ('T', 'O', 'W', 'N') :
				tracknumber = text_frame->text.p ;
				break ;

			case MAKE_MARKER ('T', 'C', 'O', 'N') :
				genre = text_frame->text.p ;
				break ;

			case MAKE_MARKER ('T', 'L', 'E', 'N') :
				tlen = text_frame->text.p ;
				break ;
			} ;
		} ;

	/* Use mpg123's handling of comment headers, but print all the comment headers anyways. */
	if (tags->comment)
		comment = tags->comment->p ;
	for (i = 0 ; i < tags->comments ; i++)
	{	text_frame = &tags->comment_list [i] ;
		psf_log_printf (psf, "  %.4s        : (%s)[%s] %s\n", text_frame->id,
			text_frame->description. p, text_frame->lang, text_frame->text.p) ;
		} ;

	/* Print extra headers */
	for (i = 0 ; i < tags->extras ; i++)
	{	text_frame = &tags->extra [i] ;
		psf_log_printf (psf, "  %.4s        : (%s) %s\n", text_frame->id,
			text_frame->description.p, text_frame->text.p) ;
		} ;

	if (title != NULL)
		psf_store_string (psf, SF_STR_TITLE, title) ;
	if (copyright != NULL)
		psf_store_string (psf, SF_STR_COPYRIGHT, copyright) ;
	if (software != NULL)
		psf_store_string (psf, SF_STR_SOFTWARE, software) ;
	if (artist != NULL)
		psf_store_string (psf, SF_STR_ARTIST, artist) ;
	if (comment != NULL)
		psf_store_string (psf, SF_STR_COMMENT, comment) ;
	if (date != NULL)
		psf_store_string (psf, SF_STR_DATE, date) ;
	if (album != NULL)
		psf_store_string (psf, SF_STR_ALBUM, album) ;
	if (license != NULL)
		psf_store_string (psf, SF_STR_LICENSE, license) ;
	if (tracknumber != NULL)
		psf_store_string (psf, SF_STR_TRACKNUMBER, tracknumber) ;
	if (genre != NULL)
		psf_store_string (psf, SF_STR_GENRE, id3_process_v2_genre (genre)) ;
	if (tlen != NULL)
	{	/* If non-seekable, set framecount? Can we trust it? */
		} ;
} /* mpeg_decoder_read_strings_id3v2 */

static void
mpeg_decoder_read_strings (SF_PRIVATE *psf)
{	MPEG_DEC_PRIVATE *pmp3d = (MPEG_DEC_PRIVATE *) psf->codec_data ;
	mpg123_id3v1 *v1_tags ;
	mpg123_id3v2 *v2_tags ;

	if (mpg123_id3 (pmp3d->pmh, &v1_tags, &v2_tags) != MPG123_OK)
		return ;

	if (v1_tags != NULL)
		mpeg_decoder_read_strings_id3v1 (psf, v1_tags) ;

	if (v2_tags != NULL)
		mpeg_decoder_read_strings_id3v2 (psf, v2_tags) ;
} /* mpeg_decoder_read_strings */

static int
mpeg_dec_byterate (SF_PRIVATE *psf)
{	MPEG_DEC_PRIVATE *pmp3d = (MPEG_DEC_PRIVATE *) psf->codec_data ;
	struct mpg123_frameinfo fi ;

	if (mpg123_info (pmp3d->pmh, &fi) == MPG123_OK)
		return (fi.bitrate + 7) / 8 ;

	return -1 ;

} /* mpeg_dec_byterate */

/*==============================================================================
** exported functions
*/

int
mpeg_decoder_init (SF_PRIVATE *psf)
{	MPEG_DEC_PRIVATE *pmp3d ;
	struct mpg123_frameinfo fi ;
	int error ;

	if (! (psf->file.mode & SFM_READ))
		return SFE_INTERNAL ;

	/*
	** *** FIXME - Threading issues ***
	**
	** mpg123_init() is a global call that should only be called once, and
	** should be paried with mpg123_exit() when done. libsndfile does not
	** provide for these requirements.
	**
	** Currently this is a moot issue as mpg123_init() non-conditionally writes
	** static areas with calculated data, and mpg123_exit() is a NOP, but this
	** could change in a future version of it!
	**
	** From mpg123.h:
	** > This should be called once in a non-parallel context. It is not explicitly
	** > thread-safe, but repeated/concurrent calls still _should_ be safe as static
	** > tables are filled with the same values anyway.
	**
	** Note that calling mpg123_init() after it has already completed is a NOP.
	**
	** Update 2021-07-04
	** mpg123 upstream has confirmed that mpg132_init() will become a NOP in future,
	** so this is moot.
	*/
	if (mpg123_init () != MPG123_OK)
		return SFE_INTERNAL ;

	psf->codec_data = pmp3d = calloc (1, sizeof (MPEG_DEC_PRIVATE)) ;
	if (!psf->codec_data)
		return SFE_MALLOC_FAILED ;

	pmp3d->pmh = mpg123_new (NULL, &error) ;
	if (!pmp3d->pmh)
	{ psf_log_printf (psf, "Could not obtain a mpg123 handle: %s\n", mpg123_plain_strerror (error)) ;
		return SFE_INTERNAL ;
		} ;

	psf->codec_close = mpeg_dec_close ;

	mpg123_replace_reader_handle (pmp3d->pmh,
		mpeg_dec_io_read, mpeg_dec_io_lseek, NULL) ;

	mpg123_param (pmp3d->pmh, MPG123_REMOVE_FLAGS, MPG123_AUTO_RESAMPLE, 1.0) ;
	mpg123_param (pmp3d->pmh, MPG123_ADD_FLAGS, MPG123_FORCE_FLOAT | MPG123_GAPLESS, 1.0) ;
#if MPG123_API_VERSION >= 45
	mpg123_param (pmp3d->pmh, MPG123_ADD_FLAGS, MPG123_NO_FRANKENSTEIN, 1.0) ;
#endif

	/*
	** Need to pass the first MPEG frame to libmpg123, but that frame was read
	** into psf->binheader in order that we could identify the stream.
	*/
	if (psf->is_pipe)
	{	/*
		** Can't seek, so setup our libmpg123 io callbacks to read the binheader
		** buffer first.
		*/
		psf_binheader_readf (psf, "p", psf->dataoffset) ;
		pmp3d->header_remaining = psf_binheader_readf (psf, NULL) - psf->dataoffset ;

		/* Tell libmpg123 we can't seek the file. */
		mpg123_param (pmp3d->pmh, MPG123_ADD_FLAGS, MPG123_NO_PEEK_END, 1.0) ;
		}
	else
	{	/*
		** libmpg123 can parse the ID3v2 header. Undo the embedded file offset if the
		** enclosing file data is the ID3v2 header.
		*/
		if (psf->id3_header.len > 0 && psf->id3_header.len + psf->id3_header.offset == psf->fileoffset)
			psf->fileoffset = psf->id3_header.offset ;

		psf_fseek (psf, 0, SEEK_SET) ;
		} ;

	error = mpg123_open_handle (pmp3d->pmh, psf) ;
	if (error != MPG123_OK)
	{	psf_log_printf (psf, "mpg123 could not open the file: %s\n", mpg123_plain_strerror (error)) ;
		return SFE_BAD_FILE ;
		} ;

	if (mpeg_dec_fill_sfinfo (psf, pmp3d->pmh, &psf->sf) != MPG123_OK)
	{	psf_log_printf (psf, "Cannot get MPEG decoder configuration: %s\n", mpg123_plain_strerror (error)) ;
		return SFE_BAD_FILE ;
		} ;

	error = mpg123_info (pmp3d->pmh, &fi) ;
	if (error != MPG123_OK)
	{	psf_log_printf (psf, "Cannot get MPEG frame info: %s\n", mpg123_plain_strerror (error)) ;
		return SFE_INTERNAL ;
		}

	switch (fi.layer)
	{	case 1 : psf->sf.format |= SF_FORMAT_MPEG_LAYER_I ; break ;
		case 2 : psf->sf.format |= SF_FORMAT_MPEG_LAYER_II ; break ;
		case 3 : psf->sf.format |= SF_FORMAT_MPEG_LAYER_III ; break ;
		default :
			return SFE_BAD_FILE ;
		} ;
	mpeg_dec_print_frameinfo (psf, &fi) ;

	psf->read_short = mpeg_dec_read_s ;
	psf->read_int	= mpeg_dec_read_i ;
	psf->read_float	= mpeg_dec_read_f ;
	psf->read_double = mpeg_dec_read_d ;
	psf->seek		= mpeg_dec_seek ;
	psf->byterate	= mpeg_dec_byterate ;

	mpeg_decoder_read_strings (psf) ;

	return 0 ;
} /* mpeg_decoder_init */

int
mpeg_decoder_get_bitrate_mode (SF_PRIVATE *psf)
{	MPEG_DEC_PRIVATE *pmp3d = (MPEG_DEC_PRIVATE *) psf->codec_data ;
	struct mpg123_frameinfo fi ;

	if (mpg123_info (pmp3d->pmh, &fi) == MPG123_OK)
	{
		switch (fi.vbr)
		{	case MPG123_CBR : return SF_BITRATE_MODE_CONSTANT ;
			case MPG123_ABR : return SF_BITRATE_MODE_AVERAGE ;
			case MPG123_VBR : return SF_BITRATE_MODE_VARIABLE ;
			default : break ;
			} ;
		} ;

	psf_log_printf (psf, "Cannot determine MPEG bitrate mode.\n") ;
	return -1 ;
} /* mpeg_decoder_get_bitrate_mode */

#else /* HAVE_MPEG */

int mpeg_decoder_init (SF_PRIVATE *psf)
{	psf_log_printf (psf, "This version of libsndfile was compiled without MPEG decode support.\n") ;
	return SFE_UNIMPLEMENTED ;
} /* mpeg_decoder_init */

#endif /* HAVE_MPEG */
