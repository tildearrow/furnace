/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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

// guiConst: constants used in the GUI like arrays, strings and other stuff
#include "guiConst.h"
#include "../engine/instrument.h"

const int opOrder[4]={
  0, 2, 1, 3
};

const char* noteNames[180]={
  "c_5", "c+5", "d_5", "d+5", "e_5", "f_5", "f+5", "g_5", "g+5", "a_5", "a+5", "b_5",
  "c_4", "c+4", "d_4", "d+4", "e_4", "f_4", "f+4", "g_4", "g+4", "a_4", "a+4", "b_4",
  "c_3", "c+3", "d_3", "d+3", "e_3", "f_3", "f+3", "g_3", "g+3", "a_3", "a+3", "b_3",
  "c_2", "c+2", "d_2", "d+2", "e_2", "f_2", "f+2", "g_2", "g+2", "a_2", "a+2", "b_2",
  "c_1", "c+1", "d_1", "d+1", "e_1", "f_1", "f+1", "g_1", "g+1", "a_1", "a+1", "b_1",
  "C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0",
  "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
  "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
  "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
  "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
  "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
  "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
  "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "B-7",
  "C-8", "C#8", "D-8", "D#8", "E-8", "F-8", "F#8", "G-8", "G#8", "A-8", "A#8", "B-8",
  "C-9", "C#9", "D-9", "D#9", "E-9", "F-9", "F#9", "G-9", "G#9", "A-9", "A#9", "B-9"
};

const char* noteNamesG[180]={
  "c_5", "c+5", "d_5", "d+5", "e_5", "f_5", "f+5", "g_5", "g+5", "a_5", "a+5", "h_5",
  "c_4", "c+4", "d_4", "d+4", "e_4", "f_4", "f+4", "g_4", "g+4", "a_4", "a+4", "h_4",
  "c_3", "c+3", "d_3", "d+3", "e_3", "f_3", "f+3", "g_3", "g+3", "a_3", "a+3", "h_3",
  "c_2", "c+2", "d_2", "d+2", "e_2", "f_2", "f+2", "g_2", "g+2", "a_2", "a+2", "h_2",
  "c_1", "c+1", "d_1", "d+1", "e_1", "f_1", "f+1", "g_1", "g+1", "a_1", "a+1", "h_1",
  "C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "H-0",
  "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "H-1",
  "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "H-2",
  "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "H-3",
  "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "H-4",
  "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "H-5",
  "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "H-6",
  "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "H-7",
  "C-8", "C#8", "D-8", "D#8", "E-8", "F-8", "F#8", "G-8", "G#8", "A-8", "A#8", "H-8",
  "C-9", "C#9", "D-9", "D#9", "E-9", "F-9", "F#9", "G-9", "G#9", "A-9", "A#9", "H-9"
};

const char* pitchLabel[11]={
  "1/6", "1/5", "1/4", "1/3", "1/2", "1x", "2x", "3x", "4x", "5x", "6x"
};

const char* insTypes[DIV_INS_MAX]={
  "Standard",
  "FM (4-operator)",
  "Game Boy",
  "C64",
  "Amiga/Sample",
  "PC Engine",
  "AY-3-8910/SSG",
  "AY8930",
  "TIA",
  "SAA1099",
  "VIC",
  "PET",
  "VRC6",
  "FM (OPLL)",
  "FM (OPL)",
  "FDS",
  "Virtual Boy",
  "Namco 163",
  "Konami SCC",
  "FM (OPZ)",
  "POKEY",
  "PC Beeper",
  "WonderSwan",
  "Atari Lynx",
  "VERA",
  "X1-010"
};