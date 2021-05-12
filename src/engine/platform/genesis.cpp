#include "genesis.h"
#include "../engine.h"
#include <string.h>

// TODO fix all the writes.
// i think there is no wait for data writes, just for ON/OFF writes
void DivPlatformGenesis::acquire(short& l, short& r) {
  short o[2];
  if (!writes.empty() && --delay<0) {
    QueuedWrite w=writes.front();
    //printf("write: %x = %.2x\n",w.addr,w.val);
    OPN2_Write(&fm,0x0+((w.addr>>8)<<1),w.addr);
    OPN2_Clock(&fm,o);
    OPN2_Write(&fm,0x1+((w.addr>>8)<<1),w.val);
    writes.pop();
    delay=24;
  }
  OPN2_Clock(&fm,o);
  //OPN2_Write(&fm,0,0);
  l=o[0]<<7;
  r=o[1]<<7;
}

void DivPlatformGenesis::tick() {
}

static unsigned short chanOffs[6]={
  0x00, 0x01, 0x02, 0x100, 0x101, 0x102
};
static unsigned short opOffs[4]={
  0x00, 0x04, 0x08, 0x0c
};
static unsigned char konOffs[6]={
  0, 1, 2, 4, 5, 6
};
static unsigned short notes[12]={
  644,681,722,765,810,858,910,964,1021,1081,1146,1214
};

int DivPlatformGenesis::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      if (c.chan>5) break;
      //chan[c.chan].freq=16.4f*pow(2.0f,((float)c.value/12.0f));
      DivInstrument* ins=parent->song.ins[chan[c.chan].ins];
      writes.emplace(0x28,0x00|konOffs[c.chan]);
      for (int i=0; i<4; i++) {
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
        DivInstrumentFM::Operator op=ins->fm.op[i];
        writes.emplace(baseAddr+0x30,op.mult&15|(op.dt<<4));
        writes.emplace(baseAddr+0x40,op.tl);
        writes.emplace(baseAddr+0x50,op.ar&31|(op.rs<<6));
        writes.emplace(baseAddr+0x60,op.dr&31|(op.am<<7));
        writes.emplace(baseAddr+0x70,op.d2r&31);
        writes.emplace(baseAddr+0x80,op.rr&15|(op.sl<<4));
      }
      writes.emplace(chanOffs[c.chan]+0xa4,((c.value/12)<<3)|(notes[c.value%12]>>8));
      writes.emplace(chanOffs[c.chan]+0xa0,notes[c.value%12]);
      writes.emplace(chanOffs[c.chan]+0xb0,ins->fm.alg&7|(ins->fm.fb<<3));
      writes.emplace(chanOffs[c.chan]+0xb4,0xc0|ins->fm.fms&7|((ins->fm.ams&3)<<4));
      writes.emplace(0x28,0xf0|konOffs[c.chan]);
      chan[c.chan].active=true;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      writes.emplace(0x28,0x00|konOffs[c.chan]);
      chan[c.chan].active=false;
      break;
    case DIV_CMD_VOLUME:
      chan[c.chan].vol=c.value;
      break;
    case DIV_CMD_INSTRUMENT:
      chan[c.chan].ins=c.value;
      break;
    default:
      break;
  }
  return 1;
}

int DivPlatformGenesis::init(DivEngine* p, int channels, int sugRate) {
  parent=p;
  rate=1278409;
  OPN2_Reset(&fm);
  
  delay=100;
  return 10;
}
