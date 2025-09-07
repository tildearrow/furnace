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

#include "taAudio.h"
#include "../ta-log.h"

void TAAudio::setSampleRateChangeCallback(void (*callback)(SampleRateChangeEvent)) {
  sampleRateChanged=callback;
}

void TAAudio::setBufferSizeChangeCallback(void (*callback)(BufferSizeChangeEvent)) {
  bufferSizeChanged=callback;
}

void TAAudio::setCallback(void (*callback)(void*,float**,float**,int,int,unsigned int), void* user) {
  audioProcCallback=callback;
  audioProcCallbackUser=user;
}

void* TAAudio::getContext() {
  return NULL;
}

bool TAAudio::quit() {
  return true;
}

bool TAAudio::setRun(bool run) {
  running=run;
  return running;
}

std::vector<String> TAAudio::listAudioDevices() {
  return std::vector<String>();
}

bool TAAudio::init(TAAudioDesc& request, TAAudioDesc& response) {
  response=request;
  return true;
}

TAAudio::~TAAudio() {
}

bool TAMidiIn::gather() {
  return false;
}

bool TAMidiOut::send(const TAMidiMessage& what) {
  logE("virtual method TAMidiOut::send() called! this is a bug!");
  return false;
}

bool TAMidiIn::isDeviceOpen() {
  return false;
}

bool TAMidiOut::isDeviceOpen() {
  return false;
}

bool TAMidiIn::openDevice(String name) {
  return false;
}

bool TAMidiOut::openDevice(String name) {
  return false;
}

bool TAMidiIn::closeDevice() {
  return false;
}

bool TAMidiOut::closeDevice() {
  return false;
}

std::vector<String> TAMidiIn::listDevices() {
  logW("attempting to list devices of abstract TAMidiIn!");
  return std::vector<String>();
}

std::vector<String> TAMidiOut::listDevices() {
  logW("attempting to list devices of abstract TAMidiOut!");
  return std::vector<String>();
}

bool TAMidiIn::init() {
  return false;
}

bool TAMidiOut::init() {
  return false;
}

bool TAMidiIn::quit() {
  return true;
}

bool TAMidiOut::quit() {
  return true;
}

TAMidiIn::~TAMidiIn() {
}

TAMidiOut::~TAMidiOut() {
}