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

class FurnaceGUIRenderSDL: public FurnaceGUIRender {
  SDL_Renderer* sdlRend;
  public:
    ImTextureID getTextureID(FurnaceGUITexture* which);
    bool lockTexture(FurnaceGUITexture* which, void** data, int* pitch);
    bool unlockTexture(FurnaceGUITexture* which);
    bool updateTexture(FurnaceGUITexture* which, void* data, int pitch);
    FurnaceGUITexture* createTexture(bool dynamic, int width, int height);
    bool destroyTexture(FurnaceGUITexture* which);
    void setTextureBlendMode(FurnaceGUITexture* which, FurnaceGUIBlendMode mode);
    void setBlendMode(FurnaceGUIBlendMode mode);
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
    FurnaceGUIRenderSDL():
      sdlRend(NULL) {}
};