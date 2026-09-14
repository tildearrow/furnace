/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

// additional modifications by tildearrow for furnace

#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DBWORD;
typedef signed char SBYTE;
typedef signed short SWORD;
typedef signed int SDBWORD;

#if !defined (FALSE)
enum false_value { FALSE, TRUE };
#endif
enum exit_type { EXIT_OK, EXIT_ERROR };
enum lower_value { LOWER, UPPER };
enum machine_mode { AUTO, NTSC, PAL, DENDY, DEFAULT = 255 };
enum reset_type {
  RESET       = 0x10,
  HARD        = 0x20,
  CHANGE_ROM  = 0x30,
  CHANGE_MODE = 0x40,
  POWER_UP    = 0x50
};
/* le dimesioni dello screen da renderizzare */
enum screen_dimension { SCR_LINES = 240, SCR_ROWS = 256 };
enum type_of_system_info { HEADER, DATABASE };
enum header_type { iNES_1_0, NES_2_0, UNIF_FORMAT, FDS_FORMAT, NSF_FORMAT, NSFE_FORMAT };
enum length_file_name_type {
  LENGTH_FILE_NAME      = 512,
  LENGTH_FILE_NAME_MID  = 1024,
  LENGTH_FILE_NAME_LONG = 2048,
  LENGTH_FILE_NAME_MAX = 4096
};
enum forced_mirroring { UNK_HORIZONTAL, UNK_VERTICAL };
enum max_chips_rom { MAX_CHIPS = 8 };
enum languages { LNG_ENGLISH, LNG_ITALIAN, LNG_RUSSIAN };
enum database_mode {
  NODIPSWITCH = 0xFF00,
  NOEXTRA = 0x0000,
  VSZAPPER = 0x0001,
  CHRRAM32K = 0x0002,
  CHRRAM256K = 0x0004
};

#define LENGTH(x) (sizeof(x)/sizeof(*(x)))
#define UNUSED(var) var __attribute__((unused))

#if defined (DEBUG)
#define INLINE
#else
#ifdef _MSC_VER
#define INLINE __forceinline
#else
#define INLINE inline __attribute__((always_inline))
#endif
#endif

#endif /* COMMON_H_ */
