#include "sms.h"
#include "../engine.h"
#include <math.h>

void DivPlatformSMS::acquire(int& l, int& r) {
  short v;
  sn->sound_stream_update(&v,1);
  l=v;
  r=v;
}

void DivPlatformSMS::tick() {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.hadVol) {
      chan[i].outVol=(chan[i].vol*chan[i].std.vol)>>4;
      sn->write(0x90|(i<<5)|(15-(chan[i].outVol&15)));
    }
    if (chan[i].std.hadArp) {
      if (chan[i].std.arpMode) {
        chan[i].baseFreq=round(1712.0f/pow(2.0f,((float)(chan[i].std.arp)/12.0f)));
      } else {
        chan[i].baseFreq=round(1712.0f/pow(2.0f,((float)(chan[i].note+chan[i].std.arp-12)/12.0f)));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.hadDuty) {
      snNoiseMode=(snNoiseMode&2)|(chan[i].std.duty&1);
      if (chan[i].std.duty<2) {
        chan[3].freqChanged=false;
      }
      updateSNMode=true;
    }
  }
  for (int i=0; i<3; i++) {
    if (chan[i].freqChanged) {
      chan[i].freq=(chan[i].baseFreq*(ONE_SEMITONE-chan[i].pitch))/ONE_SEMITONE;
      if (chan[i].note>0x5d) chan[i].freq=0x01;
      sn->write(0x80|i<<5|(chan[i].freq&15));
      sn->write(chan[i].freq>>4);
      chan[i].freqChanged=false;
    }
  }
  if (chan[3].freqChanged || updateSNMode) {
    updateSNMode=false;
    chan[3].freq=(chan[3].baseFreq*(ONE_SEMITONE-chan[3].pitch))/ONE_SEMITONE;
    if (chan[3].note>0x5d) chan[3].freq=0x01;
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
      unsigned char value;
      if (chan[3].std.hadArp) {
        if (chan[3].std.arpMode) {
          value=chan[3].std.arp%12;
        } else {
          value=(chan[3].note+chan[3].std.arp)%12;
        }
      } else {
        value=chan[3].note%12;
      }
      if (value<3) {
        value=2-value;
        sn->write(0xe0|value|((snNoiseMode&1)<<2));
      }
    }
  }
}

int DivPlatformSMS::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      chan[c.chan].baseFreq=round(1712.0f/pow(2.0f,((float)c.value/12.0f)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      chan[c.chan].active=true;
      sn->write(0x90|c.chan<<5|(15-(chan[c.chan].vol&15)));
      chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      sn->write(0x9f|c.chan<<5);
      chan[c.chan].std.init(NULL);
      break;
    case DIV_CMD_INSTRUMENT:
      chan[c.chan].ins=c.value;
      //chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.hasVol) {
          chan[c.chan].outVol=c.value;
        }
        sn->write(0x90|c.chan<<5|(15-(chan[c.chan].vol&15)));
      }
      break;
    case DIV_CMD_GET_VOLUME:
      if (chan[c.chan].std.hasVol) {
        return chan[c.chan].vol;
      }
      return chan[c.chan].outVol;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=round(1712.0f/pow(2.0f,((float)c.value2/12.0f)));
      bool return2=false;
      if (destFreq>chan[c.chan].baseFreq) {
        chan[c.chan].baseFreq+=c.value;
        if (chan[c.chan].baseFreq>=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      } else {
        chan[c.chan].baseFreq-=c.value;
        if (chan[c.chan].baseFreq<=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      }
      chan[c.chan].freqChanged=true;
      if (return2) return 2;
      break;
    }
    case DIV_CMD_STD_NOISE_MODE:
      snNoiseMode=(c.value&1)|((c.value&16)>>3);
      updateSNMode=true;
      break;
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=round(1712.0f/pow(2.0f,((float)c.value/12.0f)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
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
  snNoiseMode=3;
  updateSNMode=false;
  return 4;
}
