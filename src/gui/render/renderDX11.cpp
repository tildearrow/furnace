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

#define INCLUDE_D3D11
#include "renderDX11.h"
#include <SDL_syswm.h>
#include "backends/imgui_impl_dx11.h"
#include "../../ta-log.h"

const D3D_FEATURE_LEVEL possibleFeatureLevels[2]={
  D3D_FEATURE_LEVEL_11_0,
  D3D_FEATURE_LEVEL_10_0
};

struct FurnaceDXTexture {
  ID3D11Texture2D* tex;
  ID3D11ShaderResourceView* view;
  int width, height;
  unsigned char* lockedData;
  FurnaceDXTexture():
    tex(NULL),
    view(NULL),
    width(0),
    height(0),
    lockedData(NULL) {}
};

bool FurnaceGUIRenderDX11::destroyRenderTarget() {
  if (renderTarget!=NULL) {
    renderTarget->Release();
    renderTarget=NULL;
    return true;
  }
  return false;
}

bool FurnaceGUIRenderDX11::createRenderTarget() {
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

  return true;
}

ImTextureID FurnaceGUIRenderDX11::getTextureID(void* which) {
  FurnaceDXTexture* t=(FurnaceDXTexture*)which;
  return (ImTextureID)t->view;
}

bool FurnaceGUIRenderDX11::lockTexture(void* which, void** data, int* pitch) {
  FurnaceDXTexture* t=(FurnaceDXTexture*)which;
  if (t->lockedData!=NULL) return false;

  D3D11_MAPPED_SUBRESOURCE mappedRes;
  memset(&mappedRes,0,sizeof(mappedRes));

  HRESULT result=context->Map(t->tex,D3D11CalcSubresource(0,0,1),D3D11_MAP_WRITE,0,&mappedRes);
  if (result!=S_OK) {
    logW("could not map texture!");
    return false;
  }
  t->lockedData=(unsigned char*)mappedRes.pData;
  *data=mappedRes.pData;
  *pitch=mappedRes.RowPitch;
  return true;
}

bool FurnaceGUIRenderDX11::unlockTexture(void* which) {
  FurnaceDXTexture* t=(FurnaceDXTexture*)which;
  if (t->lockedData==NULL) return false;
  context->Unmap(t->tex,D3D11CalcSubresource(0,0,1));
  t->lockedData=NULL;
  return true;
}

bool FurnaceGUIRenderDX11::updateTexture(void* which, void* data, int pitch) {
  FurnaceDXTexture* t=(FurnaceDXTexture*)which;
  context->UpdateSubresource(t->tex,D3D11CalcSubresource(0,0,1),NULL,data,pitch,pitch*t->height);
  return true;
}

void* FurnaceGUIRenderDX11::createTexture(bool dynamic, int width, int height) {
  D3D11_TEXTURE2D_DESC texDesc;
  D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
  ID3D11Texture2D* tex=NULL;
  ID3D11ShaderResourceView* view=NULL;
  HRESULT result;

  memset(&texDesc,0,sizeof(texDesc));
  memset(&viewDesc,0,sizeof(viewDesc));

  texDesc.Width=width;
  texDesc.Height=height;
  texDesc.MipLevels=1;
  texDesc.ArraySize=1;
  texDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM; // ???
  texDesc.SampleDesc.Count=1;
  texDesc.SampleDesc.Quality=0;
  texDesc.Usage=dynamic?D3D11_USAGE_DYNAMIC:D3D11_USAGE_DEFAULT;
  texDesc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
  texDesc.CPUAccessFlags=dynamic?D3D11_CPU_ACCESS_WRITE:0;
  texDesc.MiscFlags=0;

  result=device->CreateTexture2D(&texDesc,NULL,&tex);
  if (result!=S_OK) {
    logW("could not create texture! %.8x",result);
    return NULL;
  }

  viewDesc.Format=texDesc.Format=texDesc.Format;
  viewDesc.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D;
  viewDesc.Texture2D.MostDetailedMip=0;
  viewDesc.Texture2D.MipLevels=texDesc.MipLevels;

  result=device->CreateShaderResourceView(tex,&viewDesc,&view);
  if (result!=S_OK) {
    logW("could not create texture view! %.8x",result);
    tex->Release();
    return NULL;
  }

  FurnaceDXTexture* ret=new FurnaceDXTexture;
  ret->width=width;
  ret->height=height;
  ret->tex=tex;
  ret->view=view;
  textures.push_back(ret);
  return ret;
}

bool FurnaceGUIRenderDX11::destroyTexture(void* which) {
  FurnaceDXTexture* t=(FurnaceDXTexture*)which;
  t->view->Release();
  t->tex->Release();
  delete t;

  for (size_t i=0; i<textures.size(); i++) {
    if (textures[i]==t) {
      textures.erase(textures.begin()+i);
      break;
    }
  }
  return true;
}

void FurnaceGUIRenderDX11::setTextureBlendMode(void* which, FurnaceGUIBlendMode mode) {
}

void FurnaceGUIRenderDX11::setBlendMode(FurnaceGUIBlendMode mode) {
}

void FurnaceGUIRenderDX11::resized(const SDL_Event& ev) {
  destroyRenderTarget();
  swapchain->ResizeBuffers(0,0,0,DXGI_FORMAT_UNKNOWN,0);
  createRenderTarget();
}

void FurnaceGUIRenderDX11::clear(ImVec4 color) {
  float floatColor[4]={
    color.x*color.w,
    color.y*color.w,
    color.z*color.w,
    color.w,
  };

  context->OMSetRenderTargets(1,&renderTarget,NULL);
  context->ClearRenderTargetView(renderTarget,floatColor);
}

bool FurnaceGUIRenderDX11::newFrame() {
  ImGui_ImplDX11_NewFrame();
  return true;
}

void FurnaceGUIRenderDX11::createFontsTexture() {
  ImGui_ImplDX11_CreateDeviceObjects();
}

void FurnaceGUIRenderDX11::destroyFontsTexture() {
  ImGui_ImplDX11_InvalidateDeviceObjects();
}

void FurnaceGUIRenderDX11::renderGUI() {
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void FurnaceGUIRenderDX11::wipe(float alpha) {
  // TODO
}

void FurnaceGUIRenderDX11::present() {
  swapchain->Present(1,0);
}

bool FurnaceGUIRenderDX11::getOutputSize(int& w, int& h) {
  w=outW;
  h=outH;
  return true;
}

int FurnaceGUIRenderDX11::getWindowFlags() {
  return 0;
}

void FurnaceGUIRenderDX11::preInit() {
}

bool FurnaceGUIRenderDX11::init(SDL_Window* win) {
  SDL_SysWMinfo sysWindow;
  D3D_FEATURE_LEVEL featureLevel;

  SDL_VERSION(&sysWindow.version);
  if (SDL_GetWindowWMInfo(win,&sysWindow)==SDL_FALSE) {
    logE("could not get window WM info! %s",SDL_GetError());
    return false;
  }
  HWND window=(HWND)sysWindow.info.win.window;

  DXGI_SWAP_CHAIN_DESC chainDesc;
  memset(&chainDesc,0,sizeof(chainDesc));
  chainDesc.BufferDesc.Width=0;
  chainDesc.BufferDesc.Height=0;
  chainDesc.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
  chainDesc.BufferDesc.RefreshRate.Numerator=60;
  chainDesc.BufferDesc.RefreshRate.Denominator=1;
  chainDesc.SampleDesc.Count=1;
  chainDesc.SampleDesc.Quality=0;
  chainDesc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
  chainDesc.BufferCount=2;
  chainDesc.OutputWindow=window;
  chainDesc.Windowed=TRUE; // TODO: what if we're in full screen mode?
  chainDesc.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
  chainDesc.Flags=DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  HRESULT result=D3D11CreateDeviceAndSwapChain(NULL,D3D_DRIVER_TYPE_HARDWARE,NULL,0,possibleFeatureLevels,2,D3D11_SDK_VERSION,&chainDesc,&swapchain,&device,&featureLevel,&context);
  if (result!=S_OK) {
    logE("could not create device and/or swap chain! %.8x",result);
    return false;
  }

  createRenderTarget();
  return true;
}

void FurnaceGUIRenderDX11::initGUI(SDL_Window* win) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui_ImplSDL2_InitForD3D(win);
  ImGui_ImplDX11_Init(device,context);
}

bool FurnaceGUIRenderDX11::quit() {
  destroyRenderTarget();

  for (FurnaceDXTexture* i: textures) {
    i->view->Release();
    i->tex->Release();
    delete i;
  }
  textures.clear();

  if (swapchain!=NULL) {
    swapchain->Release();
    swapchain=NULL;
  }
  if (context!=NULL) {
    context->Release();
    context=NULL;
  }
  if (device!=NULL) {
    device->Release();
    device=NULL;
  }
  return true;
}

void FurnaceGUIRenderDX11::quitGUI() { 
  ImGui_ImplDX11_Shutdown();
}
