/*
** Copyright (C) 2020 Arthur Taylor <art@ified.ca>
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
#include	"sndfile.h"
#include	"common.h"
#include	"mpeg.h"


#if HAVE_MPEG

#include <lame/lame.h>

/*
 * RANT RANT RANT
 *
 * Lame has 11 functions for inputing sample data of various types and
 * configurations, but due to bad definitions, or missing combinations, they
 * aren't really of much help to us.
 *
 */

typedef struct
{	lame_t lamef ;
	unsigned char *block ;
	size_t block_len ;
	int frame_samples ;
	double compression ;
	int initialized ;
} MPEG_L3_ENC_PRIVATE ;


/*-----------------------------------------------------------------------------------------------
** Private function prototypes.
*/

static int mpeg_l3_encoder_close (SF_PRIVATE *psf) ;
static int mpeg_l3_encoder_construct (SF_PRIVATE *psf) ;
static int mpeg_l3_encoder_byterate (SF_PRIVATE *psf) ;

static sf_count_t mpeg_l3_encode_write_short_stereo (SF_PRIVATE *psf, const short *ptr, sf_count_t len) ;
static sf_count_t mpeg_l3_encode_write_int_stereo (SF_PRIVATE *psf, const int *ptr, sf_count_t len) ;
static sf_count_t mpeg_l3_encode_write_float_stereo (SF_PRIVATE *psf, const float *ptr, sf_count_t len) ;
static sf_count_t mpeg_l3_encode_write_double_stereo (SF_PRIVATE *psf, const double *ptr, sf_count_t len) ;
static sf_count_t mpeg_l3_encode_write_short_mono (SF_PRIVATE *psf, const short *ptr, sf_count_t len) ;
static sf_count_t mpeg_l3_encode_write_int_mono (SF_PRIVATE *psf, const int *ptr, sf_count_t len) ;
static sf_count_t mpeg_l3_encode_write_float_mono (SF_PRIVATE *psf, const float *ptr, sf_count_t len) ;
static sf_count_t mpeg_l3_encode_write_double_mono (SF_PRIVATE *psf, const double *ptr, sf_count_t len) ;

/*-----------------------------------------------------------------------------------------------
** Exported functions.
*/

int
mpeg_l3_encoder_init (SF_PRIVATE *psf, int info_tag)
{	MPEG_L3_ENC_PRIVATE* pmpeg = NULL ;

	if (psf->file.mode == SFM_RDWR)
		return SFE_BAD_MODE_RW ;

	if (psf->file.mode != SFM_WRITE)
		return SFE_INTERNAL ;

	psf->codec_data = pmpeg = calloc (1, sizeof (MPEG_L3_ENC_PRIVATE)) ;
	if (!pmpeg)
		return SFE_MALLOC_FAILED ;

	if (psf->sf.channels < 1 || psf->sf.channels > 2)
		return SFE_BAD_OPEN_FORMAT ;

	if (! (pmpeg->lamef = lame_init ()))
		return SFE_MALLOC_FAILED ;

	pmpeg->compression = -1.0 ; /* Unset */

	lame_set_in_samplerate (pmpeg->lamef, psf->sf.samplerate) ;
	lame_set_num_channels (pmpeg->lamef, psf->sf.channels) ;
	if (lame_set_out_samplerate (pmpeg->lamef, psf->sf.samplerate) < 0)
		return SFE_MPEG_BAD_SAMPLERATE ;

	lame_set_write_id3tag_automatic (pmpeg->lamef, 0) ;

	if (!info_tag || psf->is_pipe)
	{	/* Can't seek back, so force disable Xing/Lame/Info header. */
		lame_set_bWriteVbrTag (pmpeg->lamef, 0) ;
		} ;

	if (psf->sf.channels == 2)
	{	psf->write_short	= mpeg_l3_encode_write_short_stereo ;
		psf->write_int		= mpeg_l3_encode_write_int_stereo ;
		psf->write_float	= mpeg_l3_encode_write_float_stereo ;
		psf->write_double	= mpeg_l3_encode_write_double_stereo ;
		}
	else
	{	psf->write_short	= mpeg_l3_encode_write_short_mono ;
		psf->write_int		= mpeg_l3_encode_write_int_mono ;
		psf->write_float	= mpeg_l3_encode_write_float_mono ;
		psf->write_double	= mpeg_l3_encode_write_double_mono ;
		}

	psf->sf.seekable	= 0 ;
	psf->codec_close	= mpeg_l3_encoder_close ;
	psf->byterate		= mpeg_l3_encoder_byterate ;
	psf->datalength		= 0 ;

	return 0 ;
} /* mpeg_l3_encoder_init */

int
mpeg_l3_encoder_write_id3tag (SF_PRIVATE *psf)
{	MPEG_L3_ENC_PRIVATE *pmpeg = (MPEG_L3_ENC_PRIVATE *) psf->codec_data ;
	unsigned char *id3v2_buffer ;
	int i, id3v2_size ;

	if (psf->have_written)
		return 0 ;

	if ((i = mpeg_l3_encoder_construct (psf)))
		return i ;

	if (psf_fseek (psf, 0, SEEK_SET) != 0)
		return SFE_NOT_SEEKABLE ;

	/* Safe to call multiple times. */
	id3tag_init (pmpeg->lamef) ;

	for (i = 0 ; i < SF_MAX_STRINGS ; i++)
	{	switch (psf->strings.data [i].type)
		{	case SF_STR_TITLE :
				id3tag_set_title (pmpeg->lamef, psf->strings.storage + psf->strings.data [i].offset) ;
				break ;

			case SF_STR_ARTIST :
				id3tag_set_artist (pmpeg->lamef, psf->strings.storage + psf->strings.data [i].offset) ;
				break ;

			case SF_STR_ALBUM :
				id3tag_set_album (pmpeg->lamef, psf->strings.storage + psf->strings.data [i].offset) ;
				break ;

			case SF_STR_DATE :
				id3tag_set_year (pmpeg->lamef, psf->strings.storage + psf->strings.data [i].offset) ;
				break ;

			case SF_STR_COMMENT :
				id3tag_set_comment (pmpeg->lamef, psf->strings.storage + psf->strings.data [i].offset) ;
				break ;

			case SF_STR_GENRE :
				id3tag_set_genre (pmpeg->lamef, psf->strings.storage + psf->strings.data [i].offset) ;
				break ;

			case SF_STR_TRACKNUMBER :
				id3tag_set_track (pmpeg->lamef, psf->strings.storage + psf->strings.data [i].offset) ;
				break ;

			default:
				break ;
			} ;
		} ;

	/* The header in this case is the ID3v2 tag header. */
	id3v2_size = lame_get_id3v2_tag (pmpeg->lamef, 0, 0) ;
	if (id3v2_size > 0)
	{	psf_log_printf (psf, "Writing ID3v2 header.\n") ;
		if (! (id3v2_buffer = malloc (id3v2_size)))
			return SFE_MALLOC_FAILED ;
		lame_get_id3v2_tag (pmpeg->lamef, id3v2_buffer, id3v2_size) ;
		psf_fwrite (id3v2_buffer, 1, id3v2_size, psf) ;
		psf->dataoffset = id3v2_size ;
		free (id3v2_buffer) ;
		} ;

	return 0 ;
}

int
mpeg_l3_encoder_set_quality (SF_PRIVATE *psf, double compression)
{	MPEG_L3_ENC_PRIVATE *pmpeg = (MPEG_L3_ENC_PRIVATE *) psf->codec_data ;
	int bitrate_mode ;
	int bitrate ;
	int ret ;

	if (compression < 0.0 || compression > 1.0)
		return SF_FALSE ;

	/*
	** Save the compression setting, as we may have to re-interpret it if
	** the bitrate mode changes.
	*/
	pmpeg->compression = compression ;

	bitrate_mode = mpeg_l3_encoder_get_bitrate_mode (psf) ;
	if (bitrate_mode == SF_BITRATE_MODE_VARIABLE)
	{	ret = lame_set_VBR_quality (pmpeg->lamef, compression * 10.0) ;
		}
	else
	{	/* Choose a bitrate. */
		if (psf->sf.samplerate >= 32000)
		{	/* MPEG-1.0, bitrates are [32,320] kbps */
			bitrate = (320.0 - (compression * (320.0 - 32.0))) ;
			}
		else if (psf->sf.samplerate >= 16000)
		{	/* MPEG-2.0, bitrates are [8,160] */
			bitrate = (160.0 - (compression * (160.0 - 8.0))) ;
			}
		else
		{	/* MPEG-2.5, bitrates are [8,64] */
			bitrate = (64.0 - (compression * (64.0 - 8.0))) ;
			}

		if (bitrate_mode == SF_BITRATE_MODE_AVERAGE)
			ret = lame_set_VBR_mean_bitrate_kbps (pmpeg->lamef, bitrate) ;
		else
			ret = lame_set_brate (pmpeg->lamef, bitrate) ;
		} ;

	if (ret == LAME_OKAY)
		return SF_TRUE ;

	psf_log_printf (psf, "Failed to set lame encoder quality.\n") ;
	return SF_FALSE ;
} /* mpeg_l3_encoder_set_quality */

int
mpeg_l3_encoder_set_bitrate_mode (SF_PRIVATE *psf, int mode)
{	MPEG_L3_ENC_PRIVATE *pmpeg = (MPEG_L3_ENC_PRIVATE *) psf->codec_data ;
	enum vbr_mode_e vbr_mode ;

	if (pmpeg->initialized)
	{	psf->error = SFE_CMD_HAS_DATA ;
		return SF_FALSE ;
		} ;

	switch (mode)
	{	case SF_BITRATE_MODE_CONSTANT :	vbr_mode = vbr_off ; break ;
		case SF_BITRATE_MODE_AVERAGE : vbr_mode = vbr_abr ; break ;
		case SF_BITRATE_MODE_VARIABLE : vbr_mode = vbr_default ; break ;
		default :
			psf->error = SFE_BAD_COMMAND_PARAM ;
			return SF_FALSE ;
		} ;

	if (lame_set_VBR (pmpeg->lamef, vbr_mode) == LAME_OKAY)
	{	/* Re-evaluate the compression setting. */
		return mpeg_l3_encoder_set_quality (psf, pmpeg->compression) ;
		} ;

	psf_log_printf (psf, "Failed to set LAME vbr mode to %d.\n", vbr_mode) ;
	return SF_FALSE ;
} /* mpeg_l3_encoder_set_bitrate_mode */

int
mpeg_l3_encoder_get_bitrate_mode (SF_PRIVATE *psf)
{	MPEG_L3_ENC_PRIVATE *pmpeg = (MPEG_L3_ENC_PRIVATE *) psf->codec_data ;
	enum vbr_mode_e vbr_mode ;

	vbr_mode = lame_get_VBR (pmpeg->lamef) ;

	if (vbr_mode == vbr_off)
		return SF_BITRATE_MODE_CONSTANT ;
	if (vbr_mode == vbr_abr)
		return SF_BITRATE_MODE_AVERAGE ;
	if (vbr_mode == vbr_default || vbr_mode < vbr_max_indicator)
		return SF_BITRATE_MODE_VARIABLE ;

	/* Something is wrong. */
	psf->error = SFE_INTERNAL ;
	return -1 ;
} /* mpeg_l3_encoder_get_bitrate_mode */


/*-----------------------------------------------------------------------------------------------
** Private functions.
*/

static int
mpeg_l3_encoder_close (SF_PRIVATE *psf)
{	MPEG_L3_ENC_PRIVATE* pmpeg = (MPEG_L3_ENC_PRIVATE *) psf->codec_data ;
	int ret, len ;
	sf_count_t pos ;
	unsigned char *buffer ;

	/* Magic number 7200 comes from a comment in lame.h */
	len = 7200 ;
	if (! (buffer = malloc (len)))
		return SFE_MALLOC_FAILED ;
	ret = lame_encode_flush (pmpeg->lamef, buffer, len) ;
	if (ret > 0)
		psf_fwrite (buffer, 1, ret, psf) ;

	/*
	** Write an IDv1 trailer. The whole tag structure is always 128 bytes, so is
	** guaranteed to fit in the buffer allocated above.
	*/
	ret = lame_get_id3v1_tag (pmpeg->lamef, buffer, len) ;
	if (ret > 0)
	{	psf_log_printf (psf, "  Writing ID3v1 trailer.\n") ;
		psf_fwrite (buffer, 1, ret, psf) ;
		} ;

	/*
	** If possible, seek back and write the LAME/XING/Info headers. This
	** contains information about the whole file and a seek table, and can
	** only be written after encoding.
	**
	** If enabled, Lame wrote an empty header at the beginning of the data
	** that we now fill in.
	*/
	ret = lame_get_lametag_frame (pmpeg->lamef, 0, 0) ;
	if (ret > 0)
	{	if (ret > len)
		{	len = ret ;
			free (buffer) ;
			if (! (buffer = malloc (len)))
				return SFE_MALLOC_FAILED ;
			} ;
		psf_log_printf (psf, "  Writing LAME info header at offset %d, %d bytes.\n",
			psf->dataoffset, len) ;
		lame_get_lametag_frame (pmpeg->lamef, buffer, len) ;
		pos = psf_ftell (psf) ;
		if (psf_fseek (psf, psf->dataoffset, SEEK_SET) == psf->dataoffset)
		{	psf_fwrite (buffer, 1, ret, psf) ;
			psf_fseek (psf, pos, SEEK_SET) ;
			} ;
		} ;
	free (buffer) ;

	free (pmpeg->block) ;
	pmpeg->block = NULL ;

	if (pmpeg->lamef)
	{	lame_close (pmpeg->lamef) ;
		pmpeg->lamef = NULL ;
		} ;

	return 0 ;
} /* mpeg_l3_encoder_close */

static void
mpeg_l3_encoder_log_config (SF_PRIVATE *psf, lame_t lamef)
{	const char *version ;
	const char *chn_mode ;

	switch (lame_get_version (lamef))
	{	case 0 : version = "2" ; break ;
		case 1 : version = "1" ; break ;
		case 2 : version = "2.5" ; break ;
		default : version = "unknown!?" ; break ;
		} ;
	switch (lame_get_mode (lamef))
	{	case STEREO : chn_mode = "stereo" ; break ;
		case JOINT_STEREO : chn_mode = "joint-stereo" ; break ;
		case MONO : chn_mode = "mono" ; break ;
		default : chn_mode = "unknown!?" ; break ;
		} ;
	psf_log_printf (psf, "  MPEG Version      : %s\n", version) ;
	psf_log_printf (psf, "  Block samples     : %d\n", lame_get_framesize (lamef)) ;
	psf_log_printf (psf, "  Channel mode      : %s\n", chn_mode) ;
	psf_log_printf (psf, "  Samplerate        : %d\n", lame_get_out_samplerate (lamef)) ;
	psf_log_printf (psf, "  Encoder mode      : ") ;
	switch (lame_get_VBR (lamef))
	{	case vbr_off :
			psf_log_printf (psf, "CBR\n") ;
			psf_log_printf (psf, "  Bitrate           : %d kbps\n", lame_get_brate (lamef)) ;
			break ;
		case vbr_abr :
			psf_log_printf (psf, "ABR\n") ;
			psf_log_printf (psf, "  Mean Bitrate      : %d kbps\n", lame_get_VBR_mean_bitrate_kbps (lamef)) ;
			break ;

		case vbr_mt :
		case vbr_default :
			psf_log_printf (psf, "VBR\n") ;
			psf_log_printf (psf, "  Quality           : %d\n", lame_get_VBR_q (lamef)) ;
			break ;

		default:
			psf_log_printf (psf, "Unknown!? (%d)\n", lame_get_VBR (lamef)) ;
			break ;
		} ;

	psf_log_printf (psf, "  Encoder delay     : %d\n", lame_get_encoder_delay (lamef)) ;
	psf_log_printf (psf, "  Write INFO header : %d\n", lame_get_bWriteVbrTag (lamef)) ;
} /* mpeg_l3_encoder_log_config */

static int
mpeg_l3_encoder_construct (SF_PRIVATE *psf)
{	MPEG_L3_ENC_PRIVATE *pmpeg = (MPEG_L3_ENC_PRIVATE *) psf->codec_data ;
	int frame_samples_per_channel ;

	if (pmpeg->initialized == SF_FALSE)
	{	if (lame_init_params (pmpeg->lamef) < 0)
		{	psf_log_printf (psf, "Failed to initialize lame encoder!\n") ;
			return SFE_INTERNAL ;
			} ;

		psf_log_printf (psf, "Initialized LAME encoder.\n") ;
		mpeg_l3_encoder_log_config (psf, pmpeg->lamef) ;

		frame_samples_per_channel = lame_get_framesize (pmpeg->lamef) ;

		/*
		 * Suggested output buffer size in bytes from lame.h comment is
		 * 1.25 * samples + 7200
		 */
		pmpeg->block_len = (frame_samples_per_channel * 4) / 3 + 7200 ;
		pmpeg->frame_samples = frame_samples_per_channel * psf->sf.channels ;

		pmpeg->block = malloc (pmpeg->block_len) ;
		if (!pmpeg->block)
			return SFE_MALLOC_FAILED ;

		pmpeg->initialized = SF_TRUE ;
		} ;

	return 0 ;
} /* mpeg_l3_encoder_construct */

static int
mpeg_l3_encoder_byterate (SF_PRIVATE *psf)
{	MPEG_L3_ENC_PRIVATE *pmpeg = (MPEG_L3_ENC_PRIVATE *) psf->codec_data ;
	int bitrate_mode ;
	int byterate ;
	float calculated_byterate ;

	bitrate_mode = mpeg_l3_encoder_get_bitrate_mode (psf) ;
	byterate = (lame_get_brate (pmpeg->lamef) + 7) / 8 ;

	if (bitrate_mode == SF_BITRATE_MODE_VARIABLE)
	{	/*
		** For VBR, lame_get_brate returns the minimum bitrate, so calculate the
		** average byterate so far.
		*/
		calculated_byterate = psf_ftell (psf) - psf->dataoffset ;
		calculated_byterate /= (float) psf->write_current ;
		calculated_byterate *= (float) psf->sf.samplerate ;

		return SF_MIN (byterate, (int) calculated_byterate) ;
	}

	return byterate ;
} /* mpeg_l3_encoder_byterate */

static sf_count_t
mpeg_l3_encode_write_short_mono (SF_PRIVATE *psf, const short *ptr, sf_count_t len)
{	MPEG_L3_ENC_PRIVATE *pmpeg = (MPEG_L3_ENC_PRIVATE*) psf->codec_data ;
	sf_count_t total = 0 ;
	int nbytes, writecount, writen ;

	if ((psf->error = mpeg_l3_encoder_construct (psf)))
		return 0 ;

	while (len)
	{	writecount = SF_MIN (len, (sf_count_t) pmpeg->frame_samples) ;

		nbytes = lame_encode_buffer (pmpeg->lamef, ptr + total, NULL, writecount, pmpeg->block, pmpeg->block_len) ;
		if (nbytes < 0)
		{	psf_log_printf (psf, "lame_encode_buffer returned %d\n", nbytes) ;
			break ;
			} ;

		if (nbytes)
		{	writen = psf_fwrite (pmpeg->block, 1, nbytes, psf) ;
			if (writen != nbytes)
			{	psf_log_printf (psf, "*** Warning : short write (%d != %d).\n", writen, nbytes) ;
				} ;
			} ;

		total += writecount ;
		len -= writecount ;
		} ;

	return total ;
}


static sf_count_t
mpeg_l3_encode_write_short_stereo (SF_PRIVATE *psf, const short *ptr, sf_count_t len)
{	BUF_UNION ubuf ;
	MPEG_L3_ENC_PRIVATE *pmpeg = (MPEG_L3_ENC_PRIVATE*) psf->codec_data ;
	sf_count_t total = 0 ;
	int nbytes, writecount, writen ;

	if ((psf->error = mpeg_l3_encoder_construct (psf)))
		return 0 ;

	const sf_count_t max_samples = SF_MIN (ARRAY_LEN (ubuf.sbuf), pmpeg->frame_samples) ;
	while (len)
	{	writecount = SF_MIN (len, max_samples) ;
		/*
		 * An oversight, but lame_encode_buffer_interleaved() lacks a const.
		 * As such, need another memcpy to not cause a warning.
		 */
		memcpy (ubuf.sbuf, ptr + total, writecount) ;
		nbytes = lame_encode_buffer_interleaved (pmpeg->lamef, ubuf.sbuf, writecount / 2, pmpeg->block, pmpeg->block_len) ;
		if (nbytes < 0)
		{	psf_log_printf (psf, "lame_encode_buffer returned %d\n", nbytes) ;
			break ;
			} ;

		if (nbytes)
		{	writen = psf_fwrite (pmpeg->block, 1, nbytes, psf) ;
			if (writen != nbytes)
			{	psf_log_printf (psf, "*** Warning : short write (%d != %d).\n", writen, nbytes) ;
				} ;
			} ;

		total += writecount ;
		len -= writecount ;
		} ;

	return total ;
}


static sf_count_t
mpeg_l3_encode_write_int_mono (SF_PRIVATE *psf, const int *ptr, sf_count_t len)
{	MPEG_L3_ENC_PRIVATE *pmpeg = (MPEG_L3_ENC_PRIVATE*) psf->codec_data ;
	sf_count_t total = 0 ;
	int nbytes, writecount, writen ;

	if ((psf->error = mpeg_l3_encoder_construct (psf)))
		return 0 ;

	while (len)
	{	writecount = SF_MIN (len, (sf_count_t) pmpeg->frame_samples) ;

		nbytes = lame_encode_buffer_int (pmpeg->lamef, ptr + total, NULL, writecount, pmpeg->block, pmpeg->block_len) ;
		if (nbytes < 0)
		{	psf_log_printf (psf, "lame_encode_buffer returned %d\n", nbytes) ;
			break ;
			} ;

		if (nbytes)
		{	writen = psf_fwrite (pmpeg->block, 1, nbytes, psf) ;
			if (writen != nbytes)
			{	psf_log_printf (psf, "*** Warning : short write (%d != %d).\n", writen, nbytes) ;
				} ;
			} ;

		total += writecount ;
		len -= writecount ;
		} ;

	return total ;
}


static sf_count_t
mpeg_l3_encode_write_int_stereo (SF_PRIVATE *psf, const int *ptr, sf_count_t len)
{	MPEG_L3_ENC_PRIVATE *pmpeg = (MPEG_L3_ENC_PRIVATE*) psf->codec_data ;
	sf_count_t total = 0 ;
	int nbytes, writecount, writen ;

	if ((psf->error = mpeg_l3_encoder_construct (psf)))
		return 0 ;

	while (len)
	{	writecount = SF_MIN (len, (sf_count_t) pmpeg->frame_samples) ;

		nbytes = lame_encode_buffer_interleaved_int (pmpeg->lamef, ptr + total, writecount / 2, pmpeg->block, pmpeg->block_len) ;
		if (nbytes < 0)
		{	psf_log_printf (psf, "lame_encode_buffer returned %d\n", nbytes) ;
			break ;
			} ;

		if (nbytes)
		{	writen = psf_fwrite (pmpeg->block, 1, nbytes, psf) ;
			if (writen != nbytes)
			{	psf_log_printf (psf, "*** Warning : short write (%d != %d).\n", writen, nbytes) ;
				} ;
			} ;

		total += writecount ;
		len -= writecount ;
		} ;

	return total ;
}


static sf_count_t
mpeg_l3_encode_write_float_mono (SF_PRIVATE *psf, const float *ptr, sf_count_t len)
{	MPEG_L3_ENC_PRIVATE *pmpeg = (MPEG_L3_ENC_PRIVATE*) psf->codec_data ;
	sf_count_t total = 0 ;
	int nbytes, writecount, writen ;

	if ((psf->error = mpeg_l3_encoder_construct (psf)))
		return 0 ;

	while (len)
	{	writecount = SF_MIN (len, (sf_count_t) pmpeg->frame_samples) ;

		if (psf->norm_float)
			nbytes = lame_encode_buffer_ieee_float (pmpeg->lamef, ptr + total, NULL, writecount, pmpeg->block, pmpeg->block_len) ;
		else
			nbytes = lame_encode_buffer_float (pmpeg->lamef, ptr + total, NULL, writecount, pmpeg->block, pmpeg->block_len) ;
		if (nbytes < 0)
		{	psf_log_printf (psf, "lame_encode_buffer returned %d\n", nbytes) ;
			break ;
			} ;

		if (nbytes)
		{	writen = psf_fwrite (pmpeg->block, 1, nbytes, psf) ;
			if (writen != nbytes)
			{	psf_log_printf (psf, "*** Warning : short write (%d != %d).\n", writen, nbytes) ;
				} ;
			} ;

		total += writecount ;
		len -= writecount ;
		} ;

	return total ;
}


static inline void
normalize_float (float *dest, const float *src, sf_count_t count, float norm_fact)
{	while (--count >= 0)
	{	dest [count] = src [count] * norm_fact ;
		} ;
}


static sf_count_t
mpeg_l3_encode_write_float_stereo (SF_PRIVATE *psf, const float *ptr, sf_count_t len)
{	BUF_UNION ubuf ;
	MPEG_L3_ENC_PRIVATE *pmpeg = (MPEG_L3_ENC_PRIVATE*) psf->codec_data ;
	sf_count_t total = 0 ;
	int nbytes, writecount, writen ;

	if ((psf->error = mpeg_l3_encoder_construct (psf)))
		return 0 ;

	const sf_count_t max_samples = SF_MIN (ARRAY_LEN (ubuf.fbuf), pmpeg->frame_samples) ;
	while (len)
	{	writecount = SF_MIN (len, max_samples) ;

		if (psf->norm_float)
			nbytes = lame_encode_buffer_interleaved_ieee_float (pmpeg->lamef, ptr + total, writecount / 2, pmpeg->block, pmpeg->block_len) ;
		else
		{	/* Lame lacks a non-normalized interleaved float write. Bummer. */
			normalize_float (ubuf.fbuf, ptr + total, writecount, 1.0 / (float) 0x8000) ;
			nbytes = lame_encode_buffer_interleaved_ieee_float (pmpeg->lamef, ubuf.fbuf, writecount / 2, pmpeg->block, pmpeg->block_len) ;
			}

		if (nbytes < 0)
		{	psf_log_printf (psf, "lame_encode_buffer returned %d\n", nbytes) ;
			break ;
			} ;

		if (nbytes)
		{	writen = psf_fwrite (pmpeg->block, 1, nbytes, psf) ;
			if (writen != nbytes)
			{	psf_log_printf (psf, "*** Warning : short write (%d != %d).\n", writen, nbytes) ;
				} ;
			} ;

		total += writecount ;
		len -= writecount ;
		} ;

	return total ;
}


static inline void
normalize_double (double *dest, const double *src, sf_count_t count, double norm_fact)
{	while (--count >= 0)
	{	dest [count] = src [count] * norm_fact ;
		} ;
}


static sf_count_t
mpeg_l3_encode_write_double_mono (SF_PRIVATE *psf, const double *ptr, sf_count_t len)
{	BUF_UNION ubuf ;
	MPEG_L3_ENC_PRIVATE *pmpeg = (MPEG_L3_ENC_PRIVATE*) psf->codec_data ;
	sf_count_t total = 0 ;
	int nbytes, writecount, writen ;

	if ((psf->error = mpeg_l3_encoder_construct (psf)))
		return 0 ;

	const sf_count_t max_samples = SF_MIN (ARRAY_LEN (ubuf.dbuf), pmpeg->frame_samples) ;
	while (len)
	{	writecount = SF_MIN (len, max_samples) ;

		if (psf->norm_double)
			nbytes = lame_encode_buffer_ieee_double (pmpeg->lamef, ptr + total, NULL, writecount, pmpeg->block, pmpeg->block_len) ;
		else
		{	/* Lame lacks non-normalized double writing */
			normalize_double (ubuf.dbuf, ptr + total, writecount, 1.0 / (double) 0x8000) ;
			nbytes = lame_encode_buffer_ieee_double (pmpeg->lamef, ubuf.dbuf, NULL, writecount, pmpeg->block, pmpeg->block_len) ;
			}

		if (nbytes < 0)
		{	psf_log_printf (psf, "lame_encode_buffer returned %d\n", nbytes) ;
			break ;
			} ;

		if (nbytes)
		{	writen = psf_fwrite (pmpeg->block, 1, nbytes, psf) ;
			if (writen != nbytes)
			{	psf_log_printf (psf, "*** Warning : short write (%d != %d).\n", writen, nbytes) ;
				} ;
			} ;

		total += writecount ;
		len -= writecount ;
		} ;

	return total ;
}


static sf_count_t
mpeg_l3_encode_write_double_stereo (SF_PRIVATE *psf, const double *ptr, sf_count_t len)
{	BUF_UNION ubuf ;
	MPEG_L3_ENC_PRIVATE *pmpeg = (MPEG_L3_ENC_PRIVATE*) psf->codec_data ;
	sf_count_t total = 0 ;
	int nbytes, writecount, writen ;

	if ((psf->error = mpeg_l3_encoder_construct (psf)))
		return 0 ;

	const sf_count_t max_samples = SF_MIN (ARRAY_LEN (ubuf.dbuf), pmpeg->frame_samples) ;
	while (len)
	{	writecount = SF_MIN (len, max_samples) ;

		if (psf->norm_double)
			nbytes = lame_encode_buffer_interleaved_ieee_double (pmpeg->lamef, ptr + total, writecount / 2, pmpeg->block, pmpeg->block_len) ;
		else
		{	/* Lame lacks interleaved non-normalized double writing */
			normalize_double (ubuf.dbuf, ptr + total, writecount, 1.0 / (double) 0x8000) ;
			nbytes = lame_encode_buffer_interleaved_ieee_double (pmpeg->lamef, ubuf.dbuf, writecount / 2, pmpeg->block, pmpeg->block_len) ;
			}

		if (nbytes < 0)
		{	psf_log_printf (psf, "lame_encode_buffer returned %d\n", nbytes) ;
			break ;
			} ;

		if (nbytes)
		{	writen = psf_fwrite (pmpeg->block, 1, nbytes, psf) ;
			if (writen != nbytes)
			{	psf_log_printf (psf, "*** Warning : short write (%d != %d).\n", writen, nbytes) ;
				} ;
			} ;

		total += writecount ;
		len -= writecount ;
		} ;

	return total ;
}

#else /* HAVE_MPEG */

int
mpeg_l3_encoder_init (SF_PRIVATE *psf, int UNUSED (vbr))
{	psf_log_printf (psf, "This version of libsndfile was compiled without MPEG Layer 3 encoding support.\n") ;
	return SFE_UNIMPLEMENTED ;
} /* mpeg_l3_encoder_init */

#endif
