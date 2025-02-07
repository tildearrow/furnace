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

#ifndef _TAAUDIO_H
#define _TAAUDIO_H
#include "../ta-utils.h"
#include <memory>
#include "../fixedQueue.h"
#include "../pch.h"

struct SampleRateChangeEvent {
  double rate;
  explicit SampleRateChangeEvent(double r):
    rate(r) {}
};

struct BufferSizeChangeEvent {
  unsigned int bufsize;
  explicit BufferSizeChangeEvent(unsigned int bs):
    bufsize(bs) {}
};

enum TAAudioFormat {
  TA_AUDIO_FORMAT_F32=0,
  TA_AUDIO_FORMAT_F64,
  TA_AUDIO_FORMAT_U8,
  TA_AUDIO_FORMAT_S8,
  TA_AUDIO_FORMAT_U16,
  TA_AUDIO_FORMAT_S16,
  TA_AUDIO_FORMAT_U32,
  TA_AUDIO_FORMAT_S32,
  TA_AUDIO_FORMAT_U16BE,
  TA_AUDIO_FORMAT_S16BE,
  TA_AUDIO_FORMAT_U32BE,
  TA_AUDIO_FORMAT_S32BE
};

struct TAAudioDesc {
  String name, deviceName;
  double rate;
  unsigned int bufsize, fragments;
  unsigned char inChans, outChans;
  TAAudioFormat outFormat;

  bool wasapiEx;

  TAAudioDesc():
    rate(0.0),
    bufsize(0),
    fragments(0),
    inChans(0),
    outChans(0),
    outFormat(TA_AUDIO_FORMAT_F32),
    wasapiEx(false) {}
};


enum TAMidiMessageTypes {
  TA_MIDI_NOTE_OFF=0x80,
  TA_MIDI_NOTE_ON=0x90,
  TA_MIDI_AFTERTOUCH=0xa0,
  TA_MIDI_CONTROL=0xb0,
  TA_MIDI_PROGRAM=0xc0,
  TA_MIDI_CHANNEL_AFTERTOUCH=0xd0,
  TA_MIDI_PITCH_BEND=0xe0,
  TA_MIDI_SYSEX=0xf0,
  TA_MIDI_MTC_FRAME=0xf1,
  TA_MIDI_POSITION=0xf2,
  TA_MIDI_SONG_SELECT=0xf3,
  TA_MIDI_TUNE_REQUEST=0xf6,
  TA_MIDI_SYSEX_END=0xf7,
  TA_MIDI_CLOCK=0xf8,
  TA_MIDI_MACHINE_PLAY=0xfa,
  TA_MIDI_MACHINE_RESUME=0xfb,
  TA_MIDI_MACHINE_STOP=0xfc,
  TA_MIDI_KEEPALIVE=0xfe,
  TA_MIDI_RESET=0xff
};

struct TAMidiMessage {
  double time;
  unsigned char type;
  unsigned char data[7];
  std::shared_ptr<unsigned char> sysExData;
  size_t sysExLen;

  void submitSysEx(std::vector<unsigned char> data);
  void done();

  TAMidiMessage(unsigned char t, unsigned char d0, unsigned char d1):
    time(0.0),
    type(t),
    sysExData(NULL),
    sysExLen(0) {
    memset(&data,0,sizeof(data));
    data[0]=d0;
    data[1]=d1;
  }

  TAMidiMessage():
    time(0.0),
    type(0),
    sysExData(NULL),
    sysExLen(0) {
    memset(&data,0,sizeof(data));
  }
};

class TAMidiIn {
  public:
    FixedQueue<TAMidiMessage,8192> queue;
    virtual bool gather();
    bool next(TAMidiMessage& where);
    virtual bool isDeviceOpen();
    virtual bool openDevice(String name);
    virtual bool closeDevice();
    virtual std::vector<String> listDevices();
    virtual bool init();
    virtual bool quit();
    TAMidiIn() {
    }
    virtual ~TAMidiIn();
};

class TAMidiOut {
  FixedQueue<TAMidiMessage,8192> queue;
  public:
    virtual bool send(const TAMidiMessage& what);
    virtual bool isDeviceOpen();
    virtual bool openDevice(String name);
    virtual bool closeDevice();
    virtual std::vector<String> listDevices();
    virtual bool init();
    virtual bool quit();
    TAMidiOut() {
    }
    virtual ~TAMidiOut();
};

class TAAudio {
  protected:
    TAAudioDesc desc;
    TAAudioFormat outFormat;
    bool running, initialized;
    float** inBufs;
    float** outBufs;
    void (*audioProcCallback)(void*,float**,float**,int,int,unsigned int);
    void* audioProcCallbackUser;
    void (*sampleRateChanged)(SampleRateChangeEvent);
    void (*bufferSizeChanged)(BufferSizeChangeEvent);
  public:
    TAMidiIn* midiIn;
    TAMidiOut* midiOut;
    void setSampleRateChangeCallback(void (*callback)(SampleRateChangeEvent));
    void setBufferSizeChangeCallback(void (*callback)(BufferSizeChangeEvent));

    void setCallback(void (*callback)(void*,float**,float**,int,int,unsigned int), void* user);

    virtual void* getContext();
    virtual bool quit();
    virtual bool setRun(bool run);
    virtual std::vector<String> listAudioDevices();
    bool initMidi(bool jack);
    void quitMidi();
    virtual bool init(TAAudioDesc& request, TAAudioDesc& response);

    TAAudio():
      outFormat(TA_AUDIO_FORMAT_F32),
      running(false),
      initialized(false),
      inBufs(NULL),
      outBufs(NULL),
      audioProcCallback(NULL),
      audioProcCallbackUser(NULL),
      sampleRateChanged(NULL),
      bufferSizeChanged(NULL),
      midiIn(NULL),
      midiOut(NULL) {}

    virtual ~TAAudio();
};
#endif
