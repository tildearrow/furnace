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

#include "renderSDL.h"
#include "backends/imgui_impl_sdlrenderer2.h"

class FurnaceSDLTexture: public FurnaceGUITexture {
  public:
  SDL_Texture* tex;
  FurnaceSDLTexture():
    tex(NULL) {}
};

ImTextureID FurnaceGUIRenderSDL::getTextureID(FurnaceGUITexture* which) {
  FurnaceSDLTexture* t=(FurnaceSDLTexture*)which;
  return t->tex;
}

bool FurnaceGUIRenderSDL::lockTexture(FurnaceGUITexture* which, void** data, int* pitch) {
  FurnaceSDLTexture* t=(FurnaceSDLTexture*)which;
  return SDL_LockTexture(t->tex,NULL,data,pitch)==0;
}

bool FurnaceGUIRenderSDL::unlockTexture(FurnaceGUITexture* which) {
  FurnaceSDLTexture* t=(FurnaceSDLTexture*)which;
  SDL_UnlockTexture(t->tex);
  return true;
}

bool FurnaceGUIRenderSDL::updateTexture(FurnaceGUITexture* which, void* data, int pitch) {
  FurnaceSDLTexture* t=(FurnaceSDLTexture*)which;
  return SDL_UpdateTexture(t->tex,NULL,data,pitch)==0;
}

FurnaceGUITexture* FurnaceGUIRenderSDL::createTexture(bool dynamic, int width, int height) {
  SDL_Texture* t=SDL_CreateTexture(sdlRend,SDL_PIXELFORMAT_ABGR8888,dynamic?SDL_TEXTUREACCESS_STREAMING:SDL_TEXTUREACCESS_STATIC,width,height);

  if (t==NULL) return NULL;
  FurnaceSDLTexture* ret=new FurnaceSDLTexture;
  ret->tex=t;
  return ret;
}

bool FurnaceGUIRenderSDL::destroyTexture(FurnaceGUITexture* which) {
  FurnaceSDLTexture* t=(FurnaceSDLTexture*)which;

  SDL_DestroyTexture(t->tex);
  delete t;
  return true;
}

void FurnaceGUIRenderSDL::setTextureBlendMode(FurnaceGUITexture* which, FurnaceGUIBlendMode mode) {
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

void FurnaceGUIRenderSDL::setBlendMode(FurnaceGUIBlendMode mode) {
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

void FurnaceGUIRenderSDL::clear(ImVec4 color) {
  SDL_SetRenderDrawColor(sdlRend,color.x*255,color.y*255,color.z*255,color.w*255);
  SDL_RenderClear(sdlRend);
}

bool FurnaceGUIRenderSDL::newFrame() {
  return ImGui_ImplSDLRenderer2_NewFrame();
}

void FurnaceGUIRenderSDL::createFontsTexture() {
  ImGui_ImplSDLRenderer2_CreateFontsTexture();
}

void FurnaceGUIRenderSDL::destroyFontsTexture() {
  ImGui_ImplSDLRenderer2_DestroyFontsTexture();
}

void FurnaceGUIRenderSDL::renderGUI() {
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
}

void FurnaceGUIRenderSDL::wipe(float alpha) {
  SDL_SetRenderDrawBlendMode(sdlRend,SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(sdlRend,0,0,0,255*alpha);
  SDL_RenderFillRect(sdlRend,NULL);
}

void FurnaceGUIRenderSDL::present() {
  SDL_RenderPresent(sdlRend);
}

bool FurnaceGUIRenderSDL::getOutputSize(int& w, int& h) {
  return SDL_GetRendererOutputSize(sdlRend,&w,&h)==0;
}

int FurnaceGUIRenderSDL::getWindowFlags() {
  return 0;
}

void FurnaceGUIRenderSDL::preInit() {
}

bool FurnaceGUIRenderSDL::init(SDL_Window* win) {
  sdlRend=SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC|SDL_RENDERER_TARGETTEXTURE);
  return (sdlRend!=NULL);
}

void FurnaceGUIRenderSDL::initGUI(SDL_Window* win) {
  ImGui_ImplSDL2_InitForSDLRenderer(win,sdlRend);
  ImGui_ImplSDLRenderer2_Init(sdlRend);
}

void FurnaceGUIRenderSDL::quitGUI() {
  ImGui_ImplSDLRenderer2_Shutdown();
}

bool FurnaceGUIRenderSDL::quit() {
  if (sdlRend==NULL) return false;
  SDL_DestroyRenderer(sdlRend);
  sdlRend=NULL;
  return true;
}
