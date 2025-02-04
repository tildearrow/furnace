/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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

class FurnaceGUIRenderGL: public FurnaceGUIRender {
  SDL_GLContext context;
  SDL_Window* sdlWin;
  float quadVertex[4][3];
  unsigned int quadBuf;
  float oscVertex[4][4];
  unsigned int oscVertexBuf;
  unsigned int oscDataTex;
  float oscData[2048];

  // SHADERS //
  // -> wipe
  int sh_wipe_vertex;
  int sh_wipe_fragment;
  int sh_wipe_program;
  int sh_wipe_uAlpha;
  bool sh_wipe_have;
  // -> oscRender
  int sh_oscRender_vertex;
  int sh_oscRender_fragment;
  int sh_oscRender_program;
  int sh_oscRender_uColor;
  int sh_oscRender_uLineWidth;
  int sh_oscRender_uResolution;
  int sh_oscRender_oscVal;
  bool sh_oscRender_have;

  bool swapIntervalSet;
  unsigned char glVer;

  int maxWidth, maxHeight;
  String backendName, vendorName, deviceName, apiVersion;

  bool createShader(const char* vertexS, const char* fragmentS, int& vertex, int& fragment, int& program, const char** attribNames);

  public:
    ImTextureID getTextureID(FurnaceGUITexture* which);
    FurnaceGUITextureFormat getTextureFormat(FurnaceGUITexture* which);
    bool lockTexture(FurnaceGUITexture* which, void** data, int* pitch);
    bool unlockTexture(FurnaceGUITexture* which);
    bool updateTexture(FurnaceGUITexture* which, void* data, int pitch);
    FurnaceGUITexture* createTexture(bool dynamic, int width, int height, bool interpolate=true, FurnaceGUITextureFormat format=GUI_TEXFORMAT_ABGR32);
    bool destroyTexture(FurnaceGUITexture* which);
    void setTextureBlendMode(FurnaceGUITexture* which, FurnaceGUIBlendMode mode);
    void setBlendMode(FurnaceGUIBlendMode mode);
    void clear(ImVec4 color);
    bool newFrame();
    bool canVSync();
    void createFontsTexture();
    void destroyFontsTexture();
    void renderGUI();
    void wipe(float alpha);
    void drawOsc(float* data, size_t len, ImVec2 pos0, ImVec2 pos1, ImVec4 color, ImVec2 canvasSize, float lineWidth);
    void present();
    bool getOutputSize(int& w, int& h);
    bool supportsDrawOsc();
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
    void setVersion(unsigned char ver);
    FurnaceGUIRenderGL():
      context(NULL),
      sdlWin(NULL),
      swapIntervalSet(true),
      glVer(3),
      maxWidth(0),
      maxHeight(0),
      backendName("What?") {
      memset(quadVertex,0,4*3*sizeof(float));
      memset(oscVertex,0,4*5*sizeof(float));
      memset(oscData,0,2048*sizeof(float));
    }
};
