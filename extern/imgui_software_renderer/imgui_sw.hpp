// By Emil Ernerfeldt 2018
// LICENSE:
//   This software is dual-licensed to the public domain and under the following
//   license: you are granted a perpetual, irrevocable license to copy, modify,
//   publish, and distribute this file as you see fit.
// WHAT:
//   This is a software renderer for Dear ImGui.
//   It is decently fast, but has a lot of room for optimization.
//   The goal was to get something fast and decently accurate in not too many lines of code.
// LIMITATIONS:
//   * It is not pixel-perfect, but it is good enough for must use cases.
//   * It does not support texture scaling for the sake of performance.
#pragma once
#include "imgui.h"      // IMGUI_IMPL_API
#ifndef IMGUI_DISABLE

#include <cstdint>

struct SDL_Window;
struct ImDrawData;

struct SWTexture
{
  uint32_t* pixels;
  int width;
  int height;
  bool managed, isAlpha;

  SWTexture(uint32_t* pix, int w, int h, bool a=false):
    pixels(pix),
    width(w),
    height(h),
    managed(false),
    isAlpha(a) {}
  SWTexture(int w, int h, bool a=false):
    width(w),
    height(h),
    managed(true),
    isAlpha(a) {
    pixels=new uint32_t[width*height];
  }
  ~SWTexture() {
    if (managed) delete[] pixels;
  }
};

IMGUI_IMPL_API bool     ImGui_ImplSW_Init(SDL_Window* win);
IMGUI_IMPL_API void     ImGui_ImplSW_Shutdown();
IMGUI_IMPL_API bool     ImGui_ImplSW_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplSW_RenderDrawData(ImDrawData* draw_data);

// Called by Init/NewFrame/Shutdown
IMGUI_IMPL_API bool     ImGui_ImplSW_CreateFontsTexture();
IMGUI_IMPL_API void     ImGui_ImplSW_DestroyFontsTexture();
IMGUI_IMPL_API bool     ImGui_ImplSW_CreateDeviceObjects();
IMGUI_IMPL_API void     ImGui_ImplSW_DestroyDeviceObjects();

#endif // #ifndef IMGUI_DISABLE
