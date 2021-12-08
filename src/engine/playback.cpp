#include "dispatch.h"
#include "engine.h"
#include "../ta-log.h"
#include <sndfile.h>

void DivEngine::nextOrder() {
  curRow=0;
  if (++curOrder>=song.ordersLen) {
    endOfSong=true;
    curOrder=0;
  }
}

const char* notes[12]={
  "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
};

const char* cmdName[DIV_CMD_MAX]={
  "NOTE_ON",
  "NOTE_OFF",
  "INSTRUMENT",
  "VOLUME",
  "GET_VOLUME",
  "GET_VOLMAX",
  "NOTE_PORTA",
  "PITCH",
  "PANNING",
  "LEGATO",
  "PRE_PORTA",
  "PRE_NOTE",

  "SAMPLE_MODE",
  "SAMPLE_FREQ",

  "FM_LFO",
  "FM_TL",
  "FM_AR",
  "FM_FB",
  "FM_MULT",
  "FM_EXTCH",

  "GENESIS_LFO",
  
  "ARCADE_LFO",

  "STD_NOISE_FREQ",
  "STD_NOISE_MODE",

  "WAVE",
  
  "GB_SWEEP_TIME",
  "GB_SWEEP_DIR",

  "PCE_LFO_MODE",
  "PCE_LFO_SPEED",

  "C64_CUTOFF",
  "C64_RESONANCE",
  "C64_FILTER_MODE",
  "C64_RESET_TIME",
  "C64_RESET_MASK",
  "C64_FILTER_RESET",
  "C64_DUTY_RESET",
  "C64_EXTENDED",

  "ALWAYS_SET_VOLUME"
};

const char* formatNote(unsigned char note, unsigned char octave) {
  static char ret[4];
  if (note==100) {
    return "OFF";
  } else if (octave==0 && note==0) {
    return "---";
  }
  snprintf(ret,4,"%s%d",notes[note%12],octave+note/12);
  return ret;
}

int DivEngine::dispatchCmd(DivCommand c) {
  if (view==DIV_STATUS_COMMANDS) {
    printf("%8d | %d: %s(%d, %d)\n",totalTicks,c.chan,cmdName[c.cmd],c.value,c.value2);
  }
  totalCmds++;
  return dispatch->dispatch(c);
}

bool DivEngine::perSystemEffect(int ch, unsigned char effect, unsigned char effectVal) {
  switch (song.system) {
    case DIV_SYSTEM_GENESIS:
    case DIV_SYSTEM_GENESIS_EXT:
      switch (effect) {
        case 0x17: // DAC enable
          dispatchCmd(DivCommand(DIV_CMD_SAMPLE_MODE,ch,(effectVal>0)));
          break;
        case 0x20: // SN noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_SMS:
      switch (effect) {
        case 0x20: // SN noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_GB:
      switch (effect) {
        case 0x10: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        case 0x11: case 0x12: // duty or noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        case 0x13: // sweep params
          dispatchCmd(DivCommand(DIV_CMD_GB_SWEEP_TIME,ch,effectVal));
          break;
        case 0x14: // sweep direction
          dispatchCmd(DivCommand(DIV_CMD_GB_SWEEP_DIR,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_PCE:
      switch (effect) {
        case 0x10: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        case 0x11: // noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        case 0x12: // LFO mode
          dispatchCmd(DivCommand(DIV_CMD_PCE_LFO_MODE,ch,effectVal));
          break;
        case 0x13: // LFO speed
          dispatchCmd(DivCommand(DIV_CMD_PCE_LFO_SPEED,ch,effectVal));
          break;
        case 0x17: // PCM enable
          dispatchCmd(DivCommand(DIV_CMD_SAMPLE_MODE,ch,(effectVal>0)));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_NES:
      switch (effect) {
        case 0x12: // duty or noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    default:
      return false;
  }
  return true;
}

bool DivEngine::perSystemPostEffect(int ch, unsigned char effect, unsigned char effectVal) {
  switch (song.system) {
    case DIV_SYSTEM_GENESIS:
    case DIV_SYSTEM_GENESIS_EXT:
      switch (effect) {
        case 0x10: // LFO
          dispatchCmd(DivCommand(DIV_CMD_FM_LFO,ch,effectVal));
          break;
        case 0x11: // FB
          dispatchCmd(DivCommand(DIV_CMD_FM_FB,ch,effectVal&7));
          break;
        case 0x12: // TL op1
          dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,0,effectVal&0x7f));
          break;
        case 0x13: // TL op2
          dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,1,effectVal&0x7f));
          break;
        case 0x14: // TL op3
          dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,2,effectVal&0x7f));
          break;
        case 0x15: // TL op4
          dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,3,effectVal&0x7f));
          break;
        case 0x16: // MULT
          if ((effectVal>>4)>0 && (effectVal>>4)<5) {
            dispatchCmd(DivCommand(DIV_CMD_FM_MULT,ch,(effectVal>>4)-1,effectVal&15));
          }
          break;
        case 0x18: // EXT
          dispatchCmd(DivCommand(DIV_CMD_FM_EXTCH,ch,effectVal));
          break;
        case 0x19: // AR global
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,-1,effectVal&31));
          break;
        case 0x1a: // AR op1
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,0,effectVal&31));
          break;
        case 0x1b: // AR op2
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,1,effectVal&31));
          break;
        case 0x1c: // AR op3
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,2,effectVal&31));
          break;
        case 0x1d: // AR op4
          dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,3,effectVal&31));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_C64_6581:
    case DIV_SYSTEM_C64_8580:
      switch (effect) {
        case 0x10: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        case 0x11: // cutoff
          dispatchCmd(DivCommand(DIV_CMD_C64_CUTOFF,ch,effectVal));
          break;
        case 0x12: // duty
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        case 0x13: // resonance
          dispatchCmd(DivCommand(DIV_CMD_C64_RESONANCE,ch,effectVal));
          break;
        case 0x14: // filter mode
          dispatchCmd(DivCommand(DIV_CMD_C64_FILTER_MODE,ch,effectVal));
          break;
        case 0x15: // reset time
          dispatchCmd(DivCommand(DIV_CMD_C64_RESET_TIME,ch,effectVal));
          break;
        case 0x1a: // reset mask
          dispatchCmd(DivCommand(DIV_CMD_C64_RESET_MASK,ch,effectVal));
          break;
        case 0x1b: // cutoff reset
          dispatchCmd(DivCommand(DIV_CMD_C64_FILTER_RESET,ch,effectVal));
          break;
        case 0x1c: // duty reset
          dispatchCmd(DivCommand(DIV_CMD_C64_DUTY_RESET,ch,effectVal));
          break;
        case 0x1e: // extended
          dispatchCmd(DivCommand(DIV_CMD_C64_EXTENDED,ch,effectVal));
          break;
        default:
          return false;
      }
    default:
      return false;
  }
  return true;
}

void DivEngine::processRow(int i, bool afterDelay) {
  int whatOrder=afterDelay?chan[i].delayOrder:curOrder;
  int whatRow=afterDelay?chan[i].delayRow:curRow;
  DivPattern* pat=song.pat[i]->data[whatOrder];
  // pre effects
  if (!afterDelay) for (int j=0; j<song.pat[i]->effectRows; j++) {
    short effect=pat->data[whatRow][4+(j<<1)];
    short effectVal=pat->data[whatRow][5+(j<<1)];

    if (effectVal==-1) effectVal=0;
    if (effect==0xed && effectVal!=0) {
      if (effectVal<=nextSpeed) {
        chan[i].rowDelay=effectVal+1;
        chan[i].delayOrder=whatOrder;
        chan[i].delayRow=whatRow;
        if (effectVal==nextSpeed) {
          chan[i].delayLocked=true;
        } else {
          chan[i].delayLocked=false;
        }
        return;
      } else {
        chan[i].delayLocked=false;
      }
    }
  }

  if (chan[i].delayLocked) return;

  // instrument
  if (pat->data[whatRow][2]!=-1) {
    dispatchCmd(DivCommand(DIV_CMD_INSTRUMENT,i,pat->data[whatRow][2]));
  }
  // note
  if (pat->data[whatRow][0]==100) {
    //chan[i].note=-1;
    chan[i].keyOn=false;
    if (chan[i].stopOnOff) {
      chan[i].portaNote=-1;
      chan[i].portaSpeed=-1;
      chan[i].stopOnOff=false;
    }
    dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF,i));
  } else if (!(pat->data[whatRow][0]==0 && pat->data[whatRow][1]==0)) {
    chan[i].note=pat->data[whatRow][0]+pat->data[whatRow][1]*12;
    if (!chan[i].keyOn) {
      if (dispatch->keyOffAffectsArp(i)) {
        chan[i].arp=0;
      }
    }
    chan[i].doNote=true;
    if (chan[i].arp!=0) {
      chan[i].arpYield=true;
    }
  }

  // volume
  if (pat->data[whatRow][3]!=-1) {
    if (dispatchCmd(DivCommand(DIV_ALWAYS_SET_VOLUME,i)) || (MIN(chan[i].volMax,chan[i].volume)>>8)!=pat->data[whatRow][3]) {
      chan[i].volume=pat->data[whatRow][3]<<8;
      dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
    }
  }

  // effects
  for (int j=0; j<song.pat[i]->effectRows; j++) {
    short effect=pat->data[whatRow][4+(j<<1)];
    short effectVal=pat->data[whatRow][5+(j<<1)];

    if (effectVal==-1) effectVal=0;

    // per-system effect
    if (!perSystemEffect(i,effect,effectVal)) switch (effect) {
      case 0x09: // speed 1
        song.speed1=effectVal;
        break;
      case 0x0f: // speed 2
        song.speed2=effectVal;
        break;
      case 0x0b: // change order
        if (changeOrd==-1) {
          changeOrd=effectVal;
          changePos=0;
        }
        break;
      case 0x0d: // next order
        if (changeOrd==-1) {
          changeOrd=curOrder+1;
          changePos=effectVal;
        }
        break;
      case 0x08: // panning
        dispatchCmd(DivCommand(DIV_CMD_PANNING,i,effectVal));
        break;
      case 0x01: // ramp up
        if (effectVal==0) {
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
        } else {
          chan[i].portaNote=0x60;
          chan[i].portaSpeed=effectVal;
          chan[i].portaStop=true;
          chan[i].nowYouCanStop=false;
          chan[i].stopOnOff=false;
        }
        break;
      case 0x02: // ramp down
        if (effectVal==0) {
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
        } else {
          chan[i].portaNote=0x00;
          chan[i].portaSpeed=effectVal;
          chan[i].portaStop=true;
          chan[i].nowYouCanStop=false;
          chan[i].stopOnOff=false;
        }
        break;
      case 0x03: // portamento
        if (effectVal==0) {
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
          dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,false));
        } else {
          chan[i].portaNote=chan[i].note;
          chan[i].portaSpeed=effectVal;
          chan[i].portaStop=true;
          chan[i].doNote=false;
          chan[i].stopOnOff=true;
          dispatchCmd(DivCommand(DIV_CMD_PRE_PORTA,i,true));
        }
        break;
      case 0x04: // vibrato
        chan[i].vibratoDepth=effectVal&15;
        chan[i].vibratoRate=effectVal>>4;
        dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
        break;
      case 0x0a: // volume ramp
        if (effectVal!=0) {
          if ((effectVal&15)!=0) {
            chan[i].volSpeed=-(effectVal&15)*64;
          } else {
            chan[i].volSpeed=(effectVal>>4)*64;
          }
        } else {
          chan[i].volSpeed=0;
        }
        break;
      case 0x00: // arpeggio
        chan[i].arp=effectVal;
        break;
      case 0x0c: // retrigger
        chan[i].rowDelay=effectVal+1;
        chan[i].delayOrder=whatOrder;
        chan[i].delayRow=whatRow;
        break;
      
      case 0xe0: // arp speed
        song.arpLen=effectVal;
        break;
      case 0xe1: // portamento up
        chan[i].portaNote=chan[i].note+(effectVal&15);
        chan[i].portaSpeed=(effectVal>>4)*4;
        chan[i].portaStop=true;
        chan[i].nowYouCanStop=false;
        chan[i].stopOnOff=true;
        break;
      case 0xe2: // portamento down
        chan[i].portaNote=chan[i].note-(effectVal&15);
        chan[i].portaSpeed=(effectVal>>4)*4;
        chan[i].portaStop=true;
        chan[i].nowYouCanStop=false;
        chan[i].stopOnOff=true;
        break;
      case 0xe3: // vibrato direction
        chan[i].vibratoDir=effectVal;
        break;
      case 0xe4: // vibrato fine
        chan[i].vibratoFine=effectVal;
        break;
      case 0xe5: // pitch
        chan[i].pitch=effectVal-0x80;
        dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
        break;
      case 0xea: // legato mode
        chan[i].legato=effectVal;
        break;
      case 0xec: // delayed note cut
        if (effectVal>0 && effectVal<nextSpeed) {
          chan[i].cut=effectVal+1;
        }
        break;
      case 0xee: // external command
        printf("\x1b[1;36m%d: extern command %d\x1b[m\n",i,effectVal);
        break;
    }
  }

  if (chan[i].doNote) {
    chan[i].vibratoPos=0;
    dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
    if (chan[i].legato) {
      dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
    } else {
      dispatchCmd(DivCommand(DIV_CMD_NOTE_ON,i,chan[i].note,chan[i].volume>>8));
    }
    chan[i].doNote=false;
    /*
    if (!chan[i].keyOn) {
      if (chan[i].portaStop && chan[i].nowYouCanStop) {
        chan[i].portaNote=-1;
        chan[i].portaSpeed=-1;
      }
    }*/
    chan[i].keyOn=true;
  }
  chan[i].nowYouCanStop=true;

  // post effects
  for (int j=0; j<song.pat[i]->effectRows; j++) {
    short effect=pat->data[whatRow][4+(j<<1)];
    short effectVal=pat->data[whatRow][5+(j<<1)];

    if (effectVal==-1) effectVal=0;
    perSystemPostEffect(i,effect,effectVal);
  }
}

void DivEngine::nextRow() {
  static char pb[4096];
  static char pb1[4096];
  static char pb2[4096];
  static char pb3[4096];
  if (view==DIV_STATUS_PATTERN) {
    strcpy(pb1,"");
    strcpy(pb3,"");
    for (int i=0; i<chans; i++) {
      snprintf(pb,4095," %.2x",song.orders.ord[i][curOrder]);
      strcat(pb1,pb);
      
      DivPattern* pat=song.pat[i]->data[curOrder];
      snprintf(pb2,4095,"\x1b[37m %s",
              formatNote(pat->data[curRow][0],pat->data[curRow][1]));
      strcat(pb3,pb2);
      if (pat->data[curRow][3]==-1) {
        strcat(pb3,"\x1b[m--");
      } else {
        snprintf(pb2,4095,"\x1b[1;32m%.2x",pat->data[curRow][3]);
        strcat(pb3,pb2);
      }
      if (pat->data[curRow][2]==-1) {
        strcat(pb3,"\x1b[m--");
      } else {
        snprintf(pb2,4095,"\x1b[0;36m%.2x",pat->data[curRow][2]);
        strcat(pb3,pb2);
      }
      for (int j=0; j<song.pat[i]->effectRows; j++) {
        if (pat->data[curRow][4+(j<<1)]==-1) {
          strcat(pb3,"\x1b[m--");
        } else {
          snprintf(pb2,4095,"\x1b[1;31m%.2x",pat->data[curRow][4+(j<<1)]);
          strcat(pb3,pb2);
        }
        if (pat->data[curRow][5+(j<<1)]==-1) {
          strcat(pb3,"\x1b[m--");
        } else {
          snprintf(pb2,4095,"\x1b[1;37m%.2x",pat->data[curRow][5+(j<<1)]);
          strcat(pb3,pb2);
        }
      }
    }
    printf("| %.2x:%s | \x1b[1;33m%3d%s\x1b[m\n",curOrder,pb1,curRow,pb3);
  }

  for (int i=0; i<chans; i++) {
    chan[i].rowDelay=0;
    processRow(i,false);
  }

  if (++curRow>=song.patLen) {
    nextOrder();
  }
  if (changeOrd>=0) {
    curRow=changePos;
    if (changeOrd<=curOrder) endOfSong=true;
    curOrder=changeOrd;
    if (curOrder>=song.ordersLen) {
      curOrder=0;
      endOfSong=true;
    }
    changeOrd=-1;
  }

  if (speedAB) {
    ticks=song.speed2*(song.timeBase+1);
    nextSpeed=song.speed1;
  } else {
    ticks=song.speed1*(song.timeBase+1);
    nextSpeed=song.speed2;
  }
  speedAB=!speedAB;

  // post row details
  for (int i=0; i<chans; i++) {
    DivPattern* pat=song.pat[i]->data[curOrder];
    if (!(pat->data[curRow][0]==0 && pat->data[curRow][1]==0)) {
      if (pat->data[curRow][0]!=100) {
        dispatchCmd(DivCommand(DIV_CMD_PRE_NOTE,i,ticks));
      }
    }
  }
}

bool DivEngine::nextTick() {
  bool ret=false;
  int divider=60;
  if (song.customTempo) {
    divider=song.hz;
  } else {
    if (song.pal) {
      divider=60;
    } else {
      divider=50;
    }
  }
  cycles=dispatch->rate/divider;
  clockDrift+=dispatch->rate%divider;
  if (clockDrift>=divider) {
    clockDrift-=divider;
    cycles++;
  }

  if (--ticks<=0) {
    ret=endOfSong;
    nextRow();
  }
  // process stuff
  for (int i=0; i<chans; i++) {
    if (chan[i].rowDelay>0) {
      if (--chan[i].rowDelay==0) {
        processRow(i,true);
      }
    }
    if (chan[i].volSpeed!=0) {
      chan[i].volume=(chan[i].volume&0xff)|(dispatchCmd(DivCommand(DIV_CMD_GET_VOLUME,i))<<8);
      chan[i].volume+=chan[i].volSpeed;
      if (chan[i].volume>chan[i].volMax) {
        chan[i].volume=chan[i].volMax;
        chan[i].volSpeed=0;
        dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
      } else if (chan[i].volume<0) {
        chan[i].volSpeed=0;
        chan[i].volume=chan[i].volMax+1;
        dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
      } else {
        dispatchCmd(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
      }
    }
    if (chan[i].vibratoDepth>0) {
      chan[i].vibratoPos+=chan[i].vibratoRate;
      if (chan[i].vibratoPos>=64) chan[i].vibratoPos-=64;
      switch (chan[i].vibratoDir) {
        case 1: // up
          dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(MAX(0,(chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
          break;
        case 2: // down
          dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(MIN(0,(chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
          break;
        default: // both
          dispatchCmd(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+(((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos]*chan[i].vibratoFine)>>4)/15)));
          break;
      }
      
    }
    if (chan[i].portaSpeed>0) {
      if (dispatchCmd(DivCommand(DIV_CMD_NOTE_PORTA,i,chan[i].portaSpeed,chan[i].portaNote))==2 && chan[i].portaStop) {
        chan[i].portaSpeed=0;
        chan[i].note=chan[i].portaNote;
        dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
      }
    }
    if (chan[i].cut>0) {
      if (--chan[i].cut<1) {
        chan[i].note=-1;
        dispatchCmd(DivCommand(DIV_CMD_NOTE_OFF,i));
      }
    }
    if (chan[i].arp!=0 && !chan[i].arpYield && chan[i].portaSpeed<1) {
      if (--chan[i].arpTicks<1) {
        chan[i].arpTicks=song.arpLen;
        chan[i].arpStage++;
        if (chan[i].arpStage>2) chan[i].arpStage=0;
        switch (chan[i].arpStage) {
          case 0:
            dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
            break;
          case 1:
            dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note+(chan[i].arp>>4)));
            break;
          case 2:
            dispatchCmd(DivCommand(DIV_CMD_LEGATO,i,chan[i].note+(chan[i].arp&15)));
            break;
        }
      }
    } else {
      chan[i].arpYield=false;
    }
  }

  // system tick
  dispatch->tick();

  totalTicks++;

  int hz;
  if (song.customTempo) {
    hz=song.hz;
  } else if (song.pal) {
    hz=60;
  } else {
    hz=50;
  }
  fprintf(stderr,"\x1b[2K> %d:%.2d:%.2d.%.2d  %.2x/%.2x:%.3d/%.3d  %4dcmd/s\x1b[G",totalTicks/(hz*3600),(totalTicks/(hz*60))%60,(totalTicks/hz)%60,totalTicks%hz,curOrder,song.ordersLen,curRow,song.patLen,cmdsPerSecond);

  if ((totalTicks%hz)==0) {
    cmdsPerSecond=totalCmds-lastCmds;
    lastCmds=totalCmds;
  }

  return ret;
}

void DivEngine::nextBuf(float** in, float** out, int inChans, int outChans, unsigned int size) {
  size_t runtotal=blip_clocks_needed(bb[0],size);

  if (runtotal>bbInLen) {
    delete bbIn[0];
    delete bbIn[1];
    bbIn[0]=new short[runtotal+256];
    bbIn[1]=new short[runtotal+256];
    bbInLen=runtotal+256;
  }

  size_t runLeft=runtotal;
  size_t runPos=0;
  totalProcessed=0;
  while (runLeft) {
    if (!remainingLoops) {
      memset(bbIn[0]+runPos,0,runLeft*sizeof(short));
      memset(bbIn[1]+runPos,0,runLeft*sizeof(short));
      break;
    } else {
      if (runLeft>=cycles) {
        runLeft-=cycles;
        dispatch->acquire(bbIn[0],bbIn[1],runPos,cycles);
        runPos+=cycles;
        if (nextTick()) {
          if (remainingLoops>0) {
            remainingLoops--;
            if (!remainingLoops) logI("end of song!\n");
          }
        }
      } else {
        dispatch->acquire(bbIn[0],bbIn[1],runPos,runLeft);
        cycles-=runLeft;
        runPos=runtotal;
        break;
      }
    }
  }
  totalProcessed=(1+runPos)*got.rate/dispatch->rate;

  for (size_t i=0; i<runtotal; i++) {
    temp[0]=bbIn[0][i];
    blip_add_delta(bb[0],i,temp[0]-prevSample[0]);
    prevSample[0]=temp[0];
  }

  if (dispatch->isStereo()) for (size_t i=0; i<runtotal; i++) {
    temp[1]=bbIn[1][i];
    blip_add_delta(bb[1],i,temp[1]-prevSample[1]);
    prevSample[1]=temp[1];
  }

  blip_end_frame(bb[0],runtotal);
  blip_read_samples(bb[0],bbOut[0],size,0);

  if (dispatch->isStereo()) {
    blip_end_frame(bb[1],runtotal);
    blip_read_samples(bb[1],bbOut[1],size,0);
  }

  if (out==NULL) return;

  if (dispatch->isStereo()) {
    for (size_t i=0; i<size; i++) {
      out[0][i]=(float)bbOut[0][i]/16384.0;
      out[1][i]=(float)bbOut[1][i]/16384.0;
    }
  } else {
    for (size_t i=0; i<size; i++) {
      out[0][i]=(float)bbOut[0][i]/16384.0;
      out[1][i]=(float)bbOut[0][i]/16384.0;
    }
  }
}
