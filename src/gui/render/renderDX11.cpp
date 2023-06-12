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

#include "renderDX11.h"

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

void FurnaceGUIRenderDX11::clear(ImVec4 color) {
}

bool FurnaceGUIRenderDX11::newFrame() {
  return true;
}

void FurnaceGUIRenderDX11::createFontsTexture() {
}

void FurnaceGUIRenderDX11::destroyFontsTexture() {
}

void FurnaceGUIRenderDX11::renderGUI() {
}

void FurnaceGUIRenderDX11::wipe(float alpha) {
}

void FurnaceGUIRenderDX11::present() {
}

bool FurnaceGUIRenderDX11::getOutputSize(int& w, int& h) {
  return false;
}

int FurnaceGUIRenderDX11::getWindowFlags() {
  return 0;
}

void FurnaceGUIRenderDX11::preInit() {
}

bool FurnaceGUIRenderDX11::init(SDL_Window* win) {
  return false;
}

void FurnaceGUIRenderDX11::initGUI(SDL_Window* win) {
}

bool FurnaceGUIRenderDX11::quit() {
  return false;
}

void FurnaceGUIRenderDX11::quitGUI() { 
}

FurnaceGUIRenderDX11::~FurnaceGUIRenderDX11() {
}
