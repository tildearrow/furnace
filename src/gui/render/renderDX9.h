/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

#include "../gui.h"
#ifdef INCLUDE_D3D9
#include <d3d9.h>
struct FurnaceGUIRenderDX9Private;
#else
typedef void IDirect3D9;
typedef void IDirect3DVertexBuffer9;
typedef void FurnaceGUIRenderDX9Private;
#endif

class FurnaceGUIRenderDX9: public FurnaceGUIRender {
  IDirect3D9* iface;
  IDirect3DDevice9* device;
  FurnaceGUIRenderDX9Private* priv;
  IDirect3DVertexBuffer9* wipeBuf;

  int outW, outH, swapInterval;

  bool dead, haveScene, supportsDynamicTex, supportsVSync, mustResize, squareTex, inScene;

  // SHADERS //

  int maxWidth, maxHeight;
  String vendorName, deviceName, apiVersion;

  public:
    ImTextureID getTextureID(FurnaceGUITexture* which);
    float getTextureU(FurnaceGUITexture* which);
    float getTextureV(FurnaceGUITexture* which);
    FurnaceGUITextureFormat getTextureFormat(FurnaceGUITexture* which);
    bool isTextureValid(FurnaceGUITexture* which);
    bool lockTexture(FurnaceGUITexture* which, void** data, int* pitch);
    bool unlockTexture(FurnaceGUITexture* which);
    bool updateTexture(FurnaceGUITexture* which, void* data, int pitch);
    FurnaceGUITexture* createTexture(bool dynamic, int width, int height, bool interpolate=true, FurnaceGUITextureFormat format=GUI_TEXFORMAT_ABGR32);
    bool destroyTexture(FurnaceGUITexture* which);
    void setTextureBlendMode(FurnaceGUITexture* which, FurnaceGUIBlendMode mode);
    void setBlendMode(FurnaceGUIBlendMode mode);
    void resized(const SDL_Event& ev);
    void clear(ImVec4 color);
    void newFrame();
    bool canVSync();
    bool areTexturesSquare();
    void renderGUI();
    void wipe(float alpha);
    void present();
    bool getOutputSize(int& w, int& h);
    int getWindowFlags();
    int getMaxTextureWidth();
    int getMaxTextureHeight();
    unsigned int getTextureFormats();
    const char* getBackendName();
    const char* getVendorName();
    const char* getDeviceName();
    const char* getAPIVersion();
    void setSwapInterval(int swapInterval);
    void preInit(const DivConfig& conf);
    bool init(SDL_Window* win, int swapInterval);
    void initGUI(SDL_Window* win);
    void quitGUI();
    bool quit();
    bool isDead();
    FurnaceGUIRenderDX9():
      iface(NULL),
      device(NULL),
      priv(NULL),
      wipeBuf(NULL),
      outW(0),
      outH(0),
      swapInterval(1),
      dead(false),
      haveScene(false),
      supportsDynamicTex(false),
      supportsVSync(false),
      mustResize(false),
      squareTex(false),
      inScene(false),
      maxWidth(8192),
      maxHeight(8192) {
    }
};
