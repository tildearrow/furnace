#include "gui.h"
#include "../ta-log.h"

bool FurnaceGUI::parseSysEx(unsigned char* data, size_t len) {
  SafeReader reader(data,len);

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
    unsigned short msgLen=reader.readS_BE();
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
          reader.readC(); // KVS - ignore
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
    }

    if (!reader.seek(6+msgLen,SEEK_SET)) {
      logW("couldn't seek for checksum!");
      for (DivInstrument* i: instruments) delete i;
      return false;
    }
    unsigned char checkSum=reader.readC();
    unsigned char localCheckSum=0xff;
    if (!reader.seek(6,SEEK_SET)) {
      logW("couldn't seek for checksum!");
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
      e->addInstrumentPtr(i);
      curIns=e->song.insLen-1;
    }
  } catch (EndOfFileException e) {
    logW("end of data already?");
    return false;
  }
  return true;
}
