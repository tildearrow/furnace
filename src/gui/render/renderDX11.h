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

#include "../gui.h"
#ifdef INCLUDE_D3D11
#include <d3d11.h>
#else
typedef void ID3D11DeviceContext;
typedef void ID3D11RenderTargetView;
typedef void IDXGISwapChain;
#endif

struct FurnaceDXTexture;

class FurnaceGUIRenderDX11: public FurnaceGUIRender {
  ID3D11Device* device;
  ID3D11DeviceContext* context;
  ID3D11RenderTargetView* renderTarget;
  IDXGISwapChain* swapchain;
  int outW, outH;
  float quadVertex[4][3];

  bool destroyRenderTarget();
  bool createRenderTarget();

  std::vector<FurnaceDXTexture*> textures;

  public:
    ImTextureID getTextureID(void* which);
    bool lockTexture(void* which, void** data, int* pitch);
    bool unlockTexture(void* which);
    bool updateTexture(void* which, void* data, int pitch);
    void* createTexture(bool dynamic, int width, int height);
    bool destroyTexture(void* which);
    void setTextureBlendMode(void* which, FurnaceGUIBlendMode mode);
    void setBlendMode(FurnaceGUIBlendMode mode);
    void resized(const SDL_Event& ev);
    void clear(ImVec4 color);
    bool newFrame();
    void createFontsTexture();
    void destroyFontsTexture();
    void renderGUI();
    void wipe(float alpha);
    void present();
    bool getOutputSize(int& w, int& h);
    int getWindowFlags();
    void preInit();
    bool init(SDL_Window* win);
    void initGUI(SDL_Window* win);
    void quitGUI();
    bool quit();
    FurnaceGUIRenderDX11():
      device(NULL),
      context(NULL),
      renderTarget(NULL),
      swapchain(NULL),
      outW(0),
      outH(0) {
      memset(quadVertex,0,4*3*sizeof(float));
    }
};
