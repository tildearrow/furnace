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

#define INCLUDE_D3D9
#include "renderDX9.h"
#include <SDL_syswm.h>
#include "backends/imgui_impl_dx9.h"
#include "../../ta-log.h"
#include "../../utfutils.h"
#include "../engine/bsr.h"

class FurnaceDX9Texture: public FurnaceGUITexture {
  public:
  IDirect3DTexture9* tex;
  int width, height, widthReal, heightReal;
  unsigned char* lockedData;
  bool dynamic;
  FurnaceDX9Texture():
    tex(NULL),
    width(0),
    height(0),
    widthReal(0),
    heightReal(0),
    lockedData(NULL),
    dynamic(false) {}
};

ImTextureID FurnaceGUIRenderDX9::getTextureID(FurnaceGUITexture* which) {
  FurnaceDX9Texture* t=(FurnaceDX9Texture*)which;
  return (ImTextureID)t->tex;
}

bool FurnaceGUIRenderDX9::lockTexture(FurnaceGUITexture* which, void** data, int* pitch) {
  FurnaceDX9Texture* t=(FurnaceDX9Texture*)which;
  D3DLOCKED_RECT lockedRect;

  HRESULT result=t->tex->LockRect(0,&lockedRect,NULL,D3DLOCK_DISCARD);

  if (result!=D3D_OK) {
    logW("could not lock texture!");
    return false;
  }

  *data=lockedRect.pBits;
  *pitch=lockedRect.Pitch;
  return true;
}

bool FurnaceGUIRenderDX9::unlockTexture(FurnaceGUITexture* which) {
  FurnaceDX9Texture* t=(FurnaceDX9Texture*)which;
  HRESULT result=t->tex->UnlockRect(0);

  if (result!=D3D_OK) {
    logW("could not unlock texture!");
    return false;
  }

  return true;
}

bool FurnaceGUIRenderDX9::updateTexture(FurnaceGUITexture* which, void* data, int pitch) {
  // TODO
  return false;
}

FurnaceGUITexture* FurnaceGUIRenderDX9::createTexture(bool dynamic, int width, int height, bool interpolate) {  
  IDirect3DTexture9* tex=NULL;
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

  HRESULT result=device->CreateTexture(widthReal,heightReal,1,dynamic?D3DUSAGE_DYNAMIC:0,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&tex,NULL);

  if (result!=D3D_OK) {
    logW("could not create texture! %.8x",result);
    return NULL;
  }

  FurnaceDX9Texture* ret=new FurnaceDX9Texture;
  ret->width=width;
  ret->height=height;
  ret->widthReal=widthReal;
  ret->heightReal=heightReal;
  ret->tex=tex;
  ret->dynamic=dynamic;
  return ret;
}

bool FurnaceGUIRenderDX9::destroyTexture(FurnaceGUITexture* which) {
  FurnaceDX9Texture* t=(FurnaceDX9Texture*)which;
  t->tex->Release();
  delete t;
  return true;
}

void FurnaceGUIRenderDX9::setTextureBlendMode(FurnaceGUITexture* which, FurnaceGUIBlendMode mode) {
}

void FurnaceGUIRenderDX9::setBlendMode(FurnaceGUIBlendMode mode) {
}

void FurnaceGUIRenderDX9::resized(const SDL_Event& ev) {
  logI("DX9: resizing buffers");
  ImGui_ImplDX9_InvalidateDeviceObjects();
  HRESULT result=device->Reset(&priv->present);
  if (result==D3DERR_INVALIDCALL) {
    logE("OH NO");
  }
  ImGui_ImplDX9_CreateDeviceObjects();
}

void FurnaceGUIRenderDX9::clear(ImVec4 color) {
  device->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,ImGui::ColorConvertFloat4ToU32(color),0,0);
}

void FurnaceGUIRenderDX9::present() {
  device->Present(NULL,NULL,NULL,NULL);
}

bool FurnaceGUIRenderDX9::newFrame() {
  return ImGui_ImplDX9_NewFrame();
}

bool FurnaceGUIRenderDX9::canVSync() {
  return supportsVSync;
}

void FurnaceGUIRenderDX9::createFontsTexture() {
  ImGui_ImplDX9_CreateDeviceObjects();
}

void FurnaceGUIRenderDX9::destroyFontsTexture() {
  ImGui_ImplDX9_InvalidateDeviceObjects();
}

void FurnaceGUIRenderDX9::renderGUI() {
  HRESULT result=device->BeginScene();
  if (result==D3D_OK) {
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    device->EndScene();
  } else {
    logW("couldn't render GUI! %.8x",result);
  }
}

void FurnaceGUIRenderDX9::wipe(float alpha) {
}

bool FurnaceGUIRenderDX9::getOutputSize(int& w, int& h) {
  w=outW;
  h=outH;
  return true;
}

int FurnaceGUIRenderDX9::getWindowFlags() {
  return 0;
}

int FurnaceGUIRenderDX9::getMaxTextureWidth() {
  return maxWidth;
}

int FurnaceGUIRenderDX9::getMaxTextureHeight() {
  return maxHeight;
}

const char* FurnaceGUIRenderDX9::getBackendName() {
  return "DirectX 9";
}

const char* FurnaceGUIRenderDX9::getVendorName() {
  return vendorName.c_str();
}

const char* FurnaceGUIRenderDX9::getDeviceName() {
  return deviceName.c_str();
}

const char* FurnaceGUIRenderDX9::getAPIVersion() {
  return apiVersion.c_str();
}

void FurnaceGUIRenderDX9::setSwapInterval(int swapInt) {
  swapInterval=swapInt;
}

void FurnaceGUIRenderDX9::preInit(const DivConfig& conf) {
}

bool FurnaceGUIRenderDX9::init(SDL_Window* win, int swapInt) {
  SDL_SysWMinfo sysWindow;

  SDL_VERSION(&sysWindow.version);
  if (SDL_GetWindowWMInfo(win,&sysWindow)==SDL_FALSE) {
    logE("could not get window WM info! %s",SDL_GetError());
    return false;
  }
  HWND window=(HWND)sysWindow.info.win.window;

  iface=Direct3DCreate9(D3D_SDK_VERSION);
  if (iface==NULL) {
    logE("could not create Direct3D 9!");
    return false;
  }

  priv=new FurnaceGUIRenderDX9Private;

  memset(&priv->present,0,sizeof(D3DPRESENT_PARAMETERS));
  priv->present.Windowed=TRUE;
  priv->present.SwapEffect=D3DSWAPEFFECT_DISCARD;
  priv->present.BackBufferFormat=D3DFMT_UNKNOWN;
  priv->present.EnableAutoDepthStencil=TRUE;
  priv->present.AutoDepthStencilFormat=D3DFMT_D16;
  if (swapInt>0) {
    priv->present.PresentationInterval=D3DPRESENT_INTERVAL_ONE;
  } else {
    priv->present.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
  }
  priv->present.hDeviceWindow=window;
  
  HRESULT result=iface->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,window,D3DCREATE_HARDWARE_VERTEXPROCESSING,&priv->present,&device);

  if (result!=D3D_OK) {
    logE("could not create device! %.8x",result);
    iface->Release();
    iface=NULL;
    return false;
  }

  D3DCAPS9 caps;

  result=device->GetDeviceCaps(&caps);

  if (result==D3D_OK) {
    supportsDynamicTex=(caps.Caps2&D3DCAPS2_DYNAMICTEXTURES);
    supportsVSync=(caps.PresentationIntervals&D3DPRESENT_INTERVAL_ONE);
    maxWidth=caps.MaxTextureWidth;
    maxHeight=caps.MaxTextureHeight;
  }

  return true;
}

void FurnaceGUIRenderDX9::initGUI(SDL_Window* win) {
  ImGui_ImplSDL2_InitForD3D(win);
  ImGui_ImplDX9_Init(device);
}

bool FurnaceGUIRenderDX9::quit() {
  if (device) {
    device->Release();
    device=NULL;
  }
  if (iface) {
    iface->Release();
    iface=NULL;
  }
  dead=false;
  return true;
}

void FurnaceGUIRenderDX9::quitGUI() { 
  ImGui_ImplDX9_Shutdown();
}

bool FurnaceGUIRenderDX9::isDead() {
  return dead;
}
