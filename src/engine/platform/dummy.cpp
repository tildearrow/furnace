#include "dummy.h"
#include <stdio.h>
#include <math.h>

void DivPlatformDummy::acquire(int& l, int& r) {
  l=0;
  for (unsigned char i=0; i<chans; i++) {
    if (chan[i].active) {
      l+=((chan[i].pos>=0x8000)?chan[i].vol:-chan[i].vol)<<5;
      chan[i].pos+=chan[i].freq;
    }
  }
  r=l;
}

void DivPlatformDummy::tick() {
}

int DivPlatformDummy::dispatch(DivCommand c) {
  printf("command: %d %d %d %d\n",c.cmd,c.chan,c.value,c.value2);
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      chan[c.chan].freq=16.4f*pow(2.0f,((float)c.value/12.0f));
      chan[c.chan].active=true;
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      break;
    case DIV_CMD_VOLUME:
      chan[c.chan].vol=c.value;
      break;
    default:
      break;
  }
  return 1;
}

int DivPlatformDummy::init(DivEngine* p, int channels, int sugRate) {
  parent=p;
  rate=65536;
  chans=channels;
  for (int i=0; i<chans; i++) {
    chan[i].vol=0x0f;
  }
  return channels;
}
