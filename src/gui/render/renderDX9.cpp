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

class FurnaceDX9Texture: public FurnaceGUITexture {
  public:
  ID3D11Texture2D* tex;
  ID3D11ShaderResourceView* view;
  int width, height;
  unsigned char* lockedData;
  bool dynamic;
  FurnaceDX9Texture():
    tex(NULL),
    view(NULL),
    width(0),
    height(0),
    lockedData(NULL),
    dynamic(false) {}
};

bool FurnaceGUIRenderDX9::destroyRenderTarget() {
  if (renderTarget!=NULL) {
    renderTarget->Release();
    renderTarget=NULL;
    return true;
  }
  return false;
}

bool FurnaceGUIRenderDX9::createRenderTarget() {
  ID3D11Texture2D* screen=NULL;
  HRESULT result;

  destroyRenderTarget();

  if (swapchain==NULL || device==NULL) {
    logW("createRenderTarget: swapchain or device are NULL!");
    return false;
  }

  DXGI_SWAP_CHAIN_DESC chainDesc;
  memset(&chainDesc,0,sizeof(chainDesc));
  if (swapchain->GetDesc(&chainDesc)!=S_OK) {
    logW("createRenderTarget: could not get swapchain desc!");
  } else {
    outW=chainDesc.BufferDesc.Width;
    outH=chainDesc.BufferDesc.Height;
    logI("DX9: buffer desc sizes: %d, %d",chainDesc.BufferDesc.Width,chainDesc.BufferDesc.Height);
  }

  result=swapchain->GetBuffer(0,IID_PPV_ARGS(&screen));
  if (result!=S_OK) {
    logW("createRenderTarget: could not get buffer! %.8x",result);
    return false;
  }
  if (screen==NULL) {
    logW("createRenderTarget: screen is null!");
    return false;
  }

  result=device->CreateRenderTargetView(screen,NULL,&renderTarget);
  if (result!=S_OK) {
    logW("createRenderTarget: could not create render target view! %.8x",result);
    screen->Release();
    return false;
  }
  if (renderTarget==NULL) {
    logW("createRenderTarget: what the hell the render target is null?");
    screen->Release();
    return false;
  }

  screen->Release();
  return true;
}

ImTextureID FurnaceGUIRenderDX9::getTextureID(FurnaceGUITexture* which) {
  FurnaceDX9Texture* t=(FurnaceDX9Texture*)which;
  return (ImTextureID)t->view;
}

bool FurnaceGUIRenderDX9::lockTexture(FurnaceGUITexture* which, void** data, int* pitch) {
  FurnaceDX9Texture* t=(FurnaceDX9Texture*)which;
  if (t->lockedData!=NULL) return false;

  D3D11_MAPPED_SUBRESOURCE mappedRes;
  memset(&mappedRes,0,sizeof(mappedRes));

  HRESULT result=context->Map(t->tex,D3D11CalcSubresource(0,0,1),D3D11_MAP_WRITE_DISCARD,0,&mappedRes);
  if (result!=S_OK) {
    logW("could not map texture! %.8x",result);
    return false;
  }
  t->lockedData=(unsigned char*)mappedRes.pData;
  *data=mappedRes.pData;
  *pitch=mappedRes.RowPitch;

  logV("texture locked... pitch: %d",mappedRes.RowPitch);
  return true;
}

bool FurnaceGUIRenderDX9::unlockTexture(FurnaceGUITexture* which) {
  FurnaceDX9Texture* t=(FurnaceDX9Texture*)which;
  if (t->lockedData==NULL) return false;
  context->Unmap(t->tex,D3D11CalcSubresource(0,0,1));
  t->lockedData=NULL;
  return true;
}

bool FurnaceGUIRenderDX9::updateTexture(FurnaceGUITexture* which, void* data, int pitch) {
  FurnaceDX9Texture* t=(FurnaceDX9Texture*)which;
  if (t->dynamic) {
    unsigned char* d=NULL;
    int p=0;
    if (!lockTexture(t,(void**)&d,&p)) return false;
    if (p==pitch) {
      memcpy(d,data,p*t->height);
    } else {
      unsigned char* ucData=(unsigned char*)data;
      int srcPos=0;
      int destPos=0;
      for (int i=0; i<t->height; i++) {
        memcpy(&d[destPos],&ucData[srcPos],pitch);
        srcPos+=pitch;
        destPos+=p;
      }
    }
    unlockTexture(t);
  } else {
    context->UpdateSubresource(t->tex,D3D11CalcSubresource(0,0,1),NULL,data,pitch,pitch*t->height);
  }
  return true;
}

FurnaceGUITexture* FurnaceGUIRenderDX9::createTexture(bool dynamic, int width, int height, bool interpolate) {
  return ret;
}

bool FurnaceGUIRenderDX9::destroyTexture(FurnaceGUITexture* which) {
  FurnaceDX9Texture* t=(FurnaceDX9Texture*)which;
  delete t;
  return true;
}

void FurnaceGUIRenderDX9::setTextureBlendMode(FurnaceGUITexture* which, FurnaceGUIBlendMode mode) {
}

void FurnaceGUIRenderDX9::setBlendMode(FurnaceGUIBlendMode mode) {
}

void FurnaceGUIRenderDX9::resized(const SDL_Event& ev) {
  destroyRenderTarget();
  logI("DX9: resizing buffers");
  HRESULT result=swapchain->ResizeBuffers(0,(unsigned int)ev.window.data1,(unsigned int)ev.window.data2,DXGI_FORMAT_UNKNOWN,DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
  if (result!=S_OK) {
    if (result==DXGI_ERROR_DEVICE_REMOVED || result==DXGI_ERROR_DEVICE_RESET) {
      dead=true;
    }
    logW("error while resizing swapchain buffers! %.8x",result);
  }
  if (!dead) {
    createRenderTarget();
  }
}

void FurnaceGUIRenderDX9::clear(ImVec4 color) {
  float floatColor[4]={
    color.x*color.w,
    color.y*color.w,
    color.z*color.w,
    color.w,
  };

  context->OMSetRenderTargets(1,&renderTarget,NULL);
  context->ClearRenderTargetView(renderTarget,floatColor);
}

bool FurnaceGUIRenderDX9::newFrame() {
  return ImGui_ImplDX9_NewFrame();
}

bool FurnaceGUIRenderDX9::canVSync() {
  // TODO: find out how to retrieve VSync status
  return true;
}

void FurnaceGUIRenderDX9::createFontsTexture() {
  ImGui_ImplDX9_CreateDeviceObjects();
}

void FurnaceGUIRenderDX9::destroyFontsTexture() {
  ImGui_ImplDX9_InvalidateDeviceObjects();
}

void FurnaceGUIRenderDX9::renderGUI() {
  ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
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

void FurnaceGUIRenderDX9::preInit() {
}

bool FurnaceGUIRenderDX9::init(SDL_Window* win, int swapInt) {
  SDL_SysWMinfo sysWindow;
  D3D_FEATURE_LEVEL featureLevel;

  SDL_VERSION(&sysWindow.version);
  if (SDL_GetWindowWMInfo(win,&sysWindow)==SDL_FALSE) {
    logE("could not get window WM info! %s",SDL_GetError());
    return false;
  }
  HWND window=(HWND)sysWindow.info.win.window;

  return true;
}

void FurnaceGUIRenderDX9::initGUI(SDL_Window* win) {
  ImGui_ImplSDL2_InitForD3D(win);
  ImGui_ImplDX9_Init(device,context);
}

bool FurnaceGUIRenderDX9::quit() {
  dead=false;
  return true;
}

void FurnaceGUIRenderDX9::quitGUI() { 
  ImGui_ImplDX9_Shutdown();
}

bool FurnaceGUIRenderDX9::isDead() {
  return dead;
}
