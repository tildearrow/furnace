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

#include "taAudio.h"
#include <SDL.h>

class TAAudioSDL: public TAAudio {
  SDL_AudioSpec ac, ar;
  SDL_AudioDeviceID ai;
  bool audioSysStarted;

  public:
    void onProcess(unsigned char* buf, int nframes);

    void* getContext();
    bool quit();
    bool setRun(bool run);
    std::vector<String> listAudioDevices();
    bool init(TAAudioDesc& request, TAAudioDesc& response);
    TAAudioSDL():
      ai(0),
      audioSysStarted(false) {}
};
