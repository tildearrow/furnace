/*
** Copyright (C) 2019 Erik de Castro Lopo <erikd@mega-nerd.com>
** Copyright (C) 2019 Arthur Taylor <art@ified.ca>
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

#ifndef SNDFILE_MPEG_H
#define SNDFILE_MPEG_H

#include	"common.h"

int mpeg_decoder_init (SF_PRIVATE *psf) ;

/*
** Get the file bitrate mode, returning one of the SF_BITRATE_MODE_ enum
** values. Purely informative, 'Frankenstein' files and VBR files without an
** Xing/LAME/Info header may not be detected properly.
*/
int mpeg_decoder_get_bitrate_mode (SF_PRIVATE *psf) ;


/*
** Initialize an encoder instance for writing. If parameter info_tag is
** SF_TRUE, a Xing/LAME/Info header is written at the beginning of the file,
** (unless the file cannot seek.)
*/
int mpeg_l3_encoder_init (SF_PRIVATE *psf, int info_tag) ;


/*
** Write an ID3v2 header from the sndfile string metadata. Must be called
** before any audio data is written. Writing an ID3v2 header will also cause
** a ID3v1 trailer to be written on close automatically.
*/
int mpeg_l3_encoder_write_id3tag (SF_PRIVATE *psf) ;

/*
** Set the encoder quality setting. Argument to compression should be identical
** to that for SFC_SET_COMPRESSION_LEVEL; It should be in the range [0-1],
** with 0 being highest quality, least compression, and 1 being the opposite.
** Returns SF_TRUE on success, SF_FALSE otherwise.
*/
int mpeg_l3_encoder_set_quality (SF_PRIVATE *psf, double compression) ;

/*
** Set the encoder bitrate mode. Can only be called before any data has been
** written. Argument mode should be one of the SF_BITRATE_MODE_ enum values.
** Returns SF_TRUE on success, SF_FALSE otherwise. The SF_BITRATE_MODE_FILE
** enum value should not be passed here but rather intercepted at the container
** level and translated according to the container.
*/
int mpeg_l3_encoder_set_bitrate_mode (SF_PRIVATE *psf, int mode) ;

/*
** Get the encoder bitrate mode in use. Returns a SF_BITRATE_MODE_ enum value.
** Will not return SF_BITRATE_MODE_FILE.
*/
int mpeg_l3_encoder_get_bitrate_mode (SF_PRIVATE *psf) ;


#endif /* SNDFILE_MPEG_H */
