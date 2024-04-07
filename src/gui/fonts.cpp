/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#include "fonts.h"

const unsigned char* builtinFont[]={
  font_plexSans_compressed_data,
  font_liberationSans_compressed_data,
  font_exo_compressed_data,
  font_proggyClean_compressed_data,
  font_unifont_compressed_data
};

const unsigned int builtinFontLen[]={
  font_plexSans_compressed_size,
  font_liberationSans_compressed_size,
  font_exo_compressed_size,
  font_proggyClean_compressed_size,
  font_unifont_compressed_size
};

const unsigned char* builtinFontM[]={
  font_plexMono_compressed_data,
  font_mononoki_compressed_data,
  font_ptMono_compressed_data,
  font_proggyClean_compressed_data,
  font_unifont_compressed_data
};

const unsigned int builtinFontMLen[]={
  font_plexMono_compressed_size,
  font_mononoki_compressed_size,
  font_ptMono_compressed_size,
  font_proggyClean_compressed_size,
  font_unifont_compressed_size
};
