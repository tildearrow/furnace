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
 */

#include "../ta-utils.h"

#ifdef _WIN32
#define META_MODIFIER_NAME "Win-"
#else
#ifdef __APPLE__
#define META_MODIFIER_NAME "Cmd-"
#else
#define META_MODIFIER_NAME "Meta-"
#endif
#endif

String getHomeDir();
String getKeyName(int key, bool emptyNone=false);

double sinus(double x);
double rectSin(double x);
double absSin(double x);
double square(double x);
double rectSquare(double x);
double quartSin(double x);
double squiSin(double x);
double squiAbsSin(double x);
double saw(double x);
double rectSaw(double x);
double absSaw(double x);
double cubSaw(double x);
double rectCubSaw(double x);
double absCubSaw(double x);
double cubSine(double x);
double rectCubSin(double x);
double absCubSin(double x);
double quartCubSin(double x);
double squishCubSin(double x);
double squishAbsCubSin(double x);
double triangle(double x);
double rectTri(double x);
double absTri(double x);
double quartTri(double x);
double squiTri(double x);
double absSquiTri(double x);
double cubTriangle(double x);
double cubRectTri(double x);
double cubAbsTri(double x);
double cubQuartTri(double x);
double cubSquiTri(double x);
double absCubSquiTri(double x);

String getMultiKeysName(const int* keys, int keyCount, bool emptyNone=false);
