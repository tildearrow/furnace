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

// portions based on imgui_widgets.cpp

#include "plot_nolerp.h"
#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"

struct FurnacePlotArrayGetterData
{
    const float* Values;
    int Stride;

    FurnacePlotArrayGetterData(const float* values, int stride) { Values = values; Stride = stride; }
};

static float Plot_ArrayGetter(void* data, int idx)
{
    FurnacePlotArrayGetterData* plot_data = (FurnacePlotArrayGetterData*)data;
    const float v = *(const float*)(const void*)((const unsigned char*)plot_data->Values + (size_t)idx * plot_data->Stride);
    return v;
}

struct FurnacePlotIntArrayGetterData
{
    const int* Values;
    int Stride;

    FurnacePlotIntArrayGetterData(const int* values, int stride) { Values = values; Stride = stride; }
};

static int Plot_IntArrayGetter(void* data, int idx)
{
    FurnacePlotIntArrayGetterData* plot_data = (FurnacePlotIntArrayGetterData*)data;
    const int v = *(const int*)(const void*)((const unsigned char*)plot_data->Values + (size_t)idx * plot_data->Stride);
    return v;
}

int PlotNoLerpEx(ImGuiPlotType plot_type, const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 frame_size)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return -1;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    if (frame_size.x == 0.0f)
        frame_size.x = ImGui::CalcItemWidth();
    if (frame_size.y == 0.0f)
        frame_size.y = label_size.y + (style.FramePadding.y * 2);

    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
    const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, 0, &frame_bb, ImGuiItemFlags_NoInertialScroll))
        return -1;
    const bool hovered = ImGui::ItemHoverable(frame_bb, id);

    // Determine scale from values if not specified
    if (scale_min == FLT_MAX || scale_max == FLT_MAX)
    {
        float v_min = FLT_MAX;
        float v_max = -FLT_MAX;
        for (int i = 0; i < values_count; i++)
        {
            const float v = values_getter(data, i);
            if (v != v) // Ignore NaN values
                continue;
            v_min = ImMin(v_min, v);
            v_max = ImMax(v_max, v);
        }
        if (scale_min == FLT_MAX)
            scale_min = v_min;
        if (scale_max == FLT_MAX)
            scale_max = v_max;
    }

    ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

    const int values_count_min = (plot_type == ImGuiPlotType_Lines) ? 2 : 1;
    int idx_hovered = -1;
    if (values_count >= values_count_min)
    {
        int res_w = ImMin((int)frame_size.x, values_count) + ((plot_type == ImGuiPlotType_Lines) ? -1 : 0);
        int item_count = values_count + ((plot_type == ImGuiPlotType_Lines) ? -1 : 0);

        // Tooltip on hover
        if (hovered && inner_bb.Contains(g.IO.MousePos))
        {
            const float t = ImClamp((g.IO.MousePos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x), 0.0f, 0.9999f);
            const int v_idx = (int)(t * item_count);
            IM_ASSERT(v_idx >= 0 && v_idx < values_count);

            const float v0 = values_getter(data, (v_idx + values_offset) % values_count);
            if (plot_type == ImGuiPlotType_Lines)
                ImGui::SetTooltip("%d: %8.4g", v_idx, v0);
            else if (plot_type == ImGuiPlotType_Histogram)
                ImGui::SetTooltip("%d: %8.4g", v_idx, v0);
            idx_hovered = v_idx;
        }

        const float t_step = 1.0f / (float)res_w;
        const float inv_scale = (scale_min == scale_max) ? 0.0f : (1.0f / (scale_max - scale_min));

        float v0 = values_getter(data, (0 + values_offset) % values_count);
        float t0 = 0.0f;
        ImVec2 tp0 = ImVec2( t0, 1.0f - ImSaturate((v0 - scale_min) * inv_scale) );                       // Point in the normalized space of our target rectangle

        const ImU32 col_base = ImGui::GetColorU32((plot_type == ImGuiPlotType_Lines) ? ImGuiCol_PlotLines : ImGuiCol_PlotHistogram);
        const ImU32 col_hovered = ImGui::GetColorU32((plot_type == ImGuiPlotType_Lines) ? ImGuiCol_PlotLinesHovered : ImGuiCol_PlotHistogramHovered);

        for (int n = 0; n < res_w; n++)
        {
            const float t1 = t0 + t_step;
            const int v1_idx = (int)(t0 * item_count + 0.5f);
            IM_ASSERT(v1_idx >= 0 && v1_idx < values_count);
            const float v1 = values_getter(data, (v1_idx + values_offset + 1) % values_count);
            const ImVec2 tp1 = ImVec2( t1, 1.0f - ImSaturate((v1 - scale_min) * inv_scale) );

            // NB: Draw calls are merged together by the DrawList system. Still, we should render our batch are lower level to save a bit of CPU.
            ImVec2 pos0 = ImLerp(inner_bb.Min, inner_bb.Max, tp0);
            ImVec2 pos1 = ImLerp(inner_bb.Min, inner_bb.Max, tp1);
            ImVec2 pos2 = ImLerp(inner_bb.Min, inner_bb.Max, tp0);
            ImVec2 pos3 = ImLerp(inner_bb.Min, inner_bb.Max, tp1);
            pos1.y=pos0.y;
            pos2.x=pos3.x;
            if (plot_type == ImGuiPlotType_Lines)
            {
                window->DrawList->AddLine(pos0, pos1, idx_hovered == v1_idx ? col_hovered : col_base);
                window->DrawList->AddLine(pos2, pos3, idx_hovered == v1_idx ? col_hovered : col_base);
            }
            else if (plot_type == ImGuiPlotType_Histogram)
            {
                if (pos1.x >= pos0.x + 2.0f)
                    pos1.x -= 1.0f;
                window->DrawList->AddRectFilled(pos0, pos1, idx_hovered == v1_idx ? col_hovered : col_base);
            }

            t0 = t1;
            tp0 = tp1;
        }
    }

    // Text overlay
    if (overlay_text)
        ImGui::RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, overlay_text, NULL, NULL, ImVec2(0.5f, 0.0f));

    if (label_size.x > 0.0f)
        ImGui::RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);

    // Return hovered index or -1 if none are hovered.
    // This is currently not exposed in the public API because we need a larger redesign of the whole thing, but in the short-term we are making it available in PlotEx().
    return idx_hovered;
}

void PlotNoLerp(const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride)
{
    FurnacePlotArrayGetterData data(values, stride);
    PlotNoLerpEx(ImGuiPlotType_Lines, label, &Plot_ArrayGetter, (void*)&data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

int PlotBitfieldEx(const char* label, int (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char** overlay_text, int bits, ImVec2 frame_size, const bool* values_highlight, ImVec4 highlightColor)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return -1;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    if (frame_size.x == 0.0f)
        frame_size.x = ImGui::CalcItemWidth();
    if (frame_size.y == 0.0f)
        frame_size.y = label_size.y + (style.FramePadding.y * 2);

    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
    const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, 0, &frame_bb, ImGuiItemFlags_NoInertialScroll))
        return -1;
    const bool hovered = ImGui::ItemHoverable(frame_bb, id);

    ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

    const int values_count_min = 1;
    int idx_hovered = -1;
    if (values_count >= values_count_min)
    {
        int res_w = ImMin((int)frame_size.x, values_count);
        int item_count = values_count;

        // Tooltip on hover
        if (hovered && inner_bb.Contains(g.IO.MousePos))
        {
            const float t = ImClamp((g.IO.MousePos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x), 0.0f, 0.9999f);
            const int v_idx = (int)(t * item_count);
            IM_ASSERT(v_idx >= 0 && v_idx < values_count);

            //const float v0 = values_getter(data, (v_idx + values_offset) % values_count);
            //ImGui::SetTooltip("%d: %8.4g", v_idx, v0);
            idx_hovered = v_idx;
        }

        const float t_step = 1.0f / (float)res_w;

        float t0 = 0.0f;
        ImVec2 tp0 = ImVec2( t0, 0.0f );                       // Point in the normalized space of our target rectangle
        const ImU32 col_base = ImGui::GetColorU32(ImGuiCol_PlotHistogram);
        const ImU32 col_hovered = ImGui::GetColorU32(ImGuiCol_PlotHistogramHovered);

        for (int n = 0; n < res_w; n++)
        {
          const float t1 = t0 + t_step;
          const int v1_idx = (int)(t0 * item_count + 0.5f);
          IM_ASSERT(v1_idx >= 0 && v1_idx < values_count);
          const int v1 = values_getter(data, (v1_idx + values_offset) % values_count);
          ImVec2 tp1 = ImVec2( t1, 0.0f );
          for (int o = 0; o < bits; o++) {
            tp0.y=float(bits-o)/float(bits);
            tp1.y=float(bits-o-1)/float(bits);
            // NB: Draw calls are merged together by the DrawList system. Still, we should render our batch are lower level to save a bit of CPU.
            ImVec2 pos0 = ImLerp(inner_bb.Min, inner_bb.Max, tp0);
            ImVec2 pos1 = ImLerp(inner_bb.Min, inner_bb.Max, tp1);
            if (pos1.x >= pos0.x + 2.0f)
                pos1.x -= 1.0f;
            if (pos1.y <= pos0.y - 2.0f)
              pos1.y += 1.0f;
            if (v1&(1<<o)) {
              ImU32 rCol=(idx_hovered == v1_idx ? col_hovered : col_base);
              if (values_highlight!=NULL) {
                if (values_highlight[v1_idx]) rCol=ImGui::GetColorU32(highlightColor);
              }
              window->DrawList->AddRectFilled(pos0, pos1, rCol);
            }
          }
          tp0 = tp1;
          t0 = t1;
        }
    }

    // Text overlay
    if (overlay_text) {
      float lineHeight=ImGui::GetTextLineHeight()/2.0;
      for (int i=0; i<bits && overlay_text[i]; i++) {
        ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0,0,0,1.0f));
        ImGui::RenderTextClipped(ImVec2(frame_bb.Min.x-1, frame_bb.Min.y-lineHeight-1), ImVec2(frame_bb.Max.x-1,frame_bb.Max.y+lineHeight-1), overlay_text[i], NULL, NULL, ImVec2(0.0f, (0.5+double(bits-1-i))/double(bits)));
        ImGui::RenderTextClipped(ImVec2(frame_bb.Min.x-1, frame_bb.Min.y-lineHeight+1), ImVec2(frame_bb.Max.x-1,frame_bb.Max.y+lineHeight+1), overlay_text[i], NULL, NULL, ImVec2(0.0f, (0.5+double(bits-1-i))/double(bits)));
        ImGui::RenderTextClipped(ImVec2(frame_bb.Min.x+1, frame_bb.Min.y-lineHeight-1), ImVec2(frame_bb.Max.x+1,frame_bb.Max.y+lineHeight-1), overlay_text[i], NULL, NULL, ImVec2(0.0f, (0.5+double(bits-1-i))/double(bits)));
        ImGui::RenderTextClipped(ImVec2(frame_bb.Min.x+1, frame_bb.Min.y-lineHeight+1), ImVec2(frame_bb.Max.x+1,frame_bb.Max.y+lineHeight+1), overlay_text[i], NULL, NULL, ImVec2(0.0f, (0.5+double(bits-1-i))/double(bits)));
        ImGui::PopStyleColor();
        ImGui::RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y-lineHeight), ImVec2(frame_bb.Max.x,frame_bb.Max.y+lineHeight), overlay_text[i], NULL, NULL, ImVec2(0.0f, (0.5+double(bits-1-i))/double(bits)));
      }
    }

    if (label_size.x > 0.0f)
        ImGui::RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);

    // Return hovered index or -1 if none are hovered.
    // This is currently not exposed in the public API because we need a larger redesign of the whole thing, but in the short-term we are making it available in PlotEx().
    return idx_hovered;
}

void PlotBitfield(const char* label, const int* values, int values_count, int values_offset, const char** overlay_text, int bits, ImVec2 graph_size, int stride, const bool* values_highlight, ImVec4 highlightColor)
{
    FurnacePlotIntArrayGetterData data(values, stride);
    PlotBitfieldEx(label, &Plot_IntArrayGetter, (void*)&data, values_count, values_offset, overlay_text, bits, graph_size, values_highlight, highlightColor);
}

int PlotCustomEx(ImGuiPlotType plot_type, const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_display_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 frame_size, ImVec4 color, int highlight, std::string (*hoverFunc)(int,float,void*), void* hoverFuncUser, bool blockMode, std::string (*guideFunc)(float), const bool* values_highlight, ImVec4 highlightColor)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return -1;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    if (frame_size.x == 0.0f)
        frame_size.x = ImGui::CalcItemWidth();
    if (frame_size.y == 0.0f)
        frame_size.y = label_size.y + (style.FramePadding.y * 2);

    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
    const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, 0, &frame_bb, ImGuiItemFlags_NoInertialScroll))
        return -1;
    const bool hovered = ImGui::ItemHoverable(frame_bb, id);

    // Determine scale from values if not specified
    if (scale_min == FLT_MAX || scale_max == FLT_MAX)
    {
        float v_min = FLT_MAX;
        float v_max = -FLT_MAX;
        for (int i = 0; i < values_count; i++)
        {
            const float v = values_getter(data, i);
            if (v != v) // Ignore NaN values
                continue;
            v_min = ImMin(v_min, v);
            v_max = ImMax(v_max, v);
        }
        if (scale_min == FLT_MAX)
            scale_min = v_min;
        if (scale_max == FLT_MAX)
            scale_max = v_max;
    }

    if (blockMode) scale_max+=1.0f;

    ImU32 bgColor=ImGui::GetColorU32(ImVec4(color.x,color.y,color.z,color.w*0.15));

    ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

    const int values_count_min = (plot_type == ImGuiPlotType_Lines) ? 2 : 1;
    int idx_hovered = -1;
    if (values_count >= values_count_min)
    {
        int res_w = ImMin((int)frame_size.x, values_count) + ((plot_type == ImGuiPlotType_Lines) ? -1 : 0);
        int item_count = values_count + ((plot_type == ImGuiPlotType_Lines) ? -1 : 0);

        // Tooltip on hover
        if (hovered && inner_bb.Contains(g.IO.MousePos))
        {
            const float t = ImClamp((g.IO.MousePos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x), 0.0f, 0.9999f);
            const int v_idx = (int)(t * item_count);
            IM_ASSERT(v_idx >= 0 && v_idx < values_count);

            const float v0 = values_getter(data, (v_idx) % values_count);
            const float v1 = values_getter(data, (v_idx + 1) % values_count);
            if (hoverFunc) {
              std::string hoverText=hoverFunc(v_idx+values_display_offset,v0,hoverFuncUser);
              if (!hoverText.empty()) {
                ImGui::SetTooltip("%s",hoverText.c_str());
              }
            } else {
              if (plot_type == ImGuiPlotType_Lines)
                  ImGui::SetTooltip("%d: %8.4g\n%d: %8.4g", v_idx, v0, v_idx + 1, v1);
              else if (plot_type == ImGuiPlotType_Histogram)
                  ImGui::SetTooltip("%d: %8.4g", v_idx+values_display_offset, v0);
            }
            idx_hovered = v_idx;
        }

        const float t_step = 1.0f / (float)res_w;
        const float inv_scale = (scale_min == scale_max) ? 0.0f : (1.0f / (scale_max - scale_min));

        float v0 = values_getter(data, (0) % values_count);
        float t0 = 0.0f;
        ImVec2 tp0 = ImVec2( t0, 1.0f - ImSaturate((v0 - scale_min) * inv_scale) );                       // Point in the normalized space of our target rectangle
        float histogram_zero_line_t = (scale_min * scale_max < 0.0f) ? (1 + (blockMode?(scale_min-0.5):scale_min) * inv_scale) : (scale_min < 0.0f ? 0.0f : 1.0f);   // Where does the zero line stands

        const ImU32 col_base = ImGui::GetColorU32(color);
        const ImU32 col_hovered = ImGui::GetColorU32(color);

        if (highlight>0) {
          window->DrawList->AddRectFilled(
            ImVec2(ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(0,0))),
            ImVec2(ImLerp(inner_bb.Min, inner_bb.Max, ImVec2((highlight>=values_count)?1:(double(highlight)/double(values_count)),1))),
            bgColor);
        }

        if (blockMode) {
          window->DrawList->AddLine(ImLerp(inner_bb.Min,inner_bb.Max,ImVec2(0.0f,histogram_zero_line_t)),ImLerp(inner_bb.Min,inner_bb.Max,ImVec2(1.0f,histogram_zero_line_t)),col_base);
        }

        for (int n = 0; n < res_w; n++)
        {
            const float t1 = t0 + t_step;
            const int v1_idx = (int)(t0 * item_count + 0.5f);
            IM_ASSERT(v1_idx >= 0 && v1_idx < values_count);
            const float v1 = values_getter(data, (v1_idx + 1) % values_count);
            const ImVec2 tp1 = ImVec2( t1, 1.0f - ImSaturate((v1 - scale_min) * inv_scale) );

            // NB: Draw calls are merged together by the DrawList system. Still, we should render our batch are lower level to save a bit of CPU.
            ImVec2 pos0 = ImLerp(inner_bb.Min, inner_bb.Max, tp0);
            ImVec2 pos1 = ImLerp(inner_bb.Min, inner_bb.Max, (plot_type == ImGuiPlotType_Lines) ? tp1 : ImVec2(tp1.x, blockMode?tp0.y:histogram_zero_line_t));
            if (plot_type == ImGuiPlotType_Lines)
            {
                window->DrawList->AddLine(pos0, pos1, idx_hovered == v1_idx ? col_hovered : col_base);
            }
            else if (plot_type == ImGuiPlotType_Histogram)
            {
                if (pos1.x >= pos0.x + 2.0f)
                    pos1.x -= 1.0f;
                if (blockMode) {
                  pos0.y-=(inner_bb.Max.y-inner_bb.Min.y)*inv_scale;
                  //pos1.y+=1.0f;
                }
                ImU32 rCol=(idx_hovered == v1_idx ? col_hovered : col_base);
                if (values_highlight!=NULL) {
                  if (values_highlight[v1_idx]) rCol=ImGui::GetColorU32(highlightColor);
                }
                window->DrawList->AddRectFilled(pos0, pos1, rCol);
            }

            t0 = t1;
            tp0 = tp1;
        }
    }

    // Text overlay
    if (overlay_text)
        ImGui::RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, overlay_text, NULL, NULL, ImVec2(0.5f, 0.0f));

    if (label_size.x > 0.0f)
        ImGui::RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);

    if (guideFunc && idx_hovered!=0) {
      std::string guide=guideFunc(scale_max);
      window->DrawList->AddText(frame_bb.Min+ImVec2(1.0f,1.0f),0xff000000,guide.c_str());
      window->DrawList->AddText(frame_bb.Min+ImVec2(1.0f,-1.0f),0xff000000,guide.c_str());
      window->DrawList->AddText(frame_bb.Min+ImVec2(-1.0f,1.0f),0xff000000,guide.c_str());
      window->DrawList->AddText(frame_bb.Min+ImVec2(-1.0f,-1.0f),0xff000000,guide.c_str());
      window->DrawList->AddText(frame_bb.Min,0xffffffff,guide.c_str());

      std::string guide1=guideFunc(scale_min);
      ImVec2 maxPos=frame_bb.Min;
      maxPos.y=frame_bb.Max.y-ImGui::GetFont()->FontSize;
      window->DrawList->AddText(maxPos+ImVec2(1.0f,1.0f),0xff000000,guide1.c_str());
      window->DrawList->AddText(maxPos+ImVec2(1.0f,-1.0f),0xff000000,guide1.c_str());
      window->DrawList->AddText(maxPos+ImVec2(-1.0f,1.0f),0xff000000,guide1.c_str());
      window->DrawList->AddText(maxPos+ImVec2(-1.0f,-1.0f),0xff000000,guide1.c_str());
      window->DrawList->AddText(maxPos,0xffffffff,guide1.c_str());
    }

    // Return hovered index or -1 if none are hovered.
    // This is currently not exposed in the public API because we need a larger redesign of the whole thing, but in the short-term we are making it available in PlotEx().
    return idx_hovered;
}

void PlotCustom(const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride, ImVec4 color, int highlight, std::string (*hoverFunc)(int,float,void*), void* hoverFuncUser, bool blockMode, std::string (*guideFunc)(float), const bool* values_highlight, ImVec4 highlightColor)
{
    FurnacePlotArrayGetterData data(values, stride);
    PlotCustomEx(ImGuiPlotType_Histogram, label, &Plot_ArrayGetter, (void*)&data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size, color, highlight, hoverFunc, hoverFuncUser, blockMode, guideFunc, values_highlight, highlightColor);
}
