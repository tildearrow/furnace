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

#include "imgui.h"
#include "../pch.h"

void PlotNoLerp(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0), int stride = sizeof(float));
void PlotBitfield(const char* label, const int* values, int values_count, int values_offset = 0, const char** overlay_text = NULL, int bits = 8, ImVec2 graph_size = ImVec2(0, 0), int stride = sizeof(float), const bool* values_highlight = NULL, ImVec4 highlightColor = ImVec4(1.0f,1.0f,1.0f,1.0f), ImVec4 color = ImVec4(1.0f,1.0f,1.0f,1.0f), std::string (*hoverFunc)(int,float,void*) = NULL, void* hoverFuncUser = NULL);
void PlotCustom(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0), int stride = sizeof(float), ImVec4 fgColor = ImVec4(1.0f,1.0f,1.0f,1.0f), int highlight = 0, std::string (*hoverFunc)(int,float,void*) = NULL, void* hoverFuncUser=NULL, bool blockMode=false, std::string (*guideFunc)(float) = NULL, const bool* values_highlight = NULL, ImVec4 highlightColor = ImVec4(1.0f,1.0f,1.0f,1.0f));
