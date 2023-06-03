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

bool FurnaceGUIRender::lockTexture(void* which, void** data, int* pitch) {
  return false;
}

bool FurnaceGUIRender::unlockTexture(void* which) {
  return false;
}

bool FurnaceGUIRender::updateTexture(void* which, void* data, int pitch) {
  return false;
}

void* FurnaceGUIRender::createTexture(bool dynamic, int width, int height) {
  return NULL;
}

bool FurnaceGUIRender::destroyTexture(void* which) {
  return false;
}

void FurnaceGUIRender::setTextureBlendMode(void* which, FurnaceGUIBlendMode mode) {
}

void FurnaceGUIRender::setBlendMode(FurnaceGUIBlendMode mode) {
}

void FurnaceGUIRender::clear(ImVec4 color) {
}

void FurnaceGUIRender::renderGUI() {
}

void FurnaceGUIRender::wipe(float alpha) {
}

void FurnaceGUIRender::present() {
}

bool FurnaceGUIRender::getOutputSize(int& w, int& h) {
  return false;
}

bool FurnaceGUIRender::init(SDL_Window* win) {
  return false;
}

void FurnaceGUIRender::initGUI(SDL_Window* win) {
}

bool FurnaceGUIRender::quit() {
  return false;
}

void FurnaceGUIRender::quitGUI(){ 
}

FurnaceGUIRender::~FurnaceGUIRender() {
}