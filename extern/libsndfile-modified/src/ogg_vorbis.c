/*
** Copyright (C) 2018-2021 Arthur Taylor <art@ified.ca>
** Copyright (C) 2002-2016 Erik de Castro Lopo <erikd@mega-nerd.com>
** Copyright (C) 2002-2005 Michael Smith <msmith@xiph.org>
** Copyright (C) 2007 John ffitch
**
** This program is free software ; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation ; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY ; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program ; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*
** Much of this code is based on the examples in libvorbis from the
** XIPHOPHORUS Company http://www.xiph.org/ which has a BSD-style Licence
** Copyright (c) 2002, Xiph.org Foundation
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** - Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** - Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** - Neither the name of the Xiph.org Foundation nor the names of its
** contributors may be used to endorse or promote products derived from
** this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION
** OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES ; LOSS OF USE,
** DATA, OR PROFITS ; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "sfconfig.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#else
#include "sf_unistd.h"
#endif

#include "sndfile.h"
#include "sfendian.h"
#include "common.h"

#if HAVE_EXTERNAL_XIPH_LIBS

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>

#include "ogg.h"

/* How many seconds in the future to not bother bisection searching for. */
#define VORBIS_SEEK_THRESHOLD 2

typedef int convert_func (SF_PRIVATE *psf, int, void *, int, int, float **) ;

static int	vorbis_read_header (SF_PRIVATE *psf) ;
static int	vorbis_write_header (SF_PRIVATE *psf, int calc_length) ;
static int	vorbis_close (SF_PRIVATE *psf) ;
static int	vorbis_command (SF_PRIVATE *psf, int command, void *data, int datasize) ;
static int	vorbis_byterate (SF_PRIVATE *psf) ;
static int	vorbis_calculate_granulepos (SF_PRIVATE *psf, uint64_t *gp_out) ;
static int	vorbis_skip (SF_PRIVATE *psf, uint64_t target_gp) ;
static int	vorbis_seek_trysearch (SF_PRIVATE *psf, uint64_t target_gp) ;
static sf_count_t	vorbis_seek (SF_PRIVATE *psf, int mode, sf_count_t offset) ;
static sf_count_t	vorbis_read_s (SF_PRIVATE *psf, short *ptr, sf_count_t len) ;
static sf_count_t	vorbis_read_i (SF_PRIVATE *psf, int *ptr, sf_count_t len) ;
static sf_count_t	vorbis_read_f (SF_PRIVATE *psf, float *ptr, sf_count_t len) ;
static sf_count_t	vorbis_read_d (SF_PRIVATE *psf, double *ptr, sf_count_t len) ;
static sf_count_t	vorbis_write_s (SF_PRIVATE *psf, const short *ptr, sf_count_t len) ;
static sf_count_t	vorbis_write_i (SF_PRIVATE *psf, const int *ptr, sf_count_t len) ;
static sf_count_t	vorbis_write_f (SF_PRIVATE *psf, const float *ptr, sf_count_t len) ;
static sf_count_t	vorbis_write_d (SF_PRIVATE *psf, const double *ptr, sf_count_t len) ;
static sf_count_t	vorbis_read_sample (SF_PRIVATE *psf, void *ptr, sf_count_t lens, convert_func *transfn) ;
static int	vorbis_rnull (SF_PRIVATE *psf, int samples, void *vptr, int off , int channels, float **pcm) ;

typedef struct
{	int id ;
	const char *name ;
} STR_PAIRS ;


/* See https://xiph.org/vorbis/doc/v-comment.html */
static STR_PAIRS vorbis_metatypes [] =
{	{	SF_STR_TITLE,		"Title" },
	{	SF_STR_COPYRIGHT,	"Copyright" },
	{	SF_STR_SOFTWARE,	"Software" },
	{	SF_STR_ARTIST,		"Artist" },
	{	SF_STR_COMMENT,		"Comment" },
	{	SF_STR_DATE,		"Date" },
	{	SF_STR_ALBUM,		"Album" },
	{	SF_STR_LICENSE,		"License" },
	{	SF_STR_TRACKNUMBER,	"Tracknumber" },
	{	SF_STR_GENRE,		"Genre" },
} ;

typedef struct
{	/* Current granule position. */
	uint64_t gp ;
	/* Struct that stores all the static vorbis bitstream settings */
	vorbis_info	vinfo ;
	/* Struct that stores all the bitstream user comments */
	vorbis_comment vcomment ;
	/* Central working state for the packet->PCM decoder */
	vorbis_dsp_state vdsp ;
	/* Local working space for packet->PCM decode */
	vorbis_block vblock ;
	/* Encoding quality in range [0.0, 1.0]. */
	double quality ;
	/* Offset of the first samples' granule position. */
	uint64_t pcm_start ;
	/* Last valid samples' granule position. */
	uint64_t pcm_end ;
	/* File offset of the start of the last page. */
	sf_count_t last_page ;
} VORBIS_PRIVATE ;

static int
vorbis_read_header (SF_PRIVATE *psf)
{	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	int printed_metadata_msg = 0 ;
	int i, nn ;
	sf_count_t last_page ;
	sf_count_t saved_offset ;

	/*
	**  The first page of the Ogg stream we are told to try and open as Vorbis
	**  has already been loaded into odata->ostream by ogg_open().
	**
	**	Extract the initial header from the first page and verify that the
	**	Ogg bitstream is in fact Vorbis data.
	*/

	vorbis_info_init (&vdata->vinfo) ;
	vorbis_comment_init (&vdata->vcomment) ;

	if (!odata->opacket.b_o_s)
	{	psf_log_printf (psf, "Vorbis: First packet does not have a beginning-of-stream bit.\n") ;
		return SFE_MALFORMED_FILE ;
		}

	if (ogg_stream_packetpeek (&odata->ostream, NULL))
	{	psf_log_printf (psf, "Vorbis: First page contains extraneous packets!\n") ;
		return SFE_MALFORMED_FILE ;
		}

	if (vorbis_synthesis_headerin (&vdata->vinfo, &vdata->vcomment, &odata->opacket) < 0)
	{	/* Error case ; not a vorbis header. */
		psf_log_printf (psf, "Found Vorbis in stream header, but vorbis_synthesis_headerin failed.\n") ;
		return SFE_MALFORMED_FILE ;
		} ;

	/*
	**	At this point, we're sure we're Vorbis.	We've set up the logical (Ogg)
	**	bitstream decoder. Get the comment and codebook headers and set up the
	**	Vorbis decoder.
	**
	**	The next two packets in order are the comment and codebook headers.
	**	They're likely large and may span multiple pages.  Thus we read
	**	and submit data until we get our two packets, watching that no
	**	pages are missing.  If a page is missing, error out ; losing a
	**	header page is the only place where missing data is fatal.
	*/

	i = 0 ;			/* Count of number of packets read */
	while (i < 2)
	{	nn = ogg_stream_packetout (&odata->ostream, &odata->opacket) ;

		if (nn == 0)
		{	nn = ogg_stream_next_page (psf, odata) ;
			if (nn == 0)
			{	psf_log_printf (psf, "End of file before finding all Vorbis headers!\n") ;
				return SFE_MALFORMED_FILE ;
				} ;
			if (nn == -1)
			{	psf_log_printf (psf, "Error reading file while finding Vorbis headers!\n") ;
				return psf->error ;
				} ;
			continue ;
			}

		if (nn < 0)
		{	/* A hole while reading headers. This could be bad. */
			psf_log_printf (psf, "Corrupt secondary header.	Exiting.\n") ;
			return SFE_MALFORMED_FILE ;
			} ;

		vorbis_synthesis_headerin (&vdata->vinfo, &vdata->vcomment, &odata->opacket) ;
		i++ ;
		} ;

	/* Check for extraneous packets in the last headers page. */
	while (ogg_stream_packetout (&odata->ostream, &odata->opacket) == 1)
	{	i++ ;
		}
	if (i > 2)
		psf_log_printf (psf, "Vorbis: stream has extraneous header packets.\n") ;

	psf_log_printf (psf, "Bitstream is %d channel, %D Hz\n", vdata->vinfo.channels, vdata->vinfo.rate) ;
	psf_log_printf (psf, "Encoded by : %s\n", vdata->vcomment.vendor) ;

	/* Save the offset of the first payload page */
	psf->dataoffset	= ogg_sync_ftell (psf) ;

	/*
	**	Calculate the granule position offset. The first page with a payload
	**	packet shouldn't end in a continued packet. The difference between the
	**	page's granule position and the sum of frames on the page tells us the
	**	granule position offset.
	**	See https://xiph.org/vorbis/doc/Vorbis_I_spec.html#x1-132000A.2
	*/
	ogg_stream_unpack_page (psf, odata) ;
	vorbis_calculate_granulepos (psf, &vdata->pcm_start) ;
	vdata->gp = vdata->pcm_start ;

	/*
	**	Find the end of the stream, save it. Only works if the file is seekable.
	*/
	vdata->pcm_end = (uint64_t) -1 ;
	psf->datalength = psf->filelength ;
	if (!psf->is_pipe)
	{	saved_offset = ogg_sync_ftell (psf) ;
		last_page = ogg_sync_last_page_before (psf, odata, &vdata->pcm_end, psf->filelength, odata->ostream.serialno) ;
		if (last_page > 0)
		{	if (!ogg_page_eos (&odata->opage))
				psf_log_printf (psf, "Ogg: Last page lacks an end-of-stream bit.\n") ;
			psf->datalength = last_page + odata->opage.header_len + odata->opage.body_len - psf->dataoffset ;
			if (psf->datalength + psf->dataoffset < psf->filelength)
				psf_log_printf (psf, "Ogg: Junk after the last page.\n") ;
			vdata->last_page = last_page ;
			} ;

		ogg_sync_fseek (psf, saved_offset, SEEK_SET) ;
		}

	psf_log_printf (psf, "PCM offset  : %D\n", vdata->pcm_start) ;
	if (vdata->pcm_end != (uint64_t) -1)
		psf_log_printf (psf, "PCM end     : %D\n", vdata->pcm_end) ;
	else
		psf_log_printf (psf, "PCM end     : unknown\n") ;

	/* Throw the comments plus a few lines about the bitstream we're decoding. */
	for (i = 0 ; i < ARRAY_LEN (vorbis_metatypes) ; i++)
	{	char *dd ;

		dd = vorbis_comment_query (&vdata->vcomment, vorbis_metatypes [i].name, 0) ;
		if (dd == NULL)
			continue ;

		if (printed_metadata_msg == 0)
		{	psf_log_printf (psf, "Metadata :\n") ;
			printed_metadata_msg = 1 ;
			} ;

		psf_store_string (psf, vorbis_metatypes [i].id, dd) ;
		psf_log_printf (psf, "  %-10s : %s\n", vorbis_metatypes [i].name, dd) ;
		} ;
	psf_log_printf (psf, "End\n") ;

	psf->sf.samplerate	= vdata->vinfo.rate ;
	psf->sf.channels	= vdata->vinfo.channels ;
	psf->sf.format		= SF_FORMAT_OGG | SF_FORMAT_VORBIS ;
	psf->sf.frames		= (vdata->pcm_end != (uint64_t) -1) ? vdata->pcm_end - vdata->pcm_start : SF_COUNT_MAX ;

	/*	OK, got and parsed all three headers. Initialize the Vorbis
	**	packet->PCM decoder.
	**	Central decode state. */
	vorbis_synthesis_init (&vdata->vdsp, &vdata->vinfo) ;

	/*	Local state for most of the decode so multiple block decodes can
	**	proceed in parallel. We could init multiple vorbis_block structures
	**	for vdsp here. */
	vorbis_block_init (&vdata->vdsp, &vdata->vblock) ;

	return 0 ;
} /* vorbis_read_header */

static int
vorbis_write_header (SF_PRIVATE *psf, int UNUSED (calc_length))
{
	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	int k, ret ;

	vorbis_info_init (&vdata->vinfo) ;

	/* The style of encoding should be selectable here, VBR quality mode. */
	ret = vorbis_encode_init_vbr (&vdata->vinfo, psf->sf.channels, psf->sf.samplerate, vdata->quality) ;

#if 0
	ret = vorbis_encode_init (&vdata->vinfo, psf->sf.channels, psf->sf.samplerate, -1, 128000, -1) ; /* average bitrate mode */
	ret = (	vorbis_encode_setup_managed (&vdata->vinfo, psf->sf.channels, psf->sf.samplerate, -1, 128000, -1)
			|| vorbis_encode_ctl (&vdata->vinfo, OV_ECTL_RATEMANAGE_AVG, NULL)
			|| vorbis_encode_setup_init (&vdata->vinfo)
			) ;
#endif
	if (ret)
		return SFE_BAD_OPEN_FORMAT ;

	vdata->gp = 0 ;

	/* add a comment */
	vorbis_comment_init (&vdata->vcomment) ;

	vorbis_comment_add_tag (&vdata->vcomment, "ENCODER", "libsndfile") ;
	for (k = 0 ; k < SF_MAX_STRINGS ; k++)
	{	const char * name ;

		if (psf->strings.data [k].type == 0)
			break ;

		switch (psf->strings.data [k].type)
		{	case SF_STR_TITLE :			name = "TITLE" ; break ;
			case SF_STR_COPYRIGHT :		name = "COPYRIGHT" ; break ;
			case SF_STR_SOFTWARE :		name = "SOFTWARE" ; break ;
			case SF_STR_ARTIST :		name = "ARTIST" ; break ;
			case SF_STR_COMMENT :		name = "COMMENT" ; break ;
			case SF_STR_DATE :			name = "DATE" ; break ;
			case SF_STR_ALBUM :			name = "ALBUM" ; break ;
			case SF_STR_LICENSE :		name = "LICENSE" ; break ;
			case SF_STR_TRACKNUMBER :	name = "Tracknumber" ; break ;
			case SF_STR_GENRE :			name = "Genre" ; break ;

			default : continue ;
			} ;

		vorbis_comment_add_tag (&vdata->vcomment, name, psf->strings.storage + psf->strings.data [k].offset) ;
		} ;

	/* set up the analysis state and auxiliary encoding storage */
	vorbis_analysis_init (&vdata->vdsp, &vdata->vinfo) ;
	vorbis_block_init (&vdata->vdsp, &vdata->vblock) ;

	/*
	**	Set up our packet->stream encoder.
	**	Pick a random serial number ; that way we can more likely build
	**	chained streams just by concatenation.
	*/

	ogg_stream_init (&odata->ostream, psf_rand_int32 ()) ;

	/* Vorbis streams begin with three headers ; the initial header (with
	   most of the codec setup parameters) which is mandated by the Ogg
	   bitstream spec.  The second header holds any comment fields.	 The
	   third header holds the bitstream codebook.  We merely need to
	   make the headers, then pass them to libvorbis one at a time ;
	   libvorbis handles the additional Ogg bitstream constraints */

	{	ogg_packet header ;
		ogg_packet header_comm ;
		ogg_packet header_code ;
		int result ;

		vorbis_analysis_headerout (&vdata->vdsp, &vdata->vcomment, &header, &header_comm, &header_code) ;
		ogg_stream_packetin (&odata->ostream, &header) ; /* automatically placed in its own page */
		ogg_stream_packetin (&odata->ostream, &header_comm) ;
		ogg_stream_packetin (&odata->ostream, &header_code) ;

		/* This ensures the actual
		 * audio data will start on a new page, as per spec
		 */
		while ((result = ogg_stream_flush (&odata->ostream, &odata->opage)) != 0)
		{	ogg_write_page (psf, &odata->opage) ;
			} ;
	}

	return 0 ;
} /* vorbis_write_header */

static int
vorbis_close (SF_PRIVATE *psf)
{	OGG_PRIVATE* odata = psf->container_data ;
	VORBIS_PRIVATE *vdata = psf->codec_data ;

	if (odata == NULL || vdata == NULL)
		return 0 ;

	/*	Clean up this logical bitstream ; before exit we shuld see if we're
	**	followed by another [chained]. */

	if (psf->file.mode == SFM_WRITE)
	{
		if (psf->write_current <= 0)
			vorbis_write_header (psf, 0) ;

		vorbis_analysis_wrote (&vdata->vdsp, 0) ;
		while (vorbis_analysis_blockout (&vdata->vdsp, &vdata->vblock) == 1)
		{

		/* analysis, assume we want to use bitrate management */
			vorbis_analysis (&vdata->vblock, NULL) ;
			vorbis_bitrate_addblock (&vdata->vblock) ;

			while (vorbis_bitrate_flushpacket (&vdata->vdsp, &odata->opacket))
			{	/* weld the packet into the bitstream */
				ogg_stream_packetin (&odata->ostream, &odata->opacket) ;

				/* write out pages (if any) */
				while (!odata->eos)
				{	int result = ogg_stream_pageout (&odata->ostream, &odata->opage) ;
					if (result == 0) break ;
					ogg_write_page (psf, &odata->opage) ;

		/* this could be set above, but for illustrative purposes, I do
		   it here (to show that vorbis does know where the stream ends) */

					if (ogg_page_eos (&odata->opage)) odata->eos = 1 ;
				}
			}
		}
	}

	/* ogg_page and ogg_packet structs always point to storage in
	   libvorbis.  They are never freed or manipulated directly */

	vorbis_block_clear (&vdata->vblock) ;
	vorbis_dsp_clear (&vdata->vdsp) ;
	vorbis_comment_clear (&vdata->vcomment) ;
	vorbis_info_clear (&vdata->vinfo) ;

	return 0 ;
} /* vorbis_close */

int
ogg_vorbis_open (SF_PRIVATE *psf)
{	OGG_PRIVATE* odata = psf->container_data ;
	VORBIS_PRIVATE* vdata ;
	int	error = 0 ;

	if (odata == NULL)
	{	psf_log_printf (psf, "%s : odata is NULL???\n", __func__) ;
		return SFE_INTERNAL ;
		} ;

	vdata = calloc (1, sizeof (VORBIS_PRIVATE)) ;
	psf->codec_data = vdata ;

	if (psf->file.mode == SFM_RDWR)
		return SFE_BAD_MODE_RW ;

	psf_log_printf (psf, "Vorbis library version : %s\n", vorbis_version_string ()) ;

	if (psf->file.mode == SFM_READ)
	{	if ((error = vorbis_read_header (psf)))
			return error ;

		psf->read_short		= vorbis_read_s ;
		psf->read_int		= vorbis_read_i ;
		psf->read_float		= vorbis_read_f ;
		psf->read_double	= vorbis_read_d ;
		} ;

	psf->codec_close = vorbis_close ;
	if (psf->file.mode == SFM_WRITE)
	{
		/* Set the default vorbis quality here. */
		vdata->quality = 0.4 ;

		psf->write_header	= vorbis_write_header ;
		psf->write_short	= vorbis_write_s ;
		psf->write_int		= vorbis_write_i ;
		psf->write_float	= vorbis_write_f ;
		psf->write_double	= vorbis_write_d ;

		psf->sf.frames = 0 ;
		psf->datalength = 0 ;
		psf->filelength = 0 ;
		psf->dataoffset = 0 ;
		psf->strings.flags = SF_STR_ALLOW_START ;
		} ;

	psf->seek = vorbis_seek ;
	psf->command = vorbis_command ;
	psf->byterate = vorbis_byterate ;
	psf->sf.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS ;
	psf->sf.sections = 1 ;

	return error ;
} /* ogg_vorbis_open */

static int
vorbis_command (SF_PRIVATE *psf, int command, void * data, int datasize)
{	OGG_PRIVATE* odata = psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;

	switch (command)
	{	case SFC_SET_COMPRESSION_LEVEL :
			if (data == NULL || datasize != sizeof (double))
				return SF_FALSE ;

			if (psf->have_written)
				return SF_FALSE ;

			vdata->quality = 1.0 - *((double *) data) ;

			/* Clip range. */
			vdata->quality = SF_MAX (0.0, SF_MIN (1.0, vdata->quality)) ;

			psf_log_printf (psf, "%s : Setting SFC_SET_VBR_ENCODING_QUALITY to %f.\n", __func__, vdata->quality) ;
			return SF_TRUE ;

		case SFC_GET_OGG_STREAM_SERIALNO :
			if (data == NULL || datasize != sizeof (int32_t))
				return SF_FALSE ;

			*((int32_t *) data) = odata->ostream.serialno ;
			return SF_TRUE ;

		default :
			return SF_FALSE ;
		} ;

	return SF_FALSE ;
} /* vorbis_command */

static int
vorbis_rnull (SF_PRIVATE *UNUSED (psf), int samples, void *UNUSED (vptr), int UNUSED (off) , int channels, float **UNUSED (pcm))
{
	return samples * channels ;
} /* vorbis_rnull */

static int
vorbis_rshort (SF_PRIVATE *psf, int samples, void *vptr, int off, int channels, float **pcm)
{
	short *ptr = (short*) vptr + off ;
	int i = 0, j, n ;
	if (psf->float_int_mult)
	{
		float inverse = 1.0 / psf->float_max ;
		for (j = 0 ; j < samples ; j++)
			for (n = 0 ; n < channels ; n++)
				ptr [i++] = psf_lrintf ((pcm [n][j] * inverse) * 32767.0f) ;
	}
	else
	{
		for (j = 0 ; j < samples ; j++)
			for (n = 0 ; n < channels ; n++)
				ptr [i++] = psf_lrintf (pcm [n][j] * 32767.0f) ;
	}
	return i ;
} /* vorbis_rshort */

static int
vorbis_rint (SF_PRIVATE *psf, int samples, void *vptr, int off, int channels, float **pcm)
{
	int *ptr = (int*) vptr + off ;
	int i = 0, j, n ;

	if (psf->float_int_mult)
	{
		float inverse = 1.0 / psf->float_max ;
		for (j = 0 ; j < samples ; j++)
			for (n = 0 ; n < channels ; n++)
				ptr [i++] = psf_lrintf ((pcm [n][j] * inverse) * 2147483647.0f) ;
	}
	else
	{
		for (j = 0 ; j < samples ; j++)
			for (n = 0 ; n < channels ; n++)
				ptr [i++] = psf_lrintf (pcm [n][j] * 2147483647.0f) ;
	}
	return i ;
} /* vorbis_rint */

static int
vorbis_rfloat (SF_PRIVATE *UNUSED (psf), int samples, void *vptr, int off, int channels, float **pcm)
{
	float *ptr = (float*) vptr + off ;
	int i = 0, j, n ;
	for (j = 0 ; j < samples ; j++)
		for (n = 0 ; n < channels ; n++)
			ptr [i++] = pcm [n][j] ;
	return i ;
} /* vorbis_rfloat */

static int
vorbis_rdouble (SF_PRIVATE *UNUSED (psf), int samples, void *vptr, int off, int channels, float **pcm)
{
	double *ptr = (double*) vptr + off ;
	int i = 0, j, n ;
	for (j = 0 ; j < samples ; j++)
		for (n = 0 ; n < channels ; n++)
			ptr [i++] = pcm [n][j] ;
	return i ;
} /* vorbis_rdouble */


static sf_count_t
vorbis_read_sample (SF_PRIVATE *psf, void *ptr, sf_count_t lens, convert_func *transfn)
{	VORBIS_PRIVATE *vdata = psf->codec_data ;
	OGG_PRIVATE *odata = psf->container_data ;
	int len, samples, i = 0 , nn ;
	float **pcm ;

	len = lens / psf->sf.channels ;

	while (len > 0)
	{	/*
		** pcm is a multichannel float vector.	 In stereo, for
		** example, pcm [0] is left, and pcm [1] is right.	 samples is
		** the size of each channel.	 Convert the float values
		** (-1.<=range<=1.) to whatever PCM format and write it out.
		*/
		while ((samples = vorbis_synthesis_pcmout (&vdata->vdsp, &pcm)) > 0)
		{	if (samples > len) samples = len ;
			i += transfn (psf, samples, ptr, i, psf->sf.channels, pcm) ;
			len -= samples ;
			/* tell libvorbis how many samples we actually consumed */
			vorbis_synthesis_read (&vdata->vdsp, samples) ;
			vdata->gp += samples ;
			if (len == 0)
				return i ;
			} ;

		/* Out of samples, load the next packet. */
		if (odata->pkt_indx == odata->pkt_len)
		{	/* Page out of packets, load and unpack the next page. */
			nn = ogg_stream_unpack_page (psf, odata) ;
			if (nn <= 0)
				return i ;
			if (nn == 2)
			{	/* Ran over a hole. gp is now out of date, need to recalculate. */
				vorbis_synthesis_restart (&vdata->vdsp) ;
				vorbis_calculate_granulepos (psf, &vdata->gp) ;
				}
			} ;

		/* Decode the packet */
		if (vorbis_synthesis (&vdata->vblock, &(odata->pkt [odata->pkt_indx])) == 0) /* test for success! */
			vorbis_synthesis_blockin (&vdata->vdsp, &vdata->vblock) ;
		odata->pkt_indx++ ;
		} ;

	return i ;
} /* vorbis_read_sample */

static sf_count_t
vorbis_read_s (SF_PRIVATE *psf, short *ptr, sf_count_t lens)
{	return vorbis_read_sample (psf, (void*) ptr, lens, vorbis_rshort) ;
} /* vorbis_read_s */

static sf_count_t
vorbis_read_i (SF_PRIVATE *psf, int *ptr, sf_count_t lens)
{	return vorbis_read_sample (psf, (void*) ptr, lens, vorbis_rint) ;
} /* vorbis_read_i */

static sf_count_t
vorbis_read_f (SF_PRIVATE *psf, float *ptr, sf_count_t lens)
{	return vorbis_read_sample (psf, (void*) ptr, lens, vorbis_rfloat) ;
} /* vorbis_read_f */

static sf_count_t
vorbis_read_d (SF_PRIVATE *psf, double *ptr, sf_count_t lens)
{	return vorbis_read_sample (psf, (void*) ptr, lens, vorbis_rdouble) ;
} /* vorbis_read_d */

/*==============================================================================
*/

static void
vorbis_write_samples (SF_PRIVATE *psf, OGG_PRIVATE *odata, VORBIS_PRIVATE *vdata, int in_frames)
{
	vorbis_analysis_wrote (&vdata->vdsp, in_frames) ;

	/*
	**	Vorbis does some data preanalysis, then divvies up blocks for
	**	more involved (potentially parallel) processing. Get a single
	**	block for encoding now.
	*/
	while (vorbis_analysis_blockout (&vdata->vdsp, &vdata->vblock) == 1)
	{
		/* analysis, assume we want to use bitrate management */
		vorbis_analysis (&vdata->vblock, NULL) ;
		vorbis_bitrate_addblock (&vdata->vblock) ;

		while (vorbis_bitrate_flushpacket (&vdata->vdsp, &odata->opacket))
		{
			/* weld the packet into the bitstream */
			ogg_stream_packetin (&odata->ostream, &odata->opacket) ;

			/* write out pages (if any) */
			while (!odata->eos)
			{	int result = ogg_stream_pageout (&odata->ostream, &odata->opage) ;
				if (result == 0)
					break ;
				ogg_write_page (psf, &odata->opage) ;

				/*	This could be set above, but for illustrative purposes, I do
				**	it here (to show that vorbis does know where the stream ends) */
				if (ogg_page_eos (&odata->opage))
					odata->eos = 1 ;
				} ;
			} ;
		} ;

	vdata->gp += in_frames ;
} /* vorbis_write_data */


static sf_count_t
vorbis_write_s (SF_PRIVATE *psf, const short *ptr, sf_count_t lens)
{
	int i, m, j = 0 ;
	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	int in_frames = lens / psf->sf.channels ;
	float **buffer = vorbis_analysis_buffer (&vdata->vdsp, in_frames) ;
	for (i = 0 ; i < in_frames ; i++)
		for (m = 0 ; m < psf->sf.channels ; m++)
			buffer [m][i] = (float) (ptr [j++]) / 32767.0f ;

	vorbis_write_samples (psf, odata, vdata, in_frames) ;

	return lens ;
} /* vorbis_write_s */

static sf_count_t
vorbis_write_i (SF_PRIVATE *psf, const int *ptr, sf_count_t lens)
{	int i, m, j = 0 ;
	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	int in_frames = lens / psf->sf.channels ;
	float **buffer = vorbis_analysis_buffer (&vdata->vdsp, in_frames) ;
	for (i = 0 ; i < in_frames ; i++)
		for (m = 0 ; m < psf->sf.channels ; m++)
			buffer [m][i] = (float) (ptr [j++]) / 2147483647.0f ;

	vorbis_write_samples (psf, odata, vdata, in_frames) ;

	return lens ;
} /* vorbis_write_i */

static sf_count_t
vorbis_write_f (SF_PRIVATE *psf, const float *ptr, sf_count_t lens)
{	int i, m, j = 0 ;
	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	int in_frames = lens / psf->sf.channels ;
	float **buffer = vorbis_analysis_buffer (&vdata->vdsp, in_frames) ;
	for (i = 0 ; i < in_frames ; i++)
		for (m = 0 ; m < psf->sf.channels ; m++)
			buffer [m][i] = ptr [j++] ;

	vorbis_write_samples (psf, odata, vdata, in_frames) ;

	return lens ;
} /* vorbis_write_f */

static sf_count_t
vorbis_write_d (SF_PRIVATE *psf, const double *ptr, sf_count_t lens)
{	int i, m, j = 0 ;
	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	int in_frames = lens / psf->sf.channels ;
	float **buffer = vorbis_analysis_buffer (&vdata->vdsp, in_frames) ;
	for (i = 0 ; i < in_frames ; i++)
		for (m = 0 ; m < psf->sf.channels ; m++)
			buffer [m][i] = (float) ptr [j++] ;

	vorbis_write_samples (psf, odata, vdata, in_frames) ;

	return lens ;
} /* vorbis_write_d */

static int
vorbis_skip (SF_PRIVATE *psf, uint64_t target)
{	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	ogg_packet *pkt ;
	int thisblock, lastblock, nn ;
	const int blocksize = vorbis_info_blocksize (&vdata->vinfo, 1) ;

	/*	Read out any samples that may be in the decoder from a seek without a
	**	search. */
	thisblock = vorbis_synthesis_pcmout (&vdata->vdsp, NULL) ;
	if (thisblock > 0)
	{	if ((uint64_t) thisblock + vdata->gp >= target)
			thisblock = SF_MIN (thisblock, (int) (target - vdata->gp)) ;

		vorbis_synthesis_read (&vdata->vdsp, thisblock) ;
		vdata->gp += thisblock ;
		if (vdata->gp == target)
			return 0 ;
		} ;

	/* Read through packets that are before our target */
	lastblock = 0 ;
	for ( ; vdata->gp < target ; )
	{	/* Ensure there are unpacked packets. */
		if (odata->pkt_indx == odata->pkt_len)
		{	/* Page out of packets, load and unpack the next page. */
			nn = ogg_stream_unpack_page (psf, odata) ;
			if (nn < 0)
				return nn ;
			if (nn == 0)
				break ;
			if (nn == 2)
			{	/* Ran over a hole. gp is now out of date, need to recalculate. */
				vorbis_synthesis_restart (&vdata->vdsp) ;
				vorbis_calculate_granulepos (psf, &vdata->gp) ;
				if (target < vdata->gp)
				{	/* Our target is inside the hole :-( */
					return 0 ;
					} ;
				} ;
			} ;

		pkt = &odata->pkt [odata->pkt_indx] ;
		thisblock = vorbis_packet_blocksize (&vdata->vinfo, pkt) ;
		if (thisblock < 0)
		{	/* Not an audio packet */
			odata->pkt_indx++ ;
			continue ;
			} ;

		if (lastblock)
		{	vdata->gp += ((lastblock + thisblock) / 4) ;
			} ;

		/* Check to see if the block contains our target */
		if (vdata->gp + ((thisblock + blocksize) / 4) >= target)
			break ;

		/* Block is before the target. Track for state, but don't decode. */
		odata->pkt_indx++ ;
		vorbis_synthesis_trackonly (&vdata->vblock, pkt) ;
		vorbis_synthesis_blockin (&vdata->vdsp, &vdata->vblock) ;
		lastblock = thisblock ;
		} ;

	/*	We are at the correct block, but still need to consume samples to reach
	**	our target. */
	vorbis_read_sample (psf, (void *) NULL, (target - vdata->gp) * psf->sf.channels, vorbis_rnull) ;

	return 0 ;
} /* vorbis_skip */

static int
vorbis_seek_trysearch (SF_PRIVATE *psf, uint64_t target_gp)
{	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	uint64_t best_gp, search_target_gp ;
	int ret ;

	/* Can't bisect a file we don't know the end of (cannot seek). */
	if (vdata->pcm_end == (uint64_t) -1)
		return 0 ;

	/* If the target is for the near future, don't bother bisecting, just skip
	** to it. */
	if (target_gp >= vdata->gp &&
		target_gp - vdata->gp < ((unsigned) (VORBIS_SEEK_THRESHOLD) * psf->sf.samplerate))
		return 0 ;

	/*	Search for a position a half large-block before our target. As Vorbis is
	**	lapped, every sample position come from two blocks, the "left" half of
	**	one block and the "right" half of the previous block.  The granule
	**	position of an Ogg page of a Vorbis stream is the sample offset of the
	**	last finished sample in the stream that can be decoded from a page.  A
	**	page also contains another half-block of samples waiting to be lapped
	**	with the first half-block of samples from the next page.
	**
	**	Searching for a sample one half of a large block before our target
	**	guarantees we always load a page containing the previous half block
	**	required to decode the target.  Downside is we can't use best_gp
	**	parameter of the page seek function. */
	search_target_gp = vorbis_info_blocksize (&vdata->vinfo, 1) / 2 ;
	search_target_gp = search_target_gp < target_gp ? target_gp - search_target_gp : 0 ;

	ret = ogg_stream_seek_page_search (psf, odata, search_target_gp, vdata->pcm_start,
			vdata->pcm_end, &best_gp, psf->dataoffset, vdata->last_page, vdata->vinfo.rate) ;
	if (ret < 0)
		return ret ;

	ret = ogg_stream_unpack_page (psf, odata) ;
	if (ret > 0)
	{	/* Reset the decoder, recalculate position */
		vorbis_synthesis_restart (&vdata->vdsp) ;
		ret = vorbis_calculate_granulepos (psf, &vdata->gp) ;
		} ;

	return ret ;
} /* vorbis_seek_trysearch */

static sf_count_t
vorbis_seek (SF_PRIVATE *psf, int UNUSED (mode), sf_count_t offset)
{	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	uint64_t target_gp ;
	int ret ;

	if (odata == NULL || vdata == NULL)
		return 0 ;

	if (offset < 0)
	{	psf->error = SFE_BAD_SEEK ;
		return ((sf_count_t) -1) ;
		} ;

	if (psf->file.mode == SFM_READ)
	{	target_gp = (uint64_t) offset + vdata->pcm_start ;

		ret = vorbis_seek_trysearch (psf, target_gp) ;

		if (ret < 0 || vdata->gp > target_gp)
		{	/* Search failed (bad data?), reset to the beginning of the stream. */
			psf_log_printf (psf, "Vorbis: Seek search failed. Reading through stream from start.\n") ;
			ogg_stream_reset_serialno (&odata->ostream, odata->ostream.serialno) ;
			odata->pkt_len = 0 ;
			odata->pkt_indx = 0 ;
			ogg_sync_fseek (psf, psf->dataoffset, SEEK_SET) ;
			vdata->gp = vdata->pcm_start ;
			vorbis_synthesis_restart (&vdata->vdsp) ;
			} ;

		vorbis_skip (psf, target_gp) ;

		return vdata->gp - vdata->pcm_start ;
		} ;

	psf->error = SFE_BAD_SEEK ;
	return ((sf_count_t) -1) ;
} /* vorbis_seek */

static int
vorbis_byterate (SF_PRIVATE *psf)
{
	if (psf->file.mode == SFM_READ)
		return (psf->datalength * psf->sf.samplerate) / psf->sf.frames ;

	return -1 ;
} /* vorbis_byterate */

static int
vorbis_calculate_granulepos (SF_PRIVATE *psf, uint64_t *gp_out)
{	OGG_PRIVATE *odata = (OGG_PRIVATE *) psf->container_data ;
	VORBIS_PRIVATE *vdata = (VORBIS_PRIVATE *) psf->codec_data ;
	ogg_packet *pkt ;
	uint64_t last_gp ;
	int thisblock, lastblock, i ;
	unsigned duration ;

	/*	Calculate the granule position when dropped into the middle of a stream
	**	with an un-primed decoder.
	**
	**	Normally the last unpacked packet contains the granule position of the
	**	last completed sample from decoding all the blocks in the page's
	**	packets.  By calculating how many samples we can decode from the blocks
	**	in the page's packets and subtracting it from the final packet's granule
	**	position we get the position of the first sample to be output from the
	**	decoder after it primes.  That is, the current granule position.
	**
	**	However, there is an ambiguity if this is the last page of a stream. The
	**	last page of a stream may have a granule position of fewer samples than
	**	the page actually contains.  The excess samples are padding leftovers
	**	for and exact sample length file. */

	if (odata->pkt_len > 0)
	{	/* Calculate how many samples can be decoded from blocks in this page,
		** accounting for the fact that blocks are 1/2 lapped. */
		lastblock = -1 ;
		duration = 0 ;
		pkt = odata->pkt ;
		for (i = 0 ; i < odata->pkt_len ; i++)
		{	thisblock = vorbis_packet_blocksize (&vdata->vinfo, &pkt [i]) ;
			if (thisblock >= 0)
			{	if (lastblock != -1)
					duration += (lastblock + thisblock) >> 2 ;
				lastblock = thisblock ;
				} ;
			} ;

		pkt = &odata->pkt [odata->pkt_len - 1] ;
		last_gp = pkt->granulepos ;
		if (last_gp == (uint64_t) -1)
		{	psf_log_printf (psf, "Vorbis: Ogg page has no granule position, cannot calculate sample position!\n") ;
			psf->error = SFE_MALFORMED_FILE ;
			return -1 ;
			} ;

		if (pkt->e_o_s)
		{	if (last_gp <= duration)
			{	/*	Corner case: One page stream. Ogg/Vorbis spec dictates the
				**	granule position offset MUST be zero, hence this first (and
				**	only) page must start at 0. */
				*gp_out = 0 ;
				return 1 ;
				} ;

			/*	Otherwise, we cannot know where we are without looking at the
			**	blocks of the previous page.  (The granule position of the
			**	previous page is not enough, we need the block sizes.)
			**
			**	We avoid this case by never allowing a bisection search to seek
			**	beyond the second-to-last page, so the last page is always
			**	approached with a known location and never dropped into.
			**
			**	The only way we should be able to end up here is if there was a
			**	hole in stream just before the last page, in which case all bets
			**	are off anyways. */
			psf_log_printf (psf, "Vorbis: Cannot calculate ambiguous last page duration. Sample count may be wrong.\n") ;
			} ;

		if (last_gp < duration)
		{	psf_log_printf (psf, "Vorbis: Granule position is nonsensical! (Missing end-of-stream marker?)\n") ;
			psf->error = SFE_MALFORMED_FILE ;
			return -1 ;
			} ;

		*gp_out = last_gp - duration ;
		return 1 ;
		} ;

	return 0 ;
} /* vorbis_calculate_granulepos */

#else /* HAVE_EXTERNAL_XIPH_LIBS */

int
ogg_vorbis_open	(SF_PRIVATE *psf)
{
	psf_log_printf (psf, "This version of libsndfile was compiled without Ogg/Vorbis support.\n") ;
	return SFE_UNIMPLEMENTED ;
} /* ogg_vorbis_open */

#endif
