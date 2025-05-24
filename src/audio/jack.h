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
#include "../../extern/weakjack/weak_libjack.h"

class TAAudioJACK: public TAAudio {
  jack_client_t* ac;
  jack_port_t** ai;
  jack_port_t** ao;

  float** iInBufs;
  float** iOutBufs;

  String printStatus(jack_status_t status);

  public:
    void onSampleRate(jack_nframes_t rate);
    void onBufferSize(jack_nframes_t bufsize);
    void onProcess(jack_nframes_t nframes);

    void* getContext();
    bool quit();
    bool setRun(bool run);
    bool init(TAAudioDesc& request, TAAudioDesc& response);

    TAAudioJACK():
      ac(NULL),
      ai(NULL),
      ao(NULL),
      iInBufs(NULL),
      iOutBufs(NULL) {}
};
