/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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
 */

enum CommandPaletteType {
  CMDPAL_TYPE_MAIN = 0,
  CMDPAL_TYPE_RECENT,
  CMDPAL_TYPE_INSTRUMENTS,
  CMDPAL_TYPE_SAMPLES,
  CMDPAL_TYPE_INSTRUMENT_CHANGE,
  CMDPAL_TYPE_ADD_CHIP,
  // a menu to select wavetables is beyond scope (they can't be put as strings)
  // TODO: are there more?

  CMDPAL_TYPE_MAX,
};
