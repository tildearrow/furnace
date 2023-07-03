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

// TODO: everything

#include "renderMetal.h"
#include "backends/imgui_impl_metal.h"
#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>

class FurnaceMetalTexture: public FurnaceGUITexture {
  public:
  MTLTexture* tex;
  FurnaceMetalTexture():
    tex(NULL) {}
};

ImTextureID FurnaceGUIRenderMetal::getTextureID(FurnaceGUITexture* which) {
  FurnaceSDLTexture* t=(FurnaceSDLTexture*)which;
  return t->tex;
}

bool FurnaceGUIRenderMetal::lockTexture(FurnaceGUITexture* which, void** data, int* pitch) {
  FurnaceSDLTexture* t=(FurnaceSDLTexture*)which;
  return SDL_LockTexture(t->tex,NULL,data,pitch)==0;
}

bool FurnaceGUIRenderMetal::unlockTexture(FurnaceGUITexture* which) {
  FurnaceSDLTexture* t=(FurnaceSDLTexture*)which;
  SDL_UnlockTexture(t->tex);
  return true;
}

bool FurnaceGUIRenderMetal::updateTexture(FurnaceGUITexture* which, void* data, int pitch) {
  FurnaceSDLTexture* t=(FurnaceSDLTexture*)which;
  return SDL_UpdateTexture(t->tex,NULL,data,pitch)==0;
}

FurnaceGUITexture* FurnaceGUIRenderMetal::createTexture(bool dynamic, int width, int height) {
  SDL_Texture* t=SDL_CreateTexture(sdlRend,SDL_PIXELFORMAT_ABGR8888,dynamic?SDL_TEXTUREACCESS_STREAMING:SDL_TEXTUREACCESS_STATIC,width,height);

  if (t==NULL) return NULL;
  FurnaceSDLTexture* ret=new FurnaceSDLTexture;
  ret->tex=t;
  return ret;
}

bool FurnaceGUIRenderMetal::destroyTexture(FurnaceGUITexture* which) {
  FurnaceSDLTexture* t=(FurnaceSDLTexture*)which;

  SDL_DestroyTexture(t->tex);
  delete t;
  return true;
}

void FurnaceGUIRenderMetal::setTextureBlendMode(FurnaceGUITexture* which, FurnaceGUIBlendMode mode) {
  FurnaceSDLTexture* t=(FurnaceSDLTexture*)which;
  switch (mode) {
    case GUI_BLEND_MODE_NONE:
      SDL_SetTextureBlendMode(t->tex,SDL_BLENDMODE_NONE);
      break;
    case GUI_BLEND_MODE_BLEND:
      SDL_SetTextureBlendMode(t->tex,SDL_BLENDMODE_BLEND);
      break;
    case GUI_BLEND_MODE_ADD:
      SDL_SetTextureBlendMode(t->tex,SDL_BLENDMODE_ADD);
      break;
    case GUI_BLEND_MODE_MULTIPLY:
      SDL_SetTextureBlendMode(t->tex,SDL_BLENDMODE_MOD);
      break;
  }
}

void FurnaceGUIRenderMetal::setBlendMode(FurnaceGUIBlendMode mode) {
  switch (mode) {
    case GUI_BLEND_MODE_NONE:
      SDL_SetRenderDrawBlendMode(sdlRend,SDL_BLENDMODE_NONE);
      break;
    case GUI_BLEND_MODE_BLEND:
      SDL_SetRenderDrawBlendMode(sdlRend,SDL_BLENDMODE_BLEND);
      break;
    case GUI_BLEND_MODE_ADD:
      SDL_SetRenderDrawBlendMode(sdlRend,SDL_BLENDMODE_ADD);
      break;
    case GUI_BLEND_MODE_MULTIPLY:
      SDL_SetRenderDrawBlendMode(sdlRend,SDL_BLENDMODE_MOD);
      break;
  }
}

void FurnaceGUIRenderMetal::clear(ImVec4 color) {
  SDL_SetRenderDrawColor(sdlRend,color.x*255,color.y*255,color.z*255,color.w*255);
  SDL_RenderClear(sdlRend);
}

bool FurnaceGUIRenderMetal::newFrame() {
  return ImGui_ImplSDLRenderer2_NewFrame();
}

void FurnaceGUIRenderMetal::createFontsTexture() {
  ImGui_ImplSDLRenderer2_CreateFontsTexture();
}

void FurnaceGUIRenderMetal::destroyFontsTexture() {
  ImGui_ImplSDLRenderer2_DestroyFontsTexture();
}

void FurnaceGUIRenderMetal::renderGUI() {
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
}

void FurnaceGUIRenderMetal::wipe(float alpha) {
  SDL_SetRenderDrawBlendMode(sdlRend,SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(sdlRend,0,0,0,255*alpha);
  SDL_RenderFillRect(sdlRend,NULL);
}

void FurnaceGUIRenderMetal::present() {
  SDL_RenderPresent(sdlRend);
}

bool FurnaceGUIRenderMetal::getOutputSize(int& w, int& h) {
  return SDL_GetRendererOutputSize(sdlRend,&w,&h)==0;
}

int FurnaceGUIRenderMetal::getWindowFlags() {
  return 0;
}

void FurnaceGUIRenderMetal::preInit() {
  SDL_SetHint(SDL_HINT_RENDER_DRIVER,"metal");
}

bool FurnaceGUIRenderMetal::init(SDL_Window* win) {
  SDL_SetHint(SDL_HINT_RENDER_DRIVER,"metal");

  sdlRend=SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC|SDL_RENDERER_TARGETTEXTURE);
  return (sdlRend!=NULL);
}

void FurnaceGUIRenderMetal::initGUI(SDL_Window* win) {
  ImGui_ImplSDL2_InitForSDLRenderer(win,sdlRend);
  ImGui_ImplSDLRenderer2_Init(sdlRend);
}

void FurnaceGUIRenderMetal::quitGUI() {
  ImGui_ImplSDLRenderer2_Shutdown();
}

bool FurnaceGUIRenderMetal::quit() {
  if (sdlRend==NULL) return false;
  SDL_DestroyRenderer(sdlRend);
  sdlRend=NULL;
  return true;
}
