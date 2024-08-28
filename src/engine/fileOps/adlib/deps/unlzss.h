/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2008 Simon Peter, <dn.tlp@gmx.net>, et al.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * unlzss.h - Custom LZSS decompression routine from Adlib Tracker II
 *            Adapted by Dmitry Smagin <dmitry.s.smagin@gmail.com>
 *            Originally by Stanislav Baranec <subz3ro.altair@gmail.com>
 *
 * REFERENCES:
 * https://github.com/ijsf/at2
 * http://www.adlibtracker.net/
 *
 */

#ifndef _UNLZSS_H_
#define _UNLZSS_H_

#ifdef __cplusplus
extern "C" {
#endif

int LZSS_decompress(char *source, char *dest, int source_size, int dest_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _UNLZSS_H_ */
