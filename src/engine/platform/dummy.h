/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

#include "../dispatch.h"

// the dummy platform outputs saw waves.
// used when a DivDispatch for a system is not found.
class DivPlatformDummy: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch;
    unsigned short pos;
    bool active, freqChanged;
    unsigned char vol;
    signed char amp;
    Channel(): freq(0), baseFreq(0), pitch(0), pos(0), active(false), freqChanged(false), vol(0), amp(64) {}
  };
  Channel chan[128];
  DivDispatchOscBuffer* oscBuf[128];
  bool isMuted[128];
  unsigned char chans;  
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);
  public:
    void acquire(short** buf, size_t len);
    void muteChannel(int ch, bool mute);
    int dispatch(DivCommand c);
    void notifyInsDeletion(void* ins);
    void* getChanState(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    void reset();
    void tick(bool sysTick=true);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformDummy();
};
