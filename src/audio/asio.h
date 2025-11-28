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
#include <asiodrivers.h>
#include <asio.h>

#define ASIO_DRIVER_MAX 64
#define ASIO_CHANNEL_MAX 16

class TAAudioASIO: public TAAudio {
  ASIODriverInfo driverInfo;
  ASIOBufferInfo bufInfo[ASIO_CHANNEL_MAX*2];
  ASIOChannelInfo chanInfo[ASIO_CHANNEL_MAX*2];
  ASIOCallbacks callbacks;
  int totalChans;

  char* driverNames[ASIO_DRIVER_MAX];
  int driverCount;
  bool driverNamesInit;

  char deviceNameCopy[64];

  public:
    void onSampleRate(double rate);
    void onBufferSize(int bufsize);
    void onProcess(int nframes);
    void requestDeviceChange();

    String getErrorStr(ASIOError which);
    String getFormatName(ASIOSampleType which);

    void* getContext();
    int specialCommand(TAAudioCommand which);
    bool quit();
    bool setRun(bool run);
    bool init(TAAudioDesc& request, TAAudioDesc& response);

    std::vector<String> listAudioDevices();

    TAAudioASIO():
      totalChans(0),
      driverCount(0),
      driverNamesInit(false) {}
};
