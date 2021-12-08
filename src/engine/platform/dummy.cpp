#include "dummy.h"
#include <stdio.h>
#include <math.h>

void DivPlatformDummy::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t i=start; i<start+len; i++) {
    bufL[i]=0;
    for (unsigned char j=0; j<chans; j++) {
      if (chan[j].active) {
        bufL[i]+=((chan[j].pos>=0x8000)?chan[j].vol:-chan[j].vol)*chan[j].amp;
        chan[j].pos+=chan[j].freq;
      }
    }
  }
}

void DivPlatformDummy::tick() {
  for (unsigned char i=0; i<chans; i++) {
    chan[i].amp--;
    if (chan[i].amp<0) chan[i].amp=0;

    if (chan[i].freqChanged) {
      chan[i].freqChanged=false;
      chan[i].freq=(chan[i].baseFreq*(ONE_SEMITONE+chan[i].pitch))/ONE_SEMITONE;
    }
  }
}

int DivPlatformDummy::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      chan[c.chan].baseFreq=16.4f*pow(2.0f,((float)c.value/12.0f));
      chan[c.chan].freqChanged=true;
      chan[c.chan].active=true;
      chan[c.chan].amp=64;
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      break;
    case DIV_CMD_VOLUME:
      chan[c.chan].vol=c.value;
      if (chan[c.chan].vol>15) chan[c.chan].vol=15;
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=16.4f*pow(2.0f,((float)c.value/12.0f));
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    default:
      break;
  }
  return 1;
}

int DivPlatformDummy::init(DivEngine* p, int channels, int sugRate, bool pal) {
  parent=p;
  rate=65536;
  chans=channels;
  for (int i=0; i<chans; i++) {
    chan[i].vol=0x0f;
  }
  return channels;
}
