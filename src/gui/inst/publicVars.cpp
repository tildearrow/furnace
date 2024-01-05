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

#include "publicVars.h"

std::vector<FurnaceGUIMacroDesc> macroList;
const char* volumeLabel="Volume";
const char* dutyLabel="Duty/Noise";
const char* waveLabel="Waveform";
int volMax=15;
int volMin=0;
int dutyMin=0;
int dutyMax=3;
int waveMax=0;
bool waveBitMode=false;
const char** waveNames=NULL;
int ex1Max=0;
int ex2Max=0;
bool ex2Bit=false;
int panMin=0;
int panMax=0;
bool panSingle=false;
bool panSingleNoBit=false;