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

#include "renderGL1.h"
#include "../../ta-log.h"
#include "SDL_opengl.h"
#include "backends/imgui_impl_opengl2.h"
#include "../engine/bsr.h"

#define C(x) x; if (glGetError()!=GL_NO_ERROR) logW("OpenGL error in %s:%d: " #x,__FILE__,__LINE__);

class FurnaceGL1Texture: public FurnaceGUITexture {
  public:
  GLuint id;
  int width, height, widthReal, heightReal;
  unsigned char* lockedData;
  FurnaceGL1Texture():
    id(0),
    width(0),
    height(0),
    widthReal(0),
    heightReal(0),
    lockedData(NULL) {}
};

ImTextureID FurnaceGUIRenderGL1::getTextureID(FurnaceGUITexture* which) {
  intptr_t ret=((FurnaceGL1Texture*)which)->id;
  return (ImTextureID)ret;
}

float FurnaceGUIRenderGL1::getTextureU(FurnaceGUITexture* which) {
  FurnaceGL1Texture* t=(FurnaceGL1Texture*)which;
  if (t->widthReal<1) return 0.0f;
  return (float)t->width/(float)t->widthReal;
}

float FurnaceGUIRenderGL1::getTextureV(FurnaceGUITexture* which) {
  FurnaceGL1Texture* t=(FurnaceGL1Texture*)which;
  if (t->heightReal<1) return 0.0f;
  return (float)t->height/(float)t->heightReal;
}

bool FurnaceGUIRenderGL1::lockTexture(FurnaceGUITexture* which, void** data, int* pitch) {
  FurnaceGL1Texture* t=(FurnaceGL1Texture*)which;
  if (t->lockedData!=NULL) return false;
  t->lockedData=new unsigned char[t->widthReal*t->heightReal*4];

  *data=t->lockedData;
  *pitch=t->widthReal*4;
  return true;
}

bool FurnaceGUIRenderGL1::unlockTexture(FurnaceGUITexture* which) {
  FurnaceGL1Texture* t=(FurnaceGL1Texture*)which;
  if (t->lockedData==NULL) return false;

  C(glBindTexture(GL_TEXTURE_2D,t->id));
  C(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,t->widthReal,t->heightReal,0,GL_RGBA,GL_UNSIGNED_BYTE,t->lockedData));
  
  C(glFlush());
  delete[] t->lockedData;
  t->lockedData=NULL;

  return true;
}

bool FurnaceGUIRenderGL1::updateTexture(FurnaceGUITexture* which, void* data, int pitch) {
  FurnaceGL1Texture* t=(FurnaceGL1Texture*)which;

  if (t->width*4!=pitch) return false;

  logV("GL1 updateTexture...");

  C(glBindTexture(GL_TEXTURE_2D,t->id));
  C(glTexSubImage2D(GL_TEXTURE_2D,0,0,0,t->width,t->height,GL_RGBA,GL_UNSIGNED_BYTE,data));
  return true;
}

FurnaceGUITexture* FurnaceGUIRenderGL1::createTexture(bool dynamic, int width, int height, bool interpolate, FurnaceGUITextureFormat format) {
  if (format!=GUI_TEXFORMAT_ABGR32) {
    logE("unsupported texture format!");
    return NULL;
  }
  FurnaceGL1Texture* t=new FurnaceGL1Texture;
  C(glGenTextures(1,&t->id));
  C(glBindTexture(GL_TEXTURE_2D,t->id));
  if (interpolate) {
    C(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));
    C(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));
  } else {
    C(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST));
    C(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST));
  }

  int widthReal=width;
  int heightReal=height;

  if ((widthReal&(widthReal-1))!=0) {
    widthReal=1<<bsr(width);
  }
  if ((heightReal&(heightReal-1))!=0) {
    heightReal=1<<bsr(height);
  }
  logV("width: %d (requested)... %d (actual)",width,widthReal);
  logV("height: %d (requested)... %d (actual)",height,heightReal);

  C(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,widthReal,heightReal,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL));
  t->width=width;
  t->height=height;
  t->widthReal=widthReal;
  t->heightReal=heightReal;
  return t;
}

bool FurnaceGUIRenderGL1::destroyTexture(FurnaceGUITexture* which) {
  FurnaceGL1Texture* t=(FurnaceGL1Texture*)which;
  C(glDeleteTextures(1,&t->id));
  delete t;
  return true;
}

void FurnaceGUIRenderGL1::setTextureBlendMode(FurnaceGUITexture* which, FurnaceGUIBlendMode mode) {
}

void FurnaceGUIRenderGL1::setBlendMode(FurnaceGUIBlendMode mode) {
  switch (mode) {
    case GUI_BLEND_MODE_NONE:
      C(glBlendFunc(GL_ONE,GL_ZERO));
      C(glDisable(GL_BLEND));
      break;
    case GUI_BLEND_MODE_BLEND:
      C(glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA));
      C(glEnable(GL_BLEND));
      break;
    case GUI_BLEND_MODE_ADD:
      C(glBlendFunc(GL_SRC_ALPHA,GL_ONE));
      C(glEnable(GL_BLEND));
      break;
    case GUI_BLEND_MODE_MULTIPLY:
      C(glBlendFunc(GL_ZERO,GL_SRC_COLOR));
      C(glEnable(GL_BLEND));
      break;
  }
}

void FurnaceGUIRenderGL1::clear(ImVec4 color) {
  SDL_GL_MakeCurrent(sdlWin,context);
  C(glClearColor(color.x,color.y,color.z,color.w));
  C(glClear(GL_COLOR_BUFFER_BIT));
}

bool FurnaceGUIRenderGL1::newFrame() {
  return ImGui_ImplOpenGL2_NewFrame();
}

bool FurnaceGUIRenderGL1::canVSync() {
  return swapIntervalSet;
}

void FurnaceGUIRenderGL1::createFontsTexture() {
  ImGui_ImplOpenGL2_CreateFontsTexture();
}

void FurnaceGUIRenderGL1::destroyFontsTexture() {
  ImGui_ImplOpenGL2_DestroyFontsTexture();
}

void FurnaceGUIRenderGL1::renderGUI() {
  ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

void FurnaceGUIRenderGL1::wipe(float alpha) {
  C(glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA));
  C(glEnable(GL_BLEND));

  C(glBindTexture(GL_TEXTURE_2D,0));
  C(glBegin(GL_TRIANGLE_STRIP));
  C(glColor4f(0.0,0.0,0.0,alpha));
  C(glNormal3f(0.0f,0.0f,0.0f));
  C(glVertex3f(-1.0f,-1.0f,0.0f));
  C(glNormal3f(1.0f,0.0f,0.0f));
  C(glVertex3f(1.0f,-1.0f,0.0f));
  C(glNormal3f(0.0f,1.0f,0.0f));
  C(glVertex3f(-1.0f,1.0f,0.0f));
  C(glNormal3f(1.0f,1.0f,0.0f));
  C(glVertex3f(1.0f,1.0f,0.0f));
  C(glEnd());
}

void FurnaceGUIRenderGL1::present() {
  SDL_GL_SwapWindow(sdlWin);
  C(glFlush());
}

bool FurnaceGUIRenderGL1::getOutputSize(int& w, int& h) {
  SDL_GL_GetDrawableSize(sdlWin,&w,&h);
  return true;
}

int FurnaceGUIRenderGL1::getWindowFlags() {
  return SDL_WINDOW_OPENGL;
}

int FurnaceGUIRenderGL1::getMaxTextureWidth() {
  return maxWidth;
}

int FurnaceGUIRenderGL1::getMaxTextureHeight() {
  return maxHeight;
}

unsigned int FurnaceGUIRenderGL1::getTextureFormats() {
  return GUI_TEXFORMAT_ABGR32;
}

const char* FurnaceGUIRenderGL1::getBackendName() {
  return "OpenGL 1.1";
}

const char* FurnaceGUIRenderGL1::getVendorName() {
  return vendorName.c_str();
}

const char* FurnaceGUIRenderGL1::getDeviceName() {
  return deviceName.c_str();
}

const char* FurnaceGUIRenderGL1::getAPIVersion() {
  return apiVersion.c_str();
}

void FurnaceGUIRenderGL1::setSwapInterval(int swapInterval) {
  SDL_GL_SetSwapInterval(swapInterval);
  if (swapInterval>0 && SDL_GL_GetSwapInterval()==0) {
    swapIntervalSet=false;
    logW("tried to enable VSync but couldn't!");
  } else {
    swapIntervalSet=true;
  }
}

void FurnaceGUIRenderGL1::preInit(const DivConfig& conf) {
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,1);

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE,conf.getInt("glRedSize",8));
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,conf.getInt("glGreenSize",8));
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,conf.getInt("glBlueSize",8));
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,conf.getInt("glAlphaSize",0));
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,conf.getInt("glDoubleBuffer",1));
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,conf.getInt("glDepthSize",24));
}

#define LOAD_PROC_MANDATORY(_v,_t,_s) \
  _v=(_t)SDL_GL_GetProcAddress(_s); \
  if (!_v) { \
    logE(_s " not found"); \
    return false; \
  }

#define LOAD_PROC_OPTIONAL(_v,_t,_s) \
  _v=(_t)SDL_GL_GetProcAddress(_s); \
  if (!_v) { \
    logW(_s " not found"); \
  }

bool FurnaceGUIRenderGL1::init(SDL_Window* win, int swapInterval) {
  sdlWin=win;
  context=SDL_GL_CreateContext(win);
  if (context==NULL) {
    return false;
  }
  SDL_GL_MakeCurrent(win,context);
  SDL_GL_SetSwapInterval(swapInterval);
  if (swapInterval>0 && SDL_GL_GetSwapInterval()==0) {
    swapIntervalSet=false;
    logW("tried to enable VSync but couldn't!");
  } else {
    swapIntervalSet=true;
  }

  const char* next=(const char*)glGetString(GL_VENDOR);
  if (next==NULL) {
    vendorName="???";
  } else {
    vendorName=next;
  }
  next=(const char*)glGetString(GL_RENDERER);
  if (next==NULL) {
    deviceName="???";
  } else {
    deviceName=next;
  }
  next=(const char*)glGetString(GL_VERSION);
  if (next==NULL) {
    apiVersion="???";
  } else {
    apiVersion=next;
  }

  int maxSize=1024;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize);

  maxWidth=maxSize;
  maxHeight=maxSize;

  return true;
}

void FurnaceGUIRenderGL1::initGUI(SDL_Window* win) {
  ImGui_ImplSDL2_InitForOpenGL(win,context);
  ImGui_ImplOpenGL2_Init();
}

bool FurnaceGUIRenderGL1::quit() {
  if (context==NULL) return false;
  SDL_GL_DeleteContext(context);
  context=NULL;
  return true;
}

void FurnaceGUIRenderGL1::quitGUI() { 
  ImGui_ImplOpenGL2_Shutdown();
}

// sadly, OpenGL 1.1 doesn't have the ability to recover from death...
bool FurnaceGUIRenderGL1::isDead() {
   return false;
}
