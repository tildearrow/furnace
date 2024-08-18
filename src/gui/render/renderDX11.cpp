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

#define INCLUDE_D3D11
#include "renderDX11.h"
#include <SDL_syswm.h>
#include "backends/imgui_impl_dx11.h"
#include "../../ta-log.h"
#include "../../utfutils.h"

typedef HRESULT (__stdcall *D3DCompile_t)(LPCVOID,SIZE_T,LPCSTR,D3D_SHADER_MACRO*,ID3DInclude*,LPCSTR,LPCSTR,UINT,UINT,ID3DBlob**,ID3DBlob*);

const char* shD3D11_wipe_srcV=
  "cbuffer WipeUniform: register(b0) {\n"
  "  float alpha;\n"
  "  float padding1;\n"
  "  float padding2;\n"
  "  float padding3;\n"
  "  float4 padding4;\n"
  "};\n"
  "\n"
  "struct vsInput {\n"
  "  float4 pos: POSITION;\n"
  "};\n"
  "\n"
  "struct fsInput {\n"
  "  float4 pos: SV_POSITION;\n"
  "  float4 color: COLOR0;\n"
  "};\n"
  "\n"
  "fsInput main(vsInput input) {\n"
  "  fsInput output;\n"
  "  output.pos=input.pos;\n"
  "  output.color=float4(0.0f,0.0f,0.0f,alpha);\n"
  "  return output;\n"
  "}";

const char* shD3D11_wipe_srcF=
  "struct fsInput {\n"
  "  float4 pos: SV_POSITION;\n"
  "  float4 color: COLOR0;\n"
  "};\n"
  "\n"
  "float4 main(fsInput input): SV_Target {\n"
  "  return input.color;\n"
  "}";

const D3D11_INPUT_ELEMENT_DESC shD3D11_wipe_inputLayout={
  "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0
};

const D3D_FEATURE_LEVEL possibleFeatureLevels[2]={
  D3D_FEATURE_LEVEL_11_0,
  D3D_FEATURE_LEVEL_10_0
};

class FurnaceDXTexture: public FurnaceGUITexture {
  public:
  ID3D11Texture2D* tex;
  ID3D11ShaderResourceView* view;
  int width, height;
  FurnaceGUITextureFormat format;
  unsigned char* lockedData;
  bool dynamic;
  FurnaceDXTexture():
    tex(NULL),
    view(NULL),
    width(0),
    height(0),
    format(GUI_TEXFORMAT_UNKNOWN),
    lockedData(NULL),
    dynamic(false) {}
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
    logI("DX11: buffer desc sizes: %d, %d",chainDesc.BufferDesc.Width,chainDesc.BufferDesc.Height);
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

ImTextureID FurnaceGUIRenderDX11::getTextureID(FurnaceGUITexture* which) {
  FurnaceDXTexture* t=(FurnaceDXTexture*)which;
  return (ImTextureID)t->view;
}

FurnaceGUITextureFormat FurnaceGUIRenderDX11::getTextureFormat(FurnaceGUITexture* which) {
  FurnaceDXTexture* t=(FurnaceDXTexture*)which;
  return t->format;
}

bool FurnaceGUIRenderDX11::lockTexture(FurnaceGUITexture* which, void** data, int* pitch) {
  FurnaceDXTexture* t=(FurnaceDXTexture*)which;
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

bool FurnaceGUIRenderDX11::unlockTexture(FurnaceGUITexture* which) {
  FurnaceDXTexture* t=(FurnaceDXTexture*)which;
  if (t->lockedData==NULL) return false;
  context->Unmap(t->tex,D3D11CalcSubresource(0,0,1));
  t->lockedData=NULL;
  return true;
}

bool FurnaceGUIRenderDX11::updateTexture(FurnaceGUITexture* which, void* data, int pitch) {
  FurnaceDXTexture* t=(FurnaceDXTexture*)which;
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

FurnaceGUITexture* FurnaceGUIRenderDX11::createTexture(bool dynamic, int width, int height, bool interpolate, FurnaceGUITextureFormat format) {
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
  switch (format) {
    case GUI_TEXFORMAT_ABGR32:
      texDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
      break;
    case GUI_TEXFORMAT_ARGB32:
      texDesc.Format=DXGI_FORMAT_B8G8R8A8_UNORM;
      break;
    default:
      logE("unsupported texture format!");
      return NULL;
  }
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
  ret->dynamic=dynamic;
  ret->format=format;
  return ret;
}

bool FurnaceGUIRenderDX11::destroyTexture(FurnaceGUITexture* which) {
  FurnaceDXTexture* t=(FurnaceDXTexture*)which;
  t->view->Release();
  t->tex->Release();
  delete t;
  return true;
}

void FurnaceGUIRenderDX11::setTextureBlendMode(FurnaceGUITexture* which, FurnaceGUIBlendMode mode) {
}

void FurnaceGUIRenderDX11::setBlendMode(FurnaceGUIBlendMode mode) {
}

void FurnaceGUIRenderDX11::resized(const SDL_Event& ev) {
  destroyRenderTarget();
  logI("DX11: resizing buffers");
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
  return ImGui_ImplDX11_NewFrame();
}

bool FurnaceGUIRenderDX11::canVSync() {
  // TODO: find out how to retrieve VSync status
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

const float blendFactor[4]={
  1.0f, 1.0f, 1.0f, 1.0f
};

void FurnaceGUIRenderDX11::wipe(float alpha) {
  D3D11_VIEWPORT viewPort;
  unsigned int strides=4*sizeof(float);
  unsigned int offsets=0;

  memset(&viewPort,0,sizeof(viewPort));
  viewPort.TopLeftX=0.0f;
  viewPort.TopLeftY=0.0f;
  viewPort.Width=outW;
  viewPort.Height=outH;
  viewPort.MinDepth=0.0f;
  viewPort.MaxDepth=1.0f;

  D3D11_MAPPED_SUBRESOURCE mappedUniform;
  if (context->Map(sh_wipe_uniform,0,D3D11_MAP_WRITE_DISCARD,0,&mappedUniform)!=S_OK) {
    logW("could not map constant");
  }
  WipeUniform* sh_wipe_uniformState=(WipeUniform*)mappedUniform.pData;
  sh_wipe_uniformState->alpha=alpha;
  context->Unmap(sh_wipe_uniform,0);

  context->RSSetViewports(1,&viewPort);
  context->RSSetState(rsState);

  context->OMSetBlendState(omBlendState,blendFactor,0xffffffff);
  context->IASetInputLayout(sh_wipe_inputLayout);
  context->IASetVertexBuffers(0,1,&quadVertex,&strides,&offsets);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  context->VSSetShader(sh_wipe_vertex,NULL,0);
  context->VSSetConstantBuffers(0,1,&sh_wipe_uniform);
  context->PSSetShader(sh_wipe_fragment,NULL,0);

  context->Draw(4,0);
}

void FurnaceGUIRenderDX11::present() {
  HRESULT result=swapchain->Present(swapInterval,0);
  if (result==DXGI_ERROR_DEVICE_REMOVED || result==DXGI_ERROR_DEVICE_RESET) {
    dead=true;
  } else if (result!=S_OK && result!=DXGI_STATUS_OCCLUDED) {
    logE("DX11: present failed! %.8x",result);
  }
}

bool FurnaceGUIRenderDX11::getOutputSize(int& w, int& h) {
  w=outW;
  h=outH;
  return true;
}

int FurnaceGUIRenderDX11::getWindowFlags() {
  return 0;
}

int FurnaceGUIRenderDX11::getMaxTextureWidth() {
  return maxWidth;
}

int FurnaceGUIRenderDX11::getMaxTextureHeight() {
  return maxHeight;
}

unsigned int FurnaceGUIRenderDX11::getTextureFormats() {
  return GUI_TEXFORMAT_ABGR32|GUI_TEXFORMAT_ARGB32;
}

const char* FurnaceGUIRenderDX11::getBackendName() {
  return "DirectX 11";
}

const char* FurnaceGUIRenderDX11::getVendorName() {
  return vendorName.c_str();
}

const char* FurnaceGUIRenderDX11::getDeviceName() {
  return deviceName.c_str();
}

const char* FurnaceGUIRenderDX11::getAPIVersion() {
  return apiVersion.c_str();
}

void FurnaceGUIRenderDX11::setSwapInterval(int swapInt) {
  swapInterval=swapInt;
}

void FurnaceGUIRenderDX11::preInit(const DivConfig& conf) {
}

const float wipeVertices[4][4]={
  {-1.0, -1.0, 0.0, 1.0},
  { 1.0, -1.0, 0.0, 1.0},
  {-1.0,  1.0, 0.0, 1.0},
  { 1.0,  1.0, 0.0, 1.0}
};

bool FurnaceGUIRenderDX11::init(SDL_Window* win, int swapInt) {
  SDL_SysWMinfo sysWindow;
  D3D_FEATURE_LEVEL featureLevel;

  SDL_VERSION(&sysWindow.version);
  if (SDL_GetWindowWMInfo(win,&sysWindow)==SDL_FALSE) {
    logE("could not get window WM info! %s",SDL_GetError());
    return false;
  }
  HWND window=(HWND)sysWindow.info.win.window;

  // prepare swapchain
  swapInterval=swapInt;

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

  // initialize
  HRESULT result=D3D11CreateDeviceAndSwapChain(NULL,D3D_DRIVER_TYPE_HARDWARE,NULL,0,possibleFeatureLevels,2,D3D11_SDK_VERSION,&chainDesc,&swapchain,&device,&featureLevel,&context);
  if (result!=S_OK) {
    logE("could not create device and/or swap chain! %.8x",result);
    return false;
  }

  IDXGIDevice* giDevice=NULL;
  IDXGIAdapter* adapter=NULL;

  result=device->QueryInterface(__uuidof(IDXGIDevice),(void**)&giDevice);
  if (result==S_OK) {
    result=giDevice->GetAdapter(&adapter);
    if (result==S_OK) {
      DXGI_ADAPTER_DESC adapterDesc;

      result=adapter->GetDesc(&adapterDesc);
      if (result!=S_OK) {
        logE("could not get adapter info! %.8x",result);
      } else {
        deviceName=utf16To8(adapterDesc.Description);
        vendorName=fmt::sprintf("%.4x:%.4x",adapterDesc.VendorId,adapterDesc.DeviceId);
        logV("device: %s",deviceName);
      }
    } else {
      logE("could not get adapter! %.8x",result);
      logE("won't be able to get adapter info...");
    }
  } else {
    logE("could not query interface! %.8x",result);
    logE("won't be able to get adapter info...");
  }

  if (featureLevel>=0xb000) {
    maxWidth=16384;
    maxHeight=16384;
  } else if (featureLevel>=0xa000) {
    maxWidth=8192;
    maxHeight=8192;
  } else {
    maxWidth=4096;
    maxHeight=4096;
  }
  apiVersion=fmt::sprintf("%d.%d",((int)featureLevel)>>12,((int)featureLevel)>>8);

  // https://github.com/ocornut/imgui/pull/638
  D3DCompile_t D3DCompile=NULL;
  char dllBuffer[20];
  for (int i=47; (i>30 && !D3DCompile); i--) {
    snprintf(dllBuffer,20,"d3dcompiler_%d.dll",i);
    HMODULE hDll=LoadLibraryA(dllBuffer);
    if (hDll) {
      D3DCompile=(D3DCompile_t)GetProcAddress(hDll,"D3DCompile");
    }
  }
  if (!D3DCompile) {
    logE("could not find D3DCompile!");
    return false;
  }

  // create wipe shader
  ID3DBlob* wipeBlobV=NULL;
  ID3DBlob* wipeBlobF=NULL;
  D3D11_BUFFER_DESC wipeConstantDesc;

  result=D3DCompile(shD3D11_wipe_srcV,strlen(shD3D11_wipe_srcV),NULL,NULL,NULL,"main","vs_4_0",0,0,&wipeBlobV,NULL);
  if (result!=S_OK) {
    logE("could not compile vertex shader! %.8x",result);
    return false;
  }
  result=D3DCompile(shD3D11_wipe_srcF,strlen(shD3D11_wipe_srcF),NULL,NULL,NULL,"main","ps_4_0",0,0,&wipeBlobF,NULL);
  if (result!=S_OK) {
    logE("could not compile pixel shader! %.8x",result);
    return false;
  }

  result=device->CreateVertexShader(wipeBlobV->GetBufferPointer(),wipeBlobV->GetBufferSize(),NULL,&sh_wipe_vertex);
  if (result!=S_OK) {
    logE("could not create vertex shader! %.8x",result);
    return false;
  }
  result=device->CreatePixelShader(wipeBlobF->GetBufferPointer(),wipeBlobF->GetBufferSize(),NULL,&sh_wipe_fragment);
  if (result!=S_OK) {
    logE("could not create pixel shader! %.8x",result);
    return false;
  }

  result=device->CreateInputLayout(&shD3D11_wipe_inputLayout,1,wipeBlobV->GetBufferPointer(),wipeBlobV->GetBufferSize(),&sh_wipe_inputLayout);
  if (result!=S_OK) {
    logE("could not create input layout! %.8x",result);
    return false;
  }

  memset(&wipeConstantDesc,0,sizeof(wipeConstantDesc));
  wipeConstantDesc.ByteWidth=sizeof(WipeUniform);
  wipeConstantDesc.Usage=D3D11_USAGE_DYNAMIC;
  wipeConstantDesc.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
  wipeConstantDesc.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
  wipeConstantDesc.MiscFlags=0;
  wipeConstantDesc.StructureByteStride=0;

  result=device->CreateBuffer(&wipeConstantDesc,NULL,&sh_wipe_uniform);
  if (result!=S_OK) {
    logE("could not create constant buffer! %.8x",result);
    return false;
  }

  // create wipe vertices
  D3D11_BUFFER_DESC vertexDesc;
  D3D11_SUBRESOURCE_DATA vertexRes;

  memset(&vertexDesc,0,sizeof(vertexDesc));
  memset(&vertexRes,0,sizeof(vertexRes));

  vertexDesc.ByteWidth=4*4*sizeof(float);
  vertexDesc.Usage=D3D11_USAGE_DEFAULT;
  vertexDesc.BindFlags=D3D11_BIND_VERTEX_BUFFER;
  vertexDesc.CPUAccessFlags=0;
  vertexDesc.MiscFlags=0;
  vertexDesc.StructureByteStride=0;

  vertexRes.pSysMem=wipeVertices;
  vertexRes.SysMemPitch=0;
  vertexRes.SysMemSlicePitch=0;

  result=device->CreateBuffer(&vertexDesc,&vertexRes,&quadVertex);
  if (result!=S_OK) {
    logE("could not create vertex buffer! %.8x",result);
    return false;
  }

  // initialize the rest
  D3D11_RASTERIZER_DESC rasterDesc;
  D3D11_BLEND_DESC blendDesc;

  memset(&rasterDesc,0,sizeof(rasterDesc));
  memset(&blendDesc,0,sizeof(blendDesc));

  rasterDesc.FillMode=D3D11_FILL_SOLID;
  rasterDesc.CullMode=D3D11_CULL_NONE;
  rasterDesc.FrontCounterClockwise=false;
  rasterDesc.DepthBias=0;
  rasterDesc.DepthBiasClamp=0.0f;
  rasterDesc.SlopeScaledDepthBias=0.0f;
  rasterDesc.DepthClipEnable=false;
  rasterDesc.ScissorEnable=false;
  rasterDesc.MultisampleEnable=false;
  rasterDesc.AntialiasedLineEnable=false;
  result=device->CreateRasterizerState(&rasterDesc,&rsState);
  if (result!=S_OK) {
    logE("could not create rasterizer state! %.8x",result);
    return false;
  }

  blendDesc.AlphaToCoverageEnable=false;
  blendDesc.IndependentBlendEnable=false;
  blendDesc.RenderTarget[0].BlendEnable=true;
  blendDesc.RenderTarget[0].SrcBlend=D3D11_BLEND_SRC_ALPHA;
  blendDesc.RenderTarget[0].DestBlend=D3D11_BLEND_INV_SRC_ALPHA;
  blendDesc.RenderTarget[0].BlendOp=D3D11_BLEND_OP_ADD;
  blendDesc.RenderTarget[0].SrcBlendAlpha=D3D11_BLEND_ONE;
  blendDesc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;
  blendDesc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
  blendDesc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
  result=device->CreateBlendState(&blendDesc,&omBlendState);
  if (result!=S_OK) {
    logE("could not create blend state! %.8x",result);
    return false;
  }

  createRenderTarget();
  return true;
}

void FurnaceGUIRenderDX11::initGUI(SDL_Window* win) {
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

  dead=false;
  return true;
}

void FurnaceGUIRenderDX11::quitGUI() { 
  ImGui_ImplDX11_Shutdown();
}

bool FurnaceGUIRenderDX11::isDead() {
  return dead;
}
