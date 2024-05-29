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

#include "../gui.h"

ImTextureID FurnaceGUIRender::getTextureID(FurnaceGUITexture* which) {
  return NULL;
}

float FurnaceGUIRender::getTextureU(FurnaceGUITexture* which) {
  return 1.0;
}

float FurnaceGUIRender::getTextureV(FurnaceGUITexture* which) {
  return 1.0;
}

FurnaceGUITextureFormat FurnaceGUIRender::getTextureFormat(FurnaceGUITexture* which) {
  return GUI_TEXFORMAT_UNKNOWN;
}

bool FurnaceGUIRender::isTextureValid(FurnaceGUITexture* which) {
  return (which!=NULL);
}

bool FurnaceGUIRender::lockTexture(FurnaceGUITexture* which, void** data, int* pitch) {
  return false;
}

bool FurnaceGUIRender::unlockTexture(FurnaceGUITexture* which) {
  return false;
}

bool FurnaceGUIRender::updateTexture(FurnaceGUITexture* which, void* data, int pitch) {
  return false;
}

FurnaceGUITexture* FurnaceGUIRender::createTexture(bool dynamic, int width, int height, bool interpolate, FurnaceGUITextureFormat format) {
  return NULL;
}

bool FurnaceGUIRender::destroyTexture(FurnaceGUITexture* which) {
  return false;
}

void FurnaceGUIRender::setTextureBlendMode(FurnaceGUITexture* which, FurnaceGUIBlendMode mode) {
}

void FurnaceGUIRender::setBlendMode(FurnaceGUIBlendMode mode) {
}

void FurnaceGUIRender::resized(const SDL_Event& ev) {
}

void FurnaceGUIRender::clear(ImVec4 color) {
}

bool FurnaceGUIRender::newFrame() {
  return true;
}

bool FurnaceGUIRender::canVSync() {
  return true;
}

void FurnaceGUIRender::createFontsTexture() {
}

void FurnaceGUIRender::destroyFontsTexture() {
}

void FurnaceGUIRender::renderGUI() {
}

void FurnaceGUIRender::wipe(float alpha) {
}

void FurnaceGUIRender::drawOsc(float* data, size_t len, ImVec2 pos0, ImVec2 pos1, ImVec4 color, ImVec2 canvasSize, float lineWidth) {
}

void FurnaceGUIRender::present() {
}

bool FurnaceGUIRender::getOutputSize(int& w, int& h) {
  return false;
}

bool FurnaceGUIRender::supportsDrawOsc() {
  return false;
}

int FurnaceGUIRender::getWindowFlags() {
  return 0;
}

int FurnaceGUIRender::getMaxTextureWidth() {
  return 0;
}

int FurnaceGUIRender::getMaxTextureHeight() {
  return 0;
}

unsigned int FurnaceGUIRender::getTextureFormats() {
  return 0;
}

const char* FurnaceGUIRender::getBackendName() {
  return "Dummy";
}

const char* FurnaceGUIRender::getVendorName() {
  return "N/A";
}

const char* FurnaceGUIRender::getDeviceName() {
  return "N/A";
}

const char* FurnaceGUIRender::getAPIVersion() {
  return "N/A";
}

void FurnaceGUIRender::setSwapInterval(int swapInterval) {
}

void FurnaceGUIRender::preInit(const DivConfig& conf) {
}

bool FurnaceGUIRender::init(SDL_Window* win, int swapInterval) {
  return false;
}

void FurnaceGUIRender::initGUI(SDL_Window* win) {
}

bool FurnaceGUIRender::quit() {
  return false;
}

void FurnaceGUIRender::quitGUI() { 
}

bool FurnaceGUIRender::isDead() {
  return false;
}

FurnaceGUIRender::~FurnaceGUIRender() {
}
