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
  IDirect3DTexture9* texPre;
  int width, height, widthReal, heightReal;
  FurnaceGUITextureFormat format;
  unsigned char* lockedData;
  bool dynamic;
  FurnaceDX9Texture():
    tex(NULL),
    texPre(NULL),
    width(0),
    height(0),
    widthReal(0),
    heightReal(0),
    format(GUI_TEXFORMAT_UNKNOWN),
    lockedData(NULL),
    dynamic(false) {}
};

ImTextureID FurnaceGUIRenderDX9::getTextureID(FurnaceGUITexture* which) {
  FurnaceDX9Texture* t=(FurnaceDX9Texture*)which;
  return (ImTextureID)t->tex;
}

float FurnaceGUIRenderDX9::getTextureU(FurnaceGUITexture* which) {
  FurnaceDX9Texture* t=(FurnaceDX9Texture*)which;
  if (which==NULL) return 0.0;
  if (t->widthReal<1) return 0.0f;
  return (float)t->width/(float)t->widthReal;
}

float FurnaceGUIRenderDX9::getTextureV(FurnaceGUITexture* which) {
  FurnaceDX9Texture* t=(FurnaceDX9Texture*)which;
  if (which==NULL) return 0.0;
  if (t->heightReal<1) return 0.0f;
  return (float)t->height/(float)t->heightReal;
}

FurnaceGUITextureFormat FurnaceGUIRenderDX9::getTextureFormat(FurnaceGUITexture* which) {
  FurnaceDX9Texture* t=(FurnaceDX9Texture*)which;
  return t->format;
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
  FurnaceDX9Texture* t=(FurnaceDX9Texture*)which;
  IDirect3DTexture9* crap=NULL;
  
  if (t->texPre==NULL) {
    // update by locking
    if (!t->dynamic) {
      logW("updating static texture but texPre does not exist!");
      return false;
    }
    crap=t->tex;
  } else {
    // update by calling UpdateTexture
    crap=t->texPre;
  }

  D3DLOCKED_RECT lockedRect;
  HRESULT result=crap->LockRect(0,&lockedRect,NULL,D3DLOCK_DISCARD);
  if (result!=D3D_OK) {
    logW("could not update texture (lock)! %.8x",result);
    return false;
  }

  if (lockedRect.Pitch==pitch) {
    memcpy(lockedRect.pBits,data,pitch*t->height);
  } else {
    unsigned char* ucData=(unsigned char*)data;
    unsigned char* d=(unsigned char*)lockedRect.pBits;
    int srcPos=0;
    int destPos=0;
    for (int i=0; i<t->height; i++) {
      memcpy(&d[destPos],&ucData[srcPos],pitch);
      srcPos+=pitch;
      destPos+=lockedRect.Pitch;
    }
  }
  
  crap->UnlockRect(0);

  if (t->texPre!=NULL) {
    result=t->tex->AddDirtyRect(NULL);
    if (result!=D3D_OK) {
      logW("could not taint texture! %.8x",result);
    }
    result=device->UpdateTexture(t->texPre,t->tex);
    if (result!=D3D_OK) {
      logW("could not update texture! %.8x",result);
      return false;
    }
  }

  return true;
}

FurnaceGUITexture* FurnaceGUIRenderDX9::createTexture(bool dynamic, int width, int height, bool interpolate, FurnaceGUITextureFormat format) {  
  IDirect3DTexture9* tex=NULL;
  IDirect3DTexture9* texPre=NULL;
  int widthReal=width;
  int heightReal=height;

  if (format!=GUI_TEXFORMAT_ARGB32) {
    logE("unsupported texture format!");
    return NULL;
  }

  if ((widthReal&(widthReal-1))!=0) {
    widthReal=1<<bsr(width);
  }
  if ((heightReal&(heightReal-1))!=0) {
    heightReal=1<<bsr(height);
  }
  if (squareTex) {
    if (widthReal>heightReal) {
      heightReal=widthReal;
    } else {
      widthReal=heightReal;
    }
  }
  logV("width: %d (requested)... %d (actual)",width,widthReal);
  logV("height: %d (requested)... %d (actual)",height,heightReal);

  if (!supportsDynamicTex) dynamic=false;

  HRESULT result=device->CreateTexture(widthReal,heightReal,1,dynamic?D3DUSAGE_DYNAMIC:0,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&tex,NULL);

  if (result!=D3D_OK) {
    logW("could not create texture! %.8x",result);
    return NULL;
  }

  if (!dynamic) {
    HRESULT result=device->CreateTexture(widthReal,heightReal,1,0,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&texPre,NULL);

    if (result!=D3D_OK) {
      logW("could not create pre-texture! %.8x",result);
      tex->Release();
      return NULL;
    }
  }

  FurnaceDX9Texture* ret=new FurnaceDX9Texture;
  ret->width=width;
  ret->height=height;
  ret->widthReal=widthReal;
  ret->heightReal=heightReal;
  ret->tex=tex;
  ret->texPre=texPre;
  ret->dynamic=dynamic;
  ret->format=format;
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
  mustResize=true;
  outW=ev.window.data1;
  outH=ev.window.data2;
}

void FurnaceGUIRenderDX9::clear(ImVec4 color) {
  device->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,ImGui::ColorConvertFloat4ToU32(color),0,0);
}

void FurnaceGUIRenderDX9::present() {
  device->Present(NULL,NULL,NULL,NULL);
}

bool FurnaceGUIRenderDX9::newFrame() {
  if (mustResize) {
    logI("DX9: resizing buffers");
    ImGui_ImplDX9_InvalidateDeviceObjects();
    priv->present.BackBufferWidth=outW;
    priv->present.BackBufferHeight=outH;
    HRESULT result=device->Reset(&priv->present);
    if (result==D3DERR_INVALIDCALL) {
      logE("OH NO");
    }
    ImGui_ImplDX9_CreateDeviceObjects();
    mustResize=false;
  }

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

struct WipeVertex {
  float x, y, z;
  unsigned int color;

  WipeVertex(float _x, float _y, float _z, unsigned int c):
    x(_x),
    y(_y),
    z(_z),
    color(c) {}
  WipeVertex():
    x(0),
    y(0),
    z(0),
    color(0) {}
};

void FurnaceGUIRenderDX9::wipe(float alpha) {
  if (wipeBuf==NULL) return;

  /*

  HRESULT result=device->BeginScene();
  if (result==D3D_OK) {
    D3DVIEWPORT9 view;
    view.X=0;
    view.Y=0;
    view.Width=outW;
    view.Height=outH;
    view.MinZ=0.0f;
    view.MaxZ=1.0f;
    result=device->SetViewport(&view);
    if (result!=D3D_OK) {
      logW("could not set viewport! %.8x",result);
    }

    unsigned int color=alpha*255;

    void* lockedData;
    WipeVertex vertex[4];
    vertex[0]=WipeVertex(0,0,0,color);
    vertex[1]=WipeVertex(outW,0,0,color);
    vertex[2]=WipeVertex(outW,outH,0,color);
    vertex[3]=WipeVertex(0,outH,0,color);

    result=wipeBuf->Lock(0,0,&lockedData,D3DLOCK_DISCARD);
    if (result==D3D_OK) {
      memcpy(lockedData,vertex,sizeof(WipeVertex)*4);
      wipeBuf->Unlock();

      device->SetStreamSource(0,wipeBuf,0,sizeof(WipeVertex));
      device->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE);
      device->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,1);
    }
    
    device->EndScene();
  }
  */
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

unsigned int FurnaceGUIRenderDX9::getTextureFormats() {
  return GUI_TEXFORMAT_ARGB32;
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

  SDL_GetWindowSize(win,&outW,&outH);

  memset(&priv->present,0,sizeof(D3DPRESENT_PARAMETERS));
  priv->present.Windowed=TRUE;
  priv->present.SwapEffect=D3DSWAPEFFECT_DISCARD;
  priv->present.BackBufferWidth=outW;
  priv->present.BackBufferHeight=outH;
  priv->present.BackBufferCount=1;
  priv->present.BackBufferFormat=D3DFMT_UNKNOWN;
  //priv->present.EnableAutoDepthStencil=TRUE;
  //priv->present.AutoDepthStencilFormat=D3DFMT_D16;
  if (swapInt>0) {
    priv->present.PresentationInterval=D3DPRESENT_INTERVAL_ONE;
  } else {
    priv->present.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
  }
  priv->present.hDeviceWindow=window;
  
  HRESULT result=iface->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,window,D3DCREATE_HARDWARE_VERTEXPROCESSING,&priv->present,&device);

  if (result!=D3D_OK) {
    logW("no hardware vertex processing!");
    result=iface->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,window,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&priv->present,&device);
    if (result!=D3D_OK) {
      logE("could not create device! %.8x",result);
      iface->Release();
      iface=NULL;
      return false;
    }
  }

  D3DCAPS9 caps;

  result=device->GetDeviceCaps(&caps);

  if (result==D3D_OK) {
    supportsDynamicTex=(caps.Caps2&D3DCAPS2_DYNAMICTEXTURES);
    squareTex=(caps.TextureCaps&D3DPTEXTURECAPS_SQUAREONLY);
    supportsVSync=(caps.PresentationIntervals&D3DPRESENT_INTERVAL_ONE);
    maxWidth=caps.MaxTextureWidth;
    maxHeight=caps.MaxTextureHeight;

    if (!supportsDynamicTex) {
      logI("no support for dynamic textures");
    }
    if (squareTex) {
      logI("square textures only");
    }
  }

  result=device->CreateVertexBuffer(sizeof(WipeVertex)*4,0,D3DFVF_XYZ|D3DFVF_DIFFUSE,D3DPOOL_DEFAULT,&wipeBuf,NULL);

  if (result!=D3D_OK) {
    logE("could not create wipe buffer! %.8x",result);
  }

  return true;
}

void FurnaceGUIRenderDX9::initGUI(SDL_Window* win) {
  ImGui_ImplSDL2_InitForD3D(win);
  ImGui_ImplDX9_Init(device);
}

bool FurnaceGUIRenderDX9::quit() {
  if (wipeBuf) {
    wipeBuf->Release();
    wipeBuf=NULL;
  }
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
