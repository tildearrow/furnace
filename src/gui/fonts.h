/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * this license only applies to the code. for the license of each font used,
 * see `papers/`.
 */

#ifndef _FONTS_H
#define _FONTS_H
extern const unsigned int font_exo_compressed_size;
extern const unsigned char font_exo_compressed_data[];
extern const unsigned int font_liberationSans_compressed_size;
extern const unsigned char font_liberationSans_compressed_data[];
extern const unsigned int font_mononoki_compressed_size;
extern const unsigned char font_mononoki_compressed_data[];
extern const unsigned int font_plexSans_compressed_size;
extern const unsigned char font_plexSans_compressed_data[];
extern const unsigned int font_plexSansJP_compressed_size;
extern const unsigned char font_plexSansJP_compressed_data[];
extern const unsigned int font_plexSansKR_compressed_size;
extern const unsigned char font_plexSansKR_compressed_data[];
extern const unsigned int font_plexMono_compressed_size;
extern const unsigned char font_plexMono_compressed_data[];
extern const unsigned int font_proggyClean_compressed_size;
extern const unsigned char font_proggyClean_compressed_data[];
extern const unsigned int font_ptMono_compressed_size;
extern const unsigned char font_ptMono_compressed_data[];
extern const unsigned int font_unifont_compressed_size;
extern const unsigned char font_unifont_compressed_data[];
extern const unsigned int iconFont_compressed_size;
extern const unsigned char iconFont_compressed_data[];
extern const unsigned int furIcons_compressed_size;
extern const unsigned char furIcons_compressed_data[];

extern const unsigned char* builtinFont[];
extern const unsigned int builtinFontLen[];
extern const unsigned char* builtinFontM[];
extern const unsigned int builtinFontMLen[];
#endif

// "(Glimmer of hope: the atlas system will be rewritten in the future to make scaling more flexible.)""
// not just that. somebody rewrite it already so I can load these glyphs at run-time and support
// all languages at once, instead of having to load Unifont's 65k+ characters and blow the GPU up in the
// process!
