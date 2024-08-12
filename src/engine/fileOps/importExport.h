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
 */

#pragma once

#include "../dataErrors.h"
#include "../engine.h"
#include "../../ta-log.h"
#include "../instrument.h"
#include "../song.h"
#include <zlib.h>
#include <fmt/printf.h>

#define DIV_READ_SIZE 131072
#define DIV_DMF_MAGIC ".DelekDefleMask."
#define DIV_FUR_MAGIC "-Furnace module-"
#define DIV_FTM_MAGIC "FamiTracker Module"
#define DIV_DN_FTM_MAGIC "Dn-FamiTracker Module"
#define DIV_FC13_MAGIC "SMOD"
#define DIV_FC14_MAGIC "FC14"
#define DIV_S3M_MAGIC "SCRM"
#define DIV_XM_MAGIC "Extended Module: "
#define DIV_IT_MAGIC "IMPM"
#define DIV_TFM_MAGIC "TFMfmtV2"

enum DivFurVariants: int {
  DIV_FUR_VARIANT_VANILLA=0,
  DIV_FUR_VARIANT_B=1,
};
