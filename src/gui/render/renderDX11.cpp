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
  return NULL;
}

bool FurnaceGUIRenderDX11::lockTexture(void* which, void** data, int* pitch) {
  return false;
}

bool FurnaceGUIRenderDX11::unlockTexture(void* which) {
  return false;
}

bool FurnaceGUIRenderDX11::updateTexture(void* which, void* data, int pitch) {
  return false;
}

void* FurnaceGUIRenderDX11::createTexture(bool dynamic, int width, int height) {
  return NULL;
}

bool FurnaceGUIRenderDX11::destroyTexture(void* which) {
  return false;
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
