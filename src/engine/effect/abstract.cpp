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

#include "../effect.h"
#include "../../ta-log.h"
#include <stdexcept>

void DivEffect::acquire(float** in, float** out, size_t len) {
}

void DivEffect::reset() {
}

int DivEffect::getInputCount() {
  return 0;
}

int DivEffect::getOutputCount() {
  return 0;
}

void DivEffect::rateChanged(double rate) {
}

String DivEffect::getParam(size_t param) {
  throw std::out_of_range("param");

  // unreachable
  return "";
}

bool DivEffect::setParam(size_t param, String value) {
  return false;
}

const char* DivEffect::getParams() {
  return NULL;
}

size_t DivEffect::getParamCount() {
  return 0;
}

String DivEffect::getDynamicText(size_t id) {
  throw std::out_of_range("param");

  // unreachable
  return "";
}

bool DivEffect::load(unsigned short version, const unsigned char* data, size_t len) {
  return false;
}

unsigned char* DivEffect::save(unsigned short* version, size_t* len) {
  *len=0;
  *version=0;
  return NULL;
}

bool DivEffect::init(DivEngine* parent, double rate, unsigned short version, const unsigned char* data, size_t len) {
  return false;
}

void DivEffect::quit() {
}

DivEffect::~DivEffect() {
}