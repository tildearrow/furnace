/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

#include "../gui.h"
#include "../guiConst.h"
#include "IconsFontAwesome4.h"
#include "furIcons.h"

extern std::vector<FurnaceGUIMacroDesc> macroList;
extern const char* volumeLabel;
extern const char* dutyLabel;
extern const char* waveLabel;
extern int volMax;
extern int volMin;
extern int dutyMin;
extern int dutyMax;
extern int waveMax;
extern bool waveBitMode;
extern const char** waveNames;
extern int ex1Max;
extern int ex2Max;
extern bool ex2Bit;