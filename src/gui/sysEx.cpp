#include "gui.h"
#include "../ta-log.h"

// table taken from https://nornand.hatenablog.com/entry/2020/11/21/201911
// Yamaha why didn't you just use 0-127 as it should be?
const unsigned char tlTable[100]={
  127, 122, 118, 114, 110, 107, 104, 102, 100, 98, 96, 94, 92, 90, 88, 86, 85, 84, 82, 81,
  // desde aquí la tabla consiste de valores que bajan de 1 en 1
  79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58,
  57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36,
  35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14,
  13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

bool FurnaceGUI::parseSysEx(unsigned char* data, size_t len) {
  SafeReader reader(data,len);

  if (!midiMap.yamahaFMResponse) return true;

  try {
    unsigned char isSysEx=reader.readC();
    if (isSysEx!=0xf0) {
      logW("but this isn't a SysEx! (%x)",isSysEx);
      return false;
    }

    unsigned char vendor=reader.readC();
    if (vendor!=0x43) {
      logV("not Yamaha. skipping.");
      return true;
    }

    unsigned char channel=reader.readC();
    logV("channel: %d",channel);
    if (channel>15) {
      logV("not a valid one");
      return true;
    }

    unsigned char msgType=reader.readC();
    unsigned char lenH=reader.readC();
    unsigned char lenL=reader.readC();
    unsigned short msgLen=(lenH<<7)|lenL;
    logV("message length: %d",msgLen);
    std::vector<DivInstrument*> instruments;

    switch (msgType) {
      case 0x03: { // VCED - voice data
        logD("reading VCED...");
        DivInstrument* ins=new DivInstrument;
        ins->type=DIV_INS_FM;
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=ins->fm.op[i];
          op.ar=reader.readC();
          op.dr=reader.readC();
          op.d2r=reader.readC();
          op.rr=reader.readC();
          op.sl=15-reader.readC();
          reader.readC(); // LS - ignore
          op.rs=reader.readC();
          reader.readC(); // EBS - ignore
          op.am=reader.readC();
          op.kvs=(reader.readC()>2)?1:0;
          op.tl=3+((99-reader.readC())*124)/99;
          unsigned char freq=reader.readC();
          logV("OP%d freq: %d",i,freq);
          op.mult=freq>>2;
          op.dt2=freq&3;
          op.dt=reader.readC();
        }

        ins->fm.alg=reader.readC();
        ins->fm.fb=reader.readC();
        reader.readC(); // LFO speed - ignore
        reader.readC(); // LFO delay - ignore
        reader.readC(); // PMD
        reader.readC(); // AMD
        reader.readC(); // LFO sync
        reader.readC(); // LFO shape
        ins->fm.fms=reader.readC();
        ins->fm.ams=reader.readC();
        reader.readC(); // transpose

        reader.readC(); // poly/mono
        reader.readC(); // pitch bend range
        reader.readC(); // porta mode
        reader.readC(); // porta time
        reader.readC(); // FC volume
        reader.readC(); // sustain
        reader.readC(); // portamento
        reader.readC(); // chorus
        reader.readC(); // mod wheel pitch
        reader.readC(); // mod wheel amp
        reader.readC(); // breath pitch
        reader.readC(); // breath amp
        reader.readC(); // breath pitch bias
        reader.readC(); // breath EG bias

        ins->name=reader.readString(10);

        for (int i=0; i<7; i++) { // reserved (except the last one, which we don't support yet)
          reader.readC();
        }

        // TX81Z-specific data
        if (hasACED) {
          hasACED=false;
          ins->type=DIV_INS_OPZ;

          for (int i=0; i<4; i++) {
            DivInstrumentFM::Operator& op=ins->fm.op[i];
            op.egt=acedData[(5*i)+0];
            if (op.egt) {
              op.dt=acedData[(5*i)+1];
            }
            op.dvb=acedData[(5*i)+2];
            op.ws=acedData[(5*i)+3];
            op.ksl=acedData[(5*i)+4];
            op.dam=acedData[20];
          }
        }

        instruments.push_back(ins);
        break;
      }
      case 0x7e: { // ACED - TX81Z extended data
        logD("reading ACED...");
        String acedMagic=reader.readString(10);
        if (acedMagic!="LM  8976AE") {
          logD("not TX81Z ACED data");
          break;
        }
        reader.read(acedData,23);
        hasACED=true;
        break;
      }
      case 0x04: { // VMEM - voice bank
        logD("reading VMEM...");
        for (int i=0; i<32; i++) {
          DivInstrument* ins=new DivInstrument;
          ins->type=DIV_INS_FM;
          for (int i=0; i<4; i++) {
            DivInstrumentFM::Operator& op=ins->fm.op[i];
            op.ar=reader.readC();
            op.dr=reader.readC();
            op.d2r=reader.readC();
            op.rr=reader.readC();
            op.sl=15-reader.readC();
            reader.readC(); // LS - ignore
            op.am=(reader.readC()&0x40)?1:0;
            op.tl=tlTable[reader.readC()%100];
            unsigned char freq=reader.readC();
            logV("OP%d freq: %d",i,freq);
            op.mult=freq>>2;
            op.dt2=freq&3;
            unsigned char rsDt=reader.readC();
            op.rs=rsDt>>3;
            op.dt=rsDt&7;
          }

          unsigned char algFb=reader.readC();
          ins->fm.alg=algFb&7;
          ins->fm.fb=(algFb>>3)&7;
          reader.readC(); // LFO speed - ignore
          reader.readC(); // LFO delay - ignore
          reader.readC(); // PMD
          reader.readC(); // AMD
          unsigned char fmsAmsShape=reader.readC();
          ins->fm.fms=(fmsAmsShape>>4)&7;
          ins->fm.ams=(fmsAmsShape>>2)&3;
          for (int i=0; i<11; i++) { // skip 11 bytes
            reader.readC();
          }

          ins->name=reader.readString(10);

          for (int i=0; i<6; i++) { // reserved
            reader.readC();
          }

          // TX81Z-specific data
          for (int i=0; i<4; i++) {
            DivInstrumentFM::Operator& op=ins->fm.op[i];
            unsigned char egShiftFixedFixRange=reader.readC();
            unsigned char wsFine=reader.readC();
            op.egt=(egShiftFixedFixRange&8)?1:0;
            if (op.egt) {
              op.dt=egShiftFixedFixRange&7;
            }
            op.dvb=wsFine&15;
            op.ws=wsFine>>4;
            op.ksl=egShiftFixedFixRange>>4;
          }
          unsigned char rev=reader.readC();
          for (int i=0; i<4; i++) {
            ins->fm.op[i].dam=rev;
          }

          // skip more bytes
          for (int i=0; i<46; i++) {
            reader.readC();
          }

          instruments.push_back(ins);
        }
        break;
      }
    }

    if (!reader.seek(6+msgLen,SEEK_SET)) {
      logW("couldn't seek for checksum! (pre)");
      for (DivInstrument* i: instruments) delete i;
      return false;
    }
    unsigned char checkSum=reader.readC();
    unsigned char localCheckSum=0xff;
    if (!reader.seek(6,SEEK_SET)) {
      logW("couldn't seek for checksum! (post)");
      for (DivInstrument* i: instruments) delete i;
      return false;
    }
    for (unsigned short i=0; i<msgLen; i++) {
      localCheckSum+=reader.readC();
    }
    localCheckSum=(localCheckSum&0x7f)^0x7f;
    logD("checksums: %.2x %.2x",checkSum,localCheckSum);

    if (checkSum!=localCheckSum) {
      logW("checksum invalid!");
      return false;
    }

    for (DivInstrument* i: instruments) {
      logI("got instrument from MIDI: %s",i->name);
      insEditSanitizeVZoomAndVScroll(i);
      e->addInstrumentPtr(i);
      curIns=e->song.insLen-1;
    }
  } catch (EndOfFileException e) {
    logW("end of data already?");
    return false;
  }
  return true;
}
