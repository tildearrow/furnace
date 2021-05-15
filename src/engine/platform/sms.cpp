#include "sms.h"
#include <math.h>

void DivPlatformSMS::acquire(short& l, short& r) {
  short v;
  sn->sound_stream_update(&v,1);
  l=v;
  r=v;
}

void DivPlatformSMS::tick() {
  for (int i=0; i<3; i++) {
    if (chan[i].freqChanged) {
      sn->write(0x80|i<<5|(chan[i].freq&15));
      sn->write(chan[i].freq>>4);
      chan[i].freqChanged=false;
    }
  }
  if (chan[3].freqChanged || updateSNMode) {
    updateSNMode=false;
    chan[3].freqChanged=false;
    if (snNoiseMode&2) { // take period from channel 3
      if (snNoiseMode&1) {
        sn->write(0xe7);
      } else {
        sn->write(0xe3);
      }
      sn->write(0xdf);
      sn->write(0xc0|(chan[3].freq&15));
      sn->write(chan[3].freq>>4);
    } else { // 3 fixed values
      unsigned char value=chan[3].note%12;
      if (value>2) value=2;
      value=2-value;
      sn->write(0xe0|value|((snNoiseMode&1)<<2));
    }
  }
}

int DivPlatformSMS::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      chan[c.chan].freq=3430/pow(2.0f,((float)c.value/12.0f));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      chan[c.chan].active=true;
      sn->write(0x90|c.chan<<5|(15-chan[c.chan].vol));
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      break;
    case DIV_CMD_VOLUME:
      chan[c.chan].vol=c.value;
      sn->write(0x90|c.chan<<5|(15-chan[c.chan].vol));
      break;
    case DIV_CMD_STD_NOISE_MODE:
      snNoiseMode=(c.value&1)|((c.value&16)>>3);
      updateSNMode=true;
      break;
    default:
      break;
  }
  return 1;
}

int DivPlatformSMS::init(DivEngine* p, int channels, int sugRate) {
  parent=p;
  rate=223722;
  sn=new sn76496_device("sn",223722);
  sn->device_start();
  snNoiseMode=0;
  updateSNMode=false;
  return 4;
}
